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


void fscc_port_empty_iframes(struct fscc_port *port);
void fscc_port_empty_oframes(struct fscc_port *port);
unsigned fscc_port_get_iframe_qty(struct fscc_port *port);
unsigned fscc_port_get_oframe_qty(struct fscc_port *port);
struct fscc_frame *fscc_port_peek_front_frame(struct fscc_port *port, 
                                              struct list_head *frames);
struct fscc_frame *fscc_port_peek_back_frame(struct fscc_port *port, 
                                              struct list_head *frames);     

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
	else
		printk(KERN_NOTICE DEVICE_NAME " invalid str passed into str_to_offset");

	return -1;
}

static ssize_t register_show(struct kobject *kobj, struct kobj_attribute *attr,
                             char *buf)
{
	struct fscc_port *port = 0;
	int register_offset = 0;
	
	port = (struct fscc_port *)dev_get_drvdata((struct device *)kobj);

	register_offset = str_to_offset(attr->attr.name);
	
	if (register_offset >= 0) {
		printk(KERN_DEBUG DEVICE_NAME " reading register 0x%02x\n", register_offset);
		return sprintf(buf, "0x%08x\n", fscc_port_get_register(port, 0, (unsigned)register_offset));
	}

	return 0;
}

static ssize_t register_store(struct kobject *kobj, struct kobj_attribute *attr,
                              const char *buf, size_t count)
{
	struct fscc_port *port = 0;
	int register_offset = 0;
	unsigned value = 0;
    char *end = 0;
	
	port = (struct fscc_port *)dev_get_drvdata((struct device *)kobj);

	register_offset = str_to_offset(attr->attr.name);
	value = (unsigned)simple_strtoul(buf, &end, 16);

	if (register_offset >= 0) {
		printk(KERN_DEBUG DEVICE_NAME " setting register 0x%02x to 0x%08x\n", register_offset, value);
		fscc_port_set_register(port, 0, register_offset, value);
		return count;
	}

	return 0;
}

static struct kobj_attribute fifot_attribute = 
	__ATTR(fifot, SYSFS_READ_WRITE_MODE, register_show, register_store);
	
static struct kobj_attribute cmdr_attribute = 
	__ATTR(cmdr, SYSFS_READ_WRITE_MODE, register_show, register_store);
	
static struct kobj_attribute ccr0_attribute = 
	__ATTR(ccr0, SYSFS_READ_WRITE_MODE, register_show, register_store);

static struct kobj_attribute ccr1_attribute = 
	__ATTR(ccr1, SYSFS_READ_WRITE_MODE, register_show, register_store);

static struct kobj_attribute ccr2_attribute = 
	__ATTR(ccr2, SYSFS_READ_WRITE_MODE, register_show, register_store);

static struct kobj_attribute bgr_attribute = 
	__ATTR(bgr, SYSFS_READ_WRITE_MODE, register_show, register_store);

static struct kobj_attribute ssr_attribute = 
	__ATTR(ssr, SYSFS_READ_WRITE_MODE, register_show, register_store);

static struct kobj_attribute smr_attribute = 
	__ATTR(smr, SYSFS_READ_WRITE_MODE, register_show, register_store);

static struct kobj_attribute tsr_attribute = 
	__ATTR(tsr, SYSFS_READ_WRITE_MODE, register_show, register_store);

static struct kobj_attribute tmr_attribute = 
	__ATTR(tmr, SYSFS_READ_WRITE_MODE, register_show, register_store);

static struct kobj_attribute rar_attribute = 
	__ATTR(rar, SYSFS_READ_WRITE_MODE, register_show, register_store);

static struct kobj_attribute ramr_attribute = 
	__ATTR(ramr, SYSFS_READ_WRITE_MODE, register_show, register_store);

static struct kobj_attribute ppr_attribute = 
	__ATTR(ppr, SYSFS_READ_WRITE_MODE, register_show, register_store);

static struct kobj_attribute tcr_attribute = 
	__ATTR(tcr, SYSFS_READ_WRITE_MODE, register_show, register_store);

static struct kobj_attribute vstr_attribute = 
	__ATTR(vstr, SYSFS_READ_ONLY_MODE, register_show, register_store);

static struct kobj_attribute imr_attribute = 
	__ATTR(imr, SYSFS_READ_WRITE_MODE, register_show, register_store);

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
	NULL,
};

static struct attribute_group attr_group = {
	.name = "registers",
	.attrs = attrs,
};
	
struct fscc_port *fscc_port_new(struct fscc_card *card, unsigned channel, 
                                unsigned major_number, unsigned minor_number, 
                                struct class *class, 
                                struct file_operations *fops)
{
	struct fscc_port *new_port = 0;
	unsigned irq_num = 0;
	
