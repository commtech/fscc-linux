/*
	Copyright (C) 2013 Commtech, Inc.

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
#include <linux/uaccess.h>

#include "frame.h"
#include "utils.h" /* return_{val_}if_true */
#include "port.h" /* struct fscc_port */
#include "card.h" /* struct fscc_card */

static unsigned frame_counter = 1;

int fscc_frame_update_buffer_size(struct fscc_frame *frame, unsigned length);
int fscc_frame_setup_descriptors(struct fscc_frame *frame, struct pci_dev *pci_dev);

struct fscc_frame *fscc_frame_new(unsigned dma, struct fscc_port *port)
{
	struct fscc_frame *frame = 0;
	unsigned long flags = 0;

	frame = kmalloc(sizeof(*frame), GFP_ATOMIC);

	return_val_if_untrue(frame, 0);

	memset(frame, 0, sizeof(*frame));

	spin_lock_init(&frame->spinlock);

	spin_lock_irqsave(&frame->spinlock, flags);

	INIT_LIST_HEAD(&frame->list);

	frame->data_length = 0;
	frame->buffer_size = 0;
	frame->buffer = 0;
	frame->handled = 0;
	frame->dma = dma;
	frame->port = port;

	frame->number = frame_counter;
	frame_counter += 1;

	if (frame->dma) {
		/* Switch to FIFO based transmission as a fall back. */
		if (!fscc_frame_setup_descriptors(frame, port->card->pci_dev))
			frame->dma = 0;
	}

	spin_unlock_irqrestore(&frame->spinlock, flags);

	return frame;
}

//Returns 0 on failure. 1 on success
int fscc_frame_setup_descriptors(struct fscc_frame *frame,
								 struct pci_dev *pci_dev)
{
	frame->d1 = kmalloc(sizeof(*frame->d1), GFP_ATOMIC | GFP_DMA);

	if (!frame->d1)
		return 0;

	frame->d2 = kmalloc(sizeof(*frame->d2), GFP_ATOMIC | GFP_DMA);

	if (!frame->d2) {
		kfree(frame->d1);
		return 0;
	}

	memset(frame->d1, 0, sizeof(*frame->d1));
	memset(frame->d2, 0, sizeof(*frame->d2));

	frame->d1_handle = pci_map_single(pci_dev, frame->d1, sizeof(*frame->d1),
									  DMA_TO_DEVICE);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)
	if (dma_mapping_error(&pci_dev->dev, frame->d1_handle)) {
#else
	if (dma_mapping_error(frame->d1_handle)) {
#endif
		dev_err(frame->port->device, "dma_mapping_error failed\n");

		kfree(frame->d1);
		kfree(frame->d2);

		frame->d1 = 0;
		frame->d2 = 0;

		return 0;
	}

	frame->d2_handle = pci_map_single(pci_dev, frame->d2, sizeof(*frame->d2),
									  DMA_TO_DEVICE);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)
	if (dma_mapping_error(&pci_dev->dev, frame->d2_handle)) {
#else
	if (dma_mapping_error(frame->d2_handle)) {
#endif
		dev_err(frame->port->device, "dma_mapping_error failed\n");

		pci_unmap_single(frame->port->card->pci_dev, frame->d1_handle,
						 sizeof(*frame->d1), DMA_TO_DEVICE);

		kfree(frame->d1);
		kfree(frame->d2);

		frame->d1 = 0;
		frame->d2 = 0;

		return 0;
	}

	frame->d2->control = 0x40000000;
	frame->d1->next_descriptor = cpu_to_le32(frame->d2_handle);

	return 1;
}

void fscc_frame_delete(struct fscc_frame *frame)
{
	unsigned long flags = 0;

	return_if_untrue(frame);

	spin_lock_irqsave(&frame->spinlock, flags);

	if (frame->dma) {
		pci_unmap_single(frame->port->card->pci_dev, frame->d1_handle,
						 sizeof(*frame->d1), DMA_TO_DEVICE);

		pci_unmap_single(frame->port->card->pci_dev, frame->d2_handle,
						 sizeof(*frame->d2), DMA_TO_DEVICE);

		if (frame->data_handle && frame->data_length) {
			pci_unmap_single(frame->port->card->pci_dev, frame->data_handle,
							 frame->data_length, DMA_TO_DEVICE);
		}

		if (frame->d1)
			kfree(frame->d1);

		if (frame->d2)
			kfree(frame->d2);
	}

	fscc_frame_update_buffer_size(frame, 0);

	spin_unlock_irqrestore(&frame->spinlock, flags);

	kfree(frame);
}

unsigned fscc_frame_get_length(struct fscc_frame *frame)
{
	return_val_if_untrue(frame, 0);

	return frame->data_length;
}

unsigned fscc_frame_get_buffer_size(struct fscc_frame *frame)
{
	return_val_if_untrue(frame, 0);

	return frame->buffer_size;
}

unsigned fscc_frame_is_empty(struct fscc_frame *frame)
{
	return_val_if_untrue(frame, 0);

	return frame->data_length == 0;
}

