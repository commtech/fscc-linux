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

#include "io.h"
#include "card.h"
#include "config.h"
#include "port.h"
#include "utils.h"

void clear_timestamp(fscc_timestamp *timestamp);
int timestamp_is_empty(fscc_timestamp *timestamp);
int fscc_io_create_rx_frames(struct fscc_port *port, size_t number_of_frames);
int fscc_io_create_tx_frames(struct fscc_port *port, size_t number_of_frames);
void fscc_io_destroy_rx_frames(struct fscc_port *port);
void fscc_io_destroy_tx_frames(struct fscc_port *port);
int fscc_dma_is_rx_running(struct fscc_port *port);
int fscc_dma_is_tx_running(struct fscc_port *port);
void fscc_dma_execute_GO_R(struct fscc_port *port);
void fscc_dma_execute_GO_T(struct fscc_port *port);
void fscc_io_execute_transmit(struct fscc_port *port, unsigned dma);
int fscc_fifo_write_data(struct fscc_port *port);

// Even if our card does not do DMA, we will use the DMA tools universally.
// We'll just use them for normal memory purposes.
int fscc_io_initialize(struct fscc_port *port, size_t rx_buffer_size,
                       size_t tx_buffer_size) {
  int status = 0;

  port->desc_pool =
      dma_pool_create("fscc_descriptors", &port->card->pci_dev->dev,
                      sizeof(struct fscc_descriptor), 4, 0);
  if (port->desc_pool == NULL) {
    dev_warn(port->device, "desc_pool: FAILED\n");
    return -ENOMEM;
  }

  port->rx_buffer_pool = dma_pool_create(
      "rx_buffers", &port->card->pci_dev->dev, rx_buffer_size, 4, 0);
  if (port->rx_buffer_pool == NULL) {
    port->memory.RxSize = 0;
    dev_warn(port->device, "rx_pool: FAILED\n");
    return -ENOMEM;
  }
  port->memory.RxSize = rx_buffer_size;

  port->tx_buffer_pool = dma_pool_create(
      "tx_buffers", &port->card->pci_dev->dev, tx_buffer_size, 4, 0);
  if (port->tx_buffer_pool == NULL) {
    port->memory.TxSize = 0;
    dev_warn(port->device, "tx_pool: FAILED\n");
    return -ENOMEM;
  }
  port->memory.TxSize = tx_buffer_size;

  status = fscc_io_create_rx_frames(port, DEFAULT_DESC_RX_NUM);
  if (status) {
    dev_warn(port->device, "rx_frames: FAILED\n");
    return -ENOMEM;
  }

  status = fscc_io_create_tx_frames(port, DEFAULT_DESC_TX_NUM);
  if (status) {
    dev_warn(port->device, "tx_frames: FAILED\n");
    return -ENOMEM;
  }

  return 0;
}

void fscc_io_destroy(struct fscc_port *port) {
  fscc_io_destroy_rx_frames(port);
  fscc_io_destroy_tx_frames(port);
  fscc_io_destroy_pools(port);
}

void fscc_io_destroy_pools(struct fscc_port *port) {
  dma_pool_destroy(port->rx_buffer_pool);
  port->rx_buffer_pool = NULL;
  dma_pool_destroy(port->tx_buffer_pool);
  port->tx_buffer_pool = NULL;
  dma_pool_destroy(port->desc_pool);
  port->desc_pool = NULL;
}

struct io_frame *fscc_io_create_frame(struct fscc_port *port,
                                      struct dma_pool *data_pool) {
  dma_addr_t temp_add;
  struct io_frame *frame = 0;

  frame = kzalloc(sizeof(struct io_frame), GFP_KERNEL);
  if (frame == NULL) {
    dev_warn(port->device, "failed to allocate frame!\n");
    return 0;
  }
  frame->desc = dma_pool_zalloc(port->desc_pool, GFP_KERNEL, &temp_add);
  frame->desc_physical_address = (u32)temp_add;
  if (frame->desc == NULL) {
    kfree(frame);
    dev_warn(port->device, "failed to allocate descriptor!\n");
    return 0;
  }
  temp_add = 0;
  frame->buffer = dma_pool_zalloc(data_pool, GFP_KERNEL, &temp_add);
  frame->desc->data_address = (u32)temp_add;
  if (frame->buffer == NULL) {
    dma_pool_free(port->desc_pool, frame->desc, frame->desc_physical_address);
    kfree(frame);
    dev_warn(port->device, "failed to allocate buffer!\n");
    return 0;
  }

  return frame;
}

