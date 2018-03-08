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

#include <linux/version.h>
#include <asm/byteorder.h> /* __BIG_ENDIAN */
#include "card.h"
#include "port.h" /* struct fscc_port */
#include "utils.h" /* return_{val_}if_true */


/*
	This handles initialization on a card (2 port) level. So anything that
	isn't specific to a single port like managing the pci space and calling the
	port specific functions will happen here.
*/
struct fscc_card *fscc_card_new(struct pci_dev *pdev,
								unsigned major_number,
								struct class *class,
								struct file_operations *fops)
{
	struct fscc_card *card = 0;
	struct fscc_port *port_iter = 0;
	static unsigned minor_number = 0;
	unsigned i = 0;

	card = kmalloc(sizeof(*card), GFP_KERNEL);

	return_val_if_untrue(card != NULL, 0);

	INIT_LIST_HEAD(&card->list);
	INIT_LIST_HEAD(&card->ports);

	card->pci_dev = pdev;
	card->dma = 0;

	switch (pdev->device) {
	case SFSCC_ID:
	case SFSCC_104_LVDS_ID:
	case SFSCC_104_UA_ID:
	case SFSCC_4_UA_ID:
	case SFSCC_UA_ID:
	case SFSCC_LVDS_ID:
	case SFSCC_4_LVDS_ID:
	case SFSCCe_4_ID:
	case SFSCC_4_UA_CPCI_ID:
	case SFSCC_UA_LVDS_ID:
	case SFSCC_4_UA_LVDS_ID:
	case SFSCCe_4_LVDS_UA_ID:
		if (pci_set_dma_mask(pdev, 0xffffffff)) {
			dev_warn(&card->pci_dev->dev, "no suitable DMA available\n");
		}
		else {
			card->dma = 1;
			pci_set_master(card->pci_dev);
		}

		break;
	}

	/* BAR 1 is handled by the serial driver so shouldn't be requested here */
	if (pci_request_region(card->pci_dev, 0, DEVICE_NAME) != 0) {
		dev_err(&card->pci_dev->dev, "pci_request_regions failed\n");
		return 0;
	}

	if (pci_request_region(card->pci_dev, 2, DEVICE_NAME) != 0) {
		dev_err(&card->pci_dev->dev, "pci_request_regions failed\n");
		return 0;
	}

	if (pci_set_dma_mask(pdev, 0xffffffff)) {
		dev_warn(&card->pci_dev->dev, "no suitable DMA available\n");
		return 0;
	}

	/* There are three register sections. */
	for (i = 0; i < 3; i++) {
		card->bar[i] = pci_iomap(card->pci_dev, i, 0);

		if (card->bar[i] == NULL) {
			dev_err(&card->pci_dev->dev, "pci_iomap failed on bar #%i\n", i);
			return 0;
		}
	}

	/* There are two ports per card. */
	for (i = 0; i < 2; i++) {
		port_iter = fscc_port_new(card, i, major_number, minor_number,
								  &card->pci_dev->dev, class, fops);

		if (port_iter)
			list_add_tail(&port_iter->list, &card->ports);

		minor_number += 1;
	}

	return card;
}

void fscc_card_delete(struct fscc_card *card)
{
	struct list_head *current_node = 0;
	struct list_head *temp_node = 0;

	return_if_untrue(card);

	list_for_each_safe(current_node, temp_node, &card->ports) {
		struct fscc_port *current_port = 0;

		current_port = list_entry(current_node, struct fscc_port, list);

		list_del(current_node);
		fscc_port_delete(current_port);
	}

	pci_release_region(card->pci_dev, 0);
	pci_release_region(card->pci_dev, 2);

	kfree(card);
}

void fscc_card_suspend(struct fscc_card *card)
{
	struct fscc_port *current_port = 0;

	return_if_untrue(card);

	list_for_each_entry(current_port, &card->ports, list) {
		fscc_port_suspend(current_port);
	}
}

void fscc_card_resume(struct fscc_card *card)
{
	struct fscc_port *current_port = 0;

	return_if_untrue(card);

	list_for_each_entry(current_port, &card->ports, list) {
		fscc_port_resume(current_port);
	}
}

struct fscc_card *fscc_card_find(struct pci_dev *pdev,
								 struct list_head *card_list)
{
	struct fscc_card *current_card = 0;

	return_val_if_untrue(pdev, 0);
	return_val_if_untrue(card_list, 0);

	list_for_each_entry(current_card, card_list, list) {
		if (current_card->pci_dev == pdev)
			return current_card;
	}

	return 0;
}

void __iomem *fscc_card_get_BAR(struct fscc_card *card, unsigned number)
{
	return_val_if_untrue(card, 0);

	if (number > 2)
		return 0;

	return card->bar[number];
}

/*
	At the card level there is no offset manipulation to get to the second port
	on each card. If you would like to pass in a register offset and get the
	appropriate address on a port basis use the fscc_port_* functions.
*/
__u32 fscc_card_get_register(struct fscc_card *card, unsigned bar,
							 unsigned offset)
{
	void __iomem *address = 0;
	__u32 value = 0;

	return_val_if_untrue(card, 0);
	return_val_if_untrue(bar <= 2, 0);

	address = fscc_card_get_BAR(card, bar);

	value = ioread32(address + offset);
	value = le32_to_cpu(value);

	return value;
}

