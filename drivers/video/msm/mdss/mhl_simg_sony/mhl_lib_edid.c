/* vendor/semc/hardware/mhl/mhl_sii8620_8061_drv/mhl_lib_edid.c
 *
 * Copyright (C) 2013 Sony Mobile Communications AB.
 * Copyright (C) 2013 Silicon Image Inc.
 *
 * Author: [Hirokuni Kawasaki <hirokuni.kawasaki@sonymobile.com>]
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/errno.h>

#include "mhl_common.h"
#include "mhl_lib_edid.h"
#include "mhl_sii8620_8061_device.h"

/* Block 0 */
#define EDID_OFFSET_HEADER_FIRST_00	0x00
#define EDID_OFFSET_HEADER_FIRST_FF	0x01
#define EDID_OFFSET_HEADER_LAST_FF	0x06
#define EDID_OFFSET_HEADER_LAST_00	0x07

#pragma GCC diagnostic ignored "-Wunused-function"

/* svd (short video descriptor) native bit mask */
#define NATIVE_BIT_MASK 0x7F

/* Start of data block collection */
#define MHL_LIB_DBC_START_OFFSET 4

#define MHL_LIB_MAX_DATA_BLOCK_SIZE 31

#define MATCHED_TIMING_FLAG_LEN_MAX 20

#define VIC_NUM_MAX MATCHED_TIMING_FLAG_LEN_MAX

#define START_4K_VIC 93
#define END_4K_VIC 107

static const uint8_t defined_hev_vic[HEV_VIC_MAX_LEN] = {
	31, 16, 64, 63, 95, 94, 93, 98, 99, 100, 89, 86, 87};

static bool matched_timing_flg[MATCHED_TIMING_FLAG_LEN_MAX] = {
	false, false, false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false, false, false};

/* CEA Data Block Tag Codes */
enum {
	RESERVED_1 = 0,
	AUDIO_DATA_BLOCK,
	VIDEO_DATA_BLOCK,
	VENDOR_SPECIFIC_DATA_BLOCK,
	SPEAKER_ALLOCATION_DATA_BLOCK,
	VESA_DTC_DATA_BLOCK,
	RESERVED_2,
	USE_EXTENDED_TAG
};

#ifdef DEBUG_PRINT
#define PRINT_DATA(_data, _count, _string) \
	{ \
		int i; \
		pr_debug("-----%s(): %s-----\n", __func__, _string);\
		for(i = 0; i<_count; i++){ \
			pr_debug("0x%02x ", _data[i]); \
			if ((i+1) %16 == 0) \
				pr_debug("\n"); \
			if (i == _count - 1) \
				pr_debug("\n"); \
		} \
		pr_debug("-----\n"); \
	}
#else
#define PRINT_DATA(_data, _count, _string)
#endif

#define OFFSET_MOVE(_offset, _number, _max_offset) \
_offset += _number;\
pr_debug("moving... offset = %d\n", _offset);\
if (_offset >= _max_offset) {\
	pr_warn("LINE : %d. Illegal offset. max offset is %d\n", __LINE__, _max_offset);\
	return _max_offset;\
}

