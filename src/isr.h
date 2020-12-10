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

#ifndef FSCC_ISR_H
#define FSCC_ISR_H

#include <linux/version.h> /* LINUX_VERSION_CODE, KERNEL_VERSION */
#include <linux/interrupt.h> /* struct pt_regs */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)
#include <linux/ktime.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 19)
irqreturn_t fscc_isr(int irq, void *dev_id);
#else
irqreturn_t fscc_isr(int irq, void *dev_id, struct pt_regs *regs);
#endif

void oframe_worker(unsigned long data);
void clear_oframe_worker(unsigned long data);
void iframe_worker(unsigned long data);
void istream_worker(unsigned long data);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)
void timer_handler(struct timer_list *data);
#else
void timer_handler(unsigned long data);
#endif

#endif
