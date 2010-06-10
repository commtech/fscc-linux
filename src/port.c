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

#include <linux/version.h> /* LINUX_VERSION_CODE, KERNEL_VERSION */
#include <linux/fs.h> /* TODO: Needed to compile but isn't correct */

#include <asm/uaccess.h> /* copy_*_user in <= 2.6.24 */

#include "port.h"
#include "frame.h" /* struct fscc_frame */
#include "card.h" /* fscc_card_* */
#include "utils.h" /* return_{val_}_if_untrue, chars_to_u32, ... */
#include "config.h" /* DEVICE_NAME, DEFAULT_* */
#include "isr.h" /* fscc_isr */
#include "sysfs.h" /* port_*_attribute_group */

#define STATUS_LENGTH 2
#define TX_FIFO_SIZE 4096
                                              
void fscc_port_empty_frames(struct fscc_port *port, struct list_head *frames);

void fscc_port_fill_TxFIFO(struct fscc_port *port, const char *data, 
                           unsigned byte_count);
                           
__u32 fscc_port_empty_RxFIFO(struct fscc_port *port, char *buffer, 
                           unsigned byte_count);
                           
void fscc_port_execute_GO_R(struct fscc_port *port);
void fscc_port_execute_GO_T(struct fscc_port *port);
void fscc_port_execute_STOP_R(struct fscc_port *port);
void fscc_port_execute_STOP_T(struct fscc_port *port);
void fscc_port_execute_RST_R(struct fscc_port *port);
void fscc_port_execute_RST_T(struct fscc_port *port);

void print_worker(unsigned long data)
{
	struct fscc_port *port = 0;
	unsigned isr_value = 0;
	
	port = (struct fscc_port *)data;
	isr_value = port->last_isr_value;
	port->last_isr_value = 0;
	
	dev_dbg(port->device, "interrupt: 0x%08x\n", isr_value);
	
	if (isr_value & RFE)
		dev_dbg(port->device, "RFE (Receive Frame End Interrupt)\n");
		
	if (isr_value & RFT)
		dev_dbg(port->device, "RFT (Receive FIFO Trigger Interrupt)\n");
		
	if (isr_value & RFS)
		dev_dbg(port->device, "RFS (Receive Frame Start Interrupt)\n");
	
	if (isr_value & RFO)
		dev_notice(port->device, "RFO (Receive Frame Overflow Interrupt)\n");
	
	if (isr_value & RDO)
		dev_notice(port->device, "RDO (Receive Data Overflow Interrupt)\n");
	
	if (isr_value & RFL)
		dev_notice(port->device, "RFL (Receive Frame Lost Interrupt)\n");
	
	if (isr_value & TIN)
		dev_dbg(port->device, "TIN (Timer Expiration Interrupt)\n");
	
	if (isr_value & TFT)
		dev_dbg(port->device, "TFT (Transmit FIFO Trigger Interrupt)\n");
		
	if (isr_value & TDU)
		dev_notice(port->device, "TDU (Transmit Data Underrun Interrupt)\n");
	
	if (isr_value & TDU)
		dev_notice(port->device, "TDU (Transmit Data Underrun Interrupt)\n");
	
	if (isr_value & ALLS)
		dev_dbg(port->device, "ALLS (All Sent Interrupt)\n");
	
	if (isr_value & CTSS)
		dev_dbg(port->device, "CTSS (CTS State Change Interrupt)\n");
	
	if (isr_value & DSRC)
		dev_dbg(port->device, "DSRC (DSR Change Interrupt)\n");
	
	if (isr_value & CDC)
		dev_dbg(port->device, "CDC (CD Change Interrupt)\n");
		
	if (isr_value & DT_STOP)
		dev_dbg(port->device, "DT_STOP (DMA Transmitter Full Stop indication)\n");
		
	if (isr_value & DR_STOP)
		dev_dbg(port->device, "DR_STOP (DMA Receiver Full Stop indication)\n");
		
	if (isr_value & DT_FE)
		dev_dbg(port->device, "DT_FE (DMA Transmit Frame End indication)\n");
		
	if (isr_value & DR_FE)
		dev_dbg(port->device, "DR_FE (DMA Receive Frame End indication)\n");
		
	if (isr_value & DT_HI)
		dev_dbg(port->device, "DT_HI (DMA Transmit Host Interrupt indication)\n");
		
	if (isr_value & DR_HI)
		dev_dbg(port->device, "DR_HI (DMA Receive Host Interrupt indication)\n");
}

