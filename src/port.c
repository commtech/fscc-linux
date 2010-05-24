/*
	Copyright (C) 2010  Commtech, Inc.
	
	This file is part of fscc-linux.

	fscc-linux is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	fscc-linux is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with fscc-linux.  If not, see <http://www.gnu.org/licenses/>.
	
*/

#include <linux/pci.h>
#include <linux/kernel.h>
#include "port.h"
#include "card.h"
#include "utils.h"
#include "config.h"
#include "isr.h"
#include "sysfs.h"

struct fscc_frame *fscc_port_peek_front_frame(struct fscc_port *port, 
                                              struct list_head *frames);
                                              
void fscc_port_empty_frames(struct fscc_port *port, struct list_head *frames);

void fscc_port_fill_TxFIFO(struct fscc_port *port, const char *data, 
                           unsigned byte_count);
                           
__u32 fscc_port_empty_RxFIFO(struct fscc_port *port, char *buffer, 
                           unsigned byte_count);

void tdu_handler(unsigned long data)
{
	struct fscc_port *port = 0;
	
	port = (struct fscc_port *)data;
	
	printk(KERN_ALERT "%s TDU (Transmit Data Underrun Interrupt)\n", port->name);
}

void tft_handler(unsigned long data)
{
	struct fscc_port *port = 0;
	
	port = (struct fscc_port *)data;

	printk(KERN_DEBUG "%s TFT (Transmit FIFO Trigger Interrupt)\n", port->name);
	
	tasklet_schedule(&port->oframe_tasklet);
}

void rfs_handler(unsigned long data)
{
	struct fscc_port *port = 0;

	port = (struct fscc_port *)data;
	
	printk(KERN_DEBUG "%s RFS (Receive Frame Start Interrupt)\n", port->name);
	
	port->started_frames += 1;
	
	tasklet_schedule(&port->iframe_tasklet);
}

void rft_handler(unsigned long data)
{
	struct fscc_port *port = 0;

	port = (struct fscc_port *)data;
		
	printk(KERN_DEBUG "%s RFT (Receive FIFO Trigger Interrupt)\n", port->name);
	
	tasklet_schedule(&port->iframe_tasklet);
}

void rfe_handler(unsigned long data)
{
	struct fscc_port *port = 0;

	port = (struct fscc_port *)data;
	
	printk(KERN_DEBUG "%s RFE (Receive Frame End Interrupt)\n", port->name);
	
	port->ended_frames += 1;
	
	tasklet_schedule(&port->iframe_tasklet);
}

