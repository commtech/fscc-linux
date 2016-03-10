/*
	Copyright (C) 2016 Commtech, Inc.

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

#include <linux/version.h> /* LINUX_VERSION_CODE, KERNEL_VERSION */

#include <asm/uaccess.h> /* copy_*_user in <= 2.6.24 */

#include "port.h"
#include "frame.h" /* struct fscc_frame */
#include "card.h" /* fscc_card_* */
#include "utils.h" /* return_{val_}_if_untrue, chars_to_u32, ... */
#include "config.h" /* DEVICE_NAME, DEFAULT_* */
#include "isr.h" /* fscc_isr */
#include "sysfs.h" /* port_*_attribute_group */


void fscc_port_execute_GO_R(struct fscc_port *port);
void fscc_port_execute_STOP_R(struct fscc_port *port);
void fscc_port_execute_STOP_T(struct fscc_port *port);
void fscc_port_execute_RST_R(struct fscc_port *port);

extern unsigned force_fifo;

/*
	This handles initialization on a port level. So things that each port have
	will be initialized in this function. /dev/ nodes, registers, clock,
	and interrupts all happen here because it is specific to the port.
*/
struct fscc_port *fscc_port_new(struct fscc_card *card, unsigned channel,
								unsigned major_number, unsigned minor_number,
								struct device *parent, struct class *class,
								struct file_operations *fops)
{
	struct fscc_port *port = 0;
	unsigned irq_num = 0;
	char clock_bits[20] = DEFAULT_CLOCK_BITS;

	port = kmalloc(sizeof(*port), GFP_KERNEL);

	if (port == NULL) {
		printk(KERN_ERR DEVICE_NAME "kmalloc failed\n");
		return 0;
	}

	port->name = kmalloc(10, GFP_KERNEL);
	if (port->name == NULL) {
		kfree(port);

		printk(KERN_ERR DEVICE_NAME "kmalloc failed\n");
		return 0;
	}

	sprintf(port->name, "%s%u", DEVICE_NAME, minor_number);

	port->class = class;
	port->dev_t = MKDEV(major_number, minor_number);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 18)

	#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)
		port->device = device_create(port->class, parent, port->dev_t, port,
									 "%s", port->name);
	#else
		port->device = device_create(port->class, parent, port->dev_t, "%s",
									 port->name);

		dev_set_drvdata(port->device, port);
	#endif

#else
	class_device_create(port->class, 0, port->dev_t, port->device, "%s",
						port->name);
#endif

	if (port->device <= 0) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 18)
		device_destroy(port->class, port->dev_t);
#endif

		if (port->name)
			kfree(port->name);

		kfree(port);

		printk(KERN_ERR DEVICE_NAME " %s: device_create failed\n", port->name);
		return 0;
	}

#ifdef DEBUG
	port->interrupt_tracker = debug_interrupt_tracker_new();
#endif

	port->channel = channel;
	port->card = card;

	port->istream = fscc_frame_new(port);

	fscc_port_set_append_status(port, DEFAULT_APPEND_STATUS_VALUE);
	fscc_port_set_append_timestamp(port, DEFAULT_APPEND_TIMESTAMP_VALUE);
	fscc_port_set_ignore_timeout(port, DEFAULT_IGNORE_TIMEOUT_VALUE);
	fscc_port_set_tx_modifiers(port, DEFAULT_TX_MODIFIERS_VALUE);
	fscc_port_set_rx_multiple(port, DEFAULT_RX_MULTIPLE_VALUE);

	port->memory_cap.input = DEFAULT_INPUT_MEMORY_CAP_VALUE;
	port->memory_cap.output = DEFAULT_OUTPUT_MEMORY_CAP_VALUE;

	port->pending_iframe = 0;
	port->pending_oframe = 0;

	spin_lock_init(&port->board_settings_spinlock);
	spin_lock_init(&port->board_rx_spinlock);
	spin_lock_init(&port->board_tx_spinlock);

	spin_lock_init(&port->istream_spinlock);
	spin_lock_init(&port->pending_iframe_spinlock);
	spin_lock_init(&port->pending_oframe_spinlock);
	spin_lock_init(&port->sent_oframes_spinlock);
	spin_lock_init(&port->queued_oframes_spinlock);
	spin_lock_init(&port->queued_iframes_spinlock);

	/* Simple check to see if the port is messed up. It won't catch all
	   instances. */
	if (fscc_port_get_PREV(port) == 0xff) {
		dev_warn(port->device, "couldn't initialize\n");

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 18)
		device_destroy(port->class, port->dev_t);
#endif

		if (port->name)
			kfree(port->name);

		kfree(port);
		return 0;
	}

	INIT_LIST_HEAD(&port->list);

	fscc_flist_init(&port->queued_oframes);
	fscc_flist_init(&port->sent_oframes);
	fscc_flist_init(&port->queued_iframes);

	sema_init(&port->read_semaphore, 1);
	sema_init(&port->write_semaphore, 1);
	sema_init(&port->poll_semaphore, 1);

	init_waitqueue_head(&port->input_queue);
	init_waitqueue_head(&port->output_queue);

	cdev_init(&port->cdev, fops);
	port->cdev.owner = THIS_MODULE;

	if (cdev_add(&port->cdev, port->dev_t, 1) < 0) {
		dev_err(port->device, "cdev_add failed\n");

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 18)
		device_destroy(port->class, port->dev_t);
