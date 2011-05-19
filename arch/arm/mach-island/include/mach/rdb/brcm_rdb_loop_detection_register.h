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
/*     Date     : Generated on 4/25/2011 11:6:8                                             */
/*     RDB file : /projects/BIGISLAND/revA0                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_LOOP_DETECTION_REGISTER_H__
#define __BRCM_RDB_LOOP_DETECTION_REGISTER_H__

#define LOOP_DETECTION_REGISTER_LOOP_DETECTION_CONTROL_REGISTER_OFFSET    0x00000000
#define LOOP_DETECTION_REGISTER_LOOP_DETECTION_CONTROL_REGISTER_TYPE      UInt16
#define LOOP_DETECTION_REGISTER_LOOP_DETECTION_CONTROL_REGISTER_RESERVED_MASK 0x0000C000
#define    LOOP_DETECTION_REGISTER_LOOP_DETECTION_CONTROL_REGISTER_EN_TXPASS_SHIFT 13
#define    LOOP_DETECTION_REGISTER_LOOP_DETECTION_CONTROL_REGISTER_EN_TXPASS_MASK 0x00002000
#define    LOOP_DETECTION_REGISTER_LOOP_DETECTION_CONTROL_REGISTER_EN_LOOP_DETECT_SHIFT 12
#define    LOOP_DETECTION_REGISTER_LOOP_DETECTION_CONTROL_REGISTER_EN_LOOP_DETECT_MASK 0x00001000
#define    LOOP_DETECTION_REGISTER_LOOP_DETECTION_CONTROL_REGISTER_LOOP_IMP_SEL_SHIFT 11
#define    LOOP_DETECTION_REGISTER_LOOP_DETECTION_CONTROL_REGISTER_LOOP_IMP_SEL_MASK 0x00000800
#define    LOOP_DETECTION_REGISTER_LOOP_DETECTION_CONTROL_REGISTER_LED_RESET_TIMER_CNTL_SHIFT 3
#define    LOOP_DETECTION_REGISTER_LOOP_DETECTION_CONTROL_REGISTER_LED_RESET_TIMER_CNTL_MASK 0x000007F8
#define    LOOP_DETECTION_REGISTER_LOOP_DETECTION_CONTROL_REGISTER_OV_PAUSE_ON_SHIFT 2
#define    LOOP_DETECTION_REGISTER_LOOP_DETECTION_CONTROL_REGISTER_OV_PAUSE_ON_MASK 0x00000004
#define    LOOP_DETECTION_REGISTER_LOOP_DETECTION_CONTROL_REGISTER_DISCOVERY_FRAME_QUEUE_SEL_SHIFT 0
#define    LOOP_DETECTION_REGISTER_LOOP_DETECTION_CONTROL_REGISTER_DISCOVERY_FRAME_QUEUE_SEL_MASK 0x00000003

#define LOOP_DETECTION_REGISTER_DISCOVERY_FRAME_TIMER_REGISTERS_OFFSET    0x00000010
#define LOOP_DETECTION_REGISTER_DISCOVERY_FRAME_TIMER_REGISTERS_TYPE      UInt8
#define LOOP_DETECTION_REGISTER_DISCOVERY_FRAME_TIMER_REGISTERS_RESERVED_MASK 0x000000F0
#define    LOOP_DETECTION_REGISTER_DISCOVERY_FRAME_TIMER_REGISTERS_DISCOVERY_FRAME_TIMER_SHIFT 0
#define    LOOP_DETECTION_REGISTER_DISCOVERY_FRAME_TIMER_REGISTERS_DISCOVERY_FRAME_TIMER_MASK 0x0000000F

#define LOOP_DETECTION_REGISTER_LED_WARNING_PORTMAP_REGISTERS_OFFSET      0x00000018
#define LOOP_DETECTION_REGISTER_LED_WARNING_PORTMAP_REGISTERS_TYPE        UInt16
#define LOOP_DETECTION_REGISTER_LED_WARNING_PORTMAP_REGISTERS_RESERVED_MASK 0x0000FE00
#define    LOOP_DETECTION_REGISTER_LED_WARNING_PORTMAP_REGISTERS_LED_WARNING_PORTMAP_SHIFT 0
#define    LOOP_DETECTION_REGISTER_LED_WARNING_PORTMAP_REGISTERS_LED_WARNING_PORTMAP_MASK 0x000001FF

#define LOOP_DETECTION_REGISTER_MODULE_ID_0_REGISTERS_OFFSET              0x00000028
#define LOOP_DETECTION_REGISTER_MODULE_ID_0_REGISTERS_TYPE                UInt64
#define LOOP_DETECTION_REGISTER_MODULE_ID_0_REGISTERS_RESERVED_MASK       0xFFFF000000000000
#define    LOOP_DETECTION_REGISTER_MODULE_ID_0_REGISTERS_MODULE_ID_SA_SHIFT 0
#define    LOOP_DETECTION_REGISTER_MODULE_ID_0_REGISTERS_MODULE_ID_SA_MASK 0xFFFFFFFFFFFF

#define LOOP_DETECTION_REGISTER_MODULE_ID_1_REGISTERS_OFFSET              0x00000058
#define LOOP_DETECTION_REGISTER_MODULE_ID_1_REGISTERS_TYPE                UInt64
#define LOOP_DETECTION_REGISTER_MODULE_ID_1_REGISTERS_RESERVED_MASK       0xFFFF7F0000000000
#define    LOOP_DETECTION_REGISTER_MODULE_ID_1_REGISTERS_MOUDLE_ID_AVAIL_SHIFT 47
#define    LOOP_DETECTION_REGISTER_MODULE_ID_1_REGISTERS_MOUDLE_ID_AVAIL_MASK 0x800000000000
#define    LOOP_DETECTION_REGISTER_MODULE_ID_1_REGISTERS_MODULE_ID_PORTNUM_SHIFT 32
#define    LOOP_DETECTION_REGISTER_MODULE_ID_1_REGISTERS_MODULE_ID_PORTNUM_MASK 0xFF00000000
#define    LOOP_DETECTION_REGISTER_MODULE_ID_1_REGISTERS_MODULE_ID_CRC_SHIFT 0
#define    LOOP_DETECTION_REGISTER_MODULE_ID_1_REGISTERS_MODULE_ID_CRC_MASK 0xFFFFFFFF

#define LOOP_DETECTION_REGISTER_LOOP_DETECT_FRAME_SA_REGISTER_OFFSET      0x00000088
#define LOOP_DETECTION_REGISTER_LOOP_DETECT_FRAME_SA_REGISTER_TYPE        UInt64
#define LOOP_DETECTION_REGISTER_LOOP_DETECT_FRAME_SA_REGISTER_RESERVED_MASK 0xFFFF000000000000
#define    LOOP_DETECTION_REGISTER_LOOP_DETECT_FRAME_SA_REGISTER_LD_SA_SHIFT 0
#define    LOOP_DETECTION_REGISTER_LOOP_DETECT_FRAME_SA_REGISTER_LD_SA_MASK 0xFFFFFFFFFFFF

#endif /* __BRCM_RDB_LOOP_DETECTION_REGISTER_H__ */