void iframe_worker(unsigned long data)
{	
	struct fscc_port *port = 0;
	int receive_length = 0; //need to be signed
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
		port->pending_iframe = fscc_frame_new(0, 0, port);
		
		return_if_untrue(port->pending_iframe);
		
		dev_dbg(port->device, "F#%i receive started\n", 
		        port->pending_iframe->number);
	}

	finished_frame = (port->handled_frames < port->ended_frames);
	
	if (finished_frame) {
		unsigned bc_fifo_l = 0;
		
		bc_fifo_l = fscc_port_get_register(port, 0, BC_FIFO_L_OFFSET);
		receive_length = bc_fifo_l - STATUS_LENGTH - fscc_frame_get_current_length(port->pending_iframe);
	} else {
		unsigned rxcnt = 0;
		
		//We take off 2 bytes from the amount we can read just in case all the
		//data got transfered in between the time we determined all of the data
		//wasn't in the FIFO and now. In this case, we let the next pass take
		//care of that data.		
		
		//3 is the maximum amount of leftover bytes
		rxcnt = fscc_port_get_RXCNT(port);
		receive_length = rxcnt - STATUS_LENGTH - 3;
		receive_length -= receive_length % 4;
	}
	
	if (receive_length > 0) {
		buffer = kmalloc(receive_length, GFP_ATOMIC);
		
		if (buffer == NULL) {
			dev_notice(port->device, "F#%i receive rejected (kmalloc of %i bytes)\n",
				   port->pending_iframe->number, receive_length);
					   
			fscc_frame_delete(port->pending_iframe);
			port->pending_iframe = 0;
		
			//TODO: Flush rx?
			//fscc_port_flush_rx(port);
			return;
		}
	
		last_data_chunk = fscc_port_empty_RxFIFO(port, buffer, receive_length);
		fscc_frame_add_data(port->pending_iframe, buffer, receive_length);

		kfree(buffer);

		dev_dbg(port->device, "F#%i %i byte%s <= FIFO\n", 
				port->pending_iframe->number, receive_length,
				(receive_length == 1) ? "" : "s");
	}
		       
	if (!finished_frame)
		return;
	
	leftover_count = receive_length % 4;
	
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
	
	if (fscc_memory_usage() + receive_length > memory_cap) {
		dev_notice(port->device, "F#%i receive rejected (memory constraint)\n",
		          port->pending_iframe->number);
		       
		fscc_frame_delete(port->pending_iframe);
	} else {
		fscc_frame_set_status(port->pending_iframe, frame_status);
		fscc_frame_trim(port->pending_iframe);
		
		list_add_tail(&port->pending_iframe->list, &port->iframes);
		
		dev_dbg(port->device, "F#%i receive successful\n", 
				    port->pending_iframe->number);
				   
		wake_up_interruptible(&port->input_queue);
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
		} else {
			return;
		}
	}

	current_length = fscc_frame_get_current_length(port->pending_oframe);
	target_length = fscc_frame_get_target_length(port->pending_oframe);
	padding_bytes = target_length % 4 ? 4 - target_length % 4 : 0;
	fifo_space = TX_FIFO_SIZE - fscc_port_get_TXCNT(port);		
	
	transmit_length = (current_length + padding_bytes > fifo_space) ? fifo_space : current_length;
	fscc_port_fill_TxFIFO(port, port->pending_oframe->data, transmit_length);
	fscc_frame_remove_data(port->pending_oframe, transmit_length);
		
	dev_dbg(port->device, "F#%i %i byte%s => FIFO\n", 
		    port->pending_oframe->number, transmit_length,
		    (transmit_length == 1) ? "" : "s");
		       
	/* If this is the first time we add data to the FIFO for this frame */
	if (current_length == target_length)
		fscc_port_set_register(port, 0, BC_FIFO_L_OFFSET, target_length);
	
	if (fscc_frame_is_empty(port->pending_oframe)) {	
		dev_dbg(port->device, "F#%i sending\n", port->pending_oframe->number);	
		fscc_frame_delete(port->pending_oframe);
		port->pending_oframe = 0;
	}	

	fscc_port_execute_XF(port);
}