#endif

		if (port->name)
			kfree(port->name);

		kfree(port);

		return 0;
	}

	port->last_isr_value = 0;

	FSCC_REGISTERS_INIT(port->register_storage);

	port->register_storage.FIFOT = DEFAULT_FIFOT_VALUE;
	port->register_storage.CCR0 = DEFAULT_CCR0_VALUE;
	port->register_storage.CCR1 = DEFAULT_CCR1_VALUE;
	port->register_storage.CCR2 = DEFAULT_CCR2_VALUE;
	port->register_storage.BGR = DEFAULT_BGR_VALUE;
	port->register_storage.SSR = DEFAULT_SSR_VALUE;
	port->register_storage.SMR = DEFAULT_SMR_VALUE;
	port->register_storage.TSR = DEFAULT_TSR_VALUE;
	port->register_storage.TMR = DEFAULT_TMR_VALUE;
	port->register_storage.RAR = DEFAULT_RAR_VALUE;
	port->register_storage.RAMR = DEFAULT_RAMR_VALUE;
	port->register_storage.PPR = DEFAULT_PPR_VALUE;
	port->register_storage.TCR = DEFAULT_TCR_VALUE;
	port->register_storage.IMR = DEFAULT_IMR_VALUE;
	port->register_storage.DPLLR = DEFAULT_DPLLR_VALUE;
	port->register_storage.FCR = DEFAULT_FCR_VALUE;

	fscc_port_set_registers(port, &port->register_storage);

	irq_num = fscc_card_get_irq(card);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 18)
	if (request_irq(irq_num, &fscc_isr, IRQF_SHARED, port->name, port)) {
#else
	if (request_irq(irq_num, &fscc_isr, SA_SHIRQ, port->name, port)) {
#endif
		dev_err(port->device, "request_irq failed on irq %i\n", irq_num);
		return 0;
	}

/* The sysfs structures I use in sysfs.c don't work prior to 2.6.25 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)
	if (sysfs_create_group(&port->device->kobj, &port_registers_attr_group)) {
		dev_err(port->device, "sysfs_create_group\n");
		return 0;
	}

	if (sysfs_create_group(&port->device->kobj, &port_commands_attr_group)) {
		dev_err(port->device, "sysfs_create_group\n");
		return 0;
	}

	if (sysfs_create_group(&port->device->kobj, &port_info_attr_group)) {
		dev_err(port->device, "sysfs_create_group\n");
		return 0;
	}

	if (sysfs_create_group(&port->device->kobj, &port_settings_attr_group)) {
		dev_err(port->device, "sysfs_create_group\n");
		return 0;
	}

#ifdef DEBUG
	if (sysfs_create_group(&port->device->kobj, &port_debug_attr_group)) {
		dev_err(port->device, "sysfs_create_group\n");
		return 0;
	}
#endif /* DEBUG */

#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25) */

	tasklet_init(&port->send_oframe_tasklet, oframe_worker, (unsigned long)port);
	tasklet_init(&port->clear_oframe_tasklet, clear_oframe_worker, (unsigned long)port);
	tasklet_init(&port->iframe_tasklet, iframe_worker, (unsigned long)port);
	tasklet_init(&port->istream_tasklet, istream_worker, (unsigned long)port);

#ifdef DEBUG
	tasklet_init(&port->print_tasklet, debug_interrupt_display, (unsigned long)port);
#endif

	if (fscc_port_has_dma(port)) {
		port->null_descriptor = kmalloc(sizeof(*port->null_descriptor),
		                                GFP_ATOMIC | GFP_DMA);

		memset(port->null_descriptor, 0, sizeof(*port->null_descriptor));

		port->null_handle = pci_map_single(port->card->pci_dev,
		                                   port->null_descriptor,
	                                       sizeof(*port->null_descriptor),
	                                       DMA_TO_DEVICE);
	}

	dev_info(port->device, "%s (%x.%02x)\n", fscc_card_get_name(port->card),
			 fscc_port_get_PREV(port), fscc_port_get_FREV(port));

	fscc_port_set_clock_bits(port, clock_bits);

	setup_timer(&port->timer, &timer_handler, (unsigned long)port);

	if (fscc_port_has_dma(port)) {
		fscc_port_execute_RST_R(port);
		fscc_port_execute_RST_T(port);
	}

	fscc_port_execute_RRES(port);
	fscc_port_execute_TRES(port);

	return port;
}

void fscc_port_delete(struct fscc_port *port)
{
	unsigned irq_num = 0;
	unsigned long stream_flags = 0;
	unsigned long queued_iframes_flags = 0;
	unsigned long queued_oframes_flags = 0;
	unsigned long sent_oframes_flags = 0;

	return_if_untrue(port);

	/* Stops the the timer and transmit repeat abailities if they are on. */
	fscc_port_set_register(port, 0, CMDR_OFFSET, 0x04000002);

	del_timer(&port->timer);

	irq_num = fscc_card_get_irq(port->card);
	free_irq(irq_num, port);

	if (fscc_port_has_dma(port)) {
		fscc_port_execute_STOP_T(port);
		fscc_port_execute_STOP_R(port);
		fscc_port_execute_RST_T(port);
		fscc_port_execute_RST_R(port);

		fscc_port_set_register(port, 2, DMACCR_OFFSET, 0x00000000);
		fscc_port_set_register(port, 2, DMA_TX_BASE_OFFSET, 0x00000000);

		pci_unmap_single(port->card->pci_dev, port->null_handle,
						 sizeof(*port->null_descriptor), DMA_TO_DEVICE);

		kfree(port->null_descriptor);
	}

	spin_lock_irqsave(&port->istream_spinlock, stream_flags);
	fscc_frame_delete(port->istream);
	spin_unlock_irqrestore(&port->istream_spinlock, stream_flags);

	spin_lock_irqsave(&port->queued_iframes_spinlock, queued_iframes_flags);
	fscc_flist_delete(&port->queued_iframes);
	spin_unlock_irqrestore(&port->queued_iframes_spinlock, queued_iframes_flags);

	spin_lock_irqsave(&port->queued_oframes_spinlock, queued_oframes_flags);
	fscc_flist_delete(&port->queued_oframes);
	spin_unlock_irqrestore(&port->queued_oframes_spinlock, queued_oframes_flags);

	spin_lock_irqsave(&port->sent_oframes_spinlock, sent_oframes_flags);
	fscc_flist_delete(&port->sent_oframes);
	spin_unlock_irqrestore(&port->sent_oframes_spinlock, sent_oframes_flags);

#ifdef DEBUG
	debug_interrupt_tracker_delete(port->interrupt_tracker);
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 18)
	device_destroy(port->class, port->dev_t);
