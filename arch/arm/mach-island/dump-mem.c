/*****************************************************************************
* Copyright 2001 - 2009 Broadcom Corporation.  All rights reserved.
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

/****************************************************************************
*
*  dump-mem.c
*
*  PURPOSE:
*       Contains a function for dumping the contents of memory.
*
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/broadcom/dump-mem.h>

#if defined( CONFIG_BCM_KNLLOG_SUPPORT )
#   include <linux/broadcom/knllog.h>
#endif

/* ---- Public Variables ------------------------------------------------- */

/* ---- Private Constants and Types -------------------------------------- */

/* ---- Private Variables ------------------------------------------------ */

/* ---- Private Function Prototypes -------------------------------------- */

static inline   int LogToConsole( DUMP_DEST dumpDest )
{
    return ( dumpDest & DUMP_TO_CONSOLE ) != 0;
}

#if defined( CONFIG_BCM_KNLLOG_SUPPORT )
static inline   int LogToKnlLog( DUMP_DEST dumpDest )
{
    return ( dumpDest & DUMP_TO_KNLLOG ) != 0;
}
#endif

/* ---- Functions  ------------------------------------------------------- */

/****************************************************************************/
/**
*   Logging function used for dumping the contents of memory
*/
/****************************************************************************/

void dump_mem
(
    const char *function, 
    DUMP_DEST   dumpDest,
    uint32_t    addr, 
    const void *mem, 
    size_t      numBytes 
)
{
    if ( LogToConsole( dumpDest ))
    {
        printk_dump_mem( function, addr, mem, numBytes );
    }

#if defined( CONFIG_BCM_KNLLOG_SUPPORT )
    if ( LogToKnlLog( dumpDest ))
    {
        knllog_dump_mem( function, addr, mem, numBytes );
    }
#endif
}
EXPORT_SYMBOL( dump_mem );

/****************************************************************************
*
*  dump_mem
*
*   Used to dumping memory bytes
*
***************************************************************************/

void printk_dump_mem( const char *label, uint32_t addr, const void *voidMem, size_t numBytes )
{
    const uint8_t  *mem = (uint8_t *)voidMem;
    size_t          offset;
    char            lineBuf[ 100 ];
    char           *s;

    while ( numBytes > 0 )
    {
        s = lineBuf;

        for ( offset = 0; offset < 16; offset++ )
        {
            if ( offset < numBytes )
            {
                s += sprintf( s, "%02x ", mem[ offset ]);
            }
            else
            {
                s += sprintf( s, "   " );
            }
        }

        for ( offset = 0; offset < 16; offset++ )
        {
            if ( offset < numBytes )
            {
                uint8_t ch = mem[ offset ];

                if (( ch < ' ' ) || ( ch > '~' ))
                {
                    ch = '.';
                }
                *s++ = (char)ch;
            }
        }
        *s++ = '\0';

        printk( KERN_INFO "%s: %08x: %s\n", label, addr, lineBuf );

        addr += 16;
        mem += 16;
        if ( numBytes > 16 )
        {
            numBytes -= 16;
        }
        else
        {
            numBytes = 0;
        }
    }

} /* printk_dump_mem */

EXPORT_SYMBOL( printk_dump_mem );

/* ************************************ The End ***************************************** */