void iframe_worker(unsigned long data)
{
	
	struct fscc_port *port = 0;
	unsigned byte_count = 0;
	unsigned receive_length = 0;
	unsigned leftover_count = 0;
	unsigned finished_frame = 0;
	__u32 last_data_chunk = 0;
	__u32 frame_status = 0;
	char *buffer = 0;

	port = (struct fscc_port *)data;
	
	return_if_untrue(port);
	
	if (port->handled_frames == port->started_frames)
		return;
		
	if (!port->pending_iframe) {
		port->pending_iframe = fscc_frame_new(0);
		
		return_if_untrue(port->pending_iframe);
		
		printk(KERN_DEBUG "%s F#%i receive started\n", port->name, 
			   port->pending_iframe->number);
	}

	finished_frame = (port->handled_frames < port->ended_frames);

	if (finished_frame) {
		byte_count = fscc_port_get_register(port, 0, BC_FIFO_L_OFFSET) - 2;
		receive_length = byte_count - fscc_frame_get_current_length(port->pending_iframe);
	}
	else {
		//We take off 2 bytes from the amount we can read just in case all the
		//data got transfered in between the time we determined all of the data
		//wasn't in the FIFO and now. In this case, we let the next pass take
		//care of that data.
		byte_count = receive_length = fscc_port_get_RXCNT(port) - 2;
	}
	
	leftover_count = receive_length % 4;
	
	if (!finished_frame)
		receive_length -= leftover_count;	
	
	buffer = (char *)kmalloc(receive_length, GFP_ATOMIC);
		
	if (buffer == NULL) {
		printk(KERN_ALERT "%s F#%i receive rejected (kmalloc)\n",
			   port->name, port->pending_iframe->number);
				   
		fscc_frame_delete(port->pending_iframe);
		port->pending_iframe = 0;
		
			//TODO: Flush rx?
		//fscc_port_flush_rx(port);
		return;
	}
	
	last_data_chunk = fscc_port_empty_RxFIFO(port, buffer, receive_length);
	fscc_frame_add_data(port->pending_iframe, buffer, receive_length);
	
	kfree(buffer);

	if (receive_length == 1)
		printk(KERN_DEBUG "%s F#%i %i byte <= FIFO\n", 
		       port->name, port->pending_iframe->number, receive_length);
	else
		printk(KERN_DEBUG "%s F#%i %i bytes <= FIFO\n", 
		       port->name, port->pending_iframe->number, receive_length);
		       
	if (!finished_frame)
		return;

	switch (leftover_count) {
		case 0:
			frame_status = fscc_port_get_register(port, 0, FIFO_OFFSET) & 0x0000FFFF;
			break;
		
		case 1:
			frame_status = (last_data_chunk & 0x00FFFF00) >> 8;
			break;
		
		case 2:
			frame_status = last_data_chunk >> 16;
			break;
		
		case 3:
			frame_status = (last_data_chunk >> 24) + ((fscc_port_get_register(port, 0, FIFO_OFFSET) & 0x000000FF) << 8);
			break;
	}
	
	if (fscc_port_get_memory_usage(port) + receive_length > memory_cap) {
		printk(KERN_ALERT "%s F#%i receive rejected (memory constraint)\n",
		       port->name, port->pending_iframe->number);
		       
		fscc_frame_delete(port->pending_iframe);
	}
	else {
		if (frame_status & 0x00000004) {
			fscc_frame_trim(port->pending_iframe);		
			list_add_tail(&port->pending_iframe->list, &port->iframes);
		
			printk(KERN_DEBUG "%s F#%i receive successful\n", port->name, 
				   port->pending_iframe->number);
				   
			wake_up_interruptible(&port->input_queue);
		}
		else {
			printk(KERN_ALERT "%s F#%i receive rejected (invalid frame)\n", port->name, 
				   port->pending_iframe->number);
				   
			fscc_frame_delete(port->pending_iframe);
		}
	}

	port->handled_frames += 1;	       
	port->pending_iframe = 0;	
}

void oframe_worker(unsigned long data)
{
	struct fscc_port *port = 0;
	
	unsigned fifo_space = 0;
	unsigned padding_bytes = 0;
	unsigned current_length = 0;
	unsigned target_length = 0;
	unsigned transmit_length = 0;

	port = (struct fscc_port *)data;
	
	return_if_untrue(port);
		
	if (!port->pending_oframe) {
		if (fscc_port_has_oframes(port)) {
			port->pending_oframe = fscc_port_peek_front_frame(port, &port->oframes);
			list_del(&port->pending_oframe->list);
		}
		else
			return;
	}

	current_length = fscc_frame_get_current_length(port->pending_oframe);
	target_length = fscc_frame_get_target_length(port->pending_oframe);
	padding_bytes = target_length % 4 ? 4 - target_length % 4 : 0;
	fifo_space = TX_FIFO_SIZE - fscc_port_get_TXCNT(port);		
	
	transmit_length = (current_length + padding_bytes > fifo_space) ? fifo_space : current_length;
	
	fscc_port_fill_TxFIFO(port, port->pending_oframe->data, transmit_length);
	fscc_frame_remove_data(port->pending_oframe, transmit_length);
		
	if (transmit_length == 1)
		printk(KERN_DEBUG "%s F#%i %i byte => FIFO\n", 
		       port->name, port->pending_oframe->number, transmit_length);
	else
		printk(KERN_DEBUG "%s F#%i %i bytes => FIFO\n", 
		       port->name, port->pending_oframe->number, transmit_length);
	
	/* If this is the first time we add data to the FIFO for this frame */
	if (current_length == target_length)
		fscc_port_set_register(port, 0, BC_FIFO_L_OFFSET, target_length);
	
	if (fscc_frame_is_empty(port->pending_oframe)) {	
		printk(KERN_DEBUG "%s F#%i sending\n", port->name, port->pending_oframe->number);	
		fscc_frame_delete(port->pending_oframe);
		port->pending_oframe = 0;
	}	

	fscc_port_execute_XF(port);
}

