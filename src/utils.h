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

#ifndef FSCC_UTILS_H
#define FSCC_UTILS_H

#include "fscc.h" /* struct fscc_registers */
#include "port.h" /* struct fscc_port */
#include "config.h" /* DEVICE_NAME */

#define warn_if_untrue(expr) \
	if (expr) {} else \
	{ \
		printk(KERN_WARNING DEVICE_NAME " %s %s\n", #expr, "is untrue."); \
	}

#define return_if_untrue(expr) \
	if (expr) {} else \
	{ \
		printk(KERN_ERR DEVICE_NAME " %s %s\n", #expr, "is untrue."); \
		return; \
	}

#define return_val_if_untrue(expr, val) \
	if (expr) {} else \
	{ \
		printk(KERN_ERR DEVICE_NAME " %s %s\n", #expr, "is untrue."); \
		return val; \
	}

__u32 chars_to_u32(const char *data);
int str_to_register_offset(const char *str);
int str_to_interrupt_offset(const char *str);
unsigned is_read_only_register(unsigned offset);
unsigned port_offset(struct fscc_port *port, unsigned bar, unsigned offset);
unsigned port_exists(void *port);
unsigned is_fscc_device(struct pci_dev *pdev);

#endif
