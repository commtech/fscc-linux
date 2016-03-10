/*
	Copyright (C) 2016 Commtech, Inc.

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

#include "isr.h"
#include "port.h" /* struct fscc_port */
#include "card.h" /* struct fscc_port */
#include "utils.h" /* port_exists */
#include "frame.h" /* struct fscc_frame */

#define TX_FIFO_SIZE 4096
#define MAX_LEFTOVER_BYTES 3

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 19)
irqreturn_t fscc_isr(int irq, void *potential_port)
#else
irqreturn_t fscc_isr(int irq, void *potential_port, struct pt_regs *regs)
#endif
{
	struct fscc_port *port = 0;
	unsigned isr_value = 0;
	unsigned streaming = 0;

	if (!port_exists(potential_port))
		return IRQ_NONE;

	port = (struct fscc_port *)potential_port;

	isr_value = fscc_port_get_register(port, 0, ISR_OFFSET);

	if (!isr_value)
		return IRQ_NONE;

	if (timer_pending(&port->timer))
		del_timer(&port->timer);

	port->last_isr_value |= isr_value;
	streaming = fscc_port_is_streaming(port);

	if (streaming) {
		if (isr_value & (RFT | RFS))
			tasklet_schedule(&port->istream_tasklet);
	}
	else {
		if (isr_value & (RFE | RFT | RFS))
			tasklet_schedule(&port->iframe_tasklet);
	}

	if (isr_value & TFT)
		tasklet_schedule(&port->send_oframe_tasklet);

	if (isr_value & ALLS)
		tasklet_schedule(&port->clear_oframe_tasklet);

#ifdef DEBUG
	tasklet_schedule(&port->print_tasklet);
	fscc_port_increment_interrupt_counts(port, isr_value);
#endif

	fscc_port_reset_timer(port);

	return IRQ_HANDLED;
}

