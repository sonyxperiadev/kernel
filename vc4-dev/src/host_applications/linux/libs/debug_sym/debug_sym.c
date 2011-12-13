/*****************************************************************************
* Copyright 2001 - 2011 Broadcom Corporation.  All rights reserved.
*
* This program is the proprietary software of Broadcom Corporation and/or
* its licensors, and may only be used, duplicated, modified or distributed
* pursuant to the terms and conditions of a separate, written license
* agreement executed between you and Broadcom (an "Authorized License").
* Except as set forth in an Authorized License, Broadcom grants no license
* (express or implied), right to use, or waiver of any kind with respect to
* the Software, and Broadcom expressly reserves all rights in and to the
* Software and all intellectual property rights therein.  IF YOU HAVE NO
* AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
* WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
* THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use
*    all reasonable efforts to protect the confidentiality thereof, and to
*    use this information only in connection with your use of Broadcom
*    integrated circuit products.
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
*    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
*    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
*    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*****************************************************************************/
/**
*
*  @file    debug_sym.c
*
*  @brief   The usermode process which implements displays the messages.
*
****************************************************************************/

// ---- Include Files -------------------------------------------------------

#include "interface/vcos/vcos.h"

#if defined(__KERNEL__)
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/cdev.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/pagemap.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include <linux/semaphore.h>
#include <linux/proc_fs.h>
#include <linux/dma-mapping.h>
#include <linux/pfn.h>
#include <linux/hugetlb.h>
#include <linux/seq_file.h>
#include <linux/list.h>
#include <linux/videocore/vc_mem.h>
#else
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#if defined( WIN32 )
#include <io.h>
#else
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videocore/vc_mem.h>
#endif
#endif

#include "debug_sym.h"
#include "vcinclude/vc_debug_sym.h"


// ---- Public Variables ----------------------------------------------------
// ---- Private Constants and Types -----------------------------------------

#ifndef PAGE_SIZE
#define PAGE_SIZE   4096
#define PAGE_MASK   (~(PAGE_SIZE - 1))
#endif

// Offset within the videocore memory map to get the address of the symbol
// table.
#define VC_SYMBOL_BASE_OFFSET       0x2800

struct opaque_vc_mem_access_handle_t
{
    int                 memFd;
    size_t              vcMemSize;

    VC_MEM_ADDR_T       vcSymbolTableOffset;
    unsigned            numSymbols;
    VC_DEBUG_SYMBOL_T  *symbol;
};

#if defined(__KERNEL__)
#define DBG( fmt, ... )
// #define DBG( fmt, ... )    printk( KERN_INFO  "[D]:%s: " fmt "\n", __func__, ##__VA_ARGS__ )
#define ERR( fmt, ... )    printk( KERN_ERR   "[E]:%s: " fmt "\n", __func__, ##__VA_ARGS__ )
#else
   #if 1
      #define DBG( fmt, ... )    vcos_log_trace( "%s: " fmt, __FUNCTION__, ##__VA_ARGS__ )
      #define ERR( fmt, ... )    vcos_log_error( "%s: " fmt, __FUNCTION__, ##__VA_ARGS__ )
   #else
      #define DBG( fmt, ... )    fprintf( stderr, "%s: " fmt "\n", __FUNCTION__, ##__VA_ARGS__ )
      #define ERR( fmt, ... )    fprintf( stderr, "%s: " fmt "\n", __FUNCTION__, ##__VA_ARGS__ )
   #endif
#endif

typedef enum
{
    READ_MEM,
    WRITE_MEM,
} MEM_OP_T;

#if defined( WIN32 )
#define	open		_open
#define	close		_close
#define	O_SYNC		0
#endif

// ---- Private Variables ---------------------------------------------------

#if !defined(__KERNEL__)
#define  VCOS_LOG_CATEGORY (&debug_sym_log_category)
static VCOS_LOG_CAT_T  debug_sym_log_category;
#endif

// ---- Private Function Prototypes -----------------------------------------

// ---- Functions -----------------------------------------------------------

/****************************************************************************
*
*   Get access to the videocore memory space. Returns zero if the memory was
*   opened successfully.
*
***************************************************************************/

int OpenVideoCoreMemory( VC_MEM_ACCESS_HANDLE_T *vcHandlePtr  )
{
    return OpenVideoCoreMemoryFile( NULL, vcHandlePtr );
}

/****************************************************************************
*
*   Get access to the videocore memory space. Returns zero if the memory was
*   opened successfully.
*
***************************************************************************/

