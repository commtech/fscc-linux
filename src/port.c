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

struct fscc_frame *fscc_port_peek_front_frame(struct fscc_port *port, 
                                              struct list_head *frames);
                                              
void fscc_port_empty_frames(struct fscc_port *port, struct list_head *frames);
unsigned fscc_port_total_frame_memory(struct fscc_port *port, 
                                      struct list_head *frames);

void fscc_port_fill_TxFIFO(struct fscc_port *port, const char *data, 
                           unsigned byte_count);
                           
__u32 fscc_port_empty_RxFIFO(struct fscc_port *port, struct fscc_frame *frame, 
                           unsigned byte_count);

int str_to_offset(const char *str)
{
	if (strcmp(str, "fifo") == 0)
		return FIFO_OFFSET;
	else if (strcmp(str, "bcfl") == 0)
		return BC_FIFO_L_OFFSET;
	else if (strcmp(str, "fifot") == 0)
		return FIFOT_OFFSET;
	else if (strcmp(str, "fifobc") == 0)
		return FIFO_BC_OFFSET;
	else if (strcmp(str, "fifofc") == 0)
		return FIFO_FC_OFFSET;
	else if (strcmp(str, "cmdr") == 0)
		return CMDR_OFFSET;
	else if (strcmp(str, "star") == 0)
		return STAR_OFFSET;
	else if (strcmp(str, "ccr0") == 0)
		return CCR0_OFFSET;
	else if (strcmp(str, "ccr1") == 0)
		return CCR1_OFFSET;
	else if (strcmp(str, "ccr2") == 0)
		return CCR2_OFFSET;
	else if (strcmp(str, "bgr") == 0)
		return BGR_OFFSET;
	else if (strcmp(str, "ssr") == 0)
		return SSR_OFFSET;
	else if (strcmp(str, "smr") == 0)
		return SMR_OFFSET;
	else if (strcmp(str, "tsr") == 0)
		return TSR_OFFSET;
	else if (strcmp(str, "tmr") == 0)
		return TMR_OFFSET;
	else if (strcmp(str, "rar") == 0)
		return RAR_OFFSET;
	else if (strcmp(str, "ramr") == 0)
		return RAMR_OFFSET;
	else if (strcmp(str, "ppr") == 0)
		return PPR_OFFSET;
	else if (strcmp(str, "tcr") == 0)
		return TCR_OFFSET;
	else if (strcmp(str, "vstr") == 0)
		return VSTR_OFFSET;
	else if (strcmp(str, "isr") == 0)
		return ISR_OFFSET;
	else if (strcmp(str, "imr") == 0)
		return IMR_OFFSET;
	else if (strcmp(str, "fcr") == 0)
		return FCR_OFFSET;
	else
		printk(KERN_NOTICE DEVICE_NAME " invalid str passed into str_to_offset\n");

	return -1;
}

static ssize_t register_store(struct kobject *kobj, struct kobj_attribute *attr,
                              const char *buf, size_t count, unsigned bar_number)
{
	struct fscc_port *port = 0;
	int register_offset = 0;
	unsigned value = 0;
    char *end = 0;
	
	port = (struct fscc_port *)dev_get_drvdata((struct device *)kobj);

	register_offset = str_to_offset(attr->attr.name);
	value = (unsigned)simple_strtoul(buf, &end, 16);

	if (register_offset >= 0) {
		printk(KERN_DEBUG "%s setting register 0x%02x to 0x%08x\n", port->name, register_offset, value);
		fscc_port_set_register(port, bar_number, register_offset, value);
		return count;
	}

	return 0;
}

static ssize_t register_show(struct kobject *kobj, struct kobj_attribute *attr,
                             char *buf, unsigned bar_number)

{
	struct fscc_port *port = 0;
	int register_offset = 0;
	
	port = (struct fscc_port *)dev_get_drvdata((struct device *)kobj);

	register_offset = str_to_offset(attr->attr.name);
	
	if (register_offset >= 0) {
		printk(KERN_DEBUG "%s reading register 0x%02x\n", port->name, register_offset);
		return sprintf(buf, "%08x\n", fscc_port_get_register(port, bar_number, (unsigned)register_offset));
	}

	return 0;
}

static ssize_t bar0_register_store(struct kobject *kobj, struct kobj_attribute *attr,
                                   const char *buf, size_t count)
{
	return register_store(kobj, attr, buf, count, 0);
}