void iframe_worker(unsigned long data)
{
	struct fscc_port *port = 0;
	int receive_length = 0; /* Needs to be signed */
	unsigned finished_frame = 0;
	unsigned long board_flags = 0;
	unsigned long frame_flags = 0;
	unsigned long queued_flags = 0;
	static int rejected_last_frame = 0;
	unsigned current_memory = 0;
	unsigned memory_cap = 0;
	unsigned rfcnt = 0;

	port = (struct fscc_port *)data;

	return_if_untrue(port);

	do {
		current_memory = fscc_port_get_input_memory_usage(port);
		memory_cap = fscc_port_get_input_memory_cap(port);

		spin_lock_irqsave(&port->board_rx_spinlock, board_flags);
		spin_lock_irqsave(&port->pending_iframe_spinlock, frame_flags);

		rfcnt = fscc_port_get_RFCNT(port);
		finished_frame = (rfcnt > 0) ? 1 : 0;

		if (finished_frame) {
			unsigned bc_fifo_l = 0;
			unsigned current_length = 0;

			bc_fifo_l = fscc_port_get_register(port, 0, BC_FIFO_L_OFFSET);

			if (port->pending_iframe)
				current_length = fscc_frame_get_length(port->pending_iframe);

			receive_length = bc_fifo_l - current_length;
		} else {
			unsigned rxcnt = 0;

			// Refresh rfcnt, if the frame is now complete, reloop through
			// This prevents the situation where the frame has finished being
			// received between the original rfcnt and now, causing a possible
			// read of a larger byte count than in the actual frame due to
			// another incoming frame
			rxcnt = fscc_port_get_RXCNT(port);
			rfcnt = fscc_port_get_RFCNT(port);

			if (rfcnt) {
				spin_unlock_irqrestore(&port->pending_iframe_spinlock, frame_flags);
				spin_unlock_irqrestore(&port->board_rx_spinlock, board_flags);
				break;
			}

			/* We choose a safe amount to read due to more data coming in after we
			   get our values. The rest will be read on the next interrupt. */
			receive_length = max((int)(rxcnt - rxcnt % 4), (int)0);
		}

		if (receive_length <= 0) {
			spin_unlock_irqrestore(&port->pending_iframe_spinlock, frame_flags);
			spin_unlock_irqrestore(&port->board_rx_spinlock, board_flags);
			return;
		}

		/* Make sure we don't go over the user's memory constraint. */
		if (current_memory + receive_length > memory_cap) {
			if (rejected_last_frame == 0) {
				dev_warn(port->device,
						 "Rejecting frames (memory constraint)\n");
				rejected_last_frame = 1; /* Track that we dropped a frame so we
								don't have to warn the user again. */
			}

			if (port->pending_iframe) {
				fscc_frame_delete(port->pending_iframe);
				port->pending_iframe = 0;
			}

			spin_unlock_irqrestore(&port->pending_iframe_spinlock, frame_flags);
			spin_unlock_irqrestore(&port->board_rx_spinlock, board_flags);
			return;
		}

		if (!port->pending_iframe) {
			port->pending_iframe = fscc_frame_new(port);

			if (!port->pending_iframe) {
				spin_unlock_irqrestore(&port->pending_iframe_spinlock, frame_flags);
				spin_unlock_irqrestore(&port->board_rx_spinlock, board_flags);
				return;
			}
		}

		fscc_frame_add_data_from_port(port->pending_iframe, port, receive_length);

	#ifdef __BIG_ENDIAN
		{
			char status[STATUS_LENGTH];

			/* Moves the status bytes to the end. */
			memmove(&status, port->pending_iframe->data, STATUS_LENGTH);
			memmove(port->pending_iframe->data, port->pending_iframe->data + STATUS_LENGTH, port->pending_iframe->current_length - STATUS_LENGTH);
			memmove(port->pending_iframe->data + port->pending_iframe->current_length - STATUS_LENGTH, &status, STATUS_LENGTH);
		}
	#endif

		dev_dbg(port->device, "F#%i <= %i byte%s (%sfinished)\n",
				port->pending_iframe->number, receive_length,
				(receive_length == 1) ? "" : "s",
				(finished_frame) ? "" : "un");

		if (!finished_frame) {
			spin_unlock_irqrestore(&port->pending_iframe_spinlock, frame_flags);
			spin_unlock_irqrestore(&port->board_rx_spinlock, board_flags);
			return;
		}

		if (port->pending_iframe) {
#ifdef RELEASE_PREVIEW
			getnstimeofday(&port->pending_iframe->timestamp);
#else
			do_gettimeofday(&port->pending_iframe->timestamp);
#endif
			spin_lock_irqsave(&port->queued_iframes_spinlock, queued_flags);
			fscc_flist_add_frame(&port->queued_iframes, port->pending_iframe);
			spin_unlock_irqrestore(&port->queued_iframes_spinlock, queued_flags);
		}

		rejected_last_frame = 0; /* Track that we received a frame to reset the
									memory constraint warning print message. */

		port->pending_iframe = 0;

		spin_unlock_irqrestore(&port->pending_iframe_spinlock, frame_flags);
	    spin_unlock_irqrestore(&port->board_rx_spinlock, board_flags);

		wake_up_interruptible(&port->input_queue);
	}
	while (receive_length);
}

void istream_worker(unsigned long data)
{
	struct fscc_port *port = 0;
	int receive_length = 0; /* Needs to be signed */
	unsigned rxcnt = 0;
	unsigned long board_flags = 0;
	unsigned long stream_flags = 0;
	unsigned current_memory = 0;
	unsigned memory_cap = 0;
	static int rejected_last_stream = 0;
	unsigned status;

	port = (struct fscc_port *)data;

	return_if_untrue(port);

	current_memory = fscc_port_get_input_memory_usage(port);
	memory_cap = fscc_port_get_input_memory_cap(port);

	/* Leave the interrupt handler if we are at our memory cap. */
	if (current_memory >= memory_cap) {
		if (rejected_last_stream == 0) {
			dev_warn(port->device, "Rejecting stream (memory constraint)\n");

			rejected_last_stream = 1; /* Track that we dropped stream data so we
									 don't have to warn the user again. */
		}

		return;
	}

	spin_lock_irqsave(&port->board_rx_spinlock, board_flags);

	rxcnt = fscc_port_get_RXCNT(port);

	/* We choose a safe amount to read due to more data coming in after we
	   get our values. The rest will be read on the next interrupt. */
	receive_length = rxcnt - MAX_LEFTOVER_BYTES;
	receive_length -= receive_length % 4;

	/* Leave the interrupt handler if there is no data to read. */
	if (receive_length <= 0) {
		spin_unlock_irqrestore(&port->board_rx_spinlock, board_flags);
		return;
	}

	/* Trim the amount to read if there isn't enough memory space to read all
	   of it. */
	if (receive_length + current_memory > memory_cap)
		receive_length = memory_cap - current_memory;

	spin_lock_irqsave(&port->istream_spinlock, stream_flags);
	status = fscc_frame_add_data_from_port(port->istream, port, receive_length);
	spin_unlock_irqrestore(&port->istream_spinlock, stream_flags);
	if (status == 0) {
		dev_err(port->device, "Error adding stream data");
		spin_unlock_irqrestore(&port->board_rx_spinlock, board_flags);
		return;
	}

	spin_unlock_irqrestore(&port->board_rx_spinlock, board_flags);

	rejected_last_stream = 0; /* Track that we received stream data to reset
								 the memory constraint warning print message.
							  */

	dev_dbg(port->device, "Stream <= %i byte%s\n", receive_length,
			(receive_length == 1) ? "" : "s");

	wake_up_interruptible(&port->input_queue);
}