struct fscc_port *fscc_port_new(struct fscc_card *card, unsigned channel, 
                                unsigned major_number, unsigned minor_number, 
                                struct device *parent, struct class *class, 
                                struct file_operations *fops)
{
	struct fscc_port *port = 0;
	struct fscc_registers initial_registers;
	unsigned irq_num = 0;
	
	port = kmalloc(sizeof(*port), GFP_KERNEL);
	
	port->started_frames = 0;
	port->ended_frames = 0;
	port->handled_frames = 0;
	
	port->name = kmalloc(10, GFP_KERNEL);
	sprintf(port->name, "%s%u", DEVICE_NAME, minor_number);
		
	port->channel = channel;
	port->card = card;
	port->class = class;
	port->pending_iframe = 0;
	port->pending_oframe = 0;
	port->dev_t = MKDEV(major_number, minor_number);
	port->append_status = DEFAULT_APPEND_STATUS_VALUE;
	
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)
	port->device = device_create(port->class, parent, port->dev_t, port, "%s", 
	                             port->name);
#else
	port->device = device_create(port->class, parent, port->dev_t, "%s", 
	                             port->name);
	                                     
	dev_set_drvdata(port->device, port);
#endif

	if (port->device <= 0) {
		printk(KERN_ERR DEVICE_NAME " %s: device_create failed\n", port->name);
		return 0;
	}
	
	//TODO: This needs to be better
	if (fscc_port_get_PREV(port) == 0xff) {
		dev_warn(port->device, "couldn't initialize\n");
		
		if (port->name)
			kfree(port->name);
			
		kfree(port);
		return 0;
	}
	
	INIT_LIST_HEAD(&port->list);
	INIT_LIST_HEAD(&port->oframes);
	INIT_LIST_HEAD(&port->iframes);
	
	init_MUTEX(&port->read_semaphore);
	init_MUTEX(&port->write_semaphore);
	init_MUTEX(&port->poll_semaphore);
	
	init_waitqueue_head(&port->input_queue);
	
	cdev_init(&port->cdev, fops);
	port->cdev.owner = THIS_MODULE;
	
	if (cdev_add(&port->cdev, port->dev_t, 1) < 0) {
		dev_err(port->device, "cdev_add failed\n");
		return 0;
	}
	
	list_add_tail(&port->list, fscc_card_get_ports(card));
	
	tasklet_init(&port->oframe_tasklet, oframe_worker, (unsigned long)port);
	tasklet_init(&port->iframe_tasklet, iframe_worker, (unsigned long)port);
	tasklet_init(&port->print_tasklet, print_worker, (unsigned long)port);
	
	port->last_isr_value = 0;
	
	memset(&initial_registers, -1, sizeof(initial_registers));
	
	initial_registers.FIFOT = DEFAULT_FIFOT_VALUE;
	initial_registers.CCR0 = DEFAULT_CCR0_VALUE;
	initial_registers.CCR1 = DEFAULT_CCR1_VALUE;
	initial_registers.CCR2 = DEFAULT_CCR2_VALUE;
	initial_registers.BGR = DEFAULT_BGR_VALUE;
	initial_registers.SSR = DEFAULT_SSR_VALUE;
	initial_registers.SMR = DEFAULT_SMR_VALUE;
	initial_registers.TSR = DEFAULT_TSR_VALUE;
	initial_registers.TMR = DEFAULT_TMR_VALUE;
	initial_registers.RAR = DEFAULT_RAR_VALUE;
	initial_registers.RAMR = DEFAULT_RAMR_VALUE;
	initial_registers.PPR = DEFAULT_PPR_VALUE;
	initial_registers.TCR = DEFAULT_TCR_VALUE;
	initial_registers.IMR = DEFAULT_IMR_VALUE;
	
	fscc_port_set_registers(port, &initial_registers);
	
	fscc_port_set_register(port, 2, DMACCR_OFFSET, 0x02000000);
	
	fscc_port_execute_RST_T(port);
	fscc_port_execute_RST_R(port);
	
	fscc_port_execute_RRES(port);
	fscc_port_execute_TRES(port);
	
	irq_num = fscc_card_get_irq(card);	
	
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 18)
	if (request_irq(irq_num, &fscc_isr, IRQF_SHARED, port->name, port)) {
#else
	if (request_irq(irq_num, &fscc_isr, SA_SHIRQ, port->name, port)) {
#endif
		dev_err(port->device, "request_irq failed on irq %i\n", irq_num);
		return 0;
	}

//TODO: Can I get this to work?	
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)
	if (sysfs_create_group(&port->device->kobj, &port_registers_attr_group)) {
		dev_err(port->device, "sysfs_create_group\n");
		return 0;
	}
	
	if (sysfs_create_group(&port->device->kobj, &port_commands_attr_group)) {
		dev_err(port->device, "sysfs_create_group\n");
		return 0;
	}
	
	if (sysfs_create_group(&port->device->kobj, &port_info_attr_group)) {
		dev_err(port->device, "sysfs_create_group\n");
		return 0;
	}
