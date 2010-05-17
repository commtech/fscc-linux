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

void tdu_handler(unsigned long data);
void tft_handler(unsigned long data);
void rfs_handler(unsigned long data);
void rft_handler(unsigned long data);
void rfe_handler(unsigned long data);

struct fscc_port {
	struct list_head list;
	dev_t dev_t;
	struct class *class;
	struct cdev cdev;
	struct fscc_card *card;
	struct device *device;
	unsigned channel;
	char *name;
	
	struct semaphore read_semaphore;
	struct semaphore write_semaphore;
	struct semaphore poll_semaphore;
	
	wait_queue_head_t input_queue;
	wait_queue_head_t output_queue;
	
	struct list_head oframes; /* Frames not yet in the FIFO yet */
	struct list_head iframes; /* Frames already retrieved from the FIFO */

	struct fscc_frame *pending_oframe; /* Frame being put in the FIFO */
	struct fscc_frame *pending_iframe; /* Frame retrieving from the FIFO */
	
	struct fscc_registers register_storage; /* Only valid on suspend/resume */

	struct tasklet_struct rfs_tasklet;
	struct tasklet_struct rft_tasklet;
	struct tasklet_struct rfe_tasklet;
	
	struct tasklet_struct tft_tasklet;
	struct tasklet_struct tdu_tasklet;
	
	struct tasklet_struct oframe_tasklet;
	struct tasklet_struct iframe_tasklet;
	
	unsigned started_frames;
	unsigned ended_frames;
	unsigned handled_frames;
};

struct fscc_port *fscc_port_new(struct fscc_card *card, unsigned channel, 
                                unsigned major_number, unsigned minor_number, 
                                struct class *class, struct file_operations *fops);
                                
void fscc_port_delete(struct fscc_port *port);

int fscc_port_write(struct fscc_port *port, const char *data, unsigned length);                     
ssize_t fscc_port_read(struct fscc_port *port, char *buf, size_t count);
                           
bool fscc_port_has_iframes(struct fscc_port *port);
bool fscc_port_has_oframes(struct fscc_port *port);

__u32 fscc_port_get_register(struct fscc_port *port, unsigned bar, 
                             unsigned register_offset);
                             
void fscc_port_get_register_rep(struct fscc_port *port, unsigned bar, 
                                 unsigned register_offset, char *buf,
                                 unsigned long chunks);
                                 
void fscc_port_set_register(struct fscc_port *port, unsigned bar, 
                            unsigned register_offset, __u32 value);
                            
void fscc_port_set_register_rep(struct fscc_port *port, unsigned bar,
                                unsigned register_offset, const char *data,
                                unsigned long chunks); 

__u32 fscc_port_get_TXCNT(struct fscc_port *port);
__u32 fscc_port_get_RXCNT(struct fscc_port *port);
__u32 fscc_port_get_RXTRG(struct fscc_port *port);

__u8 fscc_port_get_FREV(struct fscc_port *port);
__u8 fscc_port_get_PREV(struct fscc_port *port);

void fscc_port_execute_TRES(struct fscc_port *port);
void fscc_port_execute_RRES(struct fscc_port *port);
void fscc_port_execute_XF(struct fscc_port *port);

void fscc_port_store_registers(struct fscc_port *port);
void fscc_port_restore_registers(struct fscc_port *port);

#endif
