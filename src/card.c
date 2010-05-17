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

#include "card.h"
#include "port.h"
#include "config.h"
#include "isr.h"

unsigned minor_number = 0;

struct fscc_card *fscc_card_new(struct pci_dev *pdev,
                                const struct pci_device_id *id,
                                unsigned major_number,
                                struct class *class,
                                struct file_operations *fops)
{
	struct fscc_card *card = 0;
	struct fscc_port *port_iter = 0;
	unsigned start_minor_number = 0;
	unsigned i = 0;
	
	card = (struct fscc_card*)kmalloc(sizeof(struct fscc_card), GFP_KERNEL);
	
	INIT_LIST_HEAD(&card->list);
	INIT_LIST_HEAD(&card->ports);
	
	card->pci_dev = pdev;
	
	if (pci_request_regions(card->pci_dev, DEVICE_NAME) != 0) {
		printk(KERN_ERR DEVICE_NAME " pci_request_regions failed\n");
		return 0;
	}
	
	pci_set_drvdata(card->pci_dev, card);
	
	start_minor_number = minor_number;
	
	for (i = 0; i < 3; i++) {
		card->bar[i] = pci_iomap(card->pci_dev, i, 0);
		
		if (card->bar[i] == NULL) {
			printk(KERN_ERR DEVICE_NAME " pci_iomap failed on bar #%i\n", i);
			return 0;
		}
	}
	
	for (i = 0; i < 2; i++) {
		port_iter = fscc_port_new(card, i, major_number, minor_number, 
		                          class, fops);
		
		if (port_iter)                         
			printk(KERN_INFO "%s revision %x.%02x\n", port_iter->name,
				   fscc_port_get_PREV(port_iter), fscc_port_get_FREV(port_iter));
		                          
		minor_number += 1;        
	}
	
	return card;
}

void fscc_card_delete(struct fscc_card *card)
{
	struct list_head *current_node = 0;
	struct list_head *temp_node = 0;
	
	if (card == 0)
		return;
		
	pci_set_drvdata(card->pci_dev, 0);	
	pci_release_regions(card->pci_dev);
	
	list_for_each_safe(current_node, temp_node, &card->ports) {
		struct fscc_port *current_port = 0;
		current_port = list_entry(current_node, struct fscc_port, list);
		fscc_port_delete(current_port);
	}
	
	kfree(card);
}

void fscc_card_suspend(struct fscc_card *card)
{
	struct fscc_port *current_port = 0;
			
	list_for_each_entry(current_port, &card->ports, list) {	
		fscc_port_store_registers(current_port);
	}
}

void fscc_card_resume(struct fscc_card *card)
{
	struct fscc_port *current_port = 0;
			
	list_for_each_entry(current_port, &card->ports, list) {	
		fscc_port_restore_registers(current_port);
	}
}

struct fscc_card *fscc_card_find(struct pci_dev *pdev, 
                                 struct list_head *card_list)
{
	struct fscc_card *current_card = 0;
	
	list_for_each_entry(current_card, card_list, list) {
		if (current_card->pci_dev == pdev)
			return current_card;
	}
	
	return 0;
}

void __iomem *fscc_card_get_BAR(struct fscc_card *card, unsigned number)
{
	if (number > 2)
		return 0;

	return card->bar[number];
}

