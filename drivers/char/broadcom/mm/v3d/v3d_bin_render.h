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

#ifndef __BIN_RENDER_H__
#define __BIN_RENDER_H__
#define v3d_job_t struct _v3d_job_t

struct _v3d_job_t {
	uint32_t v3d_ct0ca;
	uint32_t v3d_ct0ea;
	uint32_t v3d_ct1ca;
	uint32_t v3d_ct1ea;
};

enum tlb_buffer_size_t {
	TLB_QUARTER_BUFFER_SIZE,
	TLB_HALF_SIZE,
	TLB_FULL_SIZE,
};

enum vri_memory_size_t {
	VRI_HALF_SIZE,
	VRI_FULL_SIZE,
};

struct v3d_version_info_t {
	int v3d_technology_version;
	int vpm_size;			/*In the multiples of 1K*/
	bool hdr_support;		/*True if HDR rendering is supported*/
	int nsem;			/*Number of semaphores*/
	int tpus;			/*Number of TMUs per slice*/
	int qpus;			/*Number of QPU's per slice*/
	int nslc;			/*Number of slices*/
	int rev;			/*V3D revision*/
	bool tlbdb;			/*TLB double buffer mode support*/
	enum tlb_buffer_size_t tlbsz;	/*Tile buffer size*/
	enum vri_memory_size_t vrisz;	/*VRI memory size*/
	uint32_t bin_mem_addr;		/*Physical address of binning memory*/
	uint32_t bin_mem_size;/*Size of the allocated memory for tile binning*/
};

#endif
