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

#ifndef _MM_FW_USR_H_
#define _MM_FW_USR_H_

#define MAX_HANDLES 4

enum {
	MM_CLEAN_JOB = 0x00000000,
	MM_DIRTY_JOB = 0x00000100,

	INTERLOCK_INVALID_JOB = 0x64000000,
	INTERLOCK_WAITING_JOB,
	INTERLOCK_LAST_JOB,

	ISP_INVALID_JOB = 0x65000000,
	ISP_CSC_JOB,
	ISP_LAST_JOB,

	V3D_INVALID_JOB = 0x66000000,
	V3D_BIN_REND_JOB,
	V3D_REND_JOB,
	V3D_LAST_JOB,

	V3D_USER_INVALID_JOB = 0x66010000,
	V3D_USER_JOB,
	V3D_USER_LAST_JOB,

	H264_JOB_BASE = 0x67000000,
	H264_SECURE_JOB_OFFSET = 0x00080000,
	H264_CME_INVALID_JOB = H264_JOB_BASE,
	H264_CME_EST_JOB,
	H264_CME_LAST_JOB,

	H264_MCIN_INVALID_JOB = 0x67010000,
	H264_MCIN_EPR_JOB,
	H264_MCIN_LAST_JOB,

	H264_CABAC_INVALID_JOB = 0x67020000,
	H264_CABAC_ENC_JOB,
	H264_CABAC_DEC_JOB,
	H264_CABAC_LAST_JOB,

	H264_VCE_INVALID_JOB = 0x67030000,
	H264_VCE_LAUNCH_JOB,
	H264_VCE_ENC_SLICE_JOB,
	H264_VCE_DEC_SLICE_JOB,
	H264_VCE_RESET_CODEC_JOB,
	H264_VCE_LAST_JOB,

	H264_OL_INVALID_JOB = 0x67040000,
	/*H264*/
	H264_GET_OFFSET_TYPE,
	H264_SPS_PARSE_TYPE,
	H264_PPS_PARSE_TYPE,
	H264_PPS_SPSID_PARSE_TYPE,
	H264_HDR1_PPSID_PARSE_TYPE,
	H264_SLICE_HDR1_PARSE_TYPE,
	H264_SLICE_HDR2_PARSE_TYPE,
	H264_SPS_EXT_PARSE_TYPE,
	H264_MVC_SSPS_PARSE_TYPE,
	H264_AU_DELIMITER_PARSE_TYPE,
	H264_NAL_HDR_EXT_PARSE_TYPE,
	H264_SLICE_ID_PARSE_TYPE,
	H264_SLICE_BC_PARTN_PARSE_TYPE,
	/*MP4*/
	MP4_VOL_PARSE_TYPE,
	MP4_VOP_HDR_PARSE_TYPE,
	MP4_VISUAL_OBJ_PARSE_TYPE,
	/*VC1*/
	VC1_SEQUENCE_PARSE_TYPE,
	VC1_ENTRY_POINT_PARSE_TYPE,
	VC1_PICTURE_PARSE_TYPE,
	VC1_PICTURE_PARSE_2_TYPE,
	VC1_SLICE_PARSE_TYPE,
	/* H264 access protect/unprotect*/
	HWCODEC_PROTECT,
	HWCODEC_UNPROTECT,
	H264_OL_LAST_JOB
};
#define mm_job_type_e unsigned int

enum {
	MM_JOB_STATUS_INVALID = 0,
	MM_JOB_STATUS_DIRTY,
	MM_JOB_STATUS_READY,
	MM_JOB_STATUS_RUNNING,
	MM_JOB_STATUS_RUNNING1,
	MM_JOB_STATUS_RUNNING2,
	MM_JOB_STATUS_SUCCESS = 8,
	MM_JOB_STATUS_ERROR,
	MM_JOB_STATUS_NOT_FOUND,
	MM_JOB_STATUS_TIMED_OUT,
	MM_JOB_STATUS_SKIP,
	MM_JOB_STATUS_LAST
};
#define mm_job_status_e int