#define IS_LATENCY_FIELDS_PRESENT(_vsd) \
	pr_debug("Latency Fields Present = 0x%x (offset = 8)\n", *(_vsd + 8));\
	pr_info("0x%02x\n",*(_vsd + 8) & 0x80);\
	if (*(_vsd + 8) & 0x80) {

#define IS_I_LATENCY_FIELDS_PRESENT(_vsd) \
	pr_debug("I Latency Fields Present = 0x%x (offset = 8)\n", *(_vsd + 8));\
	if (*(_vsd + 8) & 0x40) {

#define FILL_WITH_0_EXCEPT_FOR_IMAGE_SIZE(_vsd, _offset)\
	pr_debug("Keep Image_Size, other is 0 (offset = %d)\n", _offset);\
	pr_debug("current : 0x%02x\n", *(_vsd + _offset));\
	*(_vsd + _offset) &= 0x18;\
	pr_debug("change to : 0x%02x\n", *(_vsd + _offset))

#define GET_HDMI_VIC_LEN(_hdmi_vic_len, _vsd, _offset)\
	_hdmi_vic_len = (*(_vsd + _offset)) >> 5;\
	pr_debug("hdmi_vic_len = %d\n", _hdmi_vic_len)

#define GET_HDMI_3D_LEN(_hdmi_3d_len, _vsd, _offset)\
	_hdmi_3d_len = *(_vsd + _offset) & 0x1F;\
	pr_debug("hdmi_3d_len = %d\n", _hdmi_3d_len)

static int mhl_lib_edid_remove_vic(uint8_t *svd, uint8_t svd_size,
	uint8_t *filter, uint8_t filter_size);
static int mhl_lib_edid_pull_up_and_padding(uint8_t *head,
	int data_size, int pull_up_size);
static void mhl_lib_edid_remake_ext_blocK(
	PCEA_extension_t extension,
	int pull_up_size,
	uint8_t *last_svd_address,
	int new_svd_size);
static const uint8_t *hdmi_edid_find_block_v2(
	const uint8_t *in_buf,
	const uint8_t db_num,
	 u32 start_offset,
	 u8 type, u8 *len);

static void mhl_lib_edid_remove_unsupp_detailed_timing(
			const struct mhl_video_timing_info *support_video,
			uint8_t support_video_len,
			uint8_t *detailed_timing);
static struct mhl_video_timing_info mhl_lib_edid_get_best_support_timing(
			const struct mhl_video_timing_info *support_video,
			uint8_t support_video_len);

bool mhl_edid_check_edid_header(uint8_t *pSingleEdidBlock)
{
	uint8_t i = 0;
	uint8_t first_00 = EDID_OFFSET_HEADER_FIRST_00;
	uint8_t first_ff = EDID_OFFSET_HEADER_FIRST_FF;
	uint8_t last_ff = EDID_OFFSET_HEADER_LAST_FF;
	uint8_t last_00 = EDID_OFFSET_HEADER_LAST_00;

	if (0x00 != pSingleEdidBlock[first_00]) {
		pr_debug("%s: no offset header first\n", __func__);
		return false;
	}

	for (i = first_ff; i <= last_ff; i++)
		if (0xFF != pSingleEdidBlock[i]) {
			pr_debug("%s: no ff\n", __func__);
			return false;
		}

	if (0x00 != pSingleEdidBlock[last_00]) {
		pr_debug("%s: no offset header last\n", __func__);
		return false;
	}

	return true;
}

/*
 * Calculte checksum of the 128 byte block pointed to by the
 * pointer passed as parameter
 *
 * p_EDID_block_data : Pointer to a 128 byte block whose checksum
 * needs to be calculated
 *
 * RETURNS : true if chcksum is 0. false if not.
 */

bool mhl_lib_edid_is_valid_checksum(uint8_t *p_EDID_block_data)
{
	uint8_t i;
	uint8_t checksum = 0;

	for (i = 0; i < EDID_BLOCK_SIZE; i++)
		checksum += p_EDID_block_data[i];

	if (checksum) {
		pr_warn("%s: false\n", __func__);
		return false;
	}

	return true;
}

int mhl_edid_parser_get_num_cea_861_extensions(uint8_t *pSingleEdidBlock)
{
	PEDID_block0_t p_EDID_block_0 = (PEDID_block0_t)pSingleEdidBlock;
	/* check edid header format exists or not */

	if (!mhl_edid_check_edid_header(pSingleEdidBlock)) {
		pr_warn("%s:invalid edid header\n", __func__);
		return MHL_FAIL;
	}

	if (!mhl_lib_edid_is_valid_checksum(pSingleEdidBlock)) {
		pr_warn("%s:checksum error\n", __func__);
		return MHL_FAIL;
	}

	return p_EDID_block_0->extension_flag;
}

/*
 * ext_edid : one extension edid block (128 byte)
 */
static uint8_t get_data_block_tag_code(uint8_t data_block_collection);
static uint8_t get_data_block_length(uint8_t data_block_collection);
static void remove_head_from_data(uint8_t *data, int data_length);
static void set_data_block_length(uint8_t *data_block, uint8_t length);
static uint8_t calculate_generic_checksum(
	uint8_t *info_frame_data,
	uint8_t checksum,
	uint8_t length);

int squash_data_with_prune_0(uint8_t *data, int length)
{
	int i;
	int hit_cnt = 0;
	uint8_t *head = data;
	uint8_t *last = head + length - 1;
	int orig_len = length;

	/* squash hdmi vic */
	for (;  0 < length; length--) {
		if (*data == 0x00) {
			memcpy((void *)data, (const void *)(data + 1), (size_t)(length - 1));
			*(data + length - 1) = 0x00;

			if ((data + length - 1) > last) {
				pr_warn("exceeds max mem. length = %d\n", length);
				return -EPERM;
			}
		} else
			data++;
	}

	/* count 0 in the entire data */
	for (i = 0; i < orig_len; i++)
		if (*(head + i) == 0x00)
			hit_cnt++;

	return hit_cnt;
}

/*
 * svd : short video descriptor
 */
void mhl_lib_edid_remove_vic_from_svd(uint8_t *ext_edid,
		 uint8_t *not_removed_vic_array,
		 uint8_t not_removed_vic_array_size)
{
	PCEA_extension_t extension = (PCEA_extension_t)ext_edid;
	uint8_t db_len;		/*data block length*/
	uint8_t *db_header;	/* video data block header */

	/* search video tag code */
	db_header = (uint8_t *)hdmi_edid_find_block_v2(
		(const uint8_t *)ext_edid,
		1,
		 MHL_LIB_DBC_START_OFFSET,
		 VIDEO_DATA_BLOCK,
		 &db_len);

	if (db_header != NULL) {
		/* svd : short video descriptor */
		uint8_t *svd = db_header + 1;
		int svd_size = db_len;
		int new_svd_size = mhl_lib_edid_remove_vic(
				svd, svd_size,
				not_removed_vic_array,
				not_removed_vic_array_size);

		/* change the length of the data block */
		set_data_block_length(db_header, new_svd_size);

		{/* re-making the ext_edid data */
			int pull_up_size = svd_size - new_svd_size;
			uint8_t *last_svd_address = svd + new_svd_size;

			mhl_lib_edid_remake_ext_blocK(
					extension,
					pull_up_size,
					last_svd_address,
					new_svd_size);
		}
	} else
		pr_info("%s: VideoDataBlock is nothing!!", __func__);
}




/*
 * make edid with new svd size.
 * (svd : short video descriptor)
 */
static void mhl_lib_edid_remake_ext_blocK(
					PCEA_extension_t extension,
					 int pull_up_size,
					 uint8_t *last_svd_address,
					 int new_svd_size)
{
	int check_sum_1_byte = 1;

	/* renew svd size. check sum is not included */
	/*int pull_up_target_size =
		 block_size_128_byte -
		 cea_ext_1st_4_byte -
		 data_block_header_size_1_byte -
		 new_svd_size -
		 check_sum_1_byte;*/

	int pull_up_target_size = (uint8_t *)extension +
				 EDID_BLOCK_SIZE -
				 last_svd_address -
				 check_sum_1_byte;


	pr_debug("%s:pull up size = %d\n", __func__, pull_up_size);
	pr_debug("%s:new svd size = %d\n", __func__, new_svd_size);
	pr_debug("%s:last svd = 0x%02x, 0x%02x, 0x%02x\n", __func__, *(last_svd_address-1), *(last_svd_address), *(last_svd_address+1));
	pr_debug("%s:pull up target size = %d\n", __func__, pull_up_target_size);


	/* pull up the data after svd */
	mhl_lib_edid_pull_up_and_padding(
		last_svd_address,
		pull_up_target_size,
		pull_up_size);

	/* offset for 18 byte descriptors */
	if (*((uint8_t *)extension + 2) != 0)
		*((uint8_t *)extension + 2) = *((uint8_t *)extension + 2) - pull_up_size;

	/* check sum */
	extension->checksum = 0x00;
	extension->checksum = calculate_generic_checksum(
		(uint8_t *)extension,
		 0x00,
		 sizeof(*extension));
}

void mhl_lib_edid_set_checksum(uint8_t *one_edid_blk)
{
	PEDID_block0_t one_edid_blk_ = (PEDID_block0_t)one_edid_blk;

	/* check sum */
	one_edid_blk_->checksum = 0x00;
	one_edid_blk_->checksum = calculate_generic_checksum(
		(uint8_t *)one_edid_blk_,
		 0x00,
		 sizeof(*one_edid_blk_));
}

static int get_converted_hdmi_vic(uint8_t vic)
{
	switch (vic) {
	case 95:
		return 1;
	case 94:
		return 2;
	case 93:
		return 3;
	case 98:
		return 4;
	default:
		return -EINVAL;
	}
}

static bool is_supp_hdmi_vic(uint8_t hdmi_vic, uint8_t *supp_vic, uint8_t supp_vic_size)
{
	int i;

	for (i = 0; i < supp_vic_size; i++) {
		int supp_hdmi_vic = get_converted_hdmi_vic(supp_vic[i]);
		if (hdmi_vic == supp_hdmi_vic) {
			/* set SOMC support flg */
			matched_timing_flg[i] = true;
			return true;
		}
	}

	return false;
}



static int mhl_lib_edid_remove_hdmi_vic_and_3d(
				uint8_t *vsd, uint8_t vsd_size)
{
	uint8_t offset = 0;
	uint8_t new_vsd_size = 0;

	PRINT_DATA(vsd, vsd_size + 1, "[in] vsd")

	if ((uint8_t)vsd_size != (uint8_t)(*vsd & 0x1F)) {
		pr_warn("%s:Illegal vsd size, [in] = %d, db_len = %d\n",
		 __func__,
		 vsd_size,
		 *vsd & 0x1F);
		return (*vsd) & 0x1F;
	}

	if (vsd_size < 9)
		return vsd_size;

	/* fill HDMI_Video_present with 0 */
	*(vsd + 8) = *(vsd + 8) & 0xDF;

	offset = 9;
	IS_LATENCY_FIELDS_PRESENT(vsd)
		OFFSET_MOVE(offset, 2, vsd_size);
	}
	IS_I_LATENCY_FIELDS_PRESENT(vsd)
		OFFSET_MOVE(offset, 2, vsd_size);
	}

	FILL_WITH_0_EXCEPT_FOR_IMAGE_SIZE(vsd, offset);
	OFFSET_MOVE(offset, 1, vsd_size);

	/* remaining fields are filled up with 0 */

	memset((void *)(vsd + offset), 0x00, (size_t)(vsd_size + 1 - offset));

	pr_debug("%s:0 fill size = %d\n", __func__, vsd_size +1 - offset);
	pr_debug("%s: return = %d\n", __func__, offset);
	new_vsd_size = offset - 1;

	set_data_block_length(vsd, new_vsd_size);
	PRINT_DATA(vsd, vsd_size + 1, "[out] vsd")

	return new_vsd_size;
}


