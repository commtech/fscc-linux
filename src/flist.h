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
	along with fscc-linux.  If not, see <http://www.gnu.org/licenses/>.

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
