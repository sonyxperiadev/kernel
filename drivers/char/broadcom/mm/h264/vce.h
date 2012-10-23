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
#ifndef _VCE_H_
#define _VCE_H_

/* vce job mem layout*/
/*
   typedef struct vce_launch_s {
   unsigned int mem_start = 0xd7d7d7d7;
   unsigned int job_size;
   unsigned int pvt_offset;
   unsigned int datasize;
   unsigned int data[];
   unsigned int codesize;
   unsigned int code[];
   unsigned int startaddr;
   unsigned int finaladdr;
   unsigned int numdownloads; 
   unsigned int numuploads;
   unsigned int transfer_dir;
   unsigned int vce_addr;
   unsigned int transfer_size;
   unsigned int transfer_data;
   ..
   ..
   ..
   unsigned int mem_end = 0xa5a5a5a5;
   vce_launch_info_t info;
   }
   */

typedef enum {
	VCE_DATA_DOWNLOAD,
	VCE_DATA_UPLOAD
} vce_transfer_type_t;

typedef struct {
	unsigned int datasize;
	unsigned int data_offset;
	unsigned int codesize;
	unsigned int code_offset;
	unsigned int startaddr;
	unsigned int finaladdr;
	unsigned int numdownloads; 
	unsigned int numuploads;
	unsigned int download_start_offset;
	unsigned int upload_start_offset;
	unsigned int encode;
	unsigned int stop_reason;
	unsigned int vce_reg[62];
} vce_launch_info_t;

#endif /*_VCE_H_*/
