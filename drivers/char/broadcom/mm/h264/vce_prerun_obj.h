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
#ifndef _VCE_PRERUN_OBJ_
#define _VCE_PRERUN_OBJ_
static const unsigned int g_vce_prerun_code[] = {
 0xe81bf008,
 0x8fc6000,
 0xe81bf009,
 0x8fc6000,
 0xe81bf000,
 0xe81c1000,
 0x40047000,
 0xe8182000,
 0xe81ff000,
 0x6c047000,
 0xe81bf008,
 0x8fc6000,
 0xe81bf009,
 0x8fc6000,
 0xe81bf00a,
 0x8fc6000,
 0xe81bf008,
 0x8fc6000,
 0xe81bf009,
 0x8fc6000,
 0xe81bf000,
 0xe81c3000,
 0x40047000,
 0xe8184000,
 0xe81ff000,
 0x48047000,
 0xe81bf008,
 0x8fc6000,
 0xe81bf009,
 0x8fc6000,
 0xe81bf00a,
 0x8fc6000,
 0xcfff001,
 0xe81bf007,
 0x8fc6000,
 0xe8fc0000,
 0xe8fc0000,
0xfff000,
 0xe8fc0000,
 0xe8fc0000,
 0x70fc0000,
 0xe8fbf000,
};
static const unsigned char g_vce_prerun_data[] = {
};
static const unsigned int vce_launch_vce_prerun[] = {
 (unsigned int) g_vce_prerun_data,
 0,
 (unsigned int) g_vce_prerun_code,
168,
 0x0,
 0x0,
 0,
 0,
};
#endif /*_VCE_PRERUN_OBJ_*/
