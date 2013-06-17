/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2012  Broadcom Corporation                                                        */
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
/*     Date     : Generated on 9/25/2012 19:15:19                                             */
/*     RDB file : //HAWAII/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_DSP_AC_H__
#define __BRCM_RDB_DSP_AC_H__

#define DSP_AC_AMPCR_OFFSET                                               0x0000E546
#define DSP_AC_AMPCR_TYPE                                                 UInt16
#define DSP_AC_AMPCR_RESERVED_MASK                                        0x00007FFB
#define    DSP_AC_AMPCR_PWDAPCG1_SHIFT                                    15
#define    DSP_AC_AMPCR_PWDAPCG1_MASK                                     0x00008000
#define    DSP_AC_AMPCR_PWDAPCG0_SHIFT                                    2
#define    DSP_AC_AMPCR_PWDAPCG0_MASK                                     0x00000004

#define DSP_AC_FRQOR_OFFSET                                               0x0000E573
#define DSP_AC_FRQOR_TYPE                                                 UInt16
#define DSP_AC_FRQOR_RESERVED_MASK                                        0x00007C00
#define    DSP_AC_FRQOR_FRQPEN_SHIFT                                      15
#define    DSP_AC_FRQOR_FRQPEN_MASK                                       0x00008000
#define    DSP_AC_FRQOR_FRQOR_SHIFT                                       0
#define    DSP_AC_FRQOR_FRQOR_MASK                                        0x000003FF

#define DSP_AC_AMPCR_R_OFFSET                                             0x00000A8C
#define DSP_AC_AMPCR_R_TYPE                                               UInt16
#define DSP_AC_AMPCR_R_RESERVED_MASK                                      0x00007FFB
#define    DSP_AC_AMPCR_R_PWDAPCG1_SHIFT                                  15
#define    DSP_AC_AMPCR_R_PWDAPCG1_MASK                                   0x00008000
#define    DSP_AC_AMPCR_R_PWDAPCG0_SHIFT                                  2
#define    DSP_AC_AMPCR_R_PWDAPCG0_MASK                                   0x00000004

#define DSP_AC_FRQOR_R_OFFSET                                             0x00000AE6
#define DSP_AC_FRQOR_R_TYPE                                               UInt16
#define DSP_AC_FRQOR_R_RESERVED_MASK                                      0x00007C00
#define    DSP_AC_FRQOR_R_FRQPEN_SHIFT                                    15
#define    DSP_AC_FRQOR_R_FRQPEN_MASK                                     0x00008000
#define    DSP_AC_FRQOR_R_FRQOR_SHIFT                                     0
#define    DSP_AC_FRQOR_R_FRQOR_MASK                                      0x000003FF

#endif /* __BRCM_RDB_DSP_AC_H__ */


