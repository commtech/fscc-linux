/*
	Copyright (c) 2019 Commtech, Inc.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include "debug.h"
#include "utils.h" /* return_{val_}if_true */

struct debug_interrupt_tracker *debug_interrupt_tracker_new(void)
{
	struct debug_interrupt_tracker *tracker = 0;

	tracker = kmalloc(sizeof(*tracker), GFP_KERNEL);

	memset(tracker, 0, sizeof(*tracker));

	return tracker;
}

void debug_interrupt_tracker_delete(struct debug_interrupt_tracker *tracker)
{
	return_if_untrue(tracker);

	kfree(tracker);
}

void debug_interrupt_tracker_increment_all(struct debug_interrupt_tracker *tracker,
									       __u32 isr_value)
{
	unsigned i = 0;

	return_if_untrue(tracker);

	if (isr_value == 0)
		return;

	for (i = 0; i < sizeof(*tracker) / sizeof(unsigned); i++) {
		if (isr_value & 0x00000001)
			(*((unsigned *)tracker + i))++;

		isr_value >>= 1;
	}
}

unsigned debug_interrupt_tracker_get_count(struct debug_interrupt_tracker *tracker,
										   __u32 isr_bit)
{
	unsigned i = 0;

	return_val_if_untrue(tracker, 0);
	return_val_if_untrue(isr_bit != 0, 0);

	isr_bit >>= 1;

	while (isr_bit) {
		isr_bit >>= 1;
		i++;
	}

	return *((unsigned *)tracker + i);
}

void debug_interrupt_display(unsigned long data)
{
	struct fscc_port *port = 0;
	unsigned isr_value = 0;

	port = (struct fscc_port *)data;

	if (!port_exists(port))
		return;

	isr_value = port->last_isr_value;
	port->last_isr_value = 0;

	dev_dbg(port->device, "interrupt: 0x%08x\n", isr_value);

	if (isr_value & RFE)
		dev_dbg(port->device, "RFE (Receive Frame End Interrupt)\n");

	if (isr_value & RFT)
		dev_dbg(port->device, "RFT (Receive FIFO Trigger Interrupt)\n");

	if (isr_value & RFS)
		dev_dbg(port->device, "RFS (Receive Frame Start Interrupt)\n");

	if (isr_value & RFO)
		dev_dbg(port->device, "RFO (Receive Frame Overflow Interrupt)\n");

	if (isr_value & RDO)
		dev_dbg(port->device, "RDO (Receive Data Overflow Interrupt)\n");

	if (isr_value & RFL)
		dev_dbg(port->device, "RFL (Receive Frame Lost Interrupt)\n");

	if (isr_value & TIN)
		dev_dbg(port->device, "TIN (Timer Expiration Interrupt)\n");

	if (isr_value & TFT)
		dev_dbg(port->device, "TFT (Transmit FIFO Trigger Interrupt)\n");

	if (isr_value & TDU)
		dev_dbg(port->device, "TDU (Transmit Data Underrun Interrupt)\n");

	if (isr_value & ALLS)
		dev_dbg(port->device, "ALLS (All Sent Interrupt)\n");

	if (isr_value & CTSS)
		dev_dbg(port->device, "CTSS (CTS State Change Interrupt)\n");

	if (isr_value & DSRC)
		dev_dbg(port->device, "DSRC (DSR Change Interrupt)\n");

	if (isr_value & CDC)
		dev_dbg(port->device, "CDC (CD Change Interrupt)\n");

	if (isr_value & DT_STOP)
		dev_dbg(port->device, "DT_STOP (DMA Transmitter Full Stop indication)\n");

	if (isr_value & DR_STOP)
		dev_dbg(port->device, "DR_STOP (DMA Receiver Full Stop indication)\n");

	if (isr_value & DT_FE)
		dev_dbg(port->device, "DT_FE (DMA Transmit Frame End indication)\n");

	if (isr_value & DR_FE)
		dev_dbg(port->device, "DR_FE (DMA Receive Frame End indication)\n");

	if (isr_value & DT_HI)
		dev_dbg(port->device, "DT_HI (DMA Transmit Host Interrupt indication)\n");

	if (isr_value & DR_HI)
		dev_dbg(port->device, "DR_HI (DMA Receive Host Interrupt indication)\n");
}
