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

#ifndef __BRCM_RDB_EHCI_H__
#define __BRCM_RDB_EHCI_H__

#define EHCI_HCCAPBASE_OFFSET                                             0x00000000
#define EHCI_HCCAPBASE_TYPE                                               UInt32
#define EHCI_HCCAPBASE_RESERVED_MASK                                      0x0000FF00
#define    EHCI_HCCAPBASE_HCIVERSION_SHIFT                                16
#define    EHCI_HCCAPBASE_HCIVERSION_MASK                                 0xFFFF0000
#define    EHCI_HCCAPBASE_CAPLENGTH_SHIFT                                 0
#define    EHCI_HCCAPBASE_CAPLENGTH_MASK                                  0x000000FF

#define EHCI_HCSPARAMS_OFFSET                                             0x00000004
#define EHCI_HCSPARAMS_TYPE                                               UInt32
#define EHCI_HCSPARAMS_RESERVED_MASK                                      0xFF0E0060
#define    EHCI_HCSPARAMS_DEBUG_PORT_NUMBER_SHIFT                         20
#define    EHCI_HCSPARAMS_DEBUG_PORT_NUMBER_MASK                          0x00F00000
#define    EHCI_HCSPARAMS_P_INDICATOR_SHIFT                               16
#define    EHCI_HCSPARAMS_P_INDICATOR_MASK                                0x00010000
#define    EHCI_HCSPARAMS_N_CC_SHIFT                                      12
#define    EHCI_HCSPARAMS_N_CC_MASK                                       0x0000F000
#define    EHCI_HCSPARAMS_N_PCC_SHIFT                                     8
#define    EHCI_HCSPARAMS_N_PCC_MASK                                      0x00000F00
#define    EHCI_HCSPARAMS_PORT_ROUTING_RULES_SHIFT                        7
#define    EHCI_HCSPARAMS_PORT_ROUTING_RULES_MASK                         0x00000080
#define    EHCI_HCSPARAMS_PPC_SHIFT                                       4
#define    EHCI_HCSPARAMS_PPC_MASK                                        0x00000010
#define    EHCI_HCSPARAMS_N_PORTS_SHIFT                                   0
#define    EHCI_HCSPARAMS_N_PORTS_MASK                                    0x0000000F

#define EHCI_HCCPARAMS_OFFSET                                             0x00000008
#define EHCI_HCCPARAMS_TYPE                                               UInt32
#define EHCI_HCCPARAMS_RESERVED_MASK                                      0xFFFF0008
#define    EHCI_HCCPARAMS_EECP_SHIFT                                      8
#define    EHCI_HCCPARAMS_EECP_MASK                                       0x0000FF00
#define    EHCI_HCCPARAMS_ISOCHRONOUS_SCHEDULING_THRESHOLD_SHIFT          4
#define    EHCI_HCCPARAMS_ISOCHRONOUS_SCHEDULING_THRESHOLD_MASK           0x000000F0
#define    EHCI_HCCPARAMS_ASYNCHRONOUS_SCHEDULE_PARK_CAPABILITY_SHIFT     2
#define    EHCI_HCCPARAMS_ASYNCHRONOUS_SCHEDULE_PARK_CAPABILITY_MASK      0x00000004
#define    EHCI_HCCPARAMS_PROGRAMMABLE_FRAME_LIST_FLAG_SHIFT              1
#define    EHCI_HCCPARAMS_PROGRAMMABLE_FRAME_LIST_FLAG_MASK               0x00000002
#define    EHCI_HCCPARAMS_SIXTY_FOUR_BIT_ADDRESSING_CAPABILITY_SHIFT      0
#define    EHCI_HCCPARAMS_SIXTY_FOUR_BIT_ADDRESSING_CAPABILITY_MASK       0x00000001