void fscc_io_destroy_frame(struct fscc_port *port, struct io_frame *frame,
                           struct dma_pool *data_pool) {
  dma_pool_free(data_pool, frame->buffer, frame->desc->data_address);
  frame->buffer = 0;
  memset(frame->desc, 0, sizeof(struct fscc_descriptor));
  dma_pool_free(port->desc_pool, frame->desc, frame->desc_physical_address);
  kfree(frame);
  frame = 0;
}

void fscc_io_link_descriptors(struct fscc_port *port, struct io_frame **frames,
                              size_t amount) {
  size_t i;

  for (i = 0; i < amount; i++) {
    frames[i]->desc->next_descriptor =
        frames[i < amount - 1 ? i + 1 : 0]->desc_physical_address;
  }
}

int fscc_io_create_rx_frames(struct fscc_port *port, size_t number_of_frames) {
  size_t i;

  port->rx_descriptors =
      kzalloc((sizeof(struct io_frame *) * number_of_frames), GFP_KERNEL);
  if (port->rx_descriptors == NULL) {
    dev_err(port->device, "failed to create rx_descriptors!\n");
    return -ENOMEM;
  }
  for (i = 0; i < number_of_frames; i++) {
    port->rx_descriptors[i] = fscc_io_create_frame(port, port->rx_buffer_pool);
    if (port->rx_descriptors[i] == 0) {
      dev_warn(port->device, "failed to create io_frame at %lu out of %lu\n", i,
               number_of_frames);
      break;
    }
    if (i % 2)
      port->rx_descriptors[i]->desc->control = DESC_HI_BIT;
    else
      port->rx_descriptors[i]->desc->control = 0;

    port->rx_descriptors[i]->desc->data_count = port->memory.RxSize;
  }

  port->memory.RxNum = i;

  fscc_io_link_descriptors(port, port->rx_descriptors, port->memory.RxNum);

  return 0;
}

int fscc_io_create_tx_frames(struct fscc_port *port, size_t number_of_frames) {
  size_t i;

  port->tx_descriptors =
      kzalloc((sizeof(struct io_frame *) * number_of_frames), GFP_KERNEL);
  if (port->tx_descriptors == NULL) {
    dev_err(port->device, "failed to create tx_descriptors!\n");
    return -ENOMEM;
  }
  for (i = 0; i < number_of_frames; i++) {
    port->tx_descriptors[i] = fscc_io_create_frame(port, port->tx_buffer_pool);
    if (port->tx_descriptors[i] == 0) {
      dev_warn(port->device, "failed to create io_frame at %lu out of %lu\n", i,
               number_of_frames);
      break;
    }

    if (i % 2)
      port->tx_descriptors[i]->desc->control = DESC_HI_BIT;
    else
      port->tx_descriptors[i]->desc->control = 0;

    port->tx_descriptors[i]->desc->data_count = 0;
  }

  port->memory.TxNum = i;

  fscc_io_link_descriptors(port, port->tx_descriptors, port->memory.TxNum);

  return 0;
}

int fscc_io_execute_RRES(struct fscc_port *port) {
  return_val_if_untrue(port, 0);

  return fscc_port_set_register(port, 0, CMDR_OFFSET, 0x00020000);
}

int fscc_io_execute_TRES(struct fscc_port *port) {
  return_val_if_untrue(port, 0);

  return fscc_port_set_register(port, 0, CMDR_OFFSET, 0x08000000);
}

int fscc_io_purge_rx(struct fscc_port *port) {
  size_t i;
  unsigned long board_flags;

  return_val_if_untrue(port, 0);
  dev_dbg(port->device, "purge_rx\n");

  spin_lock_irqsave(&port->board_rx_spinlock, board_flags);
  if (fscc_port_is_dma(port)) {
    fscc_dma_execute_STOP_R(port);
    fscc_dma_execute_RST_R(port);
  }
  fscc_io_execute_RRES(port);
  for (i = 0; i < port->memory.RxNum; i++) {
    port->rx_descriptors[i]->desc->control = DESC_HI_BIT;
    port->rx_descriptors[i]->desc->data_count = port->memory.RxSize;
    clear_timestamp(&port->rx_descriptors[i]->timestamp);
  }
  port->user_rx_desc = 0;
  port->fifo_rx_desc = 0;
  port->rx_bytes_in_frame = 0;
  port->rx_frame_size = 0;
  if (fscc_port_is_dma(port))
    fscc_port_set_register(port, 2, DMA_RX_BASE_OFFSET,
                           port->rx_descriptors[0]->desc_physical_address);
  spin_unlock_irqrestore(&port->board_rx_spinlock, board_flags);

  if (fscc_port_is_dma(port))
    fscc_dma_execute_GO_R(port);

  return 1;
}