#endif

	cdev_del(&port->cdev);

	if (port->name)
		kfree(port->name);

	kfree(port);
}

void fscc_port_reset_timer(struct fscc_port *port)
{
	if (mod_timer(&port->timer, jiffies + msecs_to_jiffies(250)))
		dev_err(port->device, "mod_timer\n");
}

/* Basic check to see if the CE bit is set. */
unsigned fscc_port_timed_out(struct fscc_port *port)
{
	__u32 star_value = 0;
	unsigned i = 0;

	return_val_if_untrue(port, 0);

	for (i = 0; i < DEFAULT_TIMEOUT_VALUE; i++) {
		star_value = fscc_port_get_register(port, 0, STAR_OFFSET);

		if ((star_value & CE_BIT) == 0)
			return 0;
	}

	return 1;
}

/* Create the data structures the work horse functions use to send data. */
int fscc_port_write(struct fscc_port *port, const char *data, unsigned length)
{
	struct fscc_frame *frame = 0;
	unsigned long queued_flags = 0;

	return_val_if_untrue(port, 0);

	/* Checks to make sure there is a clock present. */
	if (port->ignore_timeout == 0 && fscc_port_timed_out(port)) {
		dev_dbg(port->device, "device stalled (wrong clock mode?)\n");
		return -ETIMEDOUT;
	}

	frame = fscc_frame_new(port);

	if (!frame)
		return -ENOMEM;

	fscc_frame_add_data_from_user(frame, data, length);

	spin_lock_irqsave(&port->queued_oframes_spinlock, queued_flags);
	fscc_flist_add_frame(&port->queued_oframes, frame);
	spin_unlock_irqrestore(&port->queued_oframes_spinlock, queued_flags);

	tasklet_schedule(&port->send_oframe_tasklet);

	return 0;
}

/*
	Handles taking the frames already retrieved from the card and giving them
	to the user. This is purely a helper for the fscc_port_read function.
*/
ssize_t fscc_port_frame_read(struct fscc_port *port, char *buf, size_t buf_length)
{
	struct fscc_frame *frame = 0;
	unsigned remaining_buf_length = 0;
	int max_frame_length = 0;
	unsigned current_frame_length = 0;
	unsigned out_length = 0;
	unsigned long queued_flags = 0;

	return_val_if_untrue(port, 0);

	do {
		remaining_buf_length = buf_length - out_length;

		if (port->append_status && port->append_timestamp)
			max_frame_length = remaining_buf_length - sizeof(fscc_timestamp);
		else if (port->append_status)
			max_frame_length = remaining_buf_length;
		else if (port->append_timestamp)
			max_frame_length = remaining_buf_length + 2 - sizeof(fscc_timestamp);
		else
			max_frame_length = remaining_buf_length + 2; // Status length

		if (max_frame_length < 0)
			break;

		spin_lock_irqsave(&port->queued_iframes_spinlock, queued_flags);
		frame = fscc_flist_remove_frame_if_lte(&port->queued_iframes, max_frame_length);
		spin_unlock_irqrestore(&port->queued_iframes_spinlock, queued_flags);

		if (!frame)
			break;

		current_frame_length = fscc_frame_get_length(frame);
		current_frame_length -= (!port->append_status) ? 2 : 0;

		fscc_frame_remove_data(frame, buf + out_length, current_frame_length);
		out_length += current_frame_length;

		if (port->append_timestamp) {
			memcpy(buf + out_length, &frame->timestamp, sizeof(frame->timestamp));
			current_frame_length += sizeof(frame->timestamp);
			out_length += sizeof(frame->timestamp);
		}

		fscc_frame_delete(frame);
	}
	while (port->rx_multiple);

	if (out_length == 0)
		return -ENOBUFS;

	return out_length;
}

/*
	Handles taking the streams already retrieved from the card and giving them
	to the user. This is purely a helper for the fscc_port_read function.
*/
ssize_t fscc_port_stream_read(struct fscc_port *port, char *buf,
							  size_t buf_length)
{
	unsigned out_length = 0;
	unsigned long flags;

	return_val_if_untrue(port, 0);

	spin_lock_irqsave(&port->istream_spinlock, flags);

	out_length = min(buf_length, (size_t)fscc_frame_get_length(port->istream));

	fscc_frame_remove_data(port->istream, buf, out_length);

	spin_unlock_irqrestore(&port->istream_spinlock, flags);

	return out_length;
}

/*
	Returns -ENOBUFS if count is smaller than pending frame size
	Buf needs to be a user buffer
*/
ssize_t fscc_port_read(struct fscc_port *port, char *buf, size_t count)
{
	if (fscc_port_is_streaming(port))
		return fscc_port_stream_read(port, buf, count);
	else
		return fscc_port_frame_read(port, buf, count);
}

/* Count is for streaming mode where we need to check there is enough
   streaming data.
*/
unsigned fscc_port_has_incoming_data(struct fscc_port *port)
{
	unsigned status = 0;
	unsigned long flags;

	return_val_if_untrue(port, 0);

	if (fscc_port_is_streaming(port)) {
		status =  (fscc_frame_is_empty(port->istream)) ? 0 : 1;
	}
	else {
		spin_lock_irqsave(&port->queued_iframes_spinlock, flags);
		if (fscc_flist_is_empty(&port->queued_iframes) == 0)
			status = 1;
		spin_unlock_irqrestore(&port->queued_iframes_spinlock, flags);
	}

	return status;
}