struct fscc_port *fscc_port_new(struct fscc_card *card, unsigned channel, 
                                unsigned major_number, unsigned minor_number, 
                                struct class *class, 
                                struct file_operations *fops)
{
	struct fscc_port *port = 0;
	unsigned irq_num = 0;
	
	port = (struct fscc_port*)kmalloc(sizeof(struct fscc_port), GFP_KERNEL);
	
	port->started_frames = 0;
	port->ended_frames = 0;
	port->handled_frames = 0;
	
	port->name = (char *)kmalloc(10, GFP_KERNEL);
	sprintf(port->name, "%s%u", DEVICE_NAME, minor_number);
		
	port->channel = channel;
	port->card = card;
	
	//TODO: This needs to be better
	if (fscc_port_get_PREV(port) == 0xff) {
		printk(KERN_NOTICE "%s couldn't initialize\n", port->name);
		
		if (port->name)
			kfree(port->name);
			
		kfree(port);
		return 0;
	}
	
	INIT_LIST_HEAD(&port->list);
	INIT_LIST_HEAD(&port->oframes);
	INIT_LIST_HEAD(&port->iframes);

	port->dev_t = MKDEV(major_number, minor_number);
	port->class = class;
	port->pending_iframe = 0;
	port->pending_oframe = 0;
	
	init_MUTEX(&port->read_semaphore);
	init_MUTEX(&port->write_semaphore);
	init_MUTEX(&port->poll_semaphore);
	
	init_waitqueue_head(&port->input_queue);
	init_waitqueue_head(&port->output_queue);
	
	cdev_init(&port->cdev, fops);
	port->cdev.owner = THIS_MODULE;
	
	if (cdev_add(&port->cdev, port->dev_t, 1) < 0) {
		printk(KERN_ERR "%s cdev_add failed\n", port->name);
		return 0;
	}
	
	list_add_tail(&port->list, &card->ports);
	
	port->device = device_create(class, 0, port->dev_t, port, port->name);
	if (port->device <= 0) {
		printk(KERN_ERR "%s device_create failed\n", port->name);
		return 0;
	}
	
	tasklet_init(&port->rfs_tasklet, rfs_handler, (unsigned long)port);
	tasklet_init(&port->rft_tasklet, rft_handler, (unsigned long)port);
	tasklet_init(&port->rfe_tasklet, rfe_handler, (unsigned long)port);
	tasklet_init(&port->tft_tasklet, tft_handler, (unsigned long)port);
	tasklet_init(&port->tdu_tasklet, tdu_handler, (unsigned long)port);
	
	tasklet_init(&port->oframe_tasklet, oframe_worker, (unsigned long)port);
	tasklet_init(&port->iframe_tasklet, iframe_worker, (unsigned long)port);
	
	fscc_port_set_register(port, 0, IMR_OFFSET, 0x0f000000);
	fscc_port_set_register(port, 0, BGR_OFFSET, 0x00000002);
	fscc_port_set_register(port, 0, CCR0_OFFSET, 0x0000001c);
	fscc_port_set_register(port, 0, CCR1_OFFSET, 0x00000008);	
	
	//TODO: Change this to a better RxFIFO level
	fscc_port_set_register(port, 0, FIFOT_OFFSET, 0x08000200);	
	
	fscc_port_execute_RRES(port);
	fscc_port_execute_TRES(port);
	
	irq_num = card->pci_dev->irq;
	if (request_irq(irq_num, &fscc_isr, IRQF_SHARED, port->name, port)) {
		printk(KERN_ERR "%s request_irq failed on irq %i\n", port->name, irq_num);
		return 0;
	}
	
	if (sysfs_create_group(&port->device->kobj, &port_registers_attr_group)) {
		printk(KERN_ERR "%s sysfs_create_group\n", port->name);
		return 0;
	}
	
	if (sysfs_create_group(&port->device->kobj, &port_commands_attr_group)) {
		printk(KERN_ERR "%s sysfs_create_group\n", port->name);
		return 0;
	}
	
	if (sysfs_create_group(&port->device->kobj, &port_info_attr_group)) {
		printk(KERN_ERR "%s sysfs_create_group\n", port->name);
		return 0;
	}
	
	printk(KERN_INFO "%s revision %x.%02x\n", port->name, 
	       fscc_port_get_PREV(port), fscc_port_get_FREV(port));
	
	return port;
}