int fscc_io_purge_tx(struct fscc_port *port) {
  size_t i;
  unsigned long board_flags;

  return_val_if_untrue(port, 0);
  dev_dbg(port->device, "purge_tx\n");

  spin_lock_irqsave(&port->board_tx_spinlock, board_flags);
  if (fscc_port_is_dma(port)) {
    fscc_dma_execute_STOP_T(port);
    fscc_dma_execute_RST_T(port);
  }
  fscc_io_execute_TRES(port);
  for (i = 0; i < port->memory.TxNum; i++) {
    port->tx_descriptors[i]->desc->control = DESC_CSTOP_BIT;
    port->tx_descriptors[i]->desc->data_count = 0;
    clear_timestamp(&port->tx_descriptors[i]->timestamp);
  }
  port->user_tx_desc = 0;
  port->fifo_tx_desc = 0;
  port->tx_bytes_in_frame = 0;
  port->tx_frame_size = 0;
  if (fscc_port_is_dma(port))
    fscc_port_set_register(port, 2, DMA_TX_BASE_OFFSET,
                           port->tx_descriptors[0]->desc_physical_address);
  spin_unlock_irqrestore(&port->board_tx_spinlock, board_flags);

  wake_up_interruptible(&port->output_queue);
  return 1;
}

void fscc_io_destroy_rx_frames(struct fscc_port *port) {
  size_t i;

  if (fscc_port_is_dma(port)) {
    fscc_dma_execute_STOP_R(port);
    fscc_dma_execute_RST_R(port);
    fscc_port_set_register(port, 2, DMA_RX_BASE_OFFSET, 0);
  }
  if (port->rx_descriptors == NULL)
    return;

  for (i = 0; i < port->memory.RxNum; i++) {
    fscc_io_destroy_frame(port, port->rx_descriptors[i], port->rx_buffer_pool);
  }

  kfree(port->rx_descriptors);
  port->rx_descriptors = 0;
  port->memory.RxNum = 0;
}

void fscc_io_destroy_tx_frames(struct fscc_port *port) {
  size_t i;

  if (fscc_port_is_dma(port)) {
    fscc_dma_execute_STOP_T(port);
    fscc_dma_execute_RST_T(port);
    fscc_port_set_register(port, 2, DMA_TX_BASE_OFFSET, 0);
  }
  if (port->tx_descriptors == NULL)
    return;

  for (i = 0; i < port->memory.TxNum; i++) {
    fscc_io_destroy_frame(port, port->tx_descriptors[i], port->tx_buffer_pool);
  }

  kfree(port->tx_descriptors);
  port->tx_descriptors = 0;
  port->memory.TxNum = 0;
}

int fscc_dma_is_rx_running(struct fscc_port *port) {
  __u32 dstar_value = 0;

  return_val_if_untrue(port, 0);
  dstar_value = fscc_port_get_register(port, 2, DSTAR_OFFSET);

  if (port->channel == 0)
    return (dstar_value & 0x1) ? 0 : 1;
  else
    return (dstar_value & 0x4) ? 0 : 1;
}

int fscc_dma_is_tx_running(struct fscc_port *port) {
  __u32 dstar_value = 0;

  return_val_if_untrue(port, 0);
  dstar_value = fscc_port_get_register(port, 2, DSTAR_OFFSET);

  if (port->channel == 0)
    return (dstar_value & 0x2) ? 0 : 1;
  else
    return (dstar_value & 0x8) ? 0 : 1;
}

int fscc_dma_is_master_dead(struct fscc_port *port) {
  u32 dstar_value = 0;

  return_val_if_untrue(port, 0);
  dstar_value = fscc_port_get_register(port, 2, DSTAR_OFFSET);

  return (dstar_value & 0x10) ? 1 : 0;
}

// To reset the master, we need to write to the first port, so we skip
// port_set_register
void fscc_dma_reset_master(struct fscc_port *port) {
  fscc_card_set_register(port->card, 2, DMACCR_OFFSET, 0x10000);
}

void fscc_dma_execute_RST_R(struct fscc_port *port) {
  return_if_untrue(port);
  return_if_untrue(fscc_port_is_dma(port) == 1);

  fscc_port_set_register(port, 2, DMACCR_OFFSET, 0x00000010);
}

void fscc_dma_execute_RST_T(struct fscc_port *port) {
  return_if_untrue(port);
  return_if_untrue(fscc_port_is_dma(port) == 1);

  fscc_port_set_register(port, 2, DMACCR_OFFSET, 0x00000020);
}

void fscc_dma_execute_GO_T(struct fscc_port *port) {
  return_if_untrue(port);
  return_if_untrue(fscc_port_is_dma(port) == 1);

  fscc_port_set_register(port, 2, DMACCR_OFFSET, 0x00000002);
}

