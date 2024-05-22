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

#ifndef FSCC_PORT
#define FSCC_PORT

#include <linux/fs.h> /* Needed to build on older kernel version */
#include <linux/cdev.h> /* struct cdev */
#include <linux/interrupt.h> /* struct tasklet_struct */
#include <linux/version.h> /* LINUX_VERSION_CODE, KERNEL_VERSION */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 26)
#include <linux/semaphore.h> /* struct semaphore */
#endif

#include "fscc.h" /* struct fscc_registers */
#include "debug.h" /* stuct debug_interrupt_tracker */

#define FIFO_OFFSET 0x00
#define BC_FIFO_L_OFFSET 0x04
#define FIFOT_OFFSET 0x08
#define FIFO_BC_OFFSET 0x0C
#define FIFO_FC_OFFSET 0x10
#define CMDR_OFFSET 0x14
#define STAR_OFFSET 0x18
#define CCR0_OFFSET 0x1C
#define CCR1_OFFSET 0x20
#define CCR2_OFFSET 0x24
#define BGR_OFFSET 0x28
#define SSR_OFFSET 0x2C
#define SMR_OFFSET 0x30
#define TSR_OFFSET 0x34
#define TMR_OFFSET 0x38
#define RAR_OFFSET 0x3C
#define RAMR_OFFSET 0x40
#define PPR_OFFSET 0x44
#define TCR_OFFSET 0x48
#define VSTR_OFFSET 0x4C
#define ISR_OFFSET 0x50
#define IMR_OFFSET 0x54
#define DPLLR_OFFSET 0x58
#define MAX_OFFSET 0x58 //must equal the highest FCore register address

#define DMACCR_OFFSET 0x04
#define DMA_RX_BASE_OFFSET 0x0c
#define DMA_TX_BASE_OFFSET 0x10
#define DMA_CURRENT_RX_BASE_OFFSET 0x20
#define DMA_CURRENT_TX_BASE_OFFSET 0x24

#define RFE 0x00000004
#define RFT 0x00000002
#define RFS 0x00000001
#define RFO 0x00000008
#define RDO 0x00000010
#define RFL 0x00000020
#define TIN 0x00000100
#define TDU 0x00040000
#define TFT 0x00010000
#define ALLS 0x00020000
#define CTSS 0x01000000
#define DSRC 0x02000000
#define CDC 0x04000000
#define CTSA 0x08000000
#define DR_STOP 0x00004000
#define DT_STOP 0x00008000
#define DT_FE 0x00002000
#define DR_FE 0x00001000
#define DT_HI 0x00000800
#define DR_HI 0x00000400

#define CE_BIT 0x00040000

struct fscc_port {
	struct list_head list;
	dev_t dev_t;
	struct class *class;
	struct cdev cdev;
	struct fscc_card *card;
	struct device *device;
	unsigned channel;
	char *name;

	/* Prevents simultaneous read(), write() and poll() calls. */
	struct semaphore read_semaphore;
	struct semaphore write_semaphore;
	struct semaphore poll_semaphore;

	wait_queue_head_t input_queue;
	wait_queue_head_t output_queue;
	wait_queue_head_t status_queue;

	struct fscc_registers register_storage; /* Only valid on suspend/resume */

	struct tasklet_struct iframe_tasklet;
	struct tasklet_struct send_oframe_tasklet;
	struct tasklet_struct timestamp_tasklet;

	unsigned last_isr_value;

	unsigned append_status;
	unsigned append_timestamp;

	spinlock_t board_settings_spinlock; /* Anything that will alter the settings at a board level */
	spinlock_t board_rx_spinlock; /* Anything that will alter the state of rx at a board level */
	spinlock_t board_tx_spinlock; /* Anything that will alter the state of rx at a board level */

	struct fscc_memory memory;
	unsigned ignore_timeout;
	unsigned rx_multiple;
	unsigned tx_modifiers;

	struct timer_list timer;

