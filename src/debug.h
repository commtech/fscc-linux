/*
	Copyright (c) 2019 Commtech, Inc.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

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
