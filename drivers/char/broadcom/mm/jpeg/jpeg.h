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
#ifndef _JPEG_H_
#define _JPEG_H_
#define JPEG_DEV_NAME	"mm_jpeg"
/* It has a structure for the values passed from the user code to kernel*/
struct jpeg_value {
	unsigned int h;
	unsigned int v;
	unsigned int p;
	unsigned int hardware_add_out_p;
	unsigned int out_size_p;
	unsigned int hardware_add_Y;
	unsigned int hardware_add_U;
	unsigned int hardware_add_V;
	unsigned int stride_0;
	unsigned int stride_1;
	unsigned int stride_2;
	unsigned int xmcus;
	unsigned int ymcus;
	unsigned int jc0w;
	unsigned int jc1w;
	unsigned int jc2w;
	unsigned int jnsb;
	unsigned int jnsb_callback_value;
};
#endif /*_JPEG_H_*/
