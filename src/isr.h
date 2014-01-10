/*
	Copyright (C) 2014 Commtech, Inc.

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
	along with fscc-linux.	If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef FSCC_ISR_H
#define FSCC_ISR_H

#include <linux/version.h> /* LINUX_VERSION_CODE, KERNEL_VERSION */
#include <linux/interrupt.h> /* struct pt_regs */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 19)
irqreturn_t fscc_isr(int irq, void *dev_id);
#else
irqreturn_t fscc_isr(int irq, void *dev_id, struct pt_regs *regs);
#endif

void oframe_worker(unsigned long data);
void clear_oframe_worker(unsigned long data);
void iframe_worker(unsigned long data);
void istream_worker(unsigned long data);

void timer_handler(unsigned long data);

#endif