#define EHCI_USBCMD_OFFSET                                                0x00000010
#define EHCI_USBCMD_TYPE                                                  UInt32
#define EHCI_USBCMD_RESERVED_MASK                                         0xFF00F400
#define    EHCI_USBCMD_INTERRUPT_THRESHOLD_CONTROL_SHIFT                  16
#define    EHCI_USBCMD_INTERRUPT_THRESHOLD_CONTROL_MASK                   0x00FF0000
#define    EHCI_USBCMD_ASYNCHRONOUS_SCHEDULE_PARK_MODE_ENABLE_SHIFT       11
#define    EHCI_USBCMD_ASYNCHRONOUS_SCHEDULE_PARK_MODE_ENABLE_MASK        0x00000800
#define    EHCI_USBCMD_ASYNCHRONOUS_SCHEDULE_PARK_MODE_COUNT_SHIFT        8
#define    EHCI_USBCMD_ASYNCHRONOUS_SCHEDULE_PARK_MODE_COUNT_MASK         0x00000300
#define    EHCI_USBCMD_LIGHT_HOST_CONTROLLER_RESET_SHIFT                  7
#define    EHCI_USBCMD_LIGHT_HOST_CONTROLLER_RESET_MASK                   0x00000080
#define    EHCI_USBCMD_INTERRUPT_ON_ASYNC_ADVANCE_DOORBELL_SHIFT          6
#define    EHCI_USBCMD_INTERRUPT_ON_ASYNC_ADVANCE_DOORBELL_MASK           0x00000040
#define    EHCI_USBCMD_ASYNCHRONOUS_SCHEDULE_ENABLE_SHIFT                 5
#define    EHCI_USBCMD_ASYNCHRONOUS_SCHEDULE_ENABLE_MASK                  0x00000020
#define    EHCI_USBCMD_PERIODIC_SCHEDULE_ENABLE_SHIFT                     4
#define    EHCI_USBCMD_PERIODIC_SCHEDULE_ENABLE_MASK                      0x00000010
#define    EHCI_USBCMD_FRAME_LIST_SIZE_SHIFT                              2
#define    EHCI_USBCMD_FRAME_LIST_SIZE_MASK                               0x0000000C
#define    EHCI_USBCMD_HCRESET_SHIFT                                      1
#define    EHCI_USBCMD_HCRESET_MASK                                       0x00000002
#define    EHCI_USBCMD_RUN_STOP_SHIFT                                     0
#define    EHCI_USBCMD_RUN_STOP_MASK                                      0x00000001

#define EHCI_USBSTS_OFFSET                                                0x00000014
#define EHCI_USBSTS_TYPE                                                  UInt32
#define EHCI_USBSTS_RESERVED_MASK                                         0xFFFF0FC0
#define    EHCI_USBSTS_ASYNCHRONOUS_SCHEDULE_STATUS_SHIFT                 15
#define    EHCI_USBSTS_ASYNCHRONOUS_SCHEDULE_STATUS_MASK                  0x00008000
#define    EHCI_USBSTS_PERIODIC_SCHEDULE_STATUS_SHIFT                     14
#define    EHCI_USBSTS_PERIODIC_SCHEDULE_STATUS_MASK                      0x00004000
#define    EHCI_USBSTS_RECLAMATION_SHIFT                                  13
#define    EHCI_USBSTS_RECLAMATION_MASK                                   0x00002000
#define    EHCI_USBSTS_HCHALTED_SHIFT                                     12
#define    EHCI_USBSTS_HCHALTED_MASK                                      0x00001000
#define    EHCI_USBSTS_INTERRUPT_ON_ASYNC_ADVANCE_SHIFT                   5
#define    EHCI_USBSTS_INTERRUPT_ON_ASYNC_ADVANCE_MASK                    0x00000020
#define    EHCI_USBSTS_HOST_SYSTEM_ERROR_SHIFT                            4
#define    EHCI_USBSTS_HOST_SYSTEM_ERROR_MASK                             0x00000010
#define    EHCI_USBSTS_FRAME_LIST_ROLLOVER_SHIFT                          3
#define    EHCI_USBSTS_FRAME_LIST_ROLLOVER_MASK                           0x00000008
#define    EHCI_USBSTS_PORT_CHANGE_DETECT_SHIFT                           2
#define    EHCI_USBSTS_PORT_CHANGE_DETECT_MASK                            0x00000004
#define    EHCI_USBSTS_USBERRINT_SHIFT                                    1
#define    EHCI_USBSTS_USBERRINT_MASK                                     0x00000002
#define    EHCI_USBSTS_USBINT_SHIFT                                       0
#define    EHCI_USBSTS_USBINT_MASK                                        0x00000001

