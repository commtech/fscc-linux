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


#include "flist.h"
#include "utils.h" /* return_{val_}if_true */
#include "frame.h"
#include "debug.h"


void fscc_flist_init(struct fscc_flist *flist)
{
	INIT_LIST_HEAD(&flist->frames);

	flist->estimated_memory_usage = 0;
}

void fscc_flist_delete(struct fscc_flist *flist)
{
	return_if_untrue(flist);

	fscc_flist_clear(flist);
}

void fscc_flist_add_frame(struct fscc_flist *flist, struct fscc_frame *frame)
{
	list_add_tail(&frame->list, &flist->frames);

	flist->estimated_memory_usage += fscc_frame_get_length(frame);
}

struct fscc_frame *fscc_flist_peek_front(struct fscc_flist *flist)
{
	if (list_empty(&flist->frames))
		return 0;

	return list_first_entry(&flist->frames, struct fscc_frame, list);
}

struct fscc_frame *fscc_flist_peek_back(struct fscc_flist *flist)
{
	if (list_empty(&flist->frames))
		return 0;

	return list_entry((&flist->frames)->prev, struct fscc_frame, list);
}

struct fscc_frame *fscc_flist_remove_frame(struct fscc_flist *flist)
{
	struct fscc_frame *frame = 0;

	if (list_empty(&flist->frames))
		return 0;

	frame = list_first_entry(&flist->frames, struct fscc_frame, list);

	list_del(&frame->list);

	flist->estimated_memory_usage -= fscc_frame_get_length(frame);

	return frame;
}

struct fscc_frame *fscc_flist_remove_frame_if_lte(struct fscc_flist *flist, 
                                                  unsigned size)
{
	struct fscc_frame *frame = 0;
	unsigned frame_length = 0;

	if (list_empty(&flist->frames))
		return 0;

	frame = list_first_entry(&flist->frames, struct fscc_frame, list);

	frame_length = fscc_frame_get_length(frame);

	if (frame_length > size)
		return 0;

	list_del(&frame->list);

	flist->estimated_memory_usage -= fscc_frame_get_length(frame);

	return frame;
}

void fscc_flist_clear(struct fscc_flist *flist)
{
	struct list_head *current_node = 0;
	struct list_head *temp_node = 0;

	list_for_each_safe(current_node, temp_node, &flist->frames) {
		struct fscc_frame *current_frame = 0;

		current_frame = list_entry(current_node, struct fscc_frame, list);

		list_del(current_node);

		fscc_frame_delete(current_frame);
	}

	flist->estimated_memory_usage = 0;
}

unsigned fscc_flist_is_empty(struct fscc_flist *flist)
{
	return list_empty(&flist->frames);
}

unsigned fscc_flist_calculate_memory_usage(struct fscc_flist *flist)
{
	unsigned memory = 0;
	struct fscc_frame *current_frame = 0;

	list_for_each_entry(current_frame, &flist->frames, list) {
		memory += fscc_frame_get_length(current_frame);
	}

	return memory;
}

unsigned fscc_flist_length(struct fscc_flist *flist)
{
	unsigned num_frames = 0;
	struct fscc_frame *current_frame = 0;

	list_for_each_entry(current_frame, &flist->frames, list) {
		num_frames++;
	}

	return num_frames;
}
