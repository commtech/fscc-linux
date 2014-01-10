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

#ifndef FSCC_DEBUG_H
#define FSCC_DEBUG_H

#include <linux/module.h> /* __u32 */

struct debug_interrupt_tracker {
	unsigned rfs;
	unsigned rft;
	unsigned rfe;
	unsigned rfo;
	unsigned rdo;
	unsigned rfl;

	unsigned reserved1[2];

	unsigned tin;

	unsigned reserved2[1];

	unsigned dr_hi;
	unsigned dt_hi;
	unsigned dr_fe;
	unsigned dt_fe;
	unsigned dr_stop;
	unsigned dt_stop;

	unsigned tft;
	unsigned alls;
	unsigned tdu;

	unsigned reserved3[5];

	unsigned ctss;
	unsigned dsrc;
	unsigned cdc;
	unsigned ctsa;

	unsigned reserved4[4];
};

struct debug_interrupt_tracker *debug_interrupt_tracker_new(void);
void debug_interrupt_tracker_delete(struct debug_interrupt_tracker *tracker);
void debug_interrupt_tracker_increment_all(struct debug_interrupt_tracker *tracker,
									   __u32 isr_value);
unsigned debug_interrupt_tracker_get_count(struct debug_interrupt_tracker *tracker,
										   __u32 isr_bit);
void debug_interrupt_display(unsigned long data);

#endif
