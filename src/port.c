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
#include <linux/proc_fs.h>
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

int register_read_proc(char *page, void *data, unsigned register_offset)
{
    struct fscc_port *port = (struct fscc_port *)data;

	printk(KERN_DEBUG DEVICE_NAME " reading register 0x%02x", register_offset);
    
	return sprintf(page, "0x%08x\n", fscc_port_get_register(port, 0, register_offset));
}

int fifot_read_proc(char *page, char **start, off_t offset, int count, int *eof, void *data) 
{
    return register_read_proc(page, data, FIFOT_OFFSET);
}       

int cmdr_read_proc(char *page, char **start, off_t offset, int count, int *eof, void *data) 
{
    return register_read_proc(page, data, CMDR_OFFSET);
}   

int star_read_proc(char *page, char **start, off_t offset, int count, int *eof, void *data) 
{
    return register_read_proc(page, data, STAR_OFFSET);
}   

int ccr0_read_proc(char *page, char **start, off_t offset, int count, int *eof, void *data) 
{
    return register_read_proc(page, data, CCR0_OFFSET);
}   

int ccr1_read_proc(char *page, char **start, off_t offset, int count, int *eof, void *data) 
{
    return register_read_proc(page, data, CCR1_OFFSET);
}   

int ccr2_read_proc(char *page, char **start, off_t offset, int count, int *eof, void *data) 
{
    return register_read_proc(page, data, CCR2_OFFSET);
}   

int bgr_read_proc(char *page, char **start, off_t offset, int count, int *eof, void *data) 
{
    return register_read_proc(page, data, BGR_OFFSET);
}   

int ssr_read_proc(char *page, char **start, off_t offset, int count, int *eof, void *data) 
{
    return register_read_proc(page, data, SSR_OFFSET);
}   

int smr_read_proc(char *page, char **start, off_t offset, int count, int *eof, void *data) 
{
    return register_read_proc(page, data, SMR_OFFSET);
}   

int tsr_read_proc(char *page, char **start, off_t offset, int count, int *eof, void *data) 
{
    return register_read_proc(page, data, TSR_OFFSET);
}   

int tmr_read_proc(char *page, char **start, off_t offset, int count, int *eof, void *data) 
{
    return register_read_proc(page, data, TMR_OFFSET);
}   

int rar_read_proc(char *page, char **start, off_t offset, int count, int *eof, void *data) 
{
    return register_read_proc(page, data, RAR_OFFSET);
}   

int ramr_read_proc(char *page, char **start, off_t offset, int count, int *eof, void *data) 
{
    return register_read_proc(page, data, RAMR_OFFSET);
}   

int ppr_read_proc(char *page, char **start, off_t offset, int count, int *eof, void *data) 
{
    return register_read_proc(page, data, PPR_OFFSET);
}         

int tcr_read_proc(char *page, char **start, off_t offset, int count, int *eof, void *data) 
{
    return register_read_proc(page, data, TCR_OFFSET);
}        

int vstr_read_proc(char *page, char **start, off_t offset, int count, int *eof, void *data) 
{
    return register_read_proc(page, data, VSTR_OFFSET);
}        

int imr_read_proc(char *page, char **start, off_t offset, int count, int *eof, void *data) 
{
    return register_read_proc(page, data, IMR_OFFSET);
}   

////////////////////////////////////////

int register_write_proc(const char *buffer, unsigned long count, void *data, unsigned register_offset)
{
    struct fscc_port *port = 0;
    char *end = 0;
    unsigned value = 0;

    port = (struct fscc_port *)data;
	
	value = simple_strtoul(buffer, &end, 16);

	printk(KERN_DEBUG DEVICE_NAME " setting register 0x%02x to 0x%08x", register_offset, value);

	fscc_port_set_register(port, 0, register_offset, value);

	return count;
}     

int fifot_write_proc(struct file *file, const char *buffer, unsigned long count, void *data) 
{
    return register_write_proc(buffer, count, data, FIFOT_OFFSET);
}      

int cmdr_write_proc(struct file *file, const char *buffer, unsigned long count, void *data) 
{
    return register_write_proc(buffer, count, data, CMDR_OFFSET);
}      

int ccr0_write_proc(struct file *file, const char *buffer, unsigned long count, void *data) 
{
    return register_write_proc(buffer, count, data, CCR0_OFFSET);
}      

int ccr1_write_proc(struct file *file, const char *buffer, unsigned long count, void *data) 
{
    return register_write_proc(buffer, count, data, CCR1_OFFSET);
}      

