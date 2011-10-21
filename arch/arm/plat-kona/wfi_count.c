#include <linux/module.h>
#include <mach/wfi_count.h>

/*
 * Storage for wfi_count (SW-7022). 
 */

atomic_t   wfi_count = ATOMIC_INIT(0);

EXPORT_SYMBOL( wfi_count );