void fscc_port_delete(struct fscc_port *port)
{
	return_if_untrue(port);
		
	if (port->pending_iframe)
		fscc_frame_delete(port->pending_iframe);
		
	if (port->pending_oframe)
		fscc_frame_delete(port->pending_oframe);
	
	fscc_port_empty_frames(port, &port->iframes);
	fscc_port_empty_frames(port, &port->oframes);
	
	free_irq(port->card->pci_dev->irq, port);	
	
	device_destroy(port->class, port->dev_t);
	cdev_del(&port->cdev);
	list_del(&port->list);
	
	if (port->name)
		kfree(port->name);
		
	kfree(port);
}

// Returns -ENOMEM if write size will go over user cap.
// Returns -ENOMEM if kmalloc fails. TODO: Should this be different than above
/* Length is user data length. Without 32bit padding. */
int fscc_port_write(struct fscc_port *port, const char *data, unsigned length)
{
	struct fscc_frame *frame = 0;
	char *temp_storage = 0;
	
	return_val_if_untrue(port, 0);
	
	if (fscc_port_get_memory_usage(port) + length > memory_cap)
		return -ENOMEM;
	
	temp_storage = (char *)kmalloc(length, GFP_KERNEL);	
	return_val_if_untrue(temp_storage != NULL, -ENOMEM);	
	copy_from_user(temp_storage, data, length);
	
	frame = fscc_frame_new(length);	
	fscc_frame_add_data(frame, temp_storage, length);
	
	kfree(temp_storage);
	
	list_add_tail(&frame->list, &port->oframes);	
	tasklet_schedule(&port->oframe_tasklet);
	
	return 0;
}

//Returns ENOBUFS if count is smaller than pending frame size
//Buf needs to be a user buffer
ssize_t fscc_port_read(struct fscc_port *port, char *buf, size_t count)
{
	struct fscc_frame *frame = 0;
	unsigned sent_length = 0;
	
	return_val_if_untrue(port, 0);
	
	frame = fscc_port_peek_front_frame(port, &port->iframes);
	
	if (frame && fscc_frame_is_full(frame)) {
		if (count < fscc_frame_get_target_length(frame))
			return ENOBUFS;
			
		copy_to_user(buf, fscc_frame_get_remaining_data(frame), 
		             fscc_frame_get_target_length(frame));
		             
		sent_length = fscc_frame_get_target_length(frame);
		list_del(&frame->list); 
		fscc_frame_delete(frame);
	}
	
	return sent_length;
}

/* Must have room available */
/* Puts data into the FIFO */
void fscc_port_fill_TxFIFO(struct fscc_port *port, const char *data, 
                           unsigned byte_count)
{
	unsigned leftover_count = 0;
	
	return_if_untrue(port);
	
	leftover_count = byte_count % 4;
	
	fscc_port_set_register_rep(port, 0, FIFO_OFFSET, data, 
	                           (byte_count - leftover_count) / 4);
	
	/* Writes the leftover bytes (non 4 byte chunk) */
	if (leftover_count)
		fscc_port_set_register(port, 0, FIFO_OFFSET, chars_to_u32(data + (byte_count - leftover_count)));
}

/* Returns the last data chunk retrieved */
__u32 fscc_port_empty_RxFIFO(struct fscc_port *port, char *buffer,
                             unsigned byte_count)
{
	unsigned leftover_count = 0;
	__u32 incoming_data = 0;
	
	return_val_if_untrue(port, 0);
	
	leftover_count = byte_count % 4;
	
	fscc_port_get_register_rep(port, 0, FIFO_OFFSET, buffer, 
	                           (byte_count - leftover_count) / 4);
	
	if (leftover_count) {
		incoming_data = fscc_port_get_register(port, 0, FIFO_OFFSET);
		memmove(buffer + (byte_count - leftover_count), (char *)(&incoming_data), leftover_count);
	}
	
	return incoming_data;
}

void fscc_port_empty_frames(struct fscc_port *port, struct list_head *frames)
{	
	struct list_head *current_node = 0;
	struct list_head *temp_node = 0;
	
	return_if_untrue(port);
	
	list_for_each_safe(current_node, temp_node, frames) {
		struct fscc_frame *current_frame = 0;
		current_frame = list_entry(current_node, struct fscc_frame, list);
		fscc_frame_delete(current_frame);
	}
}

struct fscc_frame *fscc_port_peek_front_frame(struct fscc_port *port, 
                                              struct list_head *frames)
{
	struct fscc_frame *current_frame = 0;	
	