int ccr2_write_proc(struct file *file, const char *buffer, unsigned long count, void *data) 
{
    return register_write_proc(buffer, count, data, CCR2_OFFSET);
}      

int bgr_write_proc(struct file *file, const char *buffer, unsigned long count, void *data) 
{
    return register_write_proc(buffer, count, data, BGR_OFFSET);
}      

int ssr_write_proc(struct file *file, const char *buffer, unsigned long count, void *data) 
{
    return register_write_proc(buffer, count, data, SSR_OFFSET);
}      

int smr_write_proc(struct file *file, const char *buffer, unsigned long count, void *data) 
{
    return register_write_proc(buffer, count, data, SMR_OFFSET);
}      

int tsr_write_proc(struct file *file, const char *buffer, unsigned long count, void *data) 
{
    return register_write_proc(buffer, count, data, TSR_OFFSET);
}      

int tmr_write_proc(struct file *file, const char *buffer, unsigned long count, void *data) 
{
    return register_write_proc(buffer, count, data, TMR_OFFSET);
}      

int rar_write_proc(struct file *file, const char *buffer, unsigned long count, void *data) 
{
    return register_write_proc(buffer, count, data, RAR_OFFSET);
}

int ramr_write_proc(struct file *file, const char *buffer, unsigned long count, void *data) 
{
    return register_write_proc(buffer, count, data, RAMR_OFFSET);
}      

int ppr_write_proc(struct file *file, const char *buffer, unsigned long count, void *data) 
{
    return register_write_proc(buffer, count, data, PPR_OFFSET);
}      

int tcr_write_proc(struct file *file, const char *buffer, unsigned long count, void *data) 
{
    return register_write_proc(buffer, count, data, TCR_OFFSET);
}      

int vstr_write_proc(struct file *file, const char *buffer, unsigned long count, void *data) 
{
    return register_write_proc(buffer, count, data, VSTR_OFFSET);
}      

int imr_write_proc(struct file *file, const char *buffer, unsigned long count, void *data) 
{
    return register_write_proc(buffer, count, data, IMR_OFFSET);
}      

struct fscc_port *fscc_port_new(struct fscc_card *card, unsigned channel, 
                                unsigned major_number, unsigned minor_number, 
                                struct class *class, struct file_operations *fops,
                                struct proc_dir_entry *fscc_proc_dir)
{	
	struct proc_dir_entry *fifot_proc_entry = 0;
	struct proc_dir_entry *cmdr_proc_entry = 0;
	struct proc_dir_entry *star_proc_entry = 0;
	struct proc_dir_entry *ccr0_proc_entry = 0;
	struct proc_dir_entry *ccr1_proc_entry = 0;
	struct proc_dir_entry *ccr2_proc_entry = 0;
	struct proc_dir_entry *bgr_proc_entry = 0;
	struct proc_dir_entry *ssr_proc_entry = 0;
	struct proc_dir_entry *smr_proc_entry = 0;
	struct proc_dir_entry *tsr_proc_entry = 0;
	struct proc_dir_entry *tmr_proc_entry = 0;
	struct proc_dir_entry *rar_proc_entry = 0;
	struct proc_dir_entry *ramr_proc_entry = 0;
	struct proc_dir_entry *ppr_proc_entry = 0;
	struct proc_dir_entry *tcr_proc_entry = 0;
	struct proc_dir_entry *vstr_proc_entry = 0;
	struct proc_dir_entry *imr_proc_entry = 0;
	
	struct fscc_port *new_port = 0;
	struct device *mdevice = 0;
	unsigned irq_num = 0;
	mode_t read_only_permissions;
	mode_t read_write_permissions;
	
	new_port = (struct fscc_port*)kmalloc(sizeof(struct fscc_port), GFP_KERNEL);
	
	INIT_LIST_HEAD(&new_port->list);
	INIT_LIST_HEAD(&new_port->oframes);
	INIT_LIST_HEAD(&new_port->iframes);
	
	new_port->channel = channel;
	new_port->dev_t = MKDEV(major_number, minor_number);
	new_port->class = class;
	new_port->card = card;
	
	new_port->tx_memory_cap = 10000;
	new_port->rx_memory_cap = 10000;
	
	init_MUTEX(&new_port->semaphore);
	init_waitqueue_head(&new_port->input_queue);
	init_waitqueue_head(&new_port->output_queue);
	
	cdev_init(&new_port->cdev, fops);
	new_port->cdev.owner = THIS_MODULE;
	