void fscc_dma_execute_GO_R(struct fscc_port *port) {
  return_if_untrue(port);
  return_if_untrue(fscc_port_is_dma(port) == 1);

  fscc_port_set_register(port, 2, DMACCR_OFFSET, 0x00000001);
}

void fscc_dma_execute_STOP_R(struct fscc_port *port) {
  return_if_untrue(port);
  return_if_untrue(fscc_port_is_dma(port) == 1);

  fscc_port_set_register(port, 2, DMACCR_OFFSET, 0x00000100);
}

void fscc_dma_execute_STOP_T(struct fscc_port *port) {
  return_if_untrue(port);
  return_if_untrue(fscc_port_is_dma(port) == 1);

  fscc_port_set_register(port, 2, DMACCR_OFFSET, 0x00000200);
}

unsigned fscc_io_get_RXCNT(struct fscc_port *port) {
  __u32 fifo_bc_value = 0;

  return_val_if_untrue(port, 0);

  fifo_bc_value = fscc_port_get_register(port, 0, FIFO_BC_OFFSET);

  /* Not sure why, but this can be larger than 8192. We add
 the 8192 check here so other code can count on the value
 not being larger than 8192. */
  fifo_bc_value = fifo_bc_value & 0x00003FFF;
  if (fifo_bc_value > 8192)
    fifo_bc_value = 0;

  return fifo_bc_value;
}

unsigned fscc_io_get_RFCNT(struct fscc_port *port) {
  __u32 fifo_fc_value = 0;

  return_val_if_untrue(port, 0);

  fifo_fc_value = fscc_port_get_register(port, 0, FIFO_FC_OFFSET);

  return (unsigned)(fifo_fc_value & 0x000003ff);
}

unsigned fscc_io_get_TXCNT(struct fscc_port *port) {
  __u32 fifo_bc_value = 0;

  return_val_if_untrue(port, 0);

  fifo_bc_value = fscc_port_get_register(port, 0, FIFO_BC_OFFSET);

  return (fifo_bc_value & 0x1FFF0000) >> 16;
}

unsigned fscc_io_get_TFCNT(struct fscc_port *port) {
  __u32 fifo_fc_value = 0;

  return_val_if_untrue(port, 0);

  fifo_fc_value = fscc_port_get_register(port, 0, FIFO_FC_OFFSET);

  return (unsigned)((fifo_fc_value & 0x01ff0000) >> 16);
}

int fscc_dma_enable(struct fscc_port *port) {
  return fscc_port_set_register(port, 2, DMACCR_OFFSET, 0x03000000);
}

int fscc_dma_disable(struct fscc_port *port) {
  return fscc_port_set_register(port, 2, DMACCR_OFFSET, 0x00000000);
}

unsigned fscc_user_next_read_size(struct fscc_port *port, size_t *bytes) {
  unsigned i, cur_desc;
  u32 control = 0;

  *bytes = 0;
  cur_desc = port->user_rx_desc;
  for (i = 0; i < port->memory.RxNum; i++) {
    control = port->rx_descriptors[cur_desc]->desc->control;

    // If neither FE or CSTOP, desc is unfinished.
    if (!(control & DESC_FE_BIT) && !(control & DESC_CSTOP_BIT))
      break;

    if ((control & DESC_FE_BIT) && (control & DESC_CSTOP_BIT)) {
      // CSTOP and FE means this is the final desc in a finished frame. Return
      // CNT to get the total size of the frame.
      *bytes = (control & DMA_MAX_LENGTH);
      return 1;
    }

    *bytes += port->rx_descriptors[cur_desc]->desc->data_count;

    cur_desc++;
    if (cur_desc == port->memory.RxNum)
      cur_desc = 0;
  }

  return 0;
}

int fscc_io_user_read_ready(struct fscc_port *port) {
  int frame_ready = 0;
  size_t bytes_ready = 0;

  frame_ready = fscc_user_next_read_size(port, &bytes_ready);
  if (fscc_port_is_streaming(port))
    return bytes_ready == 0 ? 0 : 1;
  else
    return frame_ready;
}

void fscc_io_apply_timestamps(struct fscc_port *port) {
  unsigned long board_flags;
  size_t i, cur_desc;
  u32 control = 0;

  spin_lock_irqsave(&port->board_rx_spinlock, board_flags);
  cur_desc = port->user_rx_desc;
  for (i = 0; i < port->memory.RxNum; i++) {
    control = port->rx_descriptors[cur_desc]->desc->control;

    if (!(control & DESC_FE_BIT) && !(control & DESC_CSTOP_BIT))
      break;

    if (timestamp_is_empty(&port->rx_descriptors[cur_desc]->timestamp))
      set_timestamp(&port->rx_descriptors[cur_desc]->timestamp);

    cur_desc++;
    if (cur_desc == port->memory.RxNum)
      cur_desc = 0;
  }
  spin_unlock_irqrestore(&port->board_rx_spinlock, board_flags);
}