/* this API does not change the vsd size info
 * even some data is removed. Keep original length data. */
int mhl_lib_edid_replace_hdmi_vic_and_remove_3d(
	uint8_t *vsd,
	int vsd_size,
	uint8_t *supp_vic,
	uint8_t supp_vic_size)
{
	int hdmi_vic_len;
	int hdmi_3d_len;
	int i;
	int new_hdmi_vic_len = 0;
	uint8_t offset = 0;
	uint8_t new_vsd_size = 0;

	PRINT_DATA(vsd, vsd_size + 1, "[in] vsd")

	if ((uint8_t)vsd_size != (uint8_t)(*vsd & 0x1F)) {
		pr_warn("%s:Illegal vsd size, vsd size = %d, db_len = %d\n",
		 __func__,
		 vsd_size,
		 *vsd & 0x1F);
		return (*vsd) & 0x1F;
	}

	if (vsd_size < 9)
		return vsd_size;

	/* check HDMI Video Present */
	if (!(*(vsd + 8) & 0x20)) {
		pr_debug("%s:no HDMI Video Present\n", __func__);
		return vsd_size;
	}

	offset = 9;

	IS_LATENCY_FIELDS_PRESENT(vsd)
		OFFSET_MOVE(offset, 2, vsd_size);
	}

	IS_I_LATENCY_FIELDS_PRESENT(vsd)
		OFFSET_MOVE(offset, 2, vsd_size);
	}

	FILL_WITH_0_EXCEPT_FOR_IMAGE_SIZE(vsd, offset);
	OFFSET_MOVE(offset, 1, vsd_size);

	GET_HDMI_VIC_LEN(hdmi_vic_len/*out*/, vsd/*in*/, offset/*in*/);
	GET_HDMI_3D_LEN (hdmi_3d_len/*out*/,  vsd/*in*/, offset/*in*/);
	OFFSET_MOVE(offset, 1, vsd_size);

	/* fill unsupported vic with 0 */
	for (i = 0; i < hdmi_vic_len; i++) {
		uint8_t *hdmi_vic = (vsd + offset + i);
		if (!is_supp_hdmi_vic(*hdmi_vic, supp_vic, supp_vic_size))
			*hdmi_vic = 0x00;
		else
			new_hdmi_vic_len++;
	}

	/* renew HDMI_VIC_LEN and HDMI_3D_LEN(intentionally 0) */
	OFFSET_MOVE(offset, -1, vsd_size);
	*(vsd + offset) =  new_hdmi_vic_len << 5;
	OFFSET_MOVE(offset, 1, vsd_size);

	/* squash hdmi vic */
	squash_data_with_prune_0((vsd + offset), hdmi_vic_len);

	/* remove 3d */
	{
		uint8_t *vsd_3d_data = vsd + offset + hdmi_vic_len;
		memset((void *)(vsd_3d_data), 0x00, (size_t)(hdmi_3d_len));
	}

	new_vsd_size = offset + new_hdmi_vic_len -1;

	set_data_block_length(vsd, new_vsd_size);

	pr_debug("new size = %d\n", new_vsd_size);
	PRINT_DATA(vsd, vsd_size + 1, "[out] vsd")

	return new_vsd_size;
}

/*
 * NOTE : "not_removed_vic_array" and "not_removed_vic_array_size"
 * don't work. Entire hdmi vic and 3d info are removed.
 */
void mhl_lib_edid_remove_hdmi_vic_and_3d_from_vsd(
	uint8_t *ext_edid,
	uint8_t *not_removed_vic_array,
	uint8_t not_removed_vic_array_size)
{
	PCEA_extension_t extension = (PCEA_extension_t)ext_edid;
	uint8_t db_len;
	uint8_t *vsd = NULL;
	uint8_t *data_block_header = NULL;

	PRINT_DATA(ext_edid, EDID_BLOCK_SIZE, "[in] ext_edid")

	/* search vendor specific tag code */
	data_block_header = (uint8_t *)hdmi_edid_find_block_v2(
		(const uint8_t *)ext_edid,
		1,
		 MHL_LIB_DBC_START_OFFSET,
		 VENDOR_SPECIFIC_DATA_BLOCK,
		 &db_len);

	if (data_block_header != NULL) {
		int vsd_size = db_len;
		int new_vsd_size = -1;
		/* vsd : vendor specific data */
		vsd = data_block_header;

		if (not_removed_vic_array != NULL) {
			new_vsd_size = mhl_lib_edid_replace_hdmi_vic_and_remove_3d(
				vsd, vsd_size,
				not_removed_vic_array, not_removed_vic_array_size);
		} else {
			new_vsd_size = mhl_lib_edid_remove_hdmi_vic_and_3d(vsd, vsd_size);

			pr_debug("%s:vsd_size=%d\n", __func__, vsd_size);
			pr_debug("%s:new_vsd_size=%d\n", __func__, new_vsd_size);
		}

		/* change the length of the data block */
		set_data_block_length(data_block_header, new_vsd_size);


		{/* re-making the ext_edid data */
			int pull_up_size = vsd_size - new_vsd_size;
			uint8_t *last_vsd_address = vsd + new_vsd_size + 1;
			int check_sum_1_byte = 1;
			int pull_up_target_size = ext_edid +
				 EDID_BLOCK_SIZE -
				 last_vsd_address -
				 check_sum_1_byte;

			pr_debug("%s:pull_up_size=%d\n",
			 __func__, pull_up_size);
			pr_debug("%s:*last_vsd_address=0x%x\n",
			 __func__, *last_vsd_address);
			pr_debug("%s:pull_up_target_size=%d\n",
			 __func__, pull_up_target_size);

			/* pull up the data after vsd */
			mhl_lib_edid_pull_up_and_padding(last_vsd_address,
				 pull_up_target_size,/* check sum must not be counted */
				 pull_up_size);

			/* offset for 18 byte descriptors */
			if ( *(ext_edid + 2) != 0)
				*(ext_edid + 2) = *(ext_edid + 2) - pull_up_size;

			/* check sum */
			extension->checksum = 0x00;
			extension->checksum = calculate_generic_checksum(
				(uint8_t *)extension,
				 0x00,
				 sizeof(*extension));
		}
	}

	PRINT_DATA(ext_edid, EDID_BLOCK_SIZE, "[out] ext_edid")
}

