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

#define FSCC_GET_STATUS _IOR(FSCC_IOCTL_MAGIC, 22, unsigned *)

#define FSCC_GET_MEMORY_USAGE _IOR(FSCC_IOCTL_MAGIC, 23, struct fscc_memory_cap *)

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
#define SFSCCe_4_LVDS_UA_ID 0x0022
#define SFSCC_4_UA_CPCI_ID 0x0023
#define SFSCC_4_UA_LVDS_ID 0x0025
#define SFSCC_UA_LVDS_ID 0x0026
#define FSCCe_4_UA_ID 0x0027

#define STATUS_LENGTH 2

#endif

