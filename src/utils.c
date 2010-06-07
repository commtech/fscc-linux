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

#include "utils.h"
#include "port.h"
#include "card.h"
#include "config.h"

__u32 chars_to_u32(const char *data)
{
	return *((__u32*)data);
}

__s32 offset_to_value(const struct fscc_registers *registers, unsigned offset)
{
	return ((__s32 *)registers)[offset / 4];
}

int str_to_offset(const char *str)
{
	if (strcmp(str, "fifo") == 0)
		return FIFO_OFFSET;
	else if (strcmp(str, "bcfl") == 0)
		return BC_FIFO_L_OFFSET;
	else if (strcmp(str, "fifot") == 0)
		return FIFOT_OFFSET;
	else if (strcmp(str, "fifobc") == 0)
		return FIFO_BC_OFFSET;
	else if (strcmp(str, "fifofc") == 0)
		return FIFO_FC_OFFSET;
	else if (strcmp(str, "cmdr") == 0)
		return CMDR_OFFSET;
	else if (strcmp(str, "star") == 0)
		return STAR_OFFSET;
	else if (strcmp(str, "ccr0") == 0)
		return CCR0_OFFSET;
	else if (strcmp(str, "ccr1") == 0)
		return CCR1_OFFSET;
	else if (strcmp(str, "ccr2") == 0)
		return CCR2_OFFSET;
	else if (strcmp(str, "bgr") == 0)
		return BGR_OFFSET;
	else if (strcmp(str, "ssr") == 0)
		return SSR_OFFSET;
	else if (strcmp(str, "smr") == 0)
		return SMR_OFFSET;
	else if (strcmp(str, "tsr") == 0)
		return TSR_OFFSET;
	else if (strcmp(str, "tmr") == 0)
		return TMR_OFFSET;
	else if (strcmp(str, "rar") == 0)
		return RAR_OFFSET;
	else if (strcmp(str, "ramr") == 0)
		return RAMR_OFFSET;
	else if (strcmp(str, "ppr") == 0)
		return PPR_OFFSET;
	else if (strcmp(str, "tcr") == 0)
		return TCR_OFFSET;
	else if (strcmp(str, "vstr") == 0)
		return VSTR_OFFSET;
	else if (strcmp(str, "isr") == 0)
		return ISR_OFFSET;
	else if (strcmp(str, "imr") == 0)
		return IMR_OFFSET;
	else if (strcmp(str, "fcr") == 0)
		return FCR_OFFSET;
	else
		printk(KERN_NOTICE DEVICE_NAME " invalid str passed into str_to_offset\n");

	return -1;
}

unsigned is_read_only_register(unsigned offset)
{
	switch (offset) {
	case STAR_OFFSET:
	case VSTR_OFFSET:
			return 1;
	}
	
	return 0;
}
