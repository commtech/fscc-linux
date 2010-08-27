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

#define TX_FIFO_SIZE 4096
                                              
void fscc_port_empty_frames(struct fscc_port *port, struct list_head *frames);
                           
void fscc_port_execute_GO_R(struct fscc_port *port);
void fscc_port_execute_GO_T(struct fscc_port *port);
void fscc_port_execute_STOP_R(struct fscc_port *port);
void fscc_port_execute_STOP_T(struct fscc_port *port);
void fscc_port_execute_RST_R(struct fscc_port *port);
void fscc_port_execute_RST_T(struct fscc_port *port);
void fscc_port_execute_XF(struct fscc_port *port);

void iframe_worker(unsigned long data)
{	
	struct fscc_port *port = 0;
	int receive_length = 0; //need to be signed
	unsigned finished_frame = 0;

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
		receive_length = bc_fifo_l - fscc_frame_get_current_length(port->pending_iframe);
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
	    char *buffer = 0;
	    
	    if (fscc_memory_usage() + receive_length > memory_cap) {
		    dev_warn(port->device, "F#%i receive rejected (memory constraint)\n",
		            port->pending_iframe->number);
		           
		    fscc_frame_delete(port->pending_iframe);
		    port->pending_iframe = 0;
		    
	        port->handled_frames += 1;
		    
		    return;
	    }
	        
	   buffer = kmalloc(receive_length, GFP_ATOMIC);
		
        if (buffer == NULL) {
		    dev_warn(port->device, "F#%i receive rejected (kmalloc of %i bytes)\n",
				    port->pending_iframe->number, receive_length);
					       
			fscc_frame_delete(port->pending_iframe);
			port->pending_iframe = 0;
	
	        port->handled_frames += 1;
			
			return;
		}

		fscc_port_get_register_rep(port, 0, FIFO_OFFSET, buffer, receive_length);
		fscc_frame_add_data(port->pending_iframe, buffer, receive_length);
		
#ifdef __BIG_ENDIAN
        {
            char status[STATUS_LENGTH];
            
            /* Moves the status bytes to the end. */
            memmove(&status, port->pending_iframe->data, STATUS_LENGTH);
            memmove(port->pending_iframe->data, port->pending_iframe->data + STATUS_LENGTH, port->pending_iframe->current_length - STATUS_LENGTH);
            memmove(port->pending_iframe->data + port->pending_iframe->current_length - STATUS_LENGTH, &status, STATUS_LENGTH);
        }
#endif		
            
		kfree(buffer);

		dev_dbg(port->device, "F#%i <= %i byte%s\n", 
				port->pending_iframe->number, receive_length,
				(receive_length == 1) ? "" : "s");
	}
	
	if (!finished_frame)
	    return;
	
	fscc_frame_trim(port->pending_iframe);
		
	list_add_tail(&port->pending_iframe->list, &port->iframes);
		
	dev_dbg(port->device, "F#%i receive successful\n", 
			port->pending_iframe->number);
				   
	wake_up_interruptible(&port->input_queue);

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
	
	if (port->card->dma) {		              
		fscc_port_set_register(port, 2, DMA_TX_BASE_OFFSET, 
		                       cpu_to_le32(port->pending_oframe->descriptor_handle));
		
		dev_dbg(port->device, "F#%i sending\n", port->pending_oframe->number);	
		fscc_port_execute_GO_T(port);
		//TODO: Memory leak because DMA needs to access the memory. What to do?
		//fscc_frame_delete(port->pending_oframe);
		port->pending_oframe = 0;
		return;
	}

	current_length = fscc_frame_get_current_length(port->pending_oframe);
	target_length = fscc_frame_get_target_length(port->pending_oframe);
	padding_bytes = target_length % 4 ? 4 - target_length % 4 : 0;
	fifo_space = TX_FIFO_SIZE - fscc_port_get_TXCNT(port);		
	
	transmit_length = (current_length + padding_bytes > fifo_space) ? fifo_space : current_length;
	fscc_port_set_register_rep(port, 0, FIFO_OFFSET, port->pending_oframe->data, transmit_length);
	fscc_frame_remove_data(port->pending_oframe, transmit_length);
		
	dev_dbg(port->device, "F#%i => %i byte%s\n", 
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
	unsigned irq_num = 0;
	char clock_bits[20] = DEFAULT_CLOCK_BITS;

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
	
#ifdef DEBUG
	port->interrupt_tracker = debug_interrupt_tracker_new();
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 18)
	                             
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)
		port->device = device_create(port->class, parent, port->dev_t, port, "%s", 
			                         port->name);
	#else
		port->device = device_create(port->class, parent, port->dev_t, "%s", 
			                         port->name);
			                                 
		dev_set_drvdata(port->device, port);
	#endif
	