#endif
	
	dev_info(port->device, "revision %x.%02x\n", fscc_port_get_PREV(port), 
	         fscc_port_get_FREV(port));
	
	return port;
}

void fscc_port_delete(struct fscc_port *port)
{
	unsigned irq_num = 0;
	
	return_if_untrue(port);
	
	fscc_port_execute_STOP_T(port);
	fscc_port_execute_STOP_R(port);
	fscc_port_execute_RST_T(port);
	fscc_port_execute_RST_R(port);
	
	fscc_port_set_register(port, 2, DMACCR_OFFSET, 0x00000000);
		
	if (port->pending_iframe)
		fscc_frame_delete(port->pending_iframe);
		
	if (port->pending_oframe)
		fscc_frame_delete(port->pending_oframe);
	
	fscc_port_empty_frames(port, &port->iframes);
	fscc_port_empty_frames(port, &port->oframes);
	
	irq_num = fscc_card_get_irq(port->card);
	free_irq(irq_num, port);
	
	device_destroy(port->class, port->dev_t);
	cdev_del(&port->cdev);
	list_del(&port->list);
	
	if (port->name)
		kfree(port->name);
		
	kfree(port);
}

void fscc_port_write(struct fscc_port *port, const char *data, unsigned length)
{
	struct fscc_frame *frame = 0;
	char *temp_storage = 0;
	unsigned uncopied_bytes = 0;
	
	return_if_untrue(port);
	
	temp_storage = kmalloc(length, GFP_KERNEL);	
	return_if_untrue(temp_storage != NULL);
	
	uncopied_bytes = copy_from_user(temp_storage, data, length);
	return_if_untrue(!uncopied_bytes);
	
	if (port->card->dma) {
		frame = fscc_frame_new(length, 1, port);
		fscc_frame_add_data(frame, temp_storage, length);
		
		list_add_tail(&frame->list, &port->oframes);
		              
		fscc_port_set_register(port, 2, DMA_TX_BASE_OFFSET, 
		                       cpu_to_le32(frame->descriptor_handle));
		
		fscc_port_execute_GO_T(port);
	} else {
		frame = fscc_frame_new(length, 0, port);
		fscc_frame_add_data(frame, temp_storage, length);
	
		list_add_tail(&frame->list, &port->oframes);	
		tasklet_schedule(&port->oframe_tasklet);
	}
	
	kfree(temp_storage);
}

