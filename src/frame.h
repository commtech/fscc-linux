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

#ifndef FSCC_FRAME_H
#define FSCC_FRAME_H

#include <linux/list.h>

struct fscc_frame {
	struct list_head list;
	char *data;
	unsigned target_length;
	unsigned current_length;
	unsigned number;
};

struct fscc_frame *fscc_frame_new(unsigned target_length);
void fscc_frame_delete(struct fscc_frame *frame);
unsigned fscc_frame_get_target_length(struct fscc_frame *frame);
unsigned fscc_frame_get_current_length(struct fscc_frame *frame);
unsigned fscc_frame_get_missing_length(struct fscc_frame *frame);
void fscc_frame_add_data(struct fscc_frame *frame, const char *data, unsigned length);
void fscc_frame_remove_data(struct fscc_frame *frame, unsigned length);
bool fscc_frame_is_empty(struct fscc_frame *frame);
bool fscc_frame_is_full(struct fscc_frame *frame);

char *fscc_frame_get_remaining_data(struct fscc_frame *frame);
void fscc_frame_trim(struct fscc_frame *frame);

#endif