	new_port = (struct fscc_port*)kmalloc(sizeof(struct fscc_port), GFP_KERNEL);
	
	INIT_LIST_HEAD(&new_port->list);
	INIT_LIST_HEAD(&new_port->oframes);
	INIT_LIST_HEAD(&new_port->iframes);
	
	new_port->channel = channel;
	new_port->dev_t = MKDEV(major_number, minor_number);
	new_port->class = class;
	new_port->card = card;
	
	init_MUTEX(&new_port->semaphore);
	init_waitqueue_head(&new_port->input_queue);
	init_waitqueue_head(&new_port->output_queue);
	
	cdev_init(&new_port->cdev, fops);
	new_port->cdev.owner = THIS_MODULE;
	
	if (cdev_add(&new_port->cdev, new_port->dev_t, 1) < 0) {
		printk(KERN_ERR DEVICE_NAME " cdev_add failed\n");
		return 0;
	}
	
	list_add_tail(&new_port->list, &card->ports);
	
	new_port->name = (char *)kmalloc(10, GFP_KERNEL);
	sprintf(new_port->name, "%s%u", DEVICE_NAME, minor_number);
	
	new_port->device = device_create(class, 0, new_port->dev_t, new_port, new_port->name);
	if (new_port->device <= 0) {
		printk(KERN_ERR DEVICE_NAME " device_create failed\n");
		return 0;
	}
	
	irq_num = card->pci_dev->irq;
	if (request_irq(irq_num, &fscc_isr, IRQF_SHARED, new_port->name, new_port)) {
		printk(KERN_ERR DEVICE_NAME " request_irq failed on irq %i\n", irq_num);
		return 0;
	}
	
	if (sysfs_create_group(&new_port->device->kobj, &attr_group)) {
		printk(KERN_ERR DEVICE_NAME " sysfs_create_group\n");
		return 0;
	}
	
	fscc_port_set_register(new_port, 0, IMR_OFFSET, 0xf0f802C0);
	fscc_port_set_register(new_port, 0, CCR0_OFFSET, 0x0000001c);
	fscc_port_set_register(new_port, 0, CCR1_OFFSET, 0x00000008);
	fscc_port_set_register(new_port, 0, FIFOT_OFFSET, 0x08001000);
	
	fscc_port_execute_RRES(new_port);
	fscc_port_execute_TRES(new_port);
	
	return new_port;
}

void fscc_port_delete(struct fscc_port *port)
{
	unsigned irq_num = 0;
	
	if (port == 0)
		return;
	
	fscc_port_empty_iframes(port);
	fscc_port_empty_oframes(port);
	
	irq_num = port->card->pci_dev->irq;	
	free_irq(irq_num, port);
	kfree(port->name);
	
	device_destroy(port->class, port->dev_t);
	cdev_del(&port->cdev);
	list_del(&port->list);
	kfree(port);
}

unsigned fscc_port_exists(struct fscc_port *port, struct list_head *card_list)
{
	struct fscc_card *current_card = 0;
	struct fscc_port *current_port = 0;
	
	list_for_each_entry(current_card, card_list, list) {		
		list_for_each_entry(current_port, &current_card->ports, list) {	
			if (current_port == port) {
				return 1;
			}
		}
	}
	
	return 0;
}

unsigned fscc_port_get_major_number(struct fscc_port *port)
{
	return MAJOR(port->dev_t);
}

unsigned fscc_port_get_minor_number(struct fscc_port *port)
{
	return MINOR(port->dev_t);
}

struct fscc_card *fscc_port_get_card(struct fscc_port *port)
{
	return port->card;
}

#define STATUS_LENGTH 2

