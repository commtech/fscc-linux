/*
	Copyright (C) 2011 Commtech, Inc.

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

	if (isr_value & TFT && !fscc_port_has_dma(port))
		tasklet_schedule(&port->oframe_tasklet);

	/* We have to wait until an ALLS to delete a DMA frame because if we
	   delete the frame right away the DMA engine will lose the data to
	   transfer. */
	if (fscc_port_has_dma(port) && isr_value & ALLS) {
		if (port->pending_oframe) {
			fscc_frame_delete(port->pending_oframe);
			port->pending_oframe = 0;
		}

		tasklet_schedule(&port->oframe_tasklet);
	}

#ifdef DEBUG
	tasklet_schedule(&port->print_tasklet);
	fscc_port_increment_interrupt_counts(port, isr_value);
#endif

	return IRQ_HANDLED;
}

void iframe_worker(unsigned long data)
{
	struct fscc_port *port = 0;
	int receive_length = 0; /* Needs to be signed */
	unsigned finished_frame = 0;
	unsigned long flags = 0;

	port = (struct fscc_port *)data;

	return_if_untrue(port);

	spin_lock_irqsave(&port->iframe_spinlock, flags);

	if (!port->pending_iframe) {
		//port->pending_iframe = fscc_frame_new(0, fscc_port_has_dma(port), port);
		port->pending_iframe = fscc_frame_new(0, 0, port);

		if (!port->pending_iframe) {
			spin_unlock_irqrestore(&port->iframe_spinlock, flags);
			return;
		}
	}

	finished_frame = (fscc_port_get_RFCNT(port) > 0) ? 1 : 0;

	if (finished_frame) {
		unsigned bc_fifo_l = 0;
		unsigned current_length = 0;

		bc_fifo_l = fscc_port_get_register(port, 0, BC_FIFO_L_OFFSET);
		current_length = fscc_frame_get_current_length(port->pending_iframe);

		receive_length = bc_fifo_l - current_length;
	} else {
		unsigned rxcnt = 0;

		rxcnt = fscc_port_get_RXCNT(port);

		/* We choose a safe amount to read due to more data coming in after we
		   get our values. The rest will be read on the next interrupt. */
		receive_length = rxcnt - STATUS_LENGTH - MAX_LEFTOVER_BYTES;
		receive_length -= receive_length % 4;
	}

	if (receive_length > 0) {
		char *buffer = 0;

		/* Make sure we don't go over the user's memory constraint. */
		if (fscc_port_get_input_memory_usage(port, 0) + receive_length > fscc_port_get_input_memory_cap(port)) {
			dev_warn(port->device, "F#%i rejected (memory constraint)\n",
					 port->pending_iframe->number);

			fscc_frame_delete(port->pending_iframe);
			port->pending_iframe = 0;

			spin_unlock_irqrestore(&port->iframe_spinlock, flags);
			return;
		}

		buffer = kmalloc(receive_length, GFP_ATOMIC);

		/* Make sure the kernel gives us enough memory to receive the data. */
		if (buffer == NULL) {
			dev_warn(port->device, "F#%i rejected (kmalloc of %i bytes)\n",
					 port->pending_iframe->number, receive_length);

			fscc_frame_delete(port->pending_iframe);
			port->pending_iframe = 0;

			spin_unlock_irqrestore(&port->iframe_spinlock, flags);
			return;
		}

		fscc_port_get_register_rep(port, 0, FIFO_OFFSET, buffer, receive_length);
		fscc_frame_add_data(port->pending_iframe, buffer, receive_length);

		kfree(buffer);

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
	}

	if (!finished_frame) {
		spin_unlock_irqrestore(&port->iframe_spinlock, flags);
		return;
	}

	fscc_frame_trim(port->pending_iframe);
	list_add_tail(&port->pending_iframe->list, &port->iframes);

	wake_up_interruptible(&port->input_queue);

	port->pending_iframe = 0;

	spin_unlock_irqrestore(&port->iframe_spinlock, flags);
}