size_t fscc_user_get_tx_space(struct fscc_port *port) {
  unsigned i, cur_desc;
  size_t space = 0;

  cur_desc = port->user_tx_desc;
  for (i = 0; i < port->memory.TxNum; i++) {
    if ((port->tx_descriptors[cur_desc]->desc->control & DESC_CSTOP_BIT) !=
        DESC_CSTOP_BIT)
      break;

    space += port->memory.TxSize;

    cur_desc++;
    if (cur_desc == port->memory.TxNum)
      cur_desc = 0;
  }

  return space;
}

void fscc_io_execute_transmit(struct fscc_port *port, unsigned dma) {
  unsigned command_register = 0;
  unsigned command_value = 0;
  unsigned command_bar = 0;

  return_if_untrue(port);

  if (dma) {
    command_bar = 2;
    command_register = DMACCR_OFFSET;
    command_value = 0x00000002;

    if (port->tx_modifiers & XREP)
      command_value |= 0x40000000;

    if (port->tx_modifiers & TXT)
      command_value |= 0x10000000;

    if (port->tx_modifiers & TXEXT)
      command_value |= 0x20000000;
  } else {
    command_bar = 0;
    command_register = CMDR_OFFSET;
    command_value = 0x01000000;

    if (port->tx_modifiers & XREP)
      command_value |= 0x02000000;

    if (port->tx_modifiers & TXT)
      command_value |= 0x10000000;

    if (port->tx_modifiers & TXEXT)
      command_value |= 0x20000000;
  }

  fscc_port_set_register(port, command_bar, command_register, command_value);
}

unsigned fscc_io_transmit_frame(struct fscc_port *port) {
  int result;

  result = fscc_fifo_write_data(port);
  if (result)
    fscc_io_execute_transmit(port, 0);

  return result;
}

#define TX_FIFO_SIZE 4096
int fscc_fifo_write_data(struct fscc_port *port) {
  unsigned fifo_space, write_length, frame_size = 0, size_in_fifo;
  unsigned long board_flags = 0;
  size_t i, data_written = 0;
  u32 txcnt = 0, tfcnt = 0;

  tfcnt = fscc_io_get_TFCNT(port);
  if (tfcnt > 254)
    return 0;

  txcnt = fscc_io_get_TXCNT(port);
  fifo_space = TX_FIFO_SIZE - txcnt - 1;
  fifo_space -= fifo_space % 4;
  if (fifo_space > TX_FIFO_SIZE)
    return 0;

  spin_lock_irqsave(&port->board_tx_spinlock, board_flags);
  for (i = 0; i < port->memory.TxNum; i++) {
    if ((port->tx_descriptors[port->fifo_tx_desc]->desc->control &
         DESC_CSTOP_BIT) == DESC_CSTOP_BIT)
      break;

    write_length = port->tx_descriptors[port->fifo_tx_desc]->desc->data_count;
    size_in_fifo = write_length + (4 - write_length % 4);
    if (fifo_space < size_in_fifo)
      break;

    if ((port->tx_descriptors[port->fifo_tx_desc]->desc->control &
         DESC_FE_BIT) == DESC_FE_BIT) {
      if (tfcnt > 255)
        break;
      if (frame_size)
        break;
      frame_size = port->tx_descriptors[port->fifo_tx_desc]->desc->control &
                   DMA_MAX_LENGTH;
    }

    fscc_port_set_register_rep(port, 0, FIFO_OFFSET,
                               port->tx_descriptors[port->fifo_tx_desc]->buffer,
                               write_length);
    data_written = 1;
    fifo_space -= size_in_fifo;

    // Descriptor is empty, time to reset it.
    port->tx_descriptors[port->fifo_tx_desc]->desc->data_count = 0;
    port->tx_descriptors[port->fifo_tx_desc]->desc->control = DESC_CSTOP_BIT;

    port->fifo_tx_desc++;
    if (port->fifo_tx_desc == port->memory.TxNum)
      port->fifo_tx_desc = 0;
  }
  if (frame_size)
    fscc_port_set_register(port, 0, BC_FIFO_L_OFFSET, frame_size);

  spin_unlock_irqrestore(&port->board_tx_spinlock, board_flags);

  return data_written;
}