int OpenVideoCoreMemoryFile( const char *filename, VC_MEM_ACCESS_HANDLE_T *vcHandlePtr )
{
    int                     rc = 0;
    VC_MEM_ACCESS_HANDLE_T  newHandle;
    VC_DEBUG_SYMBOL_T       debug_sym;
    VC_MEM_ADDR_T           symAddr;
    size_t                  symTableSize;
    unsigned                symIdx;

#if !defined(__KERNEL__)
    vcos_log_register( "debug_sym", &debug_sym_log_category );
#endif

#if defined(__KERNEL__)
    if (( newHandle = kzalloc( sizeof( *newHandle ), GFP_KERNEL )) == NULL )
    {
        return -ENOMEM;
    }
#else
    if (( newHandle = calloc( 1, sizeof( *newHandle ))) == NULL )
    {
        return -ENOMEM;
    }

    if ( filename == NULL )
    {
        filename = "/dev/vc-mem";
    }

    if (( newHandle->memFd = open( filename, O_RDWR | O_SYNC )) < 0 )
    {
        ERR( "Unable to open '%s': %s(%d)\n", filename, strerror( errno ), errno );
        return -errno;
    }
    DBG( "Opened %s memFd = %d", filename, newHandle->memFd );
#endif

#if defined( WIN32 )
#define VC_MEM_SIZE  (128 * 1024 * 1024)
    newHandle->vcMemSize = VC_MEM_SIZE;
#elif defined(__KERNEL__)
    newHandle->vcMemSize = vc_mem_get_current_size();
#else
    if ( ioctl( newHandle->memFd, VC_MEM_IOC_MEM_SIZE, &newHandle->vcMemSize ) != 0 )
    {
        ERR( "Failed to get memory size via ioctl: %s(%d)\n",
             strerror( errno ), errno );
        return -errno;
    }
#endif
    DBG( "vcMemSize = %zu\n", newHandle->vcMemSize );

    // See if we can detect the symbol table

    if ( !ReadVideoCoreMemory( newHandle,
                               &newHandle->vcSymbolTableOffset,
                               VC_SYMBOL_BASE_OFFSET,
                               sizeof( newHandle->vcSymbolTableOffset )))
    {
        ERR( "ReadVideoCoreMemory @VC_SYMBOL_BASE_OFFSET (0x%08x) failed\n", VC_SYMBOL_BASE_OFFSET );
        rc = -EIO;
        goto err_exit;
    }
    DBG( "vcSymbolTableOffset = 0x%08x", newHandle->vcSymbolTableOffset );

    // Make sure that the pointer points into the first few megabytes of
    // the memory space.

    if ( newHandle->vcSymbolTableOffset > ( 4 * 1024 * 1024 ))
    {
        ERR( "newHandle->vcSymbolTableOffset (%d) > 4Mb\n", newHandle->vcSymbolTableOffset );
        rc = -EIO;
        goto err_exit;
    }

    // Make a pass to count how many symbols there are.

    symAddr = newHandle->vcSymbolTableOffset;
    newHandle->numSymbols = 0;
    do
    {
        if ( !ReadVideoCoreMemory( newHandle,
                                   &debug_sym,
                                   symAddr,
                                   sizeof( debug_sym )))
        {
            ERR( "ReadVideoCoreMemory @ symAddr(0x%08x) failed\n", symAddr );
            rc = -EIO;
            goto err_exit;
        }

        newHandle->numSymbols++;

        DBG( "Symbol %d: label: 0x%p addr: 0x%08x size: %zu",
             newHandle->numSymbols,
             debug_sym.label,
             debug_sym.addr,
             debug_sym.size );

        if ( newHandle->numSymbols > 1024 )
        {
            // Something isn't sane.

            ERR( "numSymobls (%d) > 1024 - looks wrong\n", newHandle->numSymbols );
            rc = -EIO;
            goto err_exit;
        }
        symAddr += sizeof( debug_sym );

    } while ( debug_sym.label != 0 );
    newHandle->numSymbols--;

    DBG( "Detected %d symbols", newHandle->numSymbols );

    // Allocate some memory to hold the symbols, and read them in.

    symTableSize = newHandle->numSymbols * sizeof( debug_sym );
#if defined(__KERNEL__)
    if (( newHandle->symbol = kzalloc( symTableSize, GFP_KERNEL )) == NULL )
#else
    if (( newHandle->symbol = malloc( symTableSize )) == NULL )
#endif
    {
        rc = -ENOMEM;
        goto err_exit;
    }
    if ( !ReadVideoCoreMemory( newHandle,
                               newHandle->symbol,
                               newHandle->vcSymbolTableOffset,
                               symTableSize ))
    {
        ERR( "ReadVideoCoreMemory @ newHandle->vcSymbolTableOffset(0x%08x) failed\n", newHandle->vcSymbolTableOffset );
        rc = -EIO;
        goto err_exit;
    }

    // The names of the symbols are pointers in videocore space. We want
    // to have them available locally, so we make copies and fixup
    // the pointer.

    for ( symIdx = 0; symIdx < newHandle->numSymbols; symIdx++ )
    {
        VC_DEBUG_SYMBOL_T   *sym;
        char                 symName[ 256 ];

        sym = &newHandle->symbol[ symIdx ];

        DBG( "Symbol %d: label: 0x%p addr: 0x%08x size: %zu",
             symIdx,
             sym->label,
             sym->addr,
             sym->size );

        if ( !ReadVideoCoreMemory( newHandle,
                                   symName,
                                   TO_VC_MEM_ADDR(sym->label),
                                   sizeof( symName )))
        {
            ERR( "ReadVideoCoreMemory @ sym->label(0x%08x) failed\n", sym->addr );
            rc = -EIO;
            goto err_exit;
        }
        symName[ sizeof( symName ) - 1 ] = '\0';
        *((const char **)&sym->label) = vcos_strdup( symName );

        DBG( "Symbol %d (@0x%p): label: '%s' addr: 0x%08x size: %zu",
             symIdx,
             sym,
             sym->label,
             sym->addr,
             sym->size );
    }

    *vcHandlePtr = newHandle;
    return 0;

err_exit:
#if defined(__KERNEL__)
    kfree( newHandle );
#else
    close( newHandle->memFd );
    free( newHandle );
#endif

    return rc;
}

