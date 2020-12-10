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

#ifndef FSCC_FRAME_H
#define FSCC_FRAME_H

#include <linux/list.h> /* struct list_head */
#include <linux/version.h> // #if LINUX_VERSION_CODE
#include "descriptor.h" /* struct fscc_descriptor */


#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)
typedef struct timespec64 fscc_timestamp;
#else
typedef struct timeval fscc_timestamp;
#endif


struct fscc_frame {
	struct list_head list;
	char *buffer;
	unsigned data_length;
	unsigned buffer_size;
	unsigned number;
	unsigned dma_initialized;
	unsigned fifo_initialized;
	fscc_timestamp timestamp;

	struct fscc_descriptor *d1;
	struct fscc_descriptor *d2;

	dma_addr_t data_handle;
	dma_addr_t d1_handle;
	dma_addr_t d2_handle;

	struct fscc_port *port;
};

struct fscc_frame *fscc_frame_new(struct fscc_port *port);
void fscc_frame_delete(struct fscc_frame *frame);

unsigned fscc_frame_get_length(struct fscc_frame *frame);
unsigned fscc_frame_get_buffer_size(struct fscc_frame *frame);

int fscc_frame_add_data(struct fscc_frame *frame, const char *data,
						 unsigned length);

int fscc_frame_add_data_from_port(struct fscc_frame *frame, struct fscc_port *port,
								  unsigned length);
int fscc_frame_add_data_from_user(struct fscc_frame *frame, const char *data,
						 unsigned length);
int fscc_frame_remove_data(struct fscc_frame *frame, char *destination,
						   unsigned length);
unsigned fscc_frame_is_empty(struct fscc_frame *frame);

void fscc_frame_clear(struct fscc_frame *frame);
int fscc_frame_setup_descriptors(struct fscc_frame *frame);
unsigned fscc_frame_is_dma(struct fscc_frame *frame);
unsigned fscc_frame_is_fifo(struct fscc_frame *frame);

#endif