#define EHCI_USBINTR_OFFSET                                               0x00000018
#define EHCI_USBINTR_TYPE                                                 UInt32
#define EHCI_USBINTR_RESERVED_MASK                                        0xFFFFFFC0
#define    EHCI_USBINTR_INTERRUPT_ON_ASYNC_ADVANCE_ENABLE_SHIFT           5
#define    EHCI_USBINTR_INTERRUPT_ON_ASYNC_ADVANCE_ENABLE_MASK            0x00000020
#define    EHCI_USBINTR_HOST_SYSTEM_ERROR_ENABLE_SHIFT                    4
#define    EHCI_USBINTR_HOST_SYSTEM_ERROR_ENABLE_MASK                     0x00000010
#define    EHCI_USBINTR_FRAME_LIST_ROLLOVER_ENABLE_SHIFT                  3
#define    EHCI_USBINTR_FRAME_LIST_ROLLOVER_ENABLE_MASK                   0x00000008
#define    EHCI_USBINTR_PORT_CHANGE_INTERRUPT_ENABLE_SHIFT                2
#define    EHCI_USBINTR_PORT_CHANGE_INTERRUPT_ENABLE_MASK                 0x00000004
#define    EHCI_USBINTR_USB_ERROR_INTERRUPT_ENABLE_SHIFT                  1
#define    EHCI_USBINTR_USB_ERROR_INTERRUPT_ENABLE_MASK                   0x00000002
#define    EHCI_USBINTR_USB_INTERRUPT_ENABLE_SHIFT                        0
#define    EHCI_USBINTR_USB_INTERRUPT_ENABLE_MASK                         0x00000001

#define EHCI_FRINDEX_OFFSET                                               0x0000001C
#define EHCI_FRINDEX_TYPE                                                 UInt32
#define EHCI_FRINDEX_RESERVED_MASK                                        0xFFFFC000
#define    EHCI_FRINDEX_FRAME_INDEX_SHIFT                                 0
#define    EHCI_FRINDEX_FRAME_INDEX_MASK                                  0x00003FFF

#define EHCI_PERIODICLISTBASE_OFFSET                                      0x00000024
#define EHCI_PERIODICLISTBASE_TYPE                                        UInt32
#define EHCI_PERIODICLISTBASE_RESERVED_MASK                               0x00000FFF
#define    EHCI_PERIODICLISTBASE_BASE_ADDRESS_LOW_SHIFT                   12
#define    EHCI_PERIODICLISTBASE_BASE_ADDRESS_LOW_MASK                    0xFFFFF000

#define EHCI_ASYNCLISTADDR_OFFSET                                         0x00000028
#define EHCI_ASYNCLISTADDR_TYPE                                           UInt32
#define EHCI_ASYNCLISTADDR_RESERVED_MASK                                  0x0000001F
#define    EHCI_ASYNCLISTADDR_LPL_SHIFT                                   5
#define    EHCI_ASYNCLISTADDR_LPL_MASK                                    0xFFFFFFE0

#define EHCI_CONFIGFLAG_OFFSET                                            0x00000050
#define EHCI_CONFIGFLAG_TYPE                                              UInt32
#define EHCI_CONFIGFLAG_RESERVED_MASK                                     0xFFFFFFFE
#define    EHCI_CONFIGFLAG_CONFIGURE_FLAG_SHIFT                           0
#define    EHCI_CONFIGFLAG_CONFIGURE_FLAG_MASK                            0x00000001