#else
	class_device_create(port->class, 0, port->dev_t, port->device, "%s", 
	                    port->name);
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
	
	port->last_isr_value = 0;
	
	FSCC_REGISTERS_INIT(port->register_storage);
	
	port->register_storage.FIFOT = DEFAULT_FIFOT_VALUE;
	port->register_storage.CCR0 = DEFAULT_CCR0_VALUE;
	port->register_storage.CCR1 = DEFAULT_CCR1_VALUE;
	port->register_storage.CCR2 = DEFAULT_CCR2_VALUE;
	port->register_storage.BGR = DEFAULT_BGR_VALUE;
	port->register_storage.SSR = DEFAULT_SSR_VALUE;
	port->register_storage.SMR = DEFAULT_SMR_VALUE;
	port->register_storage.TSR = DEFAULT_TSR_VALUE;
	port->register_storage.TMR = DEFAULT_TMR_VALUE;
	port->register_storage.RAR = DEFAULT_RAR_VALUE;
	port->register_storage.RAMR = DEFAULT_RAMR_VALUE;
	port->register_storage.PPR = DEFAULT_PPR_VALUE;
	port->register_storage.TCR = DEFAULT_TCR_VALUE;
	
	if (port->card->dma)
		port->register_storage.IMR = DEFAULT_IMR_VALUE_WITH_DMA;
	else
		port->register_storage.IMR = DEFAULT_IMR_VALUE_WITHOUT_DMA;
	
	port->register_storage.FCR = DEFAULT_FCR_VALUE;
	
	fscc_port_set_registers(port, &port->register_storage);
	
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
	
	if (sysfs_create_group(&port->device->kobj, &port_settings_attr_group)) {
		dev_err(port->device, "sysfs_create_group\n");
		return 0;
	}
	
#ifdef DEBUG	
	if (sysfs_create_group(&port->device->kobj, &port_debug_attr_group)) {
		dev_err(port->device, "sysfs_create_group\n");
		return 0;
	}
#endif /* DEBUG */

#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25) */
	
	/* TODO When these were up above before setting the registers channel 1
	   wouldn't work when scheduling the prink tasklet */
	tasklet_init(&port->oframe_tasklet, oframe_worker, (unsigned long)port);
	tasklet_init(&port->iframe_tasklet, iframe_worker, (unsigned long)port);
	
#ifdef DEBUG
	tasklet_init(&port->print_tasklet, debug_interrupt_display, (unsigned long)port);
#endif
		
	dev_info(port->device, "%s (%x.%02x)\n", fscc_card_get_name(port->card), 
	         fscc_port_get_PREV(port), fscc_port_get_FREV(port));
	
	fscc_port_set_clock_bits(port, clock_bits);
	
	if (port->card->dma) {
	    fscc_port_execute_RST_R(port);
	    fscc_port_execute_RST_T(port);
	}
	
	fscc_port_execute_RRES(port);
	fscc_port_execute_TRES(port);
	
	return port;
}

void fscc_port_delete(struct fscc_port *port)
{
	unsigned irq_num = 0;
	
	return_if_untrue(port);
	
	if (port->card->dma) {
	    fscc_port_execute_STOP_T(port);
	    fscc_port_execute_STOP_R(port);
	    fscc_port_execute_RST_T(port);
	    fscc_port_execute_RST_R(port);
	
	    fscc_port_set_register(port, 2, DMACCR_OFFSET, 0x00000000);
	}
		
	if (port->pending_iframe)
		fscc_frame_delete(port->pending_iframe);
		
	if (port->pending_oframe)
		fscc_frame_delete(port->pending_oframe);
	
	fscc_port_empty_frames(port, &port->iframes);
	fscc_port_empty_frames(port, &port->oframes);

#ifdef DEBUG
	debug_interrupt_tracker_delete(port->interrupt_tracker);
#endif	
	
	irq_num = fscc_card_get_irq(port->card);
	free_irq(irq_num, port);
	
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 18)	
	device_destroy(port->class, port->dev_t);
