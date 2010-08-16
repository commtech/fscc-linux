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
#include "utils.h" /* port_exists */

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
		
	port->last_isr_value |= isr_value;
	
	if (isr_value & RFE)
		port->ended_frames += 1;
	
	if (isr_value & RFS)
		port->started_frames += 1;
	
	if (isr_value & (RFE | RFT | RFS))
		tasklet_schedule(&port->iframe_tasklet); 
	
	if (isr_value & TFT && !port->card->dma)
		tasklet_schedule(&port->oframe_tasklet);
	
	if (isr_value & DT_STOP)
		tasklet_schedule(&port->oframe_tasklet);

#ifdef DEBUG
	tasklet_schedule(&port->print_tasklet);
	fscc_port_increment_interrupt_counts(port, isr_value);
#endif
	
	return IRQ_HANDLED;
}

