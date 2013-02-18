/*
	Copyright (C) 2012 Commtech, Inc.

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
	along with fscc-linux.	If not, see <http://www.gnu.org/licenses/>.

*/

#include <linux/slab.h> /* kmalloc */

#include "stream.h"
#include "utils.h" /* return_{val_}if_true */

int fscc_stream_update_buffer_size(struct fscc_stream *stream,
									unsigned size);

void fscc_stream_init(struct fscc_stream *stream)
{
	unsigned long flags = 0;

	spin_lock_init(&stream->spinlock);

	spin_lock_irqsave(&stream->spinlock, flags);

	stream->data_length = 0;
	stream->buffer_size = 0;
    stream->buffer = 0;

	spin_unlock_irqrestore(&stream->spinlock, flags);
}

void fscc_stream_delete(struct fscc_stream *stream)
{
	unsigned long flags = 0;

	return_if_untrue(stream);

	spin_lock_irqsave(&stream->spinlock, flags);

    fscc_stream_update_buffer_size(stream, 0);

	spin_unlock_irqrestore(&stream->spinlock, flags);
}

unsigned fscc_stream_get_length(struct fscc_stream *stream)
{
	return_val_if_untrue(stream, 0);

	return stream->data_length;
}

//TODO: This could cause an issue w here data_length is less before it makes
//it into remove_Data
void fscc_stream_clear(struct fscc_stream *stream)
{
    fscc_stream_remove_data(stream, NULL, stream->data_length);
}

unsigned fscc_stream_is_empty(struct fscc_stream *stream)
{
	return_val_if_untrue(stream, 0);

    return stream->data_length == 0;
}

int fscc_stream_add_data(struct fscc_stream *stream, const char *data,
						  unsigned length)
{
	unsigned long flags = 0;

	return_val_if_untrue(stream, 0);

	spin_lock_irqsave(&stream->spinlock, flags);

    /* Only update buffer size if there isn't enough space already */
    if (stream->data_length + length > stream->buffer_size) {
        if (fscc_stream_update_buffer_size(stream, stream->data_length + length) == 0) {
	        spin_unlock_irqrestore(&stream->spinlock, flags);
    		return 0;
        }
    }

    /* Copy the new data to the end of the stream */
    memmove(stream->buffer + stream->data_length, data, length);

    stream->data_length += length;

	spin_unlock_irqrestore(&stream->spinlock, flags);

	return 1;
}


// Destination must be a user address so copy_to_user works
int fscc_stream_remove_data(struct fscc_stream *stream, char *destination, unsigned length)
{
	unsigned long flags = 0;

    return_val_if_untrue(stream, 0);

    if (length == 0)
        return 1;

	spin_lock_irqsave(&stream->spinlock, flags);

    if (stream->data_length == 0) {
	    spin_unlock_irqrestore(&stream->spinlock, flags);
        return 1;
    }

    /* Make sure we don't remove remove data than we have */
    if (length > stream->data_length) {
	    spin_unlock_irqrestore(&stream->spinlock, flags);
        return 0;
    }

    /* Copy the data into the outside buffer */
    if (destination)
        copy_to_user(destination, stream->buffer, length);

    stream->data_length -= length;

    /* Move the data up in the buffer (essentially removing the old data) */
    memmove(stream->buffer, stream->buffer + length, stream->data_length);

	spin_unlock_irqrestore(&stream->spinlock, flags);

    return 1;
}

int fscc_stream_update_buffer_size(struct fscc_stream *stream,
									unsigned size)
{
	char *new_buffer = 0;
	int malloc_flags = 0;

	return_val_if_untrue(stream, 0);

	if (size == 0) {
		if (stream->buffer) {
			kfree(stream->buffer);
			stream->buffer = 0;
		}

        stream->buffer_size = 0;
		stream->data_length = 0;

		return 1;
	}

	malloc_flags |= GFP_ATOMIC;

	new_buffer = kmalloc(size, malloc_flags);

	return_val_if_untrue(new_buffer, 0);

	memset(new_buffer, 0, size);

	if (stream->buffer) {
		if (stream->data_length) {
            /* Truncate data length if the new buffer size is less than the data length */
            stream->data_length = min(stream->data_length, size);

            /* Copy over the old buffer data to the new buffer */
            memmove(new_buffer, stream->buffer, stream->data_length);
		}

		kfree(stream->buffer);
	}

	stream->buffer = new_buffer;
	stream->buffer_size = size;

	return 1;
}