	if (cdev_add(&new_port->cdev, new_port->dev_t, 1) < 0) {
		printk(KERN_ERR DEVICE_NAME " cdev_add failed\n");
		return 0;
	}
	
	mdevice = device_create(class, 0, new_port->dev_t, 0, "fscc%i", minor_number);

	if (mdevice <= 0) {
		printk(KERN_ERR DEVICE_NAME " device_create failed\n");
		return 0;
	}
	
	list_add_tail(&new_port->list, &card->ports);
	
	fscc_port_set_register(new_port, 0, IMR_OFFSET, 0xf0f802C0);
	fscc_port_set_register(new_port, 0, CCR0_OFFSET, 0x0000001c);
	fscc_port_set_register(new_port, 0, CCR1_OFFSET, 0x00000008);
	fscc_port_set_register(new_port, 0, FIFOT_OFFSET, 0x08001000);
	
	new_port->name = (char *)kmalloc(10, GFP_KERNEL);
	sprintf(new_port->name, "%s%u", DEVICE_NAME, minor_number);
	
	irq_num = card->pci_dev->irq;
	if (request_irq(irq_num, &fscc_isr, IRQF_SHARED, new_port->name, new_port))
		printk(KERN_ERR DEVICE_NAME " request_irq failed on irq %i\n", irq_num);

	new_port->fscc_proc_dir = fscc_proc_dir;
	new_port->port_proc_dir = proc_mkdir(new_port->name, new_port->fscc_proc_dir);
	new_port->registers_proc_dir = proc_mkdir("registers", new_port->port_proc_dir);
	
	read_only_permissions = S_IRUSR | S_IRGRP | S_IROTH;
	read_write_permissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	
	fifot_proc_entry = create_proc_entry("fifot", read_write_permissions, new_port->registers_proc_dir);
	cmdr_proc_entry = create_proc_entry("cmdr", read_write_permissions, new_port->registers_proc_dir);
	star_proc_entry = create_proc_entry("star", read_only_permissions, new_port->registers_proc_dir);
	ccr0_proc_entry = create_proc_entry("ccr0", read_write_permissions, new_port->registers_proc_dir);
	ccr1_proc_entry = create_proc_entry("ccr1", read_write_permissions, new_port->registers_proc_dir);
	ccr2_proc_entry = create_proc_entry("ccr2", read_write_permissions, new_port->registers_proc_dir);
	bgr_proc_entry = create_proc_entry("bgr", read_write_permissions, new_port->registers_proc_dir);
	ssr_proc_entry = create_proc_entry("ssr", read_write_permissions, new_port->registers_proc_dir);
	smr_proc_entry = create_proc_entry("smr", read_write_permissions, new_port->registers_proc_dir);
	tsr_proc_entry = create_proc_entry("tsr", read_write_permissions, new_port->registers_proc_dir);
	tmr_proc_entry = create_proc_entry("tmr", read_write_permissions, new_port->registers_proc_dir);
	rar_proc_entry = create_proc_entry("rar", read_write_permissions, new_port->registers_proc_dir);
	ramr_proc_entry = create_proc_entry("ramr", read_write_permissions, new_port->registers_proc_dir);
	ppr_proc_entry = create_proc_entry("ppr", read_write_permissions, new_port->registers_proc_dir);
	tcr_proc_entry = create_proc_entry("tcr", read_write_permissions, new_port->registers_proc_dir);
	vstr_proc_entry = create_proc_entry("vstr", read_only_permissions, new_port->registers_proc_dir);
	imr_proc_entry = create_proc_entry("imr", read_write_permissions, new_port->registers_proc_dir);

	if (fifot_proc_entry) {
		fifot_proc_entry->data = new_port;
		fifot_proc_entry->read_proc = fifot_read_proc;
		fifot_proc_entry->write_proc = fifot_write_proc;
	}
	
	if (cmdr_proc_entry) {
		cmdr_proc_entry->data = new_port;
		cmdr_proc_entry->read_proc = cmdr_read_proc;
		cmdr_proc_entry->write_proc = cmdr_write_proc;
	}
	
	if (star_proc_entry) {
		star_proc_entry->data = new_port;
		star_proc_entry->read_proc = star_read_proc;
	}
	
	if (ccr0_proc_entry) {
		ccr0_proc_entry->data = new_port;
		ccr0_proc_entry->read_proc = ccr0_read_proc;
		ccr0_proc_entry->write_proc = ccr0_write_proc;
	}
	
	if (ccr1_proc_entry) {
		ccr1_proc_entry->data = new_port;
		ccr1_proc_entry->read_proc = ccr1_read_proc;
		ccr1_proc_entry->write_proc = ccr1_write_proc;
	}
	
