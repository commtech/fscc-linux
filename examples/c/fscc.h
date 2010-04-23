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

#ifndef FSCC_H
#define FSCC_H

#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>

struct fscc_registers {
	int32_t reserved1[2];

	int32_t FIFOT;

	int32_t reserved2[2];

	int32_t CMDR;
	int32_t STAR; /* Read-only */
	int32_t CCR0;
	int32_t CCR1;
	int32_t CCR2;
	int32_t BGR;
	int32_t SSR;
	int32_t SMR;
	int32_t TSR;
	int32_t TMR;
	int32_t RAR;
	int32_t RAMR;
	int32_t PPR;
	int32_t TCR;
	int32_t VSTR; /* Read-only */
	
	int32_t reserved3[1];
	
	int32_t IMR;
};

#define FSCC_UPDATE_VALUE -2

#define FSCC_IOCTL_MAGIC 0x18
#define FSCC_GET_REGISTERS _IOR(FSCC_IOCTL_MAGIC, 0, struct fscc_registers *)
#define FSCC_SET_REGISTERS _IOW(FSCC_IOCTL_MAGIC, 1, struct fscc_registers *)

#define FSCC_REGISTERS_INIT(registers) memset(&registers, -1, sizeof(registers))

#endif

