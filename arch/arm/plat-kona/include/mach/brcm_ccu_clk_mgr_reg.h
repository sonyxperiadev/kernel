/************************************************************************************************/
/*		                                                                                          */
/*  Copyright 2011  Broadcom Corporation		                                                    */
/*		                                                                                          */
/*		 Unless you and Broadcom execute a separate written software license agreement governing  */
/*		 use of this software, this software is licensed to you under the terms of the GNU        */
/*		 General Public License version 2 (the GPL), available at                                 */
/*		                                                                                          */
/*		      http://www.broadcom.com/licenses/GPLv2.php                                          */
/*		                                                                                          */
/*		 with the following added to such license:                                                */
/*		                                                                                          */
/*		 As a special exception, the copyright holders of this software give you permission to    */
/*		 link this software with independent modules, and to copy and distribute the resulting    */
/*		 executable under terms of your choice, provided that you also meet, for each linked      */
/*		 independent module, the terms and conditions of the license of that module.              */
/*		 An independent module is a module which is not derived from this software.  The special  */
/*		 exception does not apply to any modifications of the software.                           */
/*		                                                                                          */
/*		 Notwithstanding the above, under no circumstances may you combine this software in any   */
/*		 way with any other Broadcom software provided under a license other than the GPL,        */
/*		 without Broadcom's express prior written consent.                                        */
/*		                                                                                          */
/*		 Date     : Generated on 1/7/2011 14:24:48                                             */
/*		 RDB file : //RHEA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_CCU_CLK_MGR_REG_H__
#define __BRCM_CCU_CLK_MGR_REG_H__

#define		CCU_CLK_MGR_REG_WR_ACCESS_OFFSET					0x00000000
#define		CCU_CLK_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_SHIFT			31
#define		CCU_CLK_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_MASK		        	0x80000000
#define		CCU_CLK_MGR_REG_WR_ACCESS_PASSWORD_SHIFT		        	8
#define		CCU_CLK_MGR_REG_WR_ACCESS_PASSWORD_MASK		                    	0x00FFFF00
#define		CCU_CLK_MGR_REG_WR_ACCESS_CLKMGR_ACC_SHIFT		                0
#define		CCU_CLK_MGR_REG_WR_ACCESS_CLKMGR_ACC_MASK		                0x00000001

#define		CCU_CLK_MGR_REG_POLICY_FREQ_OFFSET		                        0x00000008
#define		CCU_CLK_MGR_REG_POLICY_FREQ_PRIV_ACCESS_MODE_SHIFT		        31
#define		CCU_CLK_MGR_REG_POLICY_FREQ_PRIV_ACCESS_MODE_MASK		        0x80000000
#define		CCU_CLK_MGR_REG_POLICY_FREQ_POLICY3_FREQ_SHIFT		             	24
#define		CCU_CLK_MGR_REG_POLICY_FREQ_POLICY3_FREQ_MASK		              	0x07000000
#define		CCU_CLK_MGR_REG_POLICY_FREQ_POLICY2_FREQ_SHIFT		             	16
#define		CCU_CLK_MGR_REG_POLICY_FREQ_POLICY2_FREQ_MASK		              	0x00070000
#define		CCU_CLK_MGR_REG_POLICY_FREQ_POLICY1_FREQ_SHIFT		             	8
#define		CCU_CLK_MGR_REG_POLICY_FREQ_POLICY1_FREQ_MASK		              	0x00000700
#define		CCU_CLK_MGR_REG_POLICY_FREQ_POLICY0_FREQ_SHIFT		             	0
#define		CCU_CLK_MGR_REG_POLICY_FREQ_POLICY0_FREQ_MASK		              	0x00000007

#define		CCU_CLK_MGR_REG_POLICY_CTL_OFFSET		                        0x0000000C
#define		CCU_CLK_MGR_REG_POLICY_CTL_PRIV_ACCESS_MODE_SHIFT		        31
#define		CCU_CLK_MGR_REG_POLICY_CTL_PRIV_ACCESS_MODE_MASK		        0x80000000
#define		CCU_CLK_MGR_REG_POLICY_CTL_TGT_VLD_SHIFT		                3
#define		CCU_CLK_MGR_REG_POLICY_CTL_TGT_VLD_MASK		                    	0x00000008
#define		CCU_CLK_MGR_REG_POLICY_CTL_GO_ATL_SHIFT		                    	2
#define		CCU_CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK		                     	0x00000004
#define		CCU_CLK_MGR_REG_POLICY_CTL_GO_AC_SHIFT		                     	1
#define		CCU_CLK_MGR_REG_POLICY_CTL_GO_AC_MASK		                      	0x00000002
#define		CCU_CLK_MGR_REG_POLICY_CTL_GO_SHIFT		                        0
#define		CCU_CLK_MGR_REG_POLICY_CTL_GO_MASK		                        0x00000001

#define		CCU_CLK_MGR_REG_POLICY0_MASK_OFFSET		                        0x00000010
#define		CCU_CLK_MGR_REG_POLICY0_MASK_PRIV_ACCESS_MODE_SHIFT		        31
#define		CCU_CLK_MGR_REG_POLICY0_MASK_PRIV_ACCESS_MODE_MASK		        0x80000000


#define		CCU_CLK_MGR_REG_POLICY1_MASK_OFFSET		                        0x00000014
#define		CCU_CLK_MGR_REG_POLICY1_MASK_PRIV_ACCESS_MODE_SHIFT		        31
#define		CCU_CLK_MGR_REG_POLICY1_MASK_PRIV_ACCESS_MODE_MASK		        0x80000000

#define		CCU_CLK_MGR_REG_POLICY2_MASK_OFFSET		                        0x00000018
#define		CCU_CLK_MGR_REG_POLICY2_MASK_PRIV_ACCESS_MODE_SHIFT		        31
#define		CCU_CLK_MGR_REG_POLICY2_MASK_PRIV_ACCESS_MODE_MASK		        0x80000000

#define		CCU_CLK_MGR_REG_POLICY3_MASK_OFFSET		                        0x0000001C
#define		CCU_CLK_MGR_REG_POLICY3_MASK_PRIV_ACCESS_MODE_SHIFT		        31
#define		CCU_CLK_MGR_REG_POLICY3_MASK_PRIV_ACCESS_MODE_MASK		        0x80000000

#define		CCU_CLK_MGR_REG_INTEN_OFFSET		                                0x00000020
#define		CCU_CLK_MGR_REG_INTEN_PRIV_ACCESS_MODE_SHIFT		               	31
#define		CCU_CLK_MGR_REG_INTEN_PRIV_ACCESS_MODE_MASK		                0x80000000
#define		CCU_CLK_MGR_REG_INTEN_INTEN_ACT_INT_EN_SHIFT		               	1
#define		CCU_CLK_MGR_REG_INTEN_INTEN_ACT_INT_EN_MASK		                0x00000002
#define		CCU_CLK_MGR_REG_INTEN_INTEN_TGT_INT_EN_SHIFT		               	0
#define		CCU_CLK_MGR_REG_INTEN_INTEN_TGT_INT_EN_MASK		                0x00000001

#define		CCU_CLK_MGR_REG_INTSTAT_OFFSET		                              	0x00000024
#define		CCU_CLK_MGR_REG_INTSTAT_PRIV_ACCESS_MODE_SHIFT		             	31
#define		CCU_CLK_MGR_REG_INTSTAT_PRIV_ACCESS_MODE_MASK		              	0x80000000
#define		CCU_CLK_MGR_REG_INTSTAT_INTSTAT_ACT_INT_SHIFT		              	1
#define		CCU_CLK_MGR_REG_INTSTAT_INTSTAT_ACT_INT_MASK		               	0x00000002
#define		CCU_CLK_MGR_REG_INTSTAT_INTSTAT_TGT_INT_SHIFT		              	0
#define		CCU_CLK_MGR_REG_INTSTAT_INTSTAT_TGT_INT_MASK		               	0x00000001

#define		CCU_CLK_MGR_REG_VLT_PERI_OFFSET		                             	0x00000030
#define		CCU_CLK_MGR_REG_VLT_PERI_PRIV_ACCESS_MODE_SHIFT		            	31
#define		CCU_CLK_MGR_REG_VLT_PERI_PRIV_ACCESS_MODE_MASK		             	0x80000000
#define		CCU_CLK_MGR_REG_VLT_PERI_VLT_HIGH_PERI_SHIFT		               	8
#define		CCU_CLK_MGR_REG_VLT_PERI_VLT_HIGH_PERI_MASK		                0x00000F00
#define		CCU_CLK_MGR_REG_VLT_PERI_VLT_NORMAL_PERI_SHIFT		             	0
#define		CCU_CLK_MGR_REG_VLT_PERI_VLT_NORMAL_PERI_MASK		              	0x0000000F

#define		CCU_CLK_MGR_REG_LVM_EN_OFFSET		                               	0x00000034
#define		CCU_CLK_MGR_REG_LVM_EN_PRIV_ACCESS_MODE_SHIFT		              	31
#define		CCU_CLK_MGR_REG_LVM_EN_PRIV_ACCESS_MODE_MASK		               	0x80000000
#define		CCU_CLK_MGR_REG_LVM_EN_POLICY_CONFIG_EN_SHIFT		              	0
#define		CCU_CLK_MGR_REG_LVM_EN_POLICY_CONFIG_EN_MASK		               	0x00000001

#define		CCU_CLK_MGR_REG_LVM0_3_OFFSET		                               	0x00000038
#define		CCU_CLK_MGR_REG_LVM0_3_PRIV_ACCESS_MODE_SHIFT		              	31
#define		CCU_CLK_MGR_REG_LVM0_3_PRIV_ACCESS_MODE_MASK		               	0x80000000
#define		CCU_CLK_MGR_REG_LVM0_3_LVM0_3_MD_03_SHIFT		                12
#define		CCU_CLK_MGR_REG_LVM0_3_LVM0_3_MD_03_MASK		                0x0000F000
#define		CCU_CLK_MGR_REG_LVM0_3_LVM0_3_MD_02_SHIFT		                8
#define		CCU_CLK_MGR_REG_LVM0_3_LVM0_3_MD_02_MASK		                0x00000F00
#define		CCU_CLK_MGR_REG_LVM0_3_LVM0_3_MD_01_SHIFT		                4
#define		CCU_CLK_MGR_REG_LVM0_3_LVM0_3_MD_01_MASK		                0x000000F0
#define		CCU_CLK_MGR_REG_LVM0_3_LVM0_3_MD_00_SHIFT		                0
#define		CCU_CLK_MGR_REG_LVM0_3_LVM0_3_MD_00_MASK		                0x0000000F

#define		CCU_CLK_MGR_REG_LVM4_7_OFFSET		                                0x0000003C
#define		CCU_CLK_MGR_REG_LVM4_7_PRIV_ACCESS_MODE_SHIFT		              	31
#define		CCU_CLK_MGR_REG_LVM4_7_PRIV_ACCESS_MODE_MASK		               	0x80000000
#define		CCU_CLK_MGR_REG_LVM4_7_LVM4_7_MD_07_SHIFT                      		12
#define		CCU_CLK_MGR_REG_LVM4_7_LVM4_7_MD_07_MASK                       		0x0000F000
#define		CCU_CLK_MGR_REG_LVM4_7_LVM4_7_MD_06_SHIFT                      		8
#define		CCU_CLK_MGR_REG_LVM4_7_LVM4_7_MD_06_MASK                       		0x00000F00
#define		CCU_CLK_MGR_REG_LVM4_7_LVM4_7_MD_05_SHIFT                      		4
#define		CCU_CLK_MGR_REG_LVM4_7_LVM4_7_MD_05_MASK                       		0x000000F0
#define		CCU_CLK_MGR_REG_LVM4_7_LVM4_7_MD_04_SHIFT                      		0
#define		CCU_CLK_MGR_REG_LVM4_7_LVM4_7_MD_04_MASK                       		0x0000000F

#define		CCU_CLK_MGR_REG_VLT0_3_OFFSET		                                0x00000040
#define		CCU_CLK_MGR_REG_VLT0_3_PRIV_ACCESS_MODE_SHIFT                  		31
#define		CCU_CLK_MGR_REG_VLT0_3_PRIV_ACCESS_MODE_MASK                   		0x80000000
#define		CCU_CLK_MGR_REG_VLT0_3_VLT0_3_VV_03_SHIFT                      		24
#define		CCU_CLK_MGR_REG_VLT0_3_VLT0_3_VV_03_MASK                       		0x0F000000
#define		CCU_CLK_MGR_REG_VLT0_3_VLT0_3_VV_02_SHIFT                      		16
#define		CCU_CLK_MGR_REG_VLT0_3_VLT0_3_VV_02_MASK                       		0x000F0000
#define		CCU_CLK_MGR_REG_VLT0_3_VLT0_3_VV_01_SHIFT                      		8
#define		CCU_CLK_MGR_REG_VLT0_3_VLT0_3_VV_01_MASK                       		0x00000F00
#define		CCU_CLK_MGR_REG_VLT0_3_VLT0_3_VV_00_SHIFT                      		0
#define		CCU_CLK_MGR_REG_VLT0_3_VLT0_3_VV_00_MASK                       		0x0000000F

#define		CCU_CLK_MGR_REG_VLT4_7_OFFSET		                                0x00000044
#define		CCU_CLK_MGR_REG_VLT4_7_PRIV_ACCESS_MODE_SHIFT                  		31
#define		CCU_CLK_MGR_REG_VLT4_7_PRIV_ACCESS_MODE_MASK                   		0x80000000
#define		CCU_CLK_MGR_REG_VLT4_7_VLT4_7_VV_07_SHIFT                      		24
#define		CCU_CLK_MGR_REG_VLT4_7_VLT4_7_VV_07_MASK                       		0x0F000000
#define		CCU_CLK_MGR_REG_VLT4_7_VLT4_7_VV_06_SHIFT                      		16
#define		CCU_CLK_MGR_REG_VLT4_7_VLT4_7_VV_06_MASK                       		0x000F0000
#define		CCU_CLK_MGR_REG_VLT4_7_VLT4_7_VV_05_SHIFT                      		8
#define		CCU_CLK_MGR_REG_VLT4_7_VLT4_7_VV_05_MASK                       		0x00000F00
#define		CCU_CLK_MGR_REG_VLT4_7_VLT4_7_VV_04_SHIFT                      		0
#define		CCU_CLK_MGR_REG_VLT4_7_VLT4_7_VV_04_MASK                      		0x0000000F

#define		CCU_CLK_MGR_REG_POLICY0_MASK2_OFFSET					0x00000048
#define		CCU_CLK_MGR_REG_POLICY0_MASK2_PRIV_ACCESS_MODE_SHIFT		        31
#define		CCU_CLK_MGR_REG_POLICY0_MASK2_PRIV_ACCESS_MODE_MASK		        0x80000000

#define		CCU_CLK_MGR_REG_POLICY1_MASK2_OFFSET					0x0000004C
#define		CCU_CLK_MGR_REG_POLICY1_MASK2_PRIV_ACCESS_MODE_SHIFT		        31
#define		CCU_CLK_MGR_REG_POLICY1_MASK2_PRIV_ACCESS_MODE_MASK		        0x80000000

#define		CCU_CLK_MGR_REG_POLICY2_MASK2_OFFSET					0x00000050
#define		CCU_CLK_MGR_REG_POLICY2_MASK2_PRIV_ACCESS_MODE_SHIFT		        31
#define		CCU_CLK_MGR_REG_POLICY2_MASK2_PRIV_ACCESS_MODE_MASK		        0x80000000

#define		CCU_CLK_MGR_REG_POLICY3_MASK2_OFFSET					0x00000054
#define		CCU_CLK_MGR_REG_POLICY3_MASK2_PRIV_ACCESS_MODE_SHIFT		        31
#define		CCU_CLK_MGR_REG_POLICY3_MASK2_PRIV_ACCESS_MODE_MASK		        0x80000000

#define		CCU_CLK_MGR_REG_BUS_QUIESC_OFFSET		                        0x00000100
#define		CCU_CLK_MGR_REG_BUS_QUIESC_PRIV_ACCESS_MODE_SHIFT             	 	31
#define		CCU_CLK_MGR_REG_BUS_QUIESC_PRIV_ACCESS_MODE_MASK               		0x80000000
#define		CCU_CLK_MGR_REG_BUS_QUIESC_SWITCH_AXI_SWITCH_REQGNT_INH_SHIFT  		14
#define		CCU_CLK_MGR_REG_BUS_QUIESC_SWITCH_AXI_SWITCH_REQGNT_INH_MASK   		0x00004000

#define		CCU_CLK_MGR_REG_DIV_TRIG_OFFSET		                               	0x00000AFC
#define		CCU_CLK_MGR_REG_DIV_TRIG_PRIV_ACCESS_MODE_SHIFT                		31
#define		CCU_CLK_MGR_REG_DIV_TRIG_PRIV_ACCESS_MODE_MASK                 		0x80000000

#define		CCU_CLK_MGR_REG_DEBUG0_OFFSET		                                0x00000E00
#define		CCU_CLK_MGR_REG_DEBUG0_PRIV_ACCESS_MODE_SHIFT                  		31
#define		CCU_CLK_MGR_REG_DEBUG0_PRIV_ACCESS_MODE_MASK                  		0x80000000

#define		CCU_CLK_MGR_REG_DEBUG1_OFFSET		                                0x00000E04
#define		CCU_CLK_MGR_REG_DEBUG1_PRIV_ACCESS_MODE_SHIFT                  		31
#define		CCU_CLK_MGR_REG_DEBUG1_PRIV_ACCESS_MODE_MASK                   		0x80000000

#define		CCU_CLK_MGR_REG_DEBUG2_OFFSET		                                 0x00000E08
#define		CCU_CLK_MGR_REG_DEBUG2_PRIV_ACCESS_MODE_SHIFT                  		31
#define		CCU_CLK_MGR_REG_DEBUG2_PRIV_ACCESS_MODE_MASK                   		0x80000000

#define		CCU_CLK_MGR_REG_DEBUG4_OFFSET		                                0x00000E10
#define		CCU_CLK_MGR_REG_DEBUG4_PRIV_ACCESS_MODE_SHIFT                  		31
#define		CCU_CLK_MGR_REG_DEBUG4_PRIV_ACCESS_MODE_MASK                   		0x80000000

#define		CCU_CLK_MGR_REG_DEBUG5_OFFSET		                                0x00000E14
#define		CCU_CLK_MGR_REG_DEBUG5_PRIV_ACCESS_MODE_SHIFT                  		31
#define		CCU_CLK_MGR_REG_DEBUG5_PRIV_ACCESS_MODE_MASK                  		0x80000000

#define		CCU_CLK_MGR_REG_DEBUG6_OFFSET		                                0x00000E18
#define		CCU_CLK_MGR_REG_DEBUG6_PRIV_ACCESS_MODE_SHIFT                  		31
#define		CCU_CLK_MGR_REG_DEBUG6_PRIV_ACCESS_MODE_MASK                   		0x80000000

#define		CCU_CLK_MGR_REG_DEBUG8_OFFSET		                                0x00000E20
#define		CCU_CLK_MGR_REG_DEBUG8_PRIV_ACCESS_MODE_SHIFT                  		31
#define		CCU_CLK_MGR_REG_DEBUG8_PRIV_ACCESS_MODE_MASK                   		0x80000000

#define		CCU_CLK_MGR_REG_DEBUG9_OFFSET		                                0x00000E24
#define		CCU_CLK_MGR_REG_DEBUG9_PRIV_ACCESS_MODE_SHIFT                  		31
#define		CCU_CLK_MGR_REG_DEBUG9_PRIV_ACCESS_MODE_MASK                   		0x80000000

#define		CCU_CLK_MGR_REG_DEBUG10_OFFSET		                                0x00000E28
#define		CCU_CLK_MGR_REG_DEBUG10_PRIV_ACCESS_MODE_SHIFT                 		31
#define		CCU_CLK_MGR_REG_DEBUG10_PRIV_ACCESS_MODE_MASK                  		0x80000000

#define		CCU_CLK_MGR_REG_DEBUG11_OFFSET		                                0x00000E2C
#define		CCU_CLK_MGR_REG_DEBUG11_PRIV_ACCESS_MODE_SHIFT                 		31
#define		CCU_CLK_MGR_REG_DEBUG11_PRIV_ACCESS_MODE_MASK                  		0x80000000

#endif /* __BRCM_RDB_CCU_CLK_MGR_REG_H__ */


