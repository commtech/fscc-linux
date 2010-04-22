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

#include "frame.h"
#include <linux/pci.h>
#include <asm/uaccess.h>

struct fscc_frame *fscc_frame_new(unsigned target_length)
{
	struct fscc_frame *frame = 0;
    
	frame = (struct fscc_frame *)kmalloc(sizeof(struct fscc_frame), GFP_KERNEL);
    
	INIT_LIST_HEAD(&frame->list);
    
	frame->data = (char *)kmalloc(target_length, GFP_KERNEL);
	frame->target_length = target_length;
	frame->current_length = 0;
    
	return frame;
}

void fscc_frame_delete(struct fscc_frame *frame)
{
	if (!frame)
		return;
        
	kfree(frame->data);
	kfree(frame);
}

unsigned fscc_frame_get_target_length(struct fscc_frame *frame)
{
	if (!frame)
		return 0;
        
	return frame->target_length;
}

unsigned fscc_frame_get_current_length(struct fscc_frame *frame)
{
	if (!frame)
		return 0;
        
	return frame->current_length;        
}

unsigned fscc_frame_get_missing_length(struct fscc_frame *frame)
{
	if (!frame)
		return 0;
        
	return frame->target_length - frame->current_length;        
}

bool fscc_frame_is_empty(struct fscc_frame *frame)
{
	return !fscc_frame_get_current_length(frame);
}

bool fscc_frame_is_full(struct fscc_frame *frame)
{
	return fscc_frame_get_current_length(frame) == fscc_frame_get_target_length(frame);
}

void fscc_frame_add_data(struct fscc_frame *frame, const char *data, unsigned length)
{
	if (!frame)
		return;
		
    memcpy(frame->data + frame->current_length, data, length);
	frame->current_length += length;
}

void fscc_frame_remove_data(struct fscc_frame *frame, unsigned length)
{
	if (!frame)
		return;
        
	frame->current_length -= length;
}

//TODO: I think this only is correct in the transmit direction
char *fscc_frame_get_data(struct fscc_frame *frame)
{
	if (frame == 0)
		return 0;
		
	return frame->data + (frame->target_length - frame->current_length);
}

