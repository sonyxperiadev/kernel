/*=============================================================================
Copyright (c) 2010 Broadcom Europe Ltd. All rights reserved.

Project  :  ARM
Module   :  vchiq_arm

FILE DESCRIPTION:
Header file for core VCHIQ logic.

==============================================================================*/

#if !defined( IPC_SHARED_MEM_H )
#define IPC_SHARED_MEM_H

/****************************************************************************
*
*   Shared memory offsets
* 
*   On big island, we have 160K of shared memory which is used for
*   communicating between the host and the videocore.
* 
*   These constants are used as offsets into that area and as such, need
*   to go into some shared header file.
* 
*   The places which use these constants:
* 
*   vc4/src/interface/vchiq_arm_bi/big_island/vchiq_bi_vc.c
*   vc4/src/interface/vchiq_arm_bi/vchiq_memdrv.c
*   vc4/src/vcfw/platform/broadcom/bcm911160-tablet/platform.c (for gpio)
*   vc4/src/vcfw/drivers/chip/vciv/2708/clock_bigisland.c
* 
*   The shared memory is 160K or 0x28000
* 
***************************************************************************/

/*
 * The PM stuff uses the low 32K of SRAM. 
 *  
 * Any changes to IPC_SHARED_MEM_VC_OFFSET must also be made to 
 * BCMHANA_ARAM_VC_OFFSET in linux/target/kernel/added/arch/arm/mach-bcmhana/include/mach/aram_layout.h 
 */


#define IPC_SHARED_MEM_VC_OFFSET            0x08000 /* 32K used for PM suspend/resume */

/*
 * The following offsets are relative to SRAM-BASE + IPC_SHARED_MEM_VC_OFFSET
 */

#define IPC_SHARED_MEM_CHANNEL_VC_OFFSET    ( 0 )
#define IPC_SHARED_MEM_CHANNEL_VC_SIZE      0x8000 /* 32K */

#define IPC_SHARED_MEM_CHANNEL_ARM_OFFSET   ( IPC_SHARED_MEM_CHANNEL_VC_OFFSET + IPC_SHARED_MEM_CHANNEL_VC_SIZE )
#define IPC_SHARED_MEM_CHANNEL_ARM_SIZE     0x8000 /* 32K */ 

#define IPC_SHARED_MEM_FRAGMENTS_OFFSET     ( IPC_SHARED_MEM_CHANNEL_ARM_OFFSET + IPC_SHARED_MEM_CHANNEL_ARM_SIZE )
#define IPC_SHARED_MEM_FRAGMENTS_SIZE       0x01000 /* 4K */

#define IPC_SHARED_MEM_CLOCK_DEBUG_OFFSET   ( 0x26000 - IPC_SHARED_MEM_VC_OFFSET ) /* 4 bytes */
#define IPC_SHARED_MEM_GPIO_WRITE_OFFSET    ( 0x26200 - IPC_SHARED_MEM_VC_OFFSET ) /* 4 bytes */
#define IPC_SHARED_MEM_GPIO_READ_OFFSET     ( 0x26204 - IPC_SHARED_MEM_VC_OFFSET ) /* 4 bytes */

#endif /* IPC_SHARED_MEM_H */