	if (ccr2_proc_entry) {
		ccr2_proc_entry->data = new_port;
		ccr2_proc_entry->read_proc = ccr2_read_proc;
		ccr2_proc_entry->write_proc = ccr2_write_proc;
	}
	
	if (bgr_proc_entry) {
		bgr_proc_entry->data = new_port;
		bgr_proc_entry->read_proc = bgr_read_proc;
		bgr_proc_entry->write_proc = bgr_write_proc;
	}
	
	if (ssr_proc_entry) {
		ssr_proc_entry->data = new_port;
		ssr_proc_entry->read_proc = ssr_read_proc;
		ssr_proc_entry->write_proc = ssr_write_proc;
	}
	
	if (smr_proc_entry) {
		smr_proc_entry->data = new_port;
		smr_proc_entry->read_proc = smr_read_proc;
		smr_proc_entry->write_proc = smr_write_proc;
	}
	
	if (tsr_proc_entry) {
		tsr_proc_entry->data = new_port;
		tsr_proc_entry->read_proc = tsr_read_proc;
		tsr_proc_entry->write_proc = tsr_write_proc;
	}
	
	if (tmr_proc_entry) {
		tmr_proc_entry->data = new_port;
		tmr_proc_entry->read_proc = tmr_read_proc;
		tmr_proc_entry->write_proc = tmr_write_proc;
	}
	
	if (rar_proc_entry) {
		rar_proc_entry->data = new_port;
		rar_proc_entry->read_proc = rar_read_proc;
		rar_proc_entry->write_proc = rar_write_proc;
	}
	
	if (tsr_proc_entry) {
		tsr_proc_entry->data = new_port;
		tsr_proc_entry->read_proc = tsr_read_proc;
		tsr_proc_entry->write_proc = tsr_write_proc;
	}
	
	if (ramr_proc_entry) {
		ramr_proc_entry->data = new_port;
		ramr_proc_entry->read_proc = ramr_read_proc;
		ramr_proc_entry->write_proc = ramr_write_proc;
	}
	
	if (ppr_proc_entry) {
		ppr_proc_entry->data = new_port;
		ppr_proc_entry->read_proc = ppr_read_proc;
		ppr_proc_entry->write_proc = ppr_write_proc;
	}
	
	if (tcr_proc_entry) {
		tcr_proc_entry->data = new_port;
		tcr_proc_entry->read_proc = tcr_read_proc;
		tcr_proc_entry->write_proc = tcr_write_proc;
	}
	
	if (vstr_proc_entry) {
		vstr_proc_entry->data = new_port;
		vstr_proc_entry->read_proc = vstr_read_proc;
	}
	
	if (imr_proc_entry) {
		imr_proc_entry->data = new_port;
		imr_proc_entry->read_proc = imr_read_proc;
		imr_proc_entry->write_proc = imr_write_proc;
	}
	
	fscc_port_execute_RRES(new_port);
	fscc_port_execute_TRES(new_port);
	
	return new_port;
}

void fscc_port_delete(struct fscc_port *port)
{	
	unsigned irq_num = 0;
	
	if (port == 0)
		return;

	remove_proc_entry("fifot", port->registers_proc_dir);
	remove_proc_entry("cmdr", port->registers_proc_dir);
	remove_proc_entry("star", port->registers_proc_dir);
	remove_proc_entry("ccr0", port->registers_proc_dir);
	remove_proc_entry("ccr1", port->registers_proc_dir);
	remove_proc_entry("ccr2", port->registers_proc_dir);
	remove_proc_entry("bgr", port->registers_proc_dir);
	remove_proc_entry("ssr", port->registers_proc_dir);
	remove_proc_entry("smr", port->registers_proc_dir);
	remove_proc_entry("tsr", port->registers_proc_dir);
	remove_proc_entry("tmr", port->registers_proc_dir);
	remove_proc_entry("rar", port->registers_proc_dir);
	remove_proc_entry("ramr", port->registers_proc_dir);
	remove_proc_entry("ppr", port->registers_proc_dir);
	remove_proc_entry("tcr", port->registers_proc_dir);
	remove_proc_entry("vstr", port->registers_proc_dir);
	remove_proc_entry("imr", port->registers_proc_dir);
	
	remove_proc_entry("registers", port->port_proc_dir);
	remove_proc_entry(port->name, port->fscc_proc_dir);
	
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
	
	if (fscc_port_total_oframe_memory(port) + length > port->tx_memory_cap)
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

