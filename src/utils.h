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