int fscc_fifo_read_data(struct fscc_port *port) {
  size_t i;
  unsigned long board_flags = 0;
  unsigned rxcnt, receive_length = 0;
  u32 new_control = 0;

  spin_lock_irqsave(&port->board_rx_spinlock, board_flags);
  for (i = 0; i < port->memory.RxNum; i++) {

    new_control = port->rx_descriptors[port->fifo_rx_desc]->desc->control;
    if ((new_control & DESC_CSTOP_BIT) == DESC_CSTOP_BIT)
      break;

    if (port->rx_frame_size == 0) {
      if (fscc_io_get_RFCNT(port))
        port->rx_frame_size = fscc_port_get_register(port, 0, BC_FIFO_L_OFFSET);
    }

    if (port->rx_frame_size) {
      if (port->rx_frame_size > port->rx_bytes_in_frame)
        receive_length = port->rx_frame_size - port->rx_bytes_in_frame;
      else
        receive_length = 0;
    } else {
      rxcnt = fscc_io_get_RXCNT(port);
      receive_length = rxcnt - (rxcnt % 4);
    }

    receive_length = min(receive_length,
                         port->memory.RxSize - (new_control & DMA_MAX_LENGTH));
    // Instead of breaking out if this is 0, we move on to allow the FE/CSTOP
    // processing.

    if (receive_length)
      fscc_port_get_register_rep(
          port, 0, FIFO_OFFSET,
          port->rx_descriptors[port->fifo_rx_desc]->buffer +
              (new_control & DMA_MAX_LENGTH),
          receive_length);
    new_control += receive_length;
    port->rx_bytes_in_frame += receive_length;

    // We've finished this descriptor, so we finalize it.
    if ((new_control & DMA_MAX_LENGTH) >= port->memory.RxSize) {
      new_control &= ~DMA_MAX_LENGTH;
      new_control |= DESC_CSTOP_BIT;
    }

    if (port->rx_frame_size > 0) {
      // We've finished a frame, so we finalize it and clean up a bit.
      if (port->rx_bytes_in_frame >= port->rx_frame_size) {
        new_control = port->rx_frame_size;
        new_control |= DESC_CSTOP_BIT;
        new_control |= DESC_FE_BIT;
        port->rx_frame_size = 0;
        port->rx_bytes_in_frame = 0;
      }
    }

    // Finalize the descriptor if it's finished.
    if (new_control & DESC_CSTOP_BIT) {
      if (timestamp_is_empty(
              &port->rx_descriptors[port->fifo_rx_desc]->timestamp))
        set_timestamp(&port->rx_descriptors[port->fifo_rx_desc]->timestamp);
      port->rx_descriptors[port->fifo_rx_desc]->desc->data_count =
          port->memory.RxSize;
    }

    port->rx_descriptors[port->fifo_rx_desc]->desc->control = new_control;

    // Desc isn't finished, which means we're out of data.
    if ((new_control & DESC_CSTOP_BIT) != DESC_CSTOP_BIT)
      break;

    port->fifo_rx_desc++;
    if (port->fifo_rx_desc == port->memory.RxNum)
      port->fifo_rx_desc = 0;
  }
  spin_unlock_irqrestore(&port->board_rx_spinlock, board_flags);
  return 1;
}

int fscc_user_write_frame(struct fscc_port *port, const char *buf,
                          size_t data_length) {
  size_t i;
  int status = 0;
  u32 new_control = 0;
  size_t transmit_length;
  u32 start_desc = 0;
  unsigned long board_flags = 0;
  int out_length = 0;
  spin_lock_irqsave(&port->board_tx_spinlock, board_flags);
  for (i = 0; i < port->memory.TxNum; i++) {
    if ((port->tx_descriptors[port->user_tx_desc]->desc->control &
         DESC_CSTOP_BIT) != DESC_CSTOP_BIT) {
      status = -ENOBUFS;
      break;
    }
    if (start_desc == 0)
      start_desc =
          port->tx_descriptors[port->user_tx_desc]->desc_physical_address;
    if (data_length - out_length > port->memory.TxSize)
      transmit_length = port->memory.TxSize;
    else
      transmit_length = data_length - out_length;

    memcpy(port->tx_descriptors[port->user_tx_desc]->buffer, buf + out_length,
           transmit_length);
    out_length += transmit_length;
    port->tx_descriptors[port->user_tx_desc]->desc->data_count =
        transmit_length;
    new_control = DESC_HI_BIT;
    if (i == 0) {
      new_control |= DESC_FE_BIT;
      new_control |= data_length;
    } else {
      new_control |= transmit_length;
    }
    port->tx_descriptors[port->user_tx_desc]->desc->control = new_control;

    port->user_tx_desc++;
    if (port->user_tx_desc == port->memory.TxNum)
      port->user_tx_desc = 0;
    if (out_length == data_length)
      break;
  }
  spin_unlock_irqrestore(&port->board_tx_spinlock, board_flags);
  // There is no additional prep for DMA.. so lets just start it.
  if (fscc_port_is_dma(port) && !fscc_dma_is_tx_running(port)) {
    fscc_port_set_register(port, 2, DMA_TX_BASE_OFFSET, start_desc);
    fscc_io_execute_transmit(port, 1);
  }
  return status;
}

