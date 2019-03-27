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

#ifndef FSCC_FLIST_H
#define FSCC_FLIST_H

#include <linux/fs.h> /* Needed to build on older kernel version */
#include <linux/cdev.h> /* struct cdev */
#include <linux/interrupt.h> /* struct tasklet_struct */
#include <linux/version.h> /* LINUX_VERSION_CODE, KERNEL_VERSION */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 26)
#include <linux/semaphore.h> /* struct semaphore */
#endif

#include "frame.h"

struct fscc_flist {
	struct list_head frames;
	unsigned estimated_memory_usage;
};

void fscc_flist_init(struct fscc_flist *flist);
void fscc_flist_delete(struct fscc_flist *flist);
void fscc_flist_add_frame(struct fscc_flist *flist, struct fscc_frame *frame);
struct fscc_frame *fscc_flist_remove_frame(struct fscc_flist *flist);
struct fscc_frame *fscc_flist_remove_frame_if_lte(struct fscc_flist *flist, unsigned size);
struct fscc_frame *fscc_flist_peek_front(struct fscc_flist *flist);
struct fscc_frame *fscc_flist_peek_back(struct fscc_flist *flist);
void fscc_flist_clear(struct fscc_flist *flist);
unsigned fscc_flist_is_empty(struct fscc_flist *flist);
unsigned fscc_flist_calculate_memory_usage(struct fscc_flist *flist);
unsigned fscc_flist_length(struct fscc_flist *flist);

#endif
