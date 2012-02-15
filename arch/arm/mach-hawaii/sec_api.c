#include <stdarg.h>
#include "sec_api.h"


extern unsigned hw_sec_rom_pub_bridge(unsigned appl_id, unsigned flags, va_list);

static unsigned read_cpsr()
{
	unsigned cpsr;
	asm volatile ("mrs  %0,cpsr" : "=r"(cpsr));
	return cpsr;
}


unsigned secure_api_call(unsigned appl_id, unsigned flags, ...)
{
    va_list ap;
    unsigned return_value;
    unsigned pub_cpsr;

    pub_cpsr = read_cpsr(); 
  
    // FIQ won't be enabled in secure mode if FIQ is currently disabled
    if (pub_cpsr & FIQ_MASK)
        flags &= ~SEC_ROM_FIQ_ENABLE_MASK;

    // IRQ won't be enabled in secure mode if IRQ is currently disabled
    if (pub_cpsr & IRQ_MASK)
        flags &= ~SEC_ROM_IRQ_ENABLE_MASK;


    va_start(ap, flags);
    return_value = hw_sec_rom_pub_bridge(appl_id, flags, ap);
    va_end(ap);

    return return_value;
}