/****************************************************************************
*
*   Returns the number of symbols which were detected.
*
***************************************************************************/

unsigned NumVideoCoreSymbols( VC_MEM_ACCESS_HANDLE_T vcHandle )
{
    return vcHandle->numSymbols;
}

/****************************************************************************
*
*   Returns the name, address and size of the i'th symbol.
*
***************************************************************************/

int GetVideoCoreSymbol( VC_MEM_ACCESS_HANDLE_T vcHandle, unsigned idx, char *labelBuf, size_t labelBufSize, VC_MEM_ADDR_T *vcMemAddr, size_t *vcMemSize )
{
    VC_DEBUG_SYMBOL_T   *sym;

    if ( idx >= vcHandle->numSymbols )
    {
        return -EINVAL;
    }
    sym = &vcHandle->symbol[ idx ];

    strncpy( labelBuf, sym->label, labelBufSize );
    labelBuf[labelBufSize - 1] = '\0';

    if ( vcMemAddr != NULL )
    {
        *vcMemAddr = (VC_MEM_ADDR_T)sym->addr;
    }
    if ( vcMemSize != NULL )
    {
        *vcMemSize = sym->size;
    }

    return 0;
}

/****************************************************************************
*
*   Looks up the named, symbol. If the symbol is found, it's value and size
*   are returned.
*
*   Returns  true if the lookup was successful.
*
***************************************************************************/

int LookupVideoCoreSymbol( VC_MEM_ACCESS_HANDLE_T vcHandle, const char *symbol, VC_MEM_ADDR_T *vcMemAddr, size_t *vcMemSize )
{
    unsigned        idx;
    char            symName[ 64 ];
    VC_MEM_ADDR_T   symAddr;
    size_t          symSize;

    for ( idx = 0; idx < vcHandle->numSymbols; idx++ )
    {
        GetVideoCoreSymbol( vcHandle, idx, symName, sizeof( symName ), &symAddr, &symSize );
        if ( strcmp( symbol, symName ) == 0 )
        {
            if ( vcMemAddr != NULL )
            {
                *vcMemAddr = symAddr;
            }
            if ( vcMemSize != 0 )
            {
                *vcMemSize = symSize;
            }

            DBG( "%s found, addr = 0x%08x size = %zu", symbol, symAddr, symSize );
            return 1;
        }
    }

    if ( vcMemAddr != NULL )
    {
        *vcMemAddr = 0;
    }
    if ( vcMemSize != 0 )
    {
        *vcMemSize = 0;
    }
    DBG( "%s not found", symbol );
    return 0;
}

/****************************************************************************
*
*   Looks up the named, symbol. If the symbol is found, and it's size is equal
*   to the sizeof a uint32_t, then true is returned.
*
***************************************************************************/