	return_val_if_untrue(port, 0);
	
	list_for_each_entry(current_frame, frames, list) {
		return current_frame;
	}
	
	return 0;
}

bool fscc_port_has_iframes(struct fscc_port *port)
{
	return_val_if_untrue(port, 0);
	
	return !list_empty(&port->iframes);
}

bool fscc_port_has_oframes(struct fscc_port *port)
{
	return_val_if_untrue(port, 0);
	
	return !list_empty(&port->oframes);
}

__u32 fscc_port_get_register(struct fscc_port *port, unsigned bar, 
                             unsigned register_offset)
{
	unsigned offset = 0;
	__u32 value = 0;
	
	return_val_if_untrue(port, 0);
	
	offset = register_offset;
	
	if (port->channel == 1)
		offset += 0x80;
		
	value = fscc_card_get_register(port->card, bar, offset);
	
	return value;
}

void fscc_port_get_register_rep(struct fscc_port *port, unsigned bar, 
                                unsigned register_offset, char *buf,
                                unsigned long chunks)
{
	unsigned offset = 0;
	
	return_if_untrue(port);
	
	offset = register_offset;
	
	if (port->channel == 1)
		offset += 0x80;
		
	fscc_card_get_register_rep(port->card, bar, offset, buf, chunks);
}

void fscc_port_set_register(struct fscc_port *port, unsigned bar, 
                            unsigned register_offset, __u32 value)
{
	unsigned offset = 0;
	
	return_if_untrue(port);
	
	offset = register_offset;
	
	if (port->channel == 1)
		offset += 0x80;
		
	fscc_card_set_register(port->card, bar, offset, value);
}

void fscc_port_set_register_rep(struct fscc_port *port, unsigned bar,

                                unsigned register_offset, const char *data,
                                unsigned long chunks) 
{
	unsigned offset = 0;
	
	return_if_untrue(port);
	
	offset = register_offset;	

	if (port->channel == 1)
		offset += 0x80;
	
	fscc_card_set_register_rep(port->card, bar, offset, data, chunks);
}

__u32 fscc_port_get_TXCNT(struct fscc_port *port)
{
	__u32 fifo_bc_value = 0;	
	
	return_val_if_untrue(port, 0);
	
	fifo_bc_value = fscc_port_get_register(port, 0, FIFO_BC_OFFSET);
	
	return (fifo_bc_value & 0x1FFF0000) >> 16;
}

__u32 fscc_port_get_RXCNT(struct fscc_port *port)
{
	__u32 fifo_bc_value = 0;	
	
	return_val_if_untrue(port, 0);
	
	fifo_bc_value = fscc_port_get_register(port, 0, FIFO_BC_OFFSET);
	
	return fifo_bc_value & 0x00003FFF;
}

__u32 fscc_port_get_RXTRG(struct fscc_port *port)
{
	__u32 fifot_value = 0;	
	
	return_val_if_untrue(port, 0);
	
	fifot_value = fscc_port_get_register(port, 0, FIFOT_OFFSET);
	
	return fifot_value & 0x00001FFF;
}

__u8 fscc_port_get_FREV(struct fscc_port *port)
{
	__u32 vstr_value = 0;	
	
	return_val_if_untrue(port, 0);
		
	vstr_value = fscc_port_get_register(port, 0, VSTR_OFFSET);
	
	return (__u8)((vstr_value & 0x000000FF));
}

__u8 fscc_port_get_PREV(struct fscc_port *port)
{
	__u32 vstr_value = 0;	
	
	return_val_if_untrue(port, 0);
		
	vstr_value = fscc_port_get_register(port, 0, VSTR_OFFSET);
	
	return (__u8)((vstr_value & 0x0000FF00) >> 8);
}

unsigned fscc_port_get_CE(struct fscc_port *port)
{
	__u32 star_value = 0;		
	
	return_val_if_untrue(port, 0);
	
	star_value = fscc_port_get_register(port, 0, STAR_OFFSET);
	
	return (unsigned)((star_value & 0x00040000) >> 18);
}

void fscc_port_execute_TRES(struct fscc_port *port)
{	
	return_if_untrue(port);
	
	fscc_port_set_register(port, 0, CMDR_OFFSET, 0x08000000);
}

