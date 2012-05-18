#include <stdarg.h>
#include <mach/sec_api.h>
#include <linux/smp.h>
#include <linux/io.h>
#include <linux/ioport.h>

#include <asm/cacheflush.h>

#define SEC_EXIT_NORMAL 1
#define SSAPI_RET_FROM_INT_SERV 4

#define SEC_BUFFER_ADDR 0x3404C000 /* SRAM */
#define SEC_BUFFER_SIZE 0x00000400 /* 1kB */

struct secure_bridge_data {
	u32 *bounce;
	int initialized;
} bridge_data;

struct sec_api_data {
	unsigned service_id;
	unsigned arg0;
	unsigned arg1;
	unsigned arg2;
};

unsigned get_secure_buffer(void)
{
	return SEC_BUFFER_ADDR;
}

unsigned get_secure_buffer_size(void)
{
	return SEC_BUFFER_SIZE;
}

static void smc(struct sec_api_data *data)
{
	/* Set Up Registers to pass data to Secure Monitor */
	register u32 r4 asm("r4") = data->service_id;
	register u32 r5 asm("r5") = 0x3; /* Keep IRQ and FIQ off in SM */
	register u32 r6 asm("r6") = SEC_BUFFER_ADDR;
	register u32 r12 asm("r12");

	do {
		/* Secure Monitor Call */
		asm volatile (
			__asmeq("%0", "ip")
			__asmeq("%1", "r4")
			__asmeq("%2", "r5")
			__asmeq("%3", "r6")
#ifdef REQUIRES_SEC
			".arch_extension sec\n"
#endif
			"smc	#0	@ switch to secure world\n"
			: "+r" (r12), "+r" (r4), "+r" (r5), "+r" (r6)
			:
			: "r0", "r1", "r2", "r3", "r7", "r8", "r14");

		/* Setup registers in case we need to re-enter secure mode */
		r4 = SSAPI_RET_FROM_INT_SERV;
		r5 = 0x3;
		r6 = SEC_BUFFER_ADDR;

	} while (r12 != SEC_EXIT_NORMAL);
}

/* Map in the bounce area */
void secure_api_call_init(void)
{
	struct resource *res;

	res = request_mem_region(SEC_BUFFER_ADDR, SEC_BUFFER_SIZE,
		"secure_bounce");
	BUG_ON(!res);

	bridge_data.bounce = (u32 *) ioremap_nocache(SEC_BUFFER_ADDR,
		SEC_BUFFER_SIZE);
	BUG_ON(!bridge_data.bounce);

	bridge_data.initialized = 1;
}

/* This function exclusively runs on Core 0 with preemption disabled */
static void secure_api_call_shim(void *info)
{
	struct sec_api_data *data = (struct sec_api_data *)info;

	/* Check map in the bounce area */
	BUG_ON(!bridge_data.initialized);

	/* Copy one 32 bit word into the bounce area */
	bridge_data.bounce[0] = data->arg0;
	bridge_data.bounce[1] = data->arg1;
	bridge_data.bounce[2] = data->arg2;

	/* Flush caches for input data passed to Secure Monitor */
	flush_cache_all();

	printk(KERN_NOTICE "About to trap into Secure Monitor on CPU %d\n",
		raw_smp_processor_id());

	/* Trap into Secure Monitor */
	smc(data);

}

unsigned secure_api_call(unsigned service_id, unsigned arg0, unsigned arg1,
	unsigned arg2)
{
	struct sec_api_data data;

	data.service_id = service_id;
	data.arg0 = arg0;
	data.arg1 = arg1;
	data.arg2 = arg2;

	/* Request Secure Monitor Code Run from Core 0 */
	smp_call_function_single(0, secure_api_call_shim, (void *)&data, 1);

	return 0;
}