//Returns ENOBUFS if count is smaller than pending frame size
//Buf needs to be a user buffer
ssize_t fscc_port_read(struct fscc_port *port, char *buf, size_t count)
{
	struct fscc_frame *frame = 0;
	unsigned data_length = 0;
	unsigned uncopied_bytes = 0;
	
	return_val_if_untrue(port, 0);
	
	if (fscc_port_get_input_frames_qty(port) == 0)
		return 0;
	
	frame = fscc_port_peek_front_frame(port, &port->iframes);
	
	data_length = fscc_frame_get_target_length(frame);	
	data_length += (port->append_status) ? STATUS_LENGTH : 0;
	
	if (count < data_length)
			return ENOBUFS;
			
	uncopied_bytes = copy_to_user(buf, fscc_frame_get_remaining_data(frame), 
			                      fscc_frame_get_target_length(frame));
			
	return_val_if_untrue(!uncopied_bytes, 0);	
		
	if (port->append_status) {
		__u16 status = 0;
		
		status = fscc_frame_get_status(frame);
			
		uncopied_bytes = copy_to_user(buf + fscc_frame_get_target_length(frame), 
		                              (void *)(&status), STATUS_LENGTH);
					
	}
		
	list_del(&frame->list); 
	fscc_frame_delete(frame);
	
	return data_length;
}

void fscc_port_fill_TxFIFO(struct fscc_port *port, const char *data, 
                           unsigned byte_count)
{
	unsigned leftover_count = 0;
	unsigned chunks = 0;
	
	return_if_untrue(port);
	
	leftover_count = byte_count % 4;
	chunks = (byte_count - leftover_count) / 4;
	
	if (chunks)
		fscc_port_set_register_rep(port, 0, FIFO_OFFSET, data, chunks);
	
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
	unsigned chunks = 0;
	
	return_val_if_untrue(port, 0);
	return_val_if_untrue(buffer, 0);
	return_val_if_untrue(byte_count, 0);
	
	leftover_count = byte_count % 4;
	chunks = (byte_count - leftover_count) / 4;
	
	if (chunks)
		fscc_port_get_register_rep(port, 0, FIFO_OFFSET, buffer, chunks);
	
	if (leftover_count) {
		incoming_data = fscc_port_get_register(port, 0, FIFO_OFFSET);
		
		memmove(buffer + (byte_count - leftover_count), 
		        (char *)(&incoming_data), leftover_count);
	}
	
	return incoming_data;
}

void fscc_port_empty_frames(struct fscc_port *port, struct list_head *frames)
{	
	struct list_head *current_node = 0;
	struct list_head *temp_node = 0;
	
	return_if_untrue(port);
	return_if_untrue(frames);
	
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
	return_val_if_untrue(frames, 0);
	
	list_for_each_entry(current_frame, frames, list) {
		return current_frame;
	}
	

	return 0;
}

unsigned fscc_port_has_iframes(struct fscc_port *port)
{
	return_val_if_untrue(port, 0);
	
	return !list_empty(&port->iframes);
}

unsigned fscc_port_has_oframes(struct fscc_port *port)
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
	return_val_if_untrue(bar <= 2, 0);

	offset = port_offset(port, bar, register_offset);
		
	value = fscc_card_get_register(port->card, bar, offset);
	
	return value;
}

void fscc_port_get_register_rep(struct fscc_port *port, unsigned bar, 
                                unsigned register_offset, char *buf,
                                unsigned long chunks)
{
	unsigned offset = 0;
	
	return_if_untrue(port);
	return_if_untrue(bar <= 2);
	return_if_untrue(buf);
	return_if_untrue(chunks > 0);

	offset = port_offset(port, bar, register_offset);
		
	fscc_card_get_register_rep(port->card, bar, offset, buf, chunks);
}

void fscc_port_set_register(struct fscc_port *port, unsigned bar, 
                            unsigned register_offset, __u32 value)
{
	unsigned offset = 0;
	
	return_if_untrue(port);
	return_if_untrue(bar <= 2);	

	offset = port_offset(port, bar, register_offset);
		
	fscc_card_set_register(port->card, bar, offset, value);
}

