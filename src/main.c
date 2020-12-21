/*
	Copyright (c) 2019 Commtech, Inc.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include <linux/poll.h> /* poll_wait, POLL* */
#include "card.h" /* struct fscc_card */
#include "port.h" /* struct fscc_port */
#include "config.h" /* DEVICE_NAME, DEFAULT_* */
#include "utils.h" /* is_fscc_device */

#if defined(__BIG_ENDIAN) && defined(__LITTLE_ENDIAN)
	#error Both __BIG_ENDIAN and __LITTLE_ENDIAN are defined
#endif

#if !defined(__BIG_ENDIAN) && !defined(__LITTLE_ENDIAN)
	#error Neither __BIG_ENDIAN or __LITTLE_ENDIAN are defined
#endif

static int fscc_major_number;
static struct class *fscc_class = 0;

unsigned force_fifo = DEFAULT_FORCE_FIFO_VALUE;

LIST_HEAD(fscc_cards);

struct pci_device_id fscc_id_table[] = {
	{ COMMTECH_VENDOR_ID, FSCC_ID, PCI_ANY_ID, 0, 0, 0 },
	{ COMMTECH_VENDOR_ID, SFSCC_ID, PCI_ANY_ID, 0, 0, 0 },
	{ COMMTECH_VENDOR_ID, SFSCC_104_LVDS_ID, PCI_ANY_ID, 0, 0, 0 },
	{ COMMTECH_VENDOR_ID, FSCC_232_ID, PCI_ANY_ID, 0, 0, 0 },
	{ COMMTECH_VENDOR_ID, SFSCC_104_UA_ID, PCI_ANY_ID, 0, 0, 0 },
	{ COMMTECH_VENDOR_ID, SFSCC_4_UA_ID, PCI_ANY_ID, 0, 0, 0 },
	{ COMMTECH_VENDOR_ID, SFSCC_UA_ID, PCI_ANY_ID, 0, 0, 0 },
	{ COMMTECH_VENDOR_ID, SFSCC_LVDS_ID, PCI_ANY_ID, 0, 0, 0 },
	{ COMMTECH_VENDOR_ID, FSCC_4_UA_ID, PCI_ANY_ID, 0, 0, 0 },
	{ COMMTECH_VENDOR_ID, SFSCC_4_LVDS_ID, PCI_ANY_ID, 0, 0, 0 },
	{ COMMTECH_VENDOR_ID, FSCC_UA_ID, PCI_ANY_ID, 0, 0, 0 },
	{ COMMTECH_VENDOR_ID, SFSCCe_4_ID, PCI_ANY_ID, 0, 0, 0 },
	{ COMMTECH_VENDOR_ID, SFSCC_4_UA_CPCI_ID, PCI_ANY_ID, 0, 0, 0 },
	{ COMMTECH_VENDOR_ID, SFSCC_4_UA_LVDS_ID, PCI_ANY_ID, 0, 0, 0 },
	{ COMMTECH_VENDOR_ID, SFSCC_UA_LVDS_ID, PCI_ANY_ID, 0, 0, 0 },
	{ COMMTECH_VENDOR_ID, FSCCe_4_UA_ID, PCI_ANY_ID, 0, 0, 0 },
	{ COMMTECH_VENDOR_ID, SFSCCe_4_LVDS_UA_ID, PCI_ANY_ID, 0, 0, 0 },
	{ 0, },
};

int fscc_open(struct inode *inode, struct file *file)
{
	struct fscc_port *current_port = 0;

	current_port = container_of(inode->i_cdev, struct fscc_port, cdev);
	file->private_data = current_port;

	return 0;
}

/*
	Returns -ENOBUFS if read size is smaller than next frame
	Returns -EOPNOTSUPP if in async mode
*/
ssize_t fscc_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	struct fscc_port *port = 0;
	ssize_t read_count = 0;

	port = file->private_data;

	if (count == 0)
		return count;

	if (fscc_port_using_async(port)) {
		dev_warn(port->device, "use /dev/ttySx nodes while in async mode\n");
		return -EOPNOTSUPP;
	}

	if (down_interruptible(&port->read_semaphore))
		return -ERESTARTSYS;

	while (!fscc_port_has_incoming_data(port)) {
		up(&port->read_semaphore);

		if (file->f_flags & O_NONBLOCK)
			return -EAGAIN;

		if (wait_event_interruptible(port->input_queue,
									 fscc_port_has_incoming_data(port))) {
			return -ERESTARTSYS;
		}

		if (down_interruptible(&port->read_semaphore))
			return -ERESTARTSYS;
	}

	read_count = fscc_port_read(port, buf, count);

	up(&port->read_semaphore);

	return read_count;
}