static ssize_t bar0_register_show(struct kobject *kobj, struct kobj_attribute *attr,
                                  char *buf)
{
	return register_show(kobj, attr, buf, 0);
}

static ssize_t bar2_register_store(struct kobject *kobj, struct kobj_attribute *attr,
                                   const char *buf, size_t count)
{
	return register_store(kobj, attr, buf, count, 2);
}

static ssize_t bar2_register_show(struct kobject *kobj, struct kobj_attribute *attr,
                                  char *buf)
{
	return register_show(kobj, attr, buf, 2);
}

static struct kobj_attribute fifot_attribute = 
	__ATTR(fifot, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);
	
static struct kobj_attribute cmdr_attribute = 
	__ATTR(cmdr, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);
	
static struct kobj_attribute ccr0_attribute = 
	__ATTR(ccr0, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute ccr1_attribute = 
	__ATTR(ccr1, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute ccr2_attribute = 
	__ATTR(ccr2, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute bgr_attribute = 
	__ATTR(bgr, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute ssr_attribute = 
	__ATTR(ssr, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute smr_attribute = 
	__ATTR(smr, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute tsr_attribute = 
	__ATTR(tsr, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute tmr_attribute = 
	__ATTR(tmr, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute rar_attribute = 
	__ATTR(rar, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute ramr_attribute = 
	__ATTR(ramr, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute ppr_attribute = 
	__ATTR(ppr, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute tcr_attribute = 
	__ATTR(tcr, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute vstr_attribute = 
	__ATTR(vstr, SYSFS_READ_ONLY_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute imr_attribute = 
	__ATTR(imr, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute fcr_attribute = 
	__ATTR(fcr, SYSFS_READ_WRITE_MODE, bar2_register_show, bar2_register_store);

static struct attribute *attrs[] = {
	&fifot_attribute.attr,
	&cmdr_attribute.attr,
	&ccr0_attribute.attr,
	&ccr1_attribute.attr,
	&ccr2_attribute.attr,
	&bgr_attribute.attr,
	&ssr_attribute.attr,
	&smr_attribute.attr,
	&tsr_attribute.attr,
	&tmr_attribute.attr,
	&rar_attribute.attr,
	&ramr_attribute.attr,
	&ppr_attribute.attr,
	&tcr_attribute.attr,
	&vstr_attribute.attr,
	&imr_attribute.attr,
	&fcr_attribute.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.name = "registers",
	.attrs = attrs,
};

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

	port = (struct fscc_port *)data;
	
	//printk(KERN_DEBUG "%s %i %i %i\n", port->name, port->started_frames, port->ended_frames, port->handled_frames);
	
	if (port->handled_frames == port->started_frames)
		return;
		
	if (!port->pending_iframe) {
		//TODO: Using a large initial frame size until I get the resize code in
		port->pending_iframe = fscc_frame_new(12000);
		
		printk(KERN_DEBUG "%s F#%i receive started\n", port->name, 
			   port->pending_iframe->number);
	}	
		
	if (port->handled_frames < port->ended_frames) {
		finished_frame = 1;
		byte_count = fscc_port_get_register(port, 0, BC_FIFO_L_OFFSET) - 2;
		receive_length = byte_count - fscc_frame_get_current_length(port->pending_iframe);
	}
	else {
		finished_frame = 0;
		byte_count = receive_length = fscc_port_get_RXCNT(port);
	}	
		
	leftover_count = receive_length % 4;
	
	if (!finished_frame)
		receive_length -= leftover_count;
	
	last_data_chunk = fscc_port_empty_RxFIFO(port, port->pending_iframe, receive_length);
	
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
	
	//printk(KERN_DEBUG "%s leftover = %i, status = 0x%08x\n", port->name, leftover_count, frame_status);
	
	if (frame_status & 0x00000004) {
		fscc_frame_trim(port->pending_iframe);		
		list_add_tail(&port->pending_iframe->list, &port->iframes);
		
		printk(KERN_DEBUG "%s F#%i receive successful\n", port->name, 
			   port->pending_iframe->number);
			   
		wake_up_interruptible(&port->input_queue);
	}
	else {		
		fscc_frame_delete(port->pending_iframe);
		
		printk(KERN_ALERT "%s F#%i receive rejected\n", port->name, 
			   port->pending_iframe->number);
	}		

	port->handled_frames += 1;
	
	//printk(KERN_DEBUG "%s B\n", port->name);
	       
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
	
	if (sysfs_create_group(&port->device->kobj, &attr_group)) {
		printk(KERN_ERR "%s sysfs_create_group\n", port->name);
		return 0;
	}
	
	return port;
}

void fscc_port_delete(struct fscc_port *port)
{
	unsigned irq_num = 0;
	
	if (port == 0)
		return;
		
	if (port->pending_iframe)
		fscc_frame_delete(port->pending_iframe);
		
	if (port->pending_oframe)
		fscc_frame_delete(port->pending_oframe);
	
	fscc_port_empty_frames(port, &port->iframes);
	fscc_port_empty_frames(port, &port->oframes);
	
	irq_num = port->card->pci_dev->irq;	
	free_irq(irq_num, port);
	kfree(port->name);
	
	device_destroy(port->class, port->dev_t);
	cdev_del(&port->cdev);
	list_del(&port->list);
	kfree(port);
}

// Returns -ENOMEM if write size will go over user cap.
/* Length is user data length. Without 32bit padding. */
int fscc_port_write(struct fscc_port *port, const char *data, unsigned length)
{
	struct fscc_frame *frame = 0;
	char *temp_storage = 0;
	
	if (fscc_port_total_frame_memory(port, &port->oframes) + length > memory_cap)
		return -ENOMEM;
	
	temp_storage = (char *)kmalloc(length, GFP_KERNEL);
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

unsigned fscc_port_get_minor_number(struct fscc_port *port)
{
	return MINOR(port->dev_t);
}

/* Must have room available */
/* Puts data into the FIFO */
void fscc_port_fill_TxFIFO(struct fscc_port *port, const char *data, 
                           unsigned byte_count)
{
	unsigned leftover_count = 0;
	
	leftover_count = byte_count % 4;
	
	fscc_port_set_register_rep(port, 0, FIFO_OFFSET, data, 
	                           (byte_count - leftover_count) / 4);
	
	/* Writes the leftover bytes (non 4 byte chunk) */
	if (leftover_count)
		fscc_port_set_register(port, 0, FIFO_OFFSET, chars_to_u32(data + (byte_count - leftover_count)));
}

/* Pulls data from FIFO and puts into the frame */
/* Returns the last data chunk retrieved */
__u32 fscc_port_empty_RxFIFO(struct fscc_port *port, struct fscc_frame *frame,
                            unsigned byte_count)
{
	unsigned leftover_count = 0;
	unsigned i = 0;
	__u32 incoming_data = 0;
	
	leftover_count = byte_count % 4;
	//printk("%i %i %i\n", leftover_count, byte_count, frame->current_length);
	
	//I think the problem with this is when I use the add_Data method like normally, it does something behind the scenes.
	//fscc_port_get_register_rep(port, 0, FIFO_OFFSET, 
	//                           frame->data + frame->current_length, 
	//                           (byte_count - leftover_count) / 4);
	
	/* Read all of the 4 byte chunks. */
	for (i = 0; i < byte_count - leftover_count; i += 4) {
		incoming_data = fscc_port_get_register(port, 0, FIFO_OFFSET);
		fscc_frame_add_data(frame, (char *)(&incoming_data), 4);
	}
	
	if (leftover_count) {
		incoming_data = fscc_port_get_register(port, 0, FIFO_OFFSET);
		fscc_frame_add_data(frame, (char *)(&incoming_data), leftover_count);
	}
	
	return incoming_data;
}

void fscc_port_empty_frames(struct fscc_port *port, struct list_head *frames)
{	
	struct list_head *current_node = 0;
	struct list_head *temp_node = 0;
	
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
	
	list_for_each_entry(current_frame, frames, list) {
		return current_frame;
	}
	
	return 0;
}

bool fscc_port_has_iframes(struct fscc_port *port)
{
	return !list_empty(&port->iframes);
}

bool fscc_port_has_oframes(struct fscc_port *port)
{
	return !list_empty(&port->oframes);
}

unsigned fscc_port_total_frame_memory(struct fscc_port *port, struct list_head *frames)
{
	struct fscc_frame *current_frame = 0;	
	unsigned memory = 0;
	
	list_for_each_entry(current_frame, frames, list) {
		memory += fscc_frame_get_target_length(current_frame);
	}	
	
	return memory;
}

__u32 fscc_port_get_register(struct fscc_port *port, unsigned bar, 
                             unsigned register_offset)
{
	void __iomem *address = 0;
	unsigned offset = 0;
	
	address = fscc_card_get_BAR(port->card, bar);
	offset = register_offset;
	
	if (port->channel == 1)
		offset += 0x80;
		
	return ioread32(address + offset);
}

void fscc_port_get_register_rep(struct fscc_port *port, unsigned bar, 
                                 unsigned register_offset, char *buf,
                                 unsigned long chunks)
{
	void __iomem *address = 0;
	unsigned offset = 0;
	
	address = fscc_card_get_BAR(port->card, bar);
	offset = register_offset;
	
	if (port->channel == 1)
		offset += 0x80;
		
	ioread32_rep(address + offset, buf, chunks);
}

void fscc_port_set_register(struct fscc_port *port, unsigned bar, 
                            unsigned register_offset, __u32 value)
{
	void __iomem *address = 0;
	unsigned offset = 0;
	
	address = fscc_card_get_BAR(port->card, bar);
	offset = register_offset;
	
	if (port->channel == 1)
		offset += 0x80;
		
	iowrite32(value, address + offset);
}

void fscc_port_set_register_rep(struct fscc_port *port, unsigned bar,
                                unsigned register_offset, const char *data,
                                unsigned long chunks) 
{
	void __iomem *address = 0;
	unsigned offset = 0;
	
	address = fscc_card_get_BAR(port->card, bar);
	offset = register_offset;
	
	if (port->channel == 1)
		offset += 0x80;
		
	iowrite32_rep(address + offset, data, chunks);
}

__u32 fscc_port_get_TXCNT(struct fscc_port *port)
{
	__u32 fifo_bc_value = 0;	
	fifo_bc_value = fscc_port_get_register(port, 0, FIFO_BC_OFFSET);
	
	return (fifo_bc_value & 0x1FFF0000) >> 16;
}

__u32 fscc_port_get_RXCNT(struct fscc_port *port)
{
	__u32 fifo_bc_value = 0;	
	fifo_bc_value = fscc_port_get_register(port, 0, FIFO_BC_OFFSET);
	
	return fifo_bc_value & 0x00003FFF;
}

__u32 fscc_port_get_RXTRG(struct fscc_port *port)
{
	__u32 fifot_value = 0;	
	fifot_value = fscc_port_get_register(port, 0, FIFOT_OFFSET);
	
	return fifot_value & 0x00001FFF;
}

__u8 fscc_port_get_FREV(struct fscc_port *port)
{
	__u32 vstr_value = 0;		
	vstr_value = fscc_port_get_register(port, 0, VSTR_OFFSET);
	
	return (__u8)((vstr_value & 0x000000FF));
}

__u8 fscc_port_get_PREV(struct fscc_port *port)
{
	__u32 vstr_value = 0;		
	vstr_value = fscc_port_get_register(port, 0, VSTR_OFFSET);
	
	return (__u8)((vstr_value & 0x0000FF00) >> 8);
}

void fscc_port_execute_TRES(struct fscc_port *port)
{
	fscc_port_set_register(port, 0, CMDR_OFFSET, 0x08000000);
}

void fscc_port_execute_RRES(struct fscc_port *port)
{
	fscc_port_set_register(port, 0, CMDR_OFFSET, 0x00020000);
}

void fscc_port_execute_XF(struct fscc_port *port)
{
	fscc_port_set_register(port, 0, CMDR_OFFSET, 0x01000000);
}

void fscc_port_store_registers(struct fscc_port *port)
{
	unsigned i = 0;
	
	for (i = 0; i < sizeof(struct fscc_registers) / sizeof(int32_t); i++)
		((uint32_t *)&port->register_storage)[i] = fscc_port_get_register(port, 0, i * 4);
}

void fscc_port_restore_registers(struct fscc_port *port)
{
	unsigned i = 0;
	
	for (i = 0; i < sizeof(struct fscc_registers) / sizeof(int32_t); i++)
		fscc_port_set_register(port, 0, i * 4, ((uint32_t *)&port->register_storage)[i]);
}