/*
	At the port level the offset will automatically be converted to the port
	specific offset.
*/
__u32 fscc_port_get_register(struct fscc_port *port, unsigned bar,
							 unsigned register_offset)
{
	unsigned offset = 0;
	__u32 value = 0;

	return_val_if_untrue(port, 0);
	return_val_if_untrue(bar <= 2, 0);

	offset = port_offset(port, bar, register_offset);
	value = fscc_card_get_register(port->card, bar, offset);

	return value;
}

/*
	At the port level the offset will automatically be converted to the port
	specific offset.
*/
int fscc_port_set_register(struct fscc_port *port, unsigned bar,
						   unsigned register_offset, __u32 value)
{
	unsigned offset = 0;

	return_val_if_untrue(port, 0);
	return_val_if_untrue(bar <= 2, 0);

	offset = port_offset(port, bar, register_offset);

	/* Checks to make sure there is a clock present. */
	if (register_offset == CMDR_OFFSET && port->ignore_timeout == 0
		&& fscc_port_timed_out(port)) {
		return -ETIMEDOUT;
	}

	fscc_card_set_register(port->card, bar, offset, value);

	if (bar == 0) {
		fscc_register old_value = ((fscc_register *)&port->register_storage)[register_offset / 4];
		((fscc_register *)&port->register_storage)[register_offset / 4] = value;

		if (old_value != value) {
			dev_dbg(port->device, "%i:%02x 0x%08x => 0x%08x\n", bar, 
					register_offset, (unsigned int)old_value, value);
		}
		else {
			dev_dbg(port->device, "%i:%02x 0x%08x\n", bar, register_offset, 
					value);
		}
	}
	else if (register_offset == FCR_OFFSET) {
		fscc_register old_value = port->register_storage.FCR;
		port->register_storage.FCR = value;

		if (old_value != value) {
			dev_dbg(port->device, "2:00 0x%08x => 0x%08x\n", 
					(unsigned int)old_value, value);
		}
		else {
			dev_dbg(port->device, "2:00 0x%08x\n", value);
		}
	}

	return 1;
}

/*
	At the port level the offset will automatically be converted to the port
	specific offset.
*/
void fscc_port_get_register_rep(struct fscc_port *port, unsigned bar,
								unsigned register_offset, char *buf,
								unsigned byte_count)
{
	unsigned offset = 0;

	return_if_untrue(port);
	return_if_untrue(bar <= 2);
	return_if_untrue(buf);
	return_if_untrue(byte_count > 0);

	offset = port_offset(port, bar, register_offset);

	fscc_card_get_register_rep(port->card, bar, offset, buf, byte_count);
}

/*
	At the port level the offset will automatically be converted to the port
	specific offset.
*/
void fscc_port_set_register_rep(struct fscc_port *port, unsigned bar,
								unsigned register_offset, const char *data,
								unsigned byte_count)
{
	unsigned offset = 0;

	return_if_untrue(port);
	return_if_untrue(bar <= 2);
	return_if_untrue(data);
	return_if_untrue(byte_count > 0);

	offset = port_offset(port, bar, register_offset);

	fscc_card_set_register_rep(port->card, bar, offset, data, byte_count);
}

/*
	At the port level the offset will automatically be converted to the port
	specific offset.
*/
int fscc_port_set_registers(struct fscc_port *port,
							const struct fscc_registers *regs)
{
	unsigned stalled = 0;
	unsigned i = 0;

	return_val_if_untrue(port, 0);
	return_val_if_untrue(regs, 0);

	for (i = 0; i < sizeof(*regs) / sizeof(fscc_register); i++) {
		unsigned register_offset = i * 4;

		if (is_read_only_register(register_offset)
			|| ((fscc_register *)regs)[i] < 0) {
			continue;
		}

		if (register_offset <= MAX_OFFSET) {
			if (fscc_port_set_register(port, 0, register_offset, ((fscc_register *)regs)[i]) == -ETIMEDOUT)
				stalled = 1;
		}
		else {
			fscc_port_set_register(port, 2, FCR_OFFSET,
								   ((fscc_register *)regs)[i]);
		}
	}

	return (stalled) ? -ETIMEDOUT : 1;
}

/*
  fscc_port_get_registers reads only the registers that are specified with FSCC_UPDATE_VALUE.
  If the requested register is larger than the maximum FCore register it jumps to the FCR register.
*/
void fscc_port_get_registers(struct fscc_port *port, struct fscc_registers *regs)
{
	unsigned i = 0;

	return_if_untrue(port);
	return_if_untrue(regs);

	for (i = 0; i < sizeof(*regs) / sizeof(fscc_register); i++) {
		if (((fscc_register *)regs)[i] != FSCC_UPDATE_VALUE)
			continue;

		if (i * 4 <= MAX_OFFSET) {
			((fscc_register *)regs)[i] = fscc_port_get_register(port, 0, i * 4);
		}
		else {
			((fscc_register *)regs)[i] = fscc_port_get_register(port, 2,FCR_OFFSET);
		}
	}
}

__u32 fscc_port_get_TXCNT(struct fscc_port *port)
{
	__u32 fifo_bc_value = 0;

	return_val_if_untrue(port, 0);

	fifo_bc_value = fscc_port_get_register(port, 0, FIFO_BC_OFFSET);

	return (fifo_bc_value & 0x1FFF0000) >> 16;
}

unsigned fscc_port_get_RXCNT(struct fscc_port *port)
{
	__u32 fifo_bc_value = 0;

	return_val_if_untrue(port, 0);

	fifo_bc_value = fscc_port_get_register(port, 0, FIFO_BC_OFFSET);

	/* Not sure why, but this can be larger than 8192. We add
       the 8192 check here so other code can count on the value
       not being larger than 8192. */
	if(fifo_bc_value > 8192)
	    fifo_bc_value = 0;
	    
	return fifo_bc_value;
}
__u8 fscc_port_get_FREV(struct fscc_port *port)
{
	__u32 vstr_value = 0;

	return_val_if_untrue(port, 0);

	vstr_value = fscc_port_get_register(port, 0, VSTR_OFFSET);

	return (__u8)((vstr_value & 0x000000FF));
}