int LookupVideoCoreUInt32Symbol( VC_MEM_ACCESS_HANDLE_T vcHandle,
                                 const char *symbol,
                                 VC_MEM_ADDR_T *vcMemAddr )
{
    size_t  vcMemSize;

    if ( !LookupVideoCoreSymbol( vcHandle, symbol, vcMemAddr, &vcMemSize ))
    {
        return 0;
    }

    if ( vcMemSize != sizeof( uint32_t ))
    {
        ERR( "Symbol: '%s' has a size of %zu, expecting %zu", symbol, vcMemSize, sizeof( uint32_t ));
        return 0;
    }
    return 1;
}

/****************************************************************************
*
*   Does Reads or Writes on the videocore memory.
*
***************************************************************************/

static int AccessVideoCoreMemory( VC_MEM_ACCESS_HANDLE_T vcHandle,
                                  MEM_OP_T               mem_op,
                                  void                  *buf,
                                  VC_MEM_ADDR_T          vcMemAddr,
                                  size_t                 numBytes )
{
    DBG( "%s %zu bytes @ 0x%08x", mem_op == WRITE_MEM ? "Write" : "Read", numBytes, vcMemAddr );

    /*
     * Since we'll be passed videocore pointers, we need to deal with the high bits.
     *
     * We need to strip off the high 2 bits to convert to a physical address, except
     * for when the high 3 bits are equal to 011, which means that it corresponds to
     * a peripheral and isn't accessible.
     */

    if ( IS_ALIAS_PERIPHERAL( vcMemAddr ))
    {
        // This is a peripheral address.

        ERR( "Can't access peripheral address 0x%08x", vcMemAddr );
        return 0;
    }
    vcMemAddr = TO_VC_MEM_ADDR(ALIAS_NORMAL( vcMemAddr ));

    if ( vcMemAddr > vcHandle->vcMemSize )
    {
        ERR( "Memory address 0x%08x is > memory size 0x%08zx", vcMemAddr,
             vcHandle->vcMemSize );
        return 0;
    }
    if (( vcMemAddr + numBytes ) > vcHandle->vcMemSize )
    {
        ERR( "Memory address 0x%08x + numBytes 0x%08zx is > memory size 0x%08zx",
             vcMemAddr, numBytes, vcHandle->vcMemSize );
        return 0;
    }

#if defined( WIN32 )
    if ( mem_op != READ_MEM )
    {
        ERR( "Only reads are supported" );
        return 0;
    }
    if ( _lseek( vcHandle->memFd, vcMemAddr, SEEK_SET ) < 0 )
    {
        ERR( "_lseek position 0x%08x failed", vcMemAddr );
        return  0;
    }
    if ( _read( vcHandle->memFd, buf, numBytes ) < 0 )
    {
        ERR( "_read failed: errno = %d", errno );
        return 0;
    }
#elif defined(__KERNEL__)
    {
        uint8_t    *mapAddr;
        size_t      mapSize;
        size_t      memOffset;
        off_t       vcMapAddr;

        memOffset = vcMemAddr & ~PAGE_MASK;

        vcMapAddr = vcMemAddr & PAGE_MASK;

        vcMapAddr += mm_vc_mem_phys_addr;

        mapSize = ( memOffset + numBytes + PAGE_SIZE - 1 ) & PAGE_MASK;

        if (( mapAddr = ioremap_nocache( vcMapAddr, mapSize )) == 0 )
        {
           ERR( "ioremap_nocache failed." );
           return 0;
        }

        if ( mem_op == WRITE_MEM )
        {
           memcpy( mapAddr + memOffset, buf, numBytes );
        }
        else
        {
           memcpy( buf, mapAddr + memOffset, numBytes );
        }

        iounmap ( mapAddr );
    }
#else
    {
        uint8_t    *mapAddr;
        size_t      mapSize;
        size_t      memOffset;
        off_t       vcMapAddr;
        int         mmap_prot;

        if ( mem_op == WRITE_MEM )
        {
            mmap_prot = PROT_WRITE;
        }
        else
        {
            mmap_prot = PROT_READ;
        }

        // We can only map pages on 4K boundaries, so round the address down and the size up.

        memOffset = vcMemAddr & ~PAGE_MASK;

        vcMapAddr = vcMemAddr & PAGE_MASK;

        mapSize = ( memOffset + numBytes + PAGE_SIZE - 1 ) & PAGE_MASK;

        if (( mapAddr = mmap( 0, mapSize, mmap_prot, MAP_SHARED, vcHandle->memFd, vcMapAddr )) == MAP_FAILED )
        {
           ERR( "mmap failed: errno = %d",  errno );
           return 0;
        }
                if ( mem_op == WRITE_MEM )
        {
           memcpy( mapAddr + memOffset, buf, numBytes );
        }
        else
        {
           memcpy( buf, mapAddr + memOffset, numBytes );
        }

        munmap( mapAddr, mapSize );
    }
#endif

    return 1;
}