/*
	Returns -ENOBUFS if write size is larger than memory_cap
*/
ssize_t fscc_write(struct file *file, const char *buf, size_t count,
				   loff_t *ppos)
{
	struct fscc_port *port = 0;
	int error_code = 0;

	port = file->private_data;

	if (count == 0)
		return count;

	if (fscc_port_using_async(port)) {
		dev_warn(port->device, "use /dev/ttySx nodes while in async mode\n");
		return -EOPNOTSUPP;
	}

	if (count > fscc_port_get_output_memory_cap(port))
		return -ENOBUFS;

	if (down_interruptible(&port->write_semaphore))
		return -ERESTARTSYS;

	while (fscc_port_get_output_memory_usage(port) + count > fscc_port_get_output_memory_cap(port)) {
		up(&port->write_semaphore);

		if (file->f_flags & O_NONBLOCK)
			return -EAGAIN;

		if (wait_event_interruptible(port->output_queue,
				fscc_port_get_output_memory_usage(port) + count <= fscc_port_get_output_memory_cap(port))) {
			return -ERESTARTSYS;
		}

		if (down_interruptible(&port->write_semaphore))
			return -ERESTARTSYS;
	}

	error_code = fscc_port_write(port, buf, count);

	up(&port->write_semaphore);

	return (error_code < 0) ? error_code : count;
}

unsigned fscc_poll(struct file *file, struct poll_table_struct *wait)
{
	struct fscc_port *port = 0;
	unsigned mask = 0;

	port = file->private_data;

	down(&port->poll_semaphore);

	poll_wait(file, &port->input_queue, wait);
	poll_wait(file, &port->output_queue, wait);

	if (fscc_port_has_incoming_data(port))
		mask |= POLLIN | POLLRDNORM;

	if (fscc_port_get_output_memory_usage(port) < fscc_port_get_output_memory_cap(port))
		mask |= POLLOUT | POLLWRNORM;

	up(&port->poll_semaphore);

	return mask;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 11)
long fscc_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#else
int fscc_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
			   unsigned long arg)
