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

#define FCR_OFFSET 0x00
#define DMACCR_OFFSET 0x04

struct fscc_card *fscc_card_new(struct pci_dev *pdev,
                                const struct pci_device_id *id,
                                unsigned major_number,
                                unsigned minor_number_start,
                                struct class *class,
                                struct file_operations *fops)
{
	struct fscc_card *new_card = 0;
	struct fscc_port *port_iter = 0;
	unsigned i = 0;
	
	new_card = (struct fscc_card*)kmalloc(sizeof(struct fscc_card), GFP_KERNEL);
	
	INIT_LIST_HEAD(&new_card->list);
	INIT_LIST_HEAD(&new_card->ports);
	
	new_card->pci_dev = pdev;
	
	switch (id->device) {
		case FSCC_ID:
			new_card->type = FSCC;
			break;
			
		case FSCC_232_ID:
			new_card->type = FSCC_232;
			break;
			
		case FSCC_4_ID:
			new_card->type = FSCC_4;
			break;
			
		case SFSCC_ID:	
			new_card->type = SFSCC;
			break;
			
		case SFSCC_4_ID:	
			new_card->type = SFSCC_4;
			break;
			
		default:
			printk(KERN_NOTICE DEVICE_NAME " unknown device %02x\n", id->device);
	}
	
	pci_request_regions(new_card->pci_dev, DEVICE_NAME);	
	pci_set_drvdata(new_card->pci_dev, new_card);
	
	for (i = 0; i < 2; i++)
		port_iter = fscc_port_new(new_card, i, major_number, minor_number_start + i, class, fops);
	
	printk(KERN_INFO DEVICE_NAME " revision %x.%02x\n", 
	       fscc_port_get_PREV(port_iter), fscc_port_get_FREV(port_iter));
	
	return new_card;
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
	
	pciserial_suspend_ports(card->serial_private);
}

void fscc_card_resume(struct fscc_card *card)
{
	struct fscc_port *current_port = 0;
			
	list_for_each_entry(current_port, &card->ports, list) {	
		fscc_port_restore_registers(current_port);
	}
	
	pciserial_resume_ports(card->serial_private);
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

unsigned long fscc_card_get_BAR(struct fscc_card *card, unsigned number)
{
	if (number > 2)
		return 0;

	return pci_resource_start(card->pci_dev, number);
}

