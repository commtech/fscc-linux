#include "debug.h"
#include "utils.h" /* return_{val_}if_true */

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

void debug_interrupt_tracker_increment_single(struct debug_interrupt_tracker *tracker,
                                       __u32 isr_bit)
{
	unsigned i = 0;
	
	return_if_untrue(tracker);
	
	if (isr_bit == 0)
		return;
	
	isr_bit >>= 1;
	
	while (isr_bit) {
		isr_bit >>= 1;
		i++;
	}
	
	(*((unsigned *)tracker + i))++;
}

void debug_interrupt_tracker_increment_all(struct debug_interrupt_tracker *tracker,
                                       __u32 isr_value)
{
	unsigned i = 0;
	
	return_if_untrue(tracker);
	
	if (isr_value == 0)
		return;
	
	for (i = 0; i < sizeof(*tracker) / sizeof(unsigned); i++) {
		if (isr_value & 0x00000001)
			(*((unsigned *)tracker + i))++;
		
		isr_value >>= 1;
	}
}

unsigned debug_interrupt_tracker_get_count(struct debug_interrupt_tracker *tracker,
                                           __u32 isr_bit)
{
	unsigned i = 0;
	
	return_val_if_untrue(tracker, 0);
	return_val_if_untrue(isr_bit != 0, 0);
	
	isr_bit >>= 1;
	
	while (isr_bit) {
		isr_bit >>= 1;
		i++;
	}
	
	return *((unsigned *)tracker + i);
}
