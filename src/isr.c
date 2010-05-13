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
#include "card.h"
#include "utils.h"
#include "config.h"

unsigned port_exists(void *port)
{	
	struct fscc_card *current_card = 0;
	struct fscc_port *current_port = 0;
	
	list_for_each_entry(current_card, &fscc_cards, list) {		
		list_for_each_entry(current_port, &current_card->ports, list) {
			if (port == current_port)
				return 1;
		}
	}
	
	return 0;
}

irqreturn_t fscc_isr(int irq, void *potential_port)
{
	struct fscc_port *port = 0;
	unsigned isr_value = 0;
	
	if (!port_exists(potential_port))
		return IRQ_NONE;
	
	port = (struct fscc_port *)potential_port;
	
	isr_value = fscc_port_get_register(port, 0, ISR_OFFSET);
	
	if (isr_value)
		printk(KERN_DEBUG "%s interrupt: 0x%08x\n",port->name, isr_value);
	else
		return IRQ_NONE;
		
	/* Receive interrupts need to be in this order */
	if (isr_value & 0x00000004)
		tasklet_schedule(&port->rfe_tasklet);
	
	if (isr_value & 0x00000002)
		tasklet_schedule(&port->rft_tasklet);
	
	if (isr_value & 0x00000001)
		tasklet_schedule(&port->rfs_tasklet);
	
	if (isr_value & 0x00000008)
		printk(KERN_ALERT "%s RFO (Receive Frame Overflow Interrupt)\n", port->name);
	
	if (isr_value & 0x00000010)
		printk(KERN_ALERT "%s RDO (Receive Data Overflow Interrupt)\n", port->name);
	
	if (isr_value & 0x00000020)
		printk(KERN_ALERT "%s RFL (Receive Frame Lost Interrupt)\n", port->name);
	
	if (isr_value & 0x00000100)
		printk(KERN_DEBUG "%s TIN (Timer Expiration Interrupt)\n", port->name);
	
	if (isr_value & 0x00040000)
		tasklet_schedule(&port->tdu_tasklet);
	
	if (isr_value & 0x00010000)
		tasklet_schedule(&port->tft_tasklet);
	
	if (isr_value & 0x00020000)
		printk(KERN_DEBUG "%s ALLS (All Sent Interrupt)\n", port->name);
	
	if (isr_value & 0x01000000)
		printk(KERN_DEBUG "%s CTSS (CTS State Change Interrupt)\n", port->name);
	
	if (isr_value & 0x02000000)
		printk(KERN_DEBUG "%s DSRC (DSR Change Interrupt)\n", port->name);
	
	if (isr_value & 0x04000000)
		printk(KERN_DEBUG "%s CDC (CD Change Interrupt)\n", port->name);
	
	return IRQ_HANDLED;
}

