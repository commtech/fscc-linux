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

#ifndef FSCC_STREAM_H
#define FSCC_STREAM_H

struct fscc_stream {
	char *buffer;
	unsigned data_length;
    unsigned buffer_size;
    spinlock_t spinlock;
};

void fscc_stream_init(struct fscc_stream *stream);
void fscc_stream_delete(struct fscc_stream *stream);

int fscc_stream_add_data(struct fscc_stream *stream, const char *data,
						  unsigned length);
char *fscc_stream_get_data(struct fscc_stream *stream);
unsigned fscc_stream_get_length(struct fscc_stream *stream);
int fscc_stream_remove_data(struct fscc_stream *stream, char *destination, unsigned length);
unsigned fscc_stream_is_empty(struct fscc_stream *stream);
void fscc_stream_clear(struct fscc_stream *stream);

#endif