#endif
{
	struct fscc_port *port = 0;
	int error_code = 0;
	unsigned long flags;
	char clock_bits[20];	
	unsigned tmp=0;
	struct fscc_registers regs;
	struct fscc_memory_cap tmp_memcap;
    	unsigned copy_success = 0;


	port = file->private_data;

	switch (cmd) {
	case FSCC_GET_REGISTERS:
		copy_from_user(&regs, (struct fscc_registers *)arg, sizeof(struct fscc_registers));
		spin_lock_irqsave(&port->board_settings_spinlock, flags);
		fscc_port_get_registers(port, &regs);
		spin_unlock_irqrestore(&port->board_settings_spinlock, flags);
		copy_to_user((struct fscc_registers *)arg, &regs, sizeof(struct fscc_registers));
		break;

	case FSCC_SET_REGISTERS:
		copy_from_user(&regs, (struct fscc_registers *)arg, sizeof(struct fscc_registers));
		spin_lock_irqsave(&port->board_settings_spinlock, flags);
		fscc_port_set_registers(port, &regs);
		spin_unlock_irqrestore(&port->board_settings_spinlock, flags);
		break;

	case FSCC_PURGE_TX:
		if ((error_code = fscc_port_purge_tx(port)) < 0)
			return error_code;
		break;

	case FSCC_PURGE_RX:
		if ((error_code = fscc_port_purge_rx(port)) < 0)
			return error_code;
		break;

	case FSCC_ENABLE_APPEND_STATUS:
		if ((error_code = fscc_port_set_append_status(port, 1)) < 0)
			return error_code;
		break;

	case FSCC_DISABLE_APPEND_STATUS:
		fscc_port_set_append_status(port, 0);
		break;

	case FSCC_GET_APPEND_STATUS:
		tmp = fscc_port_get_append_status(port);
		copy_to_user((void *)arg, &tmp, sizeof(tmp));
		break;

	case FSCC_ENABLE_APPEND_TIMESTAMP:
		if ((error_code = fscc_port_set_append_timestamp(port, 1)) < 0)
			return error_code;
		break;

	case FSCC_DISABLE_APPEND_TIMESTAMP:
		fscc_port_set_append_timestamp(port, 0);
		break;

	case FSCC_GET_APPEND_TIMESTAMP:
		tmp = fscc_port_get_append_timestamp(port);
		copy_to_user((void *)arg, &tmp, sizeof(tmp));
		break;

	case FSCC_SET_MEMORY_CAP:
		copy_from_user(&tmp_memcap, (void *)arg, sizeof(tmp_memcap));
		fscc_port_set_memory_cap(port, &tmp_memcap);
		break;

	case FSCC_GET_MEMORY_CAP:
		tmp_memcap.input = fscc_port_get_input_memory_cap(port);
		tmp_memcap.output = fscc_port_get_output_memory_cap(port);
		copy_to_user(&(((struct fscc_memory_cap *)arg)->input), &tmp_memcap.input, sizeof(tmp_memcap.input));
		copy_to_user(&(((struct fscc_memory_cap *)arg)->output), &tmp_memcap.output, sizeof(tmp_memcap.output));
		break;

	case FSCC_SET_CLOCK_BITS:
		copy_from_user(clock_bits, (char *)arg, 20);
		fscc_port_set_clock_bits(port, clock_bits);
		break;

	case FSCC_ENABLE_IGNORE_TIMEOUT:
		fscc_port_set_ignore_timeout(port, 1);
		break;

	case FSCC_DISABLE_IGNORE_TIMEOUT:
		fscc_port_set_ignore_timeout(port, 0);
		break;

	case FSCC_GET_IGNORE_TIMEOUT:
		tmp = fscc_port_get_ignore_timeout(port);
		copy_to_user((void *)arg, &tmp, sizeof(tmp));
		break;

	case FSCC_SET_TX_MODIFIERS:
		// this is apparently fine for basic types.
		tmp = (unsigned)arg;
		if ((error_code = fscc_port_set_tx_modifiers(port, (unsigned)tmp)) < 0) return error_code;
		break;

	case FSCC_GET_TX_MODIFIERS:
		tmp = fscc_port_get_tx_modifiers(port);
		copy_success = copy_to_user((void *)arg, &tmp, sizeof(tmp));
        	if(copy_success != 0) printk("GET_TX_MODIFIERS: copy_to_user failed with: %d\n", copy_success);
		break;

	case FSCC_ENABLE_RX_MULTIPLE:
		fscc_port_set_rx_multiple(port, 1);
		break;

	case FSCC_DISABLE_RX_MULTIPLE:
		fscc_port_set_rx_multiple(port, 0);
		break;

	case FSCC_GET_RX_MULTIPLE:
		tmp = fscc_port_get_rx_multiple(port);
		copy_to_user((void *)arg, &tmp, sizeof(tmp));
		break;
	/*
	case FSCC_GET_STATUS:
		tmp = port->last_isr_value;
		port->last_isr_value = 0;
		if(tmp != 0 || (file->f_flags & O_NONBLOCK))
		{
			copy_to_user((void *)arg, &tmp, sizeof(tmp));
			break;
		}
		interruptible_sleep_on(&port->status_queue);
		tmp = port->last_isr_value;
		port->last_isr_value = 0;
		copy_to_user((void *)arg, &tmp, sizeof(tmp));
		break;
	*/
	case FSCC_GET_MEMORY_USAGE:
		tmp_memcap.input = fscc_port_get_input_memory_usage(port);
		tmp_memcap.output = fscc_port_get_output_memory_usage(port);
		copy_to_user(&(((struct fscc_memory_cap *)arg)->input), &tmp_memcap.input, sizeof(tmp_memcap.input));
		copy_to_user(&(((struct fscc_memory_cap *)arg)->output), &tmp_memcap.output, sizeof(tmp_memcap.output));
		break;
        
	default:
		dev_dbg(port->device, "unknown ioctl 0x%x\n", cmd);
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
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 11)
	.unlocked_ioctl = fscc_ioctl,
#else
	.ioctl = fscc_ioctl,
#endif
};

#if 0
static int fscc_probe(struct pci_dev *pdev,
								const struct pci_device_id *id)
{
	struct fscc_card *new_card = 0;

	if (pci_enable_device(pdev))
		return -EIO;

	new_card = fscc_card_new(pdev, fscc_major_number, fscc_class,
							 &fscc_fops);

	if (new_card)
		list_add_tail(&new_card->list, &fscc_cards);

	return 0;
}
#endif

