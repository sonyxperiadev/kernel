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

#if !defined( DEBUG_SYM_H )
#define DEBUG_SYM_H

/* ---- Include Files ----------------------------------------------------- */

#include "interface/vcos/vcos.h"
#include "interface/vcos/vcos_stdint.h"

/* ---- Constants and Types ---------------------------------------------- */

typedef struct opaque_vc_mem_access_handle_t *VC_MEM_ACCESS_HANDLE_T;

typedef uint32_t    VC_MEM_ADDR_T;

#define TO_VC_MEM_ADDR(ptr)    ((VC_MEM_ADDR_T)(unsigned long)(ptr))

/* ---- Variable Externs ------------------------------------------------- */

/* ---- Function Prototypes ---------------------------------------------- */

/*
 * The following were taken from vcinclude/hardware_vc4_bigisland.h
 */

#define ALIAS_NORMAL(x)             ((void*)(((unsigned long)(x)&~0xc0000000uL)|0x00000000uL)) // normal cached data (uses main 128K L2 cache)
#define IS_ALIAS_PERIPHERAL(x)      (((unsigned long)(x)>>29)==0x3uL)

/*
 * Get access to the videocore memory space. Returns zero if the memory was
 * opened successfully, or a negative value (-errno) if the access could not
 * be obtained.
 */
int OpenVideoCoreMemory( VC_MEM_ACCESS_HANDLE_T *handle  );

/*
 * Get access to the videocore space from a file. The file might be /dev/mem, or 
 * it might be saved image on disk. 
 */
int OpenVideoCoreMemoryFile( const char *filename, VC_MEM_ACCESS_HANDLE_T *vcHandlePtr );

/*
 * Returns the number of symbols which were detected.
 */
unsigned NumVideoCoreSymbols( VC_MEM_ACCESS_HANDLE_T handle );

/*
 * Returns the name, address and size of the i'th symbol.
 */
int GetVideoCoreSymbol( VC_MEM_ACCESS_HANDLE_T handle,
                        unsigned idx,
                        char *nameBuf,
                        size_t nameBufSize,
                        VC_MEM_ADDR_T *vcMemAddr,
                        size_t *vcMemSize );

/*
 * Looks up the named, symbol. If the symbol is found, it's value and size
 * are returned.
 *
 * Returns  true if the lookup was successful.
 */
int LookupVideoCoreSymbol( VC_MEM_ACCESS_HANDLE_T handle,
                           const char *symbol,
                           VC_MEM_ADDR_T *vcMemAddr,
                           size_t *vcMemSize );

/*
 * Looks up the named, symbol. If the symbol is found, and it's size is equal
 * to the sizeof a uint32_t, then true is returned.
 */
int LookupVideoCoreUInt32Symbol( VC_MEM_ACCESS_HANDLE_T handle,
                                 const char *symbol,
                                 VC_MEM_ADDR_T *vcMemAddr );

/*
 * Reads 'numBytes' from the videocore memory starting at 'vcMemAddr'. The
 * results are stored in 'buf'.
 *
 * Returns true if the read was successful.
 */
int ReadVideoCoreMemory( VC_MEM_ACCESS_HANDLE_T handle,
                         void *buf,
                         VC_MEM_ADDR_T vcMemAddr,
                         size_t numBytes );

/*
 * Reads an unsigned 32-bit value from videocore memory.
 */
VCOS_STATIC_INLINE int ReadVideoCoreUInt32( VC_MEM_ACCESS_HANDLE_T handle,
                                       uint32_t *val,
                                       VC_MEM_ADDR_T vcMemAddr )
{
    return ReadVideoCoreMemory( handle, val, vcMemAddr, sizeof( val ));
}

/*
 * Reads a block of memory using the address associated with a symbol.
 */
int ReadVideoCoreMemoryBySymbol( VC_MEM_ACCESS_HANDLE_T vcHandle,
                                 const char            *symbol,
                                 void                  *buf,
                                 size_t                 numBytes );
/*
 * Reads an unsigned 32-bit value from videocore memory.
 */
VCOS_STATIC_INLINE int ReadVideoCoreUInt32BySymbol( VC_MEM_ACCESS_HANDLE_T handle,
                                               const char *symbol,
                                               uint32_t *val )
{
    return ReadVideoCoreMemoryBySymbol( handle, symbol, val, sizeof( val ));
}

/*
 * Looksup a string symbol by name, and reads the contents into a user
 * supplied buffer.
 */
int ReadVideoCoreStringBySymbol( VC_MEM_ACCESS_HANDLE_T handle,
                                 const char *symbol,
                                 char *buf,
                                 size_t bufSize );

/*
 * Writes 'numBytes' into the videocore memory starting at 'vcMemAddr'. The
 * data is taken from 'buf'.
 *
 * Returns true if the write was successful.
 */
int WriteVideoCoreMemory( VC_MEM_ACCESS_HANDLE_T handle,
                          void *buf,
                          VC_MEM_ADDR_T vcMemAddr,
                          size_t numBytes );

/*
 * Writes an unsigned 32-bit value into videocore memory.
 */
VCOS_STATIC_INLINE int WriteVideoCoreUInt32( VC_MEM_ACCESS_HANDLE_T handle,
                                        uint32_t val,
                                        VC_MEM_ADDR_T vcMemAddr )
{
    return WriteVideoCoreMemory( handle, &val, vcMemAddr, sizeof( val ));
}

/*
 * Closes the memory space opened previously via OpenVideoCoreMemory.
 */
void CloseVideoCoreMemory( VC_MEM_ACCESS_HANDLE_T handle );

/*
 * Returns the size of the videocore memory space.
 */
size_t GetVideoCoreMemorySize( VC_MEM_ACCESS_HANDLE_T handle );

#endif /* DEBUG_SYM_H */