#endif

	cdev_del(&port->cdev);
	
	if (port->name)
		kfree(port->name);
		
	kfree(port);
}

int fscc_port_write(struct fscc_port *port, const char *data, unsigned length)
{
	struct fscc_frame *frame = 0;
	char *temp_storage = 0;
	unsigned uncopied_bytes = 0;
	
	return_val_if_untrue(port, 0);

	/* Checks to make sure there is a clock present. */
    if (ignore_timeout == 0) {
        __u32 star_value = 0;
        unsigned i = 0;
        unsigned stalled = 0;

        for (i = 0; i < 5; i++) {			
            star_value = fscc_port_get_register(port, 0, STAR_OFFSET);

            if (star_value & CE_BIT) {
                stalled = 1;
            }
            else {
                stalled = 0;
                break;
            }
        }
        
        if (stalled) {
	        dev_dbg(port->device, "device stalled (wrong clock mode?)\n");
	        return -ETIMEDOUT;
	    }
    }
	
	temp_storage = kmalloc(length, GFP_KERNEL);	
	return_val_if_untrue(temp_storage != NULL, 0);
	
	uncopied_bytes = copy_from_user(temp_storage, data, length);
	return_val_if_untrue(!uncopied_bytes, 0);
	
	if (port->card->dma)
		frame = fscc_frame_new(length, 1, port);
	else
		frame = fscc_frame_new(length, 0, port);
	
	fscc_frame_add_data(frame, temp_storage, length);
	
	kfree(temp_storage);
	
	list_add_tail(&frame->list, &port->oframes);
    
	tasklet_schedule(&port->oframe_tasklet);	
	
	return 0;
}

//Returns -ENOBUFS if count is smaller than pending frame size
//Buf needs to be a user buffer
ssize_t fscc_port_read(struct fscc_port *port, char *buf, size_t count)
{
	struct fscc_frame *frame = 0;
	unsigned data_length = 0;
	unsigned uncopied_bytes = 0;
	
	return_val_if_untrue(port, 0);
	
	frame = fscc_port_peek_front_frame(port, &port->iframes);
	
	if (!frame)
		return 0;
		
	data_length = fscc_frame_get_target_length(frame);	
	data_length -= (port->append_status) ? 0 : STATUS_LENGTH;
	
	if (count < data_length)
		return -ENOBUFS;
			
	uncopied_bytes = copy_to_user(buf, fscc_frame_get_remaining_data(frame), 
			                      data_length);
			
	return_val_if_untrue(!uncopied_bytes, 0);
		
	list_del(&frame->list); 
	fscc_frame_delete(frame);
	
	return data_length;
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
		list_del(current_node);
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
	
	return_val_if_untrue(port, 0);
	return_val_if_untrue(bar <= 2, 0);

	offset = port_offset(port, bar, register_offset);
		
	return fscc_card_get_register(port->card, bar, offset);
}

void fscc_port_set_register(struct fscc_port *port, unsigned bar, 
                            unsigned register_offset, __u32 value)
{
	unsigned offset = 0;
	
	return_if_untrue(port);
	return_if_untrue(bar <= 2);

	offset = port_offset(port, bar, register_offset);
		
	fscc_card_set_register(port->card, bar, offset, value);
	
	if (bar == 0)
		((fscc_register *)&port->register_storage)[register_offset / 4] = value;
	else if (register_offset == FCR_OFFSET)
		port->register_storage.FCR = value;
}

void fscc_port_get_register_rep(struct fscc_port *port, unsigned bar, 
                                unsigned register_offset, char *buf,
                                unsigned byte_count)
{
	unsigned offset = 0;
	
	return_if_untrue(port);
	return_if_untrue(bar <= 2);
	return_if_untrue(buf);
	return_if_untrue(byte_count > 0);

	offset = port_offset(port, bar, register_offset);
		
	fscc_card_get_register_rep(port->card, bar, offset, buf, byte_count);
}

void fscc_port_set_register_rep(struct fscc_port *port, unsigned bar,
                                unsigned register_offset, const char *data,
                                unsigned byte_count) 
{
	unsigned offset = 0;
	
	return_if_untrue(port);
	return_if_untrue(bar <= 2);
	return_if_untrue(data);
	return_if_untrue(byte_count > 0);

	offset = port_offset(port, bar, register_offset);
		
	fscc_card_set_register_rep(port->card, bar, offset, data, byte_count);
}