#define EHCI_PORTSC_0_OFFSET                                              0x00000054
#define EHCI_PORTSC_0_TYPE                                                UInt32
#define EHCI_PORTSC_0_RESERVED_MASK                                       0xFF800200
#define    EHCI_PORTSC_0_WKOC_E_SHIFT                                     22
#define    EHCI_PORTSC_0_WKOC_E_MASK                                      0x00400000
#define    EHCI_PORTSC_0_WKDSCNNT_E_SHIFT                                 21
#define    EHCI_PORTSC_0_WKDSCNNT_E_MASK                                  0x00200000
#define    EHCI_PORTSC_0_WKCNNT_E_SHIFT                                   20
#define    EHCI_PORTSC_0_WKCNNT_E_MASK                                    0x00100000
#define    EHCI_PORTSC_0_PORT_TEST_CONTROL_SHIFT                          16
#define    EHCI_PORTSC_0_PORT_TEST_CONTROL_MASK                           0x000F0000
#define    EHCI_PORTSC_0_PORT_INDICATOR_CONTROL_SHIFT                     14
#define    EHCI_PORTSC_0_PORT_INDICATOR_CONTROL_MASK                      0x0000C000
#define    EHCI_PORTSC_0_PORT_OWNER_SHIFT                                 13
#define    EHCI_PORTSC_0_PORT_OWNER_MASK                                  0x00002000
#define    EHCI_PORTSC_0_PP_SHIFT                                         12
#define    EHCI_PORTSC_0_PP_MASK                                          0x00001000
#define    EHCI_PORTSC_0_LINE_STATUS_SHIFT                                10
#define    EHCI_PORTSC_0_LINE_STATUS_MASK                                 0x00000C00
#define    EHCI_PORTSC_0_PORT_RESET_SHIFT                                 8
#define    EHCI_PORTSC_0_PORT_RESET_MASK                                  0x00000100
#define    EHCI_PORTSC_0_SUSPEND_SHIFT                                    7
#define    EHCI_PORTSC_0_SUSPEND_MASK                                     0x00000080
#define    EHCI_PORTSC_0_FORCE_PORT_RESUME_SHIFT                          6
#define    EHCI_PORTSC_0_FORCE_PORT_RESUME_MASK                           0x00000040
#define    EHCI_PORTSC_0_OVER_CURRENT_CHANGE_SHIFT                        5
#define    EHCI_PORTSC_0_OVER_CURRENT_CHANGE_MASK                         0x00000020
#define    EHCI_PORTSC_0_OVER_CURRENT_ACTIVE_SHIFT                        4
#define    EHCI_PORTSC_0_OVER_CURRENT_ACTIVE_MASK                         0x00000010
#define    EHCI_PORTSC_0_PORT_ENABLE_DISABLE_CHANGE_SHIFT                 3
#define    EHCI_PORTSC_0_PORT_ENABLE_DISABLE_CHANGE_MASK                  0x00000008
#define    EHCI_PORTSC_0_PORT_ENABLED_DISABLED_SHIFT                      2
#define    EHCI_PORTSC_0_PORT_ENABLED_DISABLED_MASK                       0x00000004
#define    EHCI_PORTSC_0_CONNECT_STATUS_CHANGE_SHIFT                      1
#define    EHCI_PORTSC_0_CONNECT_STATUS_CHANGE_MASK                       0x00000002
#define    EHCI_PORTSC_0_CURRENT_CONNECT_STATUS_SHIFT                     0
#define    EHCI_PORTSC_0_CURRENT_CONNECT_STATUS_MASK                      0x00000001

#define EHCI_PORTSC_1_OFFSET                                              0x00000058
#define EHCI_PORTSC_1_TYPE                                                UInt32
#define EHCI_PORTSC_1_RESERVED_MASK                                       0xFF800200
#define    EHCI_PORTSC_1_WKOC_E_SHIFT                                     22
#define    EHCI_PORTSC_1_WKOC_E_MASK                                      0x00400000
#define    EHCI_PORTSC_1_WKDSCNNT_E_SHIFT                                 21
#define    EHCI_PORTSC_1_WKDSCNNT_E_MASK                                  0x00200000
#define    EHCI_PORTSC_1_WKCNNT_E_SHIFT                                   20
#define    EHCI_PORTSC_1_WKCNNT_E_MASK                                    0x00100000
#define    EHCI_PORTSC_1_PORT_TEST_CONTROL_SHIFT                          16
#define    EHCI_PORTSC_1_PORT_TEST_CONTROL_MASK                           0x000F0000
#define    EHCI_PORTSC_1_PORT_INDICATOR_CONTROL_SHIFT                     14
#define    EHCI_PORTSC_1_PORT_INDICATOR_CONTROL_MASK                      0x0000C000
#define    EHCI_PORTSC_1_PORT_OWNER_SHIFT                                 13
#define    EHCI_PORTSC_1_PORT_OWNER_MASK                                  0x00002000
#define    EHCI_PORTSC_1_PP_SHIFT                                         12
#define    EHCI_PORTSC_1_PP_MASK                                          0x00001000
#define    EHCI_PORTSC_1_LINE_STATUS_SHIFT                                10
#define    EHCI_PORTSC_1_LINE_STATUS_MASK                                 0x00000C00
#define    EHCI_PORTSC_1_PORT_RESET_SHIFT                                 8
#define    EHCI_PORTSC_1_PORT_RESET_MASK                                  0x00000100
#define    EHCI_PORTSC_1_SUSPEND_SHIFT                                    7
#define    EHCI_PORTSC_1_SUSPEND_MASK                                     0x00000080
#define    EHCI_PORTSC_1_FORCE_PORT_RESUME_SHIFT                          6
#define    EHCI_PORTSC_1_FORCE_PORT_RESUME_MASK                           0x00000040
#define    EHCI_PORTSC_1_OVER_CURRENT_CHANGE_SHIFT                        5
#define    EHCI_PORTSC_1_OVER_CURRENT_CHANGE_MASK                         0x00000020
#define    EHCI_PORTSC_1_OVER_CURRENT_ACTIVE_SHIFT                        4
#define    EHCI_PORTSC_1_OVER_CURRENT_ACTIVE_MASK                         0x00000010
#define    EHCI_PORTSC_1_PORT_ENABLE_DISABLE_CHANGE_SHIFT                 3
#define    EHCI_PORTSC_1_PORT_ENABLE_DISABLE_CHANGE_MASK                  0x00000008
#define    EHCI_PORTSC_1_PORT_ENABLED_DISABLED_SHIFT                      2
#define    EHCI_PORTSC_1_PORT_ENABLED_DISABLED_MASK                       0x00000004
#define    EHCI_PORTSC_1_CONNECT_STATUS_CHANGE_SHIFT                      1
#define    EHCI_PORTSC_1_CONNECT_STATUS_CHANGE_MASK                       0x00000002
#define    EHCI_PORTSC_1_CURRENT_CONNECT_STATUS_SHIFT                     0
#define    EHCI_PORTSC_1_CURRENT_CONNECT_STATUS_MASK                      0x00000001