static void fscc_remove(struct pci_dev *pdev)
{
	struct fscc_card *card = 0;

	card = fscc_card_find(pdev, &fscc_cards);

	if (card == 0)
		return;

	list_del(&card->list);
	fscc_card_delete(card);
	pci_disable_device(pdev);
}

static int fscc_suspend(struct pci_dev *pdev, pm_message_t state)
{
	struct fscc_card *card = 0;

	card = fscc_card_find(pdev, &fscc_cards);

	dev_dbg(fscc_card_get_device(card), "suspending\n");

	fscc_card_suspend(card);

	pci_save_state(pdev);
	pci_set_power_state(pdev, pci_choose_state(pdev, state));

	return 0;
}

static int fscc_resume(struct pci_dev *pdev)
{
	struct fscc_card *card = 0;

	card = fscc_card_find(pdev, &fscc_cards);

	dev_dbg(fscc_card_get_device(card), "resuming\n");

	pci_set_power_state(pdev, PCI_D0);
	pci_restore_state(pdev);

	fscc_card_resume(card);

	return 0;
}

struct pci_driver fscc_pci_driver = {
	.name = DEVICE_NAME,
	//.probe = fscc_probe,
	.remove = fscc_remove,
	.suspend = fscc_suspend,
	.resume = fscc_resume,
	.id_table = fscc_id_table,
};

static int __init fscc_init(void)
{
	int error_code = 0;

	fscc_class = class_create(THIS_MODULE, DEVICE_NAME);

	if (IS_ERR(fscc_class)) {
		printk(KERN_ERR DEVICE_NAME " class_create failed\n");
		return PTR_ERR(fscc_class);
	}

	fscc_major_number = error_code = register_chrdev(0, "fscc", &fscc_fops);

	if (fscc_major_number < 0) {
		printk(KERN_ERR DEVICE_NAME " register_chrdev failed\n");
		class_destroy(fscc_class);
		return error_code;
	}

	error_code = pci_register_driver(&fscc_pci_driver);

	if (error_code < 0) {
		printk(KERN_ERR DEVICE_NAME " pci_register_driver failed\n");
		unregister_chrdev(fscc_major_number, "fscc");
		class_destroy(fscc_class);
		return error_code;
	}

	if (error_code == 0) {
		struct pci_dev *pdev = NULL;
        unsigned num_devices = 0;
        struct fscc_card *new_card = 0;

		pdev = pci_get_device(COMMTECH_VENDOR_ID, PCI_ANY_ID, pdev);

		while (pdev != NULL) {
			if (is_fscc_device(pdev)) {
				++num_devices;

			    if (pci_enable_device(pdev))
				    return -EIO;

			    new_card = fscc_card_new(pdev, fscc_major_number,
			                             fscc_class, &fscc_fops);

			    if (new_card)
				    list_add_tail(&new_card->list, &fscc_cards);
	        }

			pdev = pci_get_device(COMMTECH_VENDOR_ID, PCI_ANY_ID, pdev);
		}

		if (num_devices == 0) {
			pci_unregister_driver(&fscc_pci_driver);
		    unregister_chrdev(fscc_major_number, "fscc");
		    class_destroy(fscc_class);
			return -ENODEV;
		}
	}

#ifdef DEBUG
	printk(KERN_INFO DEVICE_NAME " setting: debug (on)\n");

	printk(KERN_INFO DEVICE_NAME " setting: force_fifo (%s)\n",
		   (force_fifo) ? "on" : "off");
#endif

	return 0;
}

static void __exit fscc_exit(void)
{
	struct list_head *current_node = 0;
	struct list_head *temp_node = 0;

	list_for_each_safe(current_node, temp_node, &fscc_cards) {
		struct fscc_card *current_card = 0;

		current_card = list_entry(current_node, struct fscc_card, list);

		list_del(current_node);
		fscc_card_delete(current_card);
	}

	pci_unregister_driver(&fscc_pci_driver);
	unregister_chrdev(fscc_major_number, DEVICE_NAME);
	class_destroy(fscc_class);
}

MODULE_DEVICE_TABLE(pci, fscc_id_table);

MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("2.5.3");
MODULE_AUTHOR("William Fagan <willf@commtech-fastcom.com>");

MODULE_DESCRIPTION("Driver for the FSCC series of cards from Commtech, Inc.");

module_param(force_fifo, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(force_fifo, "Disables DMA (SuperFSCC* series), forcing FIFO operation.");

module_init(fscc_init);
module_exit(fscc_exit);

