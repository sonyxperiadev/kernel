/*******************************************************************************
  Copyright 2010 Broadcom Corporation.  All rights reserved.

  Unless you and Broadcom execute a separate written software license agreement
  governing use of this software, this software is licensed to you under the
  terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/
#ifndef _VCE_POSTRUN_OBJ_
#define _VCE_POSTRUN_OBJ_
static const unsigned int g_vce_postrun_code[] = {
 0xe813f008,
 0x8fc4000,
 0xe813f009,
 0x8fc4000,
 0xe813f000,
 0xe8141000,
 0x44045000,
 0xe8102000,
 0xe817f000,
 0x4c045000,
 0xe813f008,
 0x8fc4000,
 0xe813f009,
 0x8fc4000,
 0xe813f00a,
 0x8fc4000,
 0xcfff001,
 0xe813f007,
 0x8fc4000,
 0xe8fc0000,
 0xe8fc0000,
0xfff000,
 0xe8fc0000,
 0xe8fc0000,
 0x70fc0000,
 0xe8fbf000,
};
static const unsigned char g_vce_postrun_data[] = {
};
static const unsigned int vce_launch_vce_postrun[] = {
 (unsigned int) g_vce_postrun_data,
 0,
 (unsigned int) g_vce_postrun_code,
104,
 0x0,
 0x0,
 0,
 0,
};
#endif /*_VCE_POSTRUN_OBJ_*/
