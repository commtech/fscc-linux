/*
	Copyright (C) 2014 Commtech, Inc.

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

#include "utils.h"
#include "port.h" /* *_OFFSET */
#include "card.h" /* FCR_OFFSET */

__u32 chars_to_u32(const char *data)
{
	return *((__u32*)data);
}

int str_to_register_offset(const char *str)
{
	if (strcmp(str, "fifo") == 0)
		return FIFO_OFFSET;
	else if (strcmp(str, "bcfl") == 0)
		return BC_FIFO_L_OFFSET;
	else if (strcmp(str, "fifot") == 0)
		return FIFOT_OFFSET;
	else if (strcmp(str, "fifobc") == 0)
		return FIFO_BC_OFFSET;
	else if (strcmp(str, "fifofc") == 0)
		return FIFO_FC_OFFSET;
	else if (strcmp(str, "cmdr") == 0)
		return CMDR_OFFSET;
	else if (strcmp(str, "star") == 0)
		return STAR_OFFSET;
	else if (strcmp(str, "ccr0") == 0)
		return CCR0_OFFSET;
	else if (strcmp(str, "ccr1") == 0)
		return CCR1_OFFSET;
	else if (strcmp(str, "ccr2") == 0)
		return CCR2_OFFSET;
	else if (strcmp(str, "bgr") == 0)
		return BGR_OFFSET;
	else if (strcmp(str, "ssr") == 0)
		return SSR_OFFSET;
	else if (strcmp(str, "smr") == 0)
		return SMR_OFFSET;
	else if (strcmp(str, "tsr") == 0)
		return TSR_OFFSET;
	else if (strcmp(str, "tmr") == 0)
		return TMR_OFFSET;
	else if (strcmp(str, "rar") == 0)
		return RAR_OFFSET;
	else if (strcmp(str, "ramr") == 0)
		return RAMR_OFFSET;
	else if (strcmp(str, "ppr") == 0)
		return PPR_OFFSET;
	else if (strcmp(str, "tcr") == 0)
		return TCR_OFFSET;
	else if (strcmp(str, "vstr") == 0)
		return VSTR_OFFSET;
	else if (strcmp(str, "isr") == 0)
		return ISR_OFFSET;
	else if (strcmp(str, "imr") == 0)
		return IMR_OFFSET;
	else if (strcmp(str, "dpllr") == 0)
		return DPLLR_OFFSET;
	else if (strcmp(str, "fstel") == 0)
		return 0x5c;
	else if (strcmp(str, "fstew") == 0)
		return 0x60;
	else if (strcmp(str, "fcr") == 0)
		return FCR_OFFSET;
	else
		printk(KERN_NOTICE DEVICE_NAME " invalid str passed into str_to_register_offset\n");

	return -1;
}

int str_to_interrupt_offset(const char *str)
{
	if (strcmp(str, "rfs") == 0)
		return RFS;
	else if (strcmp(str, "rft") == 0)
		return RFT;
	else if (strcmp(str, "rfe") == 0)
		return RFE;
	else if (strcmp(str, "rfo") == 0)
		return RFO;
	else if (strcmp(str, "rdo") == 0)
		return RDO;
	else if (strcmp(str, "rfl") == 0)
		return RFL;

	else if (strcmp(str, "tin") == 0)
		return TIN;

	else if (strcmp(str, "dr_hi") == 0)
		return DR_HI;
	else if (strcmp(str, "dt_hi") == 0)
		return DT_HI;
	else if (strcmp(str, "dr_fe") == 0)
		return DR_FE;
	else if (strcmp(str, "dt_fe") == 0)
		return DT_FE;
	else if (strcmp(str, "dr_stop") == 0)
		return DR_STOP;
	else if (strcmp(str, "dt_stop") == 0)
		return DT_STOP;

	else if (strcmp(str, "tft") == 0)
		return TFT;
	else if (strcmp(str, "alls") == 0)
		return ALLS;
	else if (strcmp(str, "tdu") == 0)
		return TDU;

	else if (strcmp(str, "ctss") == 0)
		return CTSS;
	else if (strcmp(str, "dsrc") == 0)
		return DSRC;
	else if (strcmp(str, "cdc") == 0)
		return CDC;
	else if (strcmp(str, "ctsa") == 0)
		return CTSA;

	else
		printk(KERN_NOTICE DEVICE_NAME " invalid str passed into str_to_interrupt_offset\n");

	return -1;
}

unsigned is_read_only_register(unsigned offset)
{
	switch (offset) {
	case STAR_OFFSET:
	case VSTR_OFFSET:
			return 1;
	}

	return 0;
}

unsigned port_offset(struct fscc_port *port, unsigned bar, unsigned offset)
{
	switch (bar) {
	case 0:
		return (port->channel == 0) ? offset : offset + 0x80;

	case 2:
		switch (offset) {
			case DMACCR_OFFSET:
				return (port->channel == 0) ? offset : offset + 0x04;

			case DMA_RX_BASE_OFFSET:
			case DMA_TX_BASE_OFFSET:
			case DMA_CURRENT_RX_BASE_OFFSET:
			case DMA_CURRENT_TX_BASE_OFFSET:
				return (port->channel == 0) ? offset : offset + 0x08;

			default:
				break;
		}

		break;

	default:
		break;
	}

	return offset;
}

unsigned port_exists(void *port)
{
	struct fscc_card *current_card = 0;
	struct fscc_port *current_port = 0;

	return_val_if_untrue(port, 0);

	list_for_each_entry(current_card, &fscc_cards, list) {
		struct list_head *ports = fscc_card_get_ports(current_card);

		list_for_each_entry(current_port, ports, list) {
			if (port == current_port)
				return 1;
		}
	}

	return 0;
}

unsigned is_fscc_device(struct pci_dev *pdev)
{
	switch (pdev->device) {
		case FSCC_ID:
		case SFSCC_ID:
		case SFSCC_104_LVDS_ID:
		case FSCC_232_ID:
		case SFSCC_104_UA_ID:
		case SFSCC_4_UA_ID:
		case SFSCC_UA_ID:
		case SFSCC_LVDS_ID:
		case FSCC_4_UA_ID:
		case SFSCC_4_LVDS_ID:
		case FSCC_UA_ID:
		case SFSCCe_4_ID:
		case SFSCC_4_UA_CPCI_ID:
		case SFSCC_4_UA_LVDS_ID:
		case SFSCC_UA_LVDS_ID:
		case FSCCe_4_UA_ID:
       		return 1;
	}

	return 0;
}