__u8 fscc_port_get_PREV(struct fscc_port *port)
{
	__u32 vstr_value = 0;

	return_val_if_untrue(port, 0);

	vstr_value = fscc_port_get_register(port, 0, VSTR_OFFSET);

	return (__u8)((vstr_value & 0x0000FF00) >> 8);
}

__u16 fscc_port_get_PDEV(struct fscc_port *port)
{
	__u32 vstr_value = 0;

	return_val_if_untrue(port, 0);

	vstr_value = fscc_port_get_register(port, 0, VSTR_OFFSET);

	return (__u16)((vstr_value & 0xFFFF0000) >> 16);
}

unsigned fscc_port_get_CE(struct fscc_port *port)
{
	__u32 star_value = 0;

	return_val_if_untrue(port, 0);

	star_value = fscc_port_get_register(port, 0, STAR_OFFSET);

	return (unsigned)((star_value & 0x00040000) >> 18);
}

unsigned fscc_port_get_RFCNT(struct fscc_port *port)
{
	__u32 fifo_fc_value = 0;

	return_val_if_untrue(port, 0);

	fifo_fc_value = fscc_port_get_register(port, 0, FIFO_FC_OFFSET);

	return (unsigned)(fifo_fc_value & 0x000003ff);
}

void fscc_port_suspend(struct fscc_port *port)
{
	return_if_untrue(port);

	fscc_port_get_registers(port, &port->register_storage);
}

void fscc_port_resume(struct fscc_port *port)
{
	return_if_untrue(port);

	fscc_port_set_registers(port, &port->register_storage);
}

int fscc_port_purge_rx(struct fscc_port *port)
{
	int error_code = 0;
	unsigned long board_flags;
	unsigned long istream_flags;
	unsigned long queued_flags;
	unsigned long pending_flags;

	return_val_if_untrue(port, 0);

	dev_dbg(port->device, "purge_rx\n");

	spin_lock_irqsave(&port->board_rx_spinlock, board_flags);
	error_code = fscc_port_execute_RRES(port);
	spin_unlock_irqrestore(&port->board_rx_spinlock, board_flags);

	if (error_code < 0)
		return error_code;

	spin_lock_irqsave(&port->queued_iframes_spinlock, queued_flags);
	fscc_flist_clear(&port->queued_iframes);
	spin_unlock_irqrestore(&port->queued_iframes_spinlock, queued_flags);

	spin_lock_irqsave(&port->istream_spinlock, istream_flags);
	fscc_frame_clear(port->istream);
	spin_unlock_irqrestore(&port->istream_spinlock, istream_flags);

	spin_lock_irqsave(&port->pending_iframe_spinlock, pending_flags);
	if (port->pending_iframe) {
		fscc_frame_delete(port->pending_iframe);
		port->pending_iframe = 0;
	}
	spin_unlock_irqrestore(&port->pending_iframe_spinlock, pending_flags);

	return 1;
}

int fscc_port_purge_tx(struct fscc_port *port)
{
	int error_code = 0;
	unsigned long board_flags;
	unsigned long queued_flags = 0;
	unsigned long sent_flags = 0;
	unsigned long pending_flags = 0;

	return_val_if_untrue(port, 0);

	dev_dbg(port->device, "purge_tx\n");

	spin_lock_irqsave(&port->board_tx_spinlock, board_flags);
	error_code = fscc_port_execute_TRES(port);
	spin_unlock_irqrestore(&port->board_tx_spinlock, board_flags);

	if (error_code < 0)
		return error_code;

	spin_lock_irqsave(&port->queued_oframes_spinlock, queued_flags);
	fscc_flist_clear(&port->queued_oframes);
	spin_unlock_irqrestore(&port->queued_oframes_spinlock, queued_flags);

	spin_lock_irqsave(&port->sent_oframes_spinlock, sent_flags);
	fscc_flist_clear(&port->sent_oframes);
	spin_unlock_irqrestore(&port->sent_oframes_spinlock, sent_flags);

	spin_lock_irqsave(&port->pending_oframe_spinlock, pending_flags);
	if (port->pending_oframe) {
		fscc_frame_delete(port->pending_oframe);
		port->pending_oframe = 0;
	}
	spin_unlock_irqrestore(&port->pending_oframe_spinlock, pending_flags);

	wake_up_interruptible(&port->output_queue);

	return 1;
}

unsigned fscc_port_get_input_memory_usage(struct fscc_port *port)
{
	unsigned value = 0;
	unsigned long pending_flags;
	unsigned long queued_flags;
	unsigned long stream_flags;

	return_val_if_untrue(port, 0);

	spin_lock_irqsave(&port->queued_iframes_spinlock, queued_flags);
	value = port->queued_iframes.estimated_memory_usage;
	spin_unlock_irqrestore(&port->queued_iframes_spinlock, queued_flags);

	spin_lock_irqsave(&port->istream_spinlock, stream_flags);
	value += fscc_frame_get_length(port->istream);
	spin_unlock_irqrestore(&port->istream_spinlock, stream_flags);

	spin_lock_irqsave(&port->pending_iframe_spinlock, pending_flags);
	if (port->pending_iframe)
		value += fscc_frame_get_length(port->pending_iframe);
	spin_unlock_irqrestore(&port->pending_iframe_spinlock, pending_flags);

	return value;
}

unsigned fscc_port_get_output_memory_usage(struct fscc_port *port)
{
	unsigned value = 0;
	unsigned long pending_flags;
	unsigned long queued_flags;

	return_val_if_untrue(port, 0);

	spin_lock_irqsave(&port->queued_oframes_spinlock, queued_flags);
	value = port->queued_oframes.estimated_memory_usage;
	spin_unlock_irqrestore(&port->queued_oframes_spinlock, queued_flags);

	spin_lock_irqsave(&port->pending_oframe_spinlock, pending_flags);
	if (port->pending_oframe)
		value += fscc_frame_get_length(port->pending_oframe);
	spin_unlock_irqrestore(&port->pending_oframe_spinlock, pending_flags);

	return value;
}