/*
 * It removes vic from 1)svd with using 2)filter and returns
 * its remained vic data size in the svd. Any vic in svd
 * which is not specified in filter is removed from svd.
 *
 * 1)svd : short video descriptor
 * 2)filter : it is used for remove vic from short video descriptor.
 */
static int mhl_lib_edid_remove_vic(
	uint8_t *svd, uint8_t svd_size,
	uint8_t *filter, uint8_t filter_size)
{
	int i;
	int j;
	int removed_size = 0;
	uint8_t *work_filter = filter;

	for (i = 0; i < svd_size; i++) {
		bool remove = true;
		for (j = 0; j < filter_size; j++) {

			if ((NATIVE_BIT_MASK & *svd) == *work_filter++) {
				remove = false;
				/* set SOMC support flg */
				matched_timing_flg[j] = true;
				break;
			}
		}
		work_filter = filter;

		if (remove) {
			remove_head_from_data(svd, svd_size - (i + 1));
			removed_size++;
		} else {
			svd++;
		}
	}

	return svd_size - removed_size;
}

static void set_data_block_length(uint8_t *data_block, uint8_t length)
{
	*data_block &= 0xE0;
	*data_block |= (length & 0x1F);
}

static uint8_t get_data_block_tag_code(uint8_t data_block_collection)
{
	return (data_block_collection & 0xE0) >> 5;
}

static uint8_t get_data_block_length(uint8_t data_block_collection)
{
	return data_block_collection & 0x1F;
}

/*
 * pull up data of array (head) with pull_up_size
 * and padding 0 to the last by the pull up size.
 *
 * e.g.
 *  scnenario : pull_up_size is 2, head is pointing 1st element (1)
 *
 *  API execution :
 *    uint8_t data[10] = {1,2,3,4,5,6,7,8,9,10};
 *    mhl_lib_edid_pull_up_and_padding(data, 10, 2);
 *    result : data[10] = {1,2,3,4,5,6,7,8,9,10}; -->
 *             data[10] = {3,4,5,6,7,8,9,10,0,0};
 */
/* #define DEBUG_PRINT_EDID_LIB */
static int mhl_lib_edid_pull_up_and_padding(
	uint8_t *head,
	 int data_size,
	 int pull_up_size)
{
	int i;
	int loop_cnt;
	int remainder;
	uint8_t *org_head = head;
	uint8_t *last_address = org_head + data_size;

	if (pull_up_size == 0)
		return MHL_SUCCESS;

	loop_cnt = data_size / pull_up_size;
	remainder = data_size % pull_up_size;

#ifdef DEBUG_PRINT_EDID_LIB
	pr_info("loop_cnt : %d\n", loop_cnt);
	pr_info("remainder : %d\n", remainder);
#endif

	for (i = 0; i < loop_cnt; i++) {
		if (head + pull_up_size < last_address)
			memcpy((void *)head,
			 (const void *)(head + pull_up_size),
			 (size_t)pull_up_size);

		head += pull_up_size;
#ifdef DEBUG_PRINT_EDID_LIB
		{
			int j;
			pr_debug(" ---- pull up ----\n");
			for (j = 0; j < data_size; j++)
				pr_debug("0x%02x, ", *(org_head + j));
			pr_debug("\n");
		}
#endif
	}
	/* padding */
	head -= pull_up_size - remainder;
	for (i = 0; i < pull_up_size; i++) {

		if (head <= last_address) {
			*head++ = 0x00;
		} else {
			pr_warn("%s:illegal padding\n", __func__);
			return MHL_FAIL;
		}
#ifdef DEBUG_PRINT_EDID_LIB
		{
			int j;
			pr_info(" ---- padding ----\n");
			for (j = 0; j < data_size; j++)
				pr_info("0x%02x, ", *(org_head + j));
			pr_info("\n");
		}
#endif
	}

	return MHL_SUCCESS;
}

static int mhl_lib_edid_pull_up(
	uint8_t *head,
	 int data_size,
	 int pull_up_size)
{
	/* will be inplemented in future */
	return 0;
}


/* data_length : length from next pointer of *data  */
static void remove_head_from_data(uint8_t *data, int data_length)
{
	int i;
	for (i = 0; i < data_length; i++) {
		*data = *(data + 1);
		data++;
	}
	pr_debug("\n");
	*data = 0;
}

static uint8_t calculate_generic_checksum(
	uint8_t *info_frame_data,
	 uint8_t checksum,
	 uint8_t length)
{
	uint8_t i;

	for (i = 0; i < length; i++)
		checksum += info_frame_data[i];

	checksum = 0x100 - checksum;

	return checksum;
}

/*
 * A caller's edid pointer is stored in.
 * The caller must not free or change the
 * pointer until other callers finish using it.
 */
static const uint8_t *edid_data;

void mhl_lib_edid_init(const uint8_t *edid)
{
	edid_data = edid;
	memset(matched_timing_flg, false, SUPPORT_SOMC_VIDEO_NUM);
}


static const uint8_t *hdmi_edid_find_block_v2(
	const uint8_t *in_buffer,
	const uint8_t db_number,
	 u32 in_start_offset,
	 u8 in_type, u8 *p_len)
{
	/* the start of data block collection, start of Video Data Block */
	u32 temp_offset = in_start_offset;
	u32 end_offset = in_buffer[2];
	uint8_t target_db_number = db_number;

	*p_len = 0;

	/*
	 * * edid buffer 1, byte 2 being 4 means no non-DTD/Data block
	 *   collection present.
	 * * edid buffer 1, byte 2 being 0 menas no non-DTD/DATA block
	 *   collection present and no DTD data present.
	 */
	if ((end_offset == 0) || (end_offset == 4)) {
		pr_warn("EDID: no DTD or non-DTD data present\n");
		return NULL;
	}

	while (temp_offset < end_offset) {
		u8 block_len = in_buffer[temp_offset] & 0x1F;
		if ((temp_offset + block_len <= end_offset) &&
			(in_buffer[temp_offset] >> 5) == in_type) {
			*p_len = block_len;
			pr_debug("%s: EDID: type=%d found @ 0x%x w/ len=%d\n",
				__func__, in_type, temp_offset, block_len);

			if (--target_db_number <= 0)
				return in_buffer + temp_offset;
		}
		temp_offset += (uint8_t)(1 + block_len);
	}
	pr_debug("%s: EDID: type=%d block not found in EDID block\n",
		__func__, in_type);

	return NULL;
} /* hdmi_edid_find_block */