void fscc_port_set_registers(struct fscc_port *port, 
                             const struct fscc_registers *regs)
{
	unsigned i = 0;
	
	return_if_untrue(port);	
	return_if_untrue(regs);
    
	for (i = 0; i < sizeof(*regs) / sizeof(fscc_register); i++) {
		if (is_read_only_register(i * 4))
			continue;
			
		if (((fscc_register *)regs)[i] < 0)
			continue;
			
		if (i * 4 <= IMR_OFFSET) {
			fscc_port_set_register(port, 0, i * 4, ((fscc_register *)regs)[i]);
		}
		else {
			fscc_port_set_register(port, 2, FCR_OFFSET, 
			                       ((fscc_register *)regs)[i]);
		}
	}
}

void fscc_port_get_registers(struct fscc_port *port,
                             struct fscc_registers *regs)
{
	unsigned i = 0;
	
	return_if_untrue(port);	
	return_if_untrue(regs);
			
	for (i = 0; i < sizeof(*regs) / sizeof(fscc_register); i++) {
		if (((fscc_register *)regs)[i] != FSCC_UPDATE_VALUE)
			continue;
			
		if (i * 4 <= IMR_OFFSET) {
			((fscc_register *)regs)[i] = fscc_port_get_register(port, 0, i * 4);
		}
		else {
			((fscc_register *)regs)[i] = fscc_port_get_register(port, 2, 
			                                                    FCR_OFFSET);
		}
	}
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

void fscc_port_suspend(struct fscc_port *port)
{
	return_if_untrue(port);	
	
	fscc_port_get_registers(port, &port->register_storage);
}

void fscc_port_resume(struct fscc_port *port)
{
	return_if_untrue(port);
	
	fscc_port_set_registers(port, &port->register_storage);
}

void fscc_port_flush_tx(struct fscc_port *port)
{	
	return_if_untrue(port);	
	
	dev_dbg(port->device, "flush_tx\n");
	
	fscc_port_execute_TRES(port);
		
	if (port->pending_oframe) {
		fscc_frame_delete(port->pending_oframe);
		port->pending_oframe = 0;
	}

	fscc_port_empty_frames(port, &port->oframes);
}

void fscc_port_flush_rx(struct fscc_port *port)
{
	return_if_untrue(port);	
	
	dev_dbg(port->device, "flush_rx\n");
	
	fscc_port_execute_RRES(port);
		
	if (port->pending_iframe) {
		fscc_frame_delete(port->pending_iframe);
		port->pending_iframe = 0;
	}
	
	fscc_port_empty_frames(port, &port->iframes);
}

unsigned fscc_port_get_frames_qty(struct fscc_port *port, 
                                  struct list_head *frames)
{
	struct list_head *iter = 0;
	struct list_head *temp = 0;
	unsigned qty = 0;
	
	return_val_if_untrue(port, 0);	
	return_val_if_untrue(frames, 0);
	
