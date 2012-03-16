#include <stdarg.h>
#include <mach/sec_api.h>
#include <linux/smp.h>

extern unsigned hw_sec_rom_pub_bridge(unsigned appl_id, unsigned flags,
				      va_list);

struct sec_api_data {
	va_list argp;
	unsigned return_value;
	unsigned appl_id;
	unsigned flags;
};

static unsigned read_cpsr(void)
{
	unsigned cpsr;
	asm volatile ("mrs  %0,cpsr" : "=r" (cpsr));
	return cpsr;
}

/* This function exclusively runs on Core 0 with preemption disabled */
static void secure_api_call_shim(void *info)
{

	struct sec_api_data *data = (struct sec_api_data *)info;
	unsigned pub_cpsr = read_cpsr();

	printk(KERN_NOTICE "About to trap into Secure Monitor on CPU %d\n",
	       raw_smp_processor_id());

	/* FIQ won't be enabled in secure mode if FIQ is currently disabled */
	if (pub_cpsr & FIQ_MASK)
		data->flags &= ~SEC_ROM_FIQ_ENABLE_MASK;

	/* IRQ won't be enabled in secure mode if IRQ is currently disabled */
	if (pub_cpsr & IRQ_MASK)
		data->flags &= ~SEC_ROM_IRQ_ENABLE_MASK;

	data->return_value =
	    hw_sec_rom_pub_bridge(data->appl_id, data->flags, data->argp);
}

unsigned secure_api_call(unsigned appl_id, unsigned flags, ...)
{
	struct sec_api_data data;

	data.appl_id = appl_id;
	data.flags = flags;

	va_start(data.argp, flags);
	smp_call_function_single(0, secure_api_call_shim, (void *)&data, 1);
	va_end(data.argp);

	return data.return_value;
}
