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
#include "utils.h"
#include "config.h"
#include <linux/pci.h>
#include <asm/uaccess.h>

static unsigned frame_counter = 1;

void fscc_frame_update_buffer_size(struct fscc_frame *frame, unsigned length);

struct fscc_frame *fscc_frame_new(unsigned target_length)
{
	struct fscc_frame *frame = 0;
	
	frame = kmalloc(sizeof(*frame), GFP_ATOMIC);
	
	return_val_if_untrue(frame, 0);
    
	INIT_LIST_HEAD(&frame->list);
	
	frame->target_length = 0;
	frame->current_length = 0;
	frame->data = 0;
	
	frame->number = frame_counter;	
	frame_counter += 1;
	
	fscc_frame_update_buffer_size(frame, target_length);
    
	return frame;
}

void fscc_frame_delete(struct fscc_frame *frame)
{	
	return_if_untrue(frame);
        
    if (frame->data)
		kfree(frame->data);
		
	kfree(frame);
}

unsigned fscc_frame_get_target_length(struct fscc_frame *frame)
{
	return_val_if_untrue(frame, 0);
        
	return frame->target_length;
}

unsigned fscc_frame_get_current_length(struct fscc_frame *frame)
{
	return_val_if_untrue(frame, 0);
        
	return frame->current_length;        
}

unsigned fscc_frame_get_missing_length(struct fscc_frame *frame)
{
	return_val_if_untrue(frame, 0);
        
	return frame->target_length - frame->current_length;        
}

bool fscc_frame_is_empty(struct fscc_frame *frame)
{
	return_val_if_untrue(frame, 0);
	
	return !fscc_frame_get_current_length(frame);
}

bool fscc_frame_is_full(struct fscc_frame *frame)
{
	return_val_if_untrue(frame, 0);
	
	return fscc_frame_get_current_length(frame) == fscc_frame_get_target_length(frame);
}

void fscc_frame_add_data(struct fscc_frame *frame, const char *data, unsigned length)
{
	return_if_untrue(frame);	
	
	if (frame->current_length + length > frame->target_length)
		fscc_frame_update_buffer_size(frame, frame->current_length + length);
		
    memmove(frame->data + frame->current_length, data, length);
	frame->current_length += length;
}

void fscc_frame_remove_data(struct fscc_frame *frame, unsigned length)
{
	return_if_untrue(frame);
        
	frame->current_length -= length;
}

char *fscc_frame_get_remaining_data(struct fscc_frame *frame)
{
	return_val_if_untrue(frame, 0);

	return frame->data + (frame->target_length - frame->current_length);
}

void fscc_frame_trim(struct fscc_frame *frame)
{
	return_if_untrue(frame);
	
	fscc_frame_update_buffer_size(frame, frame->current_length);
}

void fscc_frame_update_buffer_size(struct fscc_frame *frame, unsigned length)
{
	char *new_data = 0;
	
	return_if_untrue(frame);
	
	warn_if_untrue(length >= frame->current_length);
	
	if (length == 0) {
		if (frame->data) {
			kfree(frame->data);
			frame->data = 0;
		}
		
		frame->current_length = 0;
		frame->target_length = 0;
		
		return;
	}
	
	if (frame->target_length == length)
		return;	

	new_data = kmalloc(length, GFP_KERNEL);
	
	return_if_untrue(new_data);
	
	memset(new_data, 0, sizeof(new_data));
	
	if (frame->data) {
		if (frame->current_length)
			memmove(new_data, frame->data, frame->current_length);
			
		kfree(frame->data);
	}
	
	frame->data = new_data;
	frame->current_length = min(frame->current_length, length);
	frame->target_length = length;
}