#define EHCI_PORTSC_2_OFFSET                                              0x0000005C
#define EHCI_PORTSC_2_TYPE                                                UInt32
#define EHCI_PORTSC_2_RESERVED_MASK                                       0xFF800200
#define    EHCI_PORTSC_2_WKOC_E_SHIFT                                     22
#define    EHCI_PORTSC_2_WKOC_E_MASK                                      0x00400000
#define    EHCI_PORTSC_2_WKDSCNNT_E_SHIFT                                 21
#define    EHCI_PORTSC_2_WKDSCNNT_E_MASK                                  0x00200000
#define    EHCI_PORTSC_2_WKCNNT_E_SHIFT                                   20
#define    EHCI_PORTSC_2_WKCNNT_E_MASK                                    0x00100000
#define    EHCI_PORTSC_2_PORT_TEST_CONTROL_SHIFT                          16
#define    EHCI_PORTSC_2_PORT_TEST_CONTROL_MASK                           0x000F0000
#define    EHCI_PORTSC_2_PORT_INDICATOR_CONTROL_SHIFT                     14
#define    EHCI_PORTSC_2_PORT_INDICATOR_CONTROL_MASK                      0x0000C000
#define    EHCI_PORTSC_2_PORT_OWNER_SHIFT                                 13
#define    EHCI_PORTSC_2_PORT_OWNER_MASK                                  0x00002000
#define    EHCI_PORTSC_2_PP_SHIFT                                         12
#define    EHCI_PORTSC_2_PP_MASK                                          0x00001000
#define    EHCI_PORTSC_2_LINE_STATUS_SHIFT                                10
#define    EHCI_PORTSC_2_LINE_STATUS_MASK                                 0x00000C00
#define    EHCI_PORTSC_2_PORT_RESET_SHIFT                                 8
#define    EHCI_PORTSC_2_PORT_RESET_MASK                                  0x00000100
#define    EHCI_PORTSC_2_SUSPEND_SHIFT                                    7
#define    EHCI_PORTSC_2_SUSPEND_MASK                                     0x00000080
#define    EHCI_PORTSC_2_FORCE_PORT_RESUME_SHIFT                          6
#define    EHCI_PORTSC_2_FORCE_PORT_RESUME_MASK                           0x00000040
#define    EHCI_PORTSC_2_OVER_CURRENT_CHANGE_SHIFT                        5
#define    EHCI_PORTSC_2_OVER_CURRENT_CHANGE_MASK                         0x00000020
#define    EHCI_PORTSC_2_OVER_CURRENT_ACTIVE_SHIFT                        4
#define    EHCI_PORTSC_2_OVER_CURRENT_ACTIVE_MASK                         0x00000010
#define    EHCI_PORTSC_2_PORT_ENABLE_DISABLE_CHANGE_SHIFT                 3
#define    EHCI_PORTSC_2_PORT_ENABLE_DISABLE_CHANGE_MASK                  0x00000008
#define    EHCI_PORTSC_2_PORT_ENABLED_DISABLED_SHIFT                      2
#define    EHCI_PORTSC_2_PORT_ENABLED_DISABLED_MASK                       0x00000004
#define    EHCI_PORTSC_2_CONNECT_STATUS_CHANGE_SHIFT                      1
#define    EHCI_PORTSC_2_CONNECT_STATUS_CHANGE_MASK                       0x00000002
#define    EHCI_PORTSC_2_CURRENT_CONNECT_STATUS_SHIFT                     0
#define    EHCI_PORTSC_2_CURRENT_CONNECT_STATUS_MASK                      0x00000001

