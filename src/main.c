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

#include <linux/fs.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/pci.h>
#include <linux/cdev.h>
#include <linux/serial_core.h>
#include <linux/interrupt.h>
#include <linux/poll.h>
#include "card.h"
#include "port.h"
#include "isr.h"
#include "config.h"
#include "utils.h"
#include "fscc.h"

unsigned fscc_get_next_minor_number(struct list_head *card_list);

static int fscc_major_number;
static struct class *fscc_class = 0;

LIST_HEAD(fscc_cards);

struct pci_device_id fscc_id_table[] __devinitdata = {
	{ COMMTECH_VENDOR_ID, FSCC_ID, PCI_ANY_ID, 0, 0, 0 },
	{ COMMTECH_VENDOR_ID, FSCC_232_ID, PCI_ANY_ID, 0, 0, 0 },
	{ COMMTECH_VENDOR_ID, FSCC_4_ID, PCI_ANY_ID, 0, 0, 0 },
	{ COMMTECH_VENDOR_ID, SFSCC_ID, PCI_ANY_ID, 0, 0, 0 },
	{ COMMTECH_VENDOR_ID, SFSCC_4_ID, PCI_ANY_ID, 0, 0, 0 },
	{ COMMTECH_VENDOR_ID, SFSCC_4_LVDS_ID, PCI_ANY_ID, 0, 0, 0 },
	{ 0, },
};

unsigned fscc_get_next_minor_number(struct list_head *card_list) 
{
	struct fscc_card *current_card = 0;
	struct fscc_port *current_port = 0;
	int highest_minor_number = -1;
	int current_minor_number = -1;
	
	list_for_each_entry(current_card, card_list, list) {		
		list_for_each_entry(current_port, &current_card->ports, list) {	
			current_minor_number = (int)fscc_port_get_minor_number(current_port);
			if (current_minor_number > highest_minor_number)
				highest_minor_number = current_minor_number;
		}
	}
	
	return (unsigned)(highest_minor_number + 1);
}

int fscc_open(struct inode *inode, struct file *file)
{
	struct fscc_port *current_port = 0;
	
	current_port = container_of(inode->i_cdev, struct fscc_port, cdev);	
	file->private_data = current_port;
	
	return 0;
}

//Returns ENOBUFS if read size is smaller than next frame
ssize_t fscc_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	struct fscc_port *current_port = 0;
	ssize_t read_count = 0;
	
	current_port = file->private_data;
	
	if (down_interruptible(&current_port->semaphore))
		return -ERESTARTSYS;
	
	while (!fscc_port_iframes_ready(current_port)) {
		up(&current_port->semaphore);
		
		if (file->f_flags & O_NONBLOCK)
			return -EAGAIN;
			
		if (wait_event_interruptible(current_port->input_queue, fscc_port_iframes_ready(current_port)))
			return -ERESTARTSYS;
			
		if (down_interruptible(&current_port->semaphore))
			return -ERESTARTSYS;
	}
	
	read_count = fscc_port_read(current_port, buf, count);
	
	up(&current_port->semaphore);
	
	return read_count;
}

/* TODO: Handle blocking situation when there is no memory available */
ssize_t fscc_write(struct file *file, const char *buf, size_t count, 
                   loff_t *ppos)
{
	struct fscc_port *current_port = 0;
	int err = 0;
	
	current_port = file->private_data;
	
	if (down_interruptible(&current_port->semaphore))
		return -ERESTARTSYS;
	
	err = fscc_port_write(current_port, buf, count);
	
	switch (err) {
		case ENOMEM:
			return err;
	}
	
	up(&current_port->semaphore);
	
	return count;
}

//TODO: This needs to determine a set of rules of when we should block
unsigned fscc_poll(struct file *file, struct poll_table_struct *wait)
{
	struct fscc_port *current_port = 0;
	unsigned mask = 0;
	
	current_port = file->private_data;
	
	down(&current_port->semaphore);
	
	poll_wait(file, &current_port->input_queue, wait);
	poll_wait(file, &current_port->output_queue, wait);
	
	if (fscc_port_iframes_ready(current_port))
		mask |= POLLIN | POLLRDNORM;
	
	up(&current_port->semaphore);
	
	return mask;
}

