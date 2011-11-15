/*****************************************************************************
*
*    (c) 2007-2010 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or
* its licensors, and may only be used, duplicated, modified or distributed
* pursuant to the terms and conditions of a separate, written license
* agreement executed between you and Broadcom (an "Authorized License").
* Except as set forth in an Authorized License, Broadcom grants no license
* (express or implied), right to use, or waiver of any kind with respect to
* the Software, and Broadcom expressly reserves all rights in and to the
* Software and all intellectual property rights therein.
* IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS
* SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
* ALL USE OF THE SOFTWARE.  
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
*    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
*    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
*    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
*****************************************************************************/
#ifndef _PLATFORM_MCONFIG_RHEA_H_
#define _PLATFORM_MCONFIG_RHEA_H_
// !< --- PreProcess --- >!

#ifdef __cplusplus
extern "C" {
#endif


#ifdef BRCM_RTOS
/*****************************************************************************/
/*                                                                           */
/*    RHEA MEMERY MAP     (RTOS SPECIFIC)                                    */
/*                                                                           */
/*****************************************************************************/
//      name                   ROM address     RAM address     size
//      ================       ==========      ==========      ==========
#if (defined (CNEON_COMMON) || defined (CNEON_MODEM) || defined (CNEON_LMP))
#define EXT_RAM_BASE_ADDR                      0xA0000000
#elif defined (RAM_BASE_ADDRESS)
#define EXT_RAM_BASE_ADDR                      RAM_BASE_ADDRESS
#else
#define EXT_RAM_BASE_ADDR                      0x80000000
#endif
#if defined (RAM_SIZE)
#define EXT_RAM_SIZE                                           RAM_SIZE
#else
#define EXT_RAM_SIZE                                           0x08000000
#endif

#define EXT_ROM_BASE_ADDR      0x00000000

#if defined (ROM_SIZE)
#define EXT_ROM_SIZE                                           ROM_SIZE
#else
#define EXT_ROM_SIZE                                           0x08000000
#endif

#define ABI_ROM_ADDR           EXT_ROM_BASE_ADDR
#define ABI_SIZE                                               0x00008000

#define ABIEXT_ROM_ADDR        0x00008000
#define ABIEXT_SIZE                                            0x00008000

#define HWCONF_ROM_ADDR        0x00010000
#define HWCONF_SIZE                                            0x00008000

#define BOOTPARM_ROM_ADDR      0x00050000
#define BOOTPARM_SIZE                                          0x00001000

#define LOADER_ROM_ADDR        0x00020000
#define LOADER_RAM_ADDR                        EXT_RAM_BASE_ADDR    
#define LOADER_SIZE                            0x00021000

#define CP_BOOT_ROM_ADDR       0x001F0000
#define CP_BOOT_SIZE                                           0x00008000

/* reserved for customer certificates */
#define CUSTOM_CERT_ROM_ADDR   0x00040000
#define CUSTOM_CERT_SIZE                                       0x00020000

#ifndef ISLANDRAY
#define PARM_IND_ROM_ADDR      0x00080000
#else
#define PARM_IND_ROM_ADDR      0x00300000
#endif
#if (defined (CNEON_COMMON) || defined (CNEON_MODEM))
#define PARM_IND_RAM_OFFSET                    0xC0000
#else
#define PARM_IND_RAM_OFFSET                    0x140000
#endif
#define PARM_IND_RAM_ADDR                      (EXT_RAM_BASE_ADDR + PARM_IND_RAM_OFFSET)
#define PARM_IND_SIZE                                          0x00040000

#ifndef ISLANDRAY
#define PARM_DEP_ROM_ADDR      0x000C0000
#else
#define PARM_DEP_ROM_ADDR      0x00380000
#endif
   
#define PARM_SPML_IND_ROM_ADDR 0x000D0000
#if (defined (CNEON_COMMON) || defined (CNEON_MODEM))
#define PARM_SPML_IND_RAM_OFFSET               0x170000 
#else
#define PARM_SPML_IND_RAM_OFFSET               0x1B0000 
#endif
#define PARM_SPML_IND_RAM_ADDR                 (EXT_RAM_BASE_ADDR + PARM_SPML_IND_RAM_OFFSET)
#define PARM_SPML_IND_SIZE                                     0x00040000

#define PARM_SPML_DEP_ROM_ADDR 0x00110000
#if (defined (CNEON_COMMON) || defined (CNEON_MODEM))
#define PARM_SPML_DEP_RAM_OFFSET               0x1B0000 
#else
#define PARM_SPML_DEP_RAM_OFFSET               0x1F0000 
#endif
#define PARM_SPML_DEP_RAM_ADDR                 (EXT_RAM_BASE_ADDR + PARM_SPML_DEP_RAM_OFFSET)
#define PARM_SPML_DEP_SIZE                                     0x00010000   

#ifndef ISLANDRAY   
#define DSP_PRAM_ROM_ADDR      0x01F20000
#else
#define DSP_PRAM_ROM_ADDR      0x00400000
#endif

#if (defined (CNEON_COMMON) || defined (CNEON_MODEM))
#define DSP_PRAM_RAM_OFFSET                    0x1E0000
#else
#define DSP_PRAM_RAM_OFFSET                    0x1A0000
#endif
#define DSP_PRAM_RAM_ADDR                      (EXT_RAM_BASE_ADDR + DSP_PRAM_RAM_OFFSET)
//#ifndef ISLANDRAY 
#define DSP_PRAM_SIZE                                          0x00004000
//#else
//#define DSP_PRAM_SIZE                                          0x000a0000
//#endif
   
#if (defined (CNEON_COMMON) || defined (CNEON_MODEM))
#define MODEMHOSTSEC_HOST_BUFF 0xA01E4000
#define MODEMHOSTSEC_HOST_BUFF_SIZE                            0x00000400

#define MODEMHOSTSEC_MODEM_BUFF  0xA01E4400
#define MODEMHOSTSEC_MODEM_BUFF_SIZE                           0x00000400

#define MODEMHOSTSEC_BUFF_BASE MODEMHOSTSEC_HOST_BUFF
#define MODEMHOSTSEC_BUFF_SIZE                                 (MODEMHOSTSEC_HOST_BUFF_SIZE + MODEMHOSTSEC_MODEM_BUFF_SIZE)
#endif   

#ifndef ISLANDRAY
#define UMTS_CAL_ROM_ADDR      0x001D0000
#else
#define UMTS_CAL_ROM_ADDR      0x00280000
#endif
#if (defined (CNEON_COMMON) || defined (CNEON_MODEM))
#define UMTS_CAL_RAM_OFFSET                    0x1C0000
#else
#define UMTS_CAL_RAM_OFFSET                    0x110000
#endif   
#define UMTS_CAL_RAM_ADDR                      (EXT_RAM_BASE_ADDR + UMTS_CAL_RAM_OFFSET)
#define UMTS_CAL_SIZE                                          0x00020000

#define DT_BLOB_ROM_ADDR          0x2300000
#define DT_BLOB_SIZE              0x2000

#ifndef ISLANDRAY   
#define DSP_DRAM_ROM_ADDR      0x02000000
#else
#define DSP_DRAM_ROM_ADDR      0x00800000
#endif

#if (defined (CNEON_COMMON) || defined (CNEON_MODEM))
#define DSP_DRAM_RAM_OFFSET                    0x1A00000     
#else
#define DSP_DRAM_RAM_OFFSET                    0x1800000     
#endif   
#define DSP_DRAM_RAM_ADDR                      (EXT_RAM_BASE_ADDR + DSP_DRAM_RAM_OFFSET)
//#ifndef ISLANDRAY 
#define DSP_DRAM_SIZE                                          0x00200000
//#else
//#define DSP_DRAM_SIZE                                          0x00400000
//#endif

#define TL3_DSP_EXT_BASE                       DSP_DRAM_RAM_ADDR
#define TL3_DSP_EXT_SIZE                                       DSP_DRAM_SIZE

#define AP_RO_ROM_ADDR         0x02200000
#define AP_RO_RAM_OFFSET                       0x2300000
#define AP_RO_RAM_ADDR                         (EXT_RAM_BASE_ADDR + AP_RO_RAM_OFFSET)

#define CP_RO_ROM_ADDR         0x00200000
#define CP_RO_RAM_OFFSET                       0x200000
#define CP_RO_RAM_ADDR                         (EXT_RAM_BASE_ADDR + CP_RO_RAM_OFFSET)
 
#define FFS_ROM_ADDR           0x03800000
#define FFS_SIZE                                               0x03000000

#define AP_POD_ROM_ADDR        0x02E00000
#define AP_POD_SIZE                                            0x00300000

#define CP_POD_ROM_ADDR        0x00F00000
#define CP_POD_SIZE                                            0x00100000

#define CONTAINER_ROM_ADDR     0x01000000

#define APPS_IMAGE_TST_ADDR    0x06000000


/*****************************************************************************/
/*   scatter loading definition                                              */
/*****************************************************************************/
#if (defined (CNEON_COMMON) || defined (CNEON_MODEM))
#define AP_MMU_L1L2_OFFSET                     0x2290000
#else
#define AP_MMU_L1L2_OFFSET                     0x2140000
#endif   
#define AP_MMU_L1L2_ADDR                       (EXT_RAM_BASE_ADDR + AP_MMU_L1L2_OFFSET)

#define MMU_L1L2_SIZE                                          0x00010000



#define DORMANT_DATA_BASE                             (AP_SH_BASE+AP_SH_SIZE)    
#define DORMANT_DATA_SIZE                                             0x00001000    // 4k reserved to save dormant data


#ifndef FUSE_AP_BSP
#ifdef VMF_INCLUDE_NEW
#if (defined (CNEON_COMMON) || defined (CNEON_MODEM))
#define VMF_PAGE1_OFFSET                       0x1600000
#define VMF_PAGE2_OFFSET                       0x1700000
#else
#define VMF_PAGE1_OFFSET                       0x1A00000
#define VMF_PAGE2_OFFSET                       0x1B00000
#endif   

#define VMF_PAGE1_BASE                         (EXT_RAM_BASE_ADDR + VMF_PAGE1_OFFSET)
#define VMF_PAGE2_BASE                         (EXT_RAM_BASE_ADDR + VMF_PAGE2_OFFSET)

#define VMF_PAGE_SIZE                                          0x00100000      // Increasing size to 1MB for each instance for future use in Phase-3
#define VMF_TOTAL_SIZE                                         2 * VMF_PAGE_SIZE
#endif
#endif

#define PROTECTED_SIZE                                         0x00200000

#define EXCEPTSTACK_SIZE                                       0x00004000

// MM data region size and 1M alignment
#define MM_DATA_ALIGN                                          0x00100000
#if (defined (CNEON_COMMON) || defined (CNEON_MODEM))
#define MM_DATA_SIZE                                           0x01000000
#else
#define MM_DATA_SIZE                                           0x02200000
#endif

//4K alignment on each region for MMU configuration
#define MMU_ALIGN_4K                                           0x00001000

// LOAD_ROM1 region base address and maximum size.
#define LOAD_ROM1_BASE                         EXT_RAM_BASE_ADDR
#define LOAD_ROM1_SIZE_MAX                                     0x00300000

// LOAD_ROM2 region base address and maximum size.
#define AP_LOAD_ROM2_BASE                      AP_RO_RAM_ADDR
#define CP_LOAD_ROM2_BASE                      CP_RO_RAM_ADDR
#if (defined (CNEON_COMMON) || defined (CNEON_MODEM))
#define AP_LOAD_ROM2_SIZE_MAX                                  0x01D00000
#else
#define AP_LOAD_ROM2_SIZE_MAX                                  0x00FC0000
#endif
#if (defined (CNEON_COMMON) || defined (CNEON_MODEM))
#define CP_LOAD_ROM2_SIZE_MAX                                  (DSP_DRAM_RAM_ADDR - CP_LOAD_ROM2_BASE - 0x300000)
#else
#define CP_LOAD_ROM2_SIZE_MAX                                  (DSP_DRAM_RAM_ADDR - CP_LOAD_ROM2_BASE)
#endif

#else  // BRCM_RTOS
/*****************************************************************************/
/*                                                                           */
/*    RHEA MEMERY MAP     (NON-RTOS SPECIFIC, ex. Linux)                     */
/*                                                                           */
/*****************************************************************************/

// Memory map for Linux build
#define EXT_RAM_BASE_ADDR  CONFIG_BCM_RAM_BASE

#endif

/*****************************************************************************/
/*                                                                           */
/*    RHEA MEMERY MAP     (COMMON to all platforms. Ex. LINUX, RTOS, WINDOWS)*/
/*                                                                           */
/*****************************************************************************/

// Shared memory defines; commonly used by RTOS and Linux builds

#define DSP_SH_BASE                            EXT_RAM_BASE_ADDR     // DSP sharedmem
#define DSP_SH_SIZE                                            0x00064000

#define CP_SH_BASE                             DSP_SH_BASE
#define CP_SH_SIZE                                             0x00020000    // 128k CP/DSP sharedmem

#define AP_SH_BASE                             (DSP_SH_BASE+CP_SH_SIZE)
#define AP_SH_SIZE                                             0x00020000    // 128k  AP/DSP sharedmem

#define	DSP_CP_SCRATCHMEM_BASE					0x3404A000
#define DSP_CP_SCRATCHMEM_SIZE					0x00000C00


#if (defined (CNEON_COMMON) || defined (CNEON_MODEM))
#define IPC_OFFSET                             0x1800000                    // 256kB IPC shared RAM
#else
#define IPC_OFFSET                             0x1E00000                    // 256kB IPC shared RAM
#endif

#define IPC_BASE                               (EXT_RAM_BASE_ADDR + IPC_OFFSET)
#define IPC_SIZE                                               0x00200000

#if (defined (CNEON_COMMON) || defined (CNEON_MODEM))
#define PARM_DEP_RAM_OFFSET                    0x160000
#else
#define PARM_DEP_RAM_OFFSET                    0x180000
#endif
#define PARM_DEP_RAM_ADDR                      (EXT_RAM_BASE_ADDR + PARM_DEP_RAM_OFFSET)
#ifndef ISLANDRAY
#define PARM_DEP_SIZE                                          0x00010000
#else
#define PARM_DEP_SIZE                                          0x00020000
#endif

#define MAIN_ADDRESS_OFFSET 0x30
#define INIT_ADDRESS_OFFSET 0x34
#define MODEM_ITCM_ADDRESS 0x3AC00000
#define MODEM_DTCM_ADDRESS 0x3AD00000
#define RESERVED_HEADER 0x400
#ifdef __cplusplus
}
#endif

// !< --- PreProcess --- >!

#endif //_PLATFORM_MCONFIG_RHEA_H_

