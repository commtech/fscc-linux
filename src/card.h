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