void istream_worker(unsigned long data)
{
	struct fscc_port *port = 0;
	int receive_length = 0; /* Needs to be signed */
	unsigned long flags = 0;

	port = (struct fscc_port *)data;

	return_if_untrue(port);

	spin_lock_irqsave(&port->iframe_spinlock, flags);

	receive_length = fscc_port_get_RXCNT(port);

	if (receive_length > 0) {
		char *buffer = 0;

		/* Make sure we don't go over the user's memory constraint. */
		if (fscc_port_get_input_memory_usage(port, 0) + receive_length > fscc_port_get_input_memory_cap(port)) {
			dev_warn(port->device, "Stream rejected (memory constraint)\n");

			spin_unlock_irqrestore(&port->iframe_spinlock, flags);
			return;
		}

		buffer = kmalloc(receive_length, GFP_ATOMIC);

		/* Make sure the kernel gives us enough memory to receive the data. */
		if (buffer == NULL) {
			dev_warn(port->device,
					 "Stream receive rejected (kmalloc of %i bytes)\n",
					 receive_length);


            spin_unlock_irqrestore(&port->iframe_spinlock, flags);
			return;
		}

		fscc_port_get_register_rep(port, 0, FIFO_OFFSET, buffer,
								   receive_length);

		fscc_stream_add_data(port->istream, buffer, receive_length);

		kfree(buffer);

#ifdef TODO_ADD_ENDIAN_CODE
#ifdef __BIG_ENDIAN
		{
			char status[STATUS_LENGTH];

			/* Moves the status bytes to the end. */
			memmove(&status, port->pending_iframe->data, STATUS_LENGTH);
			memmove(port->pending_iframe->data, port->pending_iframe->data + STATUS_LENGTH, port->pending_iframe->current_length - STATUS_LENGTH);
			memmove(port->pending_iframe->data + port->pending_iframe->current_length - STATUS_LENGTH, &status, STATUS_LENGTH);
		}
#endif
#endif

		dev_dbg(port->device, "Stream <= %i byte%s\n", receive_length,
				(receive_length == 1) ? "" : "s");
	}

	wake_up_interruptible(&port->input_queue);

	spin_unlock_irqrestore(&port->iframe_spinlock, flags);
}

#include "card.h"

void oframe_worker(unsigned long data)
{
	struct fscc_port *port = 0;

	unsigned fifo_space = 0;
	unsigned current_length = 0;
	unsigned target_length = 0;
	unsigned transmit_length = 0;
	unsigned size_in_fifo = 0;

	unsigned long flags = 0;

	port = (struct fscc_port *)data;

	return_if_untrue(port);

	spin_lock_irqsave(&port->oframe_spinlock, flags);

	/* Check if exists and if so, grabs the frame to transmit. */
	if (!port->pending_oframe) {
		if (fscc_port_has_oframes(port, 0)) {
			port->pending_oframe = fscc_port_peek_front_frame(port, &port->oframes);
			list_del(&port->pending_oframe->list);
		} else {
			spin_unlock_irqrestore(&port->oframe_spinlock, flags);
			return;
		}
	}

	if (fscc_port_has_dma(port)) {
		dma_addr_t *d1_handle = 0;

		if (port->pending_oframe->handled) {
			spin_unlock_irqrestore(&port->oframe_spinlock, flags);
			return;
		}

		dev_dbg(port->device, "F#%i => %i byte%s%s\n",
				port->pending_oframe->number, fscc_frame_get_current_length(port->pending_oframe),
				(fscc_frame_get_current_length(port->pending_oframe) == 1) ? "" : "s",
				(fscc_frame_is_empty(port->pending_oframe)) ? " (starting)" : "");

		d1_handle = &port->pending_oframe->d1_handle;

		fscc_port_set_register(port, 2, DMA_TX_BASE_OFFSET, *d1_handle);
		fscc_port_execute_GO_T(port);

		/* TODO: Add a prettier way of doing this than manually editing the
		   frame structure. */
		port->pending_oframe->handled = 1;

		spin_unlock_irqrestore(&port->oframe_spinlock, flags);
		return;
	}

	current_length = fscc_frame_get_current_length(port->pending_oframe);
	target_length = fscc_frame_get_target_length(port->pending_oframe);
	size_in_fifo = current_length + (4 - current_length % 4);

	/* Subtracts 1 so a TDO overflow doesn't happen on the 4096th byte. */
	fifo_space = TX_FIFO_SIZE - fscc_port_get_TXCNT(port) - 1;
	fifo_space -= fifo_space % 4;

	/* Determine the maximum amount of data we can send this time around. */
	transmit_length = (size_in_fifo > fifo_space) ? fifo_space : current_length;

	if (transmit_length == 0) {
		spin_unlock_irqrestore(&port->oframe_spinlock, flags);
		return;
	}

	fscc_port_set_register_rep(port, 0, FIFO_OFFSET,
							   port->pending_oframe->data,
							   transmit_length);

	fscc_frame_remove_data(port->pending_oframe, transmit_length);

	dev_dbg(port->device, "F#%i => %i byte%s%s\n",
			port->pending_oframe->number, transmit_length,
			(transmit_length == 1) ? "" : "s",
			(fscc_frame_is_empty(port->pending_oframe)) ? " (starting)" : "");

	/* If this is the first time we add data to the FIFO for this frame we
	   tell the port how much data is in this frame. */
	if (current_length == target_length)
		fscc_port_set_register(port, 0, BC_FIFO_L_OFFSET, target_length);

	/* If we have sent all of the data we clean up. */
	if (fscc_frame_is_empty(port->pending_oframe)) {
		fscc_frame_delete(port->pending_oframe);
		port->pending_oframe = 0;
		wake_up_interruptible(&port->output_queue);
	}

	fscc_port_execute_XF(port);

	spin_unlock_irqrestore(&port->oframe_spinlock, flags);
}

