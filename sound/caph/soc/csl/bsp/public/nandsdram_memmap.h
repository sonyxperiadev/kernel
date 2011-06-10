/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

/*****************************************************************************
*
* Do NOT define physical ROM/RAM address in this file
* All the definitions should be defined in platform_mconfig.h.
*
*****************************************************************************/

// !< --- PreProcess --- >!
#define LOG_MVA_BASE        0x0A000000                  // 4M logical memory of protected MVA
#define LOG_MVA_SIZE                        0x00300000  // 3MB, down from 4MB to reduce the size of MMU tables

#if defined (_ATHENA_)
#define ITCM_MVA_BASE       0x00000000
// size for a virtual ITCM page: 1 MMU section = 1MB
#if defined(DISABLE_PAGED_ITCM)
#define ITCM_PAGE1_MVA      (ITCM_MVA_BASE+ITCM_PAGE_SIZE)
#else
#define ITCM_VPAGE_SIZE     (1024*1024)
#define ITCM_PAGE1_MVA      (ITCM_MVA_BASE+1*ITCM_VPAGE_SIZE+ITCM_PAGE_SIZE)
#define ITCM_PAGE2_MVA      (ITCM_MVA_BASE+2*ITCM_VPAGE_SIZE+ITCM_PAGE_SIZE)
#define ITCM_PAGE3_MVA      (ITCM_MVA_BASE+3*ITCM_VPAGE_SIZE+ITCM_PAGE_SIZE)
#endif

#ifdef POD_ENABLED
    //
    //    Define starting virtual address for demand-paged (POD) code.  The POD
    //    virtual addresses will be mapped to physical address by the POD 
    //    infrastructure -- the virtual addresses should not conflict with
    //    any physical addresses.  The starting virtual address must be a multiple
    //    of 1 Mbyte.  
    //
    #ifdef FUSE_APPS_PROCESSOR
        #define AP_POD_VM_BASE                 0xa0000000        // multiple of 1 Mbyte
    #else // FUSE_APPS_PROCESSOR
        #define MSP_POD_VM_BASE                0xa0000000        // multiple of 1 Mbyte
    #endif // FUSE_APPS_PROCESSOR
#endif // POD_ENABLED
#endif // #if defined (_ATHENA_)

#define STACK_GUARD_BASE          0xF0000000                     // Hyperspace
#define STACK_GUARD_SIZE          1024

#ifndef FUSE_AP_BSP
#ifdef VMF_INCLUDE_NEW
#define VMF_PAGE_BASE       0xA0000000
#endif
#endif

// !< --- PreProcess --- >!

// for compatibility with existing code
#define SHARED_RAM_BASE     DSP_SH_BASE
#define PARM_IND_BASE       PARM_IND_RAM_ADDR
#define PARM_DEP_BASE       PARM_DEP_RAM_ADDR
#define PARM_SPML_IND_BASE  PARM_SPML_IND_RAM_ADDR               // 64k hybrid SPML
#define PARM_SPML_DEP_BASE  PARM_SPML_DEP_RAM_ADDR               // 64k hybrid SPML (Address is verified on HERA... need to check for Athena)
#define UMTS_CAL_BASE       UMTS_CAL_RAM_ADDR                    // Spinner CAL
#define DSP_PRAM_BASE       DSP_PRAM_RAM_ADDR                    // 7pages*64k DSP PRAM load;;For TL3:256K for TL3 PRAM, 256K for TL3 DRAM
#define DSP_DRAM_BASE       DSP_DRAM_RAM_ADDR     

#if defined (DSP_DROM_RAM_ADDR)
#define DSP_DROM_BASE       DSP_DROM_RAM_ADDR
#endif

#ifndef FUSE_APPS_PROCESSOR
#if defined (CP_MMU_L1L2_ADDR)
#define MMU_L1L2_BASE             CP_MMU_L1L2_ADDR               // 128k MMU L1/L2 tables
#endif
#define RO_BASE                   CP_RO_RAM_ADDR                 // RO starts here 
#else
#define RO_BASE                   AP_RO_RAM_ADDR                 // RO starts here 
#define MMU_L1L2_BASE             AP_MMU_L1L2_ADDR               // 128k MMU L1/L2 tables
#define CP_RO_BASE                CP_RO_RAM_ADDR                 // RO starts here 
#endif /* FUSE_APPS_PROCESSOR */

#define DUMMY_BASE_ADDR           0x00000000

#define GAP_BEFORE_RW                              MMU_ALIGN_4K    
#define RW_BASE                   DUMMY_BASE_ADDR                // RW starts after RO, 4k padding

#define GAP_BEFORE_ZI                              MMU_ALIGN_4K    
#define ZI_BASE                   DUMMY_BASE_ADDR                // ZI starts after RW, 4k padding

#define GAP_BEFORE_NOINIT                          MMU_ALIGN_4K    
#define NOINIT_BASE               DUMMY_BASE_ADDR                // NOINIT starts after ZI, 4k padding

#define GAP_BEFORE_UNCACHE                         MMU_ALIGN_4K    
#define UNCACHE_BASE              DUMMY_BASE_ADDR                // UNCACHE starts after NOINIT, 4k padding

#define GAP_BEFORE_PROTECTED                       MMU_ALIGN_4K
#define PROTECTED_BASE            DUMMY_BASE_ADDR                // PROTECTED starts after UNCACHE

#define GAP_BEFORE_EXCEPTSTACK                     MMU_ALIGN_4K    
#define EXCEPTSTACK_BASE          DUMMY_BASE_ADDR

#define GAP_BEFORE_MMDATA                          MMU_ALIGN_4K
#define MM_DATA_BASE              DUMMY_BASE_ADDR