static bool mhl_lib_is_ieee_reg_id(const uint8_t *ext_edid)
{
	u8 len;
	const u8 *vsd = NULL;
	uint8_t cnt = 1;

	while ((vsd = hdmi_edid_find_block_v2((const uint8_t *)ext_edid, cnt++, MHL_LIB_DBC_START_OFFSET,
				   VENDOR_SPECIFIC_DATA_BLOCK, &len))) {

		if (vsd == NULL || len < 3 || len > MHL_LIB_MAX_DATA_BLOCK_SIZE) {
			pr_warn("%s: No/Invalid Vendor Specific Data Block. count = %d\n",
			  __func__, cnt - 1);
			return false;
		}

		/* check ieee oui code */
		if (0x0c03 == (((u32)vsd[3] << 16) + ((u32)vsd[2] << 8) + (u32)vsd[1]))
			return true;
	}

	return false;
} /* mhl_lib_is_ieee_reg_id */


/*
 * If no data is set with mhl_lib_edid_set_edid(),
 * then always false is returned. Otherwise if the
 * set edid data is HDMI, then return true.
 */
bool mhl_lib_edid_is_hdmi(void)
{
	int cea_861_ext_num;
	int i;

	if (edid_data == NULL) {
		pr_debug("%s:arg is null\n", __func__);
		return false;
	}

	/* check the number of extention block */
	cea_861_ext_num =
		mhl_edid_parser_get_num_cea_861_extensions((uint8_t *)edid_data);

	if (cea_861_ext_num < 0) {
		pr_warn("%s:illegal ext num\n", __func__);
		return false;
	}

	switch (cea_861_ext_num) {
	case 0:
		pr_debug("%s:DVI\n", __func__);
		return false;
	case 1:
		i = 1;
		break;
	default:
		i = 2;
	}

	for (; i <= cea_861_ext_num; i++)
		if ((edid_data[i * 0x80]) == 2)
			if (mhl_lib_is_ieee_reg_id(edid_data + (i * 0x80))) {
				pr_debug("%s:HDMI\n", __func__);
				return true;
			}

	pr_debug("%s:DVI\n", __func__);
	return false;

}

bool mhl_lib_is_asus_vx239h()
{
	if (edid_data == NULL) {
		pr_debug("%s:edid is null\n", __func__);
		return false;
	}

	/* 0x0469(ACI) = ASUS */
	if ((edid_data[0x08] == 0x04) && (edid_data[0x09] == 0x69)) {
		if (memcmp(&edid_data[0x5F], "VX279", 5) == 0) {
			pr_debug("%s():asus vx239h PackedPixel off.\n",
					__func__);
			return true;
		}
	}
	return false;
}

struct std_timing {
	uint8_t first_byte;
	uint8_t second_byte;
};

/*
 * supported std_timings are returned.
 *
 * true : supported
 * false : not supported
 */
static bool is_supported_std_timing(
	uint8_t first_byte,
	uint8_t second_byte,
	const struct mhl_video_timing_info *video_timing_info,
	uint8_t length)
{
	int i = 0;

	if (length > MATCHED_TIMING_FLAG_LEN_MAX) {
		pr_err("exceeds flag array max\n");
		return false;
	}

	for (i = 0; i < length; i++) {
		if ((first_byte == (video_timing_info + i)->h_pixel) &&
			(second_byte ==
				(video_timing_info + i)->aspect_refresh)) {
			/* set SOMC support flg */
			matched_timing_flg[i] = true;
			return true;
		}
	}

	return false;
}
/*
 * remove unsupported resolution data from
 * standard timing (std_timing).
 */
void mhl_lib_edid_remove_standard_timing(
	const struct mhl_video_timing_info *support_video,
	uint8_t length,
	uint8_t *std_timing)
{
	uint8_t offset = 0;
	uint8_t std_timing_cnt = 0;
	bool is_support = false;
	/* standard timing max size is 16. It is defined by the */
	/* specification of standard timing */
	uint8_t std_timing_tmp[16];
	uint8_t hit_cnt = 0;

	/* initialize to blank data 0x01 */
	memset(std_timing_tmp, 0x01, sizeof(std_timing_tmp));

	for (std_timing_cnt = 0; std_timing_cnt < 8; std_timing_cnt++) {
		is_support = is_supported_std_timing(
			*(std_timing + offset),
			*(std_timing + offset + 1),
			 support_video,
			 length);

		/* if the sdt timing is not supported,
		 * blank is set. */
		if (!is_support) {
			if (*(std_timing + offset) != 0x01 &&
				 *(std_timing + offset + 1) != 0x01)
				pr_debug("%s:removed 0x%2x, 0x%2x\n",
				 __func__,
				 *(std_timing + offset),
				 *(std_timing + offset + 1));
		} else {
			std_timing_tmp[hit_cnt++] = *(std_timing + offset);
			std_timing_tmp[hit_cnt++] = *(std_timing + offset + 1);
		}

		offset += 2;
	}

	memcpy(std_timing, std_timing_tmp, 16);
}

void mhl_lib_edid_remove_established_timing(
	const struct mhl_video_timing_info *support_video,
	uint8_t length,
	uint8_t *est_timing)
{
	int i = 0;

	uint8_t est_timing_1 = 0;
	uint8_t est_timing_2 = 0;
	uint8_t est_timing_3 = 0;
	uint8_t est_timing_1_result = 0;
	uint8_t est_timing_2_result = 0;
	uint8_t est_timing_3_result = 0;

	if (length > MATCHED_TIMING_FLAG_LEN_MAX) {
		pr_err("exceeds flag array max\n");
		return;
	}

	for (i = 0; i < length; i++) {
		est_timing_1 =
			*est_timing & (support_video + i)->est_timing_1;
		est_timing_2 =
			*(est_timing + 1) & (support_video + i)->est_timing_2;
		est_timing_3 =
			*(est_timing + 2) & (support_video + i)->est_timing_3;

		if (est_timing_1 | est_timing_2 | est_timing_3)
			/* set SOMC support flg */
			matched_timing_flg[i] = true;

		est_timing_1_result |= est_timing_1;
		est_timing_2_result |= est_timing_2;
		est_timing_3_result |= est_timing_3;
	}
	*est_timing = est_timing_1_result;
	*(est_timing + 1) = est_timing_2_result;
	*(est_timing + 2) = est_timing_3_result;

	return;
}


/*
 * Replace the preferred display data in blk0 with edid.
 *
 * blk0 : pointer to the head of the block 0
 * edid : the data will replace the current preferred display info
 *  in block 0 (18 byte from 0x36 byte in blk0).
 */
