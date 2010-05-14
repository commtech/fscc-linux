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

#include "sysfs.h"
#include "port.h"
#include "card.h"
#include "config.h"

int str_to_offset(const char *str)
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
	else if (strcmp(str, "fcr") == 0)
		return FCR_OFFSET;
	else
		printk(KERN_NOTICE DEVICE_NAME " invalid str passed into str_to_offset\n");

	return -1;
}

static ssize_t register_store(struct kobject *kobj, struct kobj_attribute *attr,
                              const char *buf, size_t count, unsigned bar_number)
{
	struct fscc_port *port = 0;
	int register_offset = 0;
	unsigned value = 0;
    char *end = 0;
	
	port = (struct fscc_port *)dev_get_drvdata((struct device *)kobj);

	register_offset = str_to_offset(attr->attr.name);
	value = (unsigned)simple_strtoul(buf, &end, 16);

	if (register_offset >= 0) {
		printk(KERN_DEBUG "%s setting register 0x%02x to 0x%08x\n", port->name, register_offset, value);
		fscc_port_set_register(port, bar_number, register_offset, value);
		return count;
	}

	return 0;
}

static ssize_t register_show(struct kobject *kobj, struct kobj_attribute *attr,
                             char *buf, unsigned bar_number)

{
	struct fscc_port *port = 0;
	int register_offset = 0;
	
	port = (struct fscc_port *)dev_get_drvdata((struct device *)kobj);

	register_offset = str_to_offset(attr->attr.name);
	
	if (register_offset >= 0) {
		printk(KERN_DEBUG "%s reading register 0x%02x\n", port->name, register_offset);
		return sprintf(buf, "%08x\n", fscc_port_get_register(port, bar_number, (unsigned)register_offset));
	}

	return 0;
}

static ssize_t bar0_register_store(struct kobject *kobj, struct kobj_attribute *attr,
                                   const char *buf, size_t count)
{
	return register_store(kobj, attr, buf, count, 0);
}

static ssize_t bar0_register_show(struct kobject *kobj, struct kobj_attribute *attr,
                                  char *buf)
{
	return register_show(kobj, attr, buf, 0);
}

static ssize_t bar2_register_store(struct kobject *kobj, struct kobj_attribute *attr,
                                   const char *buf, size_t count)
{
	return register_store(kobj, attr, buf, count, 2);
}

static ssize_t bar2_register_show(struct kobject *kobj, struct kobj_attribute *attr,
                                  char *buf)
{
	return register_show(kobj, attr, buf, 2);
}

static struct kobj_attribute fifot_attribute = 
	__ATTR(fifot, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);
	
static struct kobj_attribute cmdr_attribute = 
	__ATTR(cmdr, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);
	
static struct kobj_attribute ccr0_attribute = 
	__ATTR(ccr0, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute ccr1_attribute = 
	__ATTR(ccr1, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute ccr2_attribute = 
	__ATTR(ccr2, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute bgr_attribute = 
	__ATTR(bgr, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute ssr_attribute = 
	__ATTR(ssr, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute smr_attribute = 
	__ATTR(smr, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute tsr_attribute = 
	__ATTR(tsr, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute tmr_attribute = 
	__ATTR(tmr, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute rar_attribute = 
	__ATTR(rar, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute ramr_attribute = 
	__ATTR(ramr, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute ppr_attribute = 
	__ATTR(ppr, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute tcr_attribute = 
	__ATTR(tcr, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute vstr_attribute = 
	__ATTR(vstr, SYSFS_READ_ONLY_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute imr_attribute = 
	__ATTR(imr, SYSFS_READ_WRITE_MODE, bar0_register_show, bar0_register_store);

static struct kobj_attribute fcr_attribute = 
	__ATTR(fcr, SYSFS_READ_WRITE_MODE, bar2_register_show, bar2_register_store);

static struct attribute *attrs[] = {
	&fifot_attribute.attr,
	&cmdr_attribute.attr,
	&ccr0_attribute.attr,
	&ccr1_attribute.attr,
	&ccr2_attribute.attr,
	&bgr_attribute.attr,
	&ssr_attribute.attr,
	&smr_attribute.attr,
	&tsr_attribute.attr,
	&tmr_attribute.attr,
	&rar_attribute.attr,
	&ramr_attribute.attr,
	&ppr_attribute.attr,
	&tcr_attribute.attr,
	&vstr_attribute.attr,
	&imr_attribute.attr,
	&fcr_attribute.attr,
	NULL,
};

struct attribute_group port_registers_attr_group = {
	.name = "registers",
	.attrs = attrs,
};

