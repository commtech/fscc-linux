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

#ifndef FSCC_CONFIG_H
#define FSCC_CONFIG_H

#define DEVICE_NAME "fscc"

#define DEFAULT_INPUT_MEMORY_CAP_VALUE 1000000
#define DEFAULT_OUTPUT_MEMORY_CAP_VALUE 1000000

#define DEFAULT_TIMEOUT_VALUE 50
#define DEFAULT_FORCE_FIFO_VALUE 0
#define DEFAULT_APPEND_STATUS_VALUE 0
#define DEFAULT_APPEND_TIMESTAMP_VALUE 0
#define DEFAULT_IGNORE_TIMEOUT_VALUE 0
#define DEFAULT_TX_MODIFIERS_VALUE XF
#define DEFAULT_RX_MULTIPLE_VALUE 0

#define DEFAULT_FIFOT_VALUE 0x08001000
#define DEFAULT_CCR0_VALUE 0x0011201c
#define DEFAULT_CCR1_VALUE 0x00000018
#define DEFAULT_CCR2_VALUE 0x00000000
#define DEFAULT_BGR_VALUE 0x00000000
#define DEFAULT_SSR_VALUE 0x0000007e
#define DEFAULT_SMR_VALUE 0x00000000
#define DEFAULT_TSR_VALUE 0x0000007e
#define DEFAULT_TMR_VALUE 0x00000000
#define DEFAULT_RAR_VALUE 0x00000000
#define DEFAULT_RAMR_VALUE 0x00000000
#define DEFAULT_PPR_VALUE 0x00000000
#define DEFAULT_TCR_VALUE 0x00000000
#define DEFAULT_IMR_VALUE 0x0f000000
#define DEFAULT_DPLLR_VALUE 0x00000004
#define DEFAULT_FCR_VALUE 0x00000000

#define DEFAULT_CLOCK_BITS {0x0f, 0x61, 0xe5, 0x00, 0x00, 0x00, 0x00, 0x00, \
							0x00, 0x00, 0x00, 0x18, 0x16, 0x40, 0x01, 0x04, \
							0x00, 0xff, 0xff, 0xff} /* 18.432 MHz */

//#define DISABLE_XTAL /* Used when setting the clock on newer cards. */

#define SYSFS_READ_ONLY_MODE S_IRUGO
#define SYSFS_WRITE_ONLY_MODE S_IWUGO
#define SYSFS_READ_WRITE_MODE S_IWUGO | S_IRUGO

#endif

