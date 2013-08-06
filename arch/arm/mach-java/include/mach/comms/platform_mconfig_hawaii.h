/***************************************************************************
*
* (c)1999-2011 Broadcom Corporation
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to
* you under the terms of the GNU General Public License version 2,
* available at http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
***************************************************************************/
#ifndef _PLATFORM_MCONFIG_HAWAII_H_
#define _PLATFORM_MCONFIG_HAWAII_H_

/*****************************************************************************/
/*      name                   ROM address     RAM address     size          */
/*      ================       ==========      ==========      ==========    */
#if defined (RAM_BASE_ADDRESS)
#define EXT_RAM_BASE_ADDR                      RAM_BASE_ADDRESS
#else
#define EXT_RAM_BASE_ADDR                      0x80000000
#endif
#if defined(RAM_SIZE)
#define EXT_RAM_SIZE                                           RAM_SIZE
#else
#define EXT_RAM_SIZE                                           0x08000000
#endif

#define EXT_ROM_BASE_ADDR      0x00000000

#if defined(ROM_SIZE)
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
#define LOADER_RAM_OFFSET                      (CP_SH_SIZE + AP_SH_SIZE)
#define LOADER_RAM_ADDR                        (EXT_RAM_BASE_ADDR + LOADER_RAM_OFFSET)
#define LOADER_SIZE                                            0x00021000

#define CP_BOOT_ROM_ADDR       0x001F0000
#define CP_BOOT_SIZE                                           0x00008000

/* reserved for customer certificates */
#define CUSTOM_CERT_ROM_ADDR   0x00040000
#define CUSTOM_CERT_SIZE                                       0x00020000

#define PARM_IND_ROM_ADDR      0x00080000
#define PARM_DEP_ROM_ADDR      0x00100000

/*** The dep sysparm RAM defines are shared with Linux build; they are defined in platform_mconfig_shared.h *** 
#define PARM_DEP_RAM_OFFSET                    0x180000
#define PARM_DEP_RAM_ADDR                      (EXT_RAM_BASE_ADDR + PARM_DEP_RAM_OFFSET)
#define PARM_DEP_SIZE                                          0x00010000
***/
   

#define PARM_SPML_IND_ROM_ADDR 0x00110000
#define PARM_SPML_DEP_ROM_ADDR 0x00150000

#define DSP_PRAM_ROM_ADDR      0x01F20000

#define UMTS_CAL_ROM_ADDR      0x001D0000

#define DT_BLOB_ROM_ADDR          0x2300000
#define DT_BLOB_SIZE              0x2000

#define DSP_DRAM_ROM_ADDR      0x02000000

#define TL3_DSP_EXT_BASE                       DSP_DRAM_RAM_ADDR
#define TL3_DSP_EXT_SIZE                                       DSP_DRAM_SIZE

#define AP_RO_ROM_ADDR         0x02200000
#define AP_RO_RAM_OFFSET                       0x2e000000
#define AP_RO_RAM_ADDR                         (EXT_RAM_BASE_ADDR + AP_RO_RAM_OFFSET)

#define CP_RO_ROM_ADDR         0x00200000

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

#define AP_MMU_L1L2_OFFSET                     0x2140000
#define AP_MMU_L1L2_ADDR                       (EXT_RAM_BASE_ADDR + AP_MMU_L1L2_OFFSET)

#define MMU_L1L2_SIZE                                          0x00010000

#define DORMANT_DATA_BASE                             (AP_SH_BASE+AP_SH_SIZE)
#define DORMANT_DATA_SIZE                                             0x00001000    /* 4k reserved to save dormant data */


#ifndef FUSE_AP_BSP
#ifdef VMF_INCLUDE_NEW
#define VMF_PAGE1_OFFSET                       0x1A00000
#define VMF_PAGE2_OFFSET                       0x1B00000
#define VMF_PAGE1_BASE                         (EXT_RAM_BASE_ADDR + VMF_PAGE1_OFFSET)
#define VMF_PAGE2_BASE                         (EXT_RAM_BASE_ADDR + VMF_PAGE2_OFFSET)
#define VMF_PAGE_SIZE                                          0x00100000      /* Increasing size to 1MB for each instance for future use in Phase-3 */
#define VMF_TOTAL_SIZE                                         2 * VMF_PAGE_SIZE
#endif
#endif

#define PROTECTED_SIZE                                         0x00200000

#define EXCEPTSTACK_SIZE                                       0x00004000

#define ER_UNCACHE_OFFSET                       0x1C00000
#define ER_UNCACHE_BASE                       (EXT_RAM_BASE_ADDR + ER_UNCACHE_OFFSET)

/* MM data region size and 1M alignment */
#define MM_DATA_ALIGN                                          0x00100000
#define MM_DATA_SIZE                                           0x02200000

/* 4K alignment on each region for MMU configuration */
#define MMU_ALIGN_4K                                           0x00001000


/* LOAD_ROM1 region base address and maximum size. */
#define LOAD_ROM1_BASE                         EXT_RAM_BASE_ADDR
#define LOAD_ROM1_SIZE_MAX                                     0x00300000

/* LOAD_ROM2 region base address and maximum size. */
#define AP_LOAD_ROM2_BASE                      AP_RO_RAM_ADDR
#define CP_LOAD_ROM2_BASE                      CP_RO_RAM_ADDR
#define AP_LOAD_ROM2_SIZE_MAX                                  0x00FC0000
#define CP_LOAD_ROM2_SIZE_MAX                                  (DSP_DRAM_RAM_ADDR - CP_LOAD_ROM2_BASE)


#if defined(_CAPRI_) || defined(_HAWAII_)
#define	DSP_CP_SCRATCHMEM_BASE					0x34050000
#define DSP_CP_SCRATCHMEM_SIZE					0x00004000
#elif defined (_RHEA_)
#define	DSP_CP_SCRATCHMEM_BASE					0x3404A000
#define DSP_CP_SCRATCHMEM_SIZE					0x00001000
#endif

#include "platform_mconfig_shared.h"

#endif //_PLATFORM_MCONFIG_HAWAII_H_