ssize_t fscc_user_read_frame(struct fscc_port *port, char *buf,
                             size_t buf_length) {
  size_t i;
  int frame_ready = 0;
  size_t planned_move_size = 0;
  size_t real_move_size = 0;
  size_t bytes_in_descs = 0;
  size_t total_valid_data = 0;
  size_t filled_frame_size = 0;
  size_t buffer_requirement = 0;
  u32 control = 0;
  unsigned long board_flags = 0;
  unsigned out_length = 0;
  unsigned long unmoved_bytes = 0;
  char *temp_data = 0;

  return_val_if_untrue(port, 0);

  //// I don't like doing this, but it must be done.
  //// copy_to_user() must be used to move data from kernel space to
  //// user space, but copy_to_user() cannot be used in a spinlock.
  //// I can either implement a mutex (perhaps in the future) or
  //// organize a temporary holding buffer until we lose the spinlock.
  temp_data = kzalloc(buf_length, GFP_KERNEL);
  if (temp_data == NULL) {
    dev_warn(port->device, "failed to allocate temp_data!\n");
    return 0;
  }

  spin_lock_irqsave(&port->board_rx_spinlock, board_flags);

  frame_ready = fscc_user_next_read_size(port, &bytes_in_descs);
  if (!frame_ready) {
    spin_unlock_irqrestore(&port->board_rx_spinlock, board_flags);
    kfree(temp_data);
    return -ENOBUFS;
  }

  total_valid_data = bytes_in_descs;
  total_valid_data -= (port->append_status) ? 0 : 2;

  buffer_requirement = bytes_in_descs;
  buffer_requirement -= (port->append_status) ? 0 : 2;
  buffer_requirement += (port->append_timestamp) ? sizeof(fscc_timestamp) : 0;
  if (buffer_requirement > buf_length) {
    spin_unlock_irqrestore(&port->board_rx_spinlock, board_flags);
    kfree(temp_data);
    return -ENOBUFS;
  }
  for (i = 0; i < port->memory.RxNum; i++) {
    control = port->rx_descriptors[port->user_rx_desc]->desc->control;

    planned_move_size =
        port->rx_descriptors[port->user_rx_desc]->desc->data_count;
    if ((control & DESC_FE_BIT) && (control & DESC_CSTOP_BIT)) {
      // CSTOP and FE means this is the final desc in a finished frame.
      // CNT to get the total size of the frame, subtract already gathered
      // bytes.
      planned_move_size = (control & DMA_MAX_LENGTH) - filled_frame_size;
    }
    if (planned_move_size > total_valid_data)
      real_move_size = total_valid_data;
    else
      real_move_size = planned_move_size;

    if (real_move_size)
      memcpy(temp_data + out_length,
             port->rx_descriptors[port->user_rx_desc]->buffer, real_move_size);

    if (planned_move_size > bytes_in_descs)
      bytes_in_descs = 0;
    else
      bytes_in_descs -= planned_move_size;
    total_valid_data -= real_move_size;
    filled_frame_size += real_move_size;
    out_length += real_move_size;

    if (bytes_in_descs == 0 && port->append_timestamp) {
      if (timestamp_is_empty(
              &port->rx_descriptors[port->user_rx_desc]->timestamp))
        set_timestamp(&port->rx_descriptors[port->user_rx_desc]->timestamp);
      memcpy(temp_data + out_length,
             &port->rx_descriptors[port->user_rx_desc]->timestamp,
             sizeof(fscc_timestamp));
      out_length += sizeof(fscc_timestamp);
    }
    clear_timestamp(&port->rx_descriptors[port->user_rx_desc]->timestamp);
    port->rx_descriptors[port->user_rx_desc]->desc->control = DESC_HI_BIT;

    port->user_rx_desc++;
    if (port->user_rx_desc == port->memory.RxNum)
      port->user_rx_desc = 0;

    if (bytes_in_descs == 0) {
      if (!port->rx_multiple)
        break;
      frame_ready = fscc_user_next_read_size(port, &bytes_in_descs);
      if (!frame_ready)
        break;

      total_valid_data = bytes_in_descs;
      total_valid_data -= (port->append_status) ? 0 : 2;

      buffer_requirement = bytes_in_descs;
      buffer_requirement -= (port->append_status) ? 0 : 2;
      buffer_requirement +=
          (port->append_timestamp) ? sizeof(fscc_timestamp) : 0;

      if (buffer_requirement > buf_length - out_length)
        break;
      filled_frame_size = 0;
    }
  }
  spin_unlock_irqrestore(&port->board_rx_spinlock, board_flags);
  if (out_length) {
    unmoved_bytes = copy_to_user(buf, temp_data, out_length);
    out_length = out_length - unmoved_bytes;
  }
  if (unmoved_bytes)
    dev_dbg(port->device, "user_read_frame unmoved_bytes: %lu\n",
            unmoved_bytes);

  kfree(temp_data);
  return out_length;
}

