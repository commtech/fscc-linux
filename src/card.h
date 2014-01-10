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

#ifndef FSCC_CARD
#define FSCC_CARD

#include <linux/pci.h> /* struct pci_dev */
#include <linux/fs.h> /* struct file_operations */
#include <linux/8250_pci.h> /* struct serial_private */

#define FCR_OFFSET 0x00
#define DSTAR_OFFSET 0x30

struct fscc_card {
	struct list_head list;
	struct list_head ports;
	struct pci_dev *pci_dev;

	void __iomem *bar[3];

	unsigned dma;
};

struct fscc_card *fscc_card_new(struct pci_dev *pdev,
								unsigned major_number,
								struct class *class,
								struct file_operations *fops);

void fscc_card_delete(struct fscc_card *card);
void fscc_card_suspend(struct fscc_card *card);
void fscc_card_resume(struct fscc_card *card);

struct fscc_card *fscc_card_find(struct pci_dev *pdev,
								 struct list_head *card_list);

void __iomem *fscc_card_get_BAR(struct fscc_card *card, unsigned number);

__u32 fscc_card_get_register(struct fscc_card *card, unsigned bar,
							 unsigned offset);

void fscc_card_set_register(struct fscc_card *card, unsigned bar,
							unsigned offset, __u32 value);

void fscc_card_get_register_rep(struct fscc_card *card, unsigned bar,
								unsigned offset, char *buf,
								unsigned byte_count);

void fscc_card_set_register_rep(struct fscc_card *card, unsigned bar,
								unsigned offset, const char *data,
								unsigned byte_count);

struct list_head *fscc_card_get_ports(struct fscc_card *card);
unsigned fscc_card_get_irq(struct fscc_card *card);
struct device *fscc_card_get_device(struct fscc_card *card);
char *fscc_card_get_name(struct fscc_card *card);

#endif