#define EHCI_INSNREG00_OFFSET                                             0x00000090
#define EHCI_INSNREG00_TYPE                                               UInt32
#define EHCI_INSNREG00_RESERVED_MASK                                      0x00000000
#define    EHCI_INSNREG00_INSNREG00_SHIFT                                 0
#define    EHCI_INSNREG00_INSNREG00_MASK                                  0xFFFFFFFF

#define EHCI_INSNREG01_OFFSET                                             0x00000094
#define EHCI_INSNREG01_TYPE                                               UInt32
#define EHCI_INSNREG01_RESERVED_MASK                                      0x00000000
#define    EHCI_INSNREG01_INSNREG01_SHIFT                                 0
#define    EHCI_INSNREG01_INSNREG01_MASK                                  0xFFFFFFFF

#define EHCI_INSNREG02_OFFSET                                             0x00000098
#define EHCI_INSNREG02_TYPE                                               UInt32
#define EHCI_INSNREG02_RESERVED_MASK                                      0x00000000
#define    EHCI_INSNREG02_INSNREG02_SHIFT                                 0
#define    EHCI_INSNREG02_INSNREG02_MASK                                  0xFFFFFFFF

#define EHCI_INSNREG03_OFFSET                                             0x0000009C
#define EHCI_INSNREG03_TYPE                                               UInt32
#define EHCI_INSNREG03_RESERVED_MASK                                      0x00000000
#define    EHCI_INSNREG03_INSNREG03_SHIFT                                 0
#define    EHCI_INSNREG03_INSNREG03_MASK                                  0xFFFFFFFF

#define EHCI_INSNREG04_OFFSET                                             0x000000A0
#define EHCI_INSNREG04_TYPE                                               UInt32
#define EHCI_INSNREG04_RESERVED_MASK                                      0x00000000
#define    EHCI_INSNREG04_INSNREG04_SHIFT                                 0
#define    EHCI_INSNREG04_INSNREG04_MASK                                  0xFFFFFFFF

#define EHCI_INSNREG05_OFFSET                                             0x000000A4
#define EHCI_INSNREG05_TYPE                                               UInt32
#define EHCI_INSNREG05_RESERVED_MASK                                      0x00000000
#define    EHCI_INSNREG05_INSNREG05_SHIFT                                 0
#define    EHCI_INSNREG05_INSNREG05_MASK                                  0xFFFFFFFF

#define EHCI_INSNREG06_OFFSET                                             0x000000A8
#define EHCI_INSNREG06_TYPE                                               UInt32
#define EHCI_INSNREG06_RESERVED_MASK                                      0x00000000
#define    EHCI_INSNREG06_INSNREG06_SHIFT                                 0
#define    EHCI_INSNREG06_INSNREG06_MASK                                  0xFFFFFFFF

#define EHCI_INSNREG07_OFFSET                                             0x000000AC
#define EHCI_INSNREG07_TYPE                                               UInt32
#define EHCI_INSNREG07_RESERVED_MASK                                      0x00000000
#define    EHCI_INSNREG07_INSNREG07_SHIFT                                 0
#define    EHCI_INSNREG07_INSNREG07_MASK                                  0xFFFFFFFF

