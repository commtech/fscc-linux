/*
        Copyright (c) 2024 Commtech, Inc.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to
   deal in the Software without restriction, including without limitation the
   rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
   sell copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
   IN THE SOFTWARE.
*/

#include "isr.h"
#include "card.h" /* struct fscc_port */
#include "io.h"
#include "port.h"  /* struct fscc_port */
#include "utils.h" /* port_exists */

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
  unsigned using_dma = 0;

  // My current DMA implementation seems to cause an interrupt storm
  // after the DMA buffers are deleted when it really shouldn't.
  // This, in turn, causes an error to the system which thinks there's a bug.
  if (!port_exists(potential_port))
    return IRQ_NONE;

  port = (struct fscc_port *)potential_port;

  isr_value = fscc_port_get_register(port, 0, ISR_OFFSET);

  if (!isr_value)
    return IRQ_NONE;

  if (timer_pending(&port->timer))
    del_timer(&port->timer);

  port->last_isr_value |= isr_value;

  using_dma = fscc_port_is_dma(port);

  if (using_dma) {
    if (isr_value & (DR_HI | DR_FE | RFT | RFS | DR_STOP | RFE))
      tasklet_schedule(&port->timestamp_tasklet);
  } else {
    if (isr_value & (RFE | RFT | RFS | RFO | RDO))
      tasklet_schedule(&port->iframe_tasklet);

    if (isr_value & (TFT | TDU | ALLS))
      tasklet_schedule(&port->send_oframe_tasklet);
  }

#ifdef DEBUG
  tasklet_schedule(&port->print_tasklet);
  fscc_port_increment_interrupt_counts(port, isr_value);
#endif
  wake_up_interruptible(&port->status_queue);
  fscc_port_reset_timer(port);

  return IRQ_HANDLED;
}

void timestamp_worker(unsigned long data) {
  struct fscc_port *port = 0;

  port = (struct fscc_port *)data;
  return_if_untrue(port);

  fscc_io_apply_timestamps(port);

  if (fscc_io_user_read_ready(port))
    wake_up_interruptible(&port->input_queue);
}

void iframe_worker(unsigned long data) {
  struct fscc_port *port = 0;

  port = (struct fscc_port *)data;
  return_if_untrue(port);

  if (fscc_port_is_dma(port))
    return;

  fscc_fifo_read_data(port);

  if (fscc_io_user_read_ready(port))
    wake_up_interruptible(&port->input_queue);
}

void oframe_worker(unsigned long data) {
  struct fscc_port *port = 0;

  port = (struct fscc_port *)data;
  return_if_untrue(port);

  if (fscc_port_is_dma(port))
    return;
  fscc_io_transmit_frame(port);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)
void timer_handler(struct timer_list *data) {
  struct fscc_port *port = from_timer(port, data, timer);
#else
void timer_handler(unsigned long data) {
  struct fscc_port *port = (struct fscc_port *)data;
#endif

  if (fscc_port_is_dma(port)) {
    if (fscc_io_user_read_ready(port))
      wake_up_interruptible(&port->input_queue);
  } else
    tasklet_schedule(&port->iframe_tasklet);
}