void fscc_port_empty_RxFIFO(struct fscc_port *port)
{
	struct fscc_frame *frame = 0;
	unsigned byte_count = 0;
	unsigned leftover_count = 0;
	unsigned i = 0;
	__u32 incoming_data;
	__u32 frame_status = 0;
	
	byte_count = fscc_port_get_register(port, 0, BC_FIFO_L_OFFSET) - STATUS_LENGTH;
	
	/* Grabs the last frame if available or creates a new one. */
	if (!fscc_port_has_iframes(port)) {
		frame = fscc_port_push_iframe(port, byte_count);
	}
	else {
		frame = fscc_port_peek_back_iframe(port);
	}
	
	/* If the frame just grabbed is full grab a new one. */	
	if (fscc_frame_is_full(frame)) {
		frame = fscc_port_push_iframe(port, byte_count);
	}
	
	/* Read all of the 4 byte chunks. */
	for (i = 0; i < byte_count - byte_count % 4; i += 4) {
		incoming_data = fscc_port_get_register(port, 0, FIFO_OFFSET);
		fscc_frame_add_data(frame, (char *)(&incoming_data), 4);
	}
	
	leftover_count = byte_count % 4;
	
	if (leftover_count) {
		incoming_data = fscc_port_get_register(port, 0, FIFO_OFFSET);
		fscc_frame_add_data(frame, (char *)(&incoming_data), leftover_count);
	}
	
	// This reuses 'incoming_data' as the last data chunk
	// All of the funky shifts are to ignore mystery extra data in the 32 bit reads
	// Then to align the status data in the correct spot within the 32 bits
	switch (leftover_count) {
		case 0:
			frame_status = ((fscc_port_get_register(port, 0, FIFO_OFFSET) << 16) >> 16);
			break;
			
		case 1:
			frame_status = ((incoming_data << 8) >> 16);
			break;
			
		case 2:
			frame_status = incoming_data >> 16;
			break;
			
		case 3:
			frame_status = (incoming_data >> 24) + ((fscc_port_get_register(port, 0, FIFO_OFFSET) << 24) >> 16);
			break;
	}
}

/* Must have room available */
void fscc_port_fill_TxFIFO(struct fscc_port *port, const char *data, 
                           unsigned byte_count)
{
	unsigned i = 0;
	unsigned leftover_count = 0;
	
	/* Writes all of the 4 byte chunks */
	for (i = 0; i + 4 <= byte_count; i += 4)
		fscc_port_set_register(port, 0, FIFO_OFFSET, chars_to_u32(data + i));
	
	leftover_count = byte_count % 4;
	
	/* Writes the leftover bytes (non 4 byte chunk) */
	if (leftover_count)
		fscc_port_set_register(port, 0, FIFO_OFFSET, chars_to_u32(data + i));
}

// Returns ENOMEM if write size will go over user cap.
/* Length is user data length. Without 32bit padding. */
int fscc_port_write(struct fscc_port *port, const char *data, unsigned length)
{
	unsigned unused_fifo_space = 0;
	unsigned padding_bytes = 0;
	struct fscc_frame *frame = 0;
	char *temp_storage = 0;
	
	if (fscc_port_total_oframe_memory(port) + length > memory_cap)
		return ENOMEM;
	
	unused_fifo_space = fscc_port_get_available_tx_bytes(port);
	padding_bytes = length % 4 ? 4 - length % 4 : 0;
	
	temp_storage = (char *)kmalloc(length, GFP_KERNEL);
	copy_from_user(temp_storage, data, length);
	
	frame = fscc_port_push_oframe(port, length);
	fscc_frame_add_data(frame, temp_storage, length);
	
	kfree(temp_storage);
		
	if (length + padding_bytes > unused_fifo_space) {
		fscc_port_fill_TxFIFO(port, frame->data, unused_fifo_space);
		fscc_frame_remove_data(frame, unused_fifo_space);
	}
	else {
		fscc_port_fill_TxFIFO(port, frame->data, length);
		fscc_frame_remove_data(frame, length);	
		fscc_port_pop_oframe(port);
	}
	
	fscc_port_set_register(port, 0, BC_FIFO_L_OFFSET, length);
	
	printk(KERN_DEBUG DEVICE_NAME " sending %u byte frame\n", length);
	
	fscc_port_execute_XF(port);
	
	return 1;
}


//Returns ENOBUFS if count is smaller than pending frame size
ssize_t fscc_port_read(struct fscc_port *port, char *buf, size_t count)
{
	struct fscc_frame *frame = 0;
	unsigned sent_length = 0;
	
	frame = fscc_port_peek_front_iframe(port);
	
	if (frame && fscc_frame_is_full(frame)) {
		if (count < fscc_frame_get_target_length(frame))
			return ENOBUFS;
			
		copy_to_user(buf, fscc_frame_get_data(frame), 
		             fscc_frame_get_target_length(frame));
		sent_length = fscc_frame_get_target_length(frame);
		fscc_port_pop_iframe(port);
	}
	
	return sent_length;
}
                     