unsigned fscc_port_get_input_number_frames(struct fscc_port *port)
{
	unsigned value = 0;
	unsigned long pending_flags;
	unsigned long queued_flags;

	return_val_if_untrue(port, 0);

	spin_lock_irqsave(&port->queued_iframes_spinlock, queued_flags);
	value = fscc_flist_length(&port->queued_iframes);
	spin_unlock_irqrestore(&port->queued_iframes_spinlock, queued_flags);

	spin_lock_irqsave(&port->pending_iframe_spinlock, pending_flags);
	if (port->pending_iframe)
		value++;
	spin_unlock_irqrestore(&port->pending_iframe_spinlock, pending_flags);

	return value;
}

unsigned fscc_port_get_output_number_frames(struct fscc_port *port)
{
	unsigned value = 0;
	unsigned long pending_flags;
	unsigned long queued_flags;

	return_val_if_untrue(port, 0);

	spin_lock_irqsave(&port->queued_oframes_spinlock, queued_flags);
	value = fscc_flist_length(&port->queued_oframes);
	spin_unlock_irqrestore(&port->queued_oframes_spinlock, queued_flags);

	spin_lock_irqsave(&port->pending_oframe_spinlock, pending_flags);
	if (port->pending_oframe)
		value++;
	spin_unlock_irqrestore(&port->pending_oframe_spinlock, pending_flags);

	return value;
}

unsigned fscc_port_get_input_memory_cap(struct fscc_port *port)
{
	return_val_if_untrue(port, 0);

	return port->memory_cap.input;
}

unsigned fscc_port_get_output_memory_cap(struct fscc_port *port)
{
	return_val_if_untrue(port, 0);

	return port->memory_cap.output;
}

void fscc_port_set_memory_cap(struct fscc_port *port,
							  struct fscc_memory_cap *value)
{
	return_if_untrue(port);
	return_if_untrue(value);

	if (value->input >= 0) {
		if (port->memory_cap.input != value->input) {
			dev_dbg(port->device, "memory cap (input) %i => %i\n",
					port->memory_cap.input, value->input);
		}
		else {
			dev_dbg(port->device, "memory cap (input) %i\n",
					value->input);
		}

		port->memory_cap.input = value->input;
	}

	if (value->output >= 0) {
		if (port->memory_cap.output != value->output) {
			dev_dbg(port->device, "memory cap (output) %i => %i\n",
					port->memory_cap.output, value->output);
		}
		else {
			dev_dbg(port->device, "memory cap (output) %i\n",
					value->output);
		}

		port->memory_cap.output = value->output;
	}
}

#define STRB_BASE 0x00000008
#define DTA_BASE 0x00000001
#define CLK_BASE 0x00000002

void fscc_port_set_clock_bits(struct fscc_port *port,
							  unsigned char *clock_data)
{
	__u32 orig_fcr_value = 0;
	__u32 new_fcr_value = 0;
	int j = 0; // Must be signed because we are going backwards through the array
	int i = 0; // Must be signed because we are going backwards through the array
	unsigned strb_value = STRB_BASE;
	unsigned dta_value = DTA_BASE;
	unsigned clk_value = CLK_BASE;
	unsigned long flags;

	__u32 *data = 0;
	unsigned data_index = 0;

	return_if_untrue(port);


#ifdef DISABLE_XTAL
	clock_data[15] &= 0xfb;
#else
	/* This enables XTAL on all cards except green FSCC cards with a revision
       greater than 6 and 232 cards. Some old protoype SuperFSCC cards will 
       need to manually disable XTAL as they are not supported in this driver 
       by default. */
    if ((fscc_port_get_PDEV(port) == 0x0f && fscc_port_get_PREV(port) <= 6) ||
        fscc_port_get_PDEV(port) == 0x16) {
        clock_data[15] &= 0xfb;
    }
    else {
        clock_data[15] |= 0x04;
    }
#endif


	data = kmalloc(sizeof(__u32) * 323, GFP_KERNEL);

	if (data == NULL) {
		printk(KERN_ERR DEVICE_NAME "kmalloc failed\n");
		return;
	}

	if (port->channel == 1) {
		strb_value <<= 0x08;
		dta_value <<= 0x08;
		clk_value <<= 0x08;
	}

	spin_lock_irqsave(&port->board_settings_spinlock, flags);

	orig_fcr_value = fscc_card_get_register(port->card, 2, FCR_OFFSET);

	data[data_index++] = new_fcr_value = orig_fcr_value & 0xfffff0f0;

	for (i = 19; i >= 0; i--) {
		for (j = 7; j >= 0; j--) {
			int bit = ((clock_data[i] >> j) & 1);

            /* This is required for 4-port cards. I'm not sure why at the
               moment */
			data[data_index++] = new_fcr_value;

			if (bit)
				new_fcr_value |= dta_value; /* Set data bit */
			else
				new_fcr_value &= ~dta_value; /* Clear data bit */

			data[data_index++] = new_fcr_value |= clk_value; /* Set clock bit */
			data[data_index++] = new_fcr_value &= ~clk_value; /* Clear clock bit */

			new_fcr_value = orig_fcr_value & 0xfffff0f0;
		}
	}

	new_fcr_value = orig_fcr_value & 0xfffff0f0;

	new_fcr_value |= strb_value; /* Set strobe bit */
	new_fcr_value &= ~clk_value; /* Clear clock bit	*/

	data[data_index++] = new_fcr_value;
	data[data_index++] = orig_fcr_value;

	fscc_port_set_register_rep(port, 2, FCR_OFFSET, (char *)data, data_index * 4);

	spin_unlock_irqrestore(&port->board_settings_spinlock, flags);

	kfree(data);
}