#define EHCI_MODE_OFFSET                                                  0x00008000
#define EHCI_MODE_TYPE                                                    UInt32
#define EHCI_MODE_RESERVED_MASK                                           0xFFFFF000
#define    EHCI_MODE_MODEP2_SHIFT                                         8
#define    EHCI_MODE_MODEP2_MASK                                          0x00000F00
#define    EHCI_MODE_MODEP1_SHIFT                                         4
#define    EHCI_MODE_MODEP1_MASK                                          0x000000F0
#define    EHCI_MODE_MODEP0_SHIFT                                         0
#define    EHCI_MODE_MODEP0_MASK                                          0x0000000F

#define EHCI_STRAP_Q_OFFSET                                               0x00008004
#define EHCI_STRAP_Q_TYPE                                                 UInt32
#define EHCI_STRAP_Q_RESERVED_MASK                                        0xFFFFFF00
#define    EHCI_STRAP_Q_STRAP_Q_SHIFT                                     0
#define    EHCI_STRAP_Q_STRAP_Q_MASK                                      0x000000FF

#define EHCI_FRAMELEN_ADJ_Q_OFFSET                                        0x00008008
#define EHCI_FRAMELEN_ADJ_Q_TYPE                                          UInt32
#define EHCI_FRAMELEN_ADJ_Q_RESERVED_MASK                                 0xFFFFFFC0
#define    EHCI_FRAMELEN_ADJ_Q_FRAMELEN_ADJ_Q_SHIFT                       0
#define    EHCI_FRAMELEN_ADJ_Q_FRAMELEN_ADJ_Q_MASK                        0x0000003F

#define EHCI_FRAMELEN_ADJ_QX0_OFFSET                                      0x0000800C
#define EHCI_FRAMELEN_ADJ_QX0_TYPE                                        UInt32
#define EHCI_FRAMELEN_ADJ_QX0_RESERVED_MASK                               0xFFFFFFC0
#define    EHCI_FRAMELEN_ADJ_QX0_FRAMELEN_ADJ_Q_SHIFT                     0
#define    EHCI_FRAMELEN_ADJ_QX0_FRAMELEN_ADJ_Q_MASK                      0x0000003F

#define EHCI_FRAMELEN_ADJ_QX1_OFFSET                                      0x00008010
#define EHCI_FRAMELEN_ADJ_QX1_TYPE                                        UInt32
#define EHCI_FRAMELEN_ADJ_QX1_RESERVED_MASK                               0xFFFFFFC0
#define    EHCI_FRAMELEN_ADJ_QX1_FRAMELEN_ADJ_Q_SHIFT                     0
#define    EHCI_FRAMELEN_ADJ_QX1_FRAMELEN_ADJ_Q_MASK                      0x0000003F

#define EHCI_FRAMELEN_ADJ_QX2_OFFSET                                      0x00008014
#define EHCI_FRAMELEN_ADJ_QX2_TYPE                                        UInt32
#define EHCI_FRAMELEN_ADJ_QX2_RESERVED_MASK                               0xFFFFFFC0
#define    EHCI_FRAMELEN_ADJ_QX2_FRAMELEN_ADJ_Q_SHIFT                     0
#define    EHCI_FRAMELEN_ADJ_QX2_FRAMELEN_ADJ_Q_MASK                      0x0000003F

#define EHCI_MISC_OFFSET                                                  0x00008018
#define EHCI_MISC_TYPE                                                    UInt32
#define EHCI_MISC_RESERVED_MASK                                           0xFFFFFFE0
#define    EHCI_MISC_SS_UTMI_BACKWARD_ENB_SHIFT                           4
#define    EHCI_MISC_SS_UTMI_BACKWARD_ENB_MASK                            0x00000010
#define    EHCI_MISC_SS_RESUME_UTMI_PLS_DIS_SHIFT                         3
#define    EHCI_MISC_SS_RESUME_UTMI_PLS_DIS_MASK                          0x00000008
#define    EHCI_MISC_ULPI_BYPASS_EN_SHIFT                                 2
#define    EHCI_MISC_ULPI_BYPASS_EN_MASK                                  0x00000004
#define    EHCI_MISC_SS_AUTOPPD_ON_OVERCUR_EN_SHIFT                       1
#define    EHCI_MISC_SS_AUTOPPD_ON_OVERCUR_EN_MASK                        0x00000002
#define    EHCI_MISC_APP_START_CLK_SHIFT                                  0
#define    EHCI_MISC_APP_START_CLK_MASK                                   0x00000001

#endif /* __BRCM_RDB_EHCI_H__ */


