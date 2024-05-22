/*
	Copyright (c) 2022 Commtech, Inc.

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

#ifndef FSCC_IO
#define FSCC_IO

#include <linux/pci.h>
#include <linux/types.h>
#include <linux/dmapool.h>
#include "port.h"

#define DESC_FE_BIT 0x80000000
#define DESC_CSTOP_BIT 0x40000000
#define DESC_HI_BIT 0x20000000
#define DMA_MAX_LENGTH 0x1fffffff

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)
typedef struct timespec64 fscc_timestamp;
#define set_timestamp(x) ktime_get_ts64(x)
#else
typedef struct timeval fscc_timestamp;
#define set_timestamp(x) do_gettimeofday(x)
#endif

struct fscc_descriptor {
	volatile u32 control;
	volatile u32 data_address;
	volatile u32 data_count;
	volatile u32 next_descriptor;
};

typedef struct io_frame {
	struct fscc_descriptor *desc;
	u32 desc_physical_address;
	unsigned char *buffer;
	fscc_timestamp timestamp;
	size_t buffer_size;
	size_t desc_size;
} IO_FRAME;

int fscc_io_initialize(struct fscc_port *port, size_t rx_buffer_size, size_t tx_buffer_size);
void fscc_io_destroy(struct fscc_port *port);
void fscc_io_destroy_pools(struct fscc_port *port);
int fscc_io_purge_tx(struct fscc_port *port);
int fscc_io_purge_rx(struct fscc_port *port);

int fscc_dma_enable(struct fscc_port *port);
int fscc_dma_disable(struct fscc_port *port);
void fscc_dma_execute_RST_R(struct fscc_port *port);
void fscc_dma_execute_RST_T(struct fscc_port *port);
void fscc_dma_execute_STOP_R(struct fscc_port *port);
void fscc_dma_execute_STOP_T(struct fscc_port *port);

int fscc_io_user_read_ready(struct fscc_port *port);
ssize_t fscc_io_read(struct fscc_port *port, char *buf, size_t count);
unsigned fscc_io_transmit_frame(struct fscc_port *port);
size_t fscc_user_get_tx_space(struct fscc_port *port);
int fscc_fifo_read_data(struct fscc_port *port);
int fscc_user_write_frame(struct fscc_port *port, const char *buf, size_t data_length);

void fscc_io_apply_timestamps(struct fscc_port *port);

#endif