ssize_t fscc_user_read_stream(struct fscc_port *port, char *buf,
                              size_t buf_length) {
  size_t i;
  unsigned out_length = 0;
  int receive_length = 0;
  u32 control;
  unsigned long board_flags = 0;
  unsigned long unmoved_bytes = 0;
  char *temp_data = 0;

  return_val_if_untrue(port, 0);

  //// I don't like doing this, but it must be done.
  //// copy_to_user() must be used to move data from kernel space to
  //// user space, but copy_to_user() cannot be used in a spinlock.
  //// I can either implement a mutex (perhaps in the future) or
  //// organize a temporary holding buffer until we lose the spinlock.
  temp_data = kzalloc(buf_length, GFP_KERNEL);
  if (temp_data == NULL) {
    dev_warn(port->device, "failed to allocate temp_data!\n");
    return 0;
  }

  spin_lock_irqsave(&port->board_rx_spinlock, board_flags);
  for (i = 0; i < port->memory.RxNum; i++) {
    control = port->rx_descriptors[port->user_rx_desc]->desc->control;

    // If not CSTOP && not FE, then break
    if (!(control & DESC_FE_BIT) && !(control & DESC_CSTOP_BIT))
      break;
    if (buf_length - out_length >
        port->rx_descriptors[port->user_rx_desc]->desc->data_count)
      receive_length =
          port->rx_descriptors[port->user_rx_desc]->desc->data_count;
    else
      receive_length = buf_length - out_length;

    memcpy(temp_data + out_length,
           port->rx_descriptors[port->user_rx_desc]->buffer, receive_length);
    out_length += receive_length;

    if (receive_length ==
        port->rx_descriptors[port->user_rx_desc]->desc->data_count) {
      port->rx_descriptors[port->user_rx_desc]->desc->data_count =
          port->memory.RxSize;
      clear_timestamp(&port->rx_descriptors[port->user_rx_desc]->timestamp);
      if (i % 2)
        port->rx_descriptors[port->user_rx_desc]->desc->control = DESC_HI_BIT;
      else
        port->rx_descriptors[port->user_rx_desc]->desc->control = 0;
    } else {
      int remaining =
          port->rx_descriptors[port->user_rx_desc]->desc->data_count -
          receive_length;
      // Moving data to the front of the descriptor.
      memmove(port->rx_descriptors[port->user_rx_desc]->buffer,
              port->rx_descriptors[port->user_rx_desc]->buffer + receive_length,
              remaining);
      port->rx_descriptors[port->user_rx_desc]->desc->data_count = remaining;
      break;
    }

    port->user_rx_desc++;
    if (port->user_rx_desc == port->memory.RxNum)
      port->user_rx_desc = 0;
  }
  spin_unlock_irqrestore(&port->board_rx_spinlock, board_flags);
  if (out_length) {
    unmoved_bytes = copy_to_user(buf, temp_data, out_length);
    out_length = out_length - unmoved_bytes;
  }
  if (unmoved_bytes)
    dev_dbg(port->device, "user_read_stream unmoved_bytes: %lu\n",
            unmoved_bytes);

  kfree(temp_data);
  return out_length;
}

/*
        Returns -ENOBUFS if count is smaller than pending frame size
        Buf needs to be a user buffer
*/
ssize_t fscc_io_read(struct fscc_port *port, char *buf, size_t count) {
  if (fscc_port_is_streaming(port))
    return fscc_user_read_stream(port, buf, count);
  else
    return fscc_user_read_frame(port, buf, count);
}

void clear_timestamp(fscc_timestamp *timestamp) {
  memset(timestamp, 0, sizeof(fscc_timestamp));
}

int timestamp_is_empty(fscc_timestamp *timestamp) {
  if (timestamp->tv_sec != 0)
    return 0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)
  if (timestamp->tv_nsec != 0)
    return 0;
#else
  if (timestamp->tv_usec != 0)
    return 0;
#endif
  return 1;
}