struct MM_JOB_POST_T {
	mm_job_status_e status;
	mm_job_type_e type;
	uint32_t id;
	unsigned int size;
	void *data;
	uint32_t handle;
	uint32_t num_dep_handles;
	uint32_t dep_handles[MAX_HANDLES];
	void *spl_data_ptr;
};

struct MM_DEV_SPL_DATA_T {
	unsigned char *buf;
	unsigned int offset;
	unsigned int size;
};

#define mm_dev_spl_data_t struct MM_DEV_SPL_DATA_T

#define mm_job_post_t struct MM_JOB_POST_T

#define INTERLOCK_DEV_NAME	"mm_interlock"
#define V3D_DEV_NAME	"mm_v3d"
#define ISP_DEV_NAME	"mm_isp"
#define ISP2_DEV_NAME	"mm_isp2"
#define H264_DEV_NAME	"mm_h264"
#define MM_DEV_MAGIC	'M'

enum {
	MM_CMD_GET_VERSION = 0x80,
	MM_CMD_POST_JOB,
	MM_CMD_WAIT_JOB,
	MM_CMD_WAIT_HANDLES,
	MM_CMD_ALLOC_SPL_DATA,
	MM_CMD_COPY_SPL_DATA,
	MM_CMD_DEVICE_TRYLOCK,
	MM_CMD_DEVICE_LOCK,
	MM_CMD_DEVICE_UNLOCK,
	MM_CMD_SECURE_JOB_WAIT,
	MM_CMD_SECURE_JOB_DONE,
	MM_CMD_LAST
};

struct MM_JOB_STATUS_T {
	uint32_t id;
	mm_job_status_e status;
	int32_t timeout;
};
#define mm_job_status_t struct MM_JOB_STATUS_T

struct MM_HANDLE_STATUS_T {
	uint32_t num_handles;
	uint32_t handles[MAX_HANDLES];
	mm_job_status_e status[MAX_HANDLES];
};

struct MM_VERSION_INFO_T {
	int size;
	void *version_info_ptr;
};

struct mm_secure_job_t_ {
	mm_job_type_e     type;
	unsigned int      id;
	mm_job_status_e   status;
};
#define mm_secure_job_t   struct mm_secure_job_t_
#define mm_secure_job_ptr struct mm_secure_job_t_ *

#define mm_version_info_t struct MM_VERSION_INFO_T

#define mm_handle_status_t struct MM_HANDLE_STATUS_T

#define MM_IOCTL_VERSION_REQ  _IOWR(MM_DEV_MAGIC, MM_CMD_GET_VERSION, \
						mm_version_info_t)
#define MM_IOCTL_ALLOC_SPL_DATA _IOWR(MM_DEV_MAGIC, MM_CMD_ALLOC_SPL_DATA, \
							unsigned int)
#define MM_IOCTL_COPY_SPL_DATA _IOWR(MM_DEV_MAGIC, MM_CMD_COPY_SPL_DATA, \
						mm_dev_spl_data_t)
#define MM_IOCTL_DEVICE_TRYLOCK _IOWR(MM_DEV_MAGIC, MM_CMD_DEVICE_TRYLOCK, \
							unsigned int)
#define MM_IOCTL_DEVICE_LOCK _IOWR(MM_DEV_MAGIC, MM_CMD_DEVICE_LOCK, \
						unsigned int)
#define MM_IOCTL_DEVICE_UNLOCK _IOWR(MM_DEV_MAGIC, MM_CMD_DEVICE_UNLOCK, \
							unsigned int)

#define MM_IOCTL_SECURE_JOB_WAIT _IOWR(MM_DEV_MAGIC, MM_CMD_SECURE_JOB_WAIT, \
		mm_secure_job_t)

#define MM_IOCTL_SECURE_JOB_DONE _IOWR(MM_DEV_MAGIC, MM_CMD_SECURE_JOB_DONE, \
		mm_secure_job_t)


#endif
