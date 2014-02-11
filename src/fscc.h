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

#ifndef FSCC_H
#define FSCC_H

#include <linux/sched.h> /* wait_queue_head_t */
#include <linux/fs.h> /* struct indode on <= 2.6.19 */

#define FSCC_REGISTERS_INIT(registers) memset(&registers, -1, sizeof(registers))
#define FSCC_MEMORY_CAP_INIT(memory_cap) memset(&memory_cap, -1, sizeof(memory_cap))
#define FSCC_UPDATE_VALUE -2

#define FSCC_IOCTL_MAGIC 0x18

#define FSCC_GET_REGISTERS _IOR(FSCC_IOCTL_MAGIC, 0, struct fscc_registers *)
#define FSCC_SET_REGISTERS _IOW(FSCC_IOCTL_MAGIC, 1, const struct fscc_registers *)

#define FSCC_PURGE_TX _IO(FSCC_IOCTL_MAGIC, 2)
#define FSCC_PURGE_RX _IO(FSCC_IOCTL_MAGIC, 3)

#define FSCC_ENABLE_APPEND_STATUS _IO(FSCC_IOCTL_MAGIC, 4)
#define FSCC_DISABLE_APPEND_STATUS _IO(FSCC_IOCTL_MAGIC, 5)
#define FSCC_GET_APPEND_STATUS _IOR(FSCC_IOCTL_MAGIC, 13, unsigned *)

#define FSCC_SET_MEMORY_CAP _IOW(FSCC_IOCTL_MAGIC, 6, struct fscc_memory_cap *)
#define FSCC_GET_MEMORY_CAP _IOR(FSCC_IOCTL_MAGIC, 7, struct fscc_memory_cap *)

#define FSCC_SET_CLOCK_BITS _IOW(FSCC_IOCTL_MAGIC, 8, const unsigned char[20])

#define FSCC_ENABLE_IGNORE_TIMEOUT _IO(FSCC_IOCTL_MAGIC, 10)
#define FSCC_DISABLE_IGNORE_TIMEOUT _IO(FSCC_IOCTL_MAGIC, 11)
#define FSCC_GET_IGNORE_TIMEOUT _IOR(FSCC_IOCTL_MAGIC, 15, unsigned *)

#define FSCC_SET_TX_MODIFIERS _IOW(FSCC_IOCTL_MAGIC, 12, const unsigned)
#define FSCC_GET_TX_MODIFIERS _IOR(FSCC_IOCTL_MAGIC, 14, unsigned *)

#define FSCC_ENABLE_RX_MULTIPLE _IO(FSCC_IOCTL_MAGIC, 16)
#define FSCC_DISABLE_RX_MULTIPLE _IO(FSCC_IOCTL_MAGIC, 17)
#define FSCC_GET_RX_MULTIPLE _IOR(FSCC_IOCTL_MAGIC, 18, unsigned *)

#define FSCC_ENABLE_APPEND_TIMESTAMP _IO(FSCC_IOCTL_MAGIC, 19)
#define FSCC_DISABLE_APPEND_TIMESTAMP _IO(FSCC_IOCTL_MAGIC, 20)
#define FSCC_GET_APPEND_TIMESTAMP _IOR(FSCC_IOCTL_MAGIC, 21, unsigned *)


enum transmit_modifiers { XF=0, XREP=1, TXT=2, TXEXT=4 };
typedef __s64 fscc_register;

struct fscc_registers {
	/* BAR 0 */
	fscc_register reserved1[2];

	fscc_register FIFOT;

	fscc_register reserved2[2];

	fscc_register CMDR;
	fscc_register STAR; /* Read-only */
	fscc_register CCR0;
	fscc_register CCR1;
	fscc_register CCR2;
	fscc_register BGR;
	fscc_register SSR;
	fscc_register SMR;
	fscc_register TSR;
	fscc_register TMR;
	fscc_register RAR;
	fscc_register RAMR;
	fscc_register PPR;
	fscc_register TCR;
	fscc_register VSTR; /* Read-only */

	fscc_register reserved3[1];

	fscc_register IMR;
	fscc_register DPLLR;

	/* BAR 2 */
	fscc_register FCR;
};

struct fscc_memory_cap {
	int input;
	int output;
};

extern struct list_head fscc_cards;

#define COMMTECH_VENDOR_ID 0x18f7

#define FSCC_ID 0x000f
#define SFSCC_ID 0x0014
#define SFSCC_104_LVDS_ID 0x0015
#define FSCC_232_ID 0x0016
#define SFSCC_104_UA_ID 0x0017
#define SFSCC_4_UA_ID 0x0018
#define SFSCC_UA_ID 0x0019
#define SFSCC_LVDS_ID 0x001a
#define FSCC_4_UA_ID 0x001b
#define SFSCC_4_LVDS_ID 0x001c
#define FSCC_UA_ID 0x001d
#define SFSCCe_4_ID 0x001e
#define SFSCC_4_CPCI_ID 0x001f
#define SFSCC_4_UA_CPCI_ID 0x0023
#define SFSCC_4_UA_LVDS_ID 0x0025
#define SFSCC_UA_LVDS_ID 0x0026
#define FSCCe_4_UA_ID 0x0027

#define STATUS_LENGTH 2

#endif

