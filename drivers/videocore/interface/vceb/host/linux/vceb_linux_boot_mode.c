/*****************************************************************************
* Copyright 2008 - 2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#include <linux/module.h>
#include <linux/proc_fs.h>

#include "vceb_linux_boot_mode.h"

static struct proc_dir_entry *vceb_boot_dir_entry;

/*
 * This little bit of code needs to be statically linked into the kernel, even 
 * if the rest of vceb is compiled as a module. 
 */

char vceb_boot_mode[VCEB_MAX_BOOT_MODE_LEN] = "none";
EXPORT_SYMBOL( vceb_boot_mode );

static int __init vc_boot_mode_setup( char *str )
{
    strlcpy( vceb_boot_mode, str, sizeof( vceb_boot_mode ));

    return 1;
}
__setup( "vc-boot-mode=", vc_boot_mode_setup );

int vceb_skip_boot( void )
{
    return strcmp( vceb_boot_mode, "skip" ) == 0;
}
EXPORT_SYMBOL( vceb_skip_boot );

int vceb_jtag_boot( void )
{
    return strcmp( vceb_boot_mode, "jtag" ) == 0;
}
EXPORT_SYMBOL( vceb_jtag_boot );

int vceb_bootloader_boot( void )
{
    return strstr( vceb_boot_mode, "boot" ) != NULL;
}
EXPORT_SYMBOL( vceb_bootloader_boot );

/****************************************************************************
*
*   vceb_boot_proc_read
*
***************************************************************************/

static int vceb_boot_proc_read(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    char    *p = buf;

    if (offset > 0)
    {
       *eof = 1;
       return 0;
    }

    p += sprintf( p, "%s\n", vceb_boot_mode );

    *eof = 1;
    return p - buf;
}

/****************************************************************************
*
*   vceb_boot_init
*
***************************************************************************/

static int __init vceb_boot_init( void )
{
    vceb_boot_dir_entry = create_proc_entry( "vc-boot-mode", 0444, NULL );
    if ( vceb_boot_dir_entry == NULL )
    {
        printk( KERN_ERR "%s: Unable to create vceb_boot_mode proc entry\n", __func__ );
        return -ENODEV;
    }
    vceb_boot_dir_entry->read_proc = vceb_boot_proc_read;

    return 0;
}

/****************************************************************************
*
*   vceb_boot_exit
*
***************************************************************************/

static void __exit vceb_boot_exit( void )
{
    if ( vceb_boot_dir_entry != NULL )
    {
        remove_proc_entry( vceb_boot_dir_entry->name, NULL );
    }
}

module_init(vceb_boot_init);
module_exit(vceb_boot_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("VCEB Boot Driver");
MODULE_LICENSE("GPL");