int fscc_ioctl(struct inode *inode, struct file *file, unsigned int cmd, 
               unsigned long arg)
{
	struct fscc_port *current_port = 0;
	
	current_port = file->private_data;
	
	switch (cmd) {			
		case FSCC_GET_REGISTERS: {
				unsigned i = 0;
			
				for (i = 0; i < sizeof(struct fscc_registers) / sizeof(int32_t); i++) {
					if (((int32_t *)arg)[i] != FSCC_UPDATE_VALUE)
						continue;
						
					((uint32_t *)arg)[i] = fscc_port_get_register(current_port, 0, i * 4);
				}
			}			
			break;

		case FSCC_SET_REGISTERS: {
				unsigned i = 0;
			
				for (i = 0; i < sizeof(struct fscc_registers) / sizeof(int32_t); i++) {
					if (((int32_t *)arg)[i] < 0)
						continue;
						
					fscc_port_set_register(current_port, 0, i * 4, ((uint32_t *)arg)[i]);
				}
			}
			break;
			
		default:
			printk(KERN_DEBUG DEVICE_NAME " unknown ioctl\n");
			return -ENOTTY;			
	}
	
	return 0;
}

static struct file_operations fscc_fops = {
	.owner = THIS_MODULE,
	.open = fscc_open,
	.read = fscc_read,
	.write = fscc_write,
	.poll = fscc_poll,
	.ioctl = fscc_ioctl,
};

#define PORTS_PER_CARD 2

static int __devinit fscc_probe(struct pci_dev *pdev, 
                                const struct pci_device_id *id)
{
	struct fscc_card *new_card = 0;
	int next_minor_number = 0;
	
	if (pci_enable_device(pdev))
		return -EIO;
	
	switch (id->device) {
		case FSCC_ID:			
		case FSCC_232_ID:
		case FSCC_4_ID:
		case SFSCC_ID:
		case SFSCC_4_ID:
		case SFSCC_4_LVDS_ID:
			next_minor_number = fscc_get_next_minor_number(&fscc_cards);
			
			new_card = fscc_card_new(pdev, id, fscc_major_number, 
			                         (unsigned)next_minor_number, fscc_class,
			                         &fscc_fops);
			                         
			list_add_tail(&new_card->list, &fscc_cards);
			break;
			
		default:
			printk(KERN_DEBUG DEVICE_NAME "unknown device\n");
	}

	return 0;
}

static void __devexit fscc_remove(struct pci_dev *pdev)
{
	struct fscc_card *card = 0;
	
	card = fscc_card_find(pdev, &fscc_cards);
	
	if (card == 0) {
		printk(KERN_DEBUG DEVICE_NAME " card not found\n");
		return;
	}
	
	list_del(&card->list);
	fscc_card_delete(card);	
	pci_disable_device(pdev);
}

static int fscc_suspend(struct pci_dev *pdev, pm_message_t state)
{
	struct fscc_card *card = 0;
		
	card = fscc_card_find(pdev, &fscc_cards);
	fscc_card_suspend(card);

	pci_save_state(pdev);
	pci_set_power_state(pdev, pci_choose_state(pdev, state));
	
	return 0;
}

static int fscc_resume(struct pci_dev *pdev)
{
	struct fscc_card *card = 0;	

	pci_set_power_state(pdev, PCI_D0);
	pci_restore_state(pdev);
	
	card = fscc_card_find(pdev, &fscc_cards);
	fscc_card_resume(card);

	return 0;
}

struct pci_driver fscc_pci_driver = {
	.name = "fscc",
	.probe = fscc_probe,
	.remove = fscc_remove,
	.suspend = fscc_suspend,
	.resume = fscc_resume,
	.id_table = fscc_id_table,
};

static int __init fscc_init(void)
{
	unsigned err;
	
	fscc_class = class_create(THIS_MODULE, DEVICE_NAME);
	
	if (IS_ERR(fscc_class)) {
		printk(KERN_ERR DEVICE_NAME " class_create failed\n");
		return PTR_ERR(fscc_class);
	}
	
	fscc_major_number = register_chrdev(0, "fscc", &fscc_fops);
	
	if (fscc_major_number < 0) {
		printk(KERN_ERR DEVICE_NAME " register_chrdev failed\n");
		class_destroy(fscc_class);
		return -1;
	}
	
	err = pci_register_driver(&fscc_pci_driver);
	
	if (err < 0) {
		printk(KERN_ERR DEVICE_NAME " pci_register_driver failed");
		unregister_chrdev(fscc_major_number, "fscc");
		class_destroy(fscc_class);
		return err;
	}
	
	return 0;
}

static void __exit fscc_exit(void)
{	
	pci_unregister_driver(&fscc_pci_driver);
	unregister_chrdev(fscc_major_number, "fscc");
	class_destroy(fscc_class);
}

MODULE_DEVICE_TABLE(pci, fscc_id_table);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Driver for the FSCC series of cards from Commtech, Inc."); 
MODULE_VERSION("2.0");

module_init(fscc_init);
module_exit(fscc_exit);  
