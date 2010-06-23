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

#include <linux/version.h>
#include "card.h"
#include "port.h" /* struct fscc_port */
#include "config.h" /* DEVICE_NAME */
#include "utils.h" /* return_{val_}if_true */

unsigned minor_number = 0;

struct pciserial_board pci_board = {
	.flags = FL_BASE1,
	.num_ports = 2,
	.base_baud = 921600,
	.uart_offset = 8,
};

struct fscc_card *fscc_card_new(struct pci_dev *pdev,
                                unsigned major_number,
                                struct class *class,
                                struct file_operations *fops,
                                const struct pci_device_id *id)
{
	struct fscc_card *card = 0;
	struct fscc_port *port_iter = 0;
	unsigned start_minor_number = 0;
	unsigned i = 0;
	
	card = kmalloc(sizeof(*card), GFP_KERNEL);
	
	return_val_if_untrue(card != NULL, 0);
	
	INIT_LIST_HEAD(&card->list);
	INIT_LIST_HEAD(&card->ports);
	
	card->pci_dev = pdev;
	card->dma = 0;
	
#ifdef EXPERIMENTAL_DMA
	switch (id->device) {
		case SFSCC_ID:
		case SFSCC_4_ID:
		case SFSCC_4_LVDS_ID:
		case SFSCCe_4_ID:	
			if (pci_set_dma_mask(pdev, 0xffffffff)) {
				dev_warn(&card->pci_dev->dev, "no suitable DMA available\n");
			}
			else {
				card->dma = 1;
				pci_set_master(card->pci_dev);
			}
				
			break;
	}
#endif
	
	/* This requests the pci regions for us. Doing so again will cause our
	   uarts not to appear correctly. */
	card->serial_priv = pciserial_init_ports(pdev, &pci_board);
	
	if (IS_ERR(card->serial_priv)) {
		dev_err(&card->pci_dev->dev, "pciserial_init_ports failed\n");
		return 0;
	}
	
	pci_set_drvdata(pdev, card->serial_priv);
	
	if (pci_set_dma_mask(pdev, 0xffffffff)) {
		dev_warn(&card->pci_dev->dev, "no suitable DMA available\n");
		return 0;
	}
	
	start_minor_number = minor_number;
	
	for (i = 0; i < 3; i++) {
	
//#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 28)
//		card->bar[i] = pci_ioremap_bar(card->pci_dev, i);
//#else
		card->bar[i] = pci_iomap(card->pci_dev, i, 0);
//#endif
		
		if (card->bar[i] == NULL) {
			dev_err(&card->pci_dev->dev, "pci_iomap failed on bar #%i\n", i);			       
			return 0;
		}
	}
	
	fscc_card_set_register(card, 2, FCR_OFFSET, DEFAULT_FCR_VALUE);
	
	for (i = 0; i < 2; i++) {
		port_iter = fscc_port_new(card, i, major_number, minor_number, 
		                          &card->pci_dev->dev, class, fops);
		                                                  
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
		fscc_port_delete(current_port);
	}

	pciserial_remove_ports(card->serial_priv);
	pci_set_drvdata(card->pci_dev, NULL);
	
	kfree(card);
}

unsigned fscc_card_get_memory_usage(struct fscc_card *card)
{
	struct list_head *current_node = 0;
	struct list_head *temp_node = 0;
	unsigned memory = 0;
		
	return_val_if_untrue(card, 0);
	
	list_for_each_safe(current_node, temp_node, &card->ports) {
		struct fscc_port *current_port = 0;
		current_port = list_entry(current_node, struct fscc_port, list);
		memory += fscc_port_get_memory_usage(current_port);
	}
	
	return memory;
}

void fscc_card_suspend(struct fscc_card *card)
{
	struct fscc_port *current_port = 0;
		
	return_if_untrue(card);
	
	pciserial_suspend_ports(card->serial_priv);
			
	list_for_each_entry(current_port, &card->ports, list) {	
		fscc_port_suspend(current_port);
	}
	
	card->fcr_storage = fscc_card_get_register(card, 2, FCR_OFFSET);
}

void fscc_card_resume(struct fscc_card *card)
{
	struct fscc_port *current_port = 0;
	__u32 current_value = 0;	
		
	return_if_untrue(card);
	
	pciserial_resume_ports(card->serial_priv);
			
	list_for_each_entry(current_port, &card->ports, list) {	
		fscc_port_resume(current_port);
	}
	
	current_value = fscc_card_get_register(card, 2, FCR_OFFSET);
	
	dev_dbg(&card->pci_dev->dev, "FCR restoring 0x%08x => 0x%08x\n", 
            current_value, card->fcr_storage);
	
	fscc_card_set_register(card, 2, FCR_OFFSET, card->fcr_storage);
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

__u32 fscc_card_get_register(struct fscc_card *card, unsigned bar, 
                             unsigned offset)
{
	void __iomem *address = 0;
	__u32 value = 0;
	
	return_val_if_untrue(card, 0);
	return_val_if_untrue(bar <= 2, 0);
	
	address = fscc_card_get_BAR(card, bar);	
	value = ioread32(address + offset);
	
	return value;
}

void fscc_card_get_register_rep(struct fscc_card *card, unsigned bar, 
                                unsigned offset, char *buf,
                                unsigned long chunks)
{
	void __iomem *address = 0;
	
	return_if_untrue(card);
	return_if_untrue(bar <= 2);
	return_if_untrue(buf);
	return_if_untrue(chunks > 0);
	
	address = fscc_card_get_BAR(card, bar);
	
	ioread32_rep(address + offset, buf, chunks);
}

void fscc_card_set_register(struct fscc_card *card, unsigned bar, 
                            unsigned offset, __u32 value)
{
	void __iomem *address = 0;
	
	return_if_untrue(card);
	return_if_untrue(bar <= 2);
	
	address = fscc_card_get_BAR(card, bar);

	iowrite32(value, address + offset);
}

void fscc_card_set_register_rep(struct fscc_card *card, unsigned bar,
                                unsigned offset, const char *data,
                                unsigned long chunks) 
{
	void __iomem *address = 0;
	
	return_if_untrue(card);
	return_if_untrue(bar <= 2);
	return_if_untrue(data);
	return_if_untrue(chunks > 0);
	
	address = fscc_card_get_BAR(card, bar);
	
	iowrite32_rep(address + offset, data, chunks);
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

