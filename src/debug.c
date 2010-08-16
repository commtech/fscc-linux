#include "debug.h"
#include "port.h"
#include "utils.h"
#include "config.h"
#include <linux/module.h>

struct debug_interrupt_tracker *debug_interrupt_tracker_new(void)
{
	struct debug_interrupt_tracker *tracker = 0;
	
	tracker = kmalloc(sizeof(*tracker), GFP_KERNEL);
	
	memset(tracker, 0, sizeof(*tracker));
	
	return tracker;
}

void debug_interrupt_tracker_delete(struct debug_interrupt_tracker *tracker)
{
	return_if_untrue(tracker);
		
	kfree(tracker);
}

void debug_interrupt_tracker_increment(struct debug_interrupt_tracker *tracker,
                                       __u32 isr_bit)
{
	unsigned i = 0;
	
	return_if_untrue(tracker);
	
	isr_bit >>= 1;
	
	while (isr_bit) {
		isr_bit >>= 1;
		i++;
	}
	
	(*((unsigned *)tracker + sizeof(unsigned) * i))++;
}

unsigned debug_interrupt_tracker_get_count(struct debug_interrupt_tracker *tracker,
                                           __u32 isr_bit)
{
	unsigned i = 0;
	
	return_val_if_untrue(tracker, 0);
	
	isr_bit >>= 1;
	
	while (isr_bit) {
		isr_bit >>= 1;
		i++;
	}
	
	return *((unsigned *)tracker + sizeof(unsigned) * i);
}