int fscc_port_set_append_status(struct fscc_port *port, unsigned value)
{
	return_val_if_untrue(port, 0);

	if (value && fscc_port_is_streaming(port))
	    return -EOPNOTSUPP;

    if (port->append_status != value) {
		dev_dbg(port->device, "append status %i => %i", port->append_status,
		        value);
    }
    else {
		dev_dbg(port->device, "append status = %i", value);
    }

	port->append_status = (value) ? 1 : 0;

	return 1;
}

int fscc_port_set_append_timestamp(struct fscc_port *port, unsigned value)
{
	return_val_if_untrue(port, 0);

	if (value && fscc_port_is_streaming(port))
	    return -EOPNOTSUPP;

    if (port->append_timestamp != value) {
		dev_dbg(port->device, "append timestamp %i => %i",
		        port->append_timestamp, value);
    }
    else {
		dev_dbg(port->device, "append timestamp = %i", value);
    }

	port->append_timestamp = (value) ? 1 : 0;

	return 1;
}

void fscc_port_set_ignore_timeout(struct fscc_port *port,
								  unsigned value)
{
	return_if_untrue(port);

    if (port->ignore_timeout != value) {
		dev_dbg(port->device, "ignore timeout %i => %i",
		        port->ignore_timeout, value);
    }
    else {
		dev_dbg(port->device, "ignore timeout = %i", value);
    }

	port->ignore_timeout = (value) ? 1 : 0;
}

void fscc_port_set_rx_multiple(struct fscc_port *port,
							   unsigned value)
{
	return_if_untrue(port);

    if (port->rx_multiple != value) {
		dev_dbg(port->device, "receive multiple %i => %i",
		        port->rx_multiple, value);
    }
    else {
		dev_dbg(port->device, "receive multiple = %i", value);
    }

	port->rx_multiple = (value) ? 1 : 0;
}

unsigned fscc_port_get_append_status(struct fscc_port *port)
{
	return_val_if_untrue(port, 0);

	return !fscc_port_is_streaming(port) && port->append_status;
}

unsigned fscc_port_get_append_timestamp(struct fscc_port *port)
{
	return_val_if_untrue(port, 0);

	return !fscc_port_is_streaming(port) && port->append_timestamp;
}

unsigned fscc_port_get_ignore_timeout(struct fscc_port *port)
{
	return_val_if_untrue(port, 0);

	return port->ignore_timeout;
}

unsigned fscc_port_get_rx_multiple(struct fscc_port *port)
{
	return_val_if_untrue(port, 0);

	return port->rx_multiple;
}

int fscc_port_execute_TRES(struct fscc_port *port)
{
	return_val_if_untrue(port, 0);

	return fscc_port_set_register(port, 0, CMDR_OFFSET, 0x08000000);
}

int fscc_port_execute_RRES(struct fscc_port *port)
{
	return_val_if_untrue(port, 0);

	return fscc_port_set_register(port, 0, CMDR_OFFSET, 0x00020000);
}

void fscc_port_execute_GO_R(struct fscc_port *port)
{
	return_if_untrue(port);
	return_if_untrue(fscc_port_has_dma(port) == 1);

	fscc_port_set_register(port, 2, DMACCR_OFFSET, 0x00000001);
}

void fscc_port_execute_RST_R(struct fscc_port *port)
{
	return_if_untrue(port);
	return_if_untrue(fscc_port_has_dma(port) == 1);

	fscc_port_set_register(port, 2, DMACCR_OFFSET, 0x00000010);
}

void fscc_port_execute_RST_T(struct fscc_port *port)
{
	return_if_untrue(port);
	return_if_untrue(fscc_port_has_dma(port) == 1);

	fscc_port_set_register(port, 2, DMACCR_OFFSET, 0x00000020);
}

void fscc_port_execute_STOP_R(struct fscc_port *port)
{
	return_if_untrue(port);
	return_if_untrue(fscc_port_has_dma(port) == 1);

	fscc_port_set_register(port, 2, DMACCR_OFFSET, 0x00000100);
}

void fscc_port_execute_STOP_T(struct fscc_port *port)
{
	return_if_untrue(port);
	return_if_untrue(fscc_port_has_dma(port) == 1);

	fscc_port_set_register(port, 2, DMACCR_OFFSET, 0x00000200);
}

unsigned fscc_port_using_async(struct fscc_port *port)
{
	return_val_if_untrue(port, 0);

	/* We must refresh FCR because it is shared with serialfc */
	port->register_storage.FCR = fscc_port_get_register(port, 2, FCR_OFFSET);

	switch (port->channel) {
	case 0:
		return port->register_storage.FCR & 0x01000000;

	case 1:
		return port->register_storage.FCR & 0x02000000;
	}

	return 0;
}

unsigned fscc_port_is_streaming(struct fscc_port *port)
{
	unsigned transparent_mode = 0;
	unsigned xsync_mode = 0;
	unsigned rlc_mode = 0;
	unsigned fsc_mode = 0;
	unsigned ntb = 0;

	return_val_if_untrue(port, 0);

	transparent_mode = ((port->register_storage.CCR0 & 0x3) == 0x2) ? 1 : 0;
	xsync_mode = ((port->register_storage.CCR0 & 0x3) == 0x1) ? 1 : 0;
	rlc_mode = (port->register_storage.CCR2 & 0xffff0000) ? 1 : 0;
	fsc_mode = (port->register_storage.CCR0 & 0x700) ? 1 : 0;
	ntb = (port->register_storage.CCR0 & 0x70000) >> 16;

	return ((transparent_mode || xsync_mode) && !(rlc_mode || fsc_mode || ntb)) ? 1 : 0;
}

unsigned fscc_port_has_dma(struct fscc_port *port)
{
	return_val_if_untrue(port, 0);

	if (force_fifo)
		return 0;

	return port->card->dma;
}