/*
	At the card level there is no offset manipulation to get to the second port
	on each card. If you would like to pass in a register offset and get the
	appropriate address on a port basis use the fscc_port_* functions.
*/
void fscc_card_set_register(struct fscc_card *card, unsigned bar,
							unsigned offset, __u32 value)
{
	void __iomem *address = 0;

	return_if_untrue(card);
	return_if_untrue(bar <= 2);

	address = fscc_card_get_BAR(card, bar);

	value = cpu_to_le32(value);

	iowrite32(value, address + offset);
}

/*
	At the card level there is no offset manipulation to get to the second port
	on each card. If you would like to pass in a register offset and get the
	appropriate address on a port basis use the fscc_port_* functions.
*/
void fscc_card_get_register_rep(struct fscc_card *card, unsigned bar,
								unsigned offset, char *buf,
								unsigned byte_count)
{
	void __iomem *address = 0;
	unsigned leftover_count = 0;
	__u32 incoming_data = 0;
	unsigned chunks = 0;

	return_if_untrue(card);
	return_if_untrue(bar <= 2);
	return_if_untrue(buf);
	return_if_untrue(byte_count > 0);

	address = fscc_card_get_BAR(card, bar);
	leftover_count = byte_count % 4;
	chunks = (byte_count - leftover_count) / 4;

	ioread32_rep(address + offset, buf, chunks);

	if (leftover_count) {
		incoming_data = ioread32(address + offset);

		memmove(buf + (byte_count - leftover_count),
				(char *)(&incoming_data), leftover_count);
	}

#ifdef __BIG_ENDIAN
	{
		unsigned i = 0;

		for (i = 0; i < (int)(byte_count / 2); i++) {
			char first, last;

			first = buf[i];
			last = buf[byte_count - i - 1];

			buf[i] = last;
			buf[byte_count - i - 1] = first;
		}
	}
#endif
}

/*
	At the card level there is no offset manipulation to get to the second port
	on each card. If you would like to pass in a register offset and get the
	appropriate address on a port basis use the fscc_port_* functions.
*/
void fscc_card_set_register_rep(struct fscc_card *card, unsigned bar,
								unsigned offset, const char *data,
								unsigned byte_count)
{
	void __iomem *address = 0;
	unsigned leftover_count = 0;
	unsigned chunks = 0;
	char *reversed_data = 0;
	const char *outgoing_data = 0;

	return_if_untrue(card);
	return_if_untrue(bar <= 2);
	return_if_untrue(data);
	return_if_untrue(byte_count > 0);

	address = fscc_card_get_BAR(card, bar);
	leftover_count = byte_count % 4;
	chunks = (byte_count - leftover_count) / 4;

	outgoing_data = data;

#ifdef __BIG_ENDIAN
	{
		unsigned i = 0;

		reversed_data = kmalloc(byte_count, GFP_KERNEL);

		for (i = 0; i < byte_count; i++)
			reversed_data[i] = data[byte_count - i - 1];

		outgoing_data = reversed_data;
	}
#endif

	iowrite32_rep(address + offset, outgoing_data, chunks);

	if (leftover_count)
		iowrite32(chars_to_u32(outgoing_data + (byte_count - leftover_count)), address + offset);

	if (reversed_data)
		kfree(reversed_data);
}

struct list_head *fscc_card_get_ports(struct fscc_card *card)
{
	return_val_if_untrue(card, 0);

	return &card->ports;
}

unsigned fscc_card_get_irq(struct fscc_card *card)
{
	return_val_if_untrue(card, 0);

	return card->pci_dev->irq;
}

struct device *fscc_card_get_device(struct fscc_card *card)
{
	return_val_if_untrue(card, 0);

	return &card->pci_dev->dev;
}

char *fscc_card_get_name(struct fscc_card *card)
{
	switch (card->pci_dev->device) {
	case FSCC_ID:
	case FSCC_UA_ID:
		return "FSCC PCI";
	case SFSCC_ID:
	case SFSCC_UA_ID:
		return "SuperFSCC PCI";
	case SFSCC_104_LVDS_ID:
		return "SuperFSCC-104-LVDS PC/104+";
	case FSCC_232_ID:
		return "FSCC-232 PCI";
	case SFSCC_104_UA_ID:
		return "SuperFSCC-104 PC/104+";
	case SFSCC_4_UA_ID:
		return "SuperFSCC/4 PCI";
	case SFSCC_LVDS_ID:
	case SFSCC_UA_LVDS_ID:
		return "SuperFSCC-LVDS PCI";
	case FSCC_4_UA_ID:
		return "FSCC/4 PCI";
	case SFSCC_4_LVDS_ID:
	case SFSCC_4_UA_LVDS_ID:
		return "SuperFSCC/4-LVDS PCI";
	case SFSCCe_4_ID:
		return "SuperFSCC/4 PCIe";
	case SFSCC_4_CPCI_ID:
	case SFSCC_4_UA_CPCI_ID:
		return "SuperFSCC/4 cPCI";
	case FSCCe_4_UA_ID:
		return "FSCC/4 PCIe";
	case SFSCCe_4_LVDS_UA_ID:
		return "SuperFSCC/4-LVDS PCIe";
	default:
		return "Unknown Device";
	}

}