void clear_oframe_worker(unsigned long data)
{
	struct fscc_port *port = 0;
	struct fscc_frame *frame = 0;
	unsigned remove = 0;
	unsigned long sent_flags = 0;

	port = (struct fscc_port *)data;

	spin_lock_irqsave(&port->sent_oframes_spinlock, sent_flags);

	frame = fscc_flist_peek_front(&port->sent_oframes);

	if (!frame) {
		spin_unlock_irqrestore(&port->sent_oframes_spinlock, sent_flags);
		return;
	}

	if (fscc_frame_is_dma(frame)) {
		if (frame->d1->control == 0x40000000)
			remove = 1;
	}
	else {
		remove = 1;
	}

	if (remove) {
		fscc_flist_remove_frame(&port->sent_oframes);
		fscc_frame_delete(frame);

		if (!fscc_flist_is_empty(&port->sent_oframes))
		    tasklet_schedule(&port->clear_oframe_tasklet);
	}

	spin_unlock_irqrestore(&port->sent_oframes_spinlock, sent_flags);
}

void oframe_worker(unsigned long data)
{
	struct fscc_port *port = 0;
	int result;

	unsigned long board_flags = 0;
	unsigned long frame_flags = 0;
	unsigned long sent_flags = 0;
	unsigned long queued_flags = 0;

	port = (struct fscc_port *)data;

	return_if_untrue(port);

	spin_lock_irqsave(&port->board_tx_spinlock, board_flags);
	spin_lock_irqsave(&port->pending_oframe_spinlock, frame_flags);

	/* Check if exists and if so, grabs the frame to transmit. */
	if (!port->pending_oframe) {
		spin_lock_irqsave(&port->queued_oframes_spinlock, queued_flags);
		port->pending_oframe = fscc_flist_remove_frame(&port->queued_oframes);
		spin_unlock_irqrestore(&port->queued_oframes_spinlock, queued_flags);

		/* No frames in queue to transmit */
		if (!port->pending_oframe) {
			spin_unlock_irqrestore(&port->pending_oframe_spinlock, frame_flags);
			spin_unlock_irqrestore(&port->board_tx_spinlock, board_flags);
			return;
		}
	}

	result = fscc_port_transmit_frame(port, port->pending_oframe);

	if (result == 2) {
		spin_lock_irqsave(&port->sent_oframes_spinlock, sent_flags);
		fscc_flist_add_frame(&port->sent_oframes, port->pending_oframe);
		spin_unlock_irqrestore(&port->sent_oframes_spinlock, sent_flags);

		port->pending_oframe = 0;
	}

	spin_unlock_irqrestore(&port->pending_oframe_spinlock, frame_flags);
	spin_unlock_irqrestore(&port->board_tx_spinlock, board_flags);

	if (result == 2)
		wake_up_interruptible(&port->output_queue);
}

void timer_handler(unsigned long data)
{
	struct fscc_port *port = (struct fscc_port *)data;
	unsigned streaming = 0;

	streaming = fscc_port_is_streaming(port);

	if (streaming)
		tasklet_schedule(&port->istream_tasklet);
	else
		tasklet_schedule(&port->iframe_tasklet);
}