#ifdef DEBUG
unsigned fscc_port_get_interrupt_count(struct fscc_port *port, __u32 isr_bit)
{
	return_val_if_untrue(port, 0);

	return debug_interrupt_tracker_get_count(port->interrupt_tracker, isr_bit);
}

void fscc_port_increment_interrupt_counts(struct fscc_port *port,
										  __u32 isr_value)
{
	return_if_untrue(port);

	debug_interrupt_tracker_increment_all(port->interrupt_tracker, isr_value);
}
#endif

/* Returns -EINVAL if you set an incorrect transmit modifier */
int fscc_port_set_tx_modifiers(struct fscc_port *port, int value)
{
	return_val_if_untrue(port, 0);

	switch (value) {
		case XF:
		case XF|TXT:
		case XF|TXEXT:
		case XREP:
		case XREP|TXT:
			if (port->tx_modifiers != value) {
				dev_dbg(port->device, "transmit modifiers 0x%x => 0x%x\n",
						port->tx_modifiers, value);
			}
			else {
				dev_dbg(port->device, "transmit modifiers 0x%x\n",
						value);
			}

			port->tx_modifiers = value;

			break;

		default:
			dev_warn(port->device, "tx modifiers (invalid value 0x%x)\n",
					 value);

			return -EINVAL;
	}

	return 1;
}

unsigned fscc_port_get_tx_modifiers(struct fscc_port *port)
{
	return_val_if_untrue(port, 0);

	return port->tx_modifiers;
}

void fscc_port_execute_transmit(struct fscc_port *port, unsigned dma)
{
	unsigned command_register = 0;
	unsigned command_value = 0;
	unsigned command_bar = 0;

	return_if_untrue(port);

	if (dma) {
		command_bar = 2;
		command_register = DMACCR_OFFSET;
		command_value = 0x00000002;

		if (port->tx_modifiers & XREP)
			command_value |= 0x40000000;

		if (port->tx_modifiers & TXT)
			command_value |= 0x10000000;

		if (port->tx_modifiers & TXEXT)
			command_value |= 0x20000000;
	}
	else {
		command_bar = 0;
		command_register = CMDR_OFFSET;
		command_value = 0x01000000;

		if (port->tx_modifiers & XREP)
			command_value |= 0x02000000;

		if (port->tx_modifiers & TXT)
			command_value |= 0x10000000;

		if (port->tx_modifiers & TXEXT)
			command_value |= 0x20000000;
	}

	fscc_port_set_register(port, command_bar, command_register, command_value);
}

#define TX_FIFO_SIZE 4096

int prepare_frame_for_dma(struct fscc_port *port, struct fscc_frame *frame,
                          unsigned *length)
{
	struct fscc_frame *last_frame = 0;
	unsigned long sent_flags = 0;

	fscc_frame_setup_descriptors(frame);

	spin_lock_irqsave(&port->sent_oframes_spinlock, sent_flags);

	last_frame = fscc_flist_peek_back(&port->sent_oframes);

	if (last_frame && last_frame->d1->control != 0x40000000) {
		// Wait until last frame is finished
		spin_unlock_irqrestore(&port->sent_oframes_spinlock, sent_flags);
		//fscc_port_set_register(port, 2, DMA_TX_BASE_OFFSET, frame->d1_handle);
		return 0;
	}
	else {
		fscc_port_set_register(port, 2, DMA_TX_BASE_OFFSET, frame->d1_handle);
		*length = fscc_frame_get_length(frame);
		spin_unlock_irqrestore(&port->sent_oframes_spinlock, sent_flags);
		return 2;
	}
}

int prepare_frame_for_fifo(struct fscc_port *port, struct fscc_frame *frame,
                           unsigned *length)
{
	unsigned current_length = 0;
	unsigned buffer_size = 0;
	unsigned fifo_space = 0;
	unsigned size_in_fifo = 0;
	unsigned transmit_length = 0;

	current_length = fscc_frame_get_length(frame);
	buffer_size = fscc_frame_get_buffer_size(frame);
	size_in_fifo = current_length + (4 - current_length % 4);

	/* Subtracts 1 so a TDO overflow doesn't happen on the 4096th byte. */
	fifo_space = TX_FIFO_SIZE - fscc_port_get_TXCNT(port) - 1;
	fifo_space -= fifo_space % 4;

	/* Determine the maximum amount of data we can send this time around. */
	transmit_length = (size_in_fifo > fifo_space) ? fifo_space : current_length;

	frame->fifo_initialized = 1;

	if (transmit_length == 0)
		return 0;

	fscc_port_set_register_rep(port, 0, FIFO_OFFSET,
							   frame->buffer,
							   transmit_length);

	fscc_frame_remove_data(frame, NULL, transmit_length);

	*length = transmit_length;

	/* If this is the first time we add data to the FIFO for this frame we
	   tell the port how much data is in this frame. */
	if (current_length == buffer_size)
		fscc_port_set_register(port, 0, BC_FIFO_L_OFFSET, buffer_size);

	/* We still have more data to send. */
	if (!fscc_frame_is_empty(frame))
		return 1;

	return 2;
}

unsigned fscc_port_transmit_frame(struct fscc_port *port, struct fscc_frame *frame)
{
	unsigned transmit_dma = 0;
	unsigned transmit_length = 0;
	int result;

	if (fscc_port_has_dma(port) &&
	   (fscc_frame_get_length(frame) <= TX_FIFO_SIZE) &&
	   !fscc_frame_is_fifo(frame)) {
		transmit_dma = 1;
	}

	if (transmit_dma)
		result = prepare_frame_for_dma(port, frame, &transmit_length);
	else
		result = prepare_frame_for_fifo(port, frame, &transmit_length);

	if (result)
		fscc_port_execute_transmit(port, transmit_dma);

	dev_dbg(port->device, "F#%i => %i byte%s%s\n",
			frame->number, transmit_length,
			(transmit_length == 1) ? "" : "s",
			(result != 2) ? " (starting)" : "");

	return result;
}