	struct dma_pool *desc_pool;
	struct dma_pool *rx_buffer_pool;
	struct dma_pool *tx_buffer_pool;

	struct io_frame **rx_descriptors;
	unsigned user_rx_desc; // DMA & FIFO, this is where the drivers are working.
	unsigned fifo_rx_desc; // For non-DMA use, this is where the FIFO is currently working.
	int rx_bytes_in_frame; // FIFO, How many bytes are in the current RX frame
	int rx_frame_size; // FIFO, The current RX frame size

	struct io_frame **tx_descriptors;
	unsigned user_tx_desc; // DMA & FIFO, this is where the drivers are working.
	unsigned fifo_tx_desc; // For non-DMA use, this is where the FIFO is currently working.
	int tx_bytes_in_frame; // FIFO, How many bytes are in the current TX frame
	int tx_frame_size; // FIFO, The current TX frame size

#ifdef DEBUG
	struct debug_interrupt_tracker *interrupt_tracker;
	struct tasklet_struct print_tasklet;
#endif
};
#include "io.h" // Frustrating, this only works here, probably because io.h needs a port definition.

struct fscc_port *fscc_port_new(struct fscc_card *card, unsigned channel,
								unsigned major_number, unsigned minor_number,
								struct device *parent, struct class *class,
								struct file_operations *fops);

void fscc_port_delete(struct fscc_port *port);

__u32 fscc_port_get_register(struct fscc_port *port, unsigned bar,
							 unsigned register_offset);

void fscc_port_get_register_rep(struct fscc_port *port, unsigned bar,
								unsigned register_offset, char *buf,
								unsigned byte_count);

int fscc_port_set_register(struct fscc_port *port, unsigned bar,
							unsigned register_offset, __u32 value);

void fscc_port_set_register_rep(struct fscc_port *port, unsigned bar,
								unsigned register_offset, const char *data,
								unsigned byte_count);

__u8 fscc_port_get_FREV(struct fscc_port *port);
__u8 fscc_port_get_PREV(struct fscc_port *port);

void fscc_port_suspend(struct fscc_port *port);
void fscc_port_resume(struct fscc_port *port);

void fscc_port_set_ignore_timeout(struct fscc_port *port,
								  unsigned ignore_timeout);
unsigned fscc_port_get_ignore_timeout(struct fscc_port *port);

void fscc_port_set_rx_multiple(struct fscc_port *port,
								  unsigned rx_multiple);
unsigned fscc_port_get_rx_multiple(struct fscc_port *port);

void fscc_port_set_clock_bits(struct fscc_port *port,
							  unsigned char *clock_data);

int fscc_port_set_append_status(struct fscc_port *port, unsigned value);
unsigned fscc_port_get_append_status(struct fscc_port *port);

int fscc_port_set_append_timestamp(struct fscc_port *port, unsigned value);
unsigned fscc_port_get_append_timestamp(struct fscc_port *port);

int fscc_port_set_registers(struct fscc_port *port,
							 const struct fscc_registers *regs);

void fscc_port_get_registers(struct fscc_port *port,
							 struct fscc_registers *regs);

unsigned fscc_port_using_async(struct fscc_port *port);
unsigned fscc_port_is_streaming(struct fscc_port *port);

unsigned fscc_port_is_dma(struct fscc_port *port);

unsigned fscc_port_has_incoming_data(struct fscc_port *port);

unsigned fscc_port_timed_out(struct fscc_port *port);

#ifdef DEBUG
unsigned fscc_port_get_interrupt_count(struct fscc_port *port, __u32 isr_bit);
void fscc_port_increment_interrupt_counts(struct fscc_port *port,
										  __u32 isr_value);
#endif /* DEBUG */

int fscc_port_set_tx_modifiers(struct fscc_port *port, unsigned tx_modifiers);
unsigned fscc_port_get_tx_modifiers(struct fscc_port *port);

void fscc_port_reset_timer(struct fscc_port *port);

#endif