unsigned fscc_port_get_available_tx_bytes(struct fscc_port *port)
{
	int byte_count = TX_FIFO_SIZE - fscc_port_get_TXCNT(port);
	return byte_count >= 0 ? byte_count : 0;
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

void fscc_port_empty_iframes(struct fscc_port *port)
{
	fscc_port_empty_frames(port, &port->iframes);
}

void fscc_port_empty_oframes(struct fscc_port *port)
{
	fscc_port_empty_frames(port, &port->oframes);
}

/* NOTE: data needs to be a user buffer because I run copy_from_user */
struct fscc_frame *fscc_port_push_frame(struct fscc_port *port, struct list_head *frames,
                                        unsigned length)
{
	struct fscc_frame *frame = fscc_frame_new(length);	
	list_add_tail(&frame->list, frames);	
	return frame;
}

struct fscc_frame *fscc_port_push_iframe(struct fscc_port *port, unsigned length)
{
	return fscc_port_push_frame(port, &port->iframes, length);
}

struct fscc_frame *fscc_port_push_oframe(struct fscc_port *port, unsigned length)
{
	return fscc_port_push_frame(port, &port->oframes, length);
}

void fscc_port_pop_frame(struct fscc_port *port, struct list_head *frames)
{
	struct fscc_frame *frame = fscc_port_peek_front_frame(port, frames);
	list_del(&frame->list);
	fscc_frame_delete(frame);
}

void fscc_port_pop_iframe(struct fscc_port *port)
{
	fscc_port_pop_frame(port, &port->iframes);
}

void fscc_port_pop_oframe(struct fscc_port *port)
{
	fscc_port_pop_frame(port, &port->oframes);
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

struct fscc_frame *fscc_port_peek_front_iframe(struct fscc_port *port)
{	
	return fscc_port_peek_front_frame(port, &port->iframes);
}

struct fscc_frame *fscc_port_peek_front_oframe(struct fscc_port *port)
{	
	return fscc_port_peek_front_frame(port, &port->oframes);
}

struct fscc_frame *fscc_port_peek_back_frame(struct fscc_port *port, 
                                             struct list_head *frames)
{
	struct fscc_frame *current_frame = 0;	
	struct fscc_frame *last_frame = 0;
	
	list_for_each_entry(current_frame, frames, list) {
		last_frame = current_frame;
	}	
	
	return last_frame;
}

struct fscc_frame *fscc_port_peek_back_iframe(struct fscc_port *port)
{	
	return fscc_port_peek_back_frame(port, &port->iframes);
}

struct fscc_frame *fscc_port_peek_back_oframe(struct fscc_port *port)
{	
	return fscc_port_peek_back_frame(port, &port->oframes);
}

bool fscc_port_has_iframes(struct fscc_port *port)
{
	return !list_empty(&port->iframes);
}

bool fscc_port_has_oframes(struct fscc_port *port)
{
	return !list_empty(&port->oframes);
}

unsigned fscc_port_get_frame_qty(struct fscc_port *port, struct list_head *frames)
{
	struct list_head *temp_node = 0;
	unsigned frame_quantity = 0;
	
	if (port == 0)
		return 0;
		
	list_for_each(temp_node, frames) {
		++frame_quantity;
	}
	
	return frame_quantity;
}

unsigned fscc_port_get_iframe_qty(struct fscc_port *port)
{
	return fscc_port_get_frame_qty(port, &port->iframes);
}

unsigned fscc_port_get_oframe_qty(struct fscc_port *port)
{
	return fscc_port_get_frame_qty(port, &port->oframes);
}

bool fscc_port_iframes_ready(struct fscc_port *port)
{
	struct fscc_frame *frame = fscc_port_peek_front_iframe(port);	
	return (frame && fscc_frame_is_full(frame));
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

unsigned fscc_port_total_oframe_memory(struct fscc_port *port)
{
	return fscc_port_total_frame_memory(port, &port->oframes);
}

unsigned fscc_port_total_iframe_memory(struct fscc_port *port)
{
	return fscc_port_total_frame_memory(port, &port->iframes);
}

/******************************************************************************/

__u32 fscc_port_get_register(struct fscc_port *port, unsigned bar, 
                             unsigned register_offset)
{
	unsigned long address = 0;
	unsigned long offset = 0;
	
	address = fscc_card_get_BAR(port->card, bar);
	offset = register_offset;
	
	if (port->channel == 1)
		offset += 0x80;
		
	return inl(address + offset);
}

void fscc_port_set_register(struct fscc_port *port, unsigned bar, 
                            unsigned register_offset, __u32 value)
{
	unsigned long address = 0;
	unsigned long offset = 0;
	
	address = fscc_card_get_BAR(port->card, bar);
	offset = register_offset;
	
	if (port->channel == 1)
		offset += 0x80;
		
	outl(value, address + offset);
}

__u32 fscc_port_get_TXCNT(struct fscc_port *port)
{
	__u32 fifo_bc_value = 0;	
	fifo_bc_value = fscc_port_get_register(port, 0, FIFO_BC_OFFSET);
	
	return (fifo_bc_value & 0x1FFF0000) >> 16;
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