/****************************************************************************
*
*   Reads 'numBytes' from the videocore memory starting at 'vcMemAddr'. The
*   results are stored in 'buf'.
*
*   Returns true if the read was successful.
*
***************************************************************************/

int ReadVideoCoreMemory( VC_MEM_ACCESS_HANDLE_T vcHandle, void *buf, VC_MEM_ADDR_T vcMemAddr, size_t numBytes )
{
    return AccessVideoCoreMemory( vcHandle, READ_MEM, buf, vcMemAddr, numBytes );
}

/****************************************************************************
*
*   Reads 'numBytes' from the videocore memory starting at 'vcMemAddr'. The
*   results are stored in 'buf'.
*
*   Returns true if the read was successful.
*
***************************************************************************/

int ReadVideoCoreMemoryBySymbol( VC_MEM_ACCESS_HANDLE_T vcHandle, const char *symbol, void *buf, size_t bufSize )
{
    VC_MEM_ADDR_T   vcMemAddr;
    size_t          vcMemSize;

    if ( !LookupVideoCoreSymbol( vcHandle, symbol, &vcMemAddr, &vcMemSize ))
    {
        ERR( "Symbol not found: '%s'", symbol );
        return 0;
    }

    if ( vcMemSize > bufSize )
    {
        vcMemSize = bufSize;
    }

    if ( !ReadVideoCoreMemory( vcHandle, buf, vcMemAddr, vcMemSize ))
    {
        ERR( "Unable to read %zu bytes @ 0x%08x", vcMemSize, vcMemAddr );
        return 0;
    }
    return 1;
}

/****************************************************************************
*
*   Looks up a symbol and reads the contents into a user supplied buffer.
*
*   Returns true if the read was successful.
*
***************************************************************************/

int ReadVideoCoreStringBySymbol( VC_MEM_ACCESS_HANDLE_T vcHandle,
                                 const char *symbol,
                                 char *buf,
                                 size_t bufSize )
{
    VC_MEM_ADDR_T   vcMemAddr;
    size_t          vcMemSize;

    if ( !LookupVideoCoreSymbol( vcHandle, symbol, &vcMemAddr, &vcMemSize ))
    {
        ERR( "Symbol not found: '%s'", symbol );
        return 0;
    }

    if ( vcMemSize > bufSize )
    {
        vcMemSize = bufSize;
    }

    if ( !ReadVideoCoreMemory( vcHandle, buf, vcMemAddr, vcMemSize ))
    {
        ERR( "Unable to read %zu bytes @ 0x%08x", vcMemSize, vcMemAddr );
        return 0;
    }

    // Make sure that the result is null-terminated

    buf[vcMemSize-1] = '\0';
    return 1;
}

/****************************************************************************
*
*   Writes 'numBytes' into the videocore memory starting at 'vcMemAddr'. The
*   data is taken from 'buf'.
*
*   Returns true if the write was successful.
*
***************************************************************************/

int WriteVideoCoreMemory( VC_MEM_ACCESS_HANDLE_T vcHandle,
                          void *buf,
                          VC_MEM_ADDR_T vcMemAddr,
                          size_t numBytes )
{
    return AccessVideoCoreMemory( vcHandle, WRITE_MEM, buf, vcMemAddr, numBytes );
}

/****************************************************************************
*
*   Closes the memory space opened previously via OpenVideoCoreMemory.
*
***************************************************************************/

void CloseVideoCoreMemory( VC_MEM_ACCESS_HANDLE_T vcHandle )
{
#if defined(__KERNEL__)
    if ( vcHandle->symbol != NULL )
    {
        kfree( vcHandle->symbol );
    }

    kfree( vcHandle );
#else
    if ( vcHandle->symbol != NULL )
    {
        free( vcHandle->symbol );
    }

    if ( vcHandle->memFd >= 0 )
    {
        close( vcHandle->memFd );
    }
    free( vcHandle );
#endif
}

/****************************************************************************
*
*   Returns the size of the videocore memory space.
*
***************************************************************************/

size_t GetVideoCoreMemorySize( VC_MEM_ACCESS_HANDLE_T vcHandle )
{
    return vcHandle->vcMemSize;
}