void fscc_port_execute_RRES(struct fscc_port *port)
{
	return_if_untrue(port);
	
	fscc_port_set_register(port, 0, CMDR_OFFSET, 0x00020000);
}

void fscc_port_execute_XF(struct fscc_port *port)
{	
	return_if_untrue(port);
	
	fscc_port_set_register(port, 0, CMDR_OFFSET, 0x01000000);
}

void fscc_port_suspend(struct fscc_port *port)
{
	unsigned i = 0;
	
	return_if_untrue(port);	
	
	for (i = 0; i < sizeof(struct fscc_registers) / sizeof(int32_t); i++)
		((uint32_t *)&port->register_storage)[i] = fscc_port_get_register(port, 0, i * 4);
}

void fscc_port_resume(struct fscc_port *port)
{
	unsigned i = 0;
	
	return_if_untrue(port);	
	
	for (i = 0; i < sizeof(struct fscc_registers) / sizeof(int32_t); i++) {
		__u32 current_value = 0;	
		
		current_value = fscc_port_get_register(port, 0, i * 4);
		
		if (current_value != ((uint32_t *)&port->register_storage)[i]) {
			printk(KERN_DEBUG "%s register 0x%02x restoring 0x%08x => 0x%08x\n", 
			       port->name, i * 4, current_value, 
			       offset_to_value(&port->register_storage, i * 4));
			       
			fscc_port_set_register(port, 0, i * 4, 
			                    offset_to_value(&port->register_storage, i * 4));
		}
	}
	
}

void fscc_port_flush_tx(struct fscc_port *port)
{	
	return_if_untrue(port);	
	
	printk(KERN_DEBUG "%s flush_tx\n", port->name);
	
	fscc_port_execute_TRES(port);
		
	if (port->pending_oframe)
		fscc_frame_delete(port->pending_oframe);
	
	fscc_port_empty_frames(port, &port->oframes);
}

void fscc_port_flush_rx(struct fscc_port *port)
{
	return_if_untrue(port);	
	
	printk(KERN_DEBUG "%s flush_rx\n", port->name);
	
	fscc_port_execute_RRES(port);
		
	if (port->pending_iframe)
		fscc_frame_delete(port->pending_iframe);
	
	fscc_port_empty_frames(port, &port->iframes);
}

unsigned fscc_port_get_frames_qty(struct fscc_port *port, 
                                  struct list_head *frames)
{
	struct fscc_frame *current_frame = 0;	
	unsigned qty = 0;
	
	return_val_if_untrue(port, 0);		
	
	list_for_each_entry(current_frame, frames, list) {
		qty++;
	}
	
	return qty;
}

unsigned fscc_port_get_output_frames_qty(struct fscc_port *port)
{
	return_val_if_untrue(port, 0);	
	
    return fscc_port_get_frames_qty(port, &port->oframes);
}

unsigned fscc_port_get_input_frames_qty(struct fscc_port *port)
{	
	return_val_if_untrue(port, 0);	
	
    return fscc_port_get_frames_qty(port, &port->iframes);
}

unsigned fscc_port_get_output_memory_usage(struct fscc_port *port)
{
	struct fscc_frame *current_frame = 0;	
	unsigned memory = 0;
	
	return_val_if_untrue(port, 0);	
	
	list_for_each_entry(current_frame, &port->oframes, list) {
		memory += fscc_frame_get_current_length(current_frame);
	}	
	
	if (port->pending_oframe)
		memory += fscc_frame_get_current_length(port->pending_oframe);
	
	return memory;
}

unsigned fscc_port_get_input_memory_usage(struct fscc_port *port)
{
	struct fscc_frame *current_frame = 0;	
	unsigned memory = 0;
	
	return_val_if_untrue(port, 0);
	
	list_for_each_entry(current_frame, &port->iframes, list) {
		memory += fscc_frame_get_current_length(current_frame);
	}
	
	if (port->pending_iframe)
		memory += fscc_frame_get_current_length(port->pending_iframe);
	
	return memory;
}

unsigned fscc_port_get_memory_usage(struct fscc_port *port)
{
	unsigned output_memory = 0;
	unsigned input_memory = 0;
	
	return_val_if_untrue(port, 0);
	
	output_memory = fscc_port_get_output_memory_usage(port);
	input_memory = fscc_port_get_input_memory_usage(port);

	return output_memory + input_memory;
}

