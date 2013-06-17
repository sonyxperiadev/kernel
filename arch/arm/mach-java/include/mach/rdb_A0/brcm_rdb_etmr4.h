/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2013  Broadcom Corporation                                                        */
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
/*     Date     : Generated on 3/4/2013 11:52:5                                             */
/*     RDB file : //JAVA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_ETMR4_H__
#define __BRCM_RDB_ETMR4_H__

#define ETMR4_ETMCTL_OFFSET                                               0x00000000
#define ETMR4_ETMCTL_TYPE                                                 UInt32
#define ETMR4_ETMCTL_RESERVED_MASK                                        0xF0000000
#define    ETMR4_ETMCTL_CORESEL_SHIFT                                     25
#define    ETMR4_ETMCTL_CORESEL_MASK                                      0x0E000000
#define    ETMR4_ETMCTL_INSTACCCTL_SHIFT                                  24
#define    ETMR4_ETMCTL_INSTACCCTL_MASK                                   0x01000000
#define    ETMR4_ETMCTL_DISABLESWWR_SHIFT                                 23
#define    ETMR4_ETMCTL_DISABLESWWR_MASK                                  0x00800000
#define    ETMR4_ETMCTL_DISABLEDBGRWR_SHIFT                               22
#define    ETMR4_ETMCTL_DISABLEDBGRWR_MASK                                0x00400000
#define    ETMR4_ETMCTL_PORTSIZE3_SHIFT                                   21
#define    ETMR4_ETMCTL_PORTSIZE3_MASK                                    0x00200000
#define    ETMR4_ETMCTL_DATAONLYMODE_SHIFT                                20
#define    ETMR4_ETMCTL_DATAONLYMODE_MASK                                 0x00100000
#define    ETMR4_ETMCTL_FILTERCPRT_SHIFT                                  19
#define    ETMR4_ETMCTL_FILTERCPRT_MASK                                   0x00080000
#define    ETMR4_ETMCTL_SUPPRESSDATA_SHIFT                                18
#define    ETMR4_ETMCTL_SUPPRESSDATA_MASK                                 0x00040000
#define    ETMR4_ETMCTL_PORTMODE10_SHIFT                                  16
#define    ETMR4_ETMCTL_PORTMODE10_MASK                                   0x00030000
#define    ETMR4_ETMCTL_CONTEXTIDSIZE_SHIFT                               14
#define    ETMR4_ETMCTL_CONTEXTIDSIZE_MASK                                0x0000C000
#define    ETMR4_ETMCTL_PORTMODE2_SHIFT                                   13
#define    ETMR4_ETMCTL_PORTMODE2_MASK                                    0x00002000
#define    ETMR4_ETMCTL_CYCACCTRACK_SHIFT                                 12
#define    ETMR4_ETMCTL_CYCACCTRACK_MASK                                  0x00001000
#define    ETMR4_ETMCTL_ETMPORTSEL_SHIFT                                  11
#define    ETMR4_ETMCTL_ETMPORTSEL_MASK                                   0x00000800
#define    ETMR4_ETMCTL_ETMPROG_SHIFT                                     10
#define    ETMR4_ETMCTL_ETMPROG_MASK                                      0x00000400
#define    ETMR4_ETMCTL_DBGREQCTL_SHIFT                                   9
#define    ETMR4_ETMCTL_DBGREQCTL_MASK                                    0x00000200
#define    ETMR4_ETMCTL_BRANCHOUTPUT_SHIFT                                8
#define    ETMR4_ETMCTL_BRANCHOUTPUT_MASK                                 0x00000100
#define    ETMR4_ETMCTL_STALLCPU_SHIFT                                    7
#define    ETMR4_ETMCTL_STALLCPU_MASK                                     0x00000080
#define    ETMR4_ETMCTL_PORTSIZE20_SHIFT                                  4
#define    ETMR4_ETMCTL_PORTSIZE20_MASK                                   0x00000070
#define    ETMR4_ETMCTL_DATAACCESS_SHIFT                                  2
#define    ETMR4_ETMCTL_DATAACCESS_MASK                                   0x0000000C
#define    ETMR4_ETMCTL_MONITORCPRT_SHIFT                                 1
#define    ETMR4_ETMCTL_MONITORCPRT_MASK                                  0x00000002
#define    ETMR4_ETMCTL_ETMPWRDOWN_SHIFT                                  0
#define    ETMR4_ETMCTL_ETMPWRDOWN_MASK                                   0x00000001

#define ETMR4_CFGCODE_OFFSET                                              0x00000004
#define ETMR4_CFGCODE_TYPE                                                UInt32
#define ETMR4_CFGCODE_RESERVED_MASK                                       0x70400000
#define    ETMR4_CFGCODE_ETMIDREGPRSNT_SHIFT                              31
#define    ETMR4_CFGCODE_ETMIDREGPRSNT_MASK                               0x80000000
#define    ETMR4_CFGCODE_SWACCSUPP_SHIFT                                  27
#define    ETMR4_CFGCODE_SWACCSUPP_MASK                                   0x08000000
#define    ETMR4_CFGCODE_TRSTARTSTOP_SHIFT                                26
#define    ETMR4_CFGCODE_TRSTARTSTOP_MASK                                 0x04000000
#define    ETMR4_CFGCODE_CIDCMPNUM_SHIFT                                  24
#define    ETMR4_CFGCODE_CIDCMPNUM_MASK                                   0x03000000
#define    ETMR4_CFGCODE_FIFOFULL_SHIFT                                   23
#define    ETMR4_CFGCODE_FIFOFULL_MASK                                    0x00800000
#define    ETMR4_CFGCODE_EXTOUTNUM_SHIFT                                  20
#define    ETMR4_CFGCODE_EXTOUTNUM_MASK                                   0x00300000
#define    ETMR4_CFGCODE_MAXEXTIN_SHIFT                                   17
#define    ETMR4_CFGCODE_MAXEXTIN_MASK                                    0x000E0000
#define    ETMR4_CFGCODE_SEQPRESENT_SHIFT                                 16
#define    ETMR4_CFGCODE_SEQPRESENT_MASK                                  0x00010000
#define    ETMR4_CFGCODE_NUMCOUNTERS_SHIFT                                13
#define    ETMR4_CFGCODE_NUMCOUNTERS_MASK                                 0x0000E000
#define    ETMR4_CFGCODE_NUMDECODERS_SHIFT                                8
#define    ETMR4_CFGCODE_NUMDECODERS_MASK                                 0x00001F00
#define    ETMR4_CFGCODE_NUMDATACMP_SHIFT                                 4
#define    ETMR4_CFGCODE_NUMDATACMP_MASK                                  0x000000F0
#define    ETMR4_CFGCODE_NUMADDRCMP_SHIFT                                 0
#define    ETMR4_CFGCODE_NUMADDRCMP_MASK                                  0x0000000F

#define ETMR4_TRIGEVENT_OFFSET                                            0x00000008
#define ETMR4_TRIGEVENT_TYPE                                              UInt32
#define ETMR4_TRIGEVENT_RESERVED_MASK                                     0xFFFE0000
#define    ETMR4_TRIGEVENT_FUNCTION_SHIFT                                 14
#define    ETMR4_TRIGEVENT_FUNCTION_MASK                                  0x0001C000
#define    ETMR4_TRIGEVENT_RESOURCEB_SHIFT                                7
#define    ETMR4_TRIGEVENT_RESOURCEB_MASK                                 0x00003F80
#define    ETMR4_TRIGEVENT_RESOURCEA_SHIFT                                0
#define    ETMR4_TRIGEVENT_RESOURCEA_MASK                                 0x0000007F

#define ETMR4_ASICCTL_OFFSET                                              0x0000000C
#define ETMR4_ASICCTL_TYPE                                                UInt32
#define ETMR4_ASICCTL_RESERVED_MASK                                       0xFFFFFF00
#define    ETMR4_ASICCTL_ASICCTL_SHIFT                                    0
#define    ETMR4_ASICCTL_ASICCTL_MASK                                     0x000000FF

#define ETMR4_ETMSTAT_OFFSET                                              0x00000010
#define ETMR4_ETMSTAT_TYPE                                                UInt32
#define ETMR4_ETMSTAT_RESERVED_MASK                                       0xFFFFFFF0
#define    ETMR4_ETMSTAT_TRIGGERFLAG_SHIFT                                3
#define    ETMR4_ETMSTAT_TRIGGERFLAG_MASK                                 0x00000008
#define    ETMR4_ETMSTAT_STARTSTOPSTAT_SHIFT                              2
#define    ETMR4_ETMSTAT_STARTSTOPSTAT_MASK                               0x00000004
#define    ETMR4_ETMSTAT_PROGBIT_SHIFT                                    1
#define    ETMR4_ETMSTAT_PROGBIT_MASK                                     0x00000002
#define    ETMR4_ETMSTAT_UNTRACEDOVFL_SHIFT                               0
#define    ETMR4_ETMSTAT_UNTRACEDOVFL_MASK                                0x00000001

#define ETMR4_SYSCFG_OFFSET                                               0x00000014
#define ETMR4_SYSCFG_TYPE                                                 UInt32
#define ETMR4_SYSCFG_RESERVED_MASK                                        0xFFFD80F8
#define    ETMR4_SYSCFG_NOFETCHCMP_SHIFT                                  17
#define    ETMR4_SYSCFG_NOFETCHCMP_MASK                                   0x00020000
#define    ETMR4_SYSCFG_NUMCORES_SHIFT                                    12
#define    ETMR4_SYSCFG_NUMCORES_MASK                                     0x00007000
#define    ETMR4_SYSCFG_PORTMODESUP_SHIFT                                 11
#define    ETMR4_SYSCFG_PORTMODESUP_MASK                                  0x00000800
#define    ETMR4_SYSCFG_PORTSIZESUP_SHIFT                                 10
#define    ETMR4_SYSCFG_PORTSIZESUP_MASK                                  0x00000400
#define    ETMR4_SYSCFG_MAXPORTSIZE3_SHIFT                                9
#define    ETMR4_SYSCFG_MAXPORTSIZE3_MASK                                 0x00000200
#define    ETMR4_SYSCFG_FIFOFULLSUP_SHIFT                                 8
#define    ETMR4_SYSCFG_FIFOFULLSUP_MASK                                  0x00000100
#define    ETMR4_SYSCFG_MAXPORTSIZE_SHIFT                                 0
#define    ETMR4_SYSCFG_MAXPORTSIZE_MASK                                  0x00000007

#define ETMR4_TENSTARTSTOP_OFFSET                                         0x00000018
#define ETMR4_TENSTARTSTOP_TYPE                                           UInt32
#define ETMR4_TENSTARTSTOP_RESERVED_MASK                                  0x00000000
#define    ETMR4_TENSTARTSTOP_ACMPSTOPEN_SHIFT                            16
#define    ETMR4_TENSTARTSTOP_ACMPSTOPEN_MASK                             0xFFFF0000
#define    ETMR4_TENSTARTSTOP_ACMPSTARTEN_SHIFT                           0
#define    ETMR4_TENSTARTSTOP_ACMPSTARTEN_MASK                            0x0000FFFF

#define ETMR4_TENCTL2_OFFSET                                              0x0000001C
#define ETMR4_TENCTL2_TYPE                                                UInt32
#define ETMR4_TENCTL2_RESERVED_MASK                                       0xFFFF0000
#define    ETMR4_TENCTL2_ACMPINCEXC_SHIFT                                 0
#define    ETMR4_TENCTL2_ACMPINCEXC_MASK                                  0x0000FFFF

#define ETMR4_TENEVENT_OFFSET                                             0x00000020
#define ETMR4_TENEVENT_TYPE                                               UInt32
#define ETMR4_TENEVENT_RESERVED_MASK                                      0xFFFE0000
#define    ETMR4_TENEVENT_FUNCTION_SHIFT                                  14
#define    ETMR4_TENEVENT_FUNCTION_MASK                                   0x0001C000
#define    ETMR4_TENEVENT_RESOURCEB_SHIFT                                 7
#define    ETMR4_TENEVENT_RESOURCEB_MASK                                  0x00003F80
#define    ETMR4_TENEVENT_RESOURCEA_SHIFT                                 0
#define    ETMR4_TENEVENT_RESOURCEA_MASK                                  0x0000007F

#define ETMR4_TENCTL1_OFFSET                                              0x00000024
#define ETMR4_TENCTL1_TYPE                                                UInt32
#define ETMR4_TENCTL1_RESERVED_MASK                                       0xFC000000
#define    ETMR4_TENCTL1_TRCTLEN_SHIFT                                    25
#define    ETMR4_TENCTL1_TRCTLEN_MASK                                     0x02000000
#define    ETMR4_TENCTL1_INCEXCCTL_SHIFT                                  24
#define    ETMR4_TENCTL1_INCEXCCTL_MASK                                   0x01000000
#define    ETMR4_TENCTL1_MEMMAPDECODES_SHIFT                              8
#define    ETMR4_TENCTL1_MEMMAPDECODES_MASK                               0x00FFFF00
#define    ETMR4_TENCTL1_ADDRCMP_SHIFT                                    0
#define    ETMR4_TENCTL1_ADDRCMP_MASK                                     0x000000FF

#define ETMR4_FIFOFULLLVL_OFFSET                                          0x0000002C
#define ETMR4_FIFOFULLLVL_TYPE                                            UInt32
#define ETMR4_FIFOFULLLVL_RESERVED_MASK                                   0xFFFFFF00
#define    ETMR4_FIFOFULLLVL_FIFOFULLLVL_SHIFT                            0
#define    ETMR4_FIFOFULLLVL_FIFOFULLLVL_MASK                             0x000000FF

#define ETMR4_VIEWDATAEVENT_OFFSET                                        0x00000030
#define ETMR4_VIEWDATAEVENT_TYPE                                          UInt32
#define ETMR4_VIEWDATAEVENT_RESERVED_MASK                                 0xFFFE0000
#define    ETMR4_VIEWDATAEVENT_FUNCTION_SHIFT                             14
#define    ETMR4_VIEWDATAEVENT_FUNCTION_MASK                              0x0001C000
#define    ETMR4_VIEWDATAEVENT_RESOURCEB_SHIFT                            7
#define    ETMR4_VIEWDATAEVENT_RESOURCEB_MASK                             0x00003F80
#define    ETMR4_VIEWDATAEVENT_RESOURCEA_SHIFT                            0
#define    ETMR4_VIEWDATAEVENT_RESOURCEA_MASK                             0x0000007F

#define ETMR4_VIEWDATACTL1_OFFSET                                         0x00000034
#define ETMR4_VIEWDATACTL1_TYPE                                           UInt32
#define ETMR4_VIEWDATACTL1_RESERVED_MASK                                  0x00000000
#define    ETMR4_VIEWDATACTL1_ACMPEXCLEN_SHIFT                            16
#define    ETMR4_VIEWDATACTL1_ACMPEXCLEN_MASK                             0xFFFF0000
#define    ETMR4_VIEWDATACTL1_ACMPINCLEN_SHIFT                            0
#define    ETMR4_VIEWDATACTL1_ACMPINCLEN_MASK                             0x0000FFFF

#define ETMR4_VIEWDATACTL3_OFFSET                                         0x0000003C
#define ETMR4_VIEWDATACTL3_TYPE                                           UInt32
#define ETMR4_VIEWDATACTL3_RESERVED_MASK                                  0xFFFE0000
#define    ETMR4_VIEWDATACTL3_EXCLONLYMODE_SHIFT                          16
#define    ETMR4_VIEWDATACTL3_EXCLONLYMODE_MASK                           0x00010000
#define    ETMR4_VIEWDATACTL3_ARCMPEXCLEN_SHIFT                           8
#define    ETMR4_VIEWDATACTL3_ARCMPEXCLEN_MASK                            0x0000FF00
#define    ETMR4_VIEWDATACTL3_ARCMPINCLEN_SHIFT                           0
#define    ETMR4_VIEWDATACTL3_ARCMPINCLEN_MASK                            0x000000FF

#define ETMR4_ADDRCMPVAL1_OFFSET                                          0x00000040
#define ETMR4_ADDRCMPVAL1_TYPE                                            UInt32
#define ETMR4_ADDRCMPVAL1_RESERVED_MASK                                   0x00000000
#define    ETMR4_ADDRCMPVAL1_ADDRCMPVAL_SHIFT                             0
#define    ETMR4_ADDRCMPVAL1_ADDRCMPVAL_MASK                              0xFFFFFFFF

#define ETMR4_ADDRCMPVAL2_OFFSET                                          0x00000044
#define ETMR4_ADDRCMPVAL2_TYPE                                            UInt32
#define ETMR4_ADDRCMPVAL2_RESERVED_MASK                                   0x00000000
#define    ETMR4_ADDRCMPVAL2_ADDRCMPVAL_SHIFT                             0
#define    ETMR4_ADDRCMPVAL2_ADDRCMPVAL_MASK                              0xFFFFFFFF

#define ETMR4_ADDRCMPVAL3_OFFSET                                          0x00000048
#define ETMR4_ADDRCMPVAL3_TYPE                                            UInt32
#define ETMR4_ADDRCMPVAL3_RESERVED_MASK                                   0x00000000
#define    ETMR4_ADDRCMPVAL3_ADDRCMPVAL_SHIFT                             0
#define    ETMR4_ADDRCMPVAL3_ADDRCMPVAL_MASK                              0xFFFFFFFF

#define ETMR4_ADDRCMPVAL4_OFFSET                                          0x0000004C
#define ETMR4_ADDRCMPVAL4_TYPE                                            UInt32
#define ETMR4_ADDRCMPVAL4_RESERVED_MASK                                   0x00000000
#define    ETMR4_ADDRCMPVAL4_ADDRCMPVAL_SHIFT                             0
#define    ETMR4_ADDRCMPVAL4_ADDRCMPVAL_MASK                              0xFFFFFFFF

#define ETMR4_ADDRCMPVAL5_OFFSET                                          0x00000050
#define ETMR4_ADDRCMPVAL5_TYPE                                            UInt32
#define ETMR4_ADDRCMPVAL5_RESERVED_MASK                                   0x00000000
#define    ETMR4_ADDRCMPVAL5_ADDRCMPVAL_SHIFT                             0
#define    ETMR4_ADDRCMPVAL5_ADDRCMPVAL_MASK                              0xFFFFFFFF

#define ETMR4_ADDRCMPVAL6_OFFSET                                          0x00000054
#define ETMR4_ADDRCMPVAL6_TYPE                                            UInt32
#define ETMR4_ADDRCMPVAL6_RESERVED_MASK                                   0x00000000
#define    ETMR4_ADDRCMPVAL6_ADDRCMPVAL_SHIFT                             0
#define    ETMR4_ADDRCMPVAL6_ADDRCMPVAL_MASK                              0xFFFFFFFF

#define ETMR4_ADDRCMPVAL7_OFFSET                                          0x00000058
#define ETMR4_ADDRCMPVAL7_TYPE                                            UInt32
#define ETMR4_ADDRCMPVAL7_RESERVED_MASK                                   0x00000000
#define    ETMR4_ADDRCMPVAL7_ADDRCMPVAL_SHIFT                             0
#define    ETMR4_ADDRCMPVAL7_ADDRCMPVAL_MASK                              0xFFFFFFFF

#define ETMR4_ADDRCMPVAL8_OFFSET                                          0x0000005C
#define ETMR4_ADDRCMPVAL8_TYPE                                            UInt32
#define ETMR4_ADDRCMPVAL8_RESERVED_MASK                                   0x00000000
#define    ETMR4_ADDRCMPVAL8_ADDRCMPVAL_SHIFT                             0
#define    ETMR4_ADDRCMPVAL8_ADDRCMPVAL_MASK                              0xFFFFFFFF

#define ETMR4_ADDRACCTYPE1_OFFSET                                         0x00000080
#define ETMR4_ADDRACCTYPE1_TYPE                                           UInt32
#define ETMR4_ADDRACCTYPE1_RESERVED_MASK                                  0xFFFFF000
#define    ETMR4_ADDRACCTYPE1_SECLVLCTL_SHIFT                             10
#define    ETMR4_ADDRACCTYPE1_SECLVLCTL_MASK                              0x00000C00
#define    ETMR4_ADDRACCTYPE1_CIDCMPCTL_SHIFT                             8
#define    ETMR4_ADDRACCTYPE1_CIDCMPCTL_MASK                              0x00000300
#define    ETMR4_ADDRACCTYPE1_EXACTMATCH_SHIFT                            7
#define    ETMR4_ADDRACCTYPE1_EXACTMATCH_MASK                             0x00000080
#define    ETMR4_ADDRACCTYPE1_DATACMPCTL_SHIFT                            5
#define    ETMR4_ADDRACCTYPE1_DATACMPCTL_MASK                             0x00000060
#define    ETMR4_ADDRACCTYPE1_CMPACCSIZE_SHIFT                            3
#define    ETMR4_ADDRACCTYPE1_CMPACCSIZE_MASK                             0x00000018
#define    ETMR4_ADDRACCTYPE1_ACCTYPE_SHIFT                               0
#define    ETMR4_ADDRACCTYPE1_ACCTYPE_MASK                                0x00000007

#define ETMR4_ADDRACCTYPE2_OFFSET                                         0x00000084
#define ETMR4_ADDRACCTYPE2_TYPE                                           UInt32
#define ETMR4_ADDRACCTYPE2_RESERVED_MASK                                  0xFFFFF000
#define    ETMR4_ADDRACCTYPE2_SECLVLCTL_SHIFT                             10
#define    ETMR4_ADDRACCTYPE2_SECLVLCTL_MASK                              0x00000C00
#define    ETMR4_ADDRACCTYPE2_CIDCMPCTL_SHIFT                             8
#define    ETMR4_ADDRACCTYPE2_CIDCMPCTL_MASK                              0x00000300
#define    ETMR4_ADDRACCTYPE2_EXACTMATCH_SHIFT                            7
#define    ETMR4_ADDRACCTYPE2_EXACTMATCH_MASK                             0x00000080
#define    ETMR4_ADDRACCTYPE2_DATACMPCTL_SHIFT                            5
#define    ETMR4_ADDRACCTYPE2_DATACMPCTL_MASK                             0x00000060
#define    ETMR4_ADDRACCTYPE2_CMPACCSIZE_SHIFT                            3
#define    ETMR4_ADDRACCTYPE2_CMPACCSIZE_MASK                             0x00000018
#define    ETMR4_ADDRACCTYPE2_ACCTYPE_SHIFT                               0
#define    ETMR4_ADDRACCTYPE2_ACCTYPE_MASK                                0x00000007

#define ETMR4_ADDRACCTYPE3_OFFSET                                         0x00000088
#define ETMR4_ADDRACCTYPE3_TYPE                                           UInt32
#define ETMR4_ADDRACCTYPE3_RESERVED_MASK                                  0xFFFFF000
#define    ETMR4_ADDRACCTYPE3_SECLVLCTL_SHIFT                             10
#define    ETMR4_ADDRACCTYPE3_SECLVLCTL_MASK                              0x00000C00
#define    ETMR4_ADDRACCTYPE3_CIDCMPCTL_SHIFT                             8
#define    ETMR4_ADDRACCTYPE3_CIDCMPCTL_MASK                              0x00000300
#define    ETMR4_ADDRACCTYPE3_EXACTMATCH_SHIFT                            7
#define    ETMR4_ADDRACCTYPE3_EXACTMATCH_MASK                             0x00000080
#define    ETMR4_ADDRACCTYPE3_DATACMPCTL_SHIFT                            5
#define    ETMR4_ADDRACCTYPE3_DATACMPCTL_MASK                             0x00000060
#define    ETMR4_ADDRACCTYPE3_CMPACCSIZE_SHIFT                            3
#define    ETMR4_ADDRACCTYPE3_CMPACCSIZE_MASK                             0x00000018
#define    ETMR4_ADDRACCTYPE3_ACCTYPE_SHIFT                               0
#define    ETMR4_ADDRACCTYPE3_ACCTYPE_MASK                                0x00000007

#define ETMR4_ADDRACCTYPE4_OFFSET                                         0x0000008C
#define ETMR4_ADDRACCTYPE4_TYPE                                           UInt32
#define ETMR4_ADDRACCTYPE4_RESERVED_MASK                                  0xFFFFF000
#define    ETMR4_ADDRACCTYPE4_SECLVLCTL_SHIFT                             10
#define    ETMR4_ADDRACCTYPE4_SECLVLCTL_MASK                              0x00000C00
#define    ETMR4_ADDRACCTYPE4_CIDCMPCTL_SHIFT                             8
#define    ETMR4_ADDRACCTYPE4_CIDCMPCTL_MASK                              0x00000300
#define    ETMR4_ADDRACCTYPE4_EXACTMATCH_SHIFT                            7
#define    ETMR4_ADDRACCTYPE4_EXACTMATCH_MASK                             0x00000080
#define    ETMR4_ADDRACCTYPE4_DATACMPCTL_SHIFT                            5
#define    ETMR4_ADDRACCTYPE4_DATACMPCTL_MASK                             0x00000060
#define    ETMR4_ADDRACCTYPE4_CMPACCSIZE_SHIFT                            3
#define    ETMR4_ADDRACCTYPE4_CMPACCSIZE_MASK                             0x00000018
#define    ETMR4_ADDRACCTYPE4_ACCTYPE_SHIFT                               0
#define    ETMR4_ADDRACCTYPE4_ACCTYPE_MASK                                0x00000007

#define ETMR4_ADDRACCTYPE5_OFFSET                                         0x00000090
#define ETMR4_ADDRACCTYPE5_TYPE                                           UInt32
#define ETMR4_ADDRACCTYPE5_RESERVED_MASK                                  0xFFFFF000
#define    ETMR4_ADDRACCTYPE5_SECLVLCTL_SHIFT                             10
#define    ETMR4_ADDRACCTYPE5_SECLVLCTL_MASK                              0x00000C00
#define    ETMR4_ADDRACCTYPE5_CIDCMPCTL_SHIFT                             8
#define    ETMR4_ADDRACCTYPE5_CIDCMPCTL_MASK                              0x00000300
#define    ETMR4_ADDRACCTYPE5_EXACTMATCH_SHIFT                            7
#define    ETMR4_ADDRACCTYPE5_EXACTMATCH_MASK                             0x00000080
#define    ETMR4_ADDRACCTYPE5_DATACMPCTL_SHIFT                            5
#define    ETMR4_ADDRACCTYPE5_DATACMPCTL_MASK                             0x00000060
#define    ETMR4_ADDRACCTYPE5_CMPACCSIZE_SHIFT                            3
#define    ETMR4_ADDRACCTYPE5_CMPACCSIZE_MASK                             0x00000018
#define    ETMR4_ADDRACCTYPE5_ACCTYPE_SHIFT                               0
#define    ETMR4_ADDRACCTYPE5_ACCTYPE_MASK                                0x00000007

#define ETMR4_ADDRACCTYPE6_OFFSET                                         0x00000094
#define ETMR4_ADDRACCTYPE6_TYPE                                           UInt32
#define ETMR4_ADDRACCTYPE6_RESERVED_MASK                                  0xFFFFF000
#define    ETMR4_ADDRACCTYPE6_SECLVLCTL_SHIFT                             10
#define    ETMR4_ADDRACCTYPE6_SECLVLCTL_MASK                              0x00000C00
#define    ETMR4_ADDRACCTYPE6_CIDCMPCTL_SHIFT                             8
#define    ETMR4_ADDRACCTYPE6_CIDCMPCTL_MASK                              0x00000300
#define    ETMR4_ADDRACCTYPE6_EXACTMATCH_SHIFT                            7
#define    ETMR4_ADDRACCTYPE6_EXACTMATCH_MASK                             0x00000080
#define    ETMR4_ADDRACCTYPE6_DATACMPCTL_SHIFT                            5
#define    ETMR4_ADDRACCTYPE6_DATACMPCTL_MASK                             0x00000060
#define    ETMR4_ADDRACCTYPE6_CMPACCSIZE_SHIFT                            3
#define    ETMR4_ADDRACCTYPE6_CMPACCSIZE_MASK                             0x00000018
#define    ETMR4_ADDRACCTYPE6_ACCTYPE_SHIFT                               0
#define    ETMR4_ADDRACCTYPE6_ACCTYPE_MASK                                0x00000007

#define ETMR4_ADDRACCTYPE7_OFFSET                                         0x00000098
#define ETMR4_ADDRACCTYPE7_TYPE                                           UInt32
#define ETMR4_ADDRACCTYPE7_RESERVED_MASK                                  0xFFFFF000
#define    ETMR4_ADDRACCTYPE7_SECLVLCTL_SHIFT                             10
#define    ETMR4_ADDRACCTYPE7_SECLVLCTL_MASK                              0x00000C00
#define    ETMR4_ADDRACCTYPE7_CIDCMPCTL_SHIFT                             8
#define    ETMR4_ADDRACCTYPE7_CIDCMPCTL_MASK                              0x00000300
#define    ETMR4_ADDRACCTYPE7_EXACTMATCH_SHIFT                            7
#define    ETMR4_ADDRACCTYPE7_EXACTMATCH_MASK                             0x00000080
#define    ETMR4_ADDRACCTYPE7_DATACMPCTL_SHIFT                            5
#define    ETMR4_ADDRACCTYPE7_DATACMPCTL_MASK                             0x00000060
#define    ETMR4_ADDRACCTYPE7_CMPACCSIZE_SHIFT                            3
#define    ETMR4_ADDRACCTYPE7_CMPACCSIZE_MASK                             0x00000018
#define    ETMR4_ADDRACCTYPE7_ACCTYPE_SHIFT                               0
#define    ETMR4_ADDRACCTYPE7_ACCTYPE_MASK                                0x00000007

#define ETMR4_ADDRACCTYPE8_OFFSET                                         0x0000009C
#define ETMR4_ADDRACCTYPE8_TYPE                                           UInt32
#define ETMR4_ADDRACCTYPE8_RESERVED_MASK                                  0xFFFFF000
#define    ETMR4_ADDRACCTYPE8_SECLVLCTL_SHIFT                             10
#define    ETMR4_ADDRACCTYPE8_SECLVLCTL_MASK                              0x00000C00
#define    ETMR4_ADDRACCTYPE8_CIDCMPCTL_SHIFT                             8
#define    ETMR4_ADDRACCTYPE8_CIDCMPCTL_MASK                              0x00000300
#define    ETMR4_ADDRACCTYPE8_EXACTMATCH_SHIFT                            7
#define    ETMR4_ADDRACCTYPE8_EXACTMATCH_MASK                             0x00000080
#define    ETMR4_ADDRACCTYPE8_DATACMPCTL_SHIFT                            5
#define    ETMR4_ADDRACCTYPE8_DATACMPCTL_MASK                             0x00000060
#define    ETMR4_ADDRACCTYPE8_CMPACCSIZE_SHIFT                            3
#define    ETMR4_ADDRACCTYPE8_CMPACCSIZE_MASK                             0x00000018
#define    ETMR4_ADDRACCTYPE8_ACCTYPE_SHIFT                               0
#define    ETMR4_ADDRACCTYPE8_ACCTYPE_MASK                                0x00000007

#define ETMR4_DATACMPVAL1_OFFSET                                          0x000000C0
#define ETMR4_DATACMPVAL1_TYPE                                            UInt32
#define ETMR4_DATACMPVAL1_RESERVED_MASK                                   0x00000000
#define    ETMR4_DATACMPVAL1_DATACMPVAL_SHIFT                             0
#define    ETMR4_DATACMPVAL1_DATACMPVAL_MASK                              0xFFFFFFFF

#define ETMR4_DATACMPVAL3_OFFSET                                          0x000000C8
#define ETMR4_DATACMPVAL3_TYPE                                            UInt32
#define ETMR4_DATACMPVAL3_RESERVED_MASK                                   0x00000000
#define    ETMR4_DATACMPVAL3_DATACMPVAL_SHIFT                             0
#define    ETMR4_DATACMPVAL3_DATACMPVAL_MASK                              0xFFFFFFFF

#define ETMR4_DATACMPMASK1_OFFSET                                         0x00000100
#define ETMR4_DATACMPMASK1_TYPE                                           UInt32
#define ETMR4_DATACMPMASK1_RESERVED_MASK                                  0x00000000
#define    ETMR4_DATACMPMASK1_DATACMPMASK_SHIFT                           0
#define    ETMR4_DATACMPMASK1_DATACMPMASK_MASK                            0xFFFFFFFF

#define ETMR4_DATACMPMASK3_OFFSET                                         0x00000108
#define ETMR4_DATACMPMASK3_TYPE                                           UInt32
#define ETMR4_DATACMPMASK3_RESERVED_MASK                                  0x00000000
#define    ETMR4_DATACMPMASK3_DATACMPMASK_SHIFT                           0
#define    ETMR4_DATACMPMASK3_DATACMPMASK_MASK                            0xFFFFFFFF

#define ETMR4_CNTRRELDVAL1_OFFSET                                         0x00000140
#define ETMR4_CNTRRELDVAL1_TYPE                                           UInt32
#define ETMR4_CNTRRELDVAL1_RESERVED_MASK                                  0xFFFF0000
#define    ETMR4_CNTRRELDVAL1_INITIALCOUNT_SHIFT                          0
#define    ETMR4_CNTRRELDVAL1_INITIALCOUNT_MASK                           0x0000FFFF

#define ETMR4_CNTRRELDVAL2_OFFSET                                         0x00000144
#define ETMR4_CNTRRELDVAL2_TYPE                                           UInt32
#define ETMR4_CNTRRELDVAL2_RESERVED_MASK                                  0xFFFF0000
#define    ETMR4_CNTRRELDVAL2_INITIALCOUNT_SHIFT                          0
#define    ETMR4_CNTRRELDVAL2_INITIALCOUNT_MASK                           0x0000FFFF

#define ETMR4_CNTRENEVNT1_OFFSET                                          0x00000150
#define ETMR4_CNTRENEVNT1_TYPE                                            UInt32
#define ETMR4_CNTRENEVNT1_RESERVED_MASK                                   0xFFFE0000
#define    ETMR4_CNTRENEVNT1_FUNCTION_SHIFT                               14
#define    ETMR4_CNTRENEVNT1_FUNCTION_MASK                                0x0001C000
#define    ETMR4_CNTRENEVNT1_RESOURCEB_SHIFT                              7
#define    ETMR4_CNTRENEVNT1_RESOURCEB_MASK                               0x00003F80
#define    ETMR4_CNTRENEVNT1_RESOURCEA_SHIFT                              0
#define    ETMR4_CNTRENEVNT1_RESOURCEA_MASK                               0x0000007F

#define ETMR4_CNTRENEVNT2_OFFSET                                          0x00000154
#define ETMR4_CNTRENEVNT2_TYPE                                            UInt32
#define ETMR4_CNTRENEVNT2_RESERVED_MASK                                   0xFFFE0000
#define    ETMR4_CNTRENEVNT2_FUNCTION_SHIFT                               14
#define    ETMR4_CNTRENEVNT2_FUNCTION_MASK                                0x0001C000
#define    ETMR4_CNTRENEVNT2_RESOURCEB_SHIFT                              7
#define    ETMR4_CNTRENEVNT2_RESOURCEB_MASK                               0x00003F80
#define    ETMR4_CNTRENEVNT2_RESOURCEA_SHIFT                              0
#define    ETMR4_CNTRENEVNT2_RESOURCEA_MASK                               0x0000007F

#define ETMR4_CNTRRELDEVNT1_OFFSET                                        0x00000160
#define ETMR4_CNTRRELDEVNT1_TYPE                                          UInt32
#define ETMR4_CNTRRELDEVNT1_RESERVED_MASK                                 0xFFFE0000
#define    ETMR4_CNTRRELDEVNT1_FUNCTION_SHIFT                             14
#define    ETMR4_CNTRRELDEVNT1_FUNCTION_MASK                              0x0001C000
#define    ETMR4_CNTRRELDEVNT1_RESOURCEB_SHIFT                            7
#define    ETMR4_CNTRRELDEVNT1_RESOURCEB_MASK                             0x00003F80
#define    ETMR4_CNTRRELDEVNT1_RESOURCEA_SHIFT                            0
#define    ETMR4_CNTRRELDEVNT1_RESOURCEA_MASK                             0x0000007F

#define ETMR4_CNTRRELDEVNT2_OFFSET                                        0x00000164
#define ETMR4_CNTRRELDEVNT2_TYPE                                          UInt32
#define ETMR4_CNTRRELDEVNT2_RESERVED_MASK                                 0xFFFE0000
#define    ETMR4_CNTRRELDEVNT2_FUNCTION_SHIFT                             14
#define    ETMR4_CNTRRELDEVNT2_FUNCTION_MASK                              0x0001C000
#define    ETMR4_CNTRRELDEVNT2_RESOURCEB_SHIFT                            7
#define    ETMR4_CNTRRELDEVNT2_RESOURCEB_MASK                             0x00003F80
#define    ETMR4_CNTRRELDEVNT2_RESOURCEA_SHIFT                            0
#define    ETMR4_CNTRRELDEVNT2_RESOURCEA_MASK                             0x0000007F

#define ETMR4_CNTRVAL1_OFFSET                                             0x00000170
#define ETMR4_CNTRVAL1_TYPE                                               UInt32
#define ETMR4_CNTRVAL1_RESERVED_MASK                                      0xFFFF0000
#define    ETMR4_CNTRVAL1_COUNTERVAL_SHIFT                                0
#define    ETMR4_CNTRVAL1_COUNTERVAL_MASK                                 0x0000FFFF

#define ETMR4_CNTRVAL2_OFFSET                                             0x00000174
#define ETMR4_CNTRVAL2_TYPE                                               UInt32
#define ETMR4_CNTRVAL2_RESERVED_MASK                                      0xFFFF0000
#define    ETMR4_CNTRVAL2_COUNTERVAL_SHIFT                                0
#define    ETMR4_CNTRVAL2_COUNTERVAL_MASK                                 0x0000FFFF

#define ETMR4_SEQSTATEXEVNTS1_OFFSET                                      0x00000180
#define ETMR4_SEQSTATEXEVNTS1_TYPE                                        UInt32
#define ETMR4_SEQSTATEXEVNTS1_RESERVED_MASK                               0xFFFE0000
#define    ETMR4_SEQSTATEXEVNTS1_FUNCTION_SHIFT                           14
#define    ETMR4_SEQSTATEXEVNTS1_FUNCTION_MASK                            0x0001C000
#define    ETMR4_SEQSTATEXEVNTS1_RESOURCEB_SHIFT                          7
#define    ETMR4_SEQSTATEXEVNTS1_RESOURCEB_MASK                           0x00003F80
#define    ETMR4_SEQSTATEXEVNTS1_RESOURCEA_SHIFT                          0
#define    ETMR4_SEQSTATEXEVNTS1_RESOURCEA_MASK                           0x0000007F

#define ETMR4_SEQSTATEXEVNTS2_OFFSET                                      0x00000184
#define ETMR4_SEQSTATEXEVNTS2_TYPE                                        UInt32
#define ETMR4_SEQSTATEXEVNTS2_RESERVED_MASK                               0xFFFE0000
#define    ETMR4_SEQSTATEXEVNTS2_FUNCTION_SHIFT                           14
#define    ETMR4_SEQSTATEXEVNTS2_FUNCTION_MASK                            0x0001C000
#define    ETMR4_SEQSTATEXEVNTS2_RESOURCEB_SHIFT                          7
#define    ETMR4_SEQSTATEXEVNTS2_RESOURCEB_MASK                           0x00003F80
#define    ETMR4_SEQSTATEXEVNTS2_RESOURCEA_SHIFT                          0
#define    ETMR4_SEQSTATEXEVNTS2_RESOURCEA_MASK                           0x0000007F

#define ETMR4_SEQSTATEXEVNTS3_OFFSET                                      0x00000188
#define ETMR4_SEQSTATEXEVNTS3_TYPE                                        UInt32
#define ETMR4_SEQSTATEXEVNTS3_RESERVED_MASK                               0xFFFE0000
#define    ETMR4_SEQSTATEXEVNTS3_FUNCTION_SHIFT                           14
#define    ETMR4_SEQSTATEXEVNTS3_FUNCTION_MASK                            0x0001C000
#define    ETMR4_SEQSTATEXEVNTS3_RESOURCEB_SHIFT                          7
#define    ETMR4_SEQSTATEXEVNTS3_RESOURCEB_MASK                           0x00003F80
#define    ETMR4_SEQSTATEXEVNTS3_RESOURCEA_SHIFT                          0
#define    ETMR4_SEQSTATEXEVNTS3_RESOURCEA_MASK                           0x0000007F

#define ETMR4_SEQSTATEXEVNTS4_OFFSET                                      0x0000018C
#define ETMR4_SEQSTATEXEVNTS4_TYPE                                        UInt32
#define ETMR4_SEQSTATEXEVNTS4_RESERVED_MASK                               0xFFFE0000
#define    ETMR4_SEQSTATEXEVNTS4_FUNCTION_SHIFT                           14
#define    ETMR4_SEQSTATEXEVNTS4_FUNCTION_MASK                            0x0001C000
#define    ETMR4_SEQSTATEXEVNTS4_RESOURCEB_SHIFT                          7
#define    ETMR4_SEQSTATEXEVNTS4_RESOURCEB_MASK                           0x00003F80
#define    ETMR4_SEQSTATEXEVNTS4_RESOURCEA_SHIFT                          0
#define    ETMR4_SEQSTATEXEVNTS4_RESOURCEA_MASK                           0x0000007F

#define ETMR4_SEQSTATEXEVNTS5_OFFSET                                      0x00000190
#define ETMR4_SEQSTATEXEVNTS5_TYPE                                        UInt32
#define ETMR4_SEQSTATEXEVNTS5_RESERVED_MASK                               0xFFFE0000
#define    ETMR4_SEQSTATEXEVNTS5_FUNCTION_SHIFT                           14
#define    ETMR4_SEQSTATEXEVNTS5_FUNCTION_MASK                            0x0001C000
#define    ETMR4_SEQSTATEXEVNTS5_RESOURCEB_SHIFT                          7
#define    ETMR4_SEQSTATEXEVNTS5_RESOURCEB_MASK                           0x00003F80
#define    ETMR4_SEQSTATEXEVNTS5_RESOURCEA_SHIFT                          0
#define    ETMR4_SEQSTATEXEVNTS5_RESOURCEA_MASK                           0x0000007F

#define ETMR4_SEQSTATEXEVNTS6_OFFSET                                      0x00000194
#define ETMR4_SEQSTATEXEVNTS6_TYPE                                        UInt32
#define ETMR4_SEQSTATEXEVNTS6_RESERVED_MASK                               0xFFFE0000
#define    ETMR4_SEQSTATEXEVNTS6_FUNCTION_SHIFT                           14
#define    ETMR4_SEQSTATEXEVNTS6_FUNCTION_MASK                            0x0001C000
#define    ETMR4_SEQSTATEXEVNTS6_RESOURCEB_SHIFT                          7
#define    ETMR4_SEQSTATEXEVNTS6_RESOURCEB_MASK                           0x00003F80
#define    ETMR4_SEQSTATEXEVNTS6_RESOURCEA_SHIFT                          0
#define    ETMR4_SEQSTATEXEVNTS6_RESOURCEA_MASK                           0x0000007F

#define ETMR4_SEQSTATE_OFFSET                                             0x0000019C
#define ETMR4_SEQSTATE_TYPE                                               UInt32
#define ETMR4_SEQSTATE_RESERVED_MASK                                      0xFFFFFFFC
#define    ETMR4_SEQSTATE_CURRSEQSTATE_SHIFT                              0
#define    ETMR4_SEQSTATE_CURRSEQSTATE_MASK                               0x00000003

#define ETMR4_EXTOUTEVNT1_OFFSET                                          0x000001A0
#define ETMR4_EXTOUTEVNT1_TYPE                                            UInt32
#define ETMR4_EXTOUTEVNT1_RESERVED_MASK                                   0xFFFE0000
#define    ETMR4_EXTOUTEVNT1_FUNCTION_SHIFT                               14
#define    ETMR4_EXTOUTEVNT1_FUNCTION_MASK                                0x0001C000
#define    ETMR4_EXTOUTEVNT1_RESOURCEB_SHIFT                              7
#define    ETMR4_EXTOUTEVNT1_RESOURCEB_MASK                               0x00003F80
#define    ETMR4_EXTOUTEVNT1_RESOURCEA_SHIFT                              0
#define    ETMR4_EXTOUTEVNT1_RESOURCEA_MASK                               0x0000007F

#define ETMR4_EXTOUTEVNT2_OFFSET                                          0x000001A4
#define ETMR4_EXTOUTEVNT2_TYPE                                            UInt32
#define ETMR4_EXTOUTEVNT2_RESERVED_MASK                                   0xFFFE0000
#define    ETMR4_EXTOUTEVNT2_FUNCTION_SHIFT                               14
#define    ETMR4_EXTOUTEVNT2_FUNCTION_MASK                                0x0001C000
#define    ETMR4_EXTOUTEVNT2_RESOURCEB_SHIFT                              7
#define    ETMR4_EXTOUTEVNT2_RESOURCEB_MASK                               0x00003F80
#define    ETMR4_EXTOUTEVNT2_RESOURCEA_SHIFT                              0
#define    ETMR4_EXTOUTEVNT2_RESOURCEA_MASK                               0x0000007F

#define ETMR4_CIDCMPVAL_OFFSET                                            0x000001B0
#define ETMR4_CIDCMPVAL_TYPE                                              UInt32
#define ETMR4_CIDCMPVAL_RESERVED_MASK                                     0x00000000
#define    ETMR4_CIDCMPVAL_CIDVAL_SHIFT                                   0
#define    ETMR4_CIDCMPVAL_CIDVAL_MASK                                    0xFFFFFFFF

#define ETMR4_CIDCMPMASK_OFFSET                                           0x000001BC
#define ETMR4_CIDCMPMASK_TYPE                                             UInt32
#define ETMR4_CIDCMPMASK_RESERVED_MASK                                    0x00000000
#define    ETMR4_CIDCMPMASK_CIDMASK_SHIFT                                 0
#define    ETMR4_CIDCMPMASK_CIDMASK_MASK                                  0xFFFFFFFF

#define ETMR4_SYNCFREQ_OFFSET                                             0x000001E0
#define ETMR4_SYNCFREQ_TYPE                                               UInt32
#define ETMR4_SYNCFREQ_RESERVED_MASK                                      0xFFFFF000
#define    ETMR4_SYNCFREQ_SYNCFREQ_SHIFT                                  0
#define    ETMR4_SYNCFREQ_SYNCFREQ_MASK                                   0x00000FFF

#define ETMR4_ETMID_OFFSET                                                0x000001E4
#define ETMR4_ETMID_TYPE                                                  UInt32
#define ETMR4_ETMID_RESERVED_MASK                                         0x00F20000
#define    ETMR4_ETMID_IMPLEMENTOR_SHIFT                                  24
#define    ETMR4_ETMID_IMPLEMENTOR_MASK                                   0xFF000000
#define    ETMR4_ETMID_SECEXTSUPP_SHIFT                                   19
#define    ETMR4_ETMID_SECEXTSUPP_MASK                                    0x00080000
#define    ETMR4_ETMID_THUMB2SUPP_SHIFT                                   18
#define    ETMR4_ETMID_THUMB2SUPP_MASK                                    0x00040000
#define    ETMR4_ETMID_LOADPCFIRST_SHIFT                                  16
#define    ETMR4_ETMID_LOADPCFIRST_MASK                                   0x00010000
#define    ETMR4_ETMID_ARMCPUFAMILY_SHIFT                                 12
#define    ETMR4_ETMID_ARMCPUFAMILY_MASK                                  0x0000F000
#define    ETMR4_ETMID_MAJORETMVER_SHIFT                                  8
#define    ETMR4_ETMID_MAJORETMVER_MASK                                   0x00000F00
#define    ETMR4_ETMID_MINORETMVER_SHIFT                                  4
#define    ETMR4_ETMID_MINORETMVER_MASK                                   0x000000F0
#define    ETMR4_ETMID_IMPVER_SHIFT                                       0
#define    ETMR4_ETMID_IMPVER_MASK                                        0x0000000F

#define ETMR4_CFGCODEEXT_OFFSET                                           0x000001E8
#define ETMR4_CFGCODEEXT_TYPE                                             UInt32
#define ETMR4_CFGCODEEXT_RESERVED_MASK                                    0xFFFFF000
#define    ETMR4_CFGCODEEXT_ALLREGSREAD_SHIFT                             11
#define    ETMR4_CFGCODEEXT_ALLREGSREAD_MASK                              0x00000800
#define    ETMR4_CFGCODEEXT_SIZEOFEXTINS_SHIFT                            3
#define    ETMR4_CFGCODEEXT_SIZEOFEXTINS_MASK                             0x000007F8
#define    ETMR4_CFGCODEEXT_NUMEXTINSELS_SHIFT                            0
#define    ETMR4_CFGCODEEXT_NUMEXTINSELS_MASK                             0x00000007

#define ETMR4_EXTXTRNLINSEL_OFFSET                                        0x000001EC
#define ETMR4_EXTXTRNLINSEL_TYPE                                          UInt32
#define ETMR4_EXTXTRNLINSEL_RESERVED_MASK                                 0xFFFFC0C0
#define    ETMR4_EXTXTRNLINSEL_EXT2INSEL_SHIFT                            8
#define    ETMR4_EXTXTRNLINSEL_EXT2INSEL_MASK                             0x00003F00
#define    ETMR4_EXTXTRNLINSEL_EXTINSEL_SHIFT                             0
#define    ETMR4_EXTXTRNLINSEL_EXTINSEL_MASK                              0x0000003F

#define ETMR4_CORESIGHTTRID_OFFSET                                        0x00000200
#define ETMR4_CORESIGHTTRID_TYPE                                          UInt32
#define ETMR4_CORESIGHTTRID_RESERVED_MASK                                 0xFFFFFF80
#define    ETMR4_CORESIGHTTRID_TRACEID_SHIFT                              0
#define    ETMR4_CORESIGHTTRID_TRACEID_MASK                               0x0000007F

#define ETMR4_PWRDWNSTAT_OFFSET                                           0x00000314
#define ETMR4_PWRDWNSTAT_TYPE                                             UInt32
#define ETMR4_PWRDWNSTAT_RESERVED_MASK                                    0xFFFFFFFC
#define    ETMR4_PWRDWNSTAT_STCKYREGSTATE_SHIFT                           1
#define    ETMR4_PWRDWNSTAT_STCKYREGSTATE_MASK                            0x00000002
#define    ETMR4_PWRDWNSTAT_ETMPWRDUP_SHIFT                               0
#define    ETMR4_PWRDWNSTAT_ETMPWRDUP_MASK                                0x00000001

#define ETMR4_ITETMIF_OFFSET                                              0x00000ED8
#define ETMR4_ITETMIF_TYPE                                                UInt32
#define ETMR4_ITETMIF_RESERVED_MASK                                       0xFFFF8000
#define    ETMR4_ITETMIF_EVNTBUS46_SHIFT                                  14
#define    ETMR4_ITETMIF_EVNTBUS46_MASK                                   0x00004000
#define    ETMR4_ITETMIF_EVNTBUS28_SHIFT                                  13
#define    ETMR4_ITETMIF_EVNTBUS28_MASK                                   0x00002000
#define    ETMR4_ITETMIF_EVNTBUS0_SHIFT                                   12
#define    ETMR4_ITETMIF_EVNTBUS0_MASK                                    0x00001000
#define    ETMR4_ITETMIF_ETMCID31_SHIFT                                   11
#define    ETMR4_ITETMIF_ETMCID31_MASK                                    0x00000800
#define    ETMR4_ITETMIF_ETMCID0_SHIFT                                    10
#define    ETMR4_ITETMIF_ETMCID0_MASK                                     0x00000400
#define    ETMR4_ITETMIF_ETMDD63_SHIFT                                    9
#define    ETMR4_ITETMIF_ETMDD63_MASK                                     0x00000200
#define    ETMR4_ITETMIF_ETMDD0_SHIFT                                     8
#define    ETMR4_ITETMIF_ETMDD0_MASK                                      0x00000100
#define    ETMR4_ITETMIF_ETMDA31_SHIFT                                    7
#define    ETMR4_ITETMIF_ETMDA31_MASK                                     0x00000080
#define    ETMR4_ITETMIF_ETMDA0_SHIFT                                     6
#define    ETMR4_ITETMIF_ETMDA0_MASK                                      0x00000040
#define    ETMR4_ITETMIF_ETMDCTL11_SHIFT                                  5
#define    ETMR4_ITETMIF_ETMDCTL11_MASK                                   0x00000020
#define    ETMR4_ITETMIF_ETMDCTL0_SHIFT                                   4
#define    ETMR4_ITETMIF_ETMDCTL0_MASK                                    0x00000010
#define    ETMR4_ITETMIF_ETMIA31_SHIFT                                    3
#define    ETMR4_ITETMIF_ETMIA31_MASK                                     0x00000008
#define    ETMR4_ITETMIF_ETMIA1_SHIFT                                     2
#define    ETMR4_ITETMIF_ETMIA1_MASK                                      0x00000004
#define    ETMR4_ITETMIF_ETMICTL13_SHIFT                                  1
#define    ETMR4_ITETMIF_ETMICTL13_MASK                                   0x00000002
#define    ETMR4_ITETMIF_ETMICTL0_SHIFT                                   0
#define    ETMR4_ITETMIF_ETMICTL0_MASK                                    0x00000001

#define ETMR4_ITMISCOUT_OFFSET                                            0x00000EDC
#define ETMR4_ITMISCOUT_TYPE                                              UInt32
#define ETMR4_ITMISCOUT_RESERVED_MASK                                     0xFFFFFCCF
#define    ETMR4_ITMISCOUT_EXTOUT_SHIFT                                   8
#define    ETMR4_ITMISCOUT_EXTOUT_MASK                                    0x00000300
#define    ETMR4_ITMISCOUT_ETMWFIREADY_SHIFT                              5
#define    ETMR4_ITMISCOUT_ETMWFIREADY_MASK                               0x00000020
#define    ETMR4_ITMISCOUT_ETMDBGRQ_SHIFT                                 4
#define    ETMR4_ITMISCOUT_ETMDBGRQ_MASK                                  0x00000010

#define ETMR4_ITMISCIN_OFFSET                                             0x00000EE0
#define ETMR4_ITMISCIN_TYPE                                               UInt32
#define ETMR4_ITMISCIN_RESERVED_MASK                                      0xFFFFFFC0
#define    ETMR4_ITMISCIN_ETMWFIPENDING_SHIFT                             5
#define    ETMR4_ITMISCIN_ETMWFIPENDING_MASK                              0x00000020
#define    ETMR4_ITMISCIN_DBGACK_SHIFT                                    4
#define    ETMR4_ITMISCIN_DBGACK_MASK                                     0x00000010
#define    ETMR4_ITMISCIN_EXTIN_SHIFT                                     0
#define    ETMR4_ITMISCIN_EXTIN_MASK                                      0x0000000F

#define ETMR4_ITTRIGGERACK_OFFSET                                         0x00000EE4
#define ETMR4_ITTRIGGERACK_TYPE                                           UInt32
#define ETMR4_ITTRIGGERACK_RESERVED_MASK                                  0xFFFFFFFE
#define    ETMR4_ITTRIGGERACK_TRIGGERACK_SHIFT                            0
#define    ETMR4_ITTRIGGERACK_TRIGGERACK_MASK                             0x00000001

#define ETMR4_ITTRIGGERREQ_OFFSET                                         0x00000EE8
#define ETMR4_ITTRIGGERREQ_TYPE                                           UInt32
#define ETMR4_ITTRIGGERREQ_RESERVED_MASK                                  0xFFFFFFFE
#define    ETMR4_ITTRIGGERREQ_TRIGGER_SHIFT                               0
#define    ETMR4_ITTRIGGERREQ_TRIGGER_MASK                                0x00000001

#define ETMR4_ITATBDATA0_OFFSET                                           0x00000EEC
#define ETMR4_ITATBDATA0_TYPE                                             UInt32
#define ETMR4_ITATBDATA0_RESERVED_MASK                                    0xFFFFFFE0
#define    ETMR4_ITATBDATA0_ATDATA_SHIFT                                  0
#define    ETMR4_ITATBDATA0_ATDATA_MASK                                   0x0000001F

#define ETMR4_ITATBCTR2_OFFSET                                            0x00000EF0
#define ETMR4_ITATBCTR2_TYPE                                              UInt32
#define ETMR4_ITATBCTR2_RESERVED_MASK                                     0xFFFFFFFC
#define    ETMR4_ITATBCTR2_AFVALID_SHIFT                                  1
#define    ETMR4_ITATBCTR2_AFVALID_MASK                                   0x00000002
#define    ETMR4_ITATBCTR2_ATREADY_SHIFT                                  0
#define    ETMR4_ITATBCTR2_ATREADY_MASK                                   0x00000001

#define ETMR4_ITATBCTR1_OFFSET                                            0x00000EF4
#define ETMR4_ITATBCTR1_TYPE                                              UInt32
#define ETMR4_ITATBCTR1_RESERVED_MASK                                     0xFFFFFF80
#define    ETMR4_ITATBCTR1_ATID_SHIFT                                     0
#define    ETMR4_ITATBCTR1_ATID_MASK                                      0x0000007F

#define ETMR4_ITATBCTR0_OFFSET                                            0x00000EF8
#define ETMR4_ITATBCTR0_TYPE                                              UInt32
#define ETMR4_ITATBCTR0_RESERVED_MASK                                     0xFFFFFCFC
#define    ETMR4_ITATBCTR0_ATBYTES_SHIFT                                  8
#define    ETMR4_ITATBCTR0_ATBYTES_MASK                                   0x00000300
#define    ETMR4_ITATBCTR0_AFREADY_SHIFT                                  1
#define    ETMR4_ITATBCTR0_AFREADY_MASK                                   0x00000002
#define    ETMR4_ITATBCTR0_ATVALID_SHIFT                                  0
#define    ETMR4_ITATBCTR0_ATVALID_MASK                                   0x00000001

#define ETMR4_INTMODECTL_OFFSET                                           0x00000F00
#define ETMR4_INTMODECTL_TYPE                                             UInt32
#define ETMR4_INTMODECTL_RESERVED_MASK                                    0xFFFFFFFE
#define    ETMR4_INTMODECTL_INTMODEEN_SHIFT                               0
#define    ETMR4_INTMODECTL_INTMODEEN_MASK                                0x00000001

#define ETMR4_CLAIMSET_OFFSET                                             0x00000FA0
#define ETMR4_CLAIMSET_TYPE                                               UInt32
#define ETMR4_CLAIMSET_RESERVED_MASK                                      0xFFFFFF00
#define    ETMR4_CLAIMSET_CLAIMTAGSET_SHIFT                               0
#define    ETMR4_CLAIMSET_CLAIMTAGSET_MASK                                0x000000FF

#define ETMR4_CLAIMCLR_OFFSET                                             0x00000FA4
#define ETMR4_CLAIMCLR_TYPE                                               UInt32
#define ETMR4_CLAIMCLR_RESERVED_MASK                                      0xFFFFFF00
#define    ETMR4_CLAIMCLR_CLAIMTAGCLEAR_SHIFT                             0
#define    ETMR4_CLAIMCLR_CLAIMTAGCLEAR_MASK                              0x000000FF

#define ETMR4_LOCKACCESS_OFFSET                                           0x00000FB0
#define ETMR4_LOCKACCESS_TYPE                                             UInt32
#define ETMR4_LOCKACCESS_RESERVED_MASK                                    0x00000000
#define    ETMR4_LOCKACCESS_KEY_SHIFT                                     0
#define    ETMR4_LOCKACCESS_KEY_MASK                                      0xFFFFFFFF

#define ETMR4_LOCKSTATUS_OFFSET                                           0x00000FB4
#define ETMR4_LOCKSTATUS_TYPE                                             UInt32
#define ETMR4_LOCKSTATUS_RESERVED_MASK                                    0xFFFFFFFC
#define    ETMR4_LOCKSTATUS_ETMLOCKED_SHIFT                               1
#define    ETMR4_LOCKSTATUS_ETMLOCKED_MASK                                0x00000002
#define    ETMR4_LOCKSTATUS_LOCKIMP_SHIFT                                 0
#define    ETMR4_LOCKSTATUS_LOCKIMP_MASK                                  0x00000001

#define ETMR4_AUTHSTATUS_OFFSET                                           0x00000FB8
#define ETMR4_AUTHSTATUS_TYPE                                             UInt32
#define ETMR4_AUTHSTATUS_RESERVED_MASK                                    0xFFFFFF00
#define    ETMR4_AUTHSTATUS_SECNINVDEBUG_SHIFT                            6
#define    ETMR4_AUTHSTATUS_SECNINVDEBUG_MASK                             0x000000C0
#define    ETMR4_AUTHSTATUS_SECINVDEBUG_SHIFT                             4
#define    ETMR4_AUTHSTATUS_SECINVDEBUG_MASK                              0x00000030
#define    ETMR4_AUTHSTATUS_NSECNINVDEBUG_SHIFT                           2
#define    ETMR4_AUTHSTATUS_NSECNINVDEBUG_MASK                            0x0000000C
#define    ETMR4_AUTHSTATUS_NSECINVDEBUG_SHIFT                            0
#define    ETMR4_AUTHSTATUS_NSECINVDEBUG_MASK                             0x00000003

#define ETMR4_DEVID_OFFSET                                                0x00000FC8
#define ETMR4_DEVID_TYPE                                                  UInt32
#define ETMR4_DEVID_RESERVED_MASK                                         0xFFFFFFFF

#define ETMR4_DEVTYPE_OFFSET                                              0x00000FCC
#define ETMR4_DEVTYPE_TYPE                                                UInt32
#define ETMR4_DEVTYPE_RESERVED_MASK                                       0xFFFFFF00
#define    ETMR4_DEVTYPE_CPUTRACE_SHIFT                                   4
#define    ETMR4_DEVTYPE_CPUTRACE_MASK                                    0x000000F0
#define    ETMR4_DEVTYPE_TRACESOURCE_SHIFT                                0
#define    ETMR4_DEVTYPE_TRACESOURCE_MASK                                 0x0000000F

#define ETMR4_PERIPHID4_OFFSET                                            0x00000FD0
#define ETMR4_PERIPHID4_TYPE                                              UInt32
#define ETMR4_PERIPHID4_RESERVED_MASK                                     0xFFFFFF00
#define    ETMR4_PERIPHID4_FOURKBBLOCKS_SHIFT                             4
#define    ETMR4_PERIPHID4_FOURKBBLOCKS_MASK                              0x000000F0
#define    ETMR4_PERIPHID4_JEP106CNTCODE_SHIFT                            0
#define    ETMR4_PERIPHID4_JEP106CNTCODE_MASK                             0x0000000F

#define ETMR4_PERIPHID5_OFFSET                                            0x00000FD4
#define ETMR4_PERIPHID5_TYPE                                              UInt32
#define ETMR4_PERIPHID5_RESERVED_MASK                                     0xFFFFFFFF

#define ETMR4_PERIPHID6_OFFSET                                            0x00000FD8
#define ETMR4_PERIPHID6_TYPE                                              UInt32
#define ETMR4_PERIPHID6_RESERVED_MASK                                     0xFFFFFFFF

#define ETMR4_PERIPHID7_OFFSET                                            0x00000FDC
#define ETMR4_PERIPHID7_TYPE                                              UInt32
#define ETMR4_PERIPHID7_RESERVED_MASK                                     0xFFFFFFFF

#define ETMR4_PERIPHID0_OFFSET                                            0x00000FE0
#define ETMR4_PERIPHID0_TYPE                                              UInt32
#define ETMR4_PERIPHID0_RESERVED_MASK                                     0xFFFFFF00
#define    ETMR4_PERIPHID0_PARTNUM_SHIFT                                  0
#define    ETMR4_PERIPHID0_PARTNUM_MASK                                   0x000000FF

#define ETMR4_PERIPHID1_OFFSET                                            0x00000FE4
#define ETMR4_PERIPHID1_TYPE                                              UInt32
#define ETMR4_PERIPHID1_RESERVED_MASK                                     0xFFFFFF00
#define    ETMR4_PERIPHID1_JEP106IDCODE_SHIFT                             4
#define    ETMR4_PERIPHID1_JEP106IDCODE_MASK                              0x000000F0
#define    ETMR4_PERIPHID1_PARTNUM_SHIFT                                  0
#define    ETMR4_PERIPHID1_PARTNUM_MASK                                   0x0000000F

#define ETMR4_PERIPHID2_OFFSET                                            0x00000FE8
#define ETMR4_PERIPHID2_TYPE                                              UInt32
#define ETMR4_PERIPHID2_RESERVED_MASK                                     0xFFFFFF00
#define    ETMR4_PERIPHID2_REVNUM_SHIFT                                   4
#define    ETMR4_PERIPHID2_REVNUM_MASK                                    0x000000F0
#define    ETMR4_PERIPHID2_JEDECASSGN_SHIFT                               3
#define    ETMR4_PERIPHID2_JEDECASSGN_MASK                                0x00000008
#define    ETMR4_PERIPHID2_JEP106IDCODE_SHIFT                             0
#define    ETMR4_PERIPHID2_JEP106IDCODE_MASK                              0x00000007

#define ETMR4_PERIPHID3_OFFSET                                            0x00000FEC
#define ETMR4_PERIPHID3_TYPE                                              UInt32
#define ETMR4_PERIPHID3_RESERVED_MASK                                     0xFFFFFF00
#define    ETMR4_PERIPHID3_REVAND_SHIFT                                   4
#define    ETMR4_PERIPHID3_REVAND_MASK                                    0x000000F0
#define    ETMR4_PERIPHID3_CUSTMOD_SHIFT                                  0
#define    ETMR4_PERIPHID3_CUSTMOD_MASK                                   0x0000000F

#define ETMR4_COMPID0_OFFSET                                              0x00000FF0
#define ETMR4_COMPID0_TYPE                                                UInt32
#define ETMR4_COMPID0_RESERVED_MASK                                       0xFFFFFF00
#define    ETMR4_COMPID0_COMPONENTID_SHIFT                                0
#define    ETMR4_COMPID0_COMPONENTID_MASK                                 0x000000FF

#define ETMR4_COMPID1_OFFSET                                              0x00000FF4
#define ETMR4_COMPID1_TYPE                                                UInt32
#define ETMR4_COMPID1_RESERVED_MASK                                       0xFFFFFF00
#define    ETMR4_COMPID1_COMPONENTID_SHIFT                                0
#define    ETMR4_COMPID1_COMPONENTID_MASK                                 0x000000FF

#define ETMR4_COMPID2_OFFSET                                              0x00000FF8
#define ETMR4_COMPID2_TYPE                                                UInt32
#define ETMR4_COMPID2_RESERVED_MASK                                       0xFFFFFF00
#define    ETMR4_COMPID2_COMPONENTID_SHIFT                                0
#define    ETMR4_COMPID2_COMPONENTID_MASK                                 0x000000FF

#define ETMR4_COMPID3_OFFSET                                              0x00000FFC
#define ETMR4_COMPID3_TYPE                                                UInt32
#define ETMR4_COMPID3_RESERVED_MASK                                       0xFFFFFF00
#define    ETMR4_COMPID3_COMPONENTID_SHIFT                                0
#define    ETMR4_COMPID3_COMPONENTID_MASK                                 0x000000FF

#endif /* __BRCM_RDB_ETMR4_H__ */