int fscc_frame_add_data(struct fscc_frame *frame, const char *data,
						 unsigned length)
{
	unsigned long flags = 0;

	return_val_if_untrue(frame, 0);
	return_val_if_untrue(length > 0, 0);

	spin_lock_irqsave(&frame->spinlock, flags);

	if (frame->dma && frame->buffer) {
		pci_unmap_single(frame->port->card->pci_dev, frame->data_handle,
						 frame->data_length, DMA_TO_DEVICE);
	}

	/* Only update buffer size if there isn't enough space already */
	if (frame->data_length + length > frame->buffer_size) {
		if (fscc_frame_update_buffer_size(frame, frame->data_length + length) == 0) {
			spin_unlock_irqrestore(&frame->spinlock, flags);
			return 0;
		}
	}

	/* Copy the new data to the end of the frame */
	memmove(frame->buffer + frame->data_length, data, length);

	frame->data_length += length;

	if (frame->dma && frame->buffer) {
		frame->data_handle = pci_map_single(frame->port->card->pci_dev,
											frame->buffer,
											frame->data_length,
											DMA_TO_DEVICE);

		frame->d1->control = 0xA0000000 | frame->data_length;
		frame->d1->data_address = cpu_to_le32(frame->data_handle);
		frame->d1->data_count = frame->data_length;
	}

	spin_unlock_irqrestore(&frame->spinlock, flags);

	return 1;
}

int fscc_frame_add_data_from_port(struct fscc_frame *frame, struct fscc_port *port,
						 unsigned length)
{
	unsigned long flags = 0;

	return_val_if_untrue(frame, 0);
	return_val_if_untrue(length > 0, 0);

	spin_lock_irqsave(&frame->spinlock, flags);

	if (frame->dma && frame->buffer) {
		pci_unmap_single(frame->port->card->pci_dev, frame->data_handle,
						 frame->data_length, DMA_TO_DEVICE);
	}

	/* Only update buffer size if there isn't enough space already */
	if (frame->data_length + length > frame->buffer_size) {
		if (fscc_frame_update_buffer_size(frame, frame->data_length + length) == 0) {
			spin_unlock_irqrestore(&frame->spinlock, flags);
			return 0;
		}
	}

	/* Copy the new data to the end of the frame */
	fscc_port_get_register_rep(port, 0, FIFO_OFFSET, frame->buffer + frame->data_length, length);

	frame->data_length += length;

	if (frame->dma && frame->buffer) {
		frame->data_handle = pci_map_single(frame->port->card->pci_dev,
											frame->buffer,
											frame->data_length,
											DMA_TO_DEVICE);

		frame->d1->control = 0xA0000000 | frame->data_length;
		frame->d1->data_address = cpu_to_le32(frame->data_handle);
		frame->d1->data_count = frame->data_length;
	}

	spin_unlock_irqrestore(&frame->spinlock, flags);

	return 1;
}

int fscc_frame_remove_data(struct fscc_frame *frame, char *destination,
							unsigned length)
{
	unsigned long flags = 0;

	return_val_if_untrue(frame, 0);

	if (length == 0)
		return 1;

	spin_lock_irqsave(&frame->spinlock, flags);

	if (frame->data_length == 0) {
		dev_warn(frame->port->device, "attempting data removal from empty frame\n");
		spin_unlock_irqrestore(&frame->spinlock, flags);
		return 1;
	}

	/* Make sure we don't remove more data than we have */
	if (length > frame->data_length) {
		dev_warn(frame->port->device, "attempting removal of more data than available\n"); 
		spin_unlock_irqrestore(&frame->spinlock, flags);
		return 0;
	}

	/* Copy the data into the outside buffer */
	if (destination)
		copy_to_user(destination, frame->buffer, length);

	frame->data_length -= length;

	/* Move the data up in the buffer (essentially removing the old data) */
	memmove(frame->buffer, frame->buffer + length, frame->data_length);

	spin_unlock_irqrestore(&frame->spinlock, flags);

	return 1;
}

//TODO: This could cause an issue w here data_length is less before it makes it into remove_Data
void fscc_frame_clear(struct fscc_frame *frame)
{
	fscc_frame_remove_data(frame, NULL, frame->data_length);
}

void fscc_frame_trim(struct fscc_frame *frame)
{
	return_if_untrue(frame);

	fscc_frame_update_buffer_size(frame, frame->data_length);
}

int fscc_frame_update_buffer_size(struct fscc_frame *frame, unsigned size)
{
	char *new_buffer = 0;
	int malloc_flags = 0;

	return_val_if_untrue(frame, 0);

	if (size == 0) {
		if (frame->buffer) {
			kfree(frame->buffer);
			frame->buffer = 0;
		}

		frame->buffer_size = 0;
		frame->data_length = 0;

		return 1;
	}

	malloc_flags |= GFP_ATOMIC;

	new_buffer = kmalloc(size, malloc_flags);

	if (new_buffer == NULL) {
		dev_err(frame->port->device, "not enough memory to update frame buffer size\n");
		return 0;
	}

	memset(new_buffer, 0, size);

	if (frame->buffer) {
		if (frame->data_length) {
			/* Truncate data length if the new buffer size is less than the data length */
			frame->data_length = min(frame->data_length, size);

			/* Copy over the old buffer data to the new buffer */
			memmove(new_buffer, frame->buffer, frame->data_length);
		}

		kfree(frame->buffer);
	}

	frame->buffer = new_buffer;
	frame->buffer_size = size;

	return 1;
}

