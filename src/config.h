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

#ifndef FSCC_CONFIG_H
#define FSCC_CONFIG_H

#define DEVICE_NAME "fscc"

#define STATUS_LENGTH 2

#define TX_FIFO_SIZE 4096
#define RX_FIFO_SIZE 8192

#define COMMTECH_VENDOR_ID 0x18f7

#define FSCC_ID 0x000f
#define SFSCC_ID 0x0014
#define FSCC_232_ID 0x0016
#define SFSCC_4_ID 0x0018
#define FSCC_4_ID 0x001b
#define SFSCC_4_LVDS_ID 0x001c
#define SFSCCe_4_ID 0x001e

#define DEFAULT_MEMORY_CAP 20000
#define DEFAULT_APPEND_STATUS 0

#define SYSFS_READ_ONLY_MODE S_IRUGO
#define SYSFS_WRITE_ONLY_MODE S_IWUGO
#define SYSFS_READ_WRITE_MODE S_IWUGO | S_IRUGO

extern unsigned memory_cap;
extern struct list_head fscc_cards;

#endif