static void mhl_lib_edid_replace_dtd_preferred_disp_info(
		uint8_t *preferred_disp_info,
		const struct mhl_video_timing_info *video_mode_param)
{
	uint8_t *disp_info = preferred_disp_info;
	uint32_t h_image_size = 0;
	uint32_t v_image_size = 0;
	uint32_t active_h;
	uint32_t active_v;

	pr_debug("%s()\n", __func__);

	if (video_mode_param->active_h > 4095) {
		active_h = 4095;
		pr_warn("%s() active_h is over 4095\n", __func__);
	} else
		active_h = video_mode_param->active_h;

	if (video_mode_param->active_v > 4095) {
		active_v = 4095;
		pr_warn("%s() active_v is over 4095\n", __func__);
	} else
		active_v = video_mode_param->active_v;


	disp_info[0x0] =
		(uint8_t)((video_mode_param->pixel_freq_khz1 / 10)
							& 0x000000FF);
	disp_info[0x1] =
		(uint8_t)(((video_mode_param->pixel_freq_khz1 / 10)
							& 0x0000FF00)>>8);
	disp_info[0x2] = (uint8_t)(active_h & 0x000000FF);
	disp_info[0x3] = (uint8_t)(video_mode_param->total_blank_h &
								0x000000FF);
	disp_info[0x4] = (uint8_t)((active_h & 0x00000F00) >> 4) |
		(uint8_t)((video_mode_param->total_blank_h & 0x00000F00) >> 8);
	disp_info[0x5] = (uint8_t)(active_v & 0x000000FF);
	disp_info[0x6] = (uint8_t)(video_mode_param->total_blank_v &
								0x000000FF);
	disp_info[0x7] = (uint8_t)((active_v & 0x00000F00) >> 4) |
		(uint8_t)((video_mode_param->total_blank_v & 0x00000F00) >> 8);

	/*
	 * aspect ratio in original disp info is changed to video_mode_param's one.
	 * e.g.
	 * 	if original is 16:9 and video_mode_param is 4:3, then the original
	 * 	data will be replaced with 4:3 parameter.
	 */

	/* set original image size of disp info in blk0 */
	h_image_size = (uint32_t)(((disp_info[0xE] & 0xF0) << 4) | disp_info[0xC]);
	v_image_size = (uint32_t)(((disp_info[0xE] & 0x0F) << 8) | disp_info[0xD]);

	if (video_mode_param->aspect_ratio_4_3) {
		/* 4:3 v size is calculated */
		if (h_image_size > (v_image_size * 4 / 3))
			/* original does not indicate 4:3,
			 * so the v size is replaced with 4:3 */
			h_image_size = v_image_size * 4 / 3;
	} else {
		/* 16:9 v size is calculated */
		if (v_image_size > (h_image_size * 9 / 16))
			/* v_image_size is probably 4:3 */
			/* so replace it with 16:9 */
			v_image_size = h_image_size * 9 / 16;
	}

	disp_info[0xC] = (uint8_t)(h_image_size & 0x00000FF);
	disp_info[0xD] = (uint8_t)(v_image_size & 0x00000FF);
	disp_info[0xE] = (uint8_t)(((h_image_size & 0x00000F00) >> 4) | ((v_image_size & 0x00000F00) >> 8));

	return;
}

/*
 * TODO the API should be called after the all edid block filter, otherwise,
 * prefferred disp info can't contain best video timing.
 */
void mhl_lib_edid_remove_and_replace_detailed_timing(
			const struct mhl_video_timing_info *support_video,
			uint8_t support_video_len,
			uint8_t *detailed_timing)
{
	struct mhl_video_timing_info best_timing;

	/* remove unsupport detailed_timing except for 1st 18byte descripter */
	mhl_lib_edid_remove_unsupp_detailed_timing(support_video,
						support_video_len,
						detailed_timing);

	/* select best timing */
	best_timing =
		mhl_lib_edid_get_best_support_timing(support_video,
						support_video_len);

	/* replace preferred_disp_info with best_timing */
	mhl_lib_edid_replace_dtd_preferred_disp_info(&detailed_timing[0],
						&best_timing);
}

/*
 * hdmi_edid_disp_mode_lut must contain each list with high quality order.
 * (Hi quality if the index is large)
 *
 * data_buf : pointer to the one dtd block
 * disp_mode : display mode (VIC id defined in mhl_lib_edid.h).
 * 				 If no disp info in data_buf is not found
 * 				 in hdmi_edid_disp_mode_lut, then
 * 				 HDMI_VFRMT_FORCE_32BIT is returned.
 * hdmi_edid_disp_mode_lut : support display info.
 * mode_lut_len : length of edid (how many instance of the struct are contained.)
 * most_hi_quality_disp_mode_index : NULL, then nothing to be done.
 * 				 otherwise most high quality disp info of hdmi_edid_disp_mode_lut is returned.
 */