void fscc_port_set_register_rep(struct fscc_port *port, unsigned bar,
                                unsigned register_offset, const char *data,
                                unsigned long chunks) 
{
	unsigned offset = 0;
	
	return_if_untrue(port);
	return_if_untrue(bar <= 2);
	return_if_untrue(data);
	return_if_untrue(chunks > 0);

	offset = port_offset(port, bar, register_offset);
		
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
			dev_dbg(port->device, 
			        "register 0x%02x restoring 0x%08x => 0x%08x\n",
			        i * 4, current_value, 
			        offset_to_value(&port->register_storage, i * 4));
			       
			fscc_port_set_register(port, 0, i * 4, 
			                  offset_to_value(&port->register_storage, i * 4));
		}
	}
	
}

void fscc_port_flush_tx(struct fscc_port *port)
{	
	return_if_untrue(port);	
	
	dev_dbg(port->device, "flush_tx\n");
	
	fscc_port_execute_TRES(port);
		
	if (port->pending_oframe)
		fscc_frame_delete(port->pending_oframe);
	
	fscc_port_empty_frames(port, &port->oframes);
}

void fscc_port_flush_rx(struct fscc_port *port)
{
	return_if_untrue(port);	
	
	dev_dbg(port->device, "flush_rx\n");
	
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
	return_val_if_untrue(frames, 0);		
	
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

#define STRB_BASE 0x00000008
#define DTA_BASE 0x00000001
#define CLK_BASE 0x00000002

void fscc_port_set_clock_bits(struct fscc_port *port, const unsigned char *clock_data)
{
	__u32 orig_fcr_value = 0;
	__u32 new_fcr_value = 0;
	unsigned j = 0;
	int i = 0; // Must be signed because we are going backwards through the array
	unsigned strb_value = STRB_BASE;
	unsigned dta_value = DTA_BASE;
	unsigned clk_value = CLK_BASE;
	
	return_if_untrue(port);
	
	if (port->channel == 1) {
		strb_value += 0x08;
		dta_value += 0x08;
		clk_value += 0x08;
	}

	orig_fcr_value = fscc_card_get_register(port->card, 2, FCR_OFFSET);

	new_fcr_value = orig_fcr_value & 0xfffff0f0;
	
	fscc_card_set_register(port->card, 2, FCR_OFFSET, new_fcr_value);

	for (i = 19; i >= 0; i--) {
	//for (i = 0; i < 20; i++) {

		printk("byte value = 0x%02x\n", clock_data[i]);
		
		for (j = 0; j < 8; j++) {
			int bit = ((clock_data[i] >> j) & 1);
			
			if (bit) {
				new_fcr_value |= dta_value; // Set data bit
				new_fcr_value |= clk_value; // Set clock bit
			}
			else {
				new_fcr_value &= ~dta_value; // Clear clock bit
				new_fcr_value |= clk_value; // Set clock bit
			}
			
			fscc_card_set_register(port->card, 2, FCR_OFFSET, new_fcr_value);
			//printk("clk high = 0x%08x\n", new_fcr_value);
			
			new_fcr_value &= ~clk_value; // Clear clock bit
			fscc_card_set_register(port->card, 2, FCR_OFFSET, new_fcr_value);
			//printk("clk low = 0x%08x\n", new_fcr_value);
		}
	}
	
	new_fcr_value = orig_fcr_value & 0xfffff0f0;

	new_fcr_value |= strb_value; // Set strobe bit
	new_fcr_value |= clk_value; // Set clock bit		
	fscc_card_set_register(port->card, 2, FCR_OFFSET, new_fcr_value); // Signal end of clock bits	
				
	new_fcr_value &= ~clk_value; // Clear clock bit		
	fscc_card_set_register(port->card, 2, FCR_OFFSET, new_fcr_value); // Signal end of clock bits
					
	fscc_card_set_register(port->card, 2, FCR_OFFSET, orig_fcr_value); // Restore old values
}

void fscc_port_use_async(struct fscc_port *port)
{
	__u32 orig_fcr_value = 0;
	__u32 new_fcr_value = 0;
	
	return_if_untrue(port);
	
	orig_fcr_value = fscc_card_get_register(port->card, 2, FCR_OFFSET);
	
	switch (port->channel) {
	case 0:
		new_fcr_value = orig_fcr_value | 0x01000000;
		break;
		
	case 1:
		new_fcr_value = orig_fcr_value | 0x02000000;
		break;
	}
	
	if (orig_fcr_value == new_fcr_value)
		return;
	
	fscc_card_set_register(port->card, 2, FCR_OFFSET, new_fcr_value);
}

void fscc_port_use_sync(struct fscc_port *port)
{
	__u32 orig_fcr_value = 0;
	__u32 new_fcr_value = 0;
	
	return_if_untrue(port);
	
	orig_fcr_value = fscc_card_get_register(port->card, 2, FCR_OFFSET);
	
	switch (port->channel) {
	case 0:
		new_fcr_value = orig_fcr_value & ~0x01000000;
		break;
		
	case 1:
		new_fcr_value = orig_fcr_value & ~0x02000000;
		break;
	}
	
	if (orig_fcr_value == new_fcr_value)
		return;
	
	fscc_card_set_register(port->card, 2, FCR_OFFSET, new_fcr_value);
}

void fscc_port_enable_append_status(struct fscc_port *port)
{	
	return_if_untrue(port);
	
	port->append_status = 1;
}

void fscc_port_disable_append_status(struct fscc_port *port)
{	
	return_if_untrue(port);
	
	port->append_status = 0;
}

void fscc_port_set_registers(struct fscc_port *port, 
                             const struct fscc_registers *regs)
{
	unsigned i = 0;
	
	return_if_untrue(port);	
	return_if_untrue(regs);
			
	for (i = 0; i < sizeof(*regs) / sizeof(regs->FIFOT); i++) {
		if (is_read_only_register(i * 4))
			continue;
			
		if (((int32_t *)regs)[i] < 0)
			continue;
						
		fscc_port_set_register(port, 0, i * 4, ((uint32_t *)regs)[i]);
	}
}

void fscc_port_get_registers(struct fscc_port *port,
                             struct fscc_registers *regs)
{
	unsigned i = 0;
	
	return_if_untrue(port);	
	return_if_untrue(regs);
			
	for (i = 0; i < sizeof(struct fscc_registers) / sizeof(int32_t); i++) {
		if (((int32_t *)regs)[i] != FSCC_UPDATE_VALUE)
			continue;
						
		((uint32_t *)regs)[i] = fscc_port_get_register(port, 0, i * 4);
	}
}

void fscc_port_execute_GO_R(struct fscc_port *port)
{	
	return_if_untrue(port);	
	
	fscc_port_set_register(port, 2, DMACCR_OFFSET, 0x00000001);
}

void fscc_port_execute_GO_T(struct fscc_port *port)
{
	return_if_untrue(port);	
	
	fscc_port_set_register(port, 2, DMACCR_OFFSET, 0x00000002);
}

void fscc_port_execute_RST_R(struct fscc_port *port)
{
	return_if_untrue(port);	
	
	fscc_port_set_register(port, 2, DMACCR_OFFSET, 0x00000010);
}

void fscc_port_execute_RST_T(struct fscc_port *port)
{
	return_if_untrue(port);	
	
	fscc_port_set_register(port, 2, DMACCR_OFFSET, 0x00000020);
}

void fscc_port_execute_STOP_R(struct fscc_port *port)
{
	return_if_untrue(port);	
	
	fscc_port_set_register(port, 2, DMACCR_OFFSET, 0x00000100);
}

void fscc_port_execute_STOP_T(struct fscc_port *port)
{
	return_if_untrue(port);	
	
	fscc_port_set_register(port, 2, DMACCR_OFFSET, 0x00000200);
}

