#include <linux/module.h>
#include <mach/wfi_count.h>
#include <linux/sysctl.h>

/*
 * Storage for wfi_count (SW-7022). 
 */

atomic_t   wfi_count = ATOMIC_INIT(0);

EXPORT_SYMBOL( wfi_count );

int wfi_workaround_enabled = 1;

static struct ctl_table_header    *wfi_workaround_sysctl_header;
static struct ctl_table wfi_workaround_sysctl[] = {
   {
      .procname      = "wfi-workaround",
      .data          = &wfi_workaround_enabled,
      .maxlen        = sizeof( wfi_workaround_enabled ),
      .mode          = 0644,
      .proc_handler  = &proc_dointvec
   },

   {}
};

static int __init wfi_workaround_init( void )
{
   wfi_workaround_sysctl_header = register_sysctl_table( wfi_workaround_sysctl );
   if ( wfi_workaround_sysctl_header == NULL )
   {
      printk(KERN_ERR "%s: could not register sysctl table\n", __FUNCTION__);
   }
}

__initcall( wfi_workaround_init );