static bool mhl_lib_edid_is_supp_disp_info_in_detailed_timing(
			const struct mhl_video_timing_info *support_video,
			uint8_t support_video_len,
			uint8_t *data_buf)
{
	uint32_t	interlace_flg       = false;
	uint32_t	h_active            = 0;
	uint32_t	v_active            = 0;
	uint32_t	h_blank             = 0;
	uint32_t	v_blank             = 0;
	uint32_t	index               = 0;
	uint32_t	h_img_size          = 0;
	uint32_t	v_img_size          = 0;
	uint32_t	khz_pixel_freq      = 0;

	pr_debug("%s()\n", __func__);
	PRINT_DATA(data_buf, 18, "data buf")

	if (support_video_len > MATCHED_TIMING_FLAG_LEN_MAX) {
		pr_err("exceeds flag array max\n");
		return false;
	}


	/*
	 * * See VESA Spec
	 * * EDID_TIMING_DESC_UPPER_H_NIBBLE[0x4]: Relative Offset to the
	 *   EDID detailed timing descriptors - Upper 4 bit for each H
	 *   active/blank field
	 * * EDID_TIMING_DESC_H_ACTIVE[0x2]: Relative Offset to the EDID
	 *   detailed timing descriptors - H active
	 */
	h_active = ((((uint32_t)data_buf[0x4] >> 0x4) & 0xF) << 8)
		| data_buf[0x2];

	/*
	 * EDID_TIMING_DESC_H_BLANK[0x3]: Relative Offset to the EDID detailed
	 *   timing descriptors - H blank
	 */
	h_blank = (((uint32_t)data_buf[0x4] & 0xF) << 8)
		| data_buf[0x3];

	/*
	 * * EDID_TIMING_DESC_UPPER_V_NIBBLE[0x7]: Relative Offset to the
	 *   EDID detailed timing descriptors - Upper 4 bit for each V
	 *   active/blank field
	 * * EDID_TIMING_DESC_V_ACTIVE[0x5]: Relative Offset to the EDID
	 *   detailed timing descriptors - V active
	 */
	v_active = ((((uint32_t)data_buf[0x7] >> 0x4) & 0xF) << 8)
		| data_buf[0x5];

	/*
	 * EDID_TIMING_DESC_V_BLANK[0x6]: Relative Offset to the EDID
	 * detailed timing descriptors - V blank
	 */
	v_blank = (((uint32_t)data_buf[0x7] & 0xF) << 8)
		| data_buf[0x6];

	/*
	 * * EDID_TIMING_DESC_IMAGE_SIZE_UPPER_NIBBLE[0xE]: Relative Offset
	 *   to the EDID detailed timing descriptors - Image Size upper
	 *   nibble V and H
	 * * EDID_TIMING_DESC_H_IMAGE_SIZE[0xC]: Relative Offset to the EDID
	 *   detailed timing descriptors - H image size
	 * * EDID_TIMING_DESC_V_IMAGE_SIZE[0xD]: Relative Offset to the EDID
	 *   detailed timing descriptors - V image size
	 */
	h_img_size = ((((uint32_t)data_buf[0xE] >> 0x4) & 0xF) << 8)
		| data_buf[0xC];
	v_img_size = (((uint32_t)data_buf[0xE] & 0xF) << 8)
		| data_buf[0xD];

	/*
	 * EDID_TIMING_DESC_INTERLACE[0x11:7]: Relative Offset to the EDID
	 * detailed timing descriptors - Interlace flag
	 */
	pr_debug("%s: Interlaced mode byte data_buf[0x11]=[%x]\n", __func__,
		data_buf[0x11]);

	/*
	 * CEA 861-D: interlaced bit is bit[7] of byte[0x11]
	 */
	interlace_flg = (data_buf[0x11] & 0x80) >> 7;

	/*
	 * Pixel clock: convert to decimal KHz from hex MHz
	 */
	khz_pixel_freq = (uint32_t)((data_buf[1] << 8) + data_buf[0]) * 10;


	pr_debug("%s: A[%ux%u] B[%ux%u] V[%ux%u] %s pixel_freq = %d\n",
		__func__,
		h_active, v_active, h_blank, v_blank, h_img_size, v_img_size,
		interlace_flg ? "i" : "p", khz_pixel_freq);

	while (index < support_video_len) {
		const struct mhl_video_timing_info *edid = &support_video[index];

		if ((interlace_flg    == edid->interlaced)    &&
			(h_active  == edid->active_h)      &&
			(h_blank   == edid->total_blank_h) &&
			(v_blank   == edid->total_blank_v) &&
			((v_active == edid->active_v) ||
			(v_active  == (edid->active_v + 1))) &&
			((khz_pixel_freq == edid->pixel_freq_khz1) ||
			(khz_pixel_freq == edid->pixel_freq_khz2))) {
			/* set SOMC support flg */
			matched_timing_flg[index] = true;
			pr_debug("%s: mode found:%d\n", __func__, edid->vic);
			return true;
		}
		++index;
	}

	return false;
}

uint32_t mhl_lib_edid_get_16_link_clk(uint32_t pixel_clock)
{
	/* note : "10" is not multiplied
	 * since uint32 does not have enough size */
	return pixel_clock / 8 * 16;
}

uint32_t mhl_lib_edid_get_24_link_clk(uint32_t pixel_clock)
{
	/* note : "10" is not multiplied
	 * since uint32 does not have enough size */
	return pixel_clock / 8 * 24;
}

/*
 * Unsupported 4k vic is removed from the input somc_support_vic.
 * somc_support_vic : input and output for somc support vic.
 * sink_support_4k_vic sink : support 4k vic. Sink sends the information
 * with WB.
 */
uint8_t mhl_lib_edid_remove_unsupport_4k_vic(
			struct mhl_video_timing_info *somc_support_vic,
			uint8_t somc_support_vic_len,
			const uint8_t *sink_support_4k_vic,
			uint8_t sink_support_4k_vic_len)
{
	int i, j;
	uint8_t ret_length = 0;
	bool flg;

	for (i = 0; i < somc_support_vic_len; i++) {
		if ((somc_support_vic[i].vic >= START_4K_VIC) &&
			(somc_support_vic[i].vic <= END_4K_VIC)) {

			flg = false;
			/* serch sink support 4k vic */
			for (j = 0; j < sink_support_4k_vic_len; j++) {
				if (sink_support_4k_vic == NULL)
					break;

				if (somc_support_vic[i].vic ==
						sink_support_4k_vic[j]) {
					flg = true;
					break;
				}
			}

			/* if the specific 4k vic is not supported, */
			/* then the vic is replaced with "0". */
			/* (only unsuport 4k vic is removed) */
			if (flg)
				ret_length++;
			else {
				pr_debug("%s: removed from support vic %d\n",
					__func__, somc_support_vic[i].vic);
				/* set 0 to remove after */
				memset(&somc_support_vic[i],
					0,
					sizeof(struct mhl_video_timing_info));
			}
		} else
			/* not 4k vic */
			ret_length++;
	}

	/* remove 0 element */
	/* Eg [1, 2, 4, 32, 34, 16, 0, 95] -> [1, 2, 4, 32, 34, 16, 95, 0] */
	for (i = 0; i < ret_length; i++) {
		if (somc_support_vic[i].vic != 0)
			continue;

		for (j = i + 1; j < somc_support_vic_len; j++) {
			if (somc_support_vic[j].vic != 0) {
				pr_debug("vic = %d\n", somc_support_vic[j].vic);
				somc_support_vic[i] =
						somc_support_vic[j];
				memset(&somc_support_vic[j],
					0,
					sizeof(struct mhl_video_timing_info));
				break;
			}
		}
	}

	return ret_length;
}

bool *mhl_lib_edid_get_matched_timing_flg(void)
{
	return matched_timing_flg;
}

/*
 * Remove unsupported video timing info except for 1st 18 byte descripter
 * (preferred disp info).
 * 1st 18 byte descriper is always not chnaged in this API because the 1st
 * descripter contains SINK device HW dependant information. For example,
 * phisical display size (h/w) . Such some parameters will be reused even if
 * video timing info is changed in the 1st descripter.
 */
static void mhl_lib_edid_remove_unsupp_detailed_timing(
			const struct mhl_video_timing_info *support_video,
			uint8_t support_video_len,
			uint8_t *detailed_timing)
{
	uint32_t desc_offset = 0;
	int i = 0;

	pr_debug("%s()\n", __func__);

	/* if pixel clock(detailed_timing[0, 1]) is 0, */
	/* it is considered dummy block. */
	/* TODO: this is tentative implimentation. */
	/* please check EDID specifiation. */
	while (4 > i &&
		(0 != detailed_timing[desc_offset]) &&
		(0 != detailed_timing[desc_offset + 1])) {
		bool is_support = false;

		is_support = mhl_lib_edid_is_supp_disp_info_in_detailed_timing(
				support_video,
				support_video_len,
				&detailed_timing[desc_offset]);

		if (!is_support) {
			/* preferred display info (i == 0) */
			/* is not removed here */
			if (i != 0) {
				/* remove detailed_timing */
				memset(&detailed_timing[desc_offset], 0x00, 18);
				detailed_timing[desc_offset + 0x03] = 0x10;
			}
			pr_debug("%s: unsupported, cont i : %d\n",
							__func__, i);
		} else
			pr_debug("%s: supported, cont i : %d\n",
							__func__, i);

		desc_offset += 18;
		++i;
	}
}

