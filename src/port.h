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

#ifndef FSCC_PORT
#define FSCC_PORT

#include <linux/cdev.h>
#include <linux/serial_core.h>
#include <linux/list.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include "frame.h"
#include "fscc.h"

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

struct fscc_port {
	struct list_head list;
	dev_t dev_t;
	struct class *class;
	struct cdev cdev;
	struct fscc_card *card;
	unsigned channel;
	char *name;
	
	struct semaphore semaphore;
	wait_queue_head_t input_queue;
	wait_queue_head_t output_queue;
	
	struct list_head oframes;
	struct list_head iframes;
	
	unsigned tx_memory_cap;
	unsigned rx_memory_cap;
	
	struct fscc_registers register_storage; /* Only valid on suspend/resume */
	
	//struct workqueue_struct *output_workqueue;
	//struct work_struct TFT_worker;
	//struct work_struct ALLS_worker;
};

struct fscc_port *fscc_port_new(struct fscc_card *card, unsigned channel, 
                                unsigned major_number, unsigned minor_number, 
                                struct class *class, struct file_operations *fops);
                                
void fscc_port_delete(struct fscc_port *port);
unsigned fscc_port_exists(struct fscc_port *port, struct list_head *card_list);

unsigned fscc_port_get_major_number(struct fscc_port *port);
unsigned fscc_port_get_minor_number(struct fscc_port *port);

struct fscc_card *fscc_port_get_card(struct fscc_port *port);

int fscc_port_write(struct fscc_port *port, const char *data, unsigned length);                     
ssize_t fscc_port_read(struct fscc_port *port, char *buf, size_t count);
                     
unsigned fscc_port_get_available_tx_bytes(struct fscc_port *port);

void fscc_port_fill_TxFIFO(struct fscc_port *port, const char *data, 
                           unsigned byte_count);
void fscc_port_empty_RxFIFO(struct fscc_port *port);
                           
bool fscc_port_has_iframes(struct fscc_port *port);
bool fscc_port_has_oframes(struct fscc_port *port);

struct fscc_frame *fscc_port_push_iframe(struct fscc_port *port, unsigned length);
struct fscc_frame *fscc_port_push_oframe(struct fscc_port *port, unsigned length);

struct fscc_frame *fscc_port_peek_back_iframe(struct fscc_port *port);
struct fscc_frame *fscc_port_peek_back_oframe(struct fscc_port *port);

struct fscc_frame *fscc_port_peek_front_iframe(struct fscc_port *port);
struct fscc_frame *fscc_port_peek_front_oframe(struct fscc_port *port);

void fscc_port_pop_iframe(struct fscc_port *port);
void fscc_port_pop_oframe(struct fscc_port *port);

unsigned fscc_port_get_iframe_qty(struct fscc_port *port);
unsigned fscc_port_get_oframe_qty(struct fscc_port *port);

bool fscc_port_iframes_ready(struct fscc_port *port);

unsigned fscc_port_total_oframe_memory(struct fscc_port *port);
unsigned fscc_port_total_iframe_memory(struct fscc_port *port);

__u32 fscc_port_get_register(struct fscc_port *port, unsigned bar, 
                             unsigned register_offset);

void fscc_port_set_register(struct fscc_port *port, unsigned bar, 
                            unsigned register_offset, __u32 value);

__u32 fscc_port_get_TXCNT(struct fscc_port *port);

__u8 fscc_port_get_FREV(struct fscc_port *port);
__u8 fscc_port_get_PREV(struct fscc_port *port);

void fscc_port_execute_TRES(struct fscc_port *port);
void fscc_port_execute_RRES(struct fscc_port *port);
void fscc_port_execute_XF(struct fscc_port *port);

void fscc_port_store_registers(struct fscc_port *port);
void fscc_port_restore_registers(struct fscc_port *port);

#endif
