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

#include "isr.h"
#include "port.h"
#include "utils.h"
#include "config.h"

irqreturn_t fscc_isr(int irq, void *dev_id)
{
	struct fscc_port *current_port = 0;
	unsigned isr_value = 0;
	
	current_port = (struct fscc_port *)dev_id;
	
	isr_value = fscc_port_get_register(current_port, 0, ISR_OFFSET);
	
	if (isr_value)
		printk(KERN_DEBUG DEVICE_NAME " interrupt: 0x%08x\n", isr_value);
	else
		return IRQ_NONE;
	
	if (isr_value & 0x00000001)
		printk(KERN_DEBUG DEVICE_NAME " RFS interrupt\n");
	
	if (isr_value & 0x00000002) {
		printk(KERN_DEBUG DEVICE_NAME " RFT interrupt\n");
		
		fscc_port_empty_RxFIFO(current_port);
		wake_up_interruptible(&current_port->input_queue);
	}
	
	if (isr_value & 0x00000004) {
		printk(KERN_DEBUG DEVICE_NAME " RFE interrupt\n");
		
		fscc_port_empty_RxFIFO(current_port);
		wake_up_interruptible(&current_port->input_queue);
	}
	
	if (isr_value & 0x00000008)
		printk(KERN_DEBUG DEVICE_NAME " RFO interrupt\n");
	
	if (isr_value & 0x00000010)
		printk(KERN_DEBUG DEVICE_NAME " RDO interrupt\n");
	
	if (isr_value & 0x00000020)
		printk(KERN_DEBUG DEVICE_NAME " RFL interrupt\n");
	
	if (isr_value & 0x00000100)
		printk(KERN_DEBUG DEVICE_NAME " TIN interrupt\n");
	
	if (isr_value & 0x00040000) {
		printk(KERN_ALERT DEVICE_NAME " TDU interrupt\n");
		
		if (fscc_port_has_oframes(current_port))
			fscc_port_pop_oframe(current_port);
	}
	
	if (isr_value & 0x00010000) {
		printk(KERN_DEBUG DEVICE_NAME " TFT interrupt\n");
		
		if (fscc_port_has_oframes(current_port)) {
			struct fscc_frame *frame = 0;
	
			frame = fscc_port_peek_front_oframe(current_port);	
		
			while (frame) {
				unsigned unused_fifo_space = 0;
				unsigned leftover_data = 0;
			
				leftover_data = fscc_frame_get_current_length(frame);
				unused_fifo_space = fscc_port_get_available_tx_bytes(current_port);
				
				while (leftover_data) {
					if (leftover_data > unused_fifo_space) {
						fscc_port_fill_TxFIFO(current_port, fscc_frame_get_data(frame), 
									          unused_fifo_space);
						fscc_frame_remove_data(frame, unused_fifo_space);
					}
					else {
						fscc_port_fill_TxFIFO(current_port, fscc_frame_get_data(frame), 
									          leftover_data);
						fscc_frame_remove_data(frame, leftover_data);
						fscc_port_pop_oframe(current_port);
					}
					
					leftover_data = fscc_frame_get_current_length(frame);
					unused_fifo_space = fscc_port_get_available_tx_bytes(current_port);
				}
			
				frame = fscc_port_peek_front_oframe(current_port);	
			}
		}
	}
	
	if (isr_value & 0x00020000)
		printk(KERN_DEBUG DEVICE_NAME " ALLS interrupt\n");
	
	return IRQ_HANDLED;
}