	list_for_each_safe(iter, temp, frames) {
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

/* Worker function */
unsigned fscc_port_calculate_memory_usage(struct fscc_port *port, 
                                          struct fscc_frame *pending_frame, 
                                          struct list_head *frame_list)
{
	struct fscc_frame *current_frame = 0;	
	unsigned memory = 0;
	
	return_val_if_untrue(port, 0);	
	
	list_for_each_entry(current_frame, frame_list, list) {
		memory += fscc_frame_get_current_length(current_frame);
	}	
	
	if (pending_frame)
		memory += fscc_frame_get_current_length(pending_frame);
	
	return memory;
}

unsigned fscc_port_get_output_memory_usage(struct fscc_port *port)
{
	return_val_if_untrue(port, 0);	
	
	return fscc_port_calculate_memory_usage(port, port->pending_oframe, 
	                                        &port->oframes);
}

unsigned fscc_port_get_input_memory_usage(struct fscc_port *port)
{
	return_val_if_untrue(port, 0);
	
	return fscc_port_calculate_memory_usage(port, port->pending_iframe, 
	                                        &port->iframes);
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
	int j = 0; // Must be signed because we are going backwards through the array
	int i = 0; // Must be signed because we are going backwards through the array
	unsigned strb_value = STRB_BASE;
	unsigned dta_value = DTA_BASE;
	unsigned clk_value = CLK_BASE;
	
	__u32 *data = 0;
	unsigned data_index = 0;
	
	return_if_untrue(port);
	
	data = kmalloc(sizeof(__u32) * 323, GFP_KERNEL);
	
	if (port->channel == 1) {
		strb_value <<= 0x08;
		dta_value <<= 0x08;
		clk_value <<= 0x08;
	}
    
	orig_fcr_value = fscc_card_get_register(port->card, 2, FCR_OFFSET);
    
	data[data_index++] = new_fcr_value = orig_fcr_value & 0xfffff0f0;
	
	for (i = 0; i < 20; i++) {
		for (j = 7; j >= 0; j--) {
			int bit = ((clock_data[i] >> j) & 1);
			
			if (bit)
				new_fcr_value |= dta_value; /* Set data bit */
			else
				new_fcr_value &= ~dta_value; /* Clear clock bit */
				
	        data[data_index++] = new_fcr_value |= clk_value; /* Set clock bit */
	        data[data_index++] = new_fcr_value &= ~clk_value; /* Clear clock bit */
		}
	}
	
	new_fcr_value = orig_fcr_value & 0xfffff0f0;

	new_fcr_value |= strb_value; /* Set strobe bit */			
	new_fcr_value &= ~clk_value; /* Clear clock bit	*/	
	
	data[data_index++] = new_fcr_value;
	data[data_index++] = orig_fcr_value;
	
	fscc_port_set_register_rep(port, 2, FCR_OFFSET, (char *)data, data_index * 4);
	
	kfree(data);
}

void fscc_port_set_append_status(struct fscc_port *port, unsigned value)
{
	return_if_untrue(port);
	
	port->append_status = (value) ? 1 : 0;
}

unsigned fscc_port_get_append_status(struct fscc_port *port)
{
	return_val_if_untrue(port, 0);
	
	return port->append_status;
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

void fscc_port_execute_GO_R(struct fscc_port *port)
{	
	return_if_untrue(port);	
	return_if_untrue(port->card->dma == 1);
	
	fscc_port_set_register(port, 2, DMACCR_OFFSET, 0x00000001);
}

void fscc_port_execute_GO_T(struct fscc_port *port)
{
	return_if_untrue(port);	
	return_if_untrue(port->card->dma == 1);
	
	fscc_port_set_register(port, 2, DMACCR_OFFSET, 0x00000002);
}

void fscc_port_execute_RST_R(struct fscc_port *port)
{
	return_if_untrue(port);	
	return_if_untrue(port->card->dma == 1);
	
	fscc_port_set_register(port, 2, DMACCR_OFFSET, 0x00000010);
}

void fscc_port_execute_RST_T(struct fscc_port *port)
{
	return_if_untrue(port);	
	return_if_untrue(port->card->dma == 1);
	
	fscc_port_set_register(port, 2, DMACCR_OFFSET, 0x00000020);
}

void fscc_port_execute_STOP_R(struct fscc_port *port)
{
	return_if_untrue(port);	
	return_if_untrue(port->card->dma == 1);
	
	fscc_port_set_register(port, 2, DMACCR_OFFSET, 0x00000100);
}

void fscc_port_execute_STOP_T(struct fscc_port *port)
{
	return_if_untrue(port);	
	return_if_untrue(port->card->dma == 1);
	
	fscc_port_set_register(port, 2, DMACCR_OFFSET, 0x00000200);
}
 
unsigned fscc_port_using_async(struct fscc_port *port)
{
	return_val_if_untrue(port, 0);
	
	switch (port->channel) {
	case 0:
		return port->register_storage.FCR & 0x01000000;

	case 1:
		return port->register_storage.FCR & 0x02000000;
	}
	
	return 0;
}

#ifdef DEBUG
unsigned fscc_port_get_interrupt_count(struct fscc_port *port, __u32 isr_bit)
{
	return_val_if_untrue(port, 0);
	
	return debug_interrupt_tracker_get_count(port->interrupt_tracker, isr_bit);
}

void fscc_port_increment_interrupt_counts(struct fscc_port *port, __u32 isr_value)
{
	return_if_untrue(port);
	
	debug_interrupt_tracker_increment_all(port->interrupt_tracker, isr_value);
}
#endif