static struct mhl_video_timing_info mhl_lib_edid_get_best_support_timing(
			const struct mhl_video_timing_info *support_video,
			uint8_t support_video_len)
{
	int i = 0;

	for (i = support_video_len - 1; i >= 0; --i) {
		if (matched_timing_flg[i])
			return support_video[i];
	}

	return support_video[0];
}

void mhl_lib_edid_remove_unsupp_detailed_timing_from_ext_blk(
			uint8_t *ext_edid,
			const struct mhl_video_timing_info *support_video,
			uint8_t support_video_len)
{
	PCEA_extension_t extension = (PCEA_extension_t)ext_edid;
	uint32_t desc_offset;
	uint32_t last_pos;
	int i;

	pr_debug("%s()\n", __func__);

	desc_offset = ext_edid[2];

	if (desc_offset < 4)
		return;

	last_pos = 127;

	i = 0;
	while (desc_offset + 18 <= last_pos &&
		(0 != ext_edid[desc_offset]) &&
		(0 != ext_edid[desc_offset + 1]) &&
		i < 6) {
		bool is_support = false;

		is_support = mhl_lib_edid_is_supp_disp_info_in_detailed_timing(
				support_video,
				support_video_len,
				&ext_edid[desc_offset]);

		if (!is_support) {
			if (last_pos - (desc_offset + 18) >= 18) {
				/* timing is not last */
				/* next timing exists */

				memcpy(&ext_edid[desc_offset], &ext_edid[desc_offset + 18],
					last_pos - (desc_offset + 18));

				last_pos -= 18;
			} else
				/* timing is last */
				memset(&ext_edid[desc_offset], 0x00, 18);

			pr_debug("%s: unsupported, cont i : %d\n", __func__, i);
		} else {
			desc_offset += 18;
			pr_debug("%s: supported, cont i : %d\n", __func__, i);
		}

		i++;
	}

	if (last_pos < 127)
		memset(&ext_edid[last_pos], 0x00, 127 - last_pos);

	/* check sum */
	extension->checksum = 0x00;
	extension->checksum = calculate_generic_checksum(
		(uint8_t *)extension,
		 0x00,
		 sizeof(*extension));
}

/*
 * *ext_edid         : pointer to ExtBlock.
 * *hev_vic_array    : pointer to HEV VIC that you want to insert.
 *                     must not include non-HEV VIC.
 * hev_vic_array_len : length of hev_vic_array.
 *
 * return true when insert succeeds.
 */
bool mhl_lib_edid_insert_hev_vic(
			uint8_t *ext_edid,
			uint8_t *hev_vic_array,
			uint8_t hev_vic_array_len)
{
	int i, j;
	uint8_t *vdb;
	uint8_t vdb_len;
	uint8_t insert_array[HEV_VIC_MAX_LEN];
	uint8_t insert_array_len = 0;
	uint8_t dtd_offset;
	uint8_t dtd_tail;
	uint8_t *check_sum_addr;
	uint8_t *move_addr;
	PCEA_extension_t extension = (PCEA_extension_t)ext_edid;

	/* check whether argument is correct */
	if (ext_edid == NULL) {
		pr_err("%s: argment error.\n", __func__);
		return false;
	}

	/* check insert item */
	if ((hev_vic_array_len == 0) || (hev_vic_array == NULL)) {
		pr_info("%s: no insert item.\n", __func__);
		return false;
	}
	if (hev_vic_array_len > HEV_VIC_MAX_LEN) {
		pr_info("%s: insert item is over %d.\n",
					__func__, HEV_VIC_MAX_LEN);
		return false;
	}
	for (i = 0; i < hev_vic_array_len; i++) {
		for (j = 0; j < HEV_VIC_MAX_LEN; j++) {
			if (hev_vic_array[i] == defined_hev_vic[j])
				break;
		}
		if (j >= HEV_VIC_MAX_LEN) {
			pr_info("%s: non-HEV VIC detect.\n", __func__);
			return false;
		}
	}

	/* check whether there is VideoDataBlock */
	vdb = (uint8_t *)hdmi_edid_find_block_v2(
		(const uint8_t *)ext_edid,
		 1,
		 MHL_LIB_DBC_START_OFFSET,
		 VIDEO_DATA_BLOCK,
		 &vdb_len);
	if ((vdb == NULL) || (vdb_len == 0)) {
		pr_info("%s: VideoDataBlock is nothing.\n", __func__);
		return false;
	}

	/* check whether there is HEV VIC in VideoDataBlock */
	for (i = 0; i < hev_vic_array_len; i++) {
		for (j = 1; j <= vdb_len; j++) {
			if (hev_vic_array[i] == (0x7F & vdb[j]))
				break;
		}
		if (j > vdb_len) {
			insert_array[insert_array_len++] = hev_vic_array[i];
			pr_info("%s: insert HEV VIC is %d\n",
					__func__, insert_array[insert_array_len - 1]);
		}
	}
	if (insert_array_len == 0) {
		pr_info("%s: insert HEV VIC is nothing.\n", __func__);
		return false;
	}
	if (insert_array_len + vdb_len > VIC_NUM_MAX) {
		pr_info("%s: too many vic. nothing to do.\n", __func__);
		return false;
	}

	/* check whether overflow occurs if insert */
	dtd_offset = ext_edid[0x02];
	dtd_tail = dtd_offset + 17;
	while ((dtd_tail < (EDID_BLOCK_SIZE - 1)) &&
			((ext_edid[dtd_offset] != 0) &&
				(ext_edid[dtd_offset + 1] != 0))) {
		dtd_offset += 18;
		dtd_tail = dtd_offset + 17;
	}
	if ((dtd_offset - 1 + insert_array_len) >= (EDID_BLOCK_SIZE - 1)) {
		pr_info("%s: overflow occurs, so does not insert.\n",
				__func__);
		return false;
	}

	/* alloc space of insert data */
	check_sum_addr = ext_edid + EDID_BLOCK_SIZE - 1;
	move_addr = check_sum_addr - insert_array_len - 1;
	while (move_addr > (vdb + vdb_len)) {
		*(move_addr + insert_array_len) = *move_addr;
		move_addr--;
	}

	/* insert HEV_VIC to VideoDataBlock */
	memcpy(vdb + vdb_len + 1, insert_array, insert_array_len);

	/* add insert length to VideoDataBlock */
	set_data_block_length(vdb, vdb_len + insert_array_len);

	/* add insert length to ByteNumberOffset */
	ext_edid[0x02] += insert_array_len;

	/* check sum */
	extension->checksum = 0x00;
	extension->checksum = calculate_generic_checksum(
		(uint8_t *)extension,
		 0x00,
		 sizeof(*extension));

	/* success */
	return true;
}
