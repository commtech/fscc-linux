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
#include "card.h" /* struct fscc_card */
#include "port.h" /* struct fscc_port */
#include "utils.h" /* return_{val_}_if_untrue */
#include "config.h" /* DEVICE_NAME */

#include "frame.h" //TODO Remove

unsigned port_exists(void *port)
{	
	struct fscc_card *current_card = 0;
	struct fscc_port *current_port = 0;
	
	return_val_if_untrue(port, 0);
	
	list_for_each_entry(current_card, &fscc_cards, list) {	
		struct list_head *ports = fscc_card_get_ports(current_card);
			
		list_for_each_entry(current_port, ports, list) {
			if (port == current_port)
				return 1;
		}
	}
	
	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 19)
irqreturn_t fscc_isr(int irq, void *potential_port)
#else
irqreturn_t fscc_isr(int irq, void *potential_port, struct pt_regs *regs)
#endif
{
	struct fscc_port *port = 0;
	unsigned isr_value = 0;
	
	if (!port_exists(potential_port))
		return IRQ_NONE;
	
	port = (struct fscc_port *)potential_port;
		
	isr_value = fscc_port_get_register(port, 0, ISR_OFFSET);
	
	if (!isr_value)
		return IRQ_NONE;
		
	printk("isr = 0x%08x\n", isr_value);
	
	port->last_isr_value |= isr_value;
	tasklet_schedule(&port->print_tasklet);
		
	if (isr_value & RFE)
		port->ended_frames += 1;
	
	if (isr_value & RFS)
		port->started_frames += 1;	
	
	if (isr_value & (RFE | RFT | RFS))
		tasklet_schedule(&port->iframe_tasklet); 
	
	if (isr_value & TFT && !port->card->dma)
		tasklet_schedule(&port->oframe_tasklet);
		
	if (isr_value & DT_STOP) {
		struct fscc_frame *frame = 0;

		frame = fscc_port_peek_front_frame(port, &port->oframes);
		
		printk("0x%08x\n", frame->descriptor.control); 
	}
		
	return IRQ_HANDLED;
}

