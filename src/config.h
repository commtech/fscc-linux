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
#define SYSFS_WRITE_ONLY_MODE 0220
#define SYSFS_READ_WRITE_MODE 0660

#endif

