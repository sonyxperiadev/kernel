/*******************************************************************************
* Copyright 2008 - 2011 Broadcom Corporation.  All rights reserved.
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
*******************************************************************************/

#include <linux/module.h>
#include <linux/proc_fs.h>

#include <linux/videocore/vc_boot_mode.h>

static struct proc_dir_entry *vc_boot_proc_entry;

/*
 * This little bit of code needs to be statically linked into the kernel
 */

char vc_boot_mode[VC_MAX_BOOT_MODE_LEN] = "none";
EXPORT_SYMBOL( vc_boot_mode );

static int __init vc_boot_mode_setup( char *str )
{
    strlcpy( vc_boot_mode, str, sizeof( vc_boot_mode ));

    return 1;
}
__setup( "vc-boot-mode=", vc_boot_mode_setup );

static int vc_boot_proc_read( char *buf, char **start, off_t offset, int count, int *eof, void *data )
{
    char    *p = buf;

    if ( offset > 0 )
    {
       *eof = 1;
       return 0;
    }

    p += sprintf( p, "%s\n", vc_boot_mode );

    *eof = 1;
    return p - buf;
}

static int __init vc_boot_mode_init( void )
{
    vc_boot_proc_entry = create_proc_entry( "vc-boot-mode", 0444, NULL );
    if ( vc_boot_proc_entry == NULL )
    {
        printk( KERN_ERR "%s: Unable to create vc_boot_mode proc entry\n",
                __func__ );
        return -ENODEV;
    }
    vc_boot_proc_entry->read_proc = vc_boot_proc_read;

    return 0;
}

static void __exit vc_boot_mode_exit( void )
{
    if ( vc_boot_proc_entry != NULL )
    {
        remove_proc_entry( vc_boot_proc_entry->name, NULL );
    }
}
int vc_boot_mode_skip( void )
{
    return strcmp( vc_boot_mode, "skip" ) == 0;
}
EXPORT_SYMBOL( vc_boot_mode_skip );

int vc_boot_mode_jtag( void )
{
    return strcmp( vc_boot_mode, "jtag" ) == 0;
}
EXPORT_SYMBOL( vc_boot_mode_jtag );

int vc_boot_mode_bootloader( void )
{
    return strstr( vc_boot_mode, "boot" ) != NULL;
}
EXPORT_SYMBOL( vc_boot_mode_bootloader );

module_init( vc_boot_mode_init );
module_exit( vc_boot_mode_exit );

MODULE_AUTHOR( "Broadcom" );
MODULE_DESCRIPTION( "VC Boot Mode Driver" );
MODULE_LICENSE( "GPL" );

