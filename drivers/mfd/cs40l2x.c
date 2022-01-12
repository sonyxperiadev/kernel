// SPDX-License-Identifier: GPL-2.0
//
// cs40l2x.c  --  ALSA SoC Audio driver for Cirrus Logic CS40L2x
//
// Copyright 2018 Cirrus Logic Inc.
// Author: Jeff LaBundy <jeff.labundy@cirrus.com>

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/string.h>
#include <linux/workqueue.h>
#include <linux/regulator/consumer.h>
#include <linux/regmap.h>
#include <linux/sysfs.h>
#include <linux/firmware.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/hrtimer.h>
#include <linux/mfd/core.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/mfd/cs40l2x.h>

#ifdef CONFIG_ANDROID_TIMED_OUTPUT
#include "../staging/android/timed_output.h"
#else
#include <linux/leds.h>
#endif /* CONFIG_ANDROID_TIMED_OUTPUT */

static const char * const cs40l2x_supplies[] = {
	"VA",
	"VP",
};

static const char * const cs40l2x_part_nums[] = {
	"CS40L20",
	"CS40L25",
	"CS40L25A",
	"CS40L25B",
};

static const char * const cs40l2x_event_regs[] = {
	"GPIO1EVENT",
	"GPIO2EVENT",
	"GPIO3EVENT",
	"GPIO4EVENT",
	"GPIOPLAYBACKEVENT",
	"TRIGGERPLAYBACKEVENT",
	"RXREADYEVENT",
	"HARDWAREEVENT",
};

static const unsigned int cs40l2x_event_masks[] = {
	CS40L2X_EVENT_GPIO1_ENABLED,
	CS40L2X_EVENT_GPIO2_ENABLED,
	CS40L2X_EVENT_GPIO3_ENABLED,
	CS40L2X_EVENT_GPIO4_ENABLED,
	CS40L2X_EVENT_START_ENABLED | CS40L2X_EVENT_END_ENABLED,
	CS40L2X_EVENT_START_ENABLED | CS40L2X_EVENT_END_ENABLED,
	CS40L2X_EVENT_READY_ENABLED,
	CS40L2X_EVENT_HARDWARE_ENABLED,
};

static int cs40l2x_raw_write(struct cs40l2x_private *cs40l2x, unsigned int reg,
			const void *val, size_t val_len, size_t limit);

static int cs40l2x_hw_err_rls(struct cs40l2x_private *cs40l2x,
			unsigned int irq_mask);
static int cs40l2x_hw_err_chk(struct cs40l2x_private *cs40l2x);

static int cs40l2x_basic_mode_exit(struct cs40l2x_private *cs40l2x);

static int cs40l2x_firmware_swap(struct cs40l2x_private *cs40l2x,
			unsigned int fw_id);
static int cs40l2x_wavetable_swap(struct cs40l2x_private *cs40l2x,
			const char *wt_file);
static int cs40l2x_wavetable_sync(struct cs40l2x_private *cs40l2x);

static const struct cs40l2x_fw_desc *cs40l2x_firmware_match(
			struct cs40l2x_private *cs40l2x, unsigned int fw_id)
{
	int i;

	for (i = 0; i < CS40L2X_NUM_FW_FAMS; i++)
		if (cs40l2x_fw_fam[i].id == fw_id)
			return &cs40l2x_fw_fam[i];

	dev_err(cs40l2x->dev, "No matching firmware for ID 0x%06X\n", fw_id);

	return NULL;
}

static struct cs40l2x_private *cs40l2x_get_private(struct device *dev)
{
#ifdef CONFIG_ANDROID_TIMED_OUTPUT
	/* timed output device does not register under a parent device */
	return container_of(dev_get_drvdata(dev),
			struct cs40l2x_private, timed_dev);
#else
	return dev_get_drvdata(dev);
#endif /* CONFIG_ANDROID_TIMED_OUTPUT */
}

static void cs40l2x_sysfs_notify(struct cs40l2x_private *cs40l2x,
		const char *attr)
{
	struct kobject *kobj;

#ifdef CONFIG_ANDROID_TIMED_OUTPUT
	kobj = &cs40l2x->timed_dev.dev->kobj;
#else
	kobj = &cs40l2x->dev->kobj;
#endif /* CONFIG_ANDROID_TIMED_OUTPUT */

	sysfs_notify(kobj, NULL, attr);
}

static void cs40l2x_set_state(struct cs40l2x_private *cs40l2x, bool state)
{
	if (cs40l2x->vibe_state != state) {
		cs40l2x->vibe_state = state;
		cs40l2x_sysfs_notify(cs40l2x, "vibe_state");
	}
}

static void cs40l2x_set_safe_save_state(struct cs40l2x_private *cs40l2x,
	bool state)
{
	if (cs40l2x->safe_save_state != state) {
		cs40l2x->safe_save_state = state;
		cs40l2x_sysfs_notify(cs40l2x, "safe_save_state");
	}
}

static int cs40l2x_check_wt_open_space(struct cs40l2x_private *cs40l2x,
	unsigned int size)
{
	unsigned int default_empty_ym_size = (cs40l2x->wt_limit_ym -
		CS40L2X_WT_YM_EMPTY_SIZE);

	/* If YM exists, must add to end of YM to keep index order */
	if (cs40l2x->wt_open_ym < default_empty_ym_size) {
		if (size <= cs40l2x->wt_open_ym) {
			/* Add to end of existing YM */
			cs40l2x->create_ym = false;
			cs40l2x->xm_append = false;
			return 0;
		}
		/* YM exists, requested size too big */
		if (size > cs40l2x->wt_open_ym)
			return -ENOSPC;
	}
	if (size <=	cs40l2x->wt_open_xm) {
		/* Add to end of existing XM */
		cs40l2x->create_ym = false;
		cs40l2x->xm_append = true;
		return 0;
	}
	if (size > cs40l2x->wt_open_xm) {
		/* Create YM section and add to YM */
		cs40l2x->create_ym = true;
		cs40l2x->xm_append = false;
		return 0;
	}

	return -ENOSPC;
}

static int cs40l2x_calc_composite_size(struct cs40l2x_private *cs40l2x,
	unsigned int *size)
{
	unsigned int wav_count = 0;
	unsigned int total_size = 0;

	*size = total_size;

	if (cs40l2x->pbq_fw_composite_len == 0)
		return -ENODATA;

	wav_count = cs40l2x->pbq_fw_composite[2];

	/* Wvfrm Samples, Repeat, Num of Wavs, Empty Byte, Zero Pad */
	total_size = CS40L2X_WT_COMP_NONRPTNG_SIZE;

	if (cs40l2x->comp_dur_en)
		total_size += (wav_count * CS40L2X_WT_COMP_WV_DTLS);
	else
		total_size += (wav_count * CS40L2X_WT_COMP_LEGACY_DTLS);

	if (total_size > (cs40l2x->comp_bytes / CS40L2X_WT_NUM_VIRT_SLOTS)) {
		dev_err(cs40l2x->dev, "Waveform size exceeds available space\n");
		return -ENOSPC;
	}

	*size = total_size;

	return 0;
}

static void cs40l2x_set_ym_data(struct cs40l2x_private *cs40l2x)
{
	cs40l2x->ym_hdr_strt_pos = (cs40l2x->wt_xm_size +
		CS40L2X_WT_YM_PRE_HDR_BYTES +
		CS40L2X_WT_DBLK_LENGTH_SIZE);
	cs40l2x->wt_ym_size = (CS40L2X_WT_YM_PRE_HDR_BYTES +
		CS40L2X_WT_DBLK_LENGTH_SIZE +
		(CS40L2X_WT_HEADER_ENTRY_SIZE *
		CS40L2X_WT_NUM_VIRT_SLOTS) +
		CS40L2X_WT_TERMINATOR_BYTES +
		cs40l2x->comp_bytes);
}

static int cs40l2x_create_wvfrm_len_type_pairs(struct cs40l2x_private *cs40l2x)
{
	unsigned int wt_file_type;
	unsigned int wt_offset = 0;
	unsigned int wt_length = 0;
	unsigned int waveform_length;
	unsigned int cleared_extra_bits;
	unsigned int pos, offset_pos;
	int i, wt_entry_words;
	int count = 0;

	if (memcmp(cs40l2x->pbq_fw_raw_wt, "WMDR", 4)) {
		dev_err(cs40l2x->dev, "Failed to recognize raw wavetable\n");
		return -ENODATA;
	}

	if (cs40l2x->create_ym)
		cs40l2x_set_ym_data(cs40l2x);

	memset(&cs40l2x->wvfrm_lengths[0], 0,
		cs40l2x->wvfrm_lengths_size);

	cs40l2x->updated_offsets_size = cs40l2x->num_waves;

	pos = cs40l2x->xm_hdr_strt_pos;
	for (i = 0; i < cs40l2x->num_xm_wavs; i++) {
		wt_file_type = (cs40l2x->pbq_fw_raw_wt[pos] << 24)
			+ (cs40l2x->pbq_fw_raw_wt[pos + 1] << 16)
			+ (cs40l2x->pbq_fw_raw_wt[pos + 2] << 8)
			+ (cs40l2x->pbq_fw_raw_wt[pos + 3]);
		cleared_extra_bits = (wt_file_type & CS40L2X_WT_CLR_EX_TYPE);
		cs40l2x->wvfrm_lengths[count] = cleared_extra_bits;

		pos += 4;
		wt_offset = (cs40l2x->pbq_fw_raw_wt[pos] << 24)
			+ (cs40l2x->pbq_fw_raw_wt[pos + 1] << 16)
			+ (cs40l2x->pbq_fw_raw_wt[pos + 2] << 8)
			+ (cs40l2x->pbq_fw_raw_wt[pos + 3]);
		/* Add words to updated offset for new entry.
		 * Wavetable entries are in words.
		 */
		wt_entry_words = ((CS40L2X_WT_HEADER_ENTRY_SIZE *
			CS40L2X_WT_NUM_VIRT_SLOTS) / 4);
		cs40l2x->updated_offsets[i] = wt_offset + wt_entry_words;

		pos += 4;
		wt_length = (cs40l2x->pbq_fw_raw_wt[pos] << 24)
			+ (cs40l2x->pbq_fw_raw_wt[pos + 1] << 16)
			+ (cs40l2x->pbq_fw_raw_wt[pos + 2] << 8)
			+ (cs40l2x->pbq_fw_raw_wt[pos + 3]);

		offset_pos = cs40l2x->xm_hdr_strt_pos +
			(wt_offset * 4);
		waveform_length =
			(cs40l2x->pbq_fw_raw_wt[offset_pos] << 24)
			+ (cs40l2x->pbq_fw_raw_wt[offset_pos + 1] << 16)
			+ (cs40l2x->pbq_fw_raw_wt[offset_pos + 2] << 8)
			+ (cs40l2x->pbq_fw_raw_wt[offset_pos + 3]);
		cs40l2x->wvfrm_lengths[count + 1] = waveform_length;
		count = count + 2;
		pos += 4;
	}
	cs40l2x->wt_xm_header_end_pos = pos;
	cs40l2x->wt_xm_header_last_offset = wt_offset;
	cs40l2x->wt_xm_header_last_size = wt_length;

	if (cs40l2x->num_ym_wavs > 0) {
		pos = cs40l2x->ym_hdr_strt_pos;
		for (i = cs40l2x->num_xm_wavs;
			i < (cs40l2x->num_xm_wavs +
				cs40l2x->num_ym_wavs); i++) {
			wt_file_type = (cs40l2x->pbq_fw_raw_wt[pos] << 24)
				+ (cs40l2x->pbq_fw_raw_wt[pos + 1] << 16)
				+ (cs40l2x->pbq_fw_raw_wt[pos + 2] << 8)
				+ (cs40l2x->pbq_fw_raw_wt[pos + 3]);
			cleared_extra_bits =
				(wt_file_type & CS40L2X_WT_CLR_EX_TYPE);
			cs40l2x->wvfrm_lengths[count] = cleared_extra_bits;

			pos += 4;
			wt_offset = (cs40l2x->pbq_fw_raw_wt[pos] << 24)
				+ (cs40l2x->pbq_fw_raw_wt[pos + 1] << 16)
				+ (cs40l2x->pbq_fw_raw_wt[pos + 2] << 8)
				+ (cs40l2x->pbq_fw_raw_wt[pos + 3]);
			/* Add words to updated offset for new entry.
			 * Wavetable entries are in words.
			 */
			wt_entry_words = ((CS40L2X_WT_HEADER_ENTRY_SIZE *
				CS40L2X_WT_NUM_VIRT_SLOTS) / 4);
			cs40l2x->updated_offsets[i] = wt_offset +
				wt_entry_words;

			pos += 4;
			wt_length = (cs40l2x->pbq_fw_raw_wt[pos] << 24)
				+ (cs40l2x->pbq_fw_raw_wt[pos + 1] << 16)
				+ (cs40l2x->pbq_fw_raw_wt[pos + 2] << 8)
				+ (cs40l2x->pbq_fw_raw_wt[pos + 3]);

			offset_pos = cs40l2x->ym_hdr_strt_pos +
				(wt_offset * 4);
			waveform_length =
				(cs40l2x->pbq_fw_raw_wt[offset_pos] << 24)
				+ (cs40l2x->pbq_fw_raw_wt[offset_pos + 1] << 16)
				+ (cs40l2x->pbq_fw_raw_wt[offset_pos + 2] << 8)
				+ (cs40l2x->pbq_fw_raw_wt[offset_pos + 3]);
			cs40l2x->wvfrm_lengths[count + 1] = waveform_length;
			count = count + 2;
			pos += 4;
		}
		cs40l2x->wt_ym_header_end_pos = pos;
		cs40l2x->wt_ym_header_last_offset = wt_offset;
		cs40l2x->wt_ym_header_last_size = wt_length;
	}

	return 0;
}

static int cs40l2x_calc_index_samples(struct cs40l2x_private *cs40l2x,
	unsigned int *index_samples)
{
	unsigned int index, factor, len_indx, actual_wvfrm_len;
	unsigned int total_wvfrm_len = 0;
	int i;

	for (i = 0; i < cs40l2x->comp_sets_size; i++) {
		if ((!cs40l2x->comp_sets[i].index) &&
			(!cs40l2x->comp_sets[i].amp) &&
			(!cs40l2x->comp_sets[i].dur))
			continue;
		index = cs40l2x->comp_sets[i].index;
		factor = (cs40l2x->comp_sets[i].rpt + 1);
		len_indx = index * 2;
		/* First check for indefinite PWLE */
		if ((cs40l2x->wvfrm_lengths[len_indx + 1] -
			(CS40L2X_WT_COMP_INDEFINITE +
			CS40L2X_WT_COMP_LEN_CALCD)) != 0) {
			/* Make sure not index 0, not Q-Factor,
			 * not composite and bit 23 is set in length
			 */
			if ((cs40l2x->wvfrm_lengths[len_indx] != 0) &&
				(cs40l2x->wvfrm_lengths[len_indx] !=
					CS40L2X_WT_TYPE_11_Q_FILE) &&
				(cs40l2x->wvfrm_lengths[len_indx] !=
					CS40L2X_WT_TYPE_10_COMP_FILE) &&
				(cs40l2x->wvfrm_lengths[len_indx + 1] >
					CS40L2X_WT_COMP_LEN_CALCD)) {
				actual_wvfrm_len =
					(cs40l2x->wvfrm_lengths[len_indx + 1] -
					CS40L2X_WT_COMP_LEN_CALCD);
				if (cs40l2x->comp_sets[i].dur)
					actual_wvfrm_len =
						(cs40l2x->comp_sets[i].dur * 8);
				total_wvfrm_len +=
					(actual_wvfrm_len * factor);
			} else {
				dev_err(cs40l2x->dev,
					"Invalid pbq value detected\n");
				return -EINVAL;
			}
		} else {
			if (cs40l2x->comp_sets[i].dur) {
				actual_wvfrm_len =
					(cs40l2x->comp_sets[i].dur * 8);
			} else {
				dev_err(cs40l2x->dev,
					"Indefinite pbq must have duration.\n");
				return -EINVAL;
			}
			total_wvfrm_len +=
				(actual_wvfrm_len * factor);
		}
	}
	*index_samples = total_wvfrm_len;

	return 0;
}

static void cs40l2x_calc_delay_samples(struct cs40l2x_private *cs40l2x,
	unsigned int *delay_samples)
{
	unsigned int delay, factor;
	unsigned int total = 0;
	int i;

	for (i = 0; i < cs40l2x->comp_sets_size; i++) {
		if ((!cs40l2x->comp_sets[i].index) &&
			(!cs40l2x->comp_sets[i].amp) &&
			(!cs40l2x->comp_sets[i].dur)) {
			delay = cs40l2x->comp_sets[i].delay;
			factor = (cs40l2x->comp_sets[i].rpt + 1);
			total += (delay * factor);
		}
	}
	/* To get samples per ms */
	*delay_samples = (total * 8);
}

static int cs40l2x_calc_wvfrm_samples(struct cs40l2x_private *cs40l2x,
	unsigned int *wvfrm_samples)
{
	unsigned int index_samples;
	unsigned int delay_samples;
	unsigned int outer_loop_repeat;
	int pos = CS40L2X_WT_COMP_REPEAT_INDX;
	int ret;

	outer_loop_repeat = cs40l2x->pbq_fw_composite[pos];

	/* Infinite outer loop repeat */
	if (outer_loop_repeat == CS40L2X_WT_COMP_INDEF_OUTER) {
		*wvfrm_samples = (CS40L2X_WT_COMP_INDEFINITE +
			CS40L2X_WT_COMP_LEN_CALCD);
	} else {
		ret = cs40l2x_calc_index_samples(cs40l2x,
			&index_samples);

		if (ret)
			return ret;

		cs40l2x_calc_delay_samples(cs40l2x,
			&delay_samples);

		*wvfrm_samples = ((index_samples + delay_samples) *
			(outer_loop_repeat + 1));

		if (*wvfrm_samples > CS40L2X_WT_MAX_SAMPLES)
			return -EINVAL;

		*wvfrm_samples = (*wvfrm_samples + CS40L2X_WT_COMP_LEN_CALCD);
	}

	return 0;
}

static int cs40l2x_to_bytes_msb(unsigned int val, int len, char **byte_data)
{
	char two_bytes[2] = {0, 0};
	char three_bytes[3] = {0, 0, 0};
	int i;
	int count = 0;

	if ((len < 2) || (len > 3))
		return -EINVAL;

	for (i = (len - 1); i >= 0; i--) {
		if (len == 2)
			two_bytes[count] = (val >> (i * 8)) & 0xFF;
		if (len == 3)
			three_bytes[count] = (val >> (i * 8)) & 0xFF;
		count++;
	}

	if (len == 2) {
		(*byte_data)[0] = two_bytes[0];
		(*byte_data)[1] = two_bytes[1];
	}

	if (len == 3) {
		(*byte_data)[0] = three_bytes[0];
		(*byte_data)[1] = three_bytes[1];
		(*byte_data)[2] = three_bytes[2];
	}

	return 0;
}

static int cs40l2x_pack_wt_composite_data(struct cs40l2x_private *cs40l2x,
	unsigned int comp_size, char **raw_composite_data)
{
	char *two_bytes_data;
	char *three_bytes_data;
	char *unpadded_data;
	char *zero_pad_data;
	int zero_pad_count = 0;
	int count = 0;
	int i, j, k;

	unpadded_data = devm_kzalloc(cs40l2x->dev,
		(comp_size / 4 * 3), GFP_KERNEL);
	if (!unpadded_data)
		return -ENOMEM;

	zero_pad_data = devm_kzalloc(cs40l2x->dev, comp_size, GFP_KERNEL);
	if (!zero_pad_data)
		return -ENOMEM;

	cs40l2x->two_bytes[0] = 0;
	cs40l2x->two_bytes[1] = 0;
	two_bytes_data = cs40l2x->two_bytes;

	cs40l2x->three_bytes[0] = 0;
	cs40l2x->three_bytes[1] = 0;
	cs40l2x->three_bytes[2] = 0;
	three_bytes_data = cs40l2x->three_bytes;

	/* Wvfrm Length in samples */
	cs40l2x_to_bytes_msb(cs40l2x->pbq_fw_composite[0],
		CS40L2X_WT_WORD_SIZE, &three_bytes_data);
	for (i = 0; i < CS40L2X_WT_WORD_SIZE; i++) {
		unpadded_data[count] = three_bytes_data[i];
		count++;
	}

	/* Next word = MSB empty, num wvfrms, repeat */
	unpadded_data[count] = 0;
	count++;
	unpadded_data[count] = cs40l2x->pbq_fw_composite[2];
	count++;
	unpadded_data[count] = cs40l2x->pbq_fw_composite[1];
	count++;

	/* For each wvfrm in composite list there needs to be:
	 * Nested Repeat, Wvfrm Index, Amplitude, Delay.
	 * If composite duration is supported by FW,
	 * add a word for Duration after Delay.
	 */
	for (j = 3; j < cs40l2x->pbq_fw_composite_len; j++) {
		if (((j - 2) % 5) == 0) {
			if (cs40l2x->pbq_fw_composite[j]) {
				unpadded_data[count - 3] =
					CS40L2X_WT_COMP_DUR_EN_BIT;
				unpadded_data[count] = 0;
				count++;
				two_bytes_data[0] = 0;
				two_bytes_data[1] = 0;
				cs40l2x_to_bytes_msb(
					cs40l2x->pbq_fw_composite[j], 2,
					&two_bytes_data);
				unpadded_data[count] = two_bytes_data[0];
				count++;
				unpadded_data[count] = two_bytes_data[1];
				count++;
			}
		}
		if (((j - 1) % 5) == 0) {
			unpadded_data[count] = 0;
			count++;
			two_bytes_data[0] = 0;
			two_bytes_data[1] = 0;
			cs40l2x_to_bytes_msb(
				cs40l2x->pbq_fw_composite[j], 2,
				&two_bytes_data);
			unpadded_data[count] = two_bytes_data[0];
			count++;
			unpadded_data[count] = two_bytes_data[1];
			count++;
		} else if ((j % 5) == 0) {
			unpadded_data[count] = cs40l2x->pbq_fw_composite[j];
			count++;
			unpadded_data[count] = cs40l2x->pbq_fw_composite[j - 1];
			count++;
			unpadded_data[count] = cs40l2x->pbq_fw_composite[j - 2];
			count++;
		}
	}

	/* Add zero padding bytes */
	zero_pad_data[zero_pad_count] = 0;
	zero_pad_count++;
	for (k = 0; k < count; k++) {
		zero_pad_data[zero_pad_count] = unpadded_data[k];
		zero_pad_count++;
		if ((((k + 1) % 3) == 0) && ((k + 1) != count)) {
			zero_pad_data[zero_pad_count] = 0;
			zero_pad_count++;
		}
	}

	memcpy((*raw_composite_data), &zero_pad_data[0], comp_size);

	devm_kfree(cs40l2x->dev, unpadded_data);
	devm_kfree(cs40l2x->dev, zero_pad_data);

	return 0;
}

static int cs40l2x_insert_comp_wt_header(struct cs40l2x_private *cs40l2x,
	bool is_xm, unsigned int comp_size, unsigned int pos)
{
	unsigned int i_pos = pos;
	unsigned int offset;
	char *wt_file_type;
	char *wt_offset;
	char *wt_length;
	int wt_entry_words = ((CS40L2X_WT_HEADER_ENTRY_SIZE *
		CS40L2X_WT_NUM_VIRT_SLOTS) / 4);
	unsigned int indv_comp_size = (comp_size / CS40L2X_WT_NUM_VIRT_SLOTS);
	int i;

	cs40l2x->three_bytes[0] = 0;
	cs40l2x->three_bytes[1] = 0;
	cs40l2x->three_bytes[2] = 0;
	wt_file_type = cs40l2x->three_bytes;
	wt_offset = cs40l2x->three_bytes;
	wt_length = cs40l2x->three_bytes;

	if (is_xm) {
		offset = cs40l2x->wt_xm_header_last_offset +
			cs40l2x->wt_xm_header_last_size +
			wt_entry_words;
	} else {
		offset = cs40l2x->wt_ym_header_last_offset +
			cs40l2x->wt_ym_header_last_size +
			wt_entry_words;
	}

	for (i = 0; i < CS40L2X_WT_NUM_VIRT_SLOTS; i++) {
		cs40l2x_to_bytes_msb(CS40L2X_WT_TYPE_10_COMP_FILE,
			CS40L2X_WT_WORD_SIZE, &wt_file_type);
		cs40l2x->pbq_updated_fw_raw_wt[i_pos] = 0; /* Zero pad */
		cs40l2x->pbq_updated_fw_raw_wt[i_pos + 1] = wt_file_type[0];
		cs40l2x->pbq_updated_fw_raw_wt[i_pos + 2] = wt_file_type[1];
		cs40l2x->pbq_updated_fw_raw_wt[i_pos + 3] = wt_file_type[2];

		i_pos += 4;
		cs40l2x_to_bytes_msb(offset,
			CS40L2X_WT_WORD_SIZE, &wt_offset);
		cs40l2x->pbq_updated_fw_raw_wt[i_pos] = 0; /* Zero pad */
		cs40l2x->pbq_updated_fw_raw_wt[i_pos + 1] = wt_offset[0];
		cs40l2x->pbq_updated_fw_raw_wt[i_pos + 2] = wt_offset[1];
		cs40l2x->pbq_updated_fw_raw_wt[i_pos + 3] = wt_offset[2];

		i_pos += 4;
		cs40l2x_to_bytes_msb((indv_comp_size / 4),
			CS40L2X_WT_WORD_SIZE, &wt_length);
		cs40l2x->pbq_updated_fw_raw_wt[i_pos] = 0; /* Zero pad */
		cs40l2x->pbq_updated_fw_raw_wt[i_pos + 1] = wt_length[0];
		cs40l2x->pbq_updated_fw_raw_wt[i_pos + 2] = wt_length[1];
		cs40l2x->pbq_updated_fw_raw_wt[i_pos + 3] = wt_length[2];

		i_pos += 4;
		offset += (indv_comp_size / 4);
	}

	return i_pos;
}

static void cs40l2x_update_wt_header_offsets(struct cs40l2x_private *cs40l2x,
	bool is_xm)
{
	char *wt_offset;
	unsigned int pos = 0;
	unsigned int offset;
	unsigned int num_wavs;
	int i;

	cs40l2x->three_bytes[0] = 0;
	cs40l2x->three_bytes[1] = 0;
	cs40l2x->three_bytes[2] = 0;
	wt_offset = cs40l2x->three_bytes;

	if (is_xm)
		num_wavs = cs40l2x->num_xm_wavs;
	else
		num_wavs = cs40l2x->num_ym_wavs;

	for (i = 0; i < num_wavs; i++) {
		pos += 4;
		offset = cs40l2x->updated_offsets[i];
		if (!is_xm) {
			offset =
			cs40l2x->updated_offsets[i + cs40l2x->num_xm_wavs];
		}
		wt_offset[0] = 0;
		wt_offset[1] = 0;
		wt_offset[2] = 0;
		cs40l2x_to_bytes_msb(offset, CS40L2X_WT_WORD_SIZE,
			&wt_offset);
		cs40l2x->pbq_updated_fw_raw_wt[pos] = 0; /* Zero pad */
		cs40l2x->pbq_updated_fw_raw_wt[pos + 1] = wt_offset[0];
		cs40l2x->pbq_updated_fw_raw_wt[pos + 2] = wt_offset[1];
		cs40l2x->pbq_updated_fw_raw_wt[pos + 3] = wt_offset[2];
		pos += 8;
	}
}

static int cs40l2x_create_wt_header(struct cs40l2x_private *cs40l2x,
	bool is_xm)
{
	unsigned int start_pos;
	unsigned int header_length;

	if (is_xm) {
		header_length = (cs40l2x->num_xm_wavs *
			CS40L2X_WT_HEADER_ENTRY_SIZE);
		start_pos = cs40l2x->xm_hdr_strt_pos;
	} else {
		header_length = (cs40l2x->num_ym_wavs *
			CS40L2X_WT_HEADER_ENTRY_SIZE);
		start_pos = cs40l2x->ym_hdr_strt_pos;
	}

	memcpy(&cs40l2x->pbq_updated_fw_raw_wt[0],
		&cs40l2x->pbq_fw_raw_wt[start_pos],
		(start_pos + header_length));

	return header_length;
}

static int cs40l2x_copy_waveform_data(struct cs40l2x_private *cs40l2x,
	bool is_xm,	unsigned int pos)
{
	unsigned int wav_data_end;
	unsigned int i_pos = pos;
	unsigned int start;
	int i;

	if (is_xm) {
		wav_data_end = cs40l2x->xm_hdr_strt_pos +
		((cs40l2x->wt_xm_header_last_offset +
			cs40l2x->wt_xm_header_last_size) *
			CS40L2X_WT_TOTAL_WORD_SIZE);
		start = cs40l2x->wt_xm_header_end_pos;
	} else {
		wav_data_end = cs40l2x->ym_hdr_strt_pos +
		((cs40l2x->wt_ym_header_last_offset +
			cs40l2x->wt_ym_header_last_size) *
			CS40L2X_WT_TOTAL_WORD_SIZE);
		start = cs40l2x->wt_ym_header_end_pos;
	}

	for (i = start; i < wav_data_end; i++) {
		cs40l2x->pbq_updated_fw_raw_wt[i_pos] =
			cs40l2x->pbq_fw_raw_wt[i];
		i_pos++;
	}

	return i_pos;
}

static int cs40l2x_update_existing_block(struct cs40l2x_private *cs40l2x,
	unsigned int comp_size, bool is_xm)
{
	unsigned int pos;
	unsigned int end_data_pos;
	unsigned int end_header_pos;
	unsigned int existing_wt_size;
	unsigned int wav_data_end;
	unsigned int start;

	if (is_xm) {
		wav_data_end = cs40l2x->xm_hdr_strt_pos +
		((cs40l2x->wt_xm_header_last_offset +
			cs40l2x->wt_xm_header_last_size) *
			CS40L2X_WT_TOTAL_WORD_SIZE);
		start = cs40l2x->xm_hdr_strt_pos;
	} else {
		wav_data_end = cs40l2x->ym_hdr_strt_pos +
		((cs40l2x->wt_ym_header_last_offset +
			cs40l2x->wt_ym_header_last_size) *
			CS40L2X_WT_TOTAL_WORD_SIZE);
		start = cs40l2x->ym_hdr_strt_pos;
	}

	existing_wt_size = (wav_data_end - start);

	cs40l2x->pbq_updated_fw_raw_wt_size = (existing_wt_size +
		(comp_size + (CS40L2X_WT_HEADER_ENTRY_SIZE *
			CS40L2X_WT_NUM_VIRT_SLOTS)));

	/* Copies wt header up to position before wt header terminator */
	pos = cs40l2x_create_wt_header(cs40l2x, is_xm);

	/* Update the offsets to account for the extra wt header entries */
	cs40l2x_update_wt_header_offsets(cs40l2x, is_xm);

	/* Insert the new composite wt header entry
	 * after the last wt header entry
	 * and before the wt header terminator
	 */
	end_header_pos =
		cs40l2x_insert_comp_wt_header(cs40l2x, is_xm, comp_size, pos);
	cs40l2x->virt_wt_end_header_pos = end_header_pos;

	/* Copy waveform data */
	end_data_pos =
		cs40l2x_copy_waveform_data(cs40l2x, is_xm, end_header_pos);

	end_data_pos += comp_size;
	cs40l2x->updated_block_size = end_data_pos;

	if (cs40l2x->updated_block_size >
		cs40l2x->pbq_updated_fw_raw_wt_size) {
		dev_err(cs40l2x->dev, "Virtual block copy failed.\n");
		return -EINVAL;
	}

	return 0;
}

static int cs40l2x_create_block(struct cs40l2x_private *cs40l2x,
	unsigned int comp_size)
{
	char *wt_offset;
	char *wvfrm_size;
	unsigned int pos;
	unsigned int offset;
	unsigned int end_data_pos;
	unsigned int indv_comp_size = (comp_size / CS40L2X_WT_NUM_VIRT_SLOTS);
	int i, count = 0, header_slot = 0;

	cs40l2x->two_bytes[0] = 0;
	cs40l2x->two_bytes[1] = 0;
	wt_offset = cs40l2x->two_bytes;
	wvfrm_size = cs40l2x->two_bytes;

	cs40l2x->pbq_updated_fw_raw_wt_size = (CS40L2X_WT_TERMINATOR_BYTES +
		(comp_size + (CS40L2X_WT_HEADER_ENTRY_SIZE *
			CS40L2X_WT_NUM_VIRT_SLOTS)));

	/* Create the new wt header */
	offset = (((CS40L2X_WT_HEADER_ENTRY_SIZE *
		CS40L2X_WT_NUM_VIRT_SLOTS) +
		CS40L2X_WT_TERMINATOR_BYTES) / 4);
	for (i = 0; i < (CS40L2X_WT_HEADER_ENTRY_SIZE *
		CS40L2X_WT_NUM_VIRT_SLOTS); i++) {
		if ((count + 1) % 4 == 0) {
			header_slot++;
			if (header_slot == 1) {
				cs40l2x->pbq_updated_fw_raw_wt[i] =
					CS40L2X_WT_TYPE_10_COMP_FILE;
			} else if (header_slot == 2) {
				wt_offset[0] = 0;
				wt_offset[1] = 0;
				cs40l2x_to_bytes_msb(offset,
					2, &wt_offset);
				cs40l2x->pbq_updated_fw_raw_wt[i - 1] =
					wt_offset[0];
				cs40l2x->pbq_updated_fw_raw_wt[i] =
					wt_offset[1];
			} else if (header_slot == 3) {
				wvfrm_size[0] = 0;
				wvfrm_size[1] = 0;
				cs40l2x_to_bytes_msb(
					(indv_comp_size / 4), 2, &wvfrm_size);
				cs40l2x->pbq_updated_fw_raw_wt[i - 1] =
					wvfrm_size[0];
				cs40l2x->pbq_updated_fw_raw_wt[i] =
					wvfrm_size[1];
				header_slot = 0;
				offset += (indv_comp_size / 4);
			}
		} else {
			cs40l2x->pbq_updated_fw_raw_wt[i] = CS40L2X_WT_ZERO;
		}
		count++;
	}

	/* Add new wt header terminator */
	pos = (CS40L2X_WT_HEADER_ENTRY_SIZE *
		CS40L2X_WT_NUM_VIRT_SLOTS);
	cs40l2x->pbq_updated_fw_raw_wt[pos] = CS40L2X_WT_ZERO;
	pos++;
	for (i = pos; i < (pos + (CS40L2X_WT_TERMINATOR_BYTES - 1)); i++)
		cs40l2x->pbq_updated_fw_raw_wt[i] =
			CS40L2X_WT_TERMINATOR_BYTE;

	end_data_pos = ((pos + (CS40L2X_WT_TERMINATOR_BYTES - 1)) + comp_size);
	cs40l2x->updated_block_size = end_data_pos;

	cs40l2x->ym_hdr_strt_pos = 0;
	cs40l2x->wt_ym_header_end_pos = 0;

	if (!cs40l2x->ym_hdr_strt_reg) {
		dev_err(cs40l2x->dev,
			"Malformed bin file, missing YM section header.\n");
		return -EINVAL;
	}

	if (cs40l2x->updated_block_size >
		cs40l2x->pbq_updated_fw_raw_wt_size) {
		dev_err(cs40l2x->dev, "Failed to create virtual block.\n");
		return -EINVAL;
	}

	return 0;
}

static int cs40l2x_write_virtual_waveform(struct cs40l2x_private *cs40l2x,
	unsigned int index, bool is_gpio, bool is_rise, bool over_write)
{
	bool is_xm;
	int ret = 0;
	char *raw_waveform_data;
	unsigned int last_offset, size_in_words, wvfrm_type;
	unsigned int num_xm_registers, num_ym_registers;
	unsigned int size_in_bytes = 0;
	unsigned int wt_xm_header_end_reg, wt_ym_header_end_reg;
	unsigned int len_reg, off_reg, type_reg, data_reg;
	struct regmap *regmap = cs40l2x->regmap;
	struct cs40l2x_virtual_waveform *virtual_wav;
	unsigned int reg_addr_size = CS40L2X_WT_TOTAL_WORD_SIZE;

	if (over_write) {
		is_xm = cs40l2x->ovwr_wav->is_xm;
		wvfrm_type = cs40l2x->ovwr_wav->wvfrm_type +
			cs40l2x->ovwr_wav->wvfrm_feature;
		size_in_bytes = cs40l2x->ovwr_wav->data_len;
		raw_waveform_data = devm_kzalloc(cs40l2x->dev,
			size_in_bytes, GFP_KERNEL);
		if (!raw_waveform_data)
			return -ENOMEM;
		memcpy(raw_waveform_data, &cs40l2x->ovwr_wav->data[0],
			cs40l2x->ovwr_wav->data_len);
	} else {
		list_for_each_entry(virtual_wav,
			&cs40l2x->virtual_waveform_head, list) {
			if (virtual_wav->index != index)
				continue;

			is_xm = virtual_wav->is_xm;
			wvfrm_type = virtual_wav->wvfrm_type +
				virtual_wav->wvfrm_feature;
			size_in_bytes = virtual_wav->data_len;
			raw_waveform_data = devm_kzalloc(cs40l2x->dev,
				size_in_bytes, GFP_KERNEL);
			if (!raw_waveform_data)
				return -ENOMEM;
			memcpy(raw_waveform_data, &virtual_wav->data[0],
				virtual_wav->data_len);
			break;
		}
		if (size_in_bytes == 0) {
			dev_err(cs40l2x->dev,
				"Unable to find index in virtual list\n");
			return -EINVAL;
		}
	}

	/* wt_xm_header_end_pos is last byte before header terminator */
	if (is_xm) {
		num_xm_registers = ((cs40l2x->wt_xm_header_end_pos +
			(CS40L2X_WT_NUM_VIRT_SLOTS *
			CS40L2X_WT_HEADER_ENTRY_SIZE)) -
			cs40l2x->xm_hdr_strt_pos);
		wt_xm_header_end_reg = (cs40l2x->xm_hdr_strt_reg +
			num_xm_registers);
		len_reg = (wt_xm_header_end_reg - reg_addr_size);
		off_reg = (wt_xm_header_end_reg - (reg_addr_size * 2));
		type_reg = (wt_xm_header_end_reg - (reg_addr_size * 3));
		if (is_gpio) {
			len_reg -= CS40L2X_WT_HEADER_ENTRY_SIZE;
			off_reg -= CS40L2X_WT_HEADER_ENTRY_SIZE;
			type_reg -= CS40L2X_WT_HEADER_ENTRY_SIZE;
			if (is_rise) {
				len_reg -= CS40L2X_WT_HEADER_ENTRY_SIZE;
				off_reg -= CS40L2X_WT_HEADER_ENTRY_SIZE;
				type_reg -= CS40L2X_WT_HEADER_ENTRY_SIZE;
			}
		}
		ret = regmap_read(regmap, off_reg, &last_offset);
		if (ret) {
			dev_err(cs40l2x->dev,
				"Failed to read last offset 0x%08X: %d\n",
				off_reg, ret);
			goto err_free;
		}
		data_reg = (cs40l2x->xm_hdr_strt_reg +
			(last_offset * reg_addr_size));
	} else {
		num_ym_registers = ((cs40l2x->wt_ym_header_end_pos +
			(CS40L2X_WT_NUM_VIRT_SLOTS *
			CS40L2X_WT_HEADER_ENTRY_SIZE)) -
			cs40l2x->ym_hdr_strt_pos);
		wt_ym_header_end_reg = (cs40l2x->ym_hdr_strt_reg +
			num_ym_registers);
		len_reg = (wt_ym_header_end_reg - reg_addr_size);
		off_reg = (wt_ym_header_end_reg - (reg_addr_size * 2));
		type_reg = (wt_ym_header_end_reg - (reg_addr_size * 3));
		if (is_gpio) {
			len_reg -= CS40L2X_WT_HEADER_ENTRY_SIZE;
			off_reg -= CS40L2X_WT_HEADER_ENTRY_SIZE;
			type_reg -= CS40L2X_WT_HEADER_ENTRY_SIZE;
			if (is_rise) {
				len_reg -= CS40L2X_WT_HEADER_ENTRY_SIZE;
				off_reg -= CS40L2X_WT_HEADER_ENTRY_SIZE;
				type_reg -= CS40L2X_WT_HEADER_ENTRY_SIZE;
			}
		}
		ret = regmap_read(regmap, off_reg, &last_offset);
		if (ret) {
			dev_err(cs40l2x->dev,
				"Failed to read last offset 0x%08X: %d\n",
				off_reg, ret);
			goto err_free;
		}
		data_reg = (cs40l2x->ym_hdr_strt_reg +
			(last_offset * reg_addr_size));
	}

	size_in_words = (size_in_bytes / CS40L2X_WT_TOTAL_WORD_SIZE);

	ret = regmap_write(regmap, len_reg, size_in_words);
	if (ret) {
		dev_err(cs40l2x->dev,
			"Failed to write waveform size 0x%08X: %d\n",
			len_reg, ret);
		goto err_free;
	}
	ret = regmap_write(regmap, type_reg, wvfrm_type);
	if (ret) {
		dev_err(cs40l2x->dev,
			"Failed to write waveform type 0x%08X: %d\n",
			type_reg, ret);
		goto err_free;
	}
	ret = cs40l2x_raw_write(cs40l2x, data_reg, &raw_waveform_data[0],
		size_in_bytes, CS40L2X_MAX_WLEN);
	if (ret) {
		dev_err(cs40l2x->dev, "Failed to write wt virtual data\n");
		goto err_free;
	}

	if (!is_gpio)
		cs40l2x->loaded_virtual_index = index;
	else {
		if (is_rise)
			cs40l2x->loaded_gpio_index[CS40L2X_GPIO_RISE] =
				index;
		else
			cs40l2x->loaded_gpio_index[CS40L2X_GPIO_FALL] =
				index;
	}

err_free:
	devm_kfree(cs40l2x->dev, raw_waveform_data);

	return ret;
}

static int cs40l2x_add_waveform_to_virtual_list(
	struct cs40l2x_private *cs40l2x,
	unsigned int type, unsigned int feature,
	unsigned int raw_data_size, char *raw_data)
{
	struct cs40l2x_virtual_waveform *virtual_wav;

	virtual_wav = devm_kzalloc(cs40l2x->dev,
				sizeof(*virtual_wav), GFP_KERNEL);
	if (!virtual_wav)
		return -ENOMEM;

	cs40l2x->num_virtual_waves++;

	if (cs40l2x->num_virtual_waves > CS40L2X_WT_MAX_VIRT_WAVS) {
		dev_err(cs40l2x->dev,
			"Attempt to exceed maximum virtual waveforms limit\n");
		cs40l2x->num_virtual_waves--;
		return -EINVAL;
	}

	virtual_wav->index = (((cs40l2x->num_waves -
		CS40L2X_WT_NUM_VIRT_SLOTS) +
		cs40l2x->num_virtual_waves) - 1);
	/* The minus 1 is to account for zero being a valid index */

	virtual_wav->is_xm = cs40l2x->xm_append;

	virtual_wav->wvfrm_type = type;

	virtual_wav->wvfrm_feature = feature;

	virtual_wav->data_len = raw_data_size;

	memcpy(virtual_wav->data, &raw_data[0], raw_data_size);

	list_add(&virtual_wav->list, &cs40l2x->virtual_waveform_head);

	return 0;
}

static void cs40l2x_save_waveform_to_ovwr_struct(
	struct cs40l2x_private *cs40l2x,
	unsigned int wvfrm_type,
	unsigned int wvfrm_feature,
	unsigned int raw_waveform_size,
	char *raw_waveform_data)
{
	cs40l2x->ovwr_wav->is_xm = cs40l2x->xm_append;
	cs40l2x->ovwr_wav->wvfrm_type = wvfrm_type;
	cs40l2x->ovwr_wav->wvfrm_feature = wvfrm_feature;
	cs40l2x->ovwr_wav->data_len = raw_waveform_size;
	memcpy(cs40l2x->ovwr_wav->data, &raw_waveform_data[0],
		raw_waveform_size);
}

static void cs40l2x_calc_num_waves(struct cs40l2x_private *cs40l2x)
{
	unsigned int wt_file_type;
	unsigned int block_length;
	unsigned int pos;
	int i;

	cs40l2x->num_xm_wavs = 0;
	cs40l2x->num_ym_wavs = 0;

	if (cs40l2x->xm_hdr_strt_pos > 0) {
		block_length = cs40l2x->wt_xm_size -
			cs40l2x->xm_hdr_strt_pos;
		pos = cs40l2x->xm_hdr_strt_pos;
		for (i = 0; i < block_length - CS40L2X_WT_HEADER_ENTRY_SIZE;
			i += CS40L2X_WT_HEADER_ENTRY_SIZE) {
			wt_file_type = (cs40l2x->pbq_fw_raw_wt[pos] << 24)
				+ (cs40l2x->pbq_fw_raw_wt[pos + 1] << 16)
				+ (cs40l2x->pbq_fw_raw_wt[pos + 2] << 8)
				+ (cs40l2x->pbq_fw_raw_wt[pos + 3]);
			if (wt_file_type != CS40L2X_WT_TERMINATOR)
				cs40l2x->num_xm_wavs++;
			else
				break;
			pos += CS40L2X_WT_HEADER_ENTRY_SIZE;
		}
	}

	if (cs40l2x->ym_hdr_strt_pos > 0) {
		block_length = cs40l2x->wt_ym_size -
			(cs40l2x->ym_hdr_strt_pos -
				cs40l2x->wt_xm_size);
		pos = cs40l2x->ym_hdr_strt_pos;
		for (i = 0; i < block_length - CS40L2X_WT_HEADER_ENTRY_SIZE;
			i += CS40L2X_WT_HEADER_ENTRY_SIZE) {
			wt_file_type = (cs40l2x->pbq_fw_raw_wt[pos] << 24)
				+ (cs40l2x->pbq_fw_raw_wt[pos + 1] << 16)
				+ (cs40l2x->pbq_fw_raw_wt[pos + 2] << 8)
				+ (cs40l2x->pbq_fw_raw_wt[pos + 3]);
			if (wt_file_type != CS40L2X_WT_TERMINATOR)
				cs40l2x->num_ym_wavs++;
			else
				break;
			pos += CS40L2X_WT_HEADER_ENTRY_SIZE;
		}
	}

	cs40l2x->num_waves = cs40l2x->num_xm_wavs + cs40l2x->num_ym_wavs;
}

static int cs40l2x_add_wt_slots(struct cs40l2x_private *cs40l2x,
	unsigned int *is_xm)
{
	unsigned int wt_open_bytes = CS40L2X_PACKED_BYTES_MAX +
		(CS40L2X_WT_HEADER_ENTRY_SIZE * CS40L2X_WT_NUM_VIRT_SLOTS);
	unsigned int comp_size;
	int no_space = -ENOSPC;
	int ret = 0;

	while (no_space) {
		no_space = cs40l2x_check_wt_open_space(cs40l2x, wt_open_bytes);
		if (no_space) {
			wt_open_bytes = wt_open_bytes -
				CS40L2X_PBQ_FW_BYTES_MIN;
			if (wt_open_bytes < CS40L2X_PBQ_FW_BYTES_MIN) {
				dev_err(cs40l2x->dev,
					"No space in wt for virtual wvfrms\n");
				return -ENOSPC;
			}
		}
	}
	dev_dbg(cs40l2x->dev,
		"Total size of all virtual slots: %d bytes\n",
		wt_open_bytes);

	cs40l2x_calc_num_waves(cs40l2x);

	cs40l2x->virtual_slot_index = ((cs40l2x->num_waves +
		CS40L2X_WT_NUM_VIRT_SLOTS) - 1);
	cs40l2x->virtual_gpio1_fall_index =
		cs40l2x->virtual_slot_index - 1;
	cs40l2x->virtual_gpio1_rise_index =
		cs40l2x->virtual_gpio1_fall_index - 1;

	comp_size = (wt_open_bytes -
		(CS40L2X_WT_HEADER_ENTRY_SIZE * CS40L2X_WT_NUM_VIRT_SLOTS));
	cs40l2x->comp_bytes = comp_size;

	cs40l2x->display_pwle_segs =
		(((cs40l2x->comp_bytes / CS40L2X_WT_NUM_VIRT_SLOTS) -
		CS40L2X_PWLE_NON_SEG_BYTES) / CS40L2X_PWLE_MAX_SEG_BYTES);

	cs40l2x->wvfrm_lengths_size = (cs40l2x->num_waves * 2);

	ret = cs40l2x_create_wvfrm_len_type_pairs(cs40l2x);
	if (ret)
		return ret;

	if (cs40l2x->xm_append) {
		*is_xm = 1;
		ret = cs40l2x_update_existing_block(cs40l2x, comp_size, true);
	} else {
		*is_xm = 0;
		if (cs40l2x->create_ym)
			ret = cs40l2x_create_block(cs40l2x, comp_size);
		else
			ret = cs40l2x_update_existing_block(cs40l2x,
				comp_size, false);
	}

	return ret;
}

static int cs40l2x_convert_and_save_comp_data(struct cs40l2x_private *cs40l2x,
	unsigned int index, bool over_write)
{
	int ret = 0;
	unsigned int comp_size;
	unsigned int wvfrm_samples;
	char *raw_composite_data;

	ret = cs40l2x_calc_composite_size(cs40l2x, &comp_size);
	if (ret)
		return ret;

	raw_composite_data = devm_kzalloc(cs40l2x->dev,	comp_size, GFP_KERNEL);
	if (!raw_composite_data)
		return -ENOMEM;

	ret = cs40l2x_calc_wvfrm_samples(cs40l2x, &wvfrm_samples);
	if (ret)
		goto err_free;

	cs40l2x->pbq_fw_composite[0] = wvfrm_samples;
	ret = cs40l2x_pack_wt_composite_data(cs40l2x, comp_size,
		&raw_composite_data);
	if (ret)
		goto err_free;

	if (over_write)
		cs40l2x_save_waveform_to_ovwr_struct(cs40l2x,
			CS40L2X_WT_TYPE_10_COMP_FILE, 0, comp_size,
			raw_composite_data);
	else
		ret = cs40l2x_add_waveform_to_virtual_list(cs40l2x,
			CS40L2X_WT_TYPE_10_COMP_FILE, 0,
			comp_size, raw_composite_data);

err_free:
	devm_kfree(cs40l2x->dev, raw_composite_data);

	return ret;
}

static ssize_t cs40l2x_cp_trigger_index_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	unsigned int index;

	mutex_lock(&cs40l2x->lock);
	index = cs40l2x->cp_trigger_index;
	if (cs40l2x->cp_trigger_index == cs40l2x->virtual_slot_index)
		index = cs40l2x->loaded_virtual_index;
	mutex_unlock(&cs40l2x->lock);

	return snprintf(buf, PAGE_SIZE, "%d\n", index);
}

static ssize_t cs40l2x_cp_trigger_index_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	struct i2c_client *i2c_client = to_i2c_client(cs40l2x->dev);
	int ret;
	unsigned int index;

	ret = kstrtou32(buf, 10, &index);
	if (ret)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);

	if ((index == CS40L2X_INDEX_PBQ_SAVE) ||
		(index == CS40L2X_INDEX_OVWR_SAVE)) {
		if (!cs40l2x->virtual_bin) {
			dev_err(cs40l2x->dev, "Virtual slot not enabled.\n");
			return -EINVAL;
		}
		if (!cs40l2x->queue_stored) {
			dev_err(cs40l2x->dev,
				"Empty wav data, save composite or PWLE first.\n");
			return -EINVAL;
		}

#ifdef CONFIG_ANDROID_TIMED_OUTPUT
		cs40l2x->safe_save_state = true;
		/* Bypass safe_save_state check for Timed Output */
#else
		if (!cs40l2x->safe_save_state) {
			dev_err(cs40l2x->dev, "Save attempted during vibe, try again.\n");
			return -EINVAL;
		}
#endif /* CONFIG_ANDROID_TIMED_OUTPUT */

		cs40l2x->virtual_stored = false;
		disable_irq(i2c_client->irq);
	}

	switch (index) {
	case CS40L2X_INDEX_PBQ_SAVE:
		ret = cs40l2x_convert_and_save_comp_data(cs40l2x, index, false);
		if (ret)
			dev_err(cs40l2x->dev, "Unable to save virtual waveform.\n");
		/* After save or save attempt, reset flag */
		cs40l2x->queue_stored = false;
		break;
	case CS40L2X_INDEX_OVWR_SAVE:
		if (cs40l2x->last_type_entered ==
			CS40L2X_WT_TYPE_10_COMP_FILE) {
			ret = cs40l2x_convert_and_save_comp_data(cs40l2x,
				index, true);
			if (ret)
				dev_err(cs40l2x->dev, "Unable to convert waveform.\n");
		}
		ret = cs40l2x_write_virtual_waveform(cs40l2x, index,
			false, false, true);
		if (ret)
			dev_err(cs40l2x->dev, "Unable to write waveform.\n");
		index = cs40l2x->virtual_slot_index;
		/* After save or save attempt, reset flag */
		cs40l2x->queue_stored = false;
		break;
	case CS40L2X_INDEX_QEST:
		if (cs40l2x->fw_desc->id == CS40L2X_FW_ID_ORIG) {
			ret = -EPERM;
			break;
		}
		/* intentionally fall through */
	case CS40L2X_INDEX_DIAG:
		if (cs40l2x->fw_desc->id == cs40l2x->fw_id_remap)
			ret = cs40l2x_firmware_swap(cs40l2x,
					CS40L2X_FW_ID_CAL);
		break;
	case CS40L2X_INDEX_PEAK:
	case CS40L2X_INDEX_PBQ:
		if (cs40l2x->fw_desc->id == CS40L2X_FW_ID_CAL)
			ret = cs40l2x_firmware_swap(cs40l2x,
					cs40l2x->fw_id_remap);
		break;
	case CS40L2X_INDEX_IDLE:
		ret = -EINVAL;
		break;
	default:
		if (!cs40l2x->virtual_bin) {
			if ((index & CS40L2X_INDEX_MASK) >=
				cs40l2x->num_waves) {
				ret = -EINVAL;
				break;
			}
		} else {
			if ((index & CS40L2X_INDEX_MASK) >=
				(cs40l2x->num_waves -
					CS40L2X_WT_NUM_VIRT_SLOTS)) {
				if ((index & CS40L2X_INDEX_MASK) >=
					((cs40l2x->num_waves +
						cs40l2x->num_virtual_waves) -
						CS40L2X_WT_NUM_VIRT_SLOTS)) {
					ret = -EINVAL;
					break;
				}
				if (index != cs40l2x->loaded_virtual_index)
					cs40l2x_write_virtual_waveform(cs40l2x,
						index, false, false, false);
				/* else virtual waveform already loaded */
				index = cs40l2x->virtual_slot_index;
			}
		}

		if (cs40l2x->fw_desc->id == CS40L2X_FW_ID_CAL)
			ret = cs40l2x_firmware_swap(cs40l2x,
					cs40l2x->fw_id_remap);
	}
	if (ret)
		goto err_exit;

	cs40l2x->cp_trigger_index = index;
	ret = count;

err_exit:
	if (!cs40l2x->virtual_stored) {
		cs40l2x->virtual_stored = true;
		enable_irq(i2c_client->irq);
	}

	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_cp_trigger_queue_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	ssize_t len = 0;
	int i;

	if (cs40l2x->pbq_depth == 0)
		return -ENODATA;

	mutex_lock(&cs40l2x->lock);

	for (i = 0; i < cs40l2x->pbq_depth; i++) {
		switch (cs40l2x->pbq_pairs[i].tag) {
		case CS40L2X_PBQ_TAG_SILENCE:
			len += snprintf(buf + len, PAGE_SIZE - len, "%d",
					cs40l2x->pbq_pairs[i].mag);
			break;
		case CS40L2X_PBQ_TAG_START:
			len += snprintf(buf + len, PAGE_SIZE - len, "!!");
			break;
		case CS40L2X_PBQ_TAG_STOP:
			len += snprintf(buf + len, PAGE_SIZE - len, "%d!!",
					cs40l2x->pbq_pairs[i].repeat);
			break;
		default:
			len += snprintf(buf + len, PAGE_SIZE - len, "%d.%d",
					cs40l2x->pbq_pairs[i].tag,
					cs40l2x->pbq_pairs[i].mag);
		}

		if (i < (cs40l2x->pbq_depth - 1))
			len += snprintf(buf + len, PAGE_SIZE - len, ", ");
	}

	switch (cs40l2x->pbq_repeat) {
	case -1:
		len += snprintf(buf + len, PAGE_SIZE - len, ", ~\n");
		break;
	case 0:
		len += snprintf(buf + len, PAGE_SIZE - len, "\n");
		break;
	default:
		len += snprintf(buf + len, PAGE_SIZE - len, ", %d!\n",
				cs40l2x->pbq_repeat);
	}

	mutex_unlock(&cs40l2x->lock);

	return len;
}

static ssize_t cs40l2x_cp_trigger_queue_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	struct i2c_client *i2c_client = to_i2c_client(cs40l2x->dev);
	struct cs40l2x_composite_data empty_comp;
	char *pbq_str_alloc, *pbq_str, *pbq_str_tok, *pbq_temp;
	char *pbq_seg_alloc, *pbq_seg, *pbq_seg_tok;
	size_t pbq_seg_len;
	unsigned int pbq_depth = 0, wav_count = 0, s_cnt = 0;
	unsigned int num_waves = cs40l2x->num_waves;
	unsigned int val, num_empty, indx, amp;
	bool next_is_delay = false;
	bool inner_flag = false;
	int ret, l, m, n, f_cnt;
	int pbq_marker = -1;

	/* This flag must be set before any possible return calls */
	cs40l2x->queue_stored = false;

	cs40l2x->comp_dur_en = false;
	cs40l2x->pbq_fw_composite_len = 0;
	memset(&cs40l2x->pbq_fw_composite[0], 0,
		sizeof(cs40l2x->pbq_fw_composite));
	empty_comp.rpt = 0;
	empty_comp.index = 0;
	empty_comp.amp = 0;
	empty_comp.delay = 0;
	empty_comp.dur = 0;
	for (n = 0; n < CS40L2X_PBQ_DEPTH_MAX; n++)
		cs40l2x->comp_sets[n] = empty_comp;

	if (cs40l2x->virtual_bin)
		num_waves = cs40l2x->num_waves - CS40L2X_WT_NUM_VIRT_SLOTS;

	if (!cs40l2x->virtual_stored) {
		dev_err(dev, "Unsafe condition encountered.\n");
		return -EINVAL;
	}

	pbq_str_alloc = kzalloc(count, GFP_KERNEL);
	if (!pbq_str_alloc)
		return -ENOMEM;

	pbq_seg_alloc = kzalloc(CS40L2X_PBQ_SEG_LEN_MAX + 1, GFP_KERNEL);
	if (!pbq_seg_alloc) {
		kfree(pbq_str_alloc);
		return -ENOMEM;
	}

	disable_irq(i2c_client->irq);

	mutex_lock(&cs40l2x->lock);

	cs40l2x->pbq_depth = 0;
	cs40l2x->pbq_repeat = 0;

	pbq_str = pbq_str_alloc;
	strlcpy(pbq_str, buf, count);

	pbq_str_tok = strsep(&pbq_str, ",");

	while (pbq_str_tok) {
		pbq_seg = pbq_seg_alloc;
		pbq_seg_len = strlcpy(pbq_seg, strim(pbq_str_tok),
				CS40L2X_PBQ_SEG_LEN_MAX + 1);
		if (pbq_seg_len > CS40L2X_PBQ_SEG_LEN_MAX) {
			ret = -E2BIG;
			goto err_mutex;
		}

		/* waveform specifier */
		if (strnchr(pbq_seg, CS40L2X_PBQ_SEG_LEN_MAX, '.')) {
			/* index */
			pbq_seg_tok = strsep(&pbq_seg, ".");

			ret = kstrtou32(pbq_seg_tok, 10, &val);
			if (ret) {
				ret = -EINVAL;
				goto err_mutex;
			}
			if (val == 0) {
				pbq_temp = strnchr(pbq_seg, 20, '.');
				if (pbq_temp == NULL) {
					/* Valid zero entry not found. */
					ret = -EINVAL;
					goto err_mutex;
				}
			}
			if (val == 0 || val >= num_waves) {
				dev_err(cs40l2x->dev,
					"Invalid index detected.\n");
				ret = -EINVAL;
				goto err_mutex;
			}
			cs40l2x->pbq_pairs[pbq_depth].tag = val;
			indx = val;

			/* scale */
			pbq_seg_tok = strsep(&pbq_seg, ".");

			ret = kstrtou32(pbq_seg_tok, 10, &val);
			if (ret) {
				ret = -EINVAL;
				goto err_mutex;
			}
			if (val == 0 || val > CS40L2X_PBQ_SCALE_MAX) {
				dev_err(cs40l2x->dev,
					"Invalid amplitude detected.\n");
				ret = -EINVAL;
				goto err_mutex;
			}
			cs40l2x->pbq_pairs[pbq_depth].mag = val;
			amp = val;

			/* duration */
			pbq_seg_tok = strsep(&pbq_seg, ".");

			if (pbq_seg_tok != NULL) {
				ret = kstrtou32(pbq_seg_tok, 10, &val);
				if (ret) {
					ret = -EINVAL;
					goto err_mutex;
				}
				if (val == CS40L2X_PWLE_INDEF_TIME_VAL) {
					if (cs40l2x->comp_dur_min_fw)
						cs40l2x->comp_dur_en = true;
					else {
						dev_err(cs40l2x->dev,
							"Composite duration not supported in FW.\n");
						ret = -EINVAL;
						goto err_mutex;
					}
					if (inner_flag)
						cs40l2x->comp_sets[s_cnt].rpt =
							CS40L2X_PBQ_INNER_FLAG;
					cs40l2x->comp_sets[s_cnt].index =
						indx;
					cs40l2x->comp_sets[s_cnt].amp = amp;
					cs40l2x->comp_sets[s_cnt].dur = val;
					cs40l2x->pbq_pairs[pbq_depth].dur = val;
					s_cnt++;
					wav_count++;
				} else {
					if (val > CS40L2X_PWLE_MAX_TIME_VAL) {
						dev_err(cs40l2x->dev,
							"Valid Duration: 0 to 16383ms, or 65535\n");
						ret = -EINVAL;
						goto err_mutex;
					}
					if (cs40l2x->comp_dur_min_fw)
						cs40l2x->comp_dur_en = true;
					else {
						dev_err(cs40l2x->dev,
							"Composite duration not supported in FW.\n");
						ret = -EINVAL;
						goto err_mutex;
					}
					/* Time = val * 4, due to PWLE spec */
					if (inner_flag)
						cs40l2x->comp_sets[s_cnt].rpt =
							CS40L2X_PBQ_INNER_FLAG;
					cs40l2x->comp_sets[s_cnt].index =
						indx;
					cs40l2x->comp_sets[s_cnt].amp = amp;
					cs40l2x->comp_sets[s_cnt].dur =
						(val * 4);
					cs40l2x->pbq_pairs[pbq_depth].dur = val;
					s_cnt++;
					wav_count++;
				}
			} else {
				if (inner_flag)
					cs40l2x->comp_sets[s_cnt].rpt =
						CS40L2X_PBQ_INNER_FLAG;
				cs40l2x->comp_sets[s_cnt].index = indx;
				cs40l2x->comp_sets[s_cnt].amp = amp;
				s_cnt++;
				wav_count++;
			}

			pbq_depth++;

		/* repetition specifier */
		} else if (strnchr(pbq_seg, CS40L2X_PBQ_SEG_LEN_MAX, '!')) {
			val = 0;
			num_empty = 0;

			pbq_seg_tok = strsep(&pbq_seg, "!");

			while (pbq_seg_tok) {
				if (strnlen(pbq_seg_tok,
						CS40L2X_PBQ_SEG_LEN_MAX)) {
					ret = kstrtou32(pbq_seg_tok, 10, &val);
					if (ret) {
						ret = -EINVAL;
						goto err_mutex;
					}
					if (val > CS40L2X_PBQ_REPEAT_MAX) {
						dev_err(cs40l2x->dev,
							"Invalid repeat detected.\n");
						ret = -EINVAL;
						goto err_mutex;
					}
				} else {
					num_empty++;
				}

				pbq_seg_tok = strsep(&pbq_seg, "!");
			}

			/* number of empty tokens reveals specifier type */
			switch (num_empty) {
			case 1:	/* outer loop: "n!" or "!n" */
				if (cs40l2x->pbq_repeat) {
					ret = -EINVAL;
					goto err_mutex;
				}
				cs40l2x->pbq_repeat = val;
				break;

			case 2:	/* inner loop stop: "n!!" or "!!n" */
				if (pbq_marker < 0) {
					ret = -EINVAL;
					goto err_mutex;
				}

				inner_flag = false;
				for (l = 0; l < s_cnt; l++) {
					if (cs40l2x->comp_sets[l].rpt ==
						CS40L2X_PBQ_INNER_FLAG)
						cs40l2x->comp_sets[l].rpt =
							val;
				}

				cs40l2x->pbq_pairs[pbq_depth].tag =
						CS40L2X_PBQ_TAG_STOP;
				cs40l2x->pbq_pairs[pbq_depth].mag = pbq_marker;
				cs40l2x->pbq_pairs[pbq_depth++].repeat = val;
				pbq_marker = -1;
				break;

			case 3:	/* inner loop start: "!!" */
				if (pbq_marker >= 0) {
					ret = -EINVAL;
					goto err_mutex;
				}

				inner_flag = true;

				cs40l2x->pbq_pairs[pbq_depth].tag =
						CS40L2X_PBQ_TAG_START;
				pbq_marker = pbq_depth++;
				break;

			default:
				ret = -EINVAL;
				goto err_mutex;
			}

		/* loop specifier */
		} else if (strnchr(pbq_seg, CS40L2X_PBQ_SEG_LEN_MAX, '~')) {
			if (cs40l2x->pbq_repeat) {
				ret = -EINVAL;
				goto err_mutex;
			}
			cs40l2x->pbq_repeat = -1;

		/* duration specifier */
		} else {
			cs40l2x->pbq_pairs[pbq_depth].tag =
					CS40L2X_PBQ_TAG_SILENCE;

			/* Increment if composite starts with delay */
			if (pbq_depth == 0)
				wav_count++;

			ret = kstrtou32(pbq_seg, 10, &val);
			if (ret) {
				ret = -EINVAL;
				goto err_mutex;
			}
			if (val > CS40L2X_PBQ_DELAY_MAX) {
				dev_err(cs40l2x->dev,
					"Invalid delay detected.\n");
				ret = -EINVAL;
				goto err_mutex;
			}
			cs40l2x->pbq_pairs[pbq_depth++].mag = val;
			if (inner_flag)
				cs40l2x->comp_sets[s_cnt].rpt =
					CS40L2X_PBQ_INNER_FLAG;
			cs40l2x->comp_sets[s_cnt].delay = val;
			s_cnt++;
		}

		if (pbq_depth == CS40L2X_PBQ_DEPTH_MAX) {
			ret = -E2BIG;
			goto err_mutex;
		}

		pbq_str_tok = strsep(&pbq_str, ",");
	}

	cs40l2x->comp_sets_size = s_cnt;
	cs40l2x->pbq_depth = pbq_depth;
	ret = count;

	cs40l2x->pbq_fw_composite[0] = 0; /* Placeholder for wvfrm samples */
	cs40l2x->pbq_fw_composite[1] = cs40l2x->pbq_repeat;
	cs40l2x->pbq_fw_composite[2] = wav_count;

	f_cnt = 3;
	for (m = 0; m < cs40l2x->comp_sets_size; m++) {
		cs40l2x->pbq_fw_composite[f_cnt] = cs40l2x->comp_sets[m].rpt;
		f_cnt++;
		cs40l2x->pbq_fw_composite[f_cnt] = cs40l2x->comp_sets[m].index;
		f_cnt++;
		cs40l2x->pbq_fw_composite[f_cnt] = cs40l2x->comp_sets[m].amp;
		f_cnt++;
		/* Check if next one is a delay */
		if ((!cs40l2x->comp_sets[m + 1].index) &&
			(!cs40l2x->comp_sets[m + 1].amp) &&
			(!cs40l2x->comp_sets[m + 1].dur)) {
			cs40l2x->pbq_fw_composite[f_cnt] =
				cs40l2x->comp_sets[m + 1].delay;
			next_is_delay = true;
		}
		f_cnt++;
		cs40l2x->pbq_fw_composite[f_cnt] = cs40l2x->comp_sets[m].dur;
		f_cnt++;
		if (next_is_delay) {
			m++;
			next_is_delay = false;
		}
	}
	cs40l2x->pbq_fw_composite_len = f_cnt;
	cs40l2x->last_type_entered = CS40L2X_WT_TYPE_10_COMP_FILE;
	cs40l2x->queue_stored = true;

err_mutex:
	mutex_unlock(&cs40l2x->lock);

	kfree(pbq_str_alloc);
	kfree(pbq_seg_alloc);

	enable_irq(i2c_client->irq);

	return ret;
}

static void cs40l2x_calc_pwle_samples(struct cs40l2x_private *cs40l2x,
	unsigned int accu_time, bool indefinite)
{
	unsigned int wait_time = cs40l2x->wvfrm_len_wait_time;
	unsigned int repeat = cs40l2x->pwle_repeat;
	unsigned int total_time = 0;
	unsigned int samples = 0;

	total_time = (wait_time + accu_time);
	if (repeat > 0)
		total_time = ((repeat + 1) * total_time);
	samples = (total_time - wait_time);
	samples = (samples * CS40L2X_PWLE_SAMPLES_PER_MS);

	if (indefinite)
		cs40l2x->pwle_wvfrm_len = (samples +
			CS40L2X_WT_COMP_INDEFINITE +
			CS40L2X_WT_COMP_LEN_CALCD);
	else
		cs40l2x->pwle_wvfrm_len = (samples + CS40L2X_WT_COMP_LEN_CALCD);
}

static void cs40l2x_pwle_seg_free(struct cs40l2x_private *cs40l2x)
{
	struct cs40l2x_pwle_segment *pwle_seg_struct;

	while (!list_empty(&cs40l2x->pwle_segment_head)) {
		pwle_seg_struct = list_first_entry(&cs40l2x->pwle_segment_head,
				struct cs40l2x_pwle_segment, list);
		list_del(&pwle_seg_struct->list);
		devm_kfree(cs40l2x->dev, pwle_seg_struct);
	}
}

static int cs40l2x_calc_pwle_bytes(struct cs40l2x_private *cs40l2x)
{
	unsigned int unpadded_bytes;

	unpadded_bytes = CS40L2X_PWLE_FIRST_BYTES;
	unpadded_bytes += (cs40l2x->pwle_num_segs *
		CS40L2X_PWLE_SEG_BYTES);
	unpadded_bytes += (cs40l2x->pwle_num_vb_targs *
		CS40L2X_PWLE_NUM_VBT_BYTES);
	unpadded_bytes += CS40L2X_PWLE_END_PAD_BYTES;

	return unpadded_bytes;
}

static int cs40l2x_save_packed_pwle_data(struct cs40l2x_private *cs40l2x)
{
	char *three_bytes_data, *two_bytes_data, *unpadded_data, *zero_pad_data;
	char wait_time_byte, num_segs_byte, b_four_time, time_byte;
	struct cs40l2x_pwle_segment *pwle_seg_struct;
	int zero_pad_count = 0, count = 0, ret = 0;
	unsigned int zero_pad_size, pwle_size;
	int i, k;

	pwle_size = cs40l2x_calc_pwle_bytes(cs40l2x);
	zero_pad_size = ((pwle_size / 3) + pwle_size);

	if (zero_pad_size > (cs40l2x->comp_bytes / CS40L2X_WT_NUM_VIRT_SLOTS)) {
		dev_err(cs40l2x->dev, "PWLE size exceeds available space\n");
		return -ENOSPC;
	}

	unpadded_data = devm_kzalloc(cs40l2x->dev, pwle_size, GFP_KERNEL);
	if (!unpadded_data)
		return -ENOMEM;

	zero_pad_data = devm_kzalloc(cs40l2x->dev, zero_pad_size, GFP_KERNEL);
	if (!zero_pad_data) {
		devm_kfree(cs40l2x->dev, unpadded_data);
		return -ENOMEM;
	}

	cs40l2x->two_bytes[0] = CS40L2X_ZERO_INIT;
	cs40l2x->two_bytes[1] = CS40L2X_ZERO_INIT;
	two_bytes_data = cs40l2x->two_bytes;

	cs40l2x->three_bytes[0] = CS40L2X_ZERO_INIT;
	cs40l2x->three_bytes[1] = CS40L2X_ZERO_INIT;
	cs40l2x->three_bytes[2] = CS40L2X_ZERO_INIT;
	three_bytes_data = cs40l2x->three_bytes;

	/* Wvfrm Length in samples */
	cs40l2x_to_bytes_msb(cs40l2x->pwle_wvfrm_len,
		CS40L2X_WT_WORD_SIZE, &three_bytes_data);
	for (i = 0; i < CS40L2X_WT_WORD_SIZE; i++) {
		unpadded_data[count] = three_bytes_data[i];
		count++;
	}

	/* Repeat, wait time and first 4 bits of num_segs */
	unpadded_data[count] = cs40l2x->pwle_repeat;
	count++;
	cs40l2x_to_bytes_msb(cs40l2x->pwle_wait_time,
		CS40L2X_TWO_BYTES, &two_bytes_data);
	unpadded_data[count] = ((cs40l2x->two_bytes[0] << 4) +
		(cs40l2x->two_bytes[1] >> 4));
	count++;
	wait_time_byte = (two_bytes_data[1] << 4);
	num_segs_byte = cs40l2x->pwle_num_segs;
	/* Last 4bits wait time and first 4bits num segs */
	unpadded_data[count] = ((num_segs_byte >> 4) +
		wait_time_byte);
	count++;
	b_four_time = (num_segs_byte << 4);

	if (list_empty(&cs40l2x->pwle_segment_head)) {
		ret = -EINVAL;
		goto err_release;
	}

	list_for_each_entry_reverse(pwle_seg_struct,
		&cs40l2x->pwle_segment_head, list) {
		cs40l2x->two_bytes[0] = CS40L2X_ZERO_INIT;
		cs40l2x->two_bytes[1] = CS40L2X_ZERO_INIT;
		cs40l2x_to_bytes_msb(pwle_seg_struct->time,
			CS40L2X_TWO_BYTES, &two_bytes_data);
		unpadded_data[count] += ((cs40l2x->two_bytes[0] >>
			CS40L2X_FOUR_BITS) + b_four_time);
		count++;
		unpadded_data[count] = ((cs40l2x->two_bytes[0] <<
			CS40L2X_FOUR_BITS) + (cs40l2x->two_bytes[1] >>
			CS40L2X_FOUR_BITS));
		count++;
		time_byte = (cs40l2x->two_bytes[1] << CS40L2X_FOUR_BITS);
		cs40l2x->two_bytes[0] = CS40L2X_ZERO_INIT;
		cs40l2x->two_bytes[1] = CS40L2X_ZERO_INIT;
		cs40l2x_to_bytes_msb(pwle_seg_struct->level,
			CS40L2X_TWO_BYTES, &two_bytes_data);
		/* Last 4bits time + first 4bits level */
		unpadded_data[count] = ((cs40l2x->two_bytes[0] &
			CS40L2X_LS_FOUR_BYTE_MASK) + time_byte);
		count++;
		unpadded_data[count] = (cs40l2x->two_bytes[1]);
		count++;
		cs40l2x->two_bytes[0] = CS40L2X_ZERO_INIT;
		cs40l2x->two_bytes[1] = CS40L2X_ZERO_INIT;
		cs40l2x_to_bytes_msb(pwle_seg_struct->freq,
			CS40L2X_TWO_BYTES, &two_bytes_data);
		unpadded_data[count] = ((cs40l2x->two_bytes[0] <<
			CS40L2X_FOUR_BITS) + (cs40l2x->two_bytes[1] >>
			CS40L2X_FOUR_BITS));
		count++;
		unpadded_data[count] = (cs40l2x->two_bytes[1] <<
			CS40L2X_FOUR_BITS);
		if (pwle_seg_struct->chirp)
			unpadded_data[count] += CS40L2X_PWLE_CHIRP_BIT;
		if (pwle_seg_struct->brake)
			unpadded_data[count] += CS40L2X_PWLE_BRAKE_BIT;
		if (pwle_seg_struct->amp_reg) {
			unpadded_data[count] += CS40L2X_PWLE_AMP_REG_BIT;
			count++;
			cs40l2x->three_bytes[0] = CS40L2X_ZERO_INIT;
			cs40l2x->three_bytes[1] = CS40L2X_ZERO_INIT;
			cs40l2x->three_bytes[2] = CS40L2X_ZERO_INIT;
			cs40l2x_to_bytes_msb(pwle_seg_struct->vb_targ,
				CS40L2X_WT_WORD_SIZE, &three_bytes_data);
			unpadded_data[count] = (cs40l2x->three_bytes[0] >>
				CS40L2X_FOUR_BITS);
			count++;
			unpadded_data[count] = ((cs40l2x->three_bytes[0] <<
				CS40L2X_FOUR_BITS) + (cs40l2x->three_bytes[1] >>
				CS40L2X_FOUR_BITS));
			count++;
			unpadded_data[count] = ((cs40l2x->three_bytes[1] <<
				CS40L2X_FOUR_BITS) + (cs40l2x->three_bytes[2] >>
				CS40L2X_FOUR_BITS));
			count++;
			unpadded_data[count] = (cs40l2x->three_bytes[2] <<
				CS40L2X_FOUR_BITS);
			b_four_time = (cs40l2x->three_bytes[2] <<
				CS40L2X_FOUR_BITS);
		} else {
			count++;
			unpadded_data[count] = CS40L2X_ZERO_VAL;
			b_four_time = CS40L2X_ZERO_VAL;
			/* Last 4bits of chirp byte are unused */
		}
	}
	count++;
	unpadded_data[count] = CS40L2X_ZERO_VAL;
	count++;
	unpadded_data[count] = CS40L2X_ZERO_VAL;
	/* PWLE requires two zeros at end to complete last word */

	/* Add zero padding bytes */
	zero_pad_data[zero_pad_count] = CS40L2X_ZERO_VAL;
	zero_pad_count++;
	for (k = 0; k < count; k++) {
		zero_pad_data[zero_pad_count] = unpadded_data[k];
		zero_pad_count++;
		if ((((k + 1) % 3) == 0) && ((k + 1) != count)) {
			zero_pad_data[zero_pad_count] = CS40L2X_ZERO_VAL;
			zero_pad_count++;
		}
	}

	if (cs40l2x->save_pwle) {
		ret = cs40l2x_add_waveform_to_virtual_list(cs40l2x,
				CS40L2X_WT_TYPE_12_PWLE_FILE,
				cs40l2x->pwle_feature,
				zero_pad_size, zero_pad_data);
		if (!ret)
			cs40l2x->num_virtual_pwle_waves++;
	} else {
		cs40l2x_save_waveform_to_ovwr_struct(cs40l2x,
			CS40L2X_WT_TYPE_12_PWLE_FILE, cs40l2x->pwle_feature,
			zero_pad_size, zero_pad_data);
	}

err_release:
	devm_kfree(cs40l2x->dev, unpadded_data);
	devm_kfree(cs40l2x->dev, zero_pad_data);

	return ret;
}

static int cs40l2x_pwle_save_entry(struct cs40l2x_private *cs40l2x,
	char *pwle_seg_tok, unsigned int num_vals)
{
	unsigned int val;
	int ret;

	ret = kstrtou32(pwle_seg_tok, 10, &val);
	if (ret)
		return -EINVAL;

	if (val > 1) {
		dev_err(cs40l2x->dev, "Valid Save: 0 or 1\n");
		return -EINVAL;
	}

	if (num_vals == 0) {
		if (val)
			cs40l2x->save_pwle = true;
	} else {
		dev_err(cs40l2x->dev,
			"Malformed PWLE, missing Save entry\n");
		return -EINVAL;
	}

	return ret;
}

static int cs40l2x_pwle_wvfrm_feature_entry(
	struct cs40l2x_private *cs40l2x,
	char *pwle_seg_tok, unsigned int num_vals)
{
	unsigned int val;
	int ret;

	ret = kstrtou32(pwle_seg_tok, 10, &val);
	if (ret)
		return -EINVAL;

	if ((val > CS40L2X_PWLE_MAX_WVFRM_FEAT) ||
		((val % 4) != 0)) {
		dev_err(cs40l2x->dev,
			"Valid Waveform Feature: 0, 4, 8, 12\n");
		return -EINVAL;
	}

	if (num_vals == 1) {
		cs40l2x->pwle_feature =
			(val << CS40L2X_PWLE_WVFRM_FT_SHFT);
	} else {
		dev_err(cs40l2x->dev,
			"Malformed PWLE, missing Feature entry\n");
		return -EINVAL;
	}

	return ret;
}

static int cs40l2x_pwle_repeat_entry(struct cs40l2x_private *cs40l2x,
	char *pwle_seg_tok, unsigned int num_vals)
{
	unsigned int val;
	int ret;

	ret = kstrtou32(pwle_seg_tok, 10, &val);
	if (ret)
		return -EINVAL;

	if (val > CS40L2X_PWLE_MAX_RP_VAL) {
		dev_err(cs40l2x->dev, "Valid Repeat: 0 to 255\n");
		return -EINVAL;
	}

	if (num_vals == 2) {
		cs40l2x->pwle_repeat = val;
	} else {
		dev_err(cs40l2x->dev,
			"Malformed PWLE, missing Repeat entry\n");
		return -EINVAL;
	}

	return ret;
}

static int cs40l2x_pwle_wait_time_entry(struct cs40l2x_private *cs40l2x,
	char *pwle_seg_tok, char *pwle_dec, unsigned int num_vals)
{
	unsigned int adder = 0, val = 0;
	unsigned int dec_val;
	char *pwle_dec_tok;
	int ret;

	if (strnchr(pwle_dec, CS40L2X_PWLE_SEG_LEN_MAX, '.')) {
		if (strnchr(pwle_dec, 1, '.')) {
			pwle_dec++;
			ret = kstrtou32(pwle_dec, 10, &dec_val);
			if (ret)
				return -EINVAL;
			if ((dec_val == 5) &&
				(strnchr(pwle_dec, 1, '5')))
				dec_val *= 10;

			/*
			 * Wait time resolution is .25ms
			 * Add one to final wait time for each .25ms
			 */
			if (dec_val >= CS40L2X_PWLE_TIME_RES)
				adder++;
			if (dec_val >= (CS40L2X_PWLE_TIME_RES * 2))
				adder++;
			if (dec_val >= (CS40L2X_PWLE_TIME_RES * 3))
				adder++;
		} else {
			pwle_dec_tok = strsep(&pwle_dec, ".");
			ret = kstrtou32(pwle_dec_tok, 10, &val);

			pwle_dec_tok = strsep(&pwle_dec, ".");
			ret = kstrtou32(pwle_dec_tok, 10, &dec_val);
			if (ret)
				return -EINVAL;
			if ((dec_val == 5) &&
				(strnchr(pwle_dec_tok, 1, '5')))
				dec_val *= 10;

			if (dec_val >= CS40L2X_PWLE_TIME_RES)
				adder++;
			if (dec_val >= (CS40L2X_PWLE_TIME_RES * 2))
				adder++;
			if (dec_val >= (CS40L2X_PWLE_TIME_RES * 3))
				adder++;
		}
	} else {
		ret = kstrtou32(pwle_seg_tok, 10, &val);
		if (ret)
			return -EINVAL;
	}

	if (val > CS40L2X_PWLE_MAX_WT_VAL)
		return -EINVAL;

	if (num_vals == 3) {
		cs40l2x->wvfrm_len_wait_time = val;
		/* WaitTime = val * 4, due to PWLE spec */
		cs40l2x->pwle_wait_time = ((val * 4) + adder);
	} else {
		dev_err(cs40l2x->dev,
			"Malformed PWLE, WaitTime follows Repeat\n");
		return -EINVAL;
	}

	return ret;
}

static int cs40l2x_pwle_time_entry(struct cs40l2x_private *cs40l2x,
	char *pwle_seg_tok, char *pwle_dec, unsigned int num_vals,
	struct cs40l2x_pwle_segment *pwle_seg_struct,
	unsigned int *accu_time, bool *indef)
{
	unsigned int val, dec_val;
	unsigned int adder = 0;
	char *pwle_dec_tok;
	int ret;

	if (strnchr(pwle_seg_tok, CS40L2X_PWLE_SEG_LEN_MAX, '.')) {
		if (strnchr(pwle_dec, 1, '.')) {
			val = 0;
			pwle_dec++;
			ret = kstrtou32(pwle_dec, 10, &dec_val);
			if (ret)
				return -EINVAL;

			/* In case zero is omitted after .5 */
			if ((dec_val == 5) &&
				(strnchr(pwle_dec, 1, '5')))
				dec_val *= 10;

			/*
			 * Time resolution is .25ms
			 * Add one to final time for each .25ms
			 */
			if (dec_val >= CS40L2X_PWLE_TIME_RES)
				adder++;
			if (dec_val >= (CS40L2X_PWLE_TIME_RES * 2))
				adder++;
			if (dec_val >= (CS40L2X_PWLE_TIME_RES * 3))
				adder++;
		} else {
			pwle_dec_tok = strsep(&pwle_dec, ".");
			ret = kstrtou32(pwle_dec_tok, 10, &val);

			pwle_dec_tok = strsep(&pwle_dec, ".");
			ret = kstrtou32(pwle_dec_tok, 10, &dec_val);
			if (ret)
				return -EINVAL;

			/* In case zero is omitted after .5 */
			if ((dec_val == 5) &&
				(strnchr(pwle_dec_tok, 1, '5')))
				dec_val *= 10;

			if (dec_val >= CS40L2X_PWLE_TIME_RES)
				adder++;
			if (dec_val >= (CS40L2X_PWLE_TIME_RES * 2))
				adder++;
			if (dec_val >= (CS40L2X_PWLE_TIME_RES * 3))
				adder++;
		}
	} else {
		ret = kstrtou32(pwle_seg_tok, 10, &val);
		if (ret)
			return -EINVAL;
	}

	if (val == CS40L2X_PWLE_INDEF_TIME_VAL) {
		pwle_seg_struct->time = val;
		*indef = true;
	} else {
		if (val > CS40L2X_PWLE_MAX_TIME_VAL) {
			dev_err(cs40l2x->dev,
				"Valid Time: 0 to 16383.5ms, or 65535\n");
			return -EINVAL;
		}
		/* Time = val * 4, due to PWLE spec */
		pwle_seg_struct->time = ((val * 4) + adder);
		*accu_time += val;
	}

	return ret;
}

static int cs40l2x_pwle_level_entry(struct cs40l2x_private *cs40l2x,
	char *pwle_seg_tok, char *pwle_dec_tok, char *pwle_dec,
	struct cs40l2x_pwle_segment *pwle_seg_struct)
{
	unsigned int val, dec_val, limit;
	unsigned int leading_zeros = 0, modder = 0;
	bool only_whole = false, neg = false;
	int i, ret;

	if (strnchr(pwle_seg_tok, CS40L2X_PWLE_SEG_LEN_MAX, '.')) {
		if (strnchr(pwle_dec, 1, '-')) {
			neg = true;
			pwle_dec++;
		}
		if (strnchr(pwle_dec, 1, '.')) {
			val = 0;
			pwle_dec++;
			ret = kstrtou32(pwle_dec, 10, &dec_val);
			if (ret)
				return -EINVAL;

			if (strnchr(pwle_dec_tok, 1, '0')) {
				leading_zeros++;
				pwle_dec_tok++;
				if (strnchr(pwle_dec_tok, 1, '0')) {
					leading_zeros++;
					pwle_dec_tok++;
					if (strnchr(pwle_dec_tok, 1, '0'))
						leading_zeros++;
				}
			}
		} else {
			pwle_dec_tok = strsep(&pwle_dec, ".");
			ret = kstrtou32(pwle_dec_tok, 10, &val);
			if (val > 0) {
				dev_err(cs40l2x->dev,
					"Valid Level: -0.98256 to +0.98256\n");
				return -E2BIG;
			}

			pwle_dec_tok = strsep(&pwle_dec, ".");
			ret = kstrtou32(pwle_dec_tok, 10, &dec_val);
			if (ret)
				return -EINVAL;

			if (strnchr(pwle_dec_tok, 1, '0')) {
				leading_zeros++;
				pwle_dec_tok++;
				if (strnchr(pwle_dec_tok, 1, '0')) {
					leading_zeros++;
					pwle_dec_tok++;
					if (strnchr(pwle_dec_tok, 1, '0'))
						leading_zeros++;
				}
			}
		}
	} else {
		ret = kstrtou32(pwle_dec_tok, 10, &val);
		if (ret)
			return -EINVAL;

		if (val > 0) {
			dev_err(cs40l2x->dev,
				"Valid Level: -0.98256 to +0.98256\n");
			return -E2BIG;
		}
		only_whole = true;
	}

	if (!only_whole) {
		limit = (CS40L2X_PWLE_MAX_LV_RES_DIG - leading_zeros);
		modder = 10;
		for (i = 0; i < limit; i++) {
			if (dec_val < modder)
				dec_val = (dec_val * 10);
			modder = (modder * 10);
		}

		if (dec_val > CS40L2X_PWLE_MAX_LEV_VAL)
			return -EINVAL;

		/* Level = dec_val / 48, see PWLE spec */
		if (neg)
			val = ((dec_val / 48) + CS40L2X_PWLE_LEV_ADD_NEG);
		else
			val = (dec_val / 48);
	}

	pwle_seg_struct->level = val;

	return ret;
}

static int cs40l2x_pwle_frequency_entry(struct cs40l2x_private *cs40l2x,
	char *pwle_seg_tok, char *pwle_dec,
	struct cs40l2x_pwle_segment *pwle_seg_struct)
{
	unsigned int val, dec_val;
	unsigned int adder = 0;
	char *pwle_dec_tok;
	int ret;

	if (strnchr(pwle_seg_tok, CS40L2X_PWLE_SEG_LEN_MAX, '.')) {
		if (strnchr(pwle_dec, 1, '.')) {
			dev_err(cs40l2x->dev,
				"Valid Freq: 50.125 to 561.875 Hz\n");
			return -EINVAL;
		}

		pwle_dec_tok = strsep(&pwle_dec, ".");
		ret = kstrtou32(pwle_dec_tok, 10, &val);
		if (ret)
			return -EINVAL;

		pwle_dec_tok = strsep(&pwle_dec, ".");
		ret = kstrtou32(pwle_dec_tok, 10, &dec_val);
		if (ret)
			return -EINVAL;

		if ((dec_val == 25) &&
			(strnchr(pwle_dec_tok, 1, '2')))
			dec_val *= 10;
		if ((dec_val == 5) &&
			(strnchr(pwle_dec_tok, 1, '5')))
			dec_val *= 100;
		if ((dec_val == 50) &&
			(strnchr(pwle_dec_tok, 1, '5')))
			dec_val *= 10;
		if ((dec_val == 75) &&
			(strnchr(pwle_dec_tok, 1, '7')))
			dec_val *= 10;

		/*
		 * Frequency resolution is .125Hz
		 * Add one to final frequency for each .125Hz
		 */
		if (dec_val >= CS40L2X_PWLE_FREQ_RES)
			adder++;
		if (dec_val >= (CS40L2X_PWLE_FREQ_RES * 2))
			adder++;
		if (dec_val >= (CS40L2X_PWLE_FREQ_RES * 3))
			adder++;
		if (dec_val >= (CS40L2X_PWLE_FREQ_RES * 4))
			adder++;
		if (dec_val >= (CS40L2X_PWLE_FREQ_RES * 5))
			adder++;
		if (dec_val >= (CS40L2X_PWLE_FREQ_RES * 6))
			adder++;
		if (dec_val >= (CS40L2X_PWLE_FREQ_RES * 7))
			adder++;

	} else {
		ret = kstrtou32(pwle_seg_tok, 10, &val);
		if (ret)
			return -EINVAL;
	}

	if (val != 0) {
		if ((val > CS40L2X_PWLE_MAX_FREQ_VAL) ||
			(val < CS40L2X_PWLE_MIN_FREQ_VAL)) {
			dev_err(cs40l2x->dev,
					"Valid Freq: 50.125 to 561.875 Hz\n");
			return -EINVAL;
		}
		/* Freq = (val - 50) * 8, due to PWLE spec */
		pwle_seg_struct->freq = (((val - 50) * 8) + adder);
	} else {
		pwle_seg_struct->freq = 0;
	}

	return ret;
}

static int cs40l2x_pwle_vb_target_entry(struct cs40l2x_private *cs40l2x,
	char *pwle_seg_tok, char *pwle_dec, bool amp_reg,
	struct cs40l2x_pwle_segment *pwle_seg_struct)
{
	unsigned int val, dec_val, mod_val;
	unsigned int rounding_factor = 0;
	unsigned int leading_zeros = 0;
	unsigned int vb_target = 0;
	unsigned int modder = 0;
	unsigned int dig = 0;
	unsigned int limit;
	char *pwle_dec_tok;
	int i, ret;

	if (strnchr(pwle_seg_tok, CS40L2X_PWLE_SEG_LEN_MAX, '.')) {
		if (strnchr(pwle_dec, 1, '.')) {
			val = 0;
			pwle_dec++;
			ret = kstrtou32(pwle_dec, 10, &dec_val);
			if (ret)
				return -EINVAL;

		} else {
			pwle_dec_tok = strsep(&pwle_dec, ".");
			ret = kstrtou32(pwle_dec_tok, 10, &val);
			if (ret)
				return -EINVAL;

			if (val > 1) {
				dev_err(cs40l2x->dev,
					"Valid Vb Target: 0 to 1\n");
				return -E2BIG;
			}

			pwle_dec_tok = strsep(&pwle_dec, ".");
			ret = kstrtou32(pwle_dec_tok, 10, &dec_val);
			if (ret)
				return -EINVAL;

			if (strnchr(pwle_dec_tok, 1, '0')) {
				leading_zeros++;
				pwle_dec_tok++;
				if (strnchr(pwle_dec_tok, 1, '0')) {
					leading_zeros++;
					pwle_dec_tok++;
					if (strnchr(pwle_dec_tok, 1, '0'))
						leading_zeros++;
				}
			}
		}
	} else {
		ret = kstrtou32(pwle_seg_tok, 10, &val);
		if (ret)
			return -EINVAL;

		if (val > 1) {
			dev_err(cs40l2x->dev,
				"Valid Vb Target: 0 to 1\n");
			return -E2BIG;
		}
		dec_val = 0;
	}

	if (dec_val >= CS40L2X_PWLE_MAX_VB_RES) {
		dev_err(cs40l2x->dev,
				"Exceeded Vb Target resolution\n");
		return -EINVAL;
	}

	/*
	 * The section below converts decimal values into
	 * Q1.23 format without the use of floating point
	 * variables.  The numbers are not #defines in
	 * order to see which decimal point is being
	 * calculated more clearly.
	 */
	limit = (CS40L2X_PWLE_MAX_VB_RES_DIG - leading_zeros);
	modder = 10;
	for (i = 0; i < limit; i++) {
		if (dec_val < modder)
			dec_val = (dec_val * 10);
		modder = (modder * 10);
	}

	if (dec_val > 999999) {
		mod_val = (dec_val % 1000000);
		dig = ((dec_val - mod_val) / 1000000);
		rounding_factor = ((7 * dig) / 10);
		vb_target += ((dig *
			(CS40L2X_PWLE_MAX_VB_TARG / 10)) +
			rounding_factor);
		dec_val = mod_val;
	}
	if (dec_val > 99999) {
		mod_val = (dec_val % 100000);
		dig = ((dec_val - mod_val) / 100000);
		vb_target += (dig *
			(CS40L2X_PWLE_MAX_VB_TARG / 100));
		dec_val = mod_val;
	}
	if (dec_val > 9999) {
		mod_val = (dec_val % 10000);
		dig = ((dec_val - mod_val) / 10000);
		rounding_factor = ((6 * dig) / 10);
		vb_target += ((dig *
			(CS40L2X_PWLE_MAX_VB_TARG / 1000)) +
			rounding_factor);
		dec_val = mod_val;
	}
	if (dec_val > 999) {
		mod_val = (dec_val % 1000);
		dig = ((dec_val - mod_val) / 1000);
		rounding_factor = ((8 * dig) / 10);
		vb_target += ((dig *
			(CS40L2X_PWLE_MAX_VB_TARG / 10000)) +
			rounding_factor);
		dec_val = mod_val;
	}
	if (dec_val > 99) {
		mod_val = (dec_val % 100);
		dig = ((dec_val - mod_val) / 100);
		rounding_factor = ((8 * dig) / 10);
		vb_target += ((dig *
			(CS40L2X_PWLE_MAX_VB_TARG / 100000)) +
			rounding_factor);
		dec_val = mod_val;
	}
	if (dec_val > 9) {
		mod_val = (dec_val % 10);
		dig = ((dec_val - mod_val) / 10);
		rounding_factor = ((3 * dig) / 10);
		vb_target += ((dig *
			(CS40L2X_PWLE_MAX_VB_TARG / 1000000)) +
			rounding_factor);
	}

	/* Vb Target = Q1.23 fixed point, see PWLE spec */
	if (val == 1)
		val = CS40L2X_PWLE_MAX_VB_TARG;
	else
		val = vb_target;

	/* Only add vb_target to array if amp_reg set */
	if (amp_reg) {
		pwle_seg_struct->vb_targ = val;
		cs40l2x->pwle_num_vb_targs++;
		amp_reg = false;
	}

	return ret;
}

static ssize_t cs40l2x_pwle_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	struct i2c_client *i2c_client = to_i2c_client(cs40l2x->dev);
	struct cs40l2x_pwle_segment *pwle_seg_struct;
	char *pwle_str_alloc, *pwle_str, *pwle_str_tok;
	char *pwle_seg_alloc, *pwle_seg, *pwle_seg_tok;
	char *pwle_dec_alloc, *pwle_dec, *pwle_dec_tok;
	unsigned int accu_time = 0, num_vals = 0, num_segs = 0;
	unsigned int val;
	size_t pwle_seg_len;
	size_t pwle_dec_len;
	bool amp_reg = false, indef = false;
	bool t = false, l = false, f = false, c = false, b = false;
	bool a = false, v = false;
	int ret;

	/* This flag must be set before any possible return calls */
	cs40l2x->queue_stored = false;

	if (!cs40l2x->virtual_stored) {
		dev_err(dev, "Unsafe condition encountered.\n");
		return -EINVAL;
	}

	cs40l2x->pwle_num_vb_targs = 0;
	cs40l2x->pwle_num_segs = 0;
	cs40l2x->save_pwle = false;

	pwle_str_alloc = kzalloc(count, GFP_KERNEL);
	if (!pwle_str_alloc)
		return -ENOMEM;

	pwle_seg_alloc = kzalloc(CS40L2X_PWLE_SEG_LEN_MAX + 1, GFP_KERNEL);
	if (!pwle_seg_alloc) {
		kfree(pwle_str_alloc);
		return -ENOMEM;
	}

	pwle_dec_alloc = kzalloc(CS40L2X_PWLE_SEG_LEN_MAX + 1, GFP_KERNEL);
	if (!pwle_dec_alloc) {
		kfree(pwle_str_alloc);
		kfree(pwle_seg_alloc);
		return -ENOMEM;
	}

	pwle_seg_struct = devm_kzalloc(cs40l2x->dev,
		sizeof(*pwle_seg_struct), GFP_KERNEL);
	if (!pwle_seg_struct) {
		kfree(pwle_str_alloc);
		kfree(pwle_seg_alloc);
		kfree(pwle_dec_alloc);
		return -ENOMEM;
	}

	disable_irq(i2c_client->irq);

	mutex_lock(&cs40l2x->lock);

	pwle_str = pwle_str_alloc;
	strlcpy(pwle_str, buf, count);

	pwle_str_tok = strsep(&pwle_str, ",");

	while (pwle_str_tok) {
		if (num_vals >= CS40L2X_PWLE_TOTAL_VALS) {
			ret = -E2BIG;
			goto err_exit;
		}

		pwle_seg = pwle_seg_alloc;
		pwle_seg_len = strlcpy(pwle_seg, strim(pwle_str_tok),
			CS40L2X_PWLE_SEG_LEN_MAX + 1);
		if (pwle_seg_len > CS40L2X_PWLE_SEG_LEN_MAX) {
			ret = -E2BIG;
			goto err_exit;
		}

		if (strnchr(pwle_seg, CS40L2X_PWLE_SEG_LEN_MAX, ':')) {
			pwle_seg_tok = strsep(&pwle_seg, ":");

			if (strnchr(pwle_seg_tok, 1, 'S')) {
				pwle_seg_tok = strsep(&pwle_seg, ":");

				ret = cs40l2x_pwle_save_entry(cs40l2x,
					pwle_seg_tok, num_vals);
				if (ret)
					goto err_exit;
			}

			if (strnchr(pwle_seg_tok, 1, 'W')) {
				if (strnchr(pwle_seg_tok, 2, 'F')) {
					pwle_seg_tok = strsep(&pwle_seg, ":");

					ret = cs40l2x_pwle_wvfrm_feature_entry(
						cs40l2x, pwle_seg_tok,
						num_vals);
					if (ret)
						goto err_exit;
				}
			}

			if (strnchr(pwle_seg_tok, 1, 'R')) {
				if (strnchr(pwle_seg_tok, 2, 'P')) {
					pwle_seg_tok = strsep(&pwle_seg, ":");

					ret = cs40l2x_pwle_repeat_entry(cs40l2x,
						pwle_seg_tok, num_vals);
					if (ret)
						goto err_exit;
				}
			}

			if (strnchr(pwle_seg_tok, 1, 'W')) {
				if (strnchr(pwle_seg_tok, 2, 'T')) {
					pwle_seg_tok = strsep(&pwle_seg, ":");

					pwle_dec = pwle_dec_alloc;
					pwle_dec_len = strlcpy(pwle_dec,
						strim(pwle_seg_tok),
						CS40L2X_PWLE_SEG_LEN_MAX + 1);
					if (pwle_dec_len >
						CS40L2X_PWLE_SEG_LEN_MAX) {
						ret = -E2BIG;
						goto err_exit;
					}

					ret = cs40l2x_pwle_wait_time_entry(
						cs40l2x, pwle_seg_tok,
						pwle_dec, num_vals);
					if (ret)
						goto err_exit;
				}
			}

			if (strnchr(pwle_seg_tok, 1, 'T')) {
				if (num_vals > 4) {
					/* Verify complete previous segment */
					if ((!t) || (!l) || (!f) ||	(!c) ||
						(!b) || (!a) || (!v)) {
						dev_err(cs40l2x->dev,
							"Malformed PWLE. Missing entry in seg %d\n",
							(num_segs - 1));
						ret = -EINVAL;
						goto err_exit;
					}
					t = false;
					l = false;
					f = false;
					c = false;
					b = false;
					a = false;
					v = false;
				}

				pwle_seg_tok = strsep(&pwle_seg, ":");

				pwle_dec = pwle_dec_alloc;
				pwle_dec_len = strlcpy(pwle_dec,
					strim(pwle_seg_tok),
					CS40L2X_PWLE_SEG_LEN_MAX + 1);
				if (pwle_dec_len > CS40L2X_PWLE_SEG_LEN_MAX) {
					ret = -E2BIG;
					goto err_exit;
				}

				if (num_segs > 0) {
					pwle_seg_struct = devm_kzalloc(
						cs40l2x->dev,
						sizeof(*pwle_seg_struct),
						GFP_KERNEL);
					if (!pwle_seg_struct) {
						ret = -ENOMEM;
						goto err_exit;
					}
				}

				ret = cs40l2x_pwle_time_entry(cs40l2x,
					pwle_seg_tok, pwle_dec, num_vals,
					pwle_seg_struct, &accu_time, &indef);
				if (ret)
					goto err_exit;

				t = true;
				pwle_seg_struct->index = num_segs;
			}

			if (strnchr(pwle_seg_tok, 1, 'L')) {
				pwle_seg_tok = strsep(&pwle_seg, ":");
				pwle_dec_tok = pwle_seg_tok;

				pwle_dec = pwle_dec_alloc;
				pwle_dec_len = strlcpy(pwle_dec,
					strim(pwle_seg_tok),
					CS40L2X_PWLE_SEG_LEN_MAX + 1);
				if (pwle_dec_len > CS40L2X_PWLE_SEG_LEN_MAX) {
					ret = -E2BIG;
					goto err_exit;
				}

				ret = cs40l2x_pwle_level_entry(cs40l2x,
					pwle_seg_tok, pwle_dec_tok, pwle_dec,
					pwle_seg_struct);
				if (ret)
					goto err_exit;

				l = true;
			}

			if (strnchr(pwle_seg_tok, 1, 'F')) {
				pwle_seg_tok = strsep(&pwle_seg, ":");

				pwle_dec = pwle_dec_alloc;
				pwle_dec_len = strlcpy(pwle_dec,
					strim(pwle_seg_tok),
					CS40L2X_PWLE_SEG_LEN_MAX + 1);
				if (pwle_dec_len > CS40L2X_PWLE_SEG_LEN_MAX) {
					ret = -E2BIG;
					goto err_exit;
				}

				ret = cs40l2x_pwle_frequency_entry(cs40l2x,
					pwle_seg_tok, pwle_dec,
					pwle_seg_struct);
				if (ret)
					goto err_exit;

				f = true;
			}

			if (strnchr(pwle_seg_tok, 1, 'C')) {
				pwle_seg_tok = strsep(&pwle_seg, ":");

				ret = kstrtou32(pwle_seg_tok, 10, &val);
				if (ret) {
					ret = -EINVAL;
					goto err_exit;
				}

				if (val > 1) {
					dev_err(cs40l2x->dev,
						"Valid Chirp: 0 or 1\n");
					ret = -EINVAL;
					goto err_exit;
				}
				pwle_seg_struct->chirp = val;
				c = true;
			}

			if (strnchr(pwle_seg_tok, 1, 'B')) {
				pwle_seg_tok = strsep(&pwle_seg, ":");

				ret = kstrtou32(pwle_seg_tok, 10, &val);
				if (ret) {
					ret = -EINVAL;
					goto err_exit;
				}

				if (val > 1) {
					dev_err(cs40l2x->dev,
						"Valid Braking: 0 or 1\n");
					ret = -EINVAL;
					goto err_exit;
				}
				pwle_seg_struct->brake = val;
				b = true;
			}

			if (strnchr(pwle_seg_tok, 1, 'A')) {
				if (strnchr(pwle_seg_tok, 2, 'R')) {
					pwle_seg_tok = strsep(&pwle_seg, ":");

					ret = kstrtou32(pwle_seg_tok, 10, &val);
					if (ret) {
						ret = -EINVAL;
						goto err_exit;
					}

					if (val > 1) {
						dev_err(cs40l2x->dev,
							"Valid Amplitude Regulation: 0 or 1\n");
						ret = -EINVAL;
						goto err_exit;
					}
					pwle_seg_struct->amp_reg = val;
					a = true;

					if (val == 1)
						amp_reg = true;
				}
			}

			if (strnchr(pwle_seg_tok, 1, 'V')) {
				pwle_seg_tok = strsep(&pwle_seg, ":");

				pwle_dec = pwle_dec_alloc;
				pwle_dec_len = strlcpy(pwle_dec,
					strim(pwle_seg_tok),
					CS40L2X_PWLE_SEG_LEN_MAX + 1);
				if (pwle_dec_len > CS40L2X_PWLE_SEG_LEN_MAX) {
					ret = -E2BIG;
					goto err_exit;
				}

				ret = cs40l2x_pwle_vb_target_entry(cs40l2x,
					pwle_seg_tok, pwle_dec, amp_reg,
					pwle_seg_struct);
				if (ret)
					goto err_exit;

				v = true;

				list_add(&pwle_seg_struct->list,
					&cs40l2x->pwle_segment_head);

				num_segs++;
			}
		}
		num_vals++;
		pwle_str_tok = strsep(&pwle_str, ",");
	}

	/* Verify last segment was complete */
	if ((!t) || (!l) || (!f) || (!c) || (!b) || (!a) || (!v)) {
		dev_err(cs40l2x->dev,
			"Malformed PWLE. Missing entry in seg %d\n",
			(num_segs - 1));
		ret = -EINVAL;
		goto err_exit;
	}

	strlcpy(cs40l2x->pwle_str, buf, count);
	cs40l2x->pwle_str_size = count;

	cs40l2x->pwle_num_segs = num_segs;
	cs40l2x_calc_pwle_samples(cs40l2x, accu_time, indef);
	ret = cs40l2x_save_packed_pwle_data(cs40l2x);
	if (ret) {
		dev_err(cs40l2x->dev,
			"Malformed PWLE. No segments found.\n");
		ret = -EINVAL;
		goto err_exit;
	}
	cs40l2x_pwle_seg_free(cs40l2x);

	ret = count;

	if (!cs40l2x->save_pwle)
		cs40l2x->last_type_entered = CS40L2X_WT_TYPE_12_PWLE_FILE;
	cs40l2x->queue_stored = true;

err_exit:
	mutex_unlock(&cs40l2x->lock);

	kfree(pwle_str_alloc);
	kfree(pwle_seg_alloc);
	kfree(pwle_dec_alloc);

	enable_irq(i2c_client->irq);

	return ret;
}

static ssize_t cs40l2x_pwle_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	ssize_t len = 0;
	int i;

	mutex_lock(&cs40l2x->lock);

	for (i = 0; i < cs40l2x->pwle_str_size; i++)
		len += snprintf(buf + len, PAGE_SIZE - len, "%c",
			cs40l2x->pwle_str[i]);
	len += snprintf(buf + len, PAGE_SIZE - len, "\n");

	mutex_unlock(&cs40l2x->lock);

	return len;
}

static ssize_t cs40l2x_composite_indexes_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	struct cs40l2x_virtual_waveform *virtual_wav;
	ssize_t len = 0;
	int count = 0;

	mutex_lock(&cs40l2x->lock);

	list_for_each_entry_reverse(virtual_wav,
		&cs40l2x->virtual_waveform_head, list) {
		if (virtual_wav->wvfrm_type != CS40L2X_WT_TYPE_10_COMP_FILE)
			continue;

		count++;

		len += snprintf(buf + len, PAGE_SIZE - len, "%d",
			virtual_wav->index);

		if (count != (cs40l2x->num_virtual_waves -
			cs40l2x->num_virtual_pwle_waves))
			len += snprintf(buf + len, PAGE_SIZE - len, ", ");
	}
	len += snprintf(buf + len, PAGE_SIZE - len, "\n");

	mutex_unlock(&cs40l2x->lock);

	return len;
}

static ssize_t cs40l2x_pwle_indexes_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	struct cs40l2x_virtual_waveform *virtual_wav;
	ssize_t len = 0;
	int count = 0;

	mutex_lock(&cs40l2x->lock);

	list_for_each_entry_reverse(virtual_wav,
		&cs40l2x->virtual_waveform_head, list) {
		if (virtual_wav->wvfrm_type != CS40L2X_WT_TYPE_12_PWLE_FILE)
			continue;

		count++;

		len += snprintf(buf + len, PAGE_SIZE - len, "%d",
			virtual_wav->index);

		if (count != cs40l2x->num_virtual_pwle_waves)
			len += snprintf(buf + len, PAGE_SIZE - len, ", ");
	}
	len += snprintf(buf + len, PAGE_SIZE - len, "\n");

	mutex_unlock(&cs40l2x->lock);

	return len;
}

static ssize_t cs40l2x_available_pwle_segs_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	unsigned int index;

	index = cs40l2x->display_pwle_segs;

	return snprintf(buf, PAGE_SIZE, "%d\n", index);
}

static unsigned int cs40l2x_dsp_reg(struct cs40l2x_private *cs40l2x,
			const char *coeff_name, const unsigned int block_type,
			const unsigned int algo_id)
{
	struct cs40l2x_coeff_desc *coeff_desc;

	list_for_each_entry(coeff_desc, &cs40l2x->coeff_desc_head, list) {
		if (strncmp(coeff_desc->name, coeff_name,
				CS40L2X_COEFF_NAME_LEN_MAX))
			continue;
		if (coeff_desc->block_type != block_type)
			continue;
		if (coeff_desc->parent_id != algo_id)
			continue;

		return coeff_desc->reg;
	}

	return 0;
}

static int cs40l2x_dsp_cache(struct cs40l2x_private *cs40l2x,
			unsigned int reg, unsigned int val)
{
	int i;

	for (i = 0; i < cs40l2x->dsp_cache_depth; i++)
		if (cs40l2x->dsp_cache[i].reg == reg) {
			cs40l2x->dsp_cache[i].val = val;
			return 0;
		}

	if (i == CS40L2X_DSP_CACHE_MAX)
		return -E2BIG;

	cs40l2x->dsp_cache[cs40l2x->dsp_cache_depth].reg = reg;
	cs40l2x->dsp_cache[cs40l2x->dsp_cache_depth++].val = val;

	return 0;
}

static int cs40l2x_wseq_add_reg(struct cs40l2x_private *cs40l2x,
			unsigned int reg, unsigned int val)
{
	if (cs40l2x->wseq_length == CS40L2X_WSEQ_LENGTH_MAX)
		return -E2BIG;

	cs40l2x->wseq_table[cs40l2x->wseq_length].reg = reg;
	cs40l2x->wseq_table[cs40l2x->wseq_length++].val = val;

	return 0;
}

static int cs40l2x_wseq_add_seq(struct cs40l2x_private *cs40l2x,
			const struct reg_sequence *seq, unsigned int len)
{
	int ret, i;

	for (i = 0; i < len; i++) {
		ret = cs40l2x_wseq_add_reg(cs40l2x, seq[i].reg, seq[i].def);
		if (ret)
			return ret;
	}

	return 0;
}

static int cs40l2x_wseq_write(struct cs40l2x_private *cs40l2x, unsigned int pos,
			unsigned int reg, unsigned int val)
{
	unsigned int wseq_base = cs40l2x_dsp_reg(cs40l2x, "POWERONSEQUENCE",
			CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
	int ret;

	/* missing write sequencer simply means there is nothing to do here */
	if (!wseq_base)
		return 0;

	/* upper half */
	ret = regmap_write(cs40l2x->regmap,
			wseq_base + pos * CS40L2X_WSEQ_STRIDE,
			((reg & CS40L2X_WSEQ_REG_MASK1)
				<< CS40L2X_WSEQ_REG_SHIFTUP) |
					((val & CS40L2X_WSEQ_VAL_MASK1)
						>> CS40L2X_WSEQ_VAL_SHIFTDN));
	if (ret)
		return ret;

	/* lower half */
	return regmap_write(cs40l2x->regmap,
			wseq_base + pos * CS40L2X_WSEQ_STRIDE + 4,
			val & CS40L2X_WSEQ_VAL_MASK2);
}

static int cs40l2x_wseq_init(struct cs40l2x_private *cs40l2x)
{
	unsigned int wseq_base = cs40l2x_dsp_reg(cs40l2x, "POWERONSEQUENCE",
			CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
	int ret, i;

	if (!wseq_base)
		return 0;

	for (i = 0; i < cs40l2x->wseq_length; i++) {
		ret = cs40l2x_wseq_write(cs40l2x, i,
				cs40l2x->wseq_table[i].reg,
				cs40l2x->wseq_table[i].val);
		if (ret)
			return ret;
	}

	return regmap_write(cs40l2x->regmap,
			wseq_base + cs40l2x->wseq_length * CS40L2X_WSEQ_STRIDE,
			CS40L2X_WSEQ_LIST_TERM);
}

static int cs40l2x_wseq_replace(struct cs40l2x_private *cs40l2x,
			unsigned int reg, unsigned int val)
{
	int i;

	for (i = 0; i < cs40l2x->wseq_length; i++)
		if (cs40l2x->wseq_table[i].reg == reg)
			break;

	if (i == cs40l2x->wseq_length)
		return -EINVAL;

	cs40l2x->wseq_table[i].val = val;

	return cs40l2x_wseq_write(cs40l2x, i, reg, val);
}

static int cs40l2x_user_ctrl_exec(struct cs40l2x_private *cs40l2x,
			unsigned int user_ctrl_cmd, unsigned int user_ctrl_data,
			unsigned int *user_ctrl_resp)
{
	struct regmap *regmap = cs40l2x->regmap;
	struct device *dev = cs40l2x->dev;
	unsigned int user_ctrl_reg = cs40l2x_dsp_reg(cs40l2x,
			"USER_CONTROL_IPDATA",
			CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
	int ret;

	if (!user_ctrl_reg)
		return -EPERM;

	ret = regmap_write(regmap, user_ctrl_reg, user_ctrl_data);
	if (ret) {
		dev_err(dev, "Failed to write user-control data\n");
		return ret;
	}

	ret = cs40l2x_ack_write(cs40l2x, CS40L2X_MBOX_USER_CONTROL,
			user_ctrl_cmd, CS40L2X_USER_CTRL_SUCCESS);
	if (ret)
		return ret;

	if (!user_ctrl_resp)
		return 0;

	return regmap_read(regmap,
			cs40l2x_dsp_reg(cs40l2x, "USER_CONTROL_RESPONSE",
					CS40L2X_XM_UNPACKED_TYPE,
					cs40l2x->fw_desc->id),
			user_ctrl_resp);
}

static ssize_t cs40l2x_cp_trigger_duration_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int index, val;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	index = cs40l2x->cp_trigger_index;

	switch (cs40l2x->fw_desc->id) {
	case CS40L2X_FW_ID_ORIG:
		ret = -EPERM;
		goto err_mutex;
	case CS40L2X_FW_ID_CAL:
		if (index != CS40L2X_INDEX_QEST) {
			ret = -EINVAL;
			goto err_mutex;
		}

		if (cs40l2x->diag_state < CS40L2X_DIAG_STATE_DONE1) {
			ret = -ENODATA;
			goto err_mutex;
		}

		ret = regmap_read(cs40l2x->regmap,
				cs40l2x_dsp_reg(cs40l2x, "TONE_DURATION_MS",
						CS40L2X_XM_UNPACKED_TYPE,
						CS40L2X_ALGO_ID_QEST),
				&val);
		if (ret)
			goto err_mutex;

		if (val == CS40L2X_TONE_DURATION_MS_NONE) {
			ret = -ENODATA;
			goto err_mutex;
		}

		val *= CS40L2X_QEST_SRATE;
		break;
	default:
		if (index < CS40L2X_INDEX_CLICK_MIN
				|| index > CS40L2X_INDEX_CLICK_MAX) {
			ret = -EINVAL;
			goto err_mutex;
		}

		ret = cs40l2x_user_ctrl_exec(cs40l2x,
				CS40L2X_USER_CTRL_DURATION, index, &val);
		if (ret)
			goto err_mutex;
	}

	ret = snprintf(buf, PAGE_SIZE, "%d\n", val);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_cp_trigger_q_sub_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int val;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_user_ctrl_exec(cs40l2x, CS40L2X_USER_CTRL_Q_INDEX,
			cs40l2x->cp_trigger_index, &val);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%u\n", val);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static int cs40l2x_hiber_cmd_send(struct cs40l2x_private *cs40l2x,
			unsigned int hiber_cmd)
{
	struct regmap *regmap = cs40l2x->regmap;
	unsigned int val;
	int ret, i, j;

	switch (hiber_cmd) {
	case CS40L2X_POWERCONTROL_NONE:
	case CS40L2X_POWERCONTROL_FRC_STDBY:
		return cs40l2x_ack_write(cs40l2x,
				CS40L2X_MBOX_POWERCONTROL, hiber_cmd,
				CS40L2X_POWERCONTROL_NONE);

	case CS40L2X_POWERCONTROL_HIBERNATE:
		/*
		 * control port is unavailable immediately after
		 * this write, so don't poll for acknowledgment
		 */
		return regmap_write(regmap,
				CS40L2X_MBOX_POWERCONTROL, hiber_cmd);

	case CS40L2X_POWERCONTROL_WAKEUP:
		for (i = 0; i < CS40L2X_WAKEUP_RETRIES; i++) {
			/*
			 * the first several transactions are expected to be
			 * NAK'd, so retry multiple times in rapid succession
			 */
			ret = regmap_write(regmap,
					CS40L2X_MBOX_POWERCONTROL, hiber_cmd);
			if (ret) {
				usleep_range(1000, 1100);
				continue;
			}

			/*
			 * verify the previous firmware ID remains intact and
			 * brute-force a dummy hibernation cycle if otherwise
			 */
			for (j = 0; j < CS40L2X_STATUS_RETRIES; j++) {
				usleep_range(5000, 5100);

				ret = regmap_read(regmap,
						CS40L2X_XM_FW_ID, &val);
				if (ret)
					return ret;

				if (val == cs40l2x->fw_desc->id)
					break;
			}
			if (j < CS40L2X_STATUS_RETRIES)
				break;

			dev_warn(cs40l2x->dev,
					"Unexpected firmware ID: 0x%06X\n",
					val);

			/*
			 * this write may force the device into hibernation
			 * before the ACK is returned, so ignore the return
			 * value
			 */
			regmap_write(regmap, CS40L2X_PWRMGT_CTL,
					(1 << CS40L2X_MEM_RDY_SHIFT) |
					(1 << CS40L2X_TRIG_HIBER_SHIFT));

			usleep_range(1000, 1100);
		}
		if (i == CS40L2X_WAKEUP_RETRIES)
			return -EIO;

		for (i = 0; i < CS40L2X_STATUS_RETRIES; i++) {
			ret = regmap_read(regmap,
					cs40l2x_dsp_reg(cs40l2x, "POWERSTATE",
						CS40L2X_XM_UNPACKED_TYPE,
						cs40l2x->fw_desc->id),
					&val);
			if (ret)
				return ret;

			switch (val) {
			case CS40L2X_POWERSTATE_ACTIVE:
			case CS40L2X_POWERSTATE_STANDBY:
				return 0;
			case CS40L2X_POWERSTATE_HIBERNATE:
				break;
			default:
				return -EINVAL;
			}

			usleep_range(5000, 5100);
		}
		return -ETIME;

	default:
		return -EINVAL;
	}
}

static ssize_t cs40l2x_hiber_cmd_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int hiber_cmd;

	ret = kstrtou32(buf, 10, &hiber_cmd);
	if (ret)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	if (cs40l2x->fw_desc->id == CS40L2X_FW_ID_CAL
			|| cs40l2x->fw_desc->id == CS40L2X_FW_ID_ORIG) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = cs40l2x_hiber_cmd_send(cs40l2x, hiber_cmd);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_hiber_timeout_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "FALSEI2CTIMEOUT",
			CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_read(cs40l2x->regmap, reg, &val);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%d\n", val);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_hiber_timeout_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	ret = kstrtou32(buf, 10, &val);
	if (ret)
		return -EINVAL;

	if (val < CS40L2X_FALSEI2CTIMEOUT_MIN)
		return -EINVAL;

	if (val > CS40L2X_FALSEI2CTIMEOUT_MAX)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "FALSEI2CTIMEOUT",
			CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_write(cs40l2x->regmap, reg, val);
	if (ret)
		goto err_mutex;

	ret = cs40l2x_dsp_cache(cs40l2x, reg, val);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio1_enable_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "GPIO_ENABLE",
			CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_read(cs40l2x->regmap, reg, &val);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%d\n", val);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio1_enable_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	ret = kstrtou32(buf, 10, &val);
	if (ret)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "GPIO_ENABLE",
			CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_write(cs40l2x->regmap, reg,
			val ? CS40L2X_GPIO1_ENABLED : CS40L2X_GPIO1_DISABLED);
	if (ret)
		goto err_mutex;

	ret = cs40l2x_dsp_cache(cs40l2x, reg,
			val ? CS40L2X_GPIO1_ENABLED : CS40L2X_GPIO1_DISABLED);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static int cs40l2x_gpio_edge_index_get(struct cs40l2x_private *cs40l2x,
			unsigned int *index,
			unsigned int gpio_offs, bool gpio_rise)
{
	int ret;
	bool gpio_pol = cs40l2x->pdata.gpio_indv_pol & (1 << (gpio_offs >> 2));
	unsigned int reg = cs40l2x_dsp_reg(cs40l2x,
			gpio_pol ^ gpio_rise ? "INDEXBUTTONPRESS" :
				"INDEXBUTTONRELEASE",
			CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);

	if (!reg)
		return -EPERM;
	reg += gpio_offs;

	if (!(cs40l2x->gpio_mask & (1 << (gpio_offs >> 2))))
		return -EPERM;

	ret = regmap_read(cs40l2x->regmap, reg, index);

	if (cs40l2x->virtual_bin) {
		if (*index == cs40l2x->virtual_gpio1_fall_index)
			*index =
			cs40l2x->virtual_gpio_index[CS40L2X_GPIO_FALL];
		if (*index == cs40l2x->virtual_gpio1_rise_index)
			*index =
			cs40l2x->virtual_gpio_index[CS40L2X_GPIO_RISE];
	}

	return ret;
}

static int cs40l2x_gpio_edge_index_set(struct cs40l2x_private *cs40l2x,
			unsigned int index,
			unsigned int gpio_offs, bool gpio_rise)
{
	bool gpio_pol = cs40l2x->pdata.gpio_indv_pol & (1 << (gpio_offs >> 2));
	unsigned int reg = cs40l2x_dsp_reg(cs40l2x,
			gpio_pol ^ gpio_rise ? "INDEXBUTTONPRESS" :
				"INDEXBUTTONRELEASE",
			CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
	int ret;
	int r = CS40L2X_GPIO_RISE;
	int f = CS40L2X_GPIO_FALL;

	if (!reg)
		return -EPERM;
	reg += gpio_offs;

	if (!(cs40l2x->gpio_mask & (1 << (gpio_offs >> 2))))
		return -EPERM;

	if (!cs40l2x->virtual_bin) {
		if (index > (cs40l2x->num_waves - CS40L2X_WT_NUM_VIRT_SLOTS))
			return -EINVAL;
	} else {
		if (index >=
			(cs40l2x->num_waves - CS40L2X_WT_NUM_VIRT_SLOTS)) {
			if (index >=
				((cs40l2x->num_waves +
					cs40l2x->num_virtual_waves) -
					CS40L2X_WT_NUM_VIRT_SLOTS)) {
				return -EINVAL;
			}
			if (gpio_offs == 0) {
				if (gpio_rise) {
					index =
					cs40l2x->virtual_gpio1_rise_index;
					if (cs40l2x->virtual_gpio_index[r] !=
						cs40l2x->loaded_gpio_index[r])
						cs40l2x_write_virtual_waveform(
						cs40l2x,
						cs40l2x->virtual_gpio_index[r],
						true, true, false);
					/* else virtual wvfrm already loaded */
				} else {
					index =
					cs40l2x->virtual_gpio1_fall_index;
					if (cs40l2x->virtual_gpio_index[f] !=
						cs40l2x->loaded_gpio_index[f])
						cs40l2x_write_virtual_waveform(
						cs40l2x,
						cs40l2x->virtual_gpio_index[f],
						true, false, false);
				}
			}
		}
	}

	ret = regmap_write(cs40l2x->regmap, reg, index);
	if (ret)
		return ret;

	return cs40l2x_dsp_cache(cs40l2x, reg, index);
}

static ssize_t cs40l2x_gpio1_rise_index_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int index;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_gpio_edge_index_get(cs40l2x, &index,
			CS40L2X_INDEXBUTTONPRESS1, CS40L2X_GPIO_RISE);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%u\n", index);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio1_rise_index_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int index;

	ret = kstrtou32(buf, 10, &index);
	if (ret)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	cs40l2x->virtual_gpio_index[CS40L2X_GPIO_RISE] = index;

	ret = cs40l2x_gpio_edge_index_set(cs40l2x, index,
			CS40L2X_INDEXBUTTONPRESS1, CS40L2X_GPIO_RISE);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio1_fall_index_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int index;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_gpio_edge_index_get(cs40l2x, &index,
			CS40L2X_INDEXBUTTONRELEASE1, CS40L2X_GPIO_FALL);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%u\n", index);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio1_fall_index_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int index;

	ret = kstrtou32(buf, 10, &index);
	if (ret)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	cs40l2x->virtual_gpio_index[CS40L2X_GPIO_FALL] = index;

	ret = cs40l2x_gpio_edge_index_set(cs40l2x, index,
			CS40L2X_INDEXBUTTONRELEASE1, CS40L2X_GPIO_FALL);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio1_fall_timeout_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "PRESS_RELEASE_TIMEOUT",
			CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_read(cs40l2x->regmap, reg, &val);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%d\n", val);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio1_fall_timeout_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	ret = kstrtou32(buf, 10, &val);
	if (ret)
		return -EINVAL;

	if (val > CS40L2X_PR_TIMEOUT_MAX)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "PRESS_RELEASE_TIMEOUT",
			CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_write(cs40l2x->regmap, reg, val);
	if (ret)
		goto err_mutex;

	ret = cs40l2x_dsp_cache(cs40l2x, reg, val);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio2_rise_index_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int index;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_gpio_edge_index_get(cs40l2x, &index,
			CS40L2X_INDEXBUTTONPRESS2, CS40L2X_GPIO_RISE);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%u\n", index);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio2_rise_index_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int index;

	ret = kstrtou32(buf, 10, &index);
	if (ret)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_gpio_edge_index_set(cs40l2x, index,
			CS40L2X_INDEXBUTTONPRESS2, CS40L2X_GPIO_RISE);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio2_fall_index_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int index;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_gpio_edge_index_get(cs40l2x, &index,
			CS40L2X_INDEXBUTTONRELEASE2, CS40L2X_GPIO_FALL);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%u\n", index);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio2_fall_index_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int index;

	ret = kstrtou32(buf, 10, &index);
	if (ret)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_gpio_edge_index_set(cs40l2x, index,
			CS40L2X_INDEXBUTTONRELEASE2, CS40L2X_GPIO_FALL);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio3_rise_index_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int index;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_gpio_edge_index_get(cs40l2x, &index,
			CS40L2X_INDEXBUTTONPRESS3, CS40L2X_GPIO_RISE);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%u\n", index);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio3_rise_index_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int index;

	ret = kstrtou32(buf, 10, &index);
	if (ret)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_gpio_edge_index_set(cs40l2x, index,
			CS40L2X_INDEXBUTTONPRESS3, CS40L2X_GPIO_RISE);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio3_fall_index_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int index;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_gpio_edge_index_get(cs40l2x, &index,
			CS40L2X_INDEXBUTTONRELEASE3, CS40L2X_GPIO_FALL);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%u\n", index);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio3_fall_index_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int index;

	ret = kstrtou32(buf, 10, &index);
	if (ret)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_gpio_edge_index_set(cs40l2x, index,
			CS40L2X_INDEXBUTTONRELEASE3, CS40L2X_GPIO_FALL);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio4_rise_index_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int index;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_gpio_edge_index_get(cs40l2x, &index,
			CS40L2X_INDEXBUTTONPRESS4, CS40L2X_GPIO_RISE);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%u\n", index);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio4_rise_index_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int index;

	ret = kstrtou32(buf, 10, &index);
	if (ret)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_gpio_edge_index_set(cs40l2x, index,
			CS40L2X_INDEXBUTTONPRESS4, CS40L2X_GPIO_RISE);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio4_fall_index_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int index;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_gpio_edge_index_get(cs40l2x, &index,
			CS40L2X_INDEXBUTTONRELEASE4, CS40L2X_GPIO_FALL);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%u\n", index);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio4_fall_index_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int index;

	ret = kstrtou32(buf, 10, &index);
	if (ret)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_gpio_edge_index_set(cs40l2x, index,
			CS40L2X_INDEXBUTTONRELEASE4, CS40L2X_GPIO_FALL);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_standby_timeout_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "EVENT_TIMEOUT",
			CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_read(cs40l2x->regmap, reg, &val);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%d\n", val);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_standby_timeout_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	ret = kstrtou32(buf, 10, &val);
	if (ret)
		return -EINVAL;

	if (val > CS40L2X_EVENT_TIMEOUT_MAX)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "EVENT_TIMEOUT",
			CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_write(cs40l2x->regmap, reg, val);
	if (ret)
		goto err_mutex;

	ret = cs40l2x_dsp_cache(cs40l2x, reg, val);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_f0_measured_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	if (cs40l2x->diag_state < CS40L2X_DIAG_STATE_DONE1) {
		ret = -ENODATA;
		goto err_mutex;
	}

	ret = snprintf(buf, PAGE_SIZE, "%d\n", cs40l2x->f0_measured);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_f0_stored_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "F0_STORED",
			CS40L2X_XM_UNPACKED_TYPE,
			cs40l2x->fw_desc->id == CS40L2X_FW_ID_ORIG ?
				CS40L2X_ALGO_ID_F0 : cs40l2x->fw_desc->id);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_read(cs40l2x->regmap, reg, &val);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%d\n", val);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_f0_stored_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	ret = kstrtou32(buf, 10, &val);
	if (ret)
		return -EINVAL;

	if (cs40l2x->pdata.f0_min > 0 && val < cs40l2x->pdata.f0_min)
		return -EINVAL;

	if (cs40l2x->pdata.f0_max > 0 && val > cs40l2x->pdata.f0_max)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "F0_STORED",
			CS40L2X_XM_UNPACKED_TYPE,
			cs40l2x->fw_desc->id == CS40L2X_FW_ID_ORIG ?
				CS40L2X_ALGO_ID_F0 : cs40l2x->fw_desc->id);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_write(cs40l2x->regmap, reg, val);
	if (ret)
		goto err_mutex;

	ret = cs40l2x_dsp_cache(cs40l2x, reg, val);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_die_temp_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	struct regmap *regmap = cs40l2x->regmap;
	int ret;
	unsigned int val;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	/* Set filter value to 4 samples */
	ret = regmap_write(regmap, CS40L2X_DTEMP_CFG, 2);
	if (ret)
		goto err_die_temp;

	ret = regmap_read(regmap, CS40L2X_DTEMP_EN, &val);
	if (ret)
		goto err_die_temp;

	ret = snprintf(buf, PAGE_SIZE, "%d\n",
			val >> CS40L2X_DTEMP_FLT_RES_SHIFT);

err_die_temp:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_vbst_avg_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	struct regmap *regmap = cs40l2x->regmap;
	int ret = 0;
	unsigned int reg, val;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = regmap_write(regmap, CS40L2X_DSP1_RX8_SRC,
					CS40L2X_SRC_VBSTMON);
	if (ret)
		goto err_vbst_avg;

	reg = cs40l2x_dsp_reg(cs40l2x, "VBST_AVG",
			CS40L2X_XM_UNPACKED_TYPE,
			cs40l2x->fw_desc->id);
	if (!reg) {
		ret = -EINVAL;
		goto err_vbst_avg;
	}

	usleep_range(1000, 1500);
	ret = regmap_read(regmap, reg, &val);
	if (ret)
		goto err_vbst_avg;

	/* The value is unsigned Q0.24 */
	ret = snprintf(buf, PAGE_SIZE, "%d\n",  val);

err_vbst_avg:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_bemf_measured_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;

	mutex_lock(&cs40l2x->lock);

	if (cs40l2x->diag_state < CS40L2X_DIAG_STATE_DONE1) {
		ret = -ENODATA;
		goto err_bemf;
	}

	ret = snprintf(buf, PAGE_SIZE, "%d\n", cs40l2x->bemf_measured);

err_bemf:
	mutex_unlock(&cs40l2x->lock);

	return ret;
}

static ssize_t cs40l2x_dyn_f0_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret = 0, i;

	mutex_lock(&cs40l2x->lock);

	for (i = 0; i < CS40l2X_F0_MAX_ENTRIES; i++) {
		if (!cs40l2x->dynamic_f0[i].changed)
			continue;

		ret += snprintf(buf, PAGE_SIZE, "%d %d\n",
					cs40l2x->dynamic_f0[i].index,
					cs40l2x->dynamic_f0[i].f0);
		buf += strlen(buf);
	}
	mutex_unlock(&cs40l2x->lock);

	return ret;
}

static ssize_t cs40l2x_dyn_f0_index_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;

	mutex_lock(&cs40l2x->lock);

	ret = snprintf(buf, PAGE_SIZE, "%d\n", cs40l2x->dynamic_f0_index);

	mutex_unlock(&cs40l2x->lock);

	return ret;
}

static ssize_t cs40l2x_dyn_f0_index_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int val;

	ret = kstrtou32(buf, 10, &val);
	if (ret)
		return -EINVAL;

	if (val < 0 || val > CS40l2X_F0_MAX_ENTRIES - 1) {
		dev_err(dev, "Invalid index value %d\n", val);
		return -EINVAL;
	}

	mutex_lock(&cs40l2x->lock);

	cs40l2x->dynamic_f0_index = val;

	mutex_unlock(&cs40l2x->lock);

	return count;
}

static ssize_t cs40l2x_dyn_f0_val_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	struct regmap *regmap = cs40l2x->regmap;
	unsigned int val, reg;
	int ret, i, loc = -1, index = cs40l2x->dynamic_f0_index;

	ret = kstrtou32(buf, 10, &val);
	if (ret)
		return -EINVAL;

	if (val > CS40L2X_DYN_F0_MASK) {
		dev_err(dev, "Invalid f0 value %d\n", val);
		return -EINVAL;
	}

	reg = cs40l2x_dsp_reg(cs40l2x, "DYN_F0_TABLE", CS40L2X_XM_UNPACKED_TYPE,
				CS40L2X_ALGO_ID_DYN_F0);
	if (!reg) {
		dev_err(dev, "Cannot get the register for the f0 table\n");
		return -EINVAL;
	}

	mutex_lock(&cs40l2x->lock);

	for (i = 0; i < CS40l2X_F0_MAX_ENTRIES; i++) {
		if (!cs40l2x->dynamic_f0[i].changed) {
			if (loc < 0)
				loc = i;

			continue;
		}

		if (index == cs40l2x->dynamic_f0[i].index)
			break;
	}

	/* Nothing exists in the table, start from first available element */
	if (i == CS40l2X_F0_MAX_ENTRIES) {
		if (loc >= 0) {
			i = loc;
		} else {
			dev_err(dev, "Can't find F0 index.\n");
			ret = -EINVAL;
			goto err_mutex;
		}
	}

	ret = regmap_write(regmap, reg + (i*4),
		val | (index << CS40L2X_DYN_F0_INDEX_SHIFT));
	if (ret)
		goto err_mutex;

	cs40l2x->dynamic_f0[i].f0 = val;
	cs40l2x->dynamic_f0[i].index = index;
	cs40l2x->dynamic_f0[i].changed = true;

	ret = count;
err_mutex:
	mutex_unlock(&cs40l2x->lock);

	return ret;
}

static ssize_t cs40l2x_dyn_f0_val_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret = 0, i, index = cs40l2x->dynamic_f0_index;

	mutex_lock(&cs40l2x->lock);

	for (i = 0; i < CS40l2X_F0_MAX_ENTRIES; i++)
		if (index == cs40l2x->dynamic_f0[i].index)
			break;

	if (i == CS40l2X_F0_MAX_ENTRIES) {
		dev_err(dev, "Cannot find f0 index %d\n", index);
		ret = -EINVAL;
		goto err_mutex;
	}

	ret = snprintf(buf, PAGE_SIZE, "%d\n", cs40l2x->dynamic_f0[i].f0);

err_mutex:
	mutex_unlock(&cs40l2x->lock);

	return ret;
}

static ssize_t cs40l2x_f0_offset_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "F0_OFFSET",
			CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_read(cs40l2x->regmap, reg, &val);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%d\n", val);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_f0_offset_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	ret = kstrtou32(buf, 10, &val);
	if (ret)
		return -EINVAL;

	if (val > CS40L2X_F0_OFFSET_POS_MAX && val < CS40L2X_F0_OFFSET_NEG_MIN)
		return -EINVAL;

	if (val > CS40L2X_F0_OFFSET_NEG_MAX)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "F0_OFFSET",
			CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_write(cs40l2x->regmap, reg, val);
	if (ret)
		goto err_mutex;

	ret = cs40l2x_dsp_cache(cs40l2x, reg, val);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_redc_measured_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;

	mutex_lock(&cs40l2x->lock);

	if (cs40l2x->diag_state < CS40L2X_DIAG_STATE_DONE1) {
		ret = -ENODATA;
		goto err_mutex;
	}

	ret = snprintf(buf, PAGE_SIZE, "%d\n", cs40l2x->redc_measured);

err_mutex:
	mutex_unlock(&cs40l2x->lock);

	return ret;
}

static ssize_t cs40l2x_redc_stored_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "REDC_STORED",
			CS40L2X_XM_UNPACKED_TYPE,
			cs40l2x->fw_desc->id == CS40L2X_FW_ID_ORIG ?
				CS40L2X_ALGO_ID_F0 : cs40l2x->fw_desc->id);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_read(cs40l2x->regmap, reg, &val);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%d\n", val);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_redc_stored_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	ret = kstrtou32(buf, 10, &val);
	if (ret)
		return -EINVAL;

	if (cs40l2x->pdata.redc_min > 0 && val < cs40l2x->pdata.redc_min)
		return -EINVAL;

	if (cs40l2x->pdata.redc_max > 0 && val > cs40l2x->pdata.redc_max)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "REDC_STORED",
			CS40L2X_XM_UNPACKED_TYPE,
			cs40l2x->fw_desc->id == CS40L2X_FW_ID_ORIG ?
				CS40L2X_ALGO_ID_F0 : cs40l2x->fw_desc->id);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_write(cs40l2x->regmap, reg, val);
	if (ret)
		goto err_mutex;

	ret = cs40l2x_dsp_cache(cs40l2x, reg, val);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_q_measured_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;

	mutex_lock(&cs40l2x->lock);

	if (cs40l2x->fw_desc->id == CS40L2X_FW_ID_ORIG) {
		ret = -EPERM;
		goto err_mutex;
	}

	if (cs40l2x->diag_state < CS40L2X_DIAG_STATE_DONE2) {
		ret = -ENODATA;
		goto err_mutex;
	}

	ret = snprintf(buf, PAGE_SIZE, "%d\n", cs40l2x->q_measured);

err_mutex:
	mutex_unlock(&cs40l2x->lock);

	return ret;
}

static ssize_t cs40l2x_q_stored_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "Q_STORED",
			CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_read(cs40l2x->regmap, reg, &val);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%d\n", val);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_q_stored_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	ret = kstrtou32(buf, 10, &val);
	if (ret)
		return -EINVAL;

	if (cs40l2x->pdata.q_min > 0 && val < cs40l2x->pdata.q_min)
		return -EINVAL;

	if (cs40l2x->pdata.q_max > 0 && val > cs40l2x->pdata.q_max)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "Q_STORED",
			CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_write(cs40l2x->regmap, reg, val);
	if (ret)
		goto err_mutex;

	ret = cs40l2x_dsp_cache(cs40l2x, reg, val);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_comp_enable_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;

	mutex_lock(&cs40l2x->lock);

	if (cs40l2x->fw_desc->id == CS40L2X_FW_ID_CAL) {
		ret = -EPERM;
		goto err_mutex;
	}

	if (cs40l2x->comp_enable_pend) {
		ret = -EIO;
		goto err_mutex;
	}

	ret = snprintf(buf, PAGE_SIZE, "%d\n", cs40l2x->comp_enable);

err_mutex:
	mutex_unlock(&cs40l2x->lock);

	return ret;
}

static ssize_t cs40l2x_comp_enable_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int val;

	ret = kstrtou32(buf, 10, &val);
	if (ret)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	cs40l2x->comp_enable_pend = true;
	cs40l2x->comp_enable = val > 0;

	switch (cs40l2x->fw_desc->id) {
	case CS40L2X_FW_ID_CAL:
		ret = -EPERM;
		break;
	case CS40L2X_FW_ID_ORIG:
		ret = regmap_write(cs40l2x->regmap,
				cs40l2x_dsp_reg(cs40l2x, "COMPENSATION_ENABLE",
						CS40L2X_XM_UNPACKED_TYPE,
						CS40L2X_ALGO_ID_VIBE),
				cs40l2x->comp_enable);
		break;
	default:
		ret = regmap_write(cs40l2x->regmap,
				cs40l2x_dsp_reg(cs40l2x, "COMPENSATION_ENABLE",
						CS40L2X_XM_UNPACKED_TYPE,
						CS40L2X_ALGO_ID_VIBE),
				(cs40l2x->comp_enable
					& cs40l2x->comp_enable_redc)
					<< CS40L2X_COMP_EN_REDC_SHIFT |
				(cs40l2x->comp_enable
					& cs40l2x->comp_enable_f0)
					<< CS40L2X_COMP_EN_F0_SHIFT);
	}

	if (ret)
		goto err_mutex;

	cs40l2x->comp_enable_pend = false;
	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_redc_comp_enable_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;

	mutex_lock(&cs40l2x->lock);

	if (cs40l2x->fw_desc->id == CS40L2X_FW_ID_CAL
			|| cs40l2x->fw_desc->id == CS40L2X_FW_ID_ORIG) {
		ret = -EPERM;
		goto err_mutex;
	}

	if (cs40l2x->comp_enable_pend) {
		ret = -EIO;
		goto err_mutex;
	}

	ret = snprintf(buf, PAGE_SIZE, "%d\n", cs40l2x->comp_enable_redc);

err_mutex:
	mutex_unlock(&cs40l2x->lock);

	return ret;
}

static ssize_t cs40l2x_redc_comp_enable_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int val;

	ret = kstrtou32(buf, 10, &val);
	if (ret)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	cs40l2x->comp_enable_pend = true;
	cs40l2x->comp_enable_redc = val > 0;

	switch (cs40l2x->fw_desc->id) {
	case CS40L2X_FW_ID_CAL:
	case CS40L2X_FW_ID_ORIG:
		ret = -EPERM;
		break;
	default:
		ret = regmap_write(cs40l2x->regmap,
				cs40l2x_dsp_reg(cs40l2x, "COMPENSATION_ENABLE",
						CS40L2X_XM_UNPACKED_TYPE,
						CS40L2X_ALGO_ID_VIBE),
				(cs40l2x->comp_enable
					& cs40l2x->comp_enable_redc)
					<< CS40L2X_COMP_EN_REDC_SHIFT |
				(cs40l2x->comp_enable
					& cs40l2x->comp_enable_f0)
					<< CS40L2X_COMP_EN_F0_SHIFT);
	}

	if (ret)
		goto err_mutex;

	cs40l2x->comp_enable_pend = false;
	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static int cs40l2x_dig_scale_get(struct cs40l2x_private *cs40l2x,
			unsigned int *dig_scale)
{
	int ret;
	unsigned int val;

	ret = regmap_read(cs40l2x->regmap, CS40L2X_AMP_DIG_VOL_CTRL, &val);
	if (ret)
		return ret;

	*dig_scale = (CS40L2X_DIG_SCALE_ZERO - ((val & CS40L2X_AMP_VOL_PCM_MASK)
			>> CS40L2X_AMP_VOL_PCM_SHIFT)) & CS40L2X_DIG_SCALE_MASK;

	return 0;
}

static int cs40l2x_dig_scale_set(struct cs40l2x_private *cs40l2x,
			unsigned int dig_scale)
{
	int ret;
	unsigned int val;

	if (dig_scale == CS40L2X_DIG_SCALE_RESET)
		return -EINVAL;

	ret = regmap_read(cs40l2x->regmap, CS40L2X_AMP_DIG_VOL_CTRL, &val);
	if (ret)
		return ret;

	val &= ~CS40L2X_AMP_VOL_PCM_MASK;
	val |= CS40L2X_AMP_VOL_PCM_MASK &
			(((CS40L2X_DIG_SCALE_ZERO - dig_scale)
			& CS40L2X_DIG_SCALE_MASK) << CS40L2X_AMP_VOL_PCM_SHIFT);

	ret = regmap_write(cs40l2x->regmap, CS40L2X_AMP_DIG_VOL_CTRL, val);
	if (ret)
		return ret;

	return cs40l2x_wseq_replace(cs40l2x, CS40L2X_AMP_DIG_VOL_CTRL, val);
}

static ssize_t cs40l2x_dig_scale_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int dig_scale;

	pm_runtime_get_sync(cs40l2x->dev);
	/*
	 * this operation is agnostic to the variable firmware ID and may
	 * therefore be performed without mutex protection
	 */
	ret = cs40l2x_dig_scale_get(cs40l2x, &dig_scale);

	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	if (ret)
		return ret;

	return snprintf(buf, PAGE_SIZE, "%d\n", dig_scale);
}

static ssize_t cs40l2x_dig_scale_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int dig_scale;

	ret = kstrtou32(buf, 10, &dig_scale);
	if (ret)
		return -EINVAL;

	if (dig_scale > CS40L2X_DIG_SCALE_MAX)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);
	/*
	 * this operation calls cs40l2x_wseq_replace which checks the variable
	 * firmware ID and must therefore be performed within mutex protection
	 */
	ret = cs40l2x_dig_scale_set(cs40l2x, dig_scale);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static int cs40l2x_gpio1_dig_scale_get(struct cs40l2x_private *cs40l2x,
			unsigned int *dig_scale)
{
	unsigned int val;
	unsigned int reg = cs40l2x_dsp_reg(cs40l2x, "GAIN_CONTROL",
			CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
	int ret;

	if (!reg)
		return -EPERM;

	ret = regmap_read(cs40l2x->regmap, reg, &val);
	if (ret)
		return ret;

	*dig_scale = (val & CS40L2X_GAIN_CTRL_GPIO_MASK)
			>> CS40L2X_GAIN_CTRL_GPIO_SHIFT;

	return 0;
}

static int cs40l2x_gpio1_dig_scale_set(struct cs40l2x_private *cs40l2x,
			unsigned int dig_scale)
{
	unsigned int val;
	unsigned int reg = cs40l2x_dsp_reg(cs40l2x, "GAIN_CONTROL",
			CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
	int ret;

	if (!reg)
		return -EPERM;

	if (dig_scale == CS40L2X_DIG_SCALE_RESET)
		return -EINVAL;

	ret = regmap_read(cs40l2x->regmap, reg, &val);
	if (ret)
		return ret;

	val &= ~CS40L2X_GAIN_CTRL_GPIO_MASK;
	val |= CS40L2X_GAIN_CTRL_GPIO_MASK &
			(dig_scale << CS40L2X_GAIN_CTRL_GPIO_SHIFT);

	ret = regmap_write(cs40l2x->regmap, reg, val);
	if (ret)
		return ret;

	return cs40l2x_dsp_cache(cs40l2x, reg, val);
}

static ssize_t cs40l2x_gpio1_dig_scale_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int dig_scale = 0;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_gpio1_dig_scale_get(cs40l2x, &dig_scale);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%d\n", dig_scale);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio1_dig_scale_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int dig_scale;

	ret = kstrtou32(buf, 10, &dig_scale);
	if (ret)
		return -EINVAL;

	if (dig_scale > CS40L2X_DIG_SCALE_MAX)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_gpio1_dig_scale_set(cs40l2x, dig_scale);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static int cs40l2x_gpio_edge_dig_scale_get(struct cs40l2x_private *cs40l2x,
			unsigned int *dig_scale,
			unsigned int gpio_offs, bool gpio_rise)
{
	bool gpio_pol = cs40l2x->pdata.gpio_indv_pol & (1 << (gpio_offs >> 2));
	unsigned int val;
	unsigned int reg = cs40l2x_dsp_reg(cs40l2x, "GPIO_GAIN",
			CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
	int ret;

	if (!reg)
		return -EPERM;
	reg += gpio_offs;

	if (!(cs40l2x->gpio_mask & (1 << (gpio_offs >> 2))))
		return -EPERM;

	ret = regmap_read(cs40l2x->regmap, reg, &val);
	if (ret)
		return ret;

	*dig_scale = (val & (gpio_pol ^ gpio_rise ?
			CS40L2X_GPIO_GAIN_RISE_MASK :
			CS40L2X_GPIO_GAIN_FALL_MASK)) >>
				(gpio_pol ^ gpio_rise ?
					CS40L2X_GPIO_GAIN_RISE_SHIFT :
					CS40L2X_GPIO_GAIN_FALL_SHIFT);

	return 0;
}

static int cs40l2x_gpio_edge_dig_scale_set(struct cs40l2x_private *cs40l2x,
			unsigned int dig_scale,
			unsigned int gpio_offs, bool gpio_rise)
{
	bool gpio_pol = cs40l2x->pdata.gpio_indv_pol & (1 << (gpio_offs >> 2));
	unsigned int val;
	unsigned int reg = cs40l2x_dsp_reg(cs40l2x, "GPIO_GAIN",
			CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
	int ret;

	if (!reg)
		return -EPERM;
	reg += gpio_offs;

	if (!(cs40l2x->gpio_mask & (1 << (gpio_offs >> 2))))
		return -EPERM;

	if (dig_scale == CS40L2X_DIG_SCALE_RESET
			|| dig_scale > CS40L2X_DIG_SCALE_MAX)
		return -EINVAL;

	ret = regmap_read(cs40l2x->regmap, reg, &val);
	if (ret)
		return ret;

	val &= ~(gpio_pol ^ gpio_rise ?
			CS40L2X_GPIO_GAIN_RISE_MASK :
			CS40L2X_GPIO_GAIN_FALL_MASK);

	val |= (gpio_pol ^ gpio_rise ?
			CS40L2X_GPIO_GAIN_RISE_MASK :
			CS40L2X_GPIO_GAIN_FALL_MASK) &
				(dig_scale << (gpio_pol ^ gpio_rise ?
					CS40L2X_GPIO_GAIN_RISE_SHIFT :
					CS40L2X_GPIO_GAIN_FALL_SHIFT));

	ret = regmap_write(cs40l2x->regmap, reg, val);
	if (ret)
		return ret;

	return cs40l2x_dsp_cache(cs40l2x, reg, val);
}

static ssize_t cs40l2x_gpio1_rise_dig_scale_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int dig_scale;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_gpio_edge_dig_scale_get(cs40l2x, &dig_scale,
			CS40L2X_INDEXBUTTONPRESS1, CS40L2X_GPIO_RISE);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%u\n", dig_scale);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio1_rise_dig_scale_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int dig_scale;

	ret = kstrtou32(buf, 10, &dig_scale);
	if (ret)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_gpio_edge_dig_scale_set(cs40l2x, dig_scale,
			CS40L2X_INDEXBUTTONPRESS1, CS40L2X_GPIO_RISE);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio1_fall_dig_scale_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int dig_scale;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_gpio_edge_dig_scale_get(cs40l2x, &dig_scale,
			CS40L2X_INDEXBUTTONRELEASE1, CS40L2X_GPIO_FALL);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%u\n", dig_scale);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio1_fall_dig_scale_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int dig_scale;

	ret = kstrtou32(buf, 10, &dig_scale);
	if (ret)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_gpio_edge_dig_scale_set(cs40l2x, dig_scale,
			CS40L2X_INDEXBUTTONRELEASE1, CS40L2X_GPIO_FALL);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio2_rise_dig_scale_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int dig_scale;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_gpio_edge_dig_scale_get(cs40l2x, &dig_scale,
			CS40L2X_INDEXBUTTONPRESS2, CS40L2X_GPIO_RISE);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%u\n", dig_scale);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio2_rise_dig_scale_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int dig_scale;

	ret = kstrtou32(buf, 10, &dig_scale);
	if (ret)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_gpio_edge_dig_scale_set(cs40l2x, dig_scale,
			CS40L2X_INDEXBUTTONPRESS2, CS40L2X_GPIO_RISE);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio2_fall_dig_scale_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int dig_scale;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_gpio_edge_dig_scale_get(cs40l2x, &dig_scale,
			CS40L2X_INDEXBUTTONRELEASE2, CS40L2X_GPIO_FALL);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%u\n", dig_scale);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio2_fall_dig_scale_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int dig_scale;

	ret = kstrtou32(buf, 10, &dig_scale);
	if (ret)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_gpio_edge_dig_scale_set(cs40l2x, dig_scale,
			CS40L2X_INDEXBUTTONRELEASE2, CS40L2X_GPIO_FALL);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio3_rise_dig_scale_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int dig_scale;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_gpio_edge_dig_scale_get(cs40l2x, &dig_scale,
			CS40L2X_INDEXBUTTONPRESS3, CS40L2X_GPIO_RISE);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%u\n", dig_scale);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio3_rise_dig_scale_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int dig_scale;

	ret = kstrtou32(buf, 10, &dig_scale);
	if (ret)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_gpio_edge_dig_scale_set(cs40l2x, dig_scale,
			CS40L2X_INDEXBUTTONPRESS3, CS40L2X_GPIO_RISE);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio3_fall_dig_scale_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int dig_scale;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_gpio_edge_dig_scale_get(cs40l2x, &dig_scale,
			CS40L2X_INDEXBUTTONRELEASE3, CS40L2X_GPIO_FALL);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%u\n", dig_scale);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio3_fall_dig_scale_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int dig_scale;

	ret = kstrtou32(buf, 10, &dig_scale);
	if (ret)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_gpio_edge_dig_scale_set(cs40l2x, dig_scale,
			CS40L2X_INDEXBUTTONRELEASE3, CS40L2X_GPIO_FALL);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio4_rise_dig_scale_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int dig_scale;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_gpio_edge_dig_scale_get(cs40l2x, &dig_scale,
			CS40L2X_INDEXBUTTONPRESS4, CS40L2X_GPIO_RISE);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%u\n", dig_scale);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio4_rise_dig_scale_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int dig_scale;

	ret = kstrtou32(buf, 10, &dig_scale);
	if (ret)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_gpio_edge_dig_scale_set(cs40l2x, dig_scale,
			CS40L2X_INDEXBUTTONPRESS4, CS40L2X_GPIO_RISE);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio4_fall_dig_scale_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int dig_scale;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_gpio_edge_dig_scale_get(cs40l2x, &dig_scale,
			CS40L2X_INDEXBUTTONRELEASE4, CS40L2X_GPIO_FALL);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%u\n", dig_scale);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio4_fall_dig_scale_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int dig_scale;

	ret = kstrtou32(buf, 10, &dig_scale);
	if (ret)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_gpio_edge_dig_scale_set(cs40l2x, dig_scale,
			CS40L2X_INDEXBUTTONRELEASE4, CS40L2X_GPIO_FALL);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static int cs40l2x_cp_dig_scale_get(struct cs40l2x_private *cs40l2x,
			unsigned int *dig_scale)
{
	unsigned int val;
	unsigned int reg = cs40l2x_dsp_reg(cs40l2x, "GAIN_CONTROL",
			CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
	int ret;

	if (!reg)
		return -EPERM;

	ret = regmap_read(cs40l2x->regmap, reg, &val);
	if (ret)
		return ret;

	*dig_scale = (val & CS40L2X_GAIN_CTRL_TRIG_MASK)
			>> CS40L2X_GAIN_CTRL_TRIG_SHIFT;

	return 0;
}

static int cs40l2x_cp_dig_scale_set(struct cs40l2x_private *cs40l2x,
			unsigned int dig_scale)
{
	unsigned int val;
	unsigned int reg = cs40l2x_dsp_reg(cs40l2x, "GAIN_CONTROL",
			CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
	int ret;

	if (!reg)
		return -EPERM;

	if (dig_scale == CS40L2X_DIG_SCALE_RESET)
		return -EINVAL;

	ret = regmap_read(cs40l2x->regmap, reg, &val);
	if (ret)
		return ret;

	val &= ~CS40L2X_GAIN_CTRL_TRIG_MASK;
	val |= CS40L2X_GAIN_CTRL_TRIG_MASK &
			(dig_scale << CS40L2X_GAIN_CTRL_TRIG_SHIFT);

	ret = regmap_write(cs40l2x->regmap, reg, val);
	if (ret)
		return ret;

	return cs40l2x_dsp_cache(cs40l2x, reg, val);
}

static ssize_t cs40l2x_cp_dig_scale_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int dig_scale;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_cp_dig_scale_get(cs40l2x, &dig_scale);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%d\n", dig_scale);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_cp_dig_scale_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int dig_scale;

	ret = kstrtou32(buf, 10, &dig_scale);
	if (ret)
		return -EINVAL;

	if (dig_scale > CS40L2X_DIG_SCALE_MAX)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_cp_dig_scale_set(cs40l2x, dig_scale);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_heartbeat_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int val;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = regmap_read(cs40l2x->regmap,
			cs40l2x_dsp_reg(cs40l2x, "HALO_HEARTBEAT",
					CS40L2X_XM_UNPACKED_TYPE,
					cs40l2x->fw_desc->id),
			&val);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%d\n", val);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_num_waves_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	unsigned int num_waves;

	mutex_lock(&cs40l2x->lock);
	num_waves = cs40l2x->num_waves;
	if (cs40l2x->virtual_bin)
		num_waves = (cs40l2x->num_waves - CS40L2X_WT_NUM_VIRT_SLOTS) +
			cs40l2x->num_virtual_waves;
	/* The minus is for the virtual slots */
	mutex_unlock(&cs40l2x->lock);

	return snprintf(buf, PAGE_SIZE, "%d\n", num_waves);
}

static ssize_t cs40l2x_num_virtual_waves_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	unsigned int num_virtual_waves;

	mutex_lock(&cs40l2x->lock);
	num_virtual_waves = cs40l2x->num_virtual_waves;
	mutex_unlock(&cs40l2x->lock);

	return snprintf(buf, PAGE_SIZE, "%d\n", num_virtual_waves);
}

static ssize_t cs40l2x_num_virtual_composite_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	unsigned int num_virtual_comp_waves;

	mutex_lock(&cs40l2x->lock);
	num_virtual_comp_waves = (cs40l2x->num_virtual_waves -
		cs40l2x->num_virtual_pwle_waves);
	mutex_unlock(&cs40l2x->lock);

	return snprintf(buf, PAGE_SIZE, "%d\n", num_virtual_comp_waves);
}

static ssize_t cs40l2x_num_virtual_pwle_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	unsigned int num_virtual_pwle_waves;

	mutex_lock(&cs40l2x->lock);
	num_virtual_pwle_waves = cs40l2x->num_virtual_pwle_waves;
	mutex_unlock(&cs40l2x->lock);

	return snprintf(buf, PAGE_SIZE, "%d\n", num_virtual_pwle_waves);
}

static ssize_t cs40l2x_fw_rev_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	unsigned int fw_rev;

	mutex_lock(&cs40l2x->lock);
	fw_rev = cs40l2x->algo_info[0].rev;
	mutex_unlock(&cs40l2x->lock);

	return snprintf(buf, PAGE_SIZE, "%u\n", fw_rev);
}

static ssize_t cs40l2x_vpp_measured_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);

	if (cs40l2x->vpp_measured < 0)
		return -ENODATA;

	return snprintf(buf, PAGE_SIZE, "%d\n", cs40l2x->vpp_measured);
}

static ssize_t cs40l2x_ipp_measured_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);

	if (cs40l2x->ipp_measured < 0)
		return -ENODATA;

	return snprintf(buf, PAGE_SIZE, "%d\n", cs40l2x->ipp_measured);
}

static ssize_t cs40l2x_vbatt_max_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "VPMONMAX",
			CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_read(cs40l2x->regmap, reg, &val);
	if (ret)
		goto err_mutex;

	if (val == CS40L2X_VPMONMAX_RESET) {
		ret = -ENODATA;
		goto err_mutex;
	}

	ret = snprintf(buf, PAGE_SIZE, "%u\n", val);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_vbatt_max_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	ret = kstrtou32(buf, 10, &val);
	if (ret)
		return -EINVAL;

	if (val)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "VPMONMAX",
			CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_write(cs40l2x->regmap, reg, CS40L2X_VPMONMAX_RESET);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_vbatt_min_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "VPMONMIN",
			CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_read(cs40l2x->regmap, reg, &val);
	if (ret)
		goto err_mutex;

	if (val == CS40L2X_VPMONMIN_RESET) {
		ret = -ENODATA;
		goto err_mutex;
	}

	ret = snprintf(buf, PAGE_SIZE, "%u\n", val);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_vbatt_min_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	ret = kstrtou32(buf, 10, &val);
	if (ret)
		return -EINVAL;

	if (val)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "VPMONMIN",
			CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_write(cs40l2x->regmap, reg, CS40L2X_VPMONMIN_RESET);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static int cs40l2x_refclk_switch(struct cs40l2x_private *cs40l2x,
			unsigned int refclk_freq)
{
	unsigned int refclk_sel, pll_config;
	int ret, i;

	refclk_sel = (refclk_freq == CS40L2X_PLL_REFCLK_FREQ_32K) ?
			CS40L2X_PLL_REFCLK_SEL_MCLK :
			CS40L2X_PLL_REFCLK_SEL_BCLK;

	for (i = 0; i < CS40L2X_NUM_REFCLKS; i++)
		if (cs40l2x_refclks[i].freq == refclk_freq)
			break;
	if (i == CS40L2X_NUM_REFCLKS)
		return -EINVAL;

	pll_config = ((1 << CS40L2X_PLL_REFCLK_EN_SHIFT)
				& CS40L2X_PLL_REFCLK_EN_MASK) |
			((i << CS40L2X_PLL_REFCLK_FREQ_SHIFT)
				& CS40L2X_PLL_REFCLK_FREQ_MASK) |
			((refclk_sel << CS40L2X_PLL_REFCLK_SEL_SHIFT)
				& CS40L2X_PLL_REFCLK_SEL_MASK);

	ret = cs40l2x_ack_write(cs40l2x,
			CS40L2X_MBOX_POWERCONTROL,
			CS40L2X_POWERCONTROL_FRC_STDBY,
			CS40L2X_POWERCONTROL_NONE);
	if (ret)
		return ret;

	ret = regmap_write(cs40l2x->regmap, CS40L2X_PLL_CLK_CTRL, pll_config);
	if (ret)
		return ret;

	ret = cs40l2x_wseq_replace(cs40l2x, CS40L2X_PLL_CLK_CTRL, pll_config);
	if (ret)
		return ret;

	return cs40l2x_ack_write(cs40l2x,
			CS40L2X_MBOX_POWERCONTROL,
			CS40L2X_POWERCONTROL_WAKEUP,
			CS40L2X_POWERCONTROL_NONE);
}

static int cs40l2x_asp_switch(struct cs40l2x_private *cs40l2x, bool enable)
{
	unsigned int val;
	int ret;

	if (!enable) {
		ret = cs40l2x_user_ctrl_exec(cs40l2x,
				CS40L2X_USER_CTRL_STOP, 0, NULL);
		if (ret)
			return ret;

		if (cs40l2x->amp_gnd_stby) {
			ret = regmap_write(cs40l2x->regmap,
					CS40L2X_SPK_FORCE_TST_1,
					CS40L2X_FORCE_SPK_GND);
			if (ret)
				return ret;
		}
	}

	ret = regmap_read(cs40l2x->regmap, CS40L2X_SP_ENABLES, &val);
	if (ret)
		return ret;

	val &= ~CS40L2X_ASP_RX1_EN_MASK;
	val |= (enable << CS40L2X_ASP_RX1_EN_SHIFT) & CS40L2X_ASP_RX1_EN_MASK;

	ret = regmap_write(cs40l2x->regmap, CS40L2X_SP_ENABLES, val);
	if (ret)
		return ret;

	ret = cs40l2x_wseq_replace(cs40l2x, CS40L2X_SP_ENABLES, val);
	if (ret)
		return ret;

	if (enable) {
		if (cs40l2x->amp_gnd_stby) {
			ret = regmap_write(cs40l2x->regmap,
					CS40L2X_SPK_FORCE_TST_1,
					CS40L2X_FORCE_SPK_FREE);
			if (ret)
				return ret;
		}

		ret = cs40l2x_user_ctrl_exec(cs40l2x,
				CS40L2X_USER_CTRL_PLAY, 0, NULL);
		if (ret)
			return ret;
	}

	return 0;
}

static ssize_t cs40l2x_asp_enable_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);

	return snprintf(buf, PAGE_SIZE, "%d\n", cs40l2x->asp_enable);
}

static ssize_t cs40l2x_asp_enable_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int val, fw_id;

	if (!cs40l2x->asp_available)
		return -EPERM;

	mutex_lock(&cs40l2x->lock);
	fw_id = cs40l2x->fw_desc->id;
	mutex_unlock(&cs40l2x->lock);

	if (fw_id == CS40L2X_FW_ID_CAL || fw_id == CS40L2X_FW_ID_ORIG)
		return -EPERM;

	ret = kstrtou32(buf, 10, &val);
	if (ret)
		return -EINVAL;

	if (val > 0)
		cs40l2x->asp_enable = CS40L2X_ASP_ENABLED;
	else
		cs40l2x->asp_enable = CS40L2X_ASP_DISABLED;

	queue_work(cs40l2x->vibe_workqueue, &cs40l2x->vibe_mode_work);

	return count;
}

static ssize_t cs40l2x_exc_enable_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "EX_PROTECT_ENABLED",
			CS40L2X_XM_UNPACKED_TYPE, CS40L2X_ALGO_ID_EXC);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_read(cs40l2x->regmap, reg, &val);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%d\n", val);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_exc_enable_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	ret = kstrtou32(buf, 10, &val);
	if (ret)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "EX_PROTECT_ENABLED",
			CS40L2X_XM_UNPACKED_TYPE, CS40L2X_ALGO_ID_EXC);
	if (!reg || !cs40l2x->exc_available) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_write(cs40l2x->regmap,
			reg, val ? CS40L2X_EXC_ENABLED : CS40L2X_EXC_DISABLED);
	if (ret)
		goto err_mutex;

	ret = cs40l2x_dsp_cache(cs40l2x,
			reg, val ? CS40L2X_EXC_ENABLED : CS40L2X_EXC_DISABLED);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_hw_err_count_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	ssize_t len = 0;
	int ret, i;

	mutex_lock(&cs40l2x->lock);

	if (cs40l2x->fw_desc->id == CS40L2X_FW_ID_CAL
			|| cs40l2x->fw_desc->id == CS40L2X_FW_ID_ORIG) {
		ret = -EPERM;
		goto err_mutex;
	}

	for (i = 0; i < CS40L2X_NUM_HW_ERRS; i++)
		len += snprintf(buf + len, PAGE_SIZE - len, "%u %s error(s)\n",
				cs40l2x->hw_err_count[i],
				cs40l2x_hw_errs[i].err_name);

	ret = len;

err_mutex:
	mutex_unlock(&cs40l2x->lock);

	return ret;
}

static ssize_t cs40l2x_hw_err_count_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret, i;
	unsigned int val;

	ret = kstrtou32(buf, 10, &val);
	if (ret)
		return -EINVAL;

	if (val)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	if (cs40l2x->fw_desc->id == CS40L2X_FW_ID_CAL
			|| cs40l2x->fw_desc->id == CS40L2X_FW_ID_ORIG) {
		ret = -EPERM;
		goto err_mutex;
	}

	for (i = 0; i < CS40L2X_NUM_HW_ERRS; i++) {
		if (cs40l2x->hw_err_count[i] > CS40L2X_HW_ERR_COUNT_MAX) {
			ret = cs40l2x_hw_err_rls(cs40l2x,
					cs40l2x_hw_errs[i].irq_mask);
			if (ret)
				goto err_mutex;
		}

		cs40l2x->hw_err_count[i] = 0;
	}

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_hw_reset_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);

	return snprintf(buf, PAGE_SIZE, "%d\n",
			gpiod_get_value_cansleep(cs40l2x->reset_gpio));
}

static ssize_t cs40l2x_hw_reset_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	struct i2c_client *i2c_client = to_i2c_client(cs40l2x->dev);
	int ret, state;
	unsigned int val, fw_id_restore;

	ret = kstrtou32(buf, 10, &val);
	if (ret)
		return -EINVAL;

	if (cs40l2x->revid < CS40L2X_REVID_B1)
		return -EPERM;

	state = gpiod_get_value_cansleep(cs40l2x->reset_gpio);
	if (state < 0)
		return state;

	/*
	 * resetting the device prompts it to briefly assert the /ALERT pin,
	 * so disable the interrupt line until the device has been restored
	 */
	disable_irq(i2c_client->irq);

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	if (cs40l2x->vibe_mode == CS40L2X_VIBE_MODE_AUDIO
			|| cs40l2x->vibe_state == CS40L2X_VIBE_STATE_RUNNING) {
		ret = -EPERM;
		goto err_mutex;
	}

	if (val && !state) {
		gpiod_set_value_cansleep(cs40l2x->reset_gpio, 1);
		usleep_range(1000, 1100);

		fw_id_restore = cs40l2x->fw_desc->id;
		cs40l2x->fw_desc = cs40l2x_firmware_match(cs40l2x,
				CS40L2X_FW_ID_B1ROM);

		ret = cs40l2x_firmware_swap(cs40l2x, fw_id_restore);
		if (ret)
			goto err_mutex;

		cs40l2x->dsp_cache_depth = 0;
	} else if (!val && state) {
		gpiod_set_value_cansleep(cs40l2x->reset_gpio, 0);
		usleep_range(2000, 2100);
	}

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	enable_irq(i2c_client->irq);

	return ret;
}

static ssize_t cs40l2x_wt_file_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;

	mutex_lock(&cs40l2x->lock);

	if (!strncmp(cs40l2x->wt_file,
			CS40L2X_WT_FILE_NAME_MISSING,
			CS40L2X_WT_FILE_NAME_LEN_MAX)) {
		ret = -ENODATA;
		goto err_mutex;
	}

	ret = snprintf(buf, PAGE_SIZE, "%s\n", cs40l2x->wt_file);

err_mutex:
	mutex_unlock(&cs40l2x->lock);

	return ret;
}

static ssize_t cs40l2x_wt_file_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	char wt_file[CS40L2X_WT_FILE_NAME_LEN_MAX];
	size_t len = count;
	int ret;

	if (!len)
		return -EINVAL;

	if (buf[len - 1] == '\n')
		len--;

	if (len + 1 > CS40L2X_WT_FILE_NAME_LEN_MAX)
		return -ENAMETOOLONG;

	memcpy(wt_file, buf, len);
	wt_file[len] = '\0';

	if (!strncmp(wt_file,
			CS40L2X_WT_FILE_NAME_MISSING,
			CS40L2X_WT_FILE_NAME_LEN_MAX))
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	if (cs40l2x->fw_desc->id == CS40L2X_FW_ID_CAL
			|| cs40l2x->fw_desc->id == CS40L2X_FW_ID_ORIG) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = cs40l2x_wavetable_swap(cs40l2x, wt_file);
	if (ret)
		goto err_mutex;

	ret = cs40l2x_wavetable_sync(cs40l2x);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_wt_date_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;

	mutex_lock(&cs40l2x->lock);

	if (!strncmp(cs40l2x->wt_date,
			CS40L2X_WT_FILE_DATE_MISSING,
			CS40L2X_WT_FILE_DATE_LEN_MAX)) {
		ret = -ENODATA;
		goto err_mutex;
	}

	ret = snprintf(buf, PAGE_SIZE, "%s\n", cs40l2x->wt_date);

err_mutex:
	mutex_unlock(&cs40l2x->lock);

	return ret;
}

static int cs40l2x_imon_offs_sync(struct cs40l2x_private *cs40l2x)
{
	struct regmap *regmap = cs40l2x->regmap;
	unsigned int reg_calc_enable = cs40l2x_dsp_reg(cs40l2x,
			"VMON_IMON_OFFSET_ENABLE",
			CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
	unsigned int val_calc_enable = CS40L2X_IMON_OFFS_CALC_DIS;
	unsigned int reg, val;
	int ret;

	if (!reg_calc_enable)
		return 0;

	reg = cs40l2x_dsp_reg(cs40l2x, "CLAB_ENABLED",
			CS40L2X_XM_UNPACKED_TYPE, CS40L2X_ALGO_ID_CLAB);
	if (reg) {
		ret = regmap_read(regmap, reg, &val);
		if (ret)
			return ret;

		if (val == CS40L2X_CLAB_ENABLED)
			val_calc_enable = CS40L2X_IMON_OFFS_CALC_EN;
	}

	return regmap_write(regmap, reg_calc_enable, val_calc_enable);
}

static ssize_t cs40l2x_clab_enable_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "CLAB_ENABLED",
			CS40L2X_XM_UNPACKED_TYPE, CS40L2X_ALGO_ID_CLAB);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_read(cs40l2x->regmap, reg, &val);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%u\n", val);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_clab_enable_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	ret = kstrtou32(buf, 10, &val);
	if (ret)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "CLAB_ENABLED",
			CS40L2X_XM_UNPACKED_TYPE, CS40L2X_ALGO_ID_CLAB);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_write(cs40l2x->regmap, reg,
			val ? CS40L2X_CLAB_ENABLED : CS40L2X_CLAB_DISABLED);
	if (ret)
		goto err_mutex;

	ret = cs40l2x_dsp_cache(cs40l2x, reg,
			val ? CS40L2X_CLAB_ENABLED : CS40L2X_CLAB_DISABLED);
	if (ret)
		goto err_mutex;

	ret = cs40l2x_imon_offs_sync(cs40l2x);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_clab_peak_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "PEAK_AMPLITUDE_CONTROL",
			CS40L2X_XM_UNPACKED_TYPE, CS40L2X_ALGO_ID_CLAB);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_read(cs40l2x->regmap, reg, &val);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%u\n", val);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_clab_peak_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	ret = kstrtou32(buf, 10, &val);
	if (ret)
		return -EINVAL;

	if (val > CS40L2X_CLAB_PEAK_MAX)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "PEAK_AMPLITUDE_CONTROL",
			CS40L2X_XM_UNPACKED_TYPE, CS40L2X_ALGO_ID_CLAB);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_write(cs40l2x->regmap, reg, val);
	if (ret)
		goto err_mutex;

	ret = cs40l2x_dsp_cache(cs40l2x, reg, val);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_par_enable_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "PWLE_REGULATION_ENABLED",
			CS40L2X_XM_UNPACKED_TYPE, CS40L2X_ALGO_ID_PAR);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_read(cs40l2x->regmap, reg, &val);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%u\n", val);

err_mutex:
	mutex_unlock(&cs40l2x->lock);

	return ret;
}

static ssize_t cs40l2x_par_enable_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	ret = kstrtou32(buf, 10, &val);
	if (ret)
		return -EINVAL;

	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "PWLE_REGULATION_ENABLED",
			CS40L2X_XM_UNPACKED_TYPE, CS40L2X_ALGO_ID_PAR);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_write(cs40l2x->regmap, reg,
			val ? CS40L2X_PAR_ENABLED : CS40L2X_PAR_DISABLED);
	if (ret)
		goto err_mutex;

	ret = cs40l2x_dsp_cache(cs40l2x, reg,
			val ? CS40L2X_PAR_ENABLED : CS40L2X_PAR_DISABLED);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);

	return ret;
}

static ssize_t cs40l2x_par_gain_comp_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "PCM_GAIN_COMPENSATION_ENABLED",
			CS40L2X_XM_UNPACKED_TYPE, CS40L2X_ALGO_ID_PAR);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_read(cs40l2x->regmap, reg, &val);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%u\n", val);

err_mutex:
	mutex_unlock(&cs40l2x->lock);

	return ret;
}

static ssize_t cs40l2x_par_gain_comp_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	ret = kstrtou32(buf, 10, &val);
	if (ret)
		return -EINVAL;

	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "PCM_GAIN_COMPENSATION_ENABLED",
			CS40L2X_XM_UNPACKED_TYPE, CS40L2X_ALGO_ID_PAR);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_write(cs40l2x->regmap, reg,
			val ? CS40L2X_GC_ENABLED : CS40L2X_GC_DISABLED);
	if (ret)
		goto err_mutex;

	ret = cs40l2x_dsp_cache(cs40l2x, reg,
			val ? CS40L2X_GC_ENABLED : CS40L2X_GC_DISABLED);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);

	return ret;
}

static ssize_t cs40l2x_vibe_state_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);

	flush_workqueue(cs40l2x->vibe_workqueue);

	return snprintf(buf, PAGE_SIZE, "%u\n", cs40l2x->vibe_state);
}

static ssize_t cs40l2x_safe_save_state_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);

	return snprintf(buf, PAGE_SIZE, "%u\n", cs40l2x->safe_save_state);
}

static ssize_t cs40l2x_max_back_emf_show(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "MAXBACKEMF",
			      CS40L2X_XM_UNPACKED_TYPE, CS40L2X_ALGO_ID_F0);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_read(cs40l2x->regmap, reg, &val);
	if (ret)
		goto err_mutex;

	ret = snprintf(buf, PAGE_SIZE, "%u\n", val);

err_mutex:
	mutex_unlock(&cs40l2x->lock);

	return ret;
}

static ssize_t cs40l2x_max_back_emf_store(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf,
					  size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	ret = kstrtou32(buf, 10, &val);
	if (ret)
		return -EINVAL;

	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "MAXBACKEMF",
			      CS40L2X_XM_UNPACKED_TYPE, CS40L2X_ALGO_ID_F0);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_write(cs40l2x->regmap, reg, val);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);

	return ret;
}

static ssize_t cs40l2x_autosuspend_delay_show(struct device *dev,
					      struct device_attribute *attr,
					      char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	unsigned int val;

	mutex_lock(&cs40l2x->lock);
	val = cs40l2x->autosuspend_delay;
	mutex_unlock(&cs40l2x->lock);

	return snprintf(buf, PAGE_SIZE, "%u\n", val);
}

static ssize_t cs40l2x_autosuspend_delay_store(struct device *dev,
					       struct device_attribute *attr,
					       const char *buf,
					       size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int val;

	ret = kstrtou32(buf, 10, &val);
	if (ret)
		return -EINVAL;

	mutex_lock(&cs40l2x->lock);
	cs40l2x->autosuspend_delay = val;
	mutex_unlock(&cs40l2x->lock);

	pm_runtime_set_autosuspend_delay(cs40l2x->dev, val);

	return count;
}

static DEVICE_ATTR(cp_trigger_index, 0660, cs40l2x_cp_trigger_index_show,
		cs40l2x_cp_trigger_index_store);
static DEVICE_ATTR(cp_trigger_queue, 0660, cs40l2x_cp_trigger_queue_show,
		cs40l2x_cp_trigger_queue_store);
static DEVICE_ATTR(cp_trigger_duration, 0660, cs40l2x_cp_trigger_duration_show,
		NULL);
static DEVICE_ATTR(cp_trigger_q_sub, 0660, cs40l2x_cp_trigger_q_sub_show,
		NULL);
static DEVICE_ATTR(hiber_cmd, 0660, NULL, cs40l2x_hiber_cmd_store);
static DEVICE_ATTR(hiber_timeout, 0660, cs40l2x_hiber_timeout_show,
		cs40l2x_hiber_timeout_store);
static DEVICE_ATTR(gpio1_enable, 0660, cs40l2x_gpio1_enable_show,
		cs40l2x_gpio1_enable_store);
static DEVICE_ATTR(gpio1_rise_index, 0660, cs40l2x_gpio1_rise_index_show,
		cs40l2x_gpio1_rise_index_store);
static DEVICE_ATTR(gpio1_fall_index, 0660, cs40l2x_gpio1_fall_index_show,
		cs40l2x_gpio1_fall_index_store);
static DEVICE_ATTR(gpio1_fall_timeout, 0660, cs40l2x_gpio1_fall_timeout_show,
		cs40l2x_gpio1_fall_timeout_store);
static DEVICE_ATTR(gpio2_rise_index, 0660, cs40l2x_gpio2_rise_index_show,
		cs40l2x_gpio2_rise_index_store);
static DEVICE_ATTR(gpio2_fall_index, 0660, cs40l2x_gpio2_fall_index_show,
		cs40l2x_gpio2_fall_index_store);
static DEVICE_ATTR(gpio3_rise_index, 0660, cs40l2x_gpio3_rise_index_show,
		cs40l2x_gpio3_rise_index_store);
static DEVICE_ATTR(gpio3_fall_index, 0660, cs40l2x_gpio3_fall_index_show,
		cs40l2x_gpio3_fall_index_store);
static DEVICE_ATTR(gpio4_rise_index, 0660, cs40l2x_gpio4_rise_index_show,
		cs40l2x_gpio4_rise_index_store);
static DEVICE_ATTR(gpio4_fall_index, 0660, cs40l2x_gpio4_fall_index_show,
		cs40l2x_gpio4_fall_index_store);
static DEVICE_ATTR(standby_timeout, 0660, cs40l2x_standby_timeout_show,
		cs40l2x_standby_timeout_store);
static DEVICE_ATTR(f0_measured, 0660, cs40l2x_f0_measured_show, NULL);
static DEVICE_ATTR(f0_stored, 0660, cs40l2x_f0_stored_show,
		cs40l2x_f0_stored_store);
static DEVICE_ATTR(die_temp, 0660, cs40l2x_die_temp_show, NULL);
static DEVICE_ATTR(vbst_avg, 0660, cs40l2x_vbst_avg_show, NULL);
static DEVICE_ATTR(bemf_measured, 0660, cs40l2x_bemf_measured_show, NULL);
static DEVICE_ATTR(dynamic_f0, 0660, cs40l2x_dyn_f0_show, NULL);
static DEVICE_ATTR(dynamic_f0_index, 0660, cs40l2x_dyn_f0_index_show,
		cs40l2x_dyn_f0_index_store);
static DEVICE_ATTR(dynamic_f0_val, 0660, cs40l2x_dyn_f0_val_show,
		cs40l2x_dyn_f0_val_store);
static DEVICE_ATTR(f0_offset, 0660, cs40l2x_f0_offset_show,
		cs40l2x_f0_offset_store);
static DEVICE_ATTR(redc_measured, 0660, cs40l2x_redc_measured_show, NULL);
static DEVICE_ATTR(redc_stored, 0660, cs40l2x_redc_stored_show,
		cs40l2x_redc_stored_store);
static DEVICE_ATTR(q_measured, 0660, cs40l2x_q_measured_show, NULL);
static DEVICE_ATTR(q_stored, 0660, cs40l2x_q_stored_show,
		cs40l2x_q_stored_store);
static DEVICE_ATTR(comp_enable, 0660, cs40l2x_comp_enable_show,
		cs40l2x_comp_enable_store);
static DEVICE_ATTR(redc_comp_enable, 0660, cs40l2x_redc_comp_enable_show,
		cs40l2x_redc_comp_enable_store);
static DEVICE_ATTR(dig_scale, 0660, cs40l2x_dig_scale_show,
		cs40l2x_dig_scale_store);
static DEVICE_ATTR(gpio1_dig_scale, 0660, cs40l2x_gpio1_dig_scale_show,
		cs40l2x_gpio1_dig_scale_store);
static DEVICE_ATTR(gpio1_rise_dig_scale, 0660,
		cs40l2x_gpio1_rise_dig_scale_show,
		cs40l2x_gpio1_rise_dig_scale_store);
static DEVICE_ATTR(gpio1_fall_dig_scale, 0660,
		cs40l2x_gpio1_fall_dig_scale_show,
		cs40l2x_gpio1_fall_dig_scale_store);
static DEVICE_ATTR(gpio2_rise_dig_scale, 0660,
		cs40l2x_gpio2_rise_dig_scale_show,
		cs40l2x_gpio2_rise_dig_scale_store);
static DEVICE_ATTR(gpio2_fall_dig_scale, 0660,
		cs40l2x_gpio2_fall_dig_scale_show,
		cs40l2x_gpio2_fall_dig_scale_store);
static DEVICE_ATTR(gpio3_rise_dig_scale, 0660,
		cs40l2x_gpio3_rise_dig_scale_show,
		cs40l2x_gpio3_rise_dig_scale_store);
static DEVICE_ATTR(gpio3_fall_dig_scale, 0660,
		cs40l2x_gpio3_fall_dig_scale_show,
		cs40l2x_gpio3_fall_dig_scale_store);
static DEVICE_ATTR(gpio4_rise_dig_scale, 0660,
		cs40l2x_gpio4_rise_dig_scale_show,
		cs40l2x_gpio4_rise_dig_scale_store);
static DEVICE_ATTR(gpio4_fall_dig_scale, 0660,
		cs40l2x_gpio4_fall_dig_scale_show,
		cs40l2x_gpio4_fall_dig_scale_store);
static DEVICE_ATTR(cp_dig_scale, 0660, cs40l2x_cp_dig_scale_show,
		cs40l2x_cp_dig_scale_store);
static DEVICE_ATTR(heartbeat, 0660, cs40l2x_heartbeat_show, NULL);
static DEVICE_ATTR(num_waves, 0660, cs40l2x_num_waves_show, NULL);
static DEVICE_ATTR(num_virtual_waves, 0660, cs40l2x_num_virtual_waves_show,
			NULL);
static DEVICE_ATTR(fw_rev, 0660, cs40l2x_fw_rev_show, NULL);
static DEVICE_ATTR(vpp_measured, 0660, cs40l2x_vpp_measured_show, NULL);
static DEVICE_ATTR(ipp_measured, 0660, cs40l2x_ipp_measured_show, NULL);
static DEVICE_ATTR(vbatt_max, 0660, cs40l2x_vbatt_max_show,
		cs40l2x_vbatt_max_store);
static DEVICE_ATTR(vbatt_min, 0660, cs40l2x_vbatt_min_show,
		cs40l2x_vbatt_min_store);
static DEVICE_ATTR(asp_enable, 0660, cs40l2x_asp_enable_show,
		cs40l2x_asp_enable_store);
static DEVICE_ATTR(exc_enable, 0660, cs40l2x_exc_enable_show,
		cs40l2x_exc_enable_store);
static DEVICE_ATTR(hw_err_count, 0660, cs40l2x_hw_err_count_show,
		cs40l2x_hw_err_count_store);
static DEVICE_ATTR(hw_reset, 0660, cs40l2x_hw_reset_show,
		cs40l2x_hw_reset_store);
static DEVICE_ATTR(wt_file, 0660, cs40l2x_wt_file_show, cs40l2x_wt_file_store);
static DEVICE_ATTR(wt_date, 0660, cs40l2x_wt_date_show, NULL);
static DEVICE_ATTR(clab_enable, 0660, cs40l2x_clab_enable_show,
		cs40l2x_clab_enable_store);
static DEVICE_ATTR(clab_peak, 0660, cs40l2x_clab_peak_show,
		cs40l2x_clab_peak_store);
static DEVICE_ATTR(pwle_regulation_enable, 0660, cs40l2x_par_enable_show,
		cs40l2x_par_enable_store);
static DEVICE_ATTR(gain_compensation_enable, 0660, cs40l2x_par_gain_comp_show,
		cs40l2x_par_gain_comp_store);
static DEVICE_ATTR(vibe_state, 0660, cs40l2x_vibe_state_show, NULL);
static DEVICE_ATTR(safe_save_state, 0660, cs40l2x_safe_save_state_show, NULL);
static DEVICE_ATTR(max_back_emf, 0660, cs40l2x_max_back_emf_show,
		cs40l2x_max_back_emf_store);
static DEVICE_ATTR(autosuspend_delay, 0660, cs40l2x_autosuspend_delay_show,
		   cs40l2x_autosuspend_delay_store);
static DEVICE_ATTR(pwle, 0660, cs40l2x_pwle_show, cs40l2x_pwle_store);
static DEVICE_ATTR(num_virtual_composite, 0660,
	cs40l2x_num_virtual_composite_show, NULL);
static DEVICE_ATTR(num_virtual_pwle, 0660,
	cs40l2x_num_virtual_pwle_show, NULL);
static DEVICE_ATTR(virtual_composite_indexes, 0660,
	cs40l2x_composite_indexes_show, NULL);
static DEVICE_ATTR(virtual_pwle_indexes, 0660,
	cs40l2x_pwle_indexes_show, NULL);
static DEVICE_ATTR(available_pwle_segments, 0660,
	cs40l2x_available_pwle_segs_show, NULL);

static struct attribute *cs40l2x_dev_attrs[] = {
	&dev_attr_cp_trigger_index.attr,
	&dev_attr_cp_trigger_queue.attr,
	&dev_attr_cp_trigger_duration.attr,
	&dev_attr_cp_trigger_q_sub.attr,
	&dev_attr_hiber_cmd.attr,
	&dev_attr_hiber_timeout.attr,
	&dev_attr_gpio1_enable.attr,
	&dev_attr_gpio1_rise_index.attr,
	&dev_attr_gpio1_fall_index.attr,
	&dev_attr_gpio1_fall_timeout.attr,
	&dev_attr_gpio2_rise_index.attr,
	&dev_attr_gpio2_fall_index.attr,
	&dev_attr_gpio3_rise_index.attr,
	&dev_attr_gpio3_fall_index.attr,
	&dev_attr_gpio4_rise_index.attr,
	&dev_attr_gpio4_fall_index.attr,
	&dev_attr_standby_timeout.attr,
	&dev_attr_f0_measured.attr,
	&dev_attr_f0_stored.attr,
	&dev_attr_die_temp.attr,
	&dev_attr_vbst_avg.attr,
	&dev_attr_bemf_measured.attr,
	&dev_attr_dynamic_f0.attr,
	&dev_attr_dynamic_f0_index.attr,
	&dev_attr_dynamic_f0_val.attr,
	&dev_attr_f0_offset.attr,
	&dev_attr_redc_measured.attr,
	&dev_attr_redc_stored.attr,
	&dev_attr_q_measured.attr,
	&dev_attr_q_stored.attr,
	&dev_attr_comp_enable.attr,
	&dev_attr_redc_comp_enable.attr,
	&dev_attr_dig_scale.attr,
	&dev_attr_gpio1_dig_scale.attr,
	&dev_attr_gpio1_rise_dig_scale.attr,
	&dev_attr_gpio1_fall_dig_scale.attr,
	&dev_attr_gpio2_rise_dig_scale.attr,
	&dev_attr_gpio2_fall_dig_scale.attr,
	&dev_attr_gpio3_rise_dig_scale.attr,
	&dev_attr_gpio3_fall_dig_scale.attr,
	&dev_attr_gpio4_rise_dig_scale.attr,
	&dev_attr_gpio4_fall_dig_scale.attr,
	&dev_attr_cp_dig_scale.attr,
	&dev_attr_heartbeat.attr,
	&dev_attr_num_waves.attr,
	&dev_attr_num_virtual_waves.attr,
	&dev_attr_fw_rev.attr,
	&dev_attr_vpp_measured.attr,
	&dev_attr_ipp_measured.attr,
	&dev_attr_vbatt_max.attr,
	&dev_attr_vbatt_min.attr,
	&dev_attr_asp_enable.attr,
	&dev_attr_exc_enable.attr,
	&dev_attr_hw_err_count.attr,
	&dev_attr_hw_reset.attr,
	&dev_attr_wt_file.attr,
	&dev_attr_wt_date.attr,
	&dev_attr_clab_enable.attr,
	&dev_attr_clab_peak.attr,
	&dev_attr_pwle_regulation_enable.attr,
	&dev_attr_gain_compensation_enable.attr,
	&dev_attr_vibe_state.attr,
	&dev_attr_safe_save_state.attr,
	&dev_attr_max_back_emf.attr,
	&dev_attr_autosuspend_delay.attr,
	&dev_attr_pwle.attr,
	&dev_attr_num_virtual_composite.attr,
	&dev_attr_num_virtual_pwle.attr,
	&dev_attr_virtual_composite_indexes.attr,
	&dev_attr_virtual_pwle_indexes.attr,
	&dev_attr_available_pwle_segments.attr,
	NULL,
};

static struct attribute_group cs40l2x_dev_attr_group = {
	.attrs = cs40l2x_dev_attrs,
};

static void cs40l2x_wl_apply(struct cs40l2x_private *cs40l2x)
{
	struct device *dev = cs40l2x->dev;

	pm_runtime_get_sync(cs40l2x->dev);

	pm_stay_awake(dev);
	dev_dbg(dev, "Applied suspend blocker\n");
}

static void cs40l2x_wl_relax(struct cs40l2x_private *cs40l2x)
{
	struct device *dev = cs40l2x->dev;

	pm_relax(dev);

	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	dev_dbg(dev, "Relaxed suspend blocker\n");
}

static int cs40l2x_read_dyn_f0_table(struct cs40l2x_private *cs40l2x)
{
	struct regmap *regmap = cs40l2x->regmap;
	struct device *dev = cs40l2x->dev;
	unsigned int enable = 0, reg, data[CS40l2X_F0_MAX_ENTRIES];
	int ret, i, j = 0;

	reg = cs40l2x_dsp_reg(cs40l2x, "DYNAMIC_F0_ENABLED",
					CS40L2X_XM_UNPACKED_TYPE,
					CS40L2X_ALGO_ID_DYN_F0);

	if (reg) {
		ret = regmap_read(regmap, reg, &enable);
		if (ret)
			return ret;
	}

	if (!enable)
		return 0;

	memset(&data[0], 0, sizeof(data));
	ret = regmap_bulk_read(regmap, cs40l2x_dsp_reg(cs40l2x, "DYN_F0_TABLE",
				CS40L2X_XM_UNPACKED_TYPE,
				CS40L2X_ALGO_ID_DYN_F0), &data[0],
				CS40l2X_F0_MAX_ENTRIES);
	if (ret)
		return ret;

	for (i = 0; i < CS40l2X_F0_MAX_ENTRIES; i++) {
		dev_dbg(dev, "%d dyn f0 entry 0x%x\n", i,
				data[i]);

		if (data[i] == CS40L2X_DYN_F0_DEFAULT)
			continue;

		cs40l2x->dynamic_f0[j].index =
				data[i] >> CS40L2X_DYN_F0_INDEX_SHIFT;
		cs40l2x->dynamic_f0[j].f0 = data[i] & CS40L2X_DYN_F0_MASK;
		cs40l2x->dynamic_f0[j++].changed = true;
	}

	return 0;
}

static int cs40l2x_enable_classh(struct cs40l2x_private *cs40l2x)
{
	struct regmap *regmap = cs40l2x->regmap;
	int ret, i;

	for (i = 0; i < CS40L2X_MAX_WAVEFORMS; i++)
		if (cs40l2x->clab_wt_en[i] || cs40l2x->f0_wt_en[i])
			break;

	if (i == CS40L2X_MAX_WAVEFORMS)
		return 0;

	/* Add 50 ms delay to settle the waveform */
	msleep(CS40L2X_SETTLE_DELAY_MS);
	ret = regmap_update_bits(regmap, CS40L2X_BSTCVRT_VCTRL2,
					CS40L2X_BST_CTL_SEL_MASK,
					CS40L2X_BST_CTL_SEL_CLASSH);
	if (ret)
		return ret;

	ret = regmap_update_bits(regmap, CS40L2X_PWR_CTRL3,
			CS40L2X_CLASSH_EN_MASK,
			1 << CS40L2X_CLASSH_EN_SHIFT);
	if (ret)
		return ret;

	return 0;
}

static void cs40l2x_vibe_mode_worker(struct work_struct *work)
{
	struct cs40l2x_private *cs40l2x =
		container_of(work, struct cs40l2x_private, vibe_mode_work);
	struct regmap *regmap = cs40l2x->regmap;
	struct device *dev = cs40l2x->dev;
	unsigned int val, reg = 0;
	int ret;

	pm_runtime_get_sync(cs40l2x->dev);

	ret = regmap_read(regmap, cs40l2x_dsp_reg(cs40l2x, "STATUS",
			CS40L2X_XM_UNPACKED_TYPE, CS40L2X_ALGO_ID_VIBE), &val);
	if (ret) {
		dev_err(dev, "Failed to capture playback status\n");
		goto err_exit;
	}

	if (val != CS40L2X_STATUS_IDLE)
		goto err_exit;

	if (cs40l2x->vibe_mode == CS40L2X_VIBE_MODE_HAPTIC
			&& cs40l2x->asp_enable == CS40L2X_ASP_ENABLED) {
		/* switch to audio mode */
		ret = cs40l2x_refclk_switch(cs40l2x,
				cs40l2x->pdata.asp_bclk_freq);
		if (ret) {
			dev_err(dev, "Failed to switch to audio-rate REFCLK\n");
			goto err_exit;
		}

		ret = cs40l2x_asp_switch(cs40l2x, CS40L2X_ASP_ENABLED);
		if (ret) {
			dev_err(dev, "Failed to enable ASP\n");
			goto err_exit;
		}

		cs40l2x->vibe_mode = CS40L2X_VIBE_MODE_AUDIO;
		if (cs40l2x->vibe_state != CS40L2X_VIBE_STATE_RUNNING)
			cs40l2x_wl_apply(cs40l2x);
	} else if (cs40l2x->vibe_mode == CS40L2X_VIBE_MODE_AUDIO
			&& cs40l2x->asp_enable == CS40L2X_ASP_ENABLED) {
		/* resume audio mode */
		ret = regmap_read(regmap,
				cs40l2x_dsp_reg(cs40l2x, "I2S_ENABLED",
						CS40L2X_XM_UNPACKED_TYPE,
						cs40l2x->fw_desc->id),
				&val);
		if (ret) {
			dev_err(dev, "Failed to capture pause status\n");
			goto err_exit;
		}

		if (val == CS40L2X_I2S_ENABLED)
			goto err_exit;

		if (cs40l2x->pbq_state == CS40L2X_PBQ_STATE_IDLE)
			cs40l2x_set_state(cs40l2x, CS40L2X_VIBE_STATE_STOPPED);

		ret = cs40l2x_user_ctrl_exec(cs40l2x, CS40L2X_USER_CTRL_PLAY,
				0, NULL);
		if (ret)
			dev_err(dev, "Failed to resume playback\n");
	} else if (cs40l2x->vibe_mode == CS40L2X_VIBE_MODE_AUDIO
			&& cs40l2x->asp_enable == CS40L2X_ASP_DISABLED) {
		/* switch to haptic mode */
		ret = cs40l2x_asp_switch(cs40l2x, CS40L2X_ASP_DISABLED);
		if (ret) {
			dev_err(dev, "Failed to disable ASP\n");
			goto err_exit;
		}

		ret = cs40l2x_refclk_switch(cs40l2x,
				CS40L2X_PLL_REFCLK_FREQ_32K);
		if (ret) {
			dev_err(dev, "Failed to switch to 32.768-kHz REFCLK\n");
			goto err_exit;
		}

		cs40l2x->vibe_mode = CS40L2X_VIBE_MODE_HAPTIC;

		if (cs40l2x->pbq_state != CS40L2X_PBQ_STATE_IDLE)
			goto err_exit;

		cs40l2x_set_state(cs40l2x, CS40L2X_VIBE_STATE_STOPPED);
		cs40l2x_wl_relax(cs40l2x);
	} else if (cs40l2x->vibe_mode == CS40L2X_VIBE_MODE_HAPTIC &&
						cs40l2x->a2h_enable) {

		if (cs40l2x->vibe_state == CS40L2X_VIBE_STATE_RUNNING)
			cs40l2x_set_state(cs40l2x, CS40L2X_VIBE_STATE_STOPPED);

		ret = regmap_update_bits(regmap, CS40L2X_BSTCVRT_VCTRL2,
					CS40L2X_BST_CTL_SEL_MASK,
					CS40L2X_BST_CTL_SEL_CLASSH);
		if (ret)
			goto err_exit;

		ret = regmap_update_bits(regmap, CS40L2X_PWR_CTRL3,
				CS40L2X_CLASSH_EN_MASK,
				1 << CS40L2X_CLASSH_EN_SHIFT);
		if (ret)
			goto err_exit;

		reg = cs40l2x_dsp_reg(cs40l2x, "A2HEN",
					CS40L2X_XM_UNPACKED_TYPE,
					CS40L2X_ALGO_ID_A2H);
		if (!reg) {
			dev_err(dev, "Cannot get A2HEN register\n");
			ret = -EINVAL;
			goto err_exit;
		}

		ret = regmap_write(regmap, reg, CS40L2X_A2H_DISABLE);
		if (ret)
			goto err_exit;

		ret = regmap_write(regmap, CS40L2X_DSP_VIRT1_MBOX_5,
				CS40L2X_REINIT_A2H_CMD);
		if (ret)
			goto err_exit;

		usleep_range(3000, 3100);
		ret = regmap_write(regmap, reg, CS40L2X_A2H_ENABLE);
		if (ret)
			goto err_exit;

		ret = regmap_write(regmap, CS40L2X_DSP_VIRT1_MBOX_5,
					CS40L2X_A2H_I2S_START);
		if (ret)
			goto err_exit;

	} else {
		/* haptic-mode teardown */
		if (cs40l2x->vibe_state == CS40L2X_VIBE_STATE_STOPPED
				|| cs40l2x->pbq_state != CS40L2X_PBQ_STATE_IDLE)
			goto err_exit;

		if (cs40l2x->amp_gnd_stby) {
			ret = regmap_write(regmap,
					CS40L2X_SPK_FORCE_TST_1,
					CS40L2X_FORCE_SPK_GND);
			if (ret) {
				dev_err(dev,
					"Failed to ground amplifier outputs\n");
				goto err_exit;
			}
		}
		cs40l2x_set_state(cs40l2x, CS40L2X_VIBE_STATE_STOPPED);
		cs40l2x_wl_relax(cs40l2x);
	}

	if (cs40l2x->dyn_f0_enable) {
		ret = cs40l2x_read_dyn_f0_table(cs40l2x);
		if (ret) {
			dev_err(dev, "Failed to read f0 table %d\n", ret);
			goto err_exit;
		}
	}

	ret = cs40l2x_enable_classh(cs40l2x);
	if (ret)
		goto err_exit;

err_exit:
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);
}

static enum hrtimer_restart cs40l2x_asp_timer(struct hrtimer *timer)
{
	struct cs40l2x_private *cs40l2x =
		container_of(timer, struct cs40l2x_private, asp_timer);

	queue_work(cs40l2x->vibe_workqueue, &cs40l2x->vibe_mode_work);

	return HRTIMER_NORESTART;
}

static int cs40l2x_stop_playback(struct cs40l2x_private *cs40l2x)
{
	int ret, i;

	for (i = 0; i < CS40L2X_ENDPLAYBACK_RETRIES; i++) {
		ret = regmap_write(cs40l2x->regmap,
				cs40l2x_dsp_reg(cs40l2x, "ENDPLAYBACK",
						CS40L2X_XM_UNPACKED_TYPE,
						cs40l2x->fw_desc->id),
				CS40L2X_ENDPLAYBACK_REQ);
		if (!ret)
			return 0;

		usleep_range(10000, 10100);
	}

	dev_err(cs40l2x->dev, "Parking device in reset\n");
	gpiod_set_value_cansleep(cs40l2x->reset_gpio, 0);

	return -EIO;
}

static int cs40l2x_pbq_cancel(struct cs40l2x_private *cs40l2x)
{
	int ret;

	hrtimer_cancel(&cs40l2x->pbq_timer);

	switch (cs40l2x->pbq_state) {
	case CS40L2X_PBQ_STATE_SILENT:
	case CS40L2X_PBQ_STATE_IDLE:
		ret = cs40l2x_cp_dig_scale_set(cs40l2x,
				cs40l2x->pbq_cp_dig_scale);
		if (ret)
			return ret;

		cs40l2x_set_state(cs40l2x, CS40L2X_VIBE_STATE_STOPPED);
		if (cs40l2x->vibe_mode != CS40L2X_VIBE_MODE_AUDIO)
			cs40l2x_wl_relax(cs40l2x);
		break;
	case CS40L2X_PBQ_STATE_PLAYING:
		ret = cs40l2x_stop_playback(cs40l2x);
		if (ret)
			return ret;

		ret = cs40l2x_cp_dig_scale_set(cs40l2x,
				cs40l2x->pbq_cp_dig_scale);
		if (ret)
			return ret;

		if (cs40l2x->event_control & CS40L2X_EVENT_END_ENABLED)
			break;

		cs40l2x_set_state(cs40l2x, CS40L2X_VIBE_STATE_STOPPED);
		if (cs40l2x->vibe_mode != CS40L2X_VIBE_MODE_AUDIO)
			cs40l2x_wl_relax(cs40l2x);
		break;
	default:
		return -EINVAL;
	}

	cs40l2x->pbq_state = CS40L2X_PBQ_STATE_IDLE;
	cs40l2x->cp_trailer_index = CS40L2X_INDEX_IDLE;

	return 0;
}

static int cs40l2x_pbq_pair_launch(struct cs40l2x_private *cs40l2x)
{
	unsigned int tag, mag, cp_dig_scale;
	int ret, i;

	do {
		/* restart queue as necessary */
		if (cs40l2x->pbq_index == cs40l2x->pbq_depth) {
			cs40l2x->pbq_index = 0;
			for (i = 0; i < cs40l2x->pbq_depth; i++)
				cs40l2x->pbq_pairs[i].remain =
						cs40l2x->pbq_pairs[i].repeat;

			switch (cs40l2x->pbq_remain) {
			case -1:
				/* loop until stopped */
				break;
			case 0:
				/* queue is finished */
				cs40l2x->pbq_state = CS40L2X_PBQ_STATE_IDLE;
				return cs40l2x_pbq_cancel(cs40l2x);
			default:
				/* loop once more */
				cs40l2x->pbq_remain--;
			}
		}

		tag = cs40l2x->pbq_pairs[cs40l2x->pbq_index].tag;
		mag = cs40l2x->pbq_pairs[cs40l2x->pbq_index].mag;

		switch (tag) {
		case CS40L2X_PBQ_TAG_SILENCE:
			if (cs40l2x->amp_gnd_stby) {
				ret = regmap_write(cs40l2x->regmap,
						CS40L2X_SPK_FORCE_TST_1,
						CS40L2X_FORCE_SPK_GND);
				if (ret)
					return ret;
			}

			hrtimer_start(&cs40l2x->pbq_timer,
					ktime_set(mag / 1000,
							(mag % 1000) * 1000000),
					HRTIMER_MODE_REL);
			cs40l2x->pbq_state = CS40L2X_PBQ_STATE_SILENT;
			cs40l2x->pbq_index++;
			break;
		case CS40L2X_PBQ_TAG_START:
			cs40l2x->pbq_index++;
			break;
		case CS40L2X_PBQ_TAG_STOP:
			if (cs40l2x->pbq_pairs[cs40l2x->pbq_index].remain) {
				cs40l2x->pbq_pairs[cs40l2x->pbq_index].remain--;
				cs40l2x->pbq_index = mag;
			} else {
				cs40l2x->pbq_index++;
			}
			break;
		default:
			cp_dig_scale = cs40l2x->pbq_cp_dig_scale
					+ cs40l2x_pbq_dig_scale[mag];
			if (cp_dig_scale > CS40L2X_DIG_SCALE_MAX)
				cp_dig_scale = CS40L2X_DIG_SCALE_MAX;

			ret = cs40l2x_cp_dig_scale_set(cs40l2x, cp_dig_scale);
			if (ret)
				return ret;

			if (cs40l2x->amp_gnd_stby) {
				ret = regmap_write(cs40l2x->regmap,
						CS40L2X_SPK_FORCE_TST_1,
						CS40L2X_FORCE_SPK_FREE);
				if (ret)
					return ret;
			}

			ret = cs40l2x_ack_write(cs40l2x,
					CS40L2X_MBOX_TRIGGERINDEX, tag,
					CS40L2X_MBOX_TRIGGERRESET);
			if (ret) {
				cs40l2x_set_state(cs40l2x,
					CS40L2X_VIBE_STATE_STOPPED);
				dev_err(cs40l2x->dev,
					"Cannot set PBQ index %d\n", tag);
				return ret;
			}

			cs40l2x->pbq_state = CS40L2X_PBQ_STATE_PLAYING;
			cs40l2x->pbq_index++;

			if (cs40l2x->event_control & CS40L2X_EVENT_END_ENABLED)
				continue;

			hrtimer_start(&cs40l2x->pbq_timer,
					ktime_set(0, CS40L2X_PBQ_POLL_NS),
					HRTIMER_MODE_REL);
		}

	} while (tag == CS40L2X_PBQ_TAG_START || tag == CS40L2X_PBQ_TAG_STOP);

	return 0;
}

static void cs40l2x_vibe_pbq_worker(struct work_struct *work)
{
	struct cs40l2x_private *cs40l2x =
		container_of(work, struct cs40l2x_private, vibe_pbq_work);
	struct regmap *regmap = cs40l2x->regmap;
	struct device *dev = cs40l2x->dev;
	unsigned int val;
	int ret;

	pm_runtime_get_sync(cs40l2x->dev);

	switch (cs40l2x->pbq_state) {
	case CS40L2X_PBQ_STATE_IDLE:
		goto err_exit;

	case CS40L2X_PBQ_STATE_PLAYING:
		if (cs40l2x->event_control & CS40L2X_EVENT_END_ENABLED)
			break;

		ret = regmap_read(regmap,
				cs40l2x_dsp_reg(cs40l2x, "STATUS",
						CS40L2X_XM_UNPACKED_TYPE,
						CS40L2X_ALGO_ID_VIBE),
				&val);
		if (ret) {
			dev_err(dev, "Failed to capture playback status\n");
			goto err_exit;
		}

		if (val != CS40L2X_STATUS_IDLE) {
			hrtimer_start(&cs40l2x->pbq_timer,
					ktime_set(0, CS40L2X_PBQ_POLL_NS),
					HRTIMER_MODE_REL);
			goto err_exit;
		}
		break;

	case CS40L2X_PBQ_STATE_SILENT:
		break;

	default:
		dev_err(dev, "Unexpected playback queue state: %d\n",
				cs40l2x->pbq_state);
		goto err_exit;
	}

	ret = regmap_read(regmap,
			  cs40l2x_dsp_reg(cs40l2x, "STATUS",
					  CS40L2X_XM_UNPACKED_TYPE,
					  CS40L2X_ALGO_ID_VIBE),
			  &val);
	if (ret) {
		dev_err(dev, "Failed to capture playback status\n");
		goto err_exit;
	}

	if (val != CS40L2X_STATUS_IDLE)
		goto err_exit;

	ret = cs40l2x_pbq_pair_launch(cs40l2x);
	if (ret)
		dev_err(dev, "Failed to continue playback queue\n");

err_exit:
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);
}

static enum hrtimer_restart cs40l2x_pbq_timer(struct hrtimer *timer)
{
	struct cs40l2x_private *cs40l2x =
		container_of(timer, struct cs40l2x_private, pbq_timer);

	queue_work(cs40l2x->vibe_workqueue, &cs40l2x->vibe_pbq_work);

	return HRTIMER_NORESTART;
}

static int cs40l2x_diag_enable(struct cs40l2x_private *cs40l2x,
			unsigned int val)
{
	struct regmap *regmap = cs40l2x->regmap;

	switch (cs40l2x->fw_desc->id) {
	case CS40L2X_FW_ID_ORIG:
		/*
		 * STIMULUS_MODE is not automatically returned to a reset
		 * value as with other mailbox registers, therefore it is
		 * written without polling for subsequent acknowledgment
		 */
		return regmap_write(regmap, CS40L2X_MBOX_STIMULUS_MODE, val);
	case CS40L2X_FW_ID_CAL:
		return regmap_write(regmap,
				cs40l2x_dsp_reg(cs40l2x, "F0_TRACKING_ENABLE",
						CS40L2X_XM_UNPACKED_TYPE,
						CS40L2X_ALGO_ID_F0), val);
	default:
		return -EPERM;
	}
}

static int cs40l2x_diag_capture(struct cs40l2x_private *cs40l2x)
{
	struct regmap *regmap = cs40l2x->regmap;
	unsigned int val, reg;
	int ret;

	switch (cs40l2x->diag_state) {
	case CS40L2X_DIAG_STATE_RUN1:
		ret = regmap_read(regmap,
				cs40l2x_dsp_reg(cs40l2x, "F0",
						CS40L2X_XM_UNPACKED_TYPE,
						CS40L2X_ALGO_ID_F0),
				&cs40l2x->f0_measured);
		if (ret)
			return ret;

		ret = regmap_read(regmap,
				cs40l2x_dsp_reg(cs40l2x, "REDC",
						CS40L2X_XM_UNPACKED_TYPE,
						CS40L2X_ALGO_ID_F0),
				&cs40l2x->redc_measured);
		if (ret)
			return ret;

		reg = cs40l2x_dsp_reg(cs40l2x, "MAXBACKEMF",
						CS40L2X_XM_UNPACKED_TYPE,
						CS40L2X_ALGO_ID_F0);

		if (reg) {
			ret = regmap_read(regmap, reg, &cs40l2x->bemf_measured);
			if (ret)
				return ret;
		}

		cs40l2x->diag_state = CS40L2X_DIAG_STATE_DONE1;
		return 0;

	case CS40L2X_DIAG_STATE_RUN2:
		ret = regmap_read(regmap,
				cs40l2x_dsp_reg(cs40l2x, "F0_TRACKING_ENABLE",
						CS40L2X_XM_UNPACKED_TYPE,
						CS40L2X_ALGO_ID_F0),
				&val);
		if (ret)
			return ret;

		if (val != CS40L2X_F0_TRACKING_OFF)
			return -EBUSY;

		ret = regmap_read(regmap,
				cs40l2x_dsp_reg(cs40l2x, "Q_EST",
						CS40L2X_XM_UNPACKED_TYPE,
						CS40L2X_ALGO_ID_QEST),
				&val);
		if (ret)
			return ret;

		if (val & CS40L2X_QEST_ERROR)
			return -EIO;

		cs40l2x->q_measured = val;
		cs40l2x->diag_state = CS40L2X_DIAG_STATE_DONE2;
		return 0;

	default:
		return -EINVAL;
	}
}

static int cs40l2x_peak_capture(struct cs40l2x_private *cs40l2x)
{
	struct regmap *regmap = cs40l2x->regmap;
	unsigned int vmon_max, vmon_min, imon_max, imon_min;
	int ret;

	/* VMON min and max are returned as 24-bit two's-complement values */
	ret = regmap_read(regmap,
			cs40l2x_dsp_reg(cs40l2x, "VMONMAX",
					CS40L2X_XM_UNPACKED_TYPE,
					cs40l2x->fw_desc->id),
			&vmon_max);
	if (ret)
		return ret;
	if (vmon_max > CS40L2X_VMON_POSFS)
		vmon_max = ((vmon_max ^ CS40L2X_VMON_MASK) + 1) * -1;

	ret = regmap_read(regmap,
			cs40l2x_dsp_reg(cs40l2x, "VMONMIN",
					CS40L2X_XM_UNPACKED_TYPE,
					cs40l2x->fw_desc->id),
			&vmon_min);
	if (ret)
		return ret;
	if (vmon_min > CS40L2X_VMON_POSFS)
		vmon_min = ((vmon_min ^ CS40L2X_VMON_MASK) + 1) * -1;

	/* IMON min and max are returned as 24-bit two's-complement values */
	ret = regmap_read(regmap,
			cs40l2x_dsp_reg(cs40l2x, "IMONMAX",
					CS40L2X_XM_UNPACKED_TYPE,
					cs40l2x->fw_desc->id),
			&imon_max);
	if (ret)
		return ret;
	if (imon_max > CS40L2X_IMON_POSFS)
		imon_max = ((imon_max ^ CS40L2X_IMON_MASK) + 1) * -1;

	ret = regmap_read(regmap,
			cs40l2x_dsp_reg(cs40l2x, "IMONMIN",
					CS40L2X_XM_UNPACKED_TYPE,
					cs40l2x->fw_desc->id),
			&imon_min);
	if (ret)
		return ret;
	if (imon_min > CS40L2X_IMON_POSFS)
		imon_min = ((imon_min ^ CS40L2X_IMON_MASK) + 1) * -1;

	cs40l2x->vpp_measured = vmon_max - vmon_min;
	cs40l2x->ipp_measured = imon_max - imon_min;

	return 0;
}

static int cs40l2x_reset_recovery(struct cs40l2x_private *cs40l2x)
{
	bool wl_pending = (cs40l2x->vibe_mode == CS40L2X_VIBE_MODE_AUDIO)
			|| (cs40l2x->vibe_state == CS40L2X_VIBE_STATE_RUNNING);
	unsigned int fw_id_restore;
	int ret, i;

	if (cs40l2x->revid < CS40L2X_REVID_B1)
		return -EPERM;

	if (cs40l2x->asp_available) {
		ret = cs40l2x_wseq_replace(cs40l2x, CS40L2X_SP_ENABLES, 0);
		if (ret)
			return ret;

		ret = cs40l2x_wseq_replace(cs40l2x, CS40L2X_PLL_CLK_CTRL,
				((1 << CS40L2X_PLL_REFCLK_EN_SHIFT)
					& CS40L2X_PLL_REFCLK_EN_MASK) |
				((CS40L2X_PLL_REFCLK_SEL_MCLK
					<< CS40L2X_PLL_REFCLK_SEL_SHIFT)
					& CS40L2X_PLL_REFCLK_SEL_MASK));
		if (ret)
			return ret;

		cs40l2x->asp_enable = CS40L2X_ASP_DISABLED;
	}

	cs40l2x->vibe_mode = CS40L2X_VIBE_MODE_HAPTIC;
	cs40l2x_set_state(cs40l2x, CS40L2X_VIBE_STATE_STOPPED);

	cs40l2x->cp_trailer_index = CS40L2X_INDEX_IDLE;

	gpiod_set_value_cansleep(cs40l2x->reset_gpio, 0);
	usleep_range(2000, 2100);

	gpiod_set_value_cansleep(cs40l2x->reset_gpio, 1);
	usleep_range(1000, 1100);

	fw_id_restore = cs40l2x->fw_desc->id;
	cs40l2x->fw_desc = cs40l2x_firmware_match(cs40l2x, CS40L2X_FW_ID_B1ROM);

	ret = cs40l2x_firmware_swap(cs40l2x, fw_id_restore);
	if (ret)
		return ret;

	for (i = 0; i < cs40l2x->dsp_cache_depth; i++) {
		ret = regmap_write(cs40l2x->regmap,
				cs40l2x->dsp_cache[i].reg,
				cs40l2x->dsp_cache[i].val);
		if (ret) {
			dev_err(cs40l2x->dev, "Failed to restore DSP cache\n");
			return ret;
		}
	}

	if (cs40l2x->pbq_state != CS40L2X_PBQ_STATE_IDLE) {
		ret = cs40l2x_cp_dig_scale_set(cs40l2x,
				cs40l2x->pbq_cp_dig_scale);
		if (ret)
			return ret;

		cs40l2x->pbq_state = CS40L2X_PBQ_STATE_IDLE;
	}

	if (wl_pending)
		cs40l2x_wl_relax(cs40l2x);

	dev_info(cs40l2x->dev, "Successfully restored device state\n");

	return 0;
}

static int cs40l2x_check_recovery(struct cs40l2x_private *cs40l2x)
{
	struct i2c_client *i2c_client = to_i2c_client(cs40l2x->dev);
	unsigned int val;
	int ret = 0;

	ret = regmap_read(cs40l2x->regmap, CS40L2X_DSP1_RX2_SRC, &val);
	if (ret)
		dev_err(cs40l2x->dev, "Failed to read known register %d\n",
			ret);

	if (val == CS40L2X_DSP1_RXn_SRC_VMON)
		return 0;

	dev_err(cs40l2x->dev, "Failed to verify known register\n");

	/*
	 * resetting the device prompts it to briefly assert the /ALERT pin,
	 * so disable the interrupt line until the device has been restored
	 */
	disable_irq_nosync(i2c_client->irq);

	ret = cs40l2x_reset_recovery(cs40l2x);

	enable_irq(i2c_client->irq);

	return ret;
}

static int cs40l2x_classh_wt_check(struct cs40l2x_private *cs40l2x,
					const unsigned char *data,
					const int len, int *pos)
{
	struct device *dev = cs40l2x->dev;
	int i, index;
	unsigned int header_end = CS40L2X_WT_HEAD_END;

	if (*pos < 0 || *pos >= CS40L2X_MAX_WAVEFORMS)
		return -EINVAL;

	index = *pos;
	/* Check the wave table header for CLAB and F0 waveforms */
	for (i = 1; i < len; i += CS40L2X_WT_DESC_BYTE_OFFSET) {
		if (!memcmp(&header_end, (data + i), 3))
			break;

		if (*(data + i) & CS40L2X_CLAB_WT_EN)
			cs40l2x->clab_wt_en[index] = true;

		if (*(data + i) & CS40L2X_F0_WT_EN)
			cs40l2x->f0_wt_en[index] = true;

		dev_dbg(dev, "header = 0x%x clab_wt_en = 0x%x\n", *(data + i),
			 cs40l2x->clab_wt_en[index]);
		index++;
		if (index >= CS40L2X_MAX_WAVEFORMS) {
			dev_err(dev, "Overflow on waveforms\n");
			return -EFAULT;
		}
	}

	*pos += index;

	return 0;
}

static int cs40l2x_set_boost_voltage(struct cs40l2x_private *cs40l2x,
					unsigned int boost_ctl)
{
	struct regmap *regmap = cs40l2x->regmap;
	struct device *dev = cs40l2x->dev;
	unsigned int bst_ctl_scaled;
	int ret;

	if (boost_ctl)
		boost_ctl &= CS40L2X_PDATA_MASK;
	else
		boost_ctl = CS40L2X_BST_VOLT_MAX;

	switch (boost_ctl) {
	case 0:
		bst_ctl_scaled = boost_ctl;
		break;
	case CS40L2X_BST_VOLT_MIN ... CS40L2X_BST_VOLT_MAX:
		bst_ctl_scaled = ((boost_ctl - CS40L2X_BST_VOLT_MIN) / 50) + 1;
		break;
	default:
		dev_err(dev, "Invalid VBST limit: %d mV\n", boost_ctl);
		return -EINVAL;
	}

	ret = regmap_update_bits(regmap, CS40L2X_BSTCVRT_VCTRL1,
			CS40L2X_BST_CTL_MASK,
			bst_ctl_scaled << CS40L2X_BST_CTL_SHIFT);
	if (ret) {
		dev_err(dev, "Failed to write VBST limit\n");
		return ret;
	}

	ret = regmap_update_bits(regmap, CS40L2X_BSTCVRT_VCTRL2,
			CS40L2X_BST_CTL_LIM_EN_MASK,
			1 << CS40L2X_BST_CTL_LIM_EN_SHIFT);
	if (ret) {
		dev_err(dev, "Failed to configure VBST control\n");
		return ret;
	}

	return 0;
}


static int cs40l2x_cond_classh(struct cs40l2x_private *cs40l2x, int index)
{
	struct regmap *regmap = cs40l2x->regmap;
	unsigned int enable = 0, reg, boost = cs40l2x->pdata.boost_ctl;
	int ret = 0;
	bool disable_classh = false;


	if (index < 0 || index >= CS40L2X_MAX_WAVEFORMS)
		return -EINVAL;

	if (cs40l2x->dyn_f0_enable) {
		reg = cs40l2x_dsp_reg(cs40l2x, "DYNAMIC_F0_ENABLED",
						CS40L2X_XM_UNPACKED_TYPE,
							CS40L2X_ALGO_ID_DYN_F0);
		if (reg) {
			ret = regmap_read(regmap, reg, &enable);
			if (ret)
				return ret;
		}
	}

	if (enable) {
		if (cs40l2x->f0_wt_en[index]) {
			boost = cs40l2x->pdata.boost_ctl;
			disable_classh = true;
		}
	}

	reg = cs40l2x_dsp_reg(cs40l2x, "CLAB_ENABLED",
			CS40L2X_XM_UNPACKED_TYPE, CS40L2X_ALGO_ID_CLAB);

	if (reg) {

		ret = regmap_read(regmap, reg, &enable);
		if (ret)
			return ret;

		if (enable) {
			if (cs40l2x->clab_wt_en[index]) {
				boost = cs40l2x->pdata.boost_clab;
				disable_classh = true;
			}
		}
	}

	if (disable_classh) {
		ret = cs40l2x_set_boost_voltage(cs40l2x, boost);
		if (ret)
			return ret;

		ret = regmap_update_bits(regmap, CS40L2X_BSTCVRT_VCTRL2,
						CS40L2X_BST_CTL_SEL_MASK,
						CS40L2X_BST_CTL_SEL_CP_VAL);
		if (ret)
			return ret;

		ret = regmap_update_bits(regmap, CS40L2X_PWR_CTRL3,
				CS40L2X_CLASSH_EN_MASK,
				0 << CS40L2X_CLASSH_EN_SHIFT);
		if (ret)
			return ret;
	} else {
		ret = regmap_update_bits(regmap, CS40L2X_BSTCVRT_VCTRL2,
						CS40L2X_BST_CTL_SEL_MASK,
						CS40L2X_BST_CTL_SEL_CLASSH);
		if (ret)
			return ret;

		ret = regmap_update_bits(regmap, CS40L2X_PWR_CTRL3,
				CS40L2X_CLASSH_EN_MASK,
				1 << CS40L2X_CLASSH_EN_SHIFT);
		if (ret)
			return ret;

		ret = cs40l2x_set_boost_voltage(cs40l2x, boost);
		if (ret)
			return ret;

	}

	return 0;
}

static void cs40l2x_vibe_start_worker(struct work_struct *work)
{
	struct cs40l2x_private *cs40l2x =
		container_of(work, struct cs40l2x_private, vibe_start_work);
	struct regmap *regmap = cs40l2x->regmap;
	struct device *dev = cs40l2x->dev;
	int ret, i;
	unsigned int reg;

	if (!cs40l2x->virtual_stored) {
		dev_warn(dev, "Unsafe condition encountered.\n");
		return;
	}

	cs40l2x_set_safe_save_state(cs40l2x, CS40L2X_SAVE_UNSAFE);

	pm_runtime_get_sync(cs40l2x->dev);

	/* handle interruption of special cases */
	switch (cs40l2x->cp_trailer_index) {
	case CS40L2X_INDEX_QEST:
	case CS40L2X_INDEX_PEAK:
	case CS40L2X_INDEX_DIAG:
		dev_err(dev, "Ignored attempt to interrupt measurement\n");
		goto err_mutex;

	case CS40L2X_INDEX_PBQ:
		dev_err(dev, "Ignored attempt to interrupt playback queue\n");
		goto err_mutex;
	}

	for (i = 0; i < CS40L2X_NUM_HW_ERRS; i++)
		if (cs40l2x->hw_err_count[i] > CS40L2X_HW_ERR_COUNT_MAX)
			dev_warn(dev, "Pending %s error\n",
					cs40l2x_hw_errs[i].err_name);
		else
			cs40l2x->hw_err_count[i] = 0;

	if (cs40l2x->pdata.auto_recovery) {
		ret = cs40l2x_check_recovery(cs40l2x);
		if (ret)
			goto err_mutex;
	}

	if (cs40l2x->cp_trigger_index == CS40L2X_INDEX_QEST
			&& cs40l2x->diag_state < CS40L2X_DIAG_STATE_DONE1) {
		dev_err(dev, "Diagnostics index (%d) not yet administered\n",
				CS40L2X_INDEX_DIAG);
		cs40l2x->cp_trailer_index = CS40L2X_INDEX_IDLE;
		goto err_mutex;
	} else {
		cs40l2x->cp_trailer_index = cs40l2x->cp_trigger_index;
	}

	switch (cs40l2x->cp_trailer_index) {
	case CS40L2X_INDEX_DIAG:

		reg = cs40l2x_dsp_reg(cs40l2x, "MAXBACKEMF",
			      CS40L2X_XM_UNPACKED_TYPE, CS40L2X_ALGO_ID_F0);
		if (reg) {
			ret = regmap_write(regmap, reg, 0);
			if (ret)
				goto err_mutex;
		}

	case CS40L2X_INDEX_VIBE:
	case CS40L2X_INDEX_CONT_MIN ... CS40L2X_INDEX_CONT_MAX:
	case CS40L2X_INDEX_QEST:
	case CS40L2X_INDEX_PEAK:
#ifdef CONFIG_ANDROID_TIMED_OUTPUT
		hrtimer_start(&cs40l2x->vibe_timer,
				ktime_set(cs40l2x->vibe_timeout / 1000,
						(cs40l2x->vibe_timeout % 1000)
						* 1000000),
				HRTIMER_MODE_REL);
		/* intentionally fall through */

#endif /* CONFIG_ANDROID_TIMED_OUTPUT */
	case CS40L2X_INDEX_PBQ:
		if (cs40l2x->vibe_mode != CS40L2X_VIBE_MODE_AUDIO
				&& cs40l2x->vibe_state
					!= CS40L2X_VIBE_STATE_RUNNING)
			cs40l2x_wl_apply(cs40l2x);
		cs40l2x_set_state(cs40l2x, CS40L2X_VIBE_STATE_RUNNING);
		break;

	case CS40L2X_INDEX_CLICK_MIN ... CS40L2X_INDEX_CLICK_MAX:
		if (!(cs40l2x->event_control & CS40L2X_EVENT_END_ENABLED))
			break;

		if (cs40l2x->vibe_mode != CS40L2X_VIBE_MODE_AUDIO
				&& cs40l2x->vibe_state
					!= CS40L2X_VIBE_STATE_RUNNING)
			cs40l2x_wl_apply(cs40l2x);
		cs40l2x_set_state(cs40l2x, CS40L2X_VIBE_STATE_RUNNING);
		break;
	}

	if (cs40l2x->amp_gnd_stby
			&& cs40l2x->cp_trailer_index != CS40L2X_INDEX_PBQ) {
		ret = regmap_write(regmap, CS40L2X_SPK_FORCE_TST_1,
				CS40L2X_FORCE_SPK_FREE);
		if (ret) {
			dev_err(dev, "Failed to free amplifier outputs\n");
			goto err_relax;
		}
	}

	switch (cs40l2x->cp_trailer_index) {
	case CS40L2X_INDEX_PEAK:
		cs40l2x->vpp_measured = -1;
		cs40l2x->ipp_measured = -1;

		ret = regmap_read(regmap,
				cs40l2x_dsp_reg(cs40l2x, "GPIO_ENABLE",
						CS40L2X_XM_UNPACKED_TYPE,
						cs40l2x->fw_desc->id),
				&cs40l2x->peak_gpio1_enable);
		if (ret) {
			dev_err(dev, "Failed to read GPIO1 configuration\n");
			break;
		}

		ret = regmap_write(regmap,
				cs40l2x_dsp_reg(cs40l2x, "GPIO_ENABLE",
						CS40L2X_XM_UNPACKED_TYPE,
						cs40l2x->fw_desc->id),
				CS40L2X_GPIO1_DISABLED);
		if (ret) {
			dev_err(dev, "Failed to disable GPIO1\n");
			break;
		}

		ret = cs40l2x_ack_write(cs40l2x, CS40L2X_MBOX_TRIGGER_MS,
				CS40L2X_INDEX_VIBE, CS40L2X_MBOX_TRIGGERRESET);
		if (ret)
			break;

		msleep(CS40L2X_PEAK_DELAY_MS);

		ret = regmap_write(regmap,
				cs40l2x_dsp_reg(cs40l2x, "VMONMAX",
						CS40L2X_XM_UNPACKED_TYPE,
						cs40l2x->fw_desc->id),
				CS40L2X_VMON_NEGFS);
		if (ret) {
			dev_err(dev, "Failed to reset maximum VMON\n");
			break;
		}

		ret = regmap_write(regmap,
				cs40l2x_dsp_reg(cs40l2x, "VMONMIN",
						CS40L2X_XM_UNPACKED_TYPE,
						cs40l2x->fw_desc->id),
				CS40L2X_VMON_POSFS);
		if (ret) {
			dev_err(dev, "Failed to reset minimum VMON\n");
			break;
		}

		ret = regmap_write(regmap,
				cs40l2x_dsp_reg(cs40l2x, "IMONMAX",
						CS40L2X_XM_UNPACKED_TYPE,
						cs40l2x->fw_desc->id),
				CS40L2X_IMON_NEGFS);
		if (ret) {
			dev_err(dev, "Failed to reset maximum IMON\n");
			break;
		}

		ret = regmap_write(regmap,
				cs40l2x_dsp_reg(cs40l2x, "IMONMIN",
						CS40L2X_XM_UNPACKED_TYPE,
						cs40l2x->fw_desc->id),
				CS40L2X_IMON_POSFS);
		if (ret)
			dev_err(dev, "Failed to reset minimum IMON\n");
		break;

	case CS40L2X_INDEX_VIBE:
	case CS40L2X_INDEX_CONT_MIN ... CS40L2X_INDEX_CONT_MAX:
		if (completion_done(&cs40l2x->hap_done))
			reinit_completion(&cs40l2x->hap_done);

		if (cs40l2x->cond_class_h_en) {
			ret = cs40l2x_cond_classh(cs40l2x,
				cs40l2x->cp_trailer_index);
			if (ret) {
				dev_err(dev, "Conditional ClassH failed %d\n",
					ret);
				dev_err(dev, "index %d\n",
					cs40l2x->cp_trailer_index);
				break;
			}
		}

		ret = cs40l2x_ack_write(cs40l2x, CS40L2X_MBOX_TRIGGER_MS,
				cs40l2x->cp_trailer_index & CS40L2X_INDEX_MASK,
				CS40L2X_MBOX_TRIGGERRESET);
		break;

	case CS40L2X_INDEX_CLICK_MIN ... CS40L2X_INDEX_CLICK_MAX:
		if (completion_done(&cs40l2x->hap_done))
			reinit_completion(&cs40l2x->hap_done);

		if (cs40l2x->cond_class_h_en) {
			ret = cs40l2x_cond_classh(cs40l2x,
				cs40l2x->cp_trailer_index);
			if (ret) {
				dev_err(dev, "Conditional ClassH failed\n");
				break;
			}
		}

		ret = cs40l2x_ack_write(cs40l2x, CS40L2X_MBOX_TRIGGERINDEX,
				cs40l2x->cp_trailer_index,
				CS40L2X_MBOX_TRIGGERRESET);
		if (ret)
			dev_err(dev, "Cannot set %d index to mailbox\n",
				cs40l2x->cp_trailer_index);
		break;

	case CS40L2X_INDEX_PBQ:
		cs40l2x->pbq_cp_dig_scale = CS40L2X_DIG_SCALE_RESET;

		ret = cs40l2x_cp_dig_scale_get(cs40l2x,
				&cs40l2x->pbq_cp_dig_scale);
		if (ret) {
			dev_err(dev, "Failed to read digital scale\n");
			break;
		}

		cs40l2x->pbq_index = 0;
		cs40l2x->pbq_remain = cs40l2x->pbq_repeat;

		for (i = 0; i < cs40l2x->pbq_depth; i++)
			cs40l2x->pbq_pairs[i].remain =
					cs40l2x->pbq_pairs[i].repeat;

		ret = cs40l2x_pbq_pair_launch(cs40l2x);
		if (ret)
			dev_err(dev, "Failed to launch playback queue\n");
		break;

	case CS40L2X_INDEX_DIAG:
		cs40l2x->diag_state = CS40L2X_DIAG_STATE_INIT;
		cs40l2x->diag_dig_scale = CS40L2X_DIG_SCALE_RESET;

		ret = cs40l2x_dig_scale_get(cs40l2x, &cs40l2x->diag_dig_scale);
		if (ret) {
			dev_err(dev, "Failed to read digital scale\n");
			break;
		}

		ret = cs40l2x_dig_scale_set(cs40l2x, 0);
		if (ret) {
			dev_err(dev, "Failed to reset digital scale\n");
			break;
		}

		ret = regmap_write(regmap,
				cs40l2x_dsp_reg(cs40l2x, "CLOSED_LOOP",
						CS40L2X_XM_UNPACKED_TYPE,
						CS40L2X_ALGO_ID_F0),
				0);
		if (ret) {
			dev_err(dev, "Failed to disable closed-loop mode\n");
			break;
		}

		ret = cs40l2x_diag_enable(cs40l2x, CS40L2X_F0_TRACKING_DIAG);
		if (ret) {
			dev_err(dev, "Failed to enable diagnostics tone\n");
			break;
		}

		msleep(CS40L2X_DIAG_STATE_DELAY_MS);

		ret = regmap_write(regmap,
				cs40l2x_dsp_reg(cs40l2x, "CLOSED_LOOP",
						CS40L2X_XM_UNPACKED_TYPE,
						CS40L2X_ALGO_ID_F0),
				1);
		if (ret) {
			dev_err(dev, "Failed to enable closed-loop mode\n");
			break;
		}

		cs40l2x->diag_state = CS40L2X_DIAG_STATE_RUN1;
		break;

	case CS40L2X_INDEX_QEST:
		cs40l2x->diag_dig_scale = CS40L2X_DIG_SCALE_RESET;

		ret = cs40l2x_dig_scale_get(cs40l2x, &cs40l2x->diag_dig_scale);
		if (ret) {
			dev_err(dev, "Failed to read digital scale\n");
			break;
		}

		ret = cs40l2x_dig_scale_set(cs40l2x, 0);
		if (ret) {
			dev_err(dev, "Failed to reset digital scale\n");
			break;
		}

		ret = cs40l2x_diag_enable(cs40l2x, CS40L2X_F0_TRACKING_QEST);
		if (ret) {
			dev_err(dev, "Failed to enable diagnostics tone\n");
			break;
		}

		cs40l2x->diag_state = CS40L2X_DIAG_STATE_RUN2;
		break;

	default:
		ret = -EINVAL;
		dev_err(dev, "Invalid wavetable index\n");
	}

err_relax:
	if (cs40l2x->vibe_state == CS40L2X_VIBE_STATE_STOPPED)
		goto err_mutex;

	if (ret) {
		cs40l2x_set_state(cs40l2x, CS40L2X_VIBE_STATE_STOPPED);
		if (cs40l2x->vibe_mode != CS40L2X_VIBE_MODE_AUDIO)
			cs40l2x_wl_relax(cs40l2x);
	}

err_mutex:
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);
}

static void cs40l2x_vibe_stop_worker(struct work_struct *work)
{
	struct cs40l2x_private *cs40l2x =
		container_of(work, struct cs40l2x_private, vibe_stop_work);
	struct regmap *regmap = cs40l2x->regmap;
	struct device *dev = cs40l2x->dev;
	int ret;

	pm_runtime_get_sync(cs40l2x->dev);

	if (!cs40l2x->virtual_stored) {
		dev_warn(dev, "Unsafe condition encountered.\n");
		return;
	}

	switch (cs40l2x->cp_trailer_index) {
	case CS40L2X_INDEX_PEAK:
		ret = cs40l2x_peak_capture(cs40l2x);
		if (ret)
			dev_err(dev, "Failed to capture peak-to-peak values\n");

		ret = regmap_write(regmap,
				cs40l2x_dsp_reg(cs40l2x, "GPIO_ENABLE",
						CS40L2X_XM_UNPACKED_TYPE,
						cs40l2x->fw_desc->id),
				cs40l2x->peak_gpio1_enable);
		if (ret)
			dev_err(dev, "Failed to restore GPIO1 configuration\n");
		/* intentionally fall through */

	case CS40L2X_INDEX_VIBE:
	case CS40L2X_INDEX_CONT_MIN ... CS40L2X_INDEX_CONT_MAX:
		ret = cs40l2x_stop_playback(cs40l2x);
		if (ret)
			dev_err(dev, "Failed to stop playback\n");

		if (cs40l2x->event_control & CS40L2X_EVENT_END_ENABLED)
			break;

		if (cs40l2x->vibe_state == CS40L2X_VIBE_STATE_STOPPED)
			break;

		cs40l2x_set_state(cs40l2x, CS40L2X_VIBE_STATE_STOPPED);
		if (cs40l2x->vibe_mode != CS40L2X_VIBE_MODE_AUDIO)
			cs40l2x_wl_relax(cs40l2x);
		break;

	case CS40L2X_INDEX_CLICK_MIN ... CS40L2X_INDEX_CLICK_MAX:
		ret = cs40l2x_stop_playback(cs40l2x);
		if (ret)
			dev_err(dev, "Failed to stop playback\n");
		break;

	case CS40L2X_INDEX_PBQ:
		ret = cs40l2x_pbq_cancel(cs40l2x);
		if (ret)
			dev_err(dev, "Failed to cancel playback queue\n");
		break;

	case CS40L2X_INDEX_DIAG:
	case CS40L2X_INDEX_QEST:
		ret = cs40l2x_diag_capture(cs40l2x);
		if (ret)
			dev_err(dev, "Failed to capture measurement(s): %d\n",
					ret);

		ret = cs40l2x_diag_enable(cs40l2x, CS40L2X_F0_TRACKING_OFF);
		if (ret)
			dev_err(dev, "Failed to disable diagnostics tone\n");

		ret = cs40l2x_dig_scale_set(cs40l2x, cs40l2x->diag_dig_scale);
		if (ret)
			dev_err(dev, "Failed to restore digital scale\n");

		if (cs40l2x->vibe_state == CS40L2X_VIBE_STATE_STOPPED)
			break;

		cs40l2x_set_state(cs40l2x, CS40L2X_VIBE_STATE_STOPPED);
		cs40l2x_wl_relax(cs40l2x);
		break;

	case CS40L2X_INDEX_IDLE:
		break;

	default:
		dev_err(dev, "Invalid wavetable index\n");
	}

	cs40l2x->cp_trailer_index = CS40L2X_INDEX_IDLE;

	cs40l2x_set_safe_save_state(cs40l2x, CS40L2X_SAVE_SAFE);

	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);
}

#ifdef CONFIG_ANDROID_TIMED_OUTPUT
/* vibration callback for timed output device */
static void cs40l2x_vibe_enable(struct timed_output_dev *sdev, int timeout)
{
	struct cs40l2x_private *cs40l2x =
		container_of(sdev, struct cs40l2x_private, timed_dev);

	if (timeout > 0) {
		cs40l2x->vibe_timeout = timeout;
		queue_work(cs40l2x->vibe_workqueue, &cs40l2x->vibe_start_work);
	} else {
		hrtimer_cancel(&cs40l2x->vibe_timer);
		queue_work(cs40l2x->vibe_workqueue, &cs40l2x->vibe_stop_work);
	}
}

static int cs40l2x_vibe_get_time(struct timed_output_dev *sdev)
{
	struct cs40l2x_private *cs40l2x =
		container_of(sdev, struct cs40l2x_private, timed_dev);
	int ret = 0;

	if (hrtimer_active(&cs40l2x->vibe_timer))
		ret = ktime_to_ms(hrtimer_get_remaining(&cs40l2x->vibe_timer));

	return ret;
}

static enum hrtimer_restart cs40l2x_vibe_timer(struct hrtimer *timer)
{
	struct cs40l2x_private *cs40l2x =
		container_of(timer, struct cs40l2x_private, vibe_timer);

	queue_work(cs40l2x->vibe_workqueue, &cs40l2x->vibe_stop_work);

	return HRTIMER_NORESTART;
}

static int cs40l2x_create_timed_output(struct cs40l2x_private *cs40l2x)
{
	int ret;
	struct timed_output_dev *timed_dev = &cs40l2x->timed_dev;
	struct hrtimer *vibe_timer = &cs40l2x->vibe_timer;
	struct device *dev = cs40l2x->dev;

	timed_dev->name = CS40L2X_DEVICE_NAME;
	timed_dev->enable = cs40l2x_vibe_enable;
	timed_dev->get_time = cs40l2x_vibe_get_time;

	ret = timed_output_dev_register(timed_dev);
	if (ret) {
		dev_err(dev, "Failed to register timed output device: %d\n",
			ret);
		return ret;
	}

	hrtimer_init(vibe_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	vibe_timer->function = cs40l2x_vibe_timer;

	ret = sysfs_create_group(&cs40l2x->timed_dev.dev->kobj,
			&cs40l2x_dev_attr_group);
	if (ret) {
		dev_err(dev, "Failed to create sysfs group: %d\n", ret);
		return ret;
	}

	return 0;
}
#else
/* vibration callback for LED device */
static void cs40l2x_vibe_brightness_set(struct led_classdev *led_cdev,
		enum led_brightness brightness)
{
	struct cs40l2x_private *cs40l2x =
		container_of(led_cdev, struct cs40l2x_private, led_dev);

	switch (brightness) {
	case LED_OFF:
		queue_work(cs40l2x->vibe_workqueue, &cs40l2x->vibe_stop_work);
		break;
	default:
		queue_work(cs40l2x->vibe_workqueue, &cs40l2x->vibe_start_work);
	}
}

static int cs40l2x_create_led(struct cs40l2x_private *cs40l2x)
{
	int ret;
	struct led_classdev *led_dev = &cs40l2x->led_dev;
	struct device *dev = cs40l2x->dev;

	led_dev->name = CS40L2X_DEVICE_NAME;
	led_dev->max_brightness = LED_FULL;
	led_dev->brightness_set = cs40l2x_vibe_brightness_set;
	led_dev->default_trigger = "transient";

	ret = led_classdev_register(dev, led_dev);
	if (ret) {
		dev_err(dev, "Failed to register LED device: %d\n", ret);
		return ret;
	}

	ret = sysfs_create_group(&cs40l2x->dev->kobj, &cs40l2x_dev_attr_group);
	if (ret) {
		dev_err(dev, "Failed to create sysfs group: %d\n", ret);
		return ret;
	}

	return 0;
}
#endif /* CONFIG_ANDROID_TIMED_OUTPUT */

static int cs40l2x_coeff_init(struct cs40l2x_private *cs40l2x)
{
	struct regmap *regmap = cs40l2x->regmap;
	struct device *dev = cs40l2x->dev;
	struct cs40l2x_coeff_desc *coeff_desc;
	unsigned int reg = CS40L2X_XM_FW_ID;
	unsigned int val;
	int ret, i;

	ret = regmap_read(regmap, CS40L2X_XM_NUM_ALGOS, &val);
	if (ret) {
		dev_err(dev, "Failed to read number of algorithms\n");
		return ret;
	}

	if (val > CS40L2X_NUM_ALGOS_MAX) {
		dev_err(dev, "Invalid number of algorithms\n");
		return -EINVAL;
	}
	cs40l2x->num_algos = val + 1;

	for (i = 0; i < cs40l2x->num_algos; i++) {
		ret = regmap_read(regmap,
				reg + CS40L2X_ALGO_ID_OFFSET,
				&cs40l2x->algo_info[i].id);
		if (ret) {
			dev_err(dev, "Failed to read algo. %d ID\n", i);
			return ret;
		}

		ret = regmap_read(regmap,
				reg + CS40L2X_ALGO_REV_OFFSET,
				&cs40l2x->algo_info[i].rev);
		if (ret) {
			dev_err(dev, "Failed to read algo. %d revision\n", i);
			return ret;
		}

		ret = regmap_read(regmap,
				reg + CS40L2X_ALGO_XM_BASE_OFFSET,
				&cs40l2x->algo_info[i].xm_base);
		if (ret) {
			dev_err(dev, "Failed to read algo. %d XM_BASE\n", i);
			return ret;
		}

		ret = regmap_read(regmap,
				reg + CS40L2X_ALGO_XM_SIZE_OFFSET,
				&cs40l2x->algo_info[i].xm_size);
		if (ret) {
			dev_err(dev, "Failed to read algo. %d XM_SIZE\n", i);
			return ret;
		}

		ret = regmap_read(regmap,
				reg + CS40L2X_ALGO_YM_BASE_OFFSET,
				&cs40l2x->algo_info[i].ym_base);
		if (ret) {
			dev_err(dev, "Failed to read algo. %d YM_BASE\n", i);
			return ret;
		}

		ret = regmap_read(regmap,
				reg + CS40L2X_ALGO_YM_SIZE_OFFSET,
				&cs40l2x->algo_info[i].ym_size);
		if (ret) {
			dev_err(dev, "Failed to read algo. %d YM_SIZE\n", i);
			return ret;
		}

		list_for_each_entry(coeff_desc,
			&cs40l2x->coeff_desc_head, list) {

			if (coeff_desc->parent_id != cs40l2x->algo_info[i].id)
				continue;

			switch (coeff_desc->block_type) {
			case CS40L2X_XM_UNPACKED_TYPE:
				coeff_desc->reg = CS40L2X_DSP1_XMEM_UNPACK24_0
					+ cs40l2x->algo_info[i].xm_base * 4
					+ coeff_desc->block_offset * 4;
				if (!strncmp(coeff_desc->name, "WAVETABLE",
						CS40L2X_COEFF_NAME_LEN_MAX))
					cs40l2x->wt_limit_xm =
						(cs40l2x->algo_info[i].xm_size
						- coeff_desc->block_offset) * 4;
				break;
			case CS40L2X_YM_UNPACKED_TYPE:
				coeff_desc->reg = CS40L2X_DSP1_YMEM_UNPACK24_0
					+ cs40l2x->algo_info[i].ym_base * 4
					+ coeff_desc->block_offset * 4;
				if (!strncmp(coeff_desc->name, "WAVETABLEYM",
						CS40L2X_COEFF_NAME_LEN_MAX))
					cs40l2x->wt_limit_ym =
						(cs40l2x->algo_info[i].ym_size
						- coeff_desc->block_offset) * 4;
				break;
			}

			dev_dbg(dev, "Found control %s at 0x%08X\n",
				coeff_desc->name, coeff_desc->reg);
		}

		/* system algo. contains one extra register (num. algos.) */
		if (i)
			reg += CS40L2X_ALGO_ENTRY_SIZE;
		else
			reg += (CS40L2X_ALGO_ENTRY_SIZE + 4);
	}

	ret = regmap_read(regmap, reg, &val);
	if (ret) {
		dev_err(dev, "Failed to read list terminator\n");
		return ret;
	}

	if (val != CS40L2X_ALGO_LIST_TERM) {
		dev_err(dev, "Invalid list terminator: 0x%X\n", val);
		return -EINVAL;
	}

	if (cs40l2x->algo_info[0].id != cs40l2x->fw_desc->id) {
		dev_err(dev, "Invalid firmware ID: 0x%06X\n",
				cs40l2x->algo_info[0].id);
		return -EINVAL;
	}

	if (cs40l2x->algo_info[0].rev < cs40l2x->fw_desc->min_rev) {
		dev_err(dev, "Invalid firmware revision: %d.%d.%d\n",
				(cs40l2x->algo_info[0].rev & 0xFF0000) >> 16,
				(cs40l2x->algo_info[0].rev & 0xFF00) >> 8,
				cs40l2x->algo_info[0].rev & 0xFF);
		return -EINVAL;
	}

	if (cs40l2x->algo_info[0].rev >= CS40L2X_COND_CLSH_MIN_REV)
		cs40l2x->cond_class_h_en = true;

	if (cs40l2x->algo_info[0].rev >= CS40L2X_PBQ_DUR_MIN_REV)
		cs40l2x->comp_dur_min_fw = true;

	return 0;
}

static void cs40l2x_coeff_free(struct cs40l2x_private *cs40l2x)
{
	struct cs40l2x_coeff_desc *coeff_desc;

	while (!list_empty(&cs40l2x->coeff_desc_head)) {
		coeff_desc = list_first_entry(&cs40l2x->coeff_desc_head,
				struct cs40l2x_coeff_desc, list);
		list_del(&coeff_desc->list);
		devm_kfree(cs40l2x->dev, coeff_desc);
	}
}

static int cs40l2x_hw_err_rls(struct cs40l2x_private *cs40l2x,
			unsigned int irq_mask)
{
	struct regmap *regmap = cs40l2x->regmap;
	struct device *dev = cs40l2x->dev;
	int ret, i;

	for (i = 0; i < CS40L2X_NUM_HW_ERRS; i++)
		if (cs40l2x_hw_errs[i].irq_mask == irq_mask)
			break;

	if (i == CS40L2X_NUM_HW_ERRS) {
		dev_err(dev, "Unrecognized hardware error\n");
		return -EINVAL;
	}

	if (cs40l2x_hw_errs[i].bst_cycle) {
		ret = regmap_update_bits(regmap, CS40L2X_PWR_CTRL2,
				CS40L2X_BST_EN_MASK,
				CS40L2X_BST_DISABLED << CS40L2X_BST_EN_SHIFT);
		if (ret) {
			dev_err(dev, "Failed to disable boost converter\n");
			return ret;
		}
	}

	ret = regmap_write(regmap, CS40L2X_PROTECT_REL_ERR_IGN, 0);
	if (ret) {
		dev_err(dev, "Failed to cycle error release (step 1 of 3)\n");
		return ret;
	}

	ret = regmap_write(regmap, CS40L2X_PROTECT_REL_ERR_IGN,
			cs40l2x_hw_errs[i].rls_mask);
	if (ret) {
		dev_err(dev, "Failed to cycle error release (step 2 of 3)\n");
		return ret;
	}

	ret = regmap_write(regmap, CS40L2X_PROTECT_REL_ERR_IGN, 0);
	if (ret) {
		dev_err(dev, "Failed to cycle error release (step 3 of 3)\n");
		return ret;
	}

	if (cs40l2x_hw_errs[i].bst_cycle) {
		ret = regmap_update_bits(regmap, CS40L2X_PWR_CTRL2,
				CS40L2X_BST_EN_MASK,
				CS40L2X_BST_ENABLED << CS40L2X_BST_EN_SHIFT);
		if (ret) {
			dev_err(dev, "Failed to re-enable boost converter\n");
			return ret;
		}
	}

	dev_info(dev, "Released %s error\n", cs40l2x_hw_errs[i].err_name);

	return 0;
}

static int cs40l2x_hw_err_chk(struct cs40l2x_private *cs40l2x)
{
	struct regmap *regmap = cs40l2x->regmap;
	struct device *dev = cs40l2x->dev;
	unsigned int val;
	int ret, i;

	ret = regmap_read(regmap, CS40L2X_IRQ2_STATUS1, &val);
	if (ret) {
		dev_err(dev, "Failed to read hardware error status\n");
		return ret;
	}

	for (i = 0; i < CS40L2X_NUM_HW_ERRS; i++) {
		if (!(val & cs40l2x_hw_errs[i].irq_mask))
			continue;

		dev_crit(dev, "Encountered %s error\n",
				cs40l2x_hw_errs[i].err_name);

		ret = regmap_write(regmap, CS40L2X_IRQ2_STATUS1,
				cs40l2x_hw_errs[i].irq_mask);
		if (ret) {
			dev_err(dev, "Failed to acknowledge hardware error\n");
			return ret;
		}

		if (cs40l2x->hw_err_count[i]++ >= CS40L2X_HW_ERR_COUNT_MAX) {
			dev_err(dev, "Aborted %s error release\n",
					cs40l2x_hw_errs[i].err_name);
			continue;
		}

		ret = cs40l2x_hw_err_rls(cs40l2x, cs40l2x_hw_errs[i].irq_mask);
		if (ret)
			return ret;
	}

	return 0;
}

static const struct reg_sequence cs40l2x_irq2_masks[] = {
	{CS40L2X_IRQ2_MASK1,		0xFFFFFFFF},
	{CS40L2X_IRQ2_MASK2,		0xFFFFFFFF},
	{CS40L2X_IRQ2_MASK3,		0xFFFF87FF},
	{CS40L2X_IRQ2_MASK4,		0xFEFFFFFF},
};

static const struct reg_sequence cs40l2x_amp_gnd_setup[] = {
	{CS40L2X_TEST_KEY_CTL,		CS40L2X_TEST_KEY_UNLOCK_CODE1},
	{CS40L2X_TEST_KEY_CTL,		CS40L2X_TEST_KEY_UNLOCK_CODE2},
	{CS40L2X_SPK_FORCE_TST_1,	CS40L2X_FORCE_SPK_GND},
	/* leave test key unlocked to minimize overhead during playback */
};

static const struct reg_sequence cs40l2x_amp_free_setup[] = {
	{CS40L2X_TEST_KEY_CTL,		CS40L2X_TEST_KEY_UNLOCK_CODE1},
	{CS40L2X_TEST_KEY_CTL,		CS40L2X_TEST_KEY_UNLOCK_CODE2},
	{CS40L2X_SPK_FORCE_TST_1,	CS40L2X_FORCE_SPK_FREE},
	{CS40L2X_TEST_KEY_CTL,		CS40L2X_TEST_KEY_RELOCK_CODE1},
	{CS40L2X_TEST_KEY_CTL,		CS40L2X_TEST_KEY_RELOCK_CODE2},
};

static int cs40l2x_dsp_pre_config(struct cs40l2x_private *cs40l2x)
{
	struct regmap *regmap = cs40l2x->regmap;
	struct device *dev = cs40l2x->dev;
	unsigned int gpio_pol = cs40l2x_dsp_reg(cs40l2x, "GPIO_POL",
			CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
	unsigned int spk_auto = cs40l2x_dsp_reg(cs40l2x, "SPK_FORCE_TST_1_AUTO",
			CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
	unsigned int val;
	int ret, i;

	if (cs40l2x->fw_desc->id == CS40L2X_FW_ID_CAL)
		return regmap_multi_reg_write(regmap, cs40l2x_amp_free_setup,
				ARRAY_SIZE(cs40l2x_amp_free_setup));

	ret = regmap_write(regmap,
			cs40l2x_dsp_reg(cs40l2x, "GPIO_BUTTONDETECT",
					CS40L2X_XM_UNPACKED_TYPE,
					cs40l2x->fw_desc->id),
			cs40l2x->gpio_mask);
	if (ret) {
		dev_err(dev, "Failed to enable GPIO detection\n");
		return ret;
	}

	if (gpio_pol) {
		ret = regmap_write(regmap, gpio_pol,
				cs40l2x->pdata.gpio_indv_pol);
		if (ret) {
			dev_err(dev, "Failed to configure GPIO polarity\n");
			return ret;
		}
	} else if (cs40l2x->pdata.gpio_indv_pol) {
		dev_err(dev, "Active-low GPIO not supported\n");
		return -EPERM;
	}

	if (cs40l2x->pdata.gpio1_mode != CS40L2X_GPIO1_MODE_DEF_ON) {
		ret = regmap_write(regmap,
				cs40l2x_dsp_reg(cs40l2x, "GPIO_ENABLE",
						CS40L2X_XM_UNPACKED_TYPE,
						cs40l2x->fw_desc->id),
				CS40L2X_GPIO1_DISABLED);
		if (ret) {
			dev_err(dev, "Failed to pre-configure GPIO1\n");
			return ret;
		}
	}

	if (spk_auto) {
		ret = regmap_write(regmap, spk_auto,
				cs40l2x->pdata.amp_gnd_stby ?
					CS40L2X_FORCE_SPK_GND :
					CS40L2X_FORCE_SPK_FREE);
		if (ret) {
			dev_err(dev, "Failed to configure amplifier clamp\n");
			return ret;
		}
	} else if (cs40l2x->event_control != CS40L2X_EVENT_DISABLED) {
		cs40l2x->amp_gnd_stby = cs40l2x->pdata.amp_gnd_stby;
	}

	if (cs40l2x->amp_gnd_stby) {
		dev_warn(dev, "Enabling legacy amplifier clamp (no GPIO)\n");

		ret = regmap_multi_reg_write(regmap, cs40l2x_amp_gnd_setup,
				ARRAY_SIZE(cs40l2x_amp_gnd_setup));
		if (ret) {
			dev_err(dev, "Failed to ground amplifier outputs\n");
			return ret;
		}

		ret = cs40l2x_wseq_add_seq(cs40l2x, cs40l2x_amp_gnd_setup,
				ARRAY_SIZE(cs40l2x_amp_gnd_setup));
		if (ret) {
			dev_err(dev, "Failed to sequence amplifier outputs\n");
			return ret;
		}
	}

	if (cs40l2x->fw_desc->id != CS40L2X_FW_ID_ORIG) {
		ret = cs40l2x_wseq_init(cs40l2x);
		if (ret) {
			dev_err(dev, "Failed to initialize write sequencer\n");
			return ret;
		}

		ret = regmap_write(regmap,
				cs40l2x_dsp_reg(cs40l2x, "EVENTCONTROL",
						CS40L2X_XM_UNPACKED_TYPE,
						cs40l2x->fw_desc->id),
				cs40l2x->event_control);
		if (ret) {
			dev_err(dev, "Failed to configure event controls\n");
			return ret;
		}

		for (i = 0; i < ARRAY_SIZE(cs40l2x_irq2_masks); i++) {
			/* unmask hardware error interrupts */
			val = cs40l2x_irq2_masks[i].def;
			if (cs40l2x_irq2_masks[i].reg == CS40L2X_IRQ2_MASK1)
				val &= ~cs40l2x->hw_err_mask;

			/* upper half */
			ret = regmap_write(regmap,
					cs40l2x_dsp_reg(cs40l2x,
						"IRQMASKSEQUENCE",
						CS40L2X_XM_UNPACKED_TYPE,
						cs40l2x->fw_desc->id)
						+ i * CS40L2X_IRQMASKSEQ_STRIDE,
					(val & CS40L2X_IRQMASKSEQ_MASK1)
						<< CS40L2X_IRQMASKSEQ_SHIFTUP);
			if (ret) {
				dev_err(dev,
					"Failed to write IRQMASKSEQ (upper)\n");
				return ret;
			}

			/* lower half */
			ret = regmap_write(regmap,
					cs40l2x_dsp_reg(cs40l2x,
						"IRQMASKSEQUENCE",
						CS40L2X_XM_UNPACKED_TYPE,
						cs40l2x->fw_desc->id) + 4
						+ i * CS40L2X_IRQMASKSEQ_STRIDE,
					(val & CS40L2X_IRQMASKSEQ_MASK2)
						>> CS40L2X_IRQMASKSEQ_SHIFTDN);
			if (ret) {
				dev_err(dev,
					"Failed to write IRQMASKSEQ (lower)\n");
				return ret;
			}
		}

		ret = regmap_write(regmap,
				cs40l2x_dsp_reg(cs40l2x,
						"IRQMASKSEQUENCE_VALID",
						CS40L2X_XM_UNPACKED_TYPE,
						cs40l2x->fw_desc->id),
				1);
		if (ret) {
			dev_err(dev, "Failed to enable IRQMASKSEQ\n");
			return ret;
		}
	}

	return 0;
}

static const struct reg_sequence cs40l2x_dsp_errata[] = {
	{CS40L2X_DSP1_XM_ACCEL_PL0_PRI,	0x00000000},
	{CS40L2X_DSP1_YM_ACCEL_PL0_PRI,	0x00000000},
	{CS40L2X_DSP1_RX1_RATE,		0x00000001},
	{CS40L2X_DSP1_RX2_RATE,		0x00000001},
	{CS40L2X_DSP1_RX3_RATE,		0x00000001},
	{CS40L2X_DSP1_RX4_RATE,		0x00000001},
	{CS40L2X_DSP1_RX5_RATE,		0x00000001},
	{CS40L2X_DSP1_RX6_RATE,		0x00000001},
	{CS40L2X_DSP1_RX7_RATE,		0x00000001},
	{CS40L2X_DSP1_RX8_RATE,		0x00000001},
	{CS40L2X_DSP1_TX1_RATE,		0x00000001},
	{CS40L2X_DSP1_TX2_RATE,		0x00000001},
	{CS40L2X_DSP1_TX3_RATE,		0x00000001},
	{CS40L2X_DSP1_TX4_RATE,		0x00000001},
	{CS40L2X_DSP1_TX5_RATE,		0x00000001},
	{CS40L2X_DSP1_TX6_RATE,		0x00000001},
	{CS40L2X_DSP1_TX7_RATE,		0x00000001},
	{CS40L2X_DSP1_TX8_RATE,		0x00000001},
};

static int cs40l2x_dsp_start(struct cs40l2x_private *cs40l2x)
{
	struct regmap *regmap = cs40l2x->regmap;
	struct device *dev = cs40l2x->dev;
	unsigned int dsp_status, dsp_scratch;
	int dsp_timeout = CS40L2X_DSP_TIMEOUT_COUNT;
	int ret;

	ret = regmap_multi_reg_write(regmap, cs40l2x_dsp_errata,
			ARRAY_SIZE(cs40l2x_dsp_errata));
	if (ret) {
		dev_err(dev, "Failed to apply DSP-specific errata\n");
		return ret;
	}

	switch (cs40l2x->revid) {
	case CS40L2X_REVID_A0:
		ret = regmap_update_bits(regmap, CS40L2X_PWR_CTRL1,
				CS40L2X_GLOBAL_EN_MASK,
				1 << CS40L2X_GLOBAL_EN_SHIFT);
		if (ret) {
			dev_err(dev, "Failed to enable device\n");
			return ret;
		}
		break;

	default:
		ret = regmap_update_bits(regmap, CS40L2X_PWRMGT_CTL,
				CS40L2X_MEM_RDY_MASK,
				1 << CS40L2X_MEM_RDY_SHIFT);
		if (ret) {
			dev_err(dev, "Failed to set memory ready flag\n");
			return ret;
		}
	}

	ret = regmap_update_bits(regmap, CS40L2X_DSP1_CCM_CORE_CTRL,
			CS40L2X_DSP1_RESET_MASK | CS40L2X_DSP1_EN_MASK,
			(1 << CS40L2X_DSP1_RESET_SHIFT) |
				(1 << CS40L2X_DSP1_EN_SHIFT));
	if (ret) {
		dev_err(dev, "Failed to start DSP\n");
		return ret;
	}

	while (dsp_timeout > 0) {
		usleep_range(10000, 10100);

		ret = regmap_read(regmap,
				cs40l2x_dsp_reg(cs40l2x, "HALO_STATE",
						CS40L2X_XM_UNPACKED_TYPE,
						cs40l2x->fw_desc->id),
				&dsp_status);
		if (ret) {
			dev_err(dev, "Failed to read DSP status\n");
			return ret;
		}

		if (dsp_status == cs40l2x->fw_desc->halo_state_run)
			break;

		dsp_timeout--;
	}

	ret = regmap_read(regmap, CS40L2X_DSP1_SCRATCH1, &dsp_scratch);
	if (ret) {
		dev_err(dev, "Failed to read DSP scratch contents\n");
		return ret;
	}

	if (dsp_timeout == 0 || dsp_scratch != 0) {
		dev_err(dev, "Timed out with DSP status, scratch = %u, %u\n",
				dsp_status, dsp_scratch);
		return -ETIME;
	}

	cs40l2x->dsp_reg = cs40l2x_dsp_reg;

	return 0;
}

static int cs40l2x_dsp_post_config(struct cs40l2x_private *cs40l2x)
{
	struct regmap *regmap = cs40l2x->regmap;
	struct device *dev = cs40l2x->dev;
	int ret;

	if (cs40l2x->fw_desc->id == CS40L2X_FW_ID_CAL)
		return 0;

	ret = regmap_write(regmap,
			cs40l2x_dsp_reg(cs40l2x, "TIMEOUT_MS",
					CS40L2X_XM_UNPACKED_TYPE,
					CS40L2X_ALGO_ID_VIBE),
			CS40L2X_TIMEOUT_MS_MAX);
	if (ret) {
		dev_err(dev, "Failed to extend playback timeout\n");
		return ret;
	}

	ret = cs40l2x_wavetable_sync(cs40l2x);
	if (ret)
		return ret;

	ret = cs40l2x_imon_offs_sync(cs40l2x);
	if (ret)
		return ret;

	switch (cs40l2x->fw_desc->id) {
	case CS40L2X_FW_ID_ORIG:
		ret = regmap_write(regmap,
				cs40l2x_dsp_reg(cs40l2x, "COMPENSATION_ENABLE",
						CS40L2X_XM_UNPACKED_TYPE,
						CS40L2X_ALGO_ID_VIBE),
				cs40l2x->comp_enable);
		break;
	default:
		ret = regmap_write(regmap,
				cs40l2x_dsp_reg(cs40l2x, "COMPENSATION_ENABLE",
						CS40L2X_XM_UNPACKED_TYPE,
						CS40L2X_ALGO_ID_VIBE),
				(cs40l2x->comp_enable
					& cs40l2x->comp_enable_redc)
					<< CS40L2X_COMP_EN_REDC_SHIFT |
				(cs40l2x->comp_enable
					& cs40l2x->comp_enable_f0)
					<< CS40L2X_COMP_EN_F0_SHIFT);
	}

	if (ret) {
		dev_err(dev, "Failed to configure click compensation\n");
		return ret;
	}

	if (cs40l2x->pdata.f0_default) {
		ret = regmap_write(regmap,
				cs40l2x_dsp_reg(cs40l2x, "F0_STORED",
						CS40L2X_XM_UNPACKED_TYPE,
						cs40l2x->fw_desc->id ==
							CS40L2X_FW_ID_ORIG ?
							CS40L2X_ALGO_ID_F0 :
							cs40l2x->fw_desc->id),
				cs40l2x->pdata.f0_default);
		if (ret) {
			dev_err(dev, "Failed to write default f0\n");
			return ret;
		}
	}

	if (cs40l2x->pdata.redc_default) {
		ret = regmap_write(regmap,
				cs40l2x_dsp_reg(cs40l2x, "REDC_STORED",
						CS40L2X_XM_UNPACKED_TYPE,
						cs40l2x->fw_desc->id ==
							CS40L2X_FW_ID_ORIG ?
							CS40L2X_ALGO_ID_F0 :
							cs40l2x->fw_desc->id),
				cs40l2x->pdata.redc_default);
		if (ret) {
			dev_err(dev, "Failed to write default ReDC\n");
			return ret;
		}
	}

	if (cs40l2x->pdata.q_default
			&& cs40l2x->fw_desc->id != CS40L2X_FW_ID_ORIG) {
		ret = regmap_write(regmap,
				cs40l2x_dsp_reg(cs40l2x, "Q_STORED",
						CS40L2X_XM_UNPACKED_TYPE,
						cs40l2x->fw_desc->id),
				cs40l2x->pdata.q_default);
		if (ret) {
			dev_err(dev, "Failed to write default Q\n");
			return ret;
		}
	}

	if (cs40l2x->pdata.gpio1_rise_index > 0
			&& cs40l2x->pdata.gpio1_rise_index < cs40l2x->num_waves
			&& cs40l2x->gpio_mask & CS40L2X_GPIO_BTNDETECT_GPIO1) {
		ret = cs40l2x_gpio_edge_index_set(cs40l2x,
				cs40l2x->pdata.gpio1_rise_index,
				CS40L2X_INDEXBUTTONPRESS1, CS40L2X_GPIO_RISE);
		if (ret) {
			dev_err(dev,
				"Failed to write default gpio1_rise_index\n");
			return ret;
		}
	} else if ((cs40l2x->pdata.gpio1_rise_index >= cs40l2x->num_waves
			&& cs40l2x->gpio_mask & CS40L2X_GPIO_BTNDETECT_GPIO1)
			|| (cs40l2x->pdata.gpio1_rise_index > 0
				&& !(cs40l2x->gpio_mask
					& CS40L2X_GPIO_BTNDETECT_GPIO1))) {
		dev_warn(dev, "Ignored default gpio1_rise_index\n");
	}

	if (cs40l2x->pdata.gpio1_fall_index > 0
			&& cs40l2x->pdata.gpio1_fall_index < cs40l2x->num_waves
			&& cs40l2x->gpio_mask & CS40L2X_GPIO_BTNDETECT_GPIO1) {
		ret = cs40l2x_gpio_edge_index_set(cs40l2x,
				cs40l2x->pdata.gpio1_fall_index,
				CS40L2X_INDEXBUTTONRELEASE1, CS40L2X_GPIO_FALL);
		if (ret) {
			dev_err(dev,
				"Failed to write default gpio1_fall_index\n");
			return ret;
		}
	} else if ((cs40l2x->pdata.gpio1_fall_index >= cs40l2x->num_waves
			&& cs40l2x->gpio_mask & CS40L2X_GPIO_BTNDETECT_GPIO1)
			|| (cs40l2x->pdata.gpio1_fall_index > 0
				&& !(cs40l2x->gpio_mask
					& CS40L2X_GPIO_BTNDETECT_GPIO1))) {
		dev_warn(dev, "Ignored default gpio1_fall_index\n");
	}

	if (cs40l2x->pdata.gpio1_fall_timeout > 0
			&& (cs40l2x->pdata.gpio1_fall_timeout
				& CS40L2X_PDATA_MASK)
					<= CS40L2X_PR_TIMEOUT_MAX) {
		ret = regmap_write(regmap,
				cs40l2x_dsp_reg(cs40l2x,
						"PRESS_RELEASE_TIMEOUT",
						CS40L2X_XM_UNPACKED_TYPE,
						cs40l2x->fw_desc->id),
				cs40l2x->pdata.gpio1_fall_timeout
					& CS40L2X_PDATA_MASK);
		if (ret) {
			dev_err(dev,
				"Failed to write default gpio1_fall_timeout\n");
			return ret;
		}
	} else if ((cs40l2x->pdata.gpio1_fall_timeout
			& CS40L2X_PDATA_MASK) > CS40L2X_PR_TIMEOUT_MAX) {
		dev_warn(dev, "Ignored default gpio1_fall_timeout\n");
	}

	if (cs40l2x->pdata.gpio2_rise_index > 0
			&& cs40l2x->pdata.gpio2_rise_index < cs40l2x->num_waves
			&& cs40l2x->gpio_mask & CS40L2X_GPIO_BTNDETECT_GPIO2) {
		ret = cs40l2x_gpio_edge_index_set(cs40l2x,
				cs40l2x->pdata.gpio2_rise_index,
				CS40L2X_INDEXBUTTONPRESS2, CS40L2X_GPIO_RISE);
		if (ret) {
			dev_err(dev,
				"Failed to write default gpio2_rise_index\n");
			return ret;
		}
	} else if ((cs40l2x->pdata.gpio2_rise_index >= cs40l2x->num_waves
			&& cs40l2x->gpio_mask & CS40L2X_GPIO_BTNDETECT_GPIO2)
			|| (cs40l2x->pdata.gpio2_rise_index > 0
				&& !(cs40l2x->gpio_mask
					& CS40L2X_GPIO_BTNDETECT_GPIO2))) {
		dev_warn(dev, "Ignored default gpio2_rise_index\n");
	}

	if (cs40l2x->pdata.gpio2_fall_index > 0
			&& cs40l2x->pdata.gpio2_fall_index < cs40l2x->num_waves
			&& cs40l2x->gpio_mask & CS40L2X_GPIO_BTNDETECT_GPIO2) {
		ret = cs40l2x_gpio_edge_index_set(cs40l2x,
				cs40l2x->pdata.gpio2_fall_index,
				CS40L2X_INDEXBUTTONRELEASE2, CS40L2X_GPIO_FALL);
		if (ret) {
			dev_err(dev,
				"Failed to write default gpio2_fall_index\n");
			return ret;
		}
	} else if ((cs40l2x->pdata.gpio2_fall_index >= cs40l2x->num_waves
			&& cs40l2x->gpio_mask & CS40L2X_GPIO_BTNDETECT_GPIO2)
			|| (cs40l2x->pdata.gpio2_fall_index > 0
				&& !(cs40l2x->gpio_mask
					& CS40L2X_GPIO_BTNDETECT_GPIO2))) {
		dev_warn(dev, "Ignored default gpio2_fall_index\n");
	}

	if (cs40l2x->pdata.gpio3_rise_index > 0
			&& cs40l2x->pdata.gpio3_rise_index < cs40l2x->num_waves
			&& cs40l2x->gpio_mask & CS40L2X_GPIO_BTNDETECT_GPIO3) {
		ret = cs40l2x_gpio_edge_index_set(cs40l2x,
				cs40l2x->pdata.gpio3_rise_index,
				CS40L2X_INDEXBUTTONPRESS3, CS40L2X_GPIO_RISE);
		if (ret) {
			dev_err(dev,
				"Failed to write default gpio3_rise_index\n");
			return ret;
		}
	} else if ((cs40l2x->pdata.gpio3_rise_index >= cs40l2x->num_waves
			&& cs40l2x->gpio_mask & CS40L2X_GPIO_BTNDETECT_GPIO3)
			|| (cs40l2x->pdata.gpio3_rise_index > 0
				&& !(cs40l2x->gpio_mask
					& CS40L2X_GPIO_BTNDETECT_GPIO3))) {
		dev_warn(dev, "Ignored default gpio3_rise_index\n");
	}

	if (cs40l2x->pdata.gpio3_fall_index > 0
			&& cs40l2x->pdata.gpio3_fall_index < cs40l2x->num_waves
			&& cs40l2x->gpio_mask & CS40L2X_GPIO_BTNDETECT_GPIO3) {
		ret = cs40l2x_gpio_edge_index_set(cs40l2x,
				cs40l2x->pdata.gpio3_fall_index,
				CS40L2X_INDEXBUTTONRELEASE3, CS40L2X_GPIO_FALL);
		if (ret) {
			dev_err(dev,
				"Failed to write default gpio3_fall_index\n");
			return ret;
		}
	} else if ((cs40l2x->pdata.gpio3_fall_index >= cs40l2x->num_waves
			&& cs40l2x->gpio_mask & CS40L2X_GPIO_BTNDETECT_GPIO3)
			|| (cs40l2x->pdata.gpio3_fall_index > 0
				&& !(cs40l2x->gpio_mask
					& CS40L2X_GPIO_BTNDETECT_GPIO3))) {
		dev_warn(dev, "Ignored default gpio3_fall_index\n");
	}

	if (cs40l2x->pdata.gpio4_rise_index > 0
			&& cs40l2x->pdata.gpio4_rise_index < cs40l2x->num_waves
			&& cs40l2x->gpio_mask & CS40L2X_GPIO_BTNDETECT_GPIO4) {
		ret = cs40l2x_gpio_edge_index_set(cs40l2x,
				cs40l2x->pdata.gpio4_rise_index,
				CS40L2X_INDEXBUTTONPRESS4, CS40L2X_GPIO_RISE);
		if (ret) {
			dev_err(dev,
				"Failed to write default gpio4_rise_index\n");
			return ret;
		}
	} else if ((cs40l2x->pdata.gpio4_rise_index >= cs40l2x->num_waves
			&& cs40l2x->gpio_mask & CS40L2X_GPIO_BTNDETECT_GPIO4)
			|| (cs40l2x->pdata.gpio4_rise_index > 0
				&& !(cs40l2x->gpio_mask
					& CS40L2X_GPIO_BTNDETECT_GPIO4))) {
		dev_warn(dev, "Ignored default gpio4_rise_index\n");
	}

	if (cs40l2x->pdata.gpio4_fall_index > 0
			&& cs40l2x->pdata.gpio4_fall_index < cs40l2x->num_waves
			&& cs40l2x->gpio_mask & CS40L2X_GPIO_BTNDETECT_GPIO4) {
		ret = cs40l2x_gpio_edge_index_set(cs40l2x,
				cs40l2x->pdata.gpio4_fall_index,
				CS40L2X_INDEXBUTTONRELEASE4, CS40L2X_GPIO_FALL);
		if (ret) {
			dev_err(dev,
				"Failed to write default gpio4_fall_index\n");
			return ret;
		}
	} else if ((cs40l2x->pdata.gpio4_fall_index >= cs40l2x->num_waves
			&& cs40l2x->gpio_mask & CS40L2X_GPIO_BTNDETECT_GPIO4)
			|| (cs40l2x->pdata.gpio4_fall_index > 0
				&& !(cs40l2x->gpio_mask
					& CS40L2X_GPIO_BTNDETECT_GPIO4))) {
		dev_warn(dev, "Ignored default gpio4_fall_index\n");
	}

	return cs40l2x_hw_err_chk(cs40l2x);
}

static int cs40l2x_raw_write(struct cs40l2x_private *cs40l2x, unsigned int reg,
			const void *val, size_t val_len, size_t limit)
{
	int ret = 0;
	int i;

	/* split "val" into smaller writes not to exceed "limit" in length */
	for (i = 0; i < val_len; i += limit) {
		ret = regmap_raw_write(cs40l2x->regmap, (reg + i), (val + i),
				(val_len - i) > limit ? limit : (val_len - i));
		if (ret)
			break;
	}

	return ret;
}

int cs40l2x_ack_write(struct cs40l2x_private *cs40l2x, unsigned int reg,
			unsigned int write_val, unsigned int reset_val)
{
	struct regmap *regmap = cs40l2x->regmap;
	struct device *dev = cs40l2x->dev;
	unsigned int val;
	int ret, i;

	ret = regmap_write(regmap, reg, write_val);
	if (ret) {
		dev_err(dev, "Failed to write register 0x%08X: %d\n", reg, ret);
		return ret;
	}

	for (i = 0; i < CS40L2X_ACK_TIMEOUT_COUNT; i++) {
		usleep_range(1000, 1100);

		ret = regmap_read(regmap, reg, &val);
		if (ret) {
			dev_err(dev, "Failed to read register 0x%08X: %d\n",
					reg, ret);
			return ret;
		}

		if (val == reset_val)
			return 0;
	}

	dev_err(dev, "Timed out with register 0x%08X = 0x%08X\n", reg, val);

	return -ETIME;
}
EXPORT_SYMBOL_GPL(cs40l2x_ack_write);

static void cs40l2x_set_xm(struct cs40l2x_private *cs40l2x, unsigned int pos,
			unsigned int reg, unsigned int block_length,
			unsigned int size)
{
	cs40l2x->wt_open_xm = (cs40l2x->wt_limit_xm - block_length);
	/* Set YM available space in case no YM block defined in bin file. */
	cs40l2x->wt_open_ym = cs40l2x->wt_limit_ym;
	cs40l2x->xm_hdr_strt_pos = pos;
	cs40l2x->xm_hdr_strt_reg = reg;
	cs40l2x->wt_xm_size = pos + block_length;
	cs40l2x->wt_total_size = size;
}

static void cs40l2x_set_ym(struct cs40l2x_private *cs40l2x, unsigned int pos,
			unsigned int reg, unsigned int block_length)
{
	cs40l2x->wt_open_ym = (cs40l2x->wt_limit_ym - block_length);
	cs40l2x->ym_hdr_strt_pos = pos;
	cs40l2x->ym_hdr_strt_reg = reg;
	cs40l2x->wt_ym_size = ((pos - cs40l2x->wt_xm_size) + block_length);
}

int cs40l2x_coeff_file_parse(struct cs40l2x_private *cs40l2x,
			const struct firmware *fw)
{
	struct device *dev = cs40l2x->dev;
	char wt_date[CS40L2X_WT_FILE_DATE_LEN_MAX];
	bool wt_found = false;
	unsigned int pos = CS40L2X_WT_FILE_HEADER_SIZE;
	unsigned int block_offset, block_type, block_length;
	unsigned int algo_id, algo_rev;
	unsigned int reg = 0;
	unsigned int is_xm;
	int ret = -EINVAL;
	int i = 0, wt_index = 0;

	*wt_date = '\0';

	if (memcmp(fw->data, "WMDR", 4)) {
		dev_err(dev, "Failed to recognize coefficient file\n");
		goto err_rls_fw;
	}

	if (fw->size % 4) {
		dev_err(dev, "Coefficient file is not word-aligned\n");
		goto err_rls_fw;
	}

	while (pos < fw->size) {
		block_offset = fw->data[pos]
				+ (fw->data[pos + 1] << 8);
		pos += CS40L2X_WT_DBLK_OFFSET_SIZE;

		block_type = fw->data[pos]
				+ (fw->data[pos + 1] << 8);
		pos += CS40L2X_WT_DBLK_TYPE_SIZE;

		algo_id = fw->data[pos]
				+ (fw->data[pos + 1] << 8)
				+ (fw->data[pos + 2] << 16)
				+ (fw->data[pos + 3] << 24);
		pos += CS40L2X_WT_ALGO_ID_SIZE;

		algo_rev = fw->data[pos]
				+ (fw->data[pos + 1] << 8)
				+ (fw->data[pos + 2] << 16)
				+ (fw->data[pos + 3] << 24);
		pos += CS40L2X_WT_ALGO_REV_SIZE;

		/* sample rate is not used here */
		pos += CS40L2X_WT_SAMPLE_RATE_SIZE;

		block_length = fw->data[pos]
				+ (fw->data[pos + 1] << 8)
				+ (fw->data[pos + 2] << 16)
				+ (fw->data[pos + 3] << 24);
		pos += CS40L2X_WT_DBLK_LENGTH_SIZE;

		if (block_type != CS40L2X_WMDR_NAME_TYPE
				&& block_type != CS40L2X_WMDR_INFO_TYPE) {
			for (i = 0; i < cs40l2x->num_algos; i++) {
				if (algo_id == cs40l2x->algo_info[i].id)
					break;
			}
			if (i == cs40l2x->num_algos) {
				dev_err(dev, "Invalid algo. ID: 0x%06X\n",
						algo_id);
				ret = -EINVAL;
				goto err_rls_fw;
			} else {
				dev_dbg(dev, "Valid algo ID 0x%x\n", algo_id);
			}

			if (((algo_rev >> 8) & CS40L2X_ALGO_REV_MASK)
					!= (cs40l2x->algo_info[i].rev
						& CS40L2X_ALGO_REV_MASK)) {
				dev_err(dev, "Invalid algo. rev.: %d.%d.%d\n",
						(algo_rev & 0xFF000000) >> 24,
						(algo_rev & 0xFF0000) >> 16,
						(algo_rev & 0xFF00) >> 8);
				ret = -EINVAL;
				goto err_rls_fw;
			}

			switch (algo_id) {
			case CS40L2X_ALGO_ID_EXC:
				cs40l2x->exc_available = true;
				break;
			case CS40L2X_ALGO_ID_VIBE:
				wt_found = true;
				/* intentionally fall through */
			}
		}

		switch (block_type) {
		case CS40L2X_WMDR_NAME_TYPE:
		case CS40L2X_WMDR_INFO_TYPE:
			reg = 0;

			if (block_length < CS40L2X_WT_FILE_DATE_LEN_MAX)
				break;

			if (memcmp(&fw->data[pos], "Date: ", 6))
				break;

			memcpy(wt_date, &fw->data[pos + 6],
					CS40L2X_WT_FILE_DATE_LEN_MAX - 6);
			wt_date[CS40L2X_WT_FILE_DATE_LEN_MAX - 6] = '\0';
			break;
		case CS40L2X_XM_UNPACKED_TYPE:
			reg = CS40L2X_DSP1_XMEM_UNPACK24_0
				+ block_offset
				+ cs40l2x->algo_info[i].xm_base * 4;

			if (reg == cs40l2x_dsp_reg(cs40l2x,
						"WAVETABLE",
						CS40L2X_XM_UNPACKED_TYPE,
						CS40L2X_ALGO_ID_VIBE)) {
				if (block_length > cs40l2x->wt_limit_xm) {
					dev_err(dev,
						"Wvtbl too big: %d bytes XM\n",
						block_length / 4 * 3);
					ret = -EINVAL;
					goto err_rls_fw;
				} else {
					if (wt_found) {
						cs40l2x_set_xm(cs40l2x, pos,
							reg, block_length,
							fw->size);
						memcpy(cs40l2x->pbq_fw_raw_wt,
							&fw->data[0],
							fw->size);
					}
				}
			}
			if (wt_found && cs40l2x->cond_class_h_en) {
				ret = cs40l2x_classh_wt_check(cs40l2x,
						&fw->data[pos],
						block_length, &wt_index);
				if (ret)
					goto err_rls_fw;
			}

			break;
		case CS40L2X_YM_UNPACKED_TYPE:
			reg = CS40L2X_DSP1_YMEM_UNPACK24_0 + block_offset
					+ cs40l2x->algo_info[i].ym_base * 4;

			if (reg == cs40l2x_dsp_reg(cs40l2x,
						"WAVETABLEYM",
						CS40L2X_YM_UNPACKED_TYPE,
						CS40L2X_ALGO_ID_VIBE)) {
				if (block_length > cs40l2x->wt_limit_ym) {
					dev_err(dev,
						"Wvtbl too big: %d bytes YM\n",
						block_length / 4 * 3);
					ret = -EINVAL;
					goto err_rls_fw;
				} else {
					if (wt_found) {
						cs40l2x_set_ym(cs40l2x, pos,
							reg, block_length);
					}
				}
			}
			if (wt_found && cs40l2x->cond_class_h_en) {
				ret = cs40l2x_classh_wt_check(cs40l2x,
						&fw->data[pos],
						block_length, &wt_index);
				if (ret)
					goto err_rls_fw;
			}

			break;
		case CS40L2X_XM_PACKED_TYPE:
			reg = (CS40L2X_DSP1_XMEM_PACK_0 + block_offset
				+ cs40l2x->algo_info[i].xm_base * 3) & ~0x3;
			break;
		case CS40L2X_YM_PACKED_TYPE:
			reg = (CS40L2X_DSP1_YMEM_PACK_0 + block_offset
				+ cs40l2x->algo_info[i].ym_base * 3) & ~0x3;
			break;
		default:
			dev_err(dev, "Unexpected block type: 0x%04X\n",
					block_type);
			ret = -EINVAL;
			goto err_rls_fw;
		}

		if (reg) {
			ret = cs40l2x_raw_write(cs40l2x, reg, &fw->data[pos],
					block_length, CS40L2X_MAX_WLEN);
			if (ret) {
				dev_err(dev, "Failed to write coefficients\n");
				goto err_rls_fw;
			}
		} else {
			ret = 0;
		}

		/* blocks are word-aligned */
		pos += (block_length + 3) & ~0x00000003;
	}

	if (wt_found) {
		if (!strncmp(cs40l2x->wt_file,
				CS40L2X_WT_FILE_NAME_MISSING,
				CS40L2X_WT_FILE_NAME_LEN_MAX))
			strlcpy(cs40l2x->wt_file,
					CS40L2X_WT_FILE_NAME_DEFAULT,
					CS40L2X_WT_FILE_NAME_LEN_MAX);

		if (*wt_date != '\0')
			strlcpy(cs40l2x->wt_date, wt_date,
					CS40L2X_WT_FILE_DATE_LEN_MAX);
		else
			strlcpy(cs40l2x->wt_date,
					CS40L2X_WT_FILE_DATE_MISSING,
					CS40L2X_WT_FILE_DATE_LEN_MAX);

		if (cs40l2x->open_wt_enable) {
			ret = cs40l2x_add_wt_slots(cs40l2x, &is_xm);
			if (ret) {
				dev_err(dev, "Unable to add open slots, open wt disabled\n");
				goto err_rls_fw;
			}
			/* Write updated block with virtual slots */
			if (is_xm)
				reg = cs40l2x_dsp_reg(cs40l2x,
						"WAVETABLE",
						CS40L2X_XM_UNPACKED_TYPE,
						CS40L2X_ALGO_ID_VIBE);
			else
				reg = cs40l2x_dsp_reg(cs40l2x,
						"WAVETABLEYM",
						CS40L2X_YM_UNPACKED_TYPE,
						CS40L2X_ALGO_ID_VIBE);
			ret = cs40l2x_raw_write(cs40l2x, reg,
					&cs40l2x->pbq_updated_fw_raw_wt[0],
					cs40l2x->updated_block_size,
					CS40L2X_MAX_WLEN);
			if (ret) {
				dev_err(dev, "Failed to write coefficients\n");
				goto err_rls_fw;
			}
			cs40l2x->virtual_bin = true;
		}
	}

err_rls_fw:
	release_firmware(fw);

	return ret;
}
EXPORT_SYMBOL_GPL(cs40l2x_coeff_file_parse);

static void cs40l2x_coeff_file_load(const struct firmware *fw, void *context)
{
	struct cs40l2x_private *cs40l2x = (struct cs40l2x_private *)context;
	struct device *dev = cs40l2x->dev;
	unsigned int num_coeff_files = 0;
	unsigned int total_coeff_files = cs40l2x->fw_desc->num_coeff_files;
	int ret = 0;

	mutex_lock(&cs40l2x->lock);

	if (fw)
		ret = cs40l2x_coeff_file_parse(cs40l2x, fw);

	if (!ret)
		num_coeff_files = ++(cs40l2x->num_coeff_files);

	if (!cs40l2x->dyn_f0_enable && !cs40l2x->par_bin_found &&
		!cs40l2x->clab_bin_found)
		total_coeff_files = (cs40l2x->fw_desc->num_coeff_files - 1);

	if (num_coeff_files != total_coeff_files)
		goto err_mutex;

	ret = cs40l2x_dsp_pre_config(cs40l2x);
	if (ret)
		goto err_mutex;

	ret = cs40l2x_dsp_start(cs40l2x);
	if (ret)
		goto err_mutex;

	ret = cs40l2x_dsp_post_config(cs40l2x);
	if (ret)
		goto err_mutex;

#ifdef CONFIG_ANDROID_TIMED_OUTPUT
	ret = cs40l2x_create_timed_output(cs40l2x);
#else
	ret = cs40l2x_create_led(cs40l2x);
#endif /* CONFIG_ANDROID_TIMED_OUTPUT */
	if (ret)
		goto err_mutex;

	cs40l2x->vibe_init_success = true;

	dev_info(cs40l2x->dev, "Firmware revision %d.%d.%d\n",
			(cs40l2x->algo_info[0].rev & 0xFF0000) >> 16,
			(cs40l2x->algo_info[0].rev & 0xFF00) >> 8,
			cs40l2x->algo_info[0].rev & 0xFF);

	dev_info(cs40l2x->dev,
			"Max. wavetable size: %d bytes (XM), %d bytes (YM)\n",
			cs40l2x->wt_limit_xm / 4 * 3,
			cs40l2x->wt_limit_ym / 4 * 3);

	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_set_active(dev);
	pm_runtime_enable(dev);
	pm_runtime_set_autosuspend_delay(dev, cs40l2x->autosuspend_delay);
	pm_runtime_use_autosuspend(dev);

	return;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
}

static int cs40l2x_algo_parse(struct cs40l2x_private *cs40l2x,
		const unsigned char *data)
{
	struct cs40l2x_coeff_desc *coeff_desc;
	unsigned int pos = 0;
	unsigned int algo_id, algo_desc_length, coeff_count;
	unsigned int block_offset, block_type, block_length;
	unsigned char algo_name_length;
	int i;

	/* record algorithm ID */
	algo_id = *(data + pos)
			+ (*(data + pos + 1) << 8)
			+ (*(data + pos + 2) << 16)
			+ (*(data + pos + 3) << 24);
	pos += CS40L2X_ALGO_ID_SIZE;

	/* skip past algorithm name */
	algo_name_length = *(data + pos);
	pos += ((algo_name_length / 4) * 4) + 4;

	/* skip past algorithm description */
	algo_desc_length = *(data + pos)
			+ (*(data + pos + 1) << 8);
	pos += ((algo_desc_length / 4) * 4) + 4;

	/* record coefficient count */
	coeff_count = *(data + pos)
			+ (*(data + pos + 1) << 8)
			+ (*(data + pos + 2) << 16)
			+ (*(data + pos + 3) << 24);
	pos += CS40L2X_COEFF_COUNT_SIZE;

	for (i = 0; i < coeff_count; i++) {
		block_offset = *(data + pos)
				+ (*(data + pos + 1) << 8);
		pos += CS40L2X_COEFF_OFFSET_SIZE;

		block_type = *(data + pos)
				+ (*(data + pos + 1) << 8);
		pos += CS40L2X_COEFF_TYPE_SIZE;

		block_length = *(data + pos)
				+ (*(data + pos + 1) << 8)
				+ (*(data + pos + 2) << 16)
				+ (*(data + pos + 3) << 24);
		pos += CS40L2X_COEFF_LENGTH_SIZE;

		coeff_desc = devm_kzalloc(cs40l2x->dev,
				sizeof(*coeff_desc), GFP_KERNEL);
		if (!coeff_desc)
			return -ENOMEM;

		coeff_desc->parent_id = algo_id;
		coeff_desc->block_offset = block_offset;
		coeff_desc->block_type = block_type;

		memcpy(coeff_desc->name, data + pos + 1, *(data + pos));
		coeff_desc->name[*(data + pos)] = '\0';

		list_add(&coeff_desc->list, &cs40l2x->coeff_desc_head);

		pos += block_length;
	}

	return 0;
}

static int cs40l2x_firmware_parse(struct cs40l2x_private *cs40l2x,
			const struct firmware *fw)
{
	struct device *dev = cs40l2x->dev;
	unsigned int pos = CS40L2X_FW_FILE_HEADER_SIZE;
	unsigned int block_offset, block_type, block_length;
	int ret = -EINVAL;

	if (memcmp(fw->data, "WMFW", 4)) {
		dev_err(dev, "Failed to recognize firmware file\n");
		goto err_rls_fw;
	}

	if (fw->size % 4) {
		dev_err(dev, "Firmware file is not word-aligned\n");
		goto err_rls_fw;
	}

	while (pos < fw->size) {
		block_offset = fw->data[pos]
				+ (fw->data[pos + 1] << 8)
				+ (fw->data[pos + 2] << 16);
		pos += CS40L2X_FW_DBLK_OFFSET_SIZE;

		block_type = fw->data[pos];
		pos += CS40L2X_FW_DBLK_TYPE_SIZE;

		block_length = fw->data[pos]
				+ (fw->data[pos + 1] << 8)
				+ (fw->data[pos + 2] << 16)
				+ (fw->data[pos + 3] << 24);
		pos += CS40L2X_FW_DBLK_LENGTH_SIZE;

		switch (block_type) {
		case CS40L2X_WMFW_INFO_TYPE:
			break;
		case CS40L2X_PM_PACKED_TYPE:
			ret = cs40l2x_raw_write(cs40l2x,
					CS40L2X_DSP1_PMEM_0
						+ block_offset * 5,
					&fw->data[pos], block_length,
					CS40L2X_MAX_WLEN);
			if (ret) {
				dev_err(dev,
					"Failed to write PM_PACKED memory\n");
				goto err_rls_fw;
			}
			break;
		case CS40L2X_XM_PACKED_TYPE:
			ret = cs40l2x_raw_write(cs40l2x,
					CS40L2X_DSP1_XMEM_PACK_0
						+ block_offset * 3,
					&fw->data[pos], block_length,
					CS40L2X_MAX_WLEN);
			if (ret) {
				dev_err(dev,
					"Failed to write XM_PACKED memory\n");
				goto err_rls_fw;
			}
			break;
		case CS40L2X_YM_PACKED_TYPE:
			ret = cs40l2x_raw_write(cs40l2x,
					CS40L2X_DSP1_YMEM_PACK_0
						+ block_offset * 3,
					&fw->data[pos], block_length,
					CS40L2X_MAX_WLEN);
			if (ret) {
				dev_err(dev,
					"Failed to write YM_PACKED memory\n");
				goto err_rls_fw;
			}
			break;
		case CS40L2X_ALGO_INFO_TYPE:
			ret = cs40l2x_algo_parse(cs40l2x, &fw->data[pos]);
			if (ret) {
				dev_err(dev,
					"Failed to parse algorithm: %d\n", ret);
				goto err_rls_fw;
			}
			break;
		default:
			dev_err(dev, "Unexpected block type: 0x%02X\n",
					block_type);
			ret = -EINVAL;
			goto err_rls_fw;
		}

		/* blocks are word-aligned */
		pos += (block_length + 3) & ~0x00000003;
	}

	ret = cs40l2x_coeff_init(cs40l2x);

err_rls_fw:
	release_firmware(fw);

	return ret;
}

static void cs40l2x_firmware_load(const struct firmware *fw, void *context)
{
	struct cs40l2x_private *cs40l2x = (struct cs40l2x_private *)context;
	struct device *dev = cs40l2x->dev;
	int ret, i;

	if (!fw) {
		dev_err(dev, "Failed to request firmware file\n");
		return;
	}

	ret = cs40l2x_firmware_parse(cs40l2x, fw);
	if (ret)
		return;

	for (i = 0; i < cs40l2x->fw_desc->num_coeff_files; i++) {
		if ((!cs40l2x->dyn_f0_enable) &&
			(!strncmp(cs40l2x->fw_desc->coeff_files[i],
				CS40L2X_DYN_F0_FILE_NAME,
				CS40L2X_WT_FILE_NAME_LEN_MAX)))
			continue;
		if (strncmp(cs40l2x->fw_desc->coeff_files[i],
				CS40L2X_PAR_CONFIG_FILE_NAME,
				CS40L2X_WT_FILE_NAME_LEN_MAX))
			cs40l2x->par_bin_found = true;
		else if (strncmp(cs40l2x->fw_desc->coeff_files[i],
				CS40L2X_CLAB_CONFIG_FILE_NAME,
				CS40L2X_WT_FILE_NAME_LEN_MAX))
			cs40l2x->clab_bin_found = true;
		request_firmware_nowait(THIS_MODULE, FW_ACTION_HOTPLUG,
				cs40l2x->fw_desc->coeff_files[i], dev,
				GFP_KERNEL, cs40l2x, cs40l2x_coeff_file_load);
	}
}

static int cs40l2x_firmware_swap(struct cs40l2x_private *cs40l2x,
			unsigned int fw_id)
{
	const struct firmware *fw;
	struct regmap *regmap = cs40l2x->regmap;
	struct device *dev = cs40l2x->dev;
	int ret, i;

	if (cs40l2x->vibe_mode == CS40L2X_VIBE_MODE_AUDIO
			|| cs40l2x->vibe_state == CS40L2X_VIBE_STATE_RUNNING)
		return -EPERM;

	switch (cs40l2x->fw_desc->id) {
	case CS40L2X_FW_ID_ORIG:
		return -EPERM;

	case CS40L2X_FW_ID_B1ROM:
		ret = cs40l2x_basic_mode_exit(cs40l2x);
		if (ret)
			return ret;

		/* skip write sequencer if target firmware executes it */
		if (fw_id == cs40l2x->fw_id_remap)
			break;

		for (i = 0; i < cs40l2x->wseq_length; i++) {
			ret = regmap_write(regmap,
					cs40l2x->wseq_table[i].reg,
					cs40l2x->wseq_table[i].val);
			if (ret) {
				dev_err(dev, "Failed to execute write seq.\n");
				return ret;
			}
		}
		break;

	case CS40L2X_FW_ID_CAL:
		ret = cs40l2x_ack_write(cs40l2x,
				cs40l2x_dsp_reg(cs40l2x, "SHUTDOWNREQUEST",
						CS40L2X_XM_UNPACKED_TYPE,
						cs40l2x->fw_desc->id), 1, 0);
		if (ret)
			return ret;
		break;

	default:
		ret = cs40l2x_ack_write(cs40l2x,
				CS40L2X_MBOX_POWERCONTROL,
				CS40L2X_POWERCONTROL_FRC_STDBY,
				CS40L2X_POWERCONTROL_NONE);
		if (ret)
			return ret;
	}

	ret = regmap_update_bits(regmap, CS40L2X_DSP1_CCM_CORE_CTRL,
			CS40L2X_DSP1_EN_MASK, (0 << CS40L2X_DSP1_EN_SHIFT));
	if (ret) {
		dev_err(dev, "Failed to stop DSP\n");
		return ret;
	}

	cs40l2x_coeff_free(cs40l2x);

	if (fw_id == CS40L2X_FW_ID_CAL) {
		cs40l2x->diag_state = CS40L2X_DIAG_STATE_INIT;
		cs40l2x->dsp_cache_depth = 0;
		cs40l2x->open_wt_enable = false;
	}

	cs40l2x->exc_available = false;

	cs40l2x->fw_desc = cs40l2x_firmware_match(cs40l2x, fw_id);
	if (!cs40l2x->fw_desc)
		return -EINVAL;

	ret = request_firmware(&fw, cs40l2x->fw_desc->fw_file, dev);
	if (ret) {
		dev_err(dev, "Failed to request firmware file\n");
		return ret;
	}

	ret = cs40l2x_firmware_parse(cs40l2x, fw);
	if (ret)
		return ret;

	for (i = 0; i < cs40l2x->fw_desc->num_coeff_files; i++) {
		/* load alternate wavetable if one has been specified */
		if (!strncmp(cs40l2x->fw_desc->coeff_files[i],
				CS40L2X_WT_FILE_NAME_DEFAULT,
				CS40L2X_WT_FILE_NAME_LEN_MAX)
			&& strncmp(cs40l2x->wt_file,
				CS40L2X_WT_FILE_NAME_MISSING,
				CS40L2X_WT_FILE_NAME_LEN_MAX)) {
			ret = request_firmware(&fw, cs40l2x->wt_file, dev);
			if (ret)
				return ret;
		} else {
			if ((!cs40l2x->dyn_f0_enable) &&
				(!strncmp(cs40l2x->fw_desc->coeff_files[i],
					CS40L2X_DYN_F0_FILE_NAME,
					CS40L2X_WT_FILE_NAME_LEN_MAX)))
				continue;
			ret = request_firmware(&fw,
					cs40l2x->fw_desc->coeff_files[i], dev);
			if (ret)
				continue;
		}

		ret = cs40l2x_coeff_file_parse(cs40l2x, fw);
		if (ret)
			return ret;
	}

	ret = cs40l2x_dsp_pre_config(cs40l2x);
	if (ret)
		return ret;

	ret = cs40l2x_dsp_start(cs40l2x);
	if (ret)
		return ret;

	return cs40l2x_dsp_post_config(cs40l2x);
}

static int cs40l2x_wavetable_swap(struct cs40l2x_private *cs40l2x,
			const char *wt_file)
{
	const struct firmware *fw;
	struct regmap *regmap = cs40l2x->regmap;
	struct device *dev = cs40l2x->dev;
	int ret1, ret2;

	ret1 = cs40l2x_ack_write(cs40l2x,
			CS40L2X_MBOX_POWERCONTROL,
			CS40L2X_POWERCONTROL_FRC_STDBY,
			CS40L2X_POWERCONTROL_NONE);
	if (ret1)
		return ret1;

	ret1 = request_firmware(&fw, wt_file, dev);
	if (ret1) {
		dev_err(dev, "Failed to request wavetable file\n");
		goto err_wakeup;
	}

	ret1 = cs40l2x_coeff_file_parse(cs40l2x, fw);
	if (ret1)
		return ret1;

	strlcpy(cs40l2x->wt_file, wt_file, CS40L2X_WT_FILE_NAME_LEN_MAX);

	ret1 = regmap_write(regmap,
			cs40l2x_dsp_reg(cs40l2x, "NUMBEROFWAVES",
					CS40L2X_XM_UNPACKED_TYPE,
					CS40L2X_ALGO_ID_VIBE),
			0);
	if (ret1) {
		dev_err(dev, "Failed to reset wavetable\n");
		return ret1;
	}

err_wakeup:
	ret2 = cs40l2x_ack_write(cs40l2x,
			CS40L2X_MBOX_POWERCONTROL,
			CS40L2X_POWERCONTROL_WAKEUP,
			CS40L2X_POWERCONTROL_NONE);
	if (ret2)
		return ret2;

	ret2 = cs40l2x_ack_write(cs40l2x, CS40L2X_MBOX_TRIGGERINDEX,
			CS40L2X_INDEX_CONT_MIN, CS40L2X_MBOX_TRIGGERRESET);
	if (ret2)
		return ret2;

	return ret1;
}

static int cs40l2x_wavetable_sync(struct cs40l2x_private *cs40l2x)
{
	struct regmap *regmap = cs40l2x->regmap;
	struct device *dev = cs40l2x->dev;
	unsigned int cp_trigger_index = cs40l2x->cp_trigger_index;
	unsigned int tag, val;
	int ret, i;

	ret = regmap_read(regmap,
			cs40l2x_dsp_reg(cs40l2x, "NUMBEROFWAVES",
					CS40L2X_XM_UNPACKED_TYPE,
					CS40L2X_ALGO_ID_VIBE),
			&cs40l2x->num_waves);
	if (ret) {
		dev_err(dev, "Failed to count wavetable entries\n");
		return ret;
	}

	if (!cs40l2x->num_waves) {
		dev_err(dev, "Wavetable is empty\n");
		return -EINVAL;
	}

	if (!cs40l2x->virtual_bin) {
		dev_info(dev, "Loaded %u waveforms from %s, last modified on %s\n",
			cs40l2x->num_waves, cs40l2x->wt_file, cs40l2x->wt_date);

		if ((cp_trigger_index & CS40L2X_INDEX_MASK) >=
			cs40l2x->num_waves
				&& cp_trigger_index != CS40L2X_INDEX_QEST
				&& cp_trigger_index != CS40L2X_INDEX_PEAK
				&& cp_trigger_index != CS40L2X_INDEX_PBQ
				&& cp_trigger_index != CS40L2X_INDEX_DIAG)
			dev_warn(dev, "Invalid cp_trigger_index\n");

		for (i = 0; i < cs40l2x->pbq_depth; i++) {
			tag = cs40l2x->pbq_pairs[i].tag;

			if (tag >= cs40l2x->num_waves
					&& tag != CS40L2X_PBQ_TAG_SILENCE
					&& tag != CS40L2X_PBQ_TAG_START
					&& tag != CS40L2X_PBQ_TAG_STOP)
				dev_warn(dev, "Invalid cp_trigger_queue\n");
		}
	} else {
		dev_info(dev, "Loaded %u waveforms from %s, last modified on %s\n",
			(cs40l2x->num_waves - CS40L2X_WT_NUM_VIRT_SLOTS),
			cs40l2x->wt_file, cs40l2x->wt_date);
	}

	for (i = 0; i < CS40L2X_NUM_GPIO; i++) {
		if (!(cs40l2x->gpio_mask & (1 << i)))
			continue;

		ret = cs40l2x_gpio_edge_index_get(cs40l2x,
				&val, i << 2, CS40L2X_GPIO_RISE);
		if (ret)
			return ret;
		if (!cs40l2x->virtual_bin) {
			if (val >= cs40l2x->num_waves)
				dev_err(dev, "Invalid gpio%d_rise_index\n",
					i + 1);
		} else {
			if (val >= ((cs40l2x->num_waves +
				cs40l2x->num_virtual_waves) -
				CS40L2X_WT_NUM_VIRT_SLOTS))
				dev_err(dev, "Invalid gpio%d_rise_index\n",
					i + 1);
		}

		ret = cs40l2x_gpio_edge_index_get(cs40l2x,
				&val, i << 2, CS40L2X_GPIO_FALL);
		if (ret)
			return ret;
		if (!cs40l2x->virtual_bin) {
			if (val >= cs40l2x->num_waves)
				dev_err(dev, "Invalid gpio%d_fall_index\n",
					i + 1);
		} else {
			if (val >= ((cs40l2x->num_waves +
				cs40l2x->num_virtual_waves) -
				CS40L2X_WT_NUM_VIRT_SLOTS))
				dev_err(dev, "Invalid gpio%d_fall_index\n",
					i + 1);
		}
	}

	return 0;
}

static int cs40l2x_boost_short_test(struct cs40l2x_private *cs40l2x)
{
	struct regmap *regmap = cs40l2x->regmap;
	struct device *dev = cs40l2x->dev;
	unsigned int val;
	int ret;

	ret = regmap_update_bits(regmap, CS40L2X_BSTCVRT_VCTRL2,
			CS40L2X_BST_CTL_SEL_MASK,
			CS40L2X_BST_CTL_SEL_CP_VAL
				<< CS40L2X_BST_CTL_SEL_SHIFT);
	if (ret) {
		dev_err(dev, "Failed to change VBST target selection\n");
		return ret;
	}

	ret = regmap_update_bits(regmap, CS40L2X_PWR_CTRL1,
			CS40L2X_GLOBAL_EN_MASK, 1 << CS40L2X_GLOBAL_EN_SHIFT);
	if (ret) {
		dev_err(dev, "Failed to enable device\n");
		return ret;
	}

	usleep_range(10000, 10100);

	ret = regmap_read(regmap, CS40L2X_IRQ1_STATUS1, &val);
	if (ret) {
		dev_err(dev, "Failed to read boost converter error status\n");
		return ret;
	}

	if (val & CS40L2X_BST_SHORT_ERR) {
		dev_err(dev, "Encountered fatal boost converter short error\n");
		return -EIO;
	}

	ret = regmap_update_bits(regmap, CS40L2X_PWR_CTRL1,
			CS40L2X_GLOBAL_EN_MASK, 0 << CS40L2X_GLOBAL_EN_SHIFT);
	if (ret) {
		dev_err(dev, "Failed to disable device\n");
		return ret;
	}

	ret = regmap_update_bits(regmap, CS40L2X_BSTCVRT_VCTRL2,
			CS40L2X_BST_CTL_SEL_MASK,
			CS40L2X_BST_CTL_SEL_CLASSH
				<< CS40L2X_BST_CTL_SEL_SHIFT);
	if (ret) {
		dev_err(dev, "Failed to restore VBST target selection\n");
		return ret;
	}

	return cs40l2x_wseq_replace(cs40l2x,
			CS40L2X_TEST_LBST, CS40L2X_EXPL_MODE_DIS);
}

static int cs40l2x_boost_config(struct cs40l2x_private *cs40l2x)
{
	struct regmap *regmap = cs40l2x->regmap;
	struct device *dev = cs40l2x->dev;
	unsigned int boost_ind = cs40l2x->pdata.boost_ind;
	unsigned int boost_cap = cs40l2x->pdata.boost_cap;
	unsigned int boost_ipk = cs40l2x->pdata.boost_ipk;
	unsigned int boost_ctl = cs40l2x->pdata.boost_ctl;
	unsigned int boost_ovp = cs40l2x->pdata.boost_ovp;
	unsigned int bst_lbst_val, bst_cbst_range;
	unsigned int bst_ipk_scaled, bst_ctl_scaled, bst_ovp_scaled;
	int ret;

	switch (boost_ind) {
	case 1000:	/* 1.0 uH */
		bst_lbst_val = 0;
		break;
	case 1200:	/* 1.2 uH */
		bst_lbst_val = 1;
		break;
	case 1500:	/* 1.5 uH */
		bst_lbst_val = 2;
		break;
	case 2200:	/* 2.2 uH */
		bst_lbst_val = 3;
		break;
	default:
		dev_err(dev, "Invalid boost inductor value: %d nH\n",
				boost_ind);
		return -EINVAL;
	}

	switch (boost_cap) {
	case 0 ... 19:
		bst_cbst_range = 0;
		break;
	case 20 ... 50:
		bst_cbst_range = 1;
		break;
	case 51 ... 100:
		bst_cbst_range = 2;
		break;
	case 101 ... 200:
		bst_cbst_range = 3;
		break;
	default:	/* 201 uF and greater */
		bst_cbst_range = 4;
	}

	ret = regmap_update_bits(regmap, CS40L2X_BSTCVRT_COEFF,
			CS40L2X_BST_K1_MASK,
			cs40l2x_bst_k1_table[bst_lbst_val][bst_cbst_range]
				<< CS40L2X_BST_K1_SHIFT);
	if (ret) {
		dev_err(dev, "Failed to write boost K1 coefficient\n");
		return ret;
	}

	ret = regmap_update_bits(regmap, CS40L2X_BSTCVRT_COEFF,
			CS40L2X_BST_K2_MASK,
			cs40l2x_bst_k2_table[bst_lbst_val][bst_cbst_range]
				<< CS40L2X_BST_K2_SHIFT);
	if (ret) {
		dev_err(dev, "Failed to write boost K2 coefficient\n");
		return ret;
	}

	ret = cs40l2x_wseq_add_reg(cs40l2x, CS40L2X_BSTCVRT_COEFF,
			(cs40l2x_bst_k2_table[bst_lbst_val][bst_cbst_range]
				<< CS40L2X_BST_K2_SHIFT) |
			(cs40l2x_bst_k1_table[bst_lbst_val][bst_cbst_range]
				<< CS40L2X_BST_K1_SHIFT));
	if (ret) {
		dev_err(dev, "Failed to sequence boost K1/K2 coefficients\n");
		return ret;
	}

	ret = regmap_update_bits(regmap, CS40L2X_BSTCVRT_SLOPE_LBST,
			CS40L2X_BST_SLOPE_MASK,
			cs40l2x_bst_slope_table[bst_lbst_val]
				<< CS40L2X_BST_SLOPE_SHIFT);
	if (ret) {
		dev_err(dev, "Failed to write boost slope coefficient\n");
		return ret;
	}

	ret = regmap_update_bits(regmap, CS40L2X_BSTCVRT_SLOPE_LBST,
			CS40L2X_BST_LBST_VAL_MASK,
			bst_lbst_val << CS40L2X_BST_LBST_VAL_SHIFT);
	if (ret) {
		dev_err(dev, "Failed to write boost inductor value\n");
		return ret;
	}

	ret = cs40l2x_wseq_add_reg(cs40l2x, CS40L2X_BSTCVRT_SLOPE_LBST,
			(cs40l2x_bst_slope_table[bst_lbst_val]
				<< CS40L2X_BST_SLOPE_SHIFT) |
			(bst_lbst_val << CS40L2X_BST_LBST_VAL_SHIFT));
	if (ret) {
		dev_err(dev, "Failed to sequence boost inductor value\n");
		return ret;
	}

	if ((boost_ipk < 1600) || (boost_ipk > 4500)) {
		dev_err(dev, "Invalid boost inductor peak current: %d mA\n",
				boost_ipk);
		return -EINVAL;
	}
	bst_ipk_scaled = ((boost_ipk - 1600) / 50) + 0x10;

	ret = regmap_update_bits(regmap, CS40L2X_BSTCVRT_PEAK_CUR,
			CS40L2X_BST_IPK_MASK,
			bst_ipk_scaled << CS40L2X_BST_IPK_SHIFT);
	if (ret) {
		dev_err(dev, "Failed to write boost inductor peak current\n");
		return ret;
	}

	ret = cs40l2x_wseq_add_reg(cs40l2x, CS40L2X_BSTCVRT_PEAK_CUR,
			bst_ipk_scaled << CS40L2X_BST_IPK_SHIFT);
	if (ret) {
		dev_err(dev,
			"Failed to sequence boost inductor peak current\n");
		return ret;
	}

	if (boost_ctl)
		boost_ctl &= CS40L2X_PDATA_MASK;
	else
		boost_ctl = 11000;

	switch (boost_ctl) {
	case 0:
		bst_ctl_scaled = boost_ctl;
		break;
	case 2550 ... 11000:
		bst_ctl_scaled = ((boost_ctl - 2550) / 50) + 1;
		break;
	default:
		dev_err(dev, "Invalid VBST limit: %d mV\n", boost_ctl);
		return -EINVAL;
	}

	ret = regmap_update_bits(regmap, CS40L2X_BSTCVRT_VCTRL1,
			CS40L2X_BST_CTL_MASK,
			bst_ctl_scaled << CS40L2X_BST_CTL_SHIFT);
	if (ret) {
		dev_err(dev, "Failed to write VBST limit\n");
		return ret;
	}

	ret = cs40l2x_wseq_add_reg(cs40l2x, CS40L2X_BSTCVRT_VCTRL1,
			bst_ctl_scaled << CS40L2X_BST_CTL_SHIFT);
	if (ret) {
		dev_err(dev, "Failed to sequence VBST limit\n");
		return ret;
	}

	ret = regmap_update_bits(regmap, CS40L2X_BSTCVRT_VCTRL2,
			CS40L2X_BST_CTL_LIM_EN_MASK,
			1 << CS40L2X_BST_CTL_LIM_EN_SHIFT);
	if (ret) {
		dev_err(dev, "Failed to configure VBST control\n");
		return ret;
	}

	ret = cs40l2x_wseq_add_reg(cs40l2x, CS40L2X_BSTCVRT_VCTRL2,
			(1 << CS40L2X_BST_CTL_LIM_EN_SHIFT) |
			(CS40L2X_BST_CTL_SEL_CLASSH
				<< CS40L2X_BST_CTL_SEL_SHIFT));
	if (ret) {
		dev_err(dev, "Failed to sequence VBST control\n");
		return ret;
	}

	switch (boost_ovp) {
	case 0:
		break;
	case 9000 ... 12875:
		bst_ovp_scaled = ((boost_ovp - 9000) / 125) * 2;

		ret = regmap_update_bits(regmap, CS40L2X_BSTCVRT_OVERVOLT_CTRL,
				CS40L2X_BST_OVP_THLD_MASK,
				bst_ovp_scaled << CS40L2X_BST_OVP_THLD_SHIFT);
		if (ret) {
			dev_err(dev, "Failed to write OVP threshold\n");
			return ret;
		}

		ret = cs40l2x_wseq_add_reg(cs40l2x,
				CS40L2X_BSTCVRT_OVERVOLT_CTRL,
				(1 << CS40L2X_BST_OVP_EN_SHIFT) |
				(bst_ovp_scaled << CS40L2X_BST_OVP_THLD_SHIFT));
		if (ret) {
			dev_err(dev, "Failed to sequence OVP threshold\n");
			return ret;
		}
		break;
	default:
		dev_err(dev, "Invalid OVP threshold: %d mV\n", boost_ovp);
		return -EINVAL;
	}

	if (cs40l2x->devid == CS40L2X_DEVID_L20)
		return 0;

	return cs40l2x_boost_short_test(cs40l2x);
}

static int cs40l2x_asp_config(struct cs40l2x_private *cs40l2x)
{
	struct regmap *regmap = cs40l2x->regmap;
	struct device *dev = cs40l2x->dev;
	unsigned int asp_bclk_freq = cs40l2x->pdata.asp_bclk_freq;
	bool asp_bclk_inv = cs40l2x->pdata.asp_bclk_inv;
	bool asp_fsync_inv = cs40l2x->pdata.asp_fsync_inv;
	unsigned int asp_fmt = cs40l2x->pdata.asp_fmt;
	unsigned int asp_slot_num = cs40l2x->pdata.asp_slot_num;
	unsigned int asp_slot_width = cs40l2x->pdata.asp_slot_width;
	unsigned int asp_samp_width = cs40l2x->pdata.asp_samp_width;
	unsigned int asp_frame_cfg = 0;
	int ret, i;

	if (asp_bclk_freq % asp_slot_width
			|| asp_slot_width < CS40L2X_ASP_RX_WIDTH_MIN
			|| asp_slot_width > CS40L2X_ASP_RX_WIDTH_MAX) {
		dev_err(dev, "Invalid ASP slot width: %d bits\n",
				asp_slot_width);
		return -EINVAL;
	}

	if (asp_samp_width > asp_slot_width
			|| asp_samp_width < CS40L2X_ASP_RX_WL_MIN
			|| asp_samp_width > CS40L2X_ASP_RX_WL_MAX) {
		dev_err(dev, "Invalid ASP sample width: %d bits\n",
				asp_samp_width);
		return -EINVAL;
	}

	if (asp_fmt) {
		asp_fmt &= CS40L2X_PDATA_MASK;

		switch (asp_fmt) {
		case CS40L2X_ASP_FMT_TDM1:
		case CS40L2X_ASP_FMT_I2S:
		case CS40L2X_ASP_FMT_TDM1R5:
			break;
		default:
			dev_err(dev, "Invalid ASP format: %d\n", asp_fmt);
			return -EINVAL;
		}
	} else {
		asp_fmt = CS40L2X_ASP_FMT_I2S;
	}

	if (asp_slot_num > CS40L2X_ASP_RX1_SLOT_MAX) {
		dev_err(dev, "Invalid ASP slot number: %d\n", asp_slot_num);
		return -EINVAL;
	}

	ret = cs40l2x_wseq_add_reg(cs40l2x, CS40L2X_SP_ENABLES, 0);
	if (ret) {
		dev_err(dev, "Failed to sequence ASP enable controls\n");
		return ret;
	}

	for (i = 0; i < CS40L2X_NUM_REFCLKS; i++)
		if (cs40l2x_refclks[i].freq == asp_bclk_freq)
			break;
	if (i == CS40L2X_NUM_REFCLKS) {
		dev_err(dev, "Invalid ASP_BCLK frequency: %d Hz\n",
				asp_bclk_freq);
		return -EINVAL;
	}

	ret = regmap_update_bits(regmap, CS40L2X_SP_RATE_CTRL,
			CS40L2X_ASP_BCLK_FREQ_MASK,
			i << CS40L2X_ASP_BCLK_FREQ_SHIFT);
	if (ret) {
		dev_err(dev, "Failed to write ASP_BCLK frequency\n");
		return ret;
	}

	ret = cs40l2x_wseq_add_reg(cs40l2x, CS40L2X_SP_RATE_CTRL,
			i << CS40L2X_ASP_BCLK_FREQ_SHIFT);
	if (ret) {
		dev_err(dev, "Failed to sequence ASP_BCLK frequency\n");
		return ret;
	}

	ret = regmap_write(regmap, CS40L2X_FS_MON_0, cs40l2x_refclks[i].coeff);
	if (ret) {
		dev_err(dev, "Failed to write ASP coefficients\n");
		return ret;
	}

	ret = cs40l2x_wseq_add_reg(cs40l2x, CS40L2X_FS_MON_0,
			cs40l2x_refclks[i].coeff);
	if (ret) {
		dev_err(dev, "Failed to sequence ASP coefficients\n");
		return ret;
	}

	asp_frame_cfg |= (asp_slot_width << CS40L2X_ASP_RX_WIDTH_SHIFT);
	asp_frame_cfg |= (asp_slot_width << CS40L2X_ASP_TX_WIDTH_SHIFT);
	asp_frame_cfg |= (asp_fmt << CS40L2X_ASP_FMT_SHIFT);
	asp_frame_cfg |= (asp_bclk_inv ? CS40L2X_ASP_BCLK_INV_MASK : 0);
	asp_frame_cfg |= (asp_fsync_inv ? CS40L2X_ASP_FSYNC_INV_MASK : 0);

	ret = regmap_write(regmap, CS40L2X_SP_FORMAT, asp_frame_cfg);
	if (ret) {
		dev_err(dev, "Failed to write ASP frame configuration\n");
		return ret;
	}

	ret = cs40l2x_wseq_add_reg(cs40l2x, CS40L2X_SP_FORMAT, asp_frame_cfg);
	if (ret) {
		dev_err(dev, "Failed to sequence ASP frame configuration\n");
		return ret;
	}

	ret = regmap_update_bits(regmap, CS40L2X_SP_FRAME_RX_SLOT,
			CS40L2X_ASP_RX1_SLOT_MASK,
			asp_slot_num << CS40L2X_ASP_RX1_SLOT_SHIFT);
	if (ret) {
		dev_err(dev, "Failed to write ASP slot number\n");
		return ret;
	}

	ret = cs40l2x_wseq_add_reg(cs40l2x, CS40L2X_SP_FRAME_RX_SLOT,
			asp_slot_num << CS40L2X_ASP_RX1_SLOT_SHIFT);
	if (ret) {
		dev_err(dev, "Failed to sequence ASP slot number\n");
		return ret;
	}

	ret = regmap_update_bits(regmap, CS40L2X_SP_RX_WL,
			CS40L2X_ASP_RX_WL_MASK,
			asp_samp_width << CS40L2X_ASP_RX_WL_SHIFT);
	if (ret) {
		dev_err(dev, "Failed to write ASP sample width\n");
		return ret;
	}

	ret = cs40l2x_wseq_add_reg(cs40l2x, CS40L2X_SP_RX_WL,
			asp_samp_width << CS40L2X_ASP_RX_WL_SHIFT);
	if (ret) {
		dev_err(dev, "Failed to sequence ASP sample width\n");
		return ret;
	}

	return 0;
}

static int cs40l2x_brownout_config(struct cs40l2x_private *cs40l2x,
			unsigned int br_reg)
{
	struct regmap *regmap = cs40l2x->regmap;
	struct device *dev = cs40l2x->dev;
	struct cs40l2x_br_desc *br_config;
	bool br_enable;
	unsigned int br_thld1_scaled = 0;
	unsigned int br_thld1, br_thld1_mask, br_thld1_max, br_en_mask, val;
	int ret;

	switch (br_reg) {
	case CS40L2X_VPBR_CFG:
		br_enable = cs40l2x->pdata.vpbr_enable;
		br_config = &cs40l2x->pdata.vpbr_config;

		br_en_mask = CS40L2X_VPBR_EN_MASK;
		br_thld1_mask = CS40L2X_VPBR_THLD1_MASK;
		br_thld1_max = CS40L2X_VPBR_THLD1_MAX;

		br_thld1 = cs40l2x->pdata.vpbr_thld1;
		if (!br_thld1)
			break;

		if ((br_thld1 < 2497) || (br_thld1 > 3874)) {
			dev_err(dev, "Invalid VPBR threshold: %u mV\n",
					br_thld1);
			return -EINVAL;
		}
		br_thld1_scaled = ((br_thld1 - 2497) * 1000 / 47482) + 0x02;
		break;

	case CS40L2X_VBBR_CFG:
		br_enable = cs40l2x->pdata.vbbr_enable;
		br_config = &cs40l2x->pdata.vbbr_config;

		br_en_mask = CS40L2X_VBBR_EN_MASK;
		br_thld1_mask = CS40L2X_VBBR_THLD1_MASK;
		br_thld1_max = CS40L2X_VBBR_THLD1_MAX;

		br_thld1 = cs40l2x->pdata.vbbr_thld1;
		if (!br_thld1)
			break;

		if ((br_thld1 < 109) || (br_thld1 > 3445)) {
			dev_err(dev, "Invalid VBBR threshold: %u mV\n",
					br_thld1);
			return -EINVAL;
		}
		br_thld1_scaled = ((br_thld1 - 109) * 1000 / 54688) + 0x02;
		break;

	default:
		return -EINVAL;
	}

	br_enable |= br_config->enable;
	if (!br_enable)
		return 0;

	ret = regmap_read(regmap, CS40L2X_PWR_CTRL3, &val);
	if (ret) {
		dev_err(dev, "Failed to read VPBR/VBBR enable controls\n");
		return ret;
	}

	val |= br_en_mask;

	ret = regmap_write(regmap, CS40L2X_PWR_CTRL3, val);
	if (ret) {
		dev_err(dev, "Failed to write VPBR/VBBR enable controls\n");
		return ret;
	}

	ret = cs40l2x_wseq_add_reg(cs40l2x, CS40L2X_PWR_CTRL3, val);
	if (ret) {
		dev_err(dev, "Failed to sequence VPBR/VBBR enable controls\n");
		return ret;
	}

	if (!br_config->present && !br_thld1_scaled)
		return 0;

	ret = regmap_read(regmap, br_reg, &val);
	if (ret) {
		dev_err(dev, "Failed to read VPBR/VBBR configuration\n");
		return ret;
	}

	if (br_config->present) {
		if (br_config->thld1 > br_thld1_max) {
			dev_err(dev, "Invalid VPBR/VBBR threshold: %u\n",
					br_config->thld1);
			return -EINVAL;
		}
		val &= ~br_thld1_mask;
		val |= (br_config->thld1 << CS40L2X_VxBR_THLD1_SHIFT);

		if (br_config->max_att > CS40L2X_VxBR_MAX_ATT_MAX) {
			dev_err(dev, "Invalid VPBR/VBBR max. attenuation: %u\n",
					br_config->max_att);
			return -EINVAL;
		}
		val &= ~CS40L2X_VxBR_MAX_ATT_MASK;
		val |= (br_config->max_att << CS40L2X_VxBR_MAX_ATT_SHIFT);

		if (br_config->atk_vol > CS40L2X_VxBR_ATK_VOL_MAX) {
			dev_err(dev, "Invalid VPBR/VBBR attack volume: %u\n",
					br_config->atk_vol);
			return -EINVAL;
		}
		val &= ~CS40L2X_VxBR_ATK_VOL_MASK;
		val |= (br_config->atk_vol << CS40L2X_VxBR_ATK_VOL_SHIFT);

		if (br_config->atk_rate > CS40L2X_VxBR_ATK_RATE_MAX) {
			dev_err(dev, "Invalid VPBR/VBBR attack rate: %u\n",
					br_config->atk_rate);
			return -EINVAL;
		}
		val &= ~CS40L2X_VxBR_ATK_RATE_MASK;
		val |= (br_config->atk_rate << CS40L2X_VxBR_ATK_RATE_SHIFT);

		if (br_config->wait > CS40L2X_VxBR_WAIT_MAX) {
			dev_err(dev, "Invalid VPBR/VBBR wait time: %u\n",
					br_config->wait);
			return -EINVAL;
		}
		val &= ~CS40L2X_VxBR_WAIT_MASK;
		val |= (br_config->wait << CS40L2X_VxBR_WAIT_SHIFT);

		if (br_config->rel_rate > CS40L2X_VxBR_REL_RATE_MAX) {
			dev_err(dev, "Invalid VPBR/VBBR release rate: %u\n",
					br_config->rel_rate);
			return -EINVAL;
		}
		val &= ~CS40L2X_VxBR_REL_RATE_MASK;
		val |= (br_config->rel_rate << CS40L2X_VxBR_REL_RATE_SHIFT);

		if (br_config->mute_enable)
			val |= CS40L2X_VxBR_MUTE_EN_MASK;
	}

	if (br_thld1_scaled) {
		val &= ~br_thld1_mask;
		val |= (br_thld1_scaled << CS40L2X_VxBR_THLD1_SHIFT);
	}

	ret = regmap_write(regmap, br_reg, val);
	if (ret) {
		dev_err(dev, "Failed to write VPBR/VBBR configuration\n");
		return ret;
	}

	ret = cs40l2x_wseq_add_reg(cs40l2x, br_reg, val);
	if (ret) {
		dev_err(dev, "Failed to sequence VPBR/VBBR configuration\n");
		return ret;
	}

	return 0;
}

static const struct reg_sequence cs40l2x_mpu_config[] = {
	{CS40L2X_DSP1_MPU_LOCK_CONFIG,	CS40L2X_MPU_UNLOCK_CODE1},
	{CS40L2X_DSP1_MPU_LOCK_CONFIG,	CS40L2X_MPU_UNLOCK_CODE2},
	{CS40L2X_DSP1_MPU_XM_ACCESS0,	0xFFFFFFFF},
	{CS40L2X_DSP1_MPU_YM_ACCESS0,	0xFFFFFFFF},
	{CS40L2X_DSP1_MPU_WNDW_ACCESS0,	0xFFFFFFFF},
	{CS40L2X_DSP1_MPU_XREG_ACCESS0,	0xFFFFFFFF},
	{CS40L2X_DSP1_MPU_YREG_ACCESS0,	0xFFFFFFFF},
	{CS40L2X_DSP1_MPU_WNDW_ACCESS1,	0xFFFFFFFF},
	{CS40L2X_DSP1_MPU_XREG_ACCESS1,	0xFFFFFFFF},
	{CS40L2X_DSP1_MPU_YREG_ACCESS1,	0xFFFFFFFF},
	{CS40L2X_DSP1_MPU_WNDW_ACCESS2,	0xFFFFFFFF},
	{CS40L2X_DSP1_MPU_XREG_ACCESS2,	0xFFFFFFFF},
	{CS40L2X_DSP1_MPU_YREG_ACCESS2,	0xFFFFFFFF},
	{CS40L2X_DSP1_MPU_WNDW_ACCESS3,	0xFFFFFFFF},
	{CS40L2X_DSP1_MPU_XREG_ACCESS3,	0xFFFFFFFF},
	{CS40L2X_DSP1_MPU_YREG_ACCESS3,	0xFFFFFFFF},
	{CS40L2X_DSP1_MPU_LOCK_CONFIG,	0x00000000}
};

static const struct reg_sequence cs40l2x_pcm_routing[] = {
	{CS40L2X_DAC_PCM1_SRC,		CS40L2X_DAC_PCM1_SRC_DSP1TX1},
	{CS40L2X_DSP1_RX1_SRC,		CS40L2X_DSP1_RXn_SRC_ASPRX1},
	{CS40L2X_DSP1_RX2_SRC,		CS40L2X_DSP1_RXn_SRC_VMON},
	{CS40L2X_DSP1_RX3_SRC,		CS40L2X_DSP1_RXn_SRC_IMON},
	{CS40L2X_DSP1_RX4_SRC,		CS40L2X_DSP1_RXn_SRC_VPMON},
};

static int cs40l2x_init(struct cs40l2x_private *cs40l2x)
{
	int ret;
	struct regmap *regmap = cs40l2x->regmap;
	struct device *dev = cs40l2x->dev;
	unsigned int wksrc_en = CS40L2X_WKSRC_EN_SDA;
	unsigned int wksrc_pol = CS40L2X_WKSRC_POL_SDA;
	unsigned int wksrc_ctl;

	/* REFCLK configuration is handled by revision B1 ROM */
	if (cs40l2x->pdata.refclk_gpio2 &&
			(cs40l2x->revid < CS40L2X_REVID_B1)) {
		ret = regmap_update_bits(regmap, CS40L2X_GPIO_PAD_CONTROL,
				CS40L2X_GP2_CTRL_MASK,
				CS40L2X_GPx_CTRL_MCLK
					<< CS40L2X_GP2_CTRL_SHIFT);
		if (ret) {
			dev_err(dev, "Failed to select GPIO2 function\n");
			return ret;
		}

		ret = cs40l2x_wseq_add_reg(cs40l2x, CS40L2X_GPIO_PAD_CONTROL,
				((CS40L2X_GPx_CTRL_MCLK
					<< CS40L2X_GP2_CTRL_SHIFT)
					& CS40L2X_GP2_CTRL_MASK) |
				((CS40L2X_GPx_CTRL_GPIO
					<< CS40L2X_GP1_CTRL_SHIFT)
					& CS40L2X_GP1_CTRL_MASK));
		if (ret) {
			dev_err(dev,
				"Failed to sequence GPIO1/2 configuration\n");
			return ret;
		}

		ret = regmap_update_bits(regmap, CS40L2X_PLL_CLK_CTRL,
				CS40L2X_PLL_REFCLK_SEL_MASK,
				CS40L2X_PLL_REFCLK_SEL_MCLK
					<< CS40L2X_PLL_REFCLK_SEL_SHIFT);
		if (ret) {
			dev_err(dev, "Failed to select clock source\n");
			return ret;
		}

		ret = cs40l2x_wseq_add_reg(cs40l2x, CS40L2X_PLL_CLK_CTRL,
				((1 << CS40L2X_PLL_REFCLK_EN_SHIFT)
					& CS40L2X_PLL_REFCLK_EN_MASK) |
				((CS40L2X_PLL_REFCLK_SEL_MCLK
					<< CS40L2X_PLL_REFCLK_SEL_SHIFT)
					& CS40L2X_PLL_REFCLK_SEL_MASK));
		if (ret) {
			dev_err(dev,
				"Failed to sequence PLL configuration\n");
			return ret;
		}
	}

	ret = cs40l2x_boost_config(cs40l2x);
	if (ret)
		return ret;

	ret = regmap_multi_reg_write(regmap, cs40l2x_pcm_routing,
			ARRAY_SIZE(cs40l2x_pcm_routing));
	if (ret) {
		dev_err(dev, "Failed to configure PCM channel routing\n");
		return ret;
	}

	ret = cs40l2x_wseq_add_seq(cs40l2x, cs40l2x_pcm_routing,
			ARRAY_SIZE(cs40l2x_pcm_routing));
	if (ret) {
		dev_err(dev, "Failed to sequence PCM channel routing\n");
		return ret;
	}

	ret = cs40l2x_wseq_add_reg(cs40l2x, CS40L2X_AMP_DIG_VOL_CTRL,
			(1 << CS40L2X_AMP_HPF_PCM_EN_SHIFT)
				& CS40L2X_AMP_HPF_PCM_EN_MASK);
	if (ret) {
		dev_err(dev, "Failed to sequence amplifier volume control\n");
		return ret;
	}

	/* revisions A0 and B0 require MPU to be configured manually */
	if (cs40l2x->revid < CS40L2X_REVID_B1) {
		ret = regmap_multi_reg_write(regmap, cs40l2x_mpu_config,
				ARRAY_SIZE(cs40l2x_mpu_config));
		if (ret) {
			dev_err(dev, "Failed to configure MPU\n");
			return ret;
		}
	}

	/* hibernation is supported by revision B1 firmware only */
	if (cs40l2x->revid == CS40L2X_REVID_B1) {
		/* enables */
		if (cs40l2x->gpio_mask & CS40L2X_GPIO_BTNDETECT_GPIO1)
			wksrc_en |= CS40L2X_WKSRC_EN_GPIO1;
		if (cs40l2x->gpio_mask & CS40L2X_GPIO_BTNDETECT_GPIO2)
			wksrc_en |= CS40L2X_WKSRC_EN_GPIO2;
		if (cs40l2x->gpio_mask & CS40L2X_GPIO_BTNDETECT_GPIO4)
			wksrc_en |= CS40L2X_WKSRC_EN_GPIO4;

		/* polarities */
		if (cs40l2x->pdata.gpio_indv_pol & CS40L2X_GPIO_BTNDETECT_GPIO1)
			wksrc_pol |= CS40L2X_WKSRC_POL_GPIO1;
		if (cs40l2x->pdata.gpio_indv_pol & CS40L2X_GPIO_BTNDETECT_GPIO2)
			wksrc_pol |= CS40L2X_WKSRC_POL_GPIO2;
		if (cs40l2x->pdata.gpio_indv_pol & CS40L2X_GPIO_BTNDETECT_GPIO4)
			wksrc_pol |= CS40L2X_WKSRC_POL_GPIO4;

		wksrc_ctl = ((wksrc_en << CS40L2X_WKSRC_EN_SHIFT)
				& CS40L2X_WKSRC_EN_MASK)
				| ((wksrc_pol << CS40L2X_WKSRC_POL_SHIFT)
					& CS40L2X_WKSRC_POL_MASK);

		ret = regmap_write(regmap,
				CS40L2X_WAKESRC_CTL, wksrc_ctl);
		if (ret) {
			dev_err(dev, "Failed to enable wake sources\n");
			return ret;
		}

		ret = cs40l2x_wseq_add_reg(cs40l2x,
				CS40L2X_WAKESRC_CTL, wksrc_ctl);
		if (ret) {
			dev_err(dev, "Failed to sequence wake sources\n");
			return ret;
		}
	}

	if (cs40l2x->asp_available) {
		ret = cs40l2x_wseq_add_reg(cs40l2x, CS40L2X_PLL_CLK_CTRL,
				((1 << CS40L2X_PLL_REFCLK_EN_SHIFT)
					& CS40L2X_PLL_REFCLK_EN_MASK) |
				((CS40L2X_PLL_REFCLK_SEL_MCLK
					<< CS40L2X_PLL_REFCLK_SEL_SHIFT)
					& CS40L2X_PLL_REFCLK_SEL_MASK));
		if (ret) {
			dev_err(dev, "Failed to sequence PLL configuration\n");
			return ret;
		}

		ret = cs40l2x_asp_config(cs40l2x);
		if (ret)
			return ret;
	}

	ret = cs40l2x_brownout_config(cs40l2x, CS40L2X_VPBR_CFG);
	if (ret)
		return ret;

	return cs40l2x_brownout_config(cs40l2x, CS40L2X_VBBR_CFG);
}

static int cs40l2x_otp_unpack(struct cs40l2x_private *cs40l2x)
{
	struct regmap *regmap = cs40l2x->regmap;
	struct device *dev = cs40l2x->dev;
	struct cs40l2x_trim trim;
	unsigned char row_offset, col_offset;
	unsigned int val, otp_map;
	unsigned int *otp_mem;
	int ret, i;

	otp_mem = kmalloc_array(CS40L2X_NUM_OTP_WORDS,
				sizeof(*otp_mem), GFP_KERNEL);
	if (!otp_mem)
		return -ENOMEM;

	ret = regmap_read(regmap, CS40L2X_OTPID, &val);
	if (ret) {
		dev_err(dev, "Failed to read OTP ID\n");
		goto err_otp_unpack;
	}

	/* hard matching against known OTP IDs */
	for (i = 0; i < CS40L2X_NUM_OTP_MAPS; i++) {
		if (cs40l2x_otp_map[i].id == val) {
			otp_map = i;
			break;
		}
	}

	/* reject unrecognized IDs, including untrimmed devices (OTP ID = 0) */
	if (i == CS40L2X_NUM_OTP_MAPS) {
		dev_err(dev, "Unrecognized OTP ID: 0x%01X\n", val);
		ret = -ENODEV;
		goto err_otp_unpack;
	}

	dev_dbg(dev, "Found OTP ID: 0x%01X\n", val);

	ret = regmap_bulk_read(regmap, CS40L2X_OTP_MEM0, otp_mem,
			CS40L2X_NUM_OTP_WORDS);
	if (ret) {
		dev_err(dev, "Failed to read OTP contents\n");
		goto err_otp_unpack;
	}

	ret = regmap_write(regmap, CS40L2X_TEST_KEY_CTL,
			CS40L2X_TEST_KEY_UNLOCK_CODE1);
	if (ret) {
		dev_err(dev, "Failed to unlock test space (step 1 of 2)\n");
		goto err_otp_unpack;
	}

	ret = regmap_write(regmap, CS40L2X_TEST_KEY_CTL,
			CS40L2X_TEST_KEY_UNLOCK_CODE2);
	if (ret) {
		dev_err(dev, "Failed to unlock test space (step 2 of 2)\n");
		goto err_otp_unpack;
	}

	row_offset = cs40l2x_otp_map[otp_map].row_start;
	col_offset = cs40l2x_otp_map[otp_map].col_start;

	for (i = 0; i < cs40l2x_otp_map[otp_map].num_trims; i++) {
		trim = cs40l2x_otp_map[otp_map].trim_table[i];

		if (col_offset + trim.size - 1 > 31) {
			/* trim straddles word boundary */
			val = (otp_mem[row_offset] &
					GENMASK(31, col_offset)) >> col_offset;
			val |= (otp_mem[row_offset + 1] &
					GENMASK(col_offset + trim.size - 33, 0))
					<< (32 - col_offset);
		} else {
			/* trim does not straddle word boundary */
			val = (otp_mem[row_offset] &
					GENMASK(col_offset + trim.size - 1,
						col_offset)) >> col_offset;
		}

		/* advance column marker and wrap if necessary */
		col_offset += trim.size;
		if (col_offset > 31) {
			col_offset -= 32;
			row_offset++;
		}

		/* skip blank trims */
		if (trim.reg == 0)
			continue;

		ret = regmap_update_bits(regmap, trim.reg,
				GENMASK(trim.shift + trim.size - 1, trim.shift),
				val << trim.shift);
		if (ret) {
			dev_err(dev, "Failed to write trim %d\n", i + 1);
			goto err_otp_unpack;
		}

		dev_dbg(dev, "Trim %d: wrote 0x%X to 0x%08X bits [%d:%d]\n",
				i + 1, val, trim.reg,
				trim.shift + trim.size - 1, trim.shift);
	}

	ret = regmap_write(regmap, CS40L2X_TEST_KEY_CTL,
			CS40L2X_TEST_KEY_RELOCK_CODE1);
	if (ret) {
		dev_err(dev, "Failed to lock test space (step 1 of 2)\n");
		goto err_otp_unpack;
	}

	ret = regmap_write(regmap, CS40L2X_TEST_KEY_CTL,
			CS40L2X_TEST_KEY_RELOCK_CODE2);
	if (ret) {
		dev_err(dev, "Failed to lock test space (step 2 of 2)\n");
		goto err_otp_unpack;
	}

	ret = 0;

err_otp_unpack:
	kfree(otp_mem);

	return ret;
}

static void cs40l2x_handle_br_data(struct device_node *br_node,
			struct cs40l2x_br_desc *br_config)
{
	int ret;
	unsigned int out_val;

	if (!br_node)
		return;

	br_config->present = true;

	br_config->enable = of_property_read_bool(br_node,
			"cirrus,br-enable");

	ret = of_property_read_u32(br_node, "cirrus,br-thld1", &out_val);
	if (!ret)
		br_config->thld1 = out_val;

	ret = of_property_read_u32(br_node, "cirrus,br-max-att", &out_val);
	if (!ret)
		br_config->max_att = out_val;

	ret = of_property_read_u32(br_node, "cirrus,br-atk-vol", &out_val);
	if (!ret)
		br_config->atk_vol = out_val;

	ret = of_property_read_u32(br_node, "cirrus,br-atk-rate", &out_val);
	if (!ret)
		br_config->atk_rate = out_val;

	ret = of_property_read_u32(br_node, "cirrus,br-wait", &out_val);
	if (!ret)
		br_config->wait = out_val;

	ret = of_property_read_u32(br_node, "cirrus,br-rel-rate", &out_val);
	if (!ret)
		br_config->rel_rate = out_val;

	br_config->mute_enable = of_property_read_bool(br_node,
			"cirrus,br-mute-enable");
}

static int cs40l2x_handle_of_data(struct i2c_client *i2c_client,
			struct cs40l2x_platform_data *pdata)
{
	struct device_node *vpbr_node, *vbbr_node;
	struct device_node *np = i2c_client->dev.of_node;
	struct device *dev = &i2c_client->dev;
	int ret;
	unsigned int out_val;

	if (!np)
		return 0;

	ret = of_property_read_u32(np, "cirrus,boost-ind-nanohenry", &out_val);
	if (ret) {
		dev_err(dev, "Boost inductor value not specified\n");
		return -EINVAL;
	}
	pdata->boost_ind = out_val;

	ret = of_property_read_u32(np, "cirrus,boost-cap-microfarad", &out_val);
	if (ret) {
		dev_err(dev, "Boost capacitance not specified\n");
		return -EINVAL;
	}
	pdata->boost_cap = out_val;

	ret = of_property_read_u32(np, "cirrus,boost-ipk-milliamp", &out_val);
	if (ret) {
		dev_err(dev, "Boost inductor peak current not specified\n");
		return -EINVAL;
	}
	pdata->boost_ipk = out_val;

	ret = of_property_read_u32(np, "cirrus,boost-ctl-millivolt", &out_val);
	if (!ret)
		pdata->boost_ctl = out_val | CS40L2X_PDATA_PRESENT;

	ret = of_property_read_u32(np, "cirrus,boost-clab-millivolt", &out_val);
	if (!ret)
		pdata->boost_clab = out_val | CS40L2X_PDATA_PRESENT;

	ret = of_property_read_u32(np, "cirrus,boost-ovp-millivolt", &out_val);
	if (!ret)
		pdata->boost_ovp = out_val;

	pdata->refclk_gpio2 = of_property_read_bool(np, "cirrus,refclk-gpio2");

	ret = of_property_read_u32(np, "cirrus,f0-default", &out_val);
	if (!ret)
		pdata->f0_default = out_val;

	ret = of_property_read_u32(np, "cirrus,f0-min", &out_val);
	if (!ret)
		pdata->f0_min = out_val;

	ret = of_property_read_u32(np, "cirrus,f0-max", &out_val);
	if (!ret)
		pdata->f0_max = out_val;

	ret = of_property_read_u32(np, "cirrus,redc-default", &out_val);
	if (!ret)
		pdata->redc_default = out_val;

	ret = of_property_read_u32(np, "cirrus,redc-min", &out_val);
	if (!ret)
		pdata->redc_min = out_val;

	ret = of_property_read_u32(np, "cirrus,redc-max", &out_val);
	if (!ret)
		pdata->redc_max = out_val;

	ret = of_property_read_u32(np, "cirrus,q-default", &out_val);
	if (!ret)
		pdata->q_default = out_val;

	ret = of_property_read_u32(np, "cirrus,q-min", &out_val);
	if (!ret)
		pdata->q_min = out_val;

	ret = of_property_read_u32(np, "cirrus,q-max", &out_val);
	if (!ret)
		pdata->q_max = out_val;

	pdata->redc_comp_disable = of_property_read_bool(np,
			"cirrus,redc-comp-disable");

	pdata->comp_disable = of_property_read_bool(np, "cirrus,comp-disable");

	pdata->dyn_f0_disable = of_property_read_bool(np,
		"cirrus,dyn-f0-disable");

	pdata->open_wt_disable = of_property_read_bool(np,
		"cirrus,open-wt-disable");

	ret = of_property_read_u32(np, "cirrus,gpio1-rise-index", &out_val);
	if (!ret)
		pdata->gpio1_rise_index = out_val;

	ret = of_property_read_u32(np, "cirrus,gpio1-fall-index", &out_val);
	if (!ret)
		pdata->gpio1_fall_index = out_val;

	ret = of_property_read_u32(np, "cirrus,gpio1-fall-timeout", &out_val);
	if (!ret)
		pdata->gpio1_fall_timeout = out_val | CS40L2X_PDATA_PRESENT;

	ret = of_property_read_u32(np, "cirrus,gpio1-mode", &out_val);
	if (!ret) {
		if (out_val > CS40L2X_GPIO1_MODE_MAX)
			dev_warn(dev, "Ignored default gpio1_mode\n");
		else
			pdata->gpio1_mode = out_val;
	}

	ret = of_property_read_u32(np, "cirrus,gpio2-rise-index", &out_val);
	if (!ret)
		pdata->gpio2_rise_index = out_val;

	ret = of_property_read_u32(np, "cirrus,gpio2-fall-index", &out_val);
	if (!ret)
		pdata->gpio2_fall_index = out_val;

	ret = of_property_read_u32(np, "cirrus,gpio3-rise-index", &out_val);
	if (!ret)
		pdata->gpio3_rise_index = out_val;

	ret = of_property_read_u32(np, "cirrus,gpio3-fall-index", &out_val);
	if (!ret)
		pdata->gpio3_fall_index = out_val;

	ret = of_property_read_u32(np, "cirrus,gpio4-rise-index", &out_val);
	if (!ret)
		pdata->gpio4_rise_index = out_val;

	ret = of_property_read_u32(np, "cirrus,gpio4-fall-index", &out_val);
	if (!ret)
		pdata->gpio4_fall_index = out_val;

	ret = of_property_read_u32(np, "cirrus,gpio-indv-enable", &out_val);
	if (!ret) {
		if (out_val > (CS40L2X_GPIO_BTNDETECT_GPIO1
				| CS40L2X_GPIO_BTNDETECT_GPIO2
				| CS40L2X_GPIO_BTNDETECT_GPIO3
				| CS40L2X_GPIO_BTNDETECT_GPIO4))
			dev_warn(dev, "Ignored default gpio_indv_enable\n");
		else
			pdata->gpio_indv_enable = out_val;
	}

	ret = of_property_read_u32(np, "cirrus,gpio-indv-pol", &out_val);
	if (!ret) {
		if (out_val > (CS40L2X_GPIO_BTNDETECT_GPIO1
				| CS40L2X_GPIO_BTNDETECT_GPIO2
				| CS40L2X_GPIO_BTNDETECT_GPIO3
				| CS40L2X_GPIO_BTNDETECT_GPIO4))
			dev_warn(dev, "Ignored default gpio_indv_pol\n");
		else
			pdata->gpio_indv_pol = out_val;
	}

	pdata->hiber_enable = of_property_read_bool(np, "cirrus,hiber-enable");

	ret = of_property_read_u32(np, "cirrus,asp-bclk-freq-hz", &out_val);
	if (!ret)
		pdata->asp_bclk_freq = out_val;

	pdata->asp_bclk_inv = of_property_read_bool(np, "cirrus,asp-bclk-inv");

	pdata->asp_fsync_inv = of_property_read_bool(np,
			"cirrus,asp-fsync-inv");

	ret = of_property_read_u32(np, "cirrus,asp-fmt", &out_val);
	if (!ret)
		pdata->asp_fmt = out_val | CS40L2X_PDATA_PRESENT;

	ret = of_property_read_u32(np, "cirrus,asp-slot-num", &out_val);
	if (!ret)
		pdata->asp_slot_num = out_val;

	ret = of_property_read_u32(np, "cirrus,asp-slot-width", &out_val);
	if (!ret)
		pdata->asp_slot_width = out_val;

	ret = of_property_read_u32(np, "cirrus,asp-samp-width", &out_val);
	if (!ret)
		pdata->asp_samp_width = out_val;

	ret = of_property_read_u32(np, "cirrus,asp-timeout", &out_val);
	if (!ret) {
		if (out_val > CS40L2X_ASP_TIMEOUT_MAX)
			dev_warn(dev, "Ignored default ASP timeout\n");
		else
			pdata->asp_timeout = out_val;
	}

	pdata->vpbr_enable = of_property_read_bool(np, "cirrus,vpbr-enable");

	pdata->vbbr_enable = of_property_read_bool(np, "cirrus,vbbr-enable");

	ret = of_property_read_u32(np, "cirrus,vpbr-thld1-millivolt", &out_val);
	if (!ret)
		pdata->vpbr_thld1 = out_val;

	ret = of_property_read_u32(np, "cirrus,vbbr-thld1-millivolt", &out_val);
	if (!ret)
		pdata->vbbr_thld1 = out_val;

	vpbr_node = of_get_child_by_name(np, "cirrus,vpbr-config");
	cs40l2x_handle_br_data(vpbr_node, &pdata->vpbr_config);
	of_node_put(vpbr_node);

	vbbr_node = of_get_child_by_name(np, "cirrus,vbbr-config");
	cs40l2x_handle_br_data(vbbr_node, &pdata->vbbr_config);
	of_node_put(vbbr_node);

	ret = of_property_read_u32(np, "cirrus,fw-id-remap", &out_val);
	if (!ret)
		pdata->fw_id_remap = out_val;

	pdata->amp_gnd_stby = of_property_read_bool(np, "cirrus,amp-gnd-stby");

	pdata->auto_recovery = of_property_read_bool(np,
			"cirrus,auto-recovery");

	return 0;
}

static const struct reg_sequence cs40l2x_basic_mode_revert[] = {
	{CS40L2X_PWR_CTRL1,		0x00000000},
	{CS40L2X_PWR_CTRL2,		0x00003321},
	{CS40L2X_LRCK_PAD_CONTROL,	0x00000007},
	{CS40L2X_SDIN_PAD_CONTROL,	0x00000007},
	{CS40L2X_AMP_DIG_VOL_CTRL,	0x00008000},
	{CS40L2X_IRQ2_MASK1,		0xFFFFFFFF},
	{CS40L2X_IRQ2_MASK2,		0xFFFFFFFF},
};

static int cs40l2x_basic_mode_exit(struct cs40l2x_private *cs40l2x)
{
	struct regmap *regmap = cs40l2x->regmap;
	struct device *dev = cs40l2x->dev;
	unsigned int val, hb_init;
	int ret, i;

	for (i = 0; i < CS40L2X_BASIC_TIMEOUT_COUNT; i++) {
		ret = regmap_read(regmap, CS40L2X_BASIC_AMP_STATUS, &val);
		if (ret) {
			dev_err(dev, "Failed to read basic-mode boot status\n");
			return ret;
		}

		if (val & CS40L2X_BASIC_BOOT_DONE)
			break;

		usleep_range(5000, 5100);
	}

	if (i == CS40L2X_BASIC_TIMEOUT_COUNT) {
		dev_err(dev, "Timed out waiting for basic-mode boot\n");
		return -ETIME;
	}

	ret = regmap_read(regmap, CS40L2X_BASIC_HALO_HEARTBEAT, &hb_init);
	if (ret) {
		dev_err(dev, "Failed to read basic-mode heartbeat\n");
		return ret;
	}

	for (i = 0; i < CS40L2X_BASIC_TIMEOUT_COUNT; i++) {
		usleep_range(5000, 5100);

		ret = regmap_read(regmap, CS40L2X_BASIC_HALO_HEARTBEAT, &val);
		if (ret) {
			dev_err(dev, "Failed to read basic-mode heartbeat\n");
			return ret;
		}

		if (val > hb_init)
			break;
	}

	if (i == CS40L2X_BASIC_TIMEOUT_COUNT) {
		dev_err(dev, "Timed out waiting for basic-mode heartbeat\n");
		return -ETIME;
	}

	ret = cs40l2x_ack_write(cs40l2x, CS40L2X_BASIC_SHUTDOWNREQUEST, 1, 0);
	if (ret)
		return ret;

	ret = regmap_read(regmap, CS40L2X_BASIC_STATEMACHINE, &val);
	if (ret) {
		dev_err(dev, "Failed to read basic-mode state\n");
		return ret;
	}

	if (val != CS40L2X_BASIC_SHUTDOWN) {
		dev_err(dev, "Unexpected basic-mode state: 0x%02X\n", val);
		return -EBUSY;
	}

	ret = regmap_read(regmap, CS40L2X_BASIC_AMP_STATUS, &val);
	if (ret) {
		dev_err(dev, "Failed to read basic-mode error status\n");
		return ret;
	}

	if (val & CS40L2X_BASIC_OTP_ERROR) {
		dev_err(dev, "Encountered basic-mode OTP error\n");
		return -EIO;
	}

	if (val & CS40L2X_BASIC_AMP_ERROR) {
		ret = cs40l2x_hw_err_rls(cs40l2x, CS40L2X_AMP_ERR);
		if (ret)
			return ret;
	}

	if (val & CS40L2X_BASIC_TEMP_RISE_WARN) {
		ret = cs40l2x_hw_err_rls(cs40l2x, CS40L2X_TEMP_RISE_WARN);
		if (ret)
			return ret;
	}

	if (val & CS40L2X_BASIC_TEMP_ERROR) {
		ret = cs40l2x_hw_err_rls(cs40l2x, CS40L2X_TEMP_ERR);
		if (ret)
			return ret;
	}

	ret = regmap_multi_reg_write(regmap, cs40l2x_basic_mode_revert,
			ARRAY_SIZE(cs40l2x_basic_mode_revert));
	if (ret) {
		dev_err(dev, "Failed to revert basic-mode fields\n");
		return ret;
	}

	return 0;
}

static const struct reg_sequence cs40l2x_rev_a0_errata[] = {
	{CS40L2X_OTP_TRIM_30,		0x9091A1C8},
	{CS40L2X_PLL_LOOP_PARAM,	0x000C1837},
	{CS40L2X_PLL_MISC_CTRL,		0x03008E0E},
	{CS40L2X_BSTCVRT_DCM_CTRL,	0x00000051},
	{CS40L2X_CTRL_ASYNC1,		0x00000004},
	{CS40L2X_IRQ1_DB3,		0x00000000},
	{CS40L2X_IRQ2_DB3,		0x00000000},
	{CS40L2X_TEST_KEY_CTL,		CS40L2X_TEST_KEY_UNLOCK_CODE1},
	{CS40L2X_TEST_KEY_CTL,		CS40L2X_TEST_KEY_UNLOCK_CODE2},
	{CS40L2X_SPKMON_RESYNC,		0x00000000},
	{CS40L2X_TEMP_RESYNC,		0x00000000},
	{CS40L2X_TEST_KEY_CTL,		CS40L2X_TEST_KEY_RELOCK_CODE1},
	{CS40L2X_TEST_KEY_CTL,		CS40L2X_TEST_KEY_RELOCK_CODE2},
	{CS40L2X_VPVBST_FS_SEL,		0x00000000},
};

static const struct reg_sequence cs40l2x_rev_b0_errata[] = {
	{CS40L2X_PLL_LOOP_PARAM,	0x000C1837},
	{CS40L2X_PLL_MISC_CTRL,		0x03008E0E},
	{CS40L2X_TEST_KEY_CTL,		CS40L2X_TEST_KEY_UNLOCK_CODE1},
	{CS40L2X_TEST_KEY_CTL,		CS40L2X_TEST_KEY_UNLOCK_CODE2},
	{CS40L2X_TEST_LBST,		CS40L2X_EXPL_MODE_EN},
	{CS40L2X_OTP_TRIM_12,		0x002F0065},
	{CS40L2X_OTP_TRIM_13,		0x00002B4F},
	{CS40L2X_SPKMON_RESYNC,		0x00000000},
	{CS40L2X_TEMP_RESYNC,		0x00000000},
	{CS40L2X_TEST_KEY_CTL,		CS40L2X_TEST_KEY_RELOCK_CODE1},
	{CS40L2X_TEST_KEY_CTL,		CS40L2X_TEST_KEY_RELOCK_CODE2},
	{CS40L2X_VPVBST_FS_SEL,		0x00000000},
};

static int cs40l2x_part_num_resolve(struct cs40l2x_private *cs40l2x)
{
	struct regmap *regmap = cs40l2x->regmap;
	struct device *dev = cs40l2x->dev;
	unsigned int val, devid, revid;
	unsigned int part_num_index, fw_id;
	int otp_timeout = CS40L2X_OTP_TIMEOUT_COUNT;
	int ret;

	while (otp_timeout > 0) {
		usleep_range(10000, 10100);

		ret = regmap_read(regmap, CS40L2X_IRQ1_STATUS4, &val);
		if (ret) {
			dev_err(dev, "Failed to read OTP boot status\n");
			return ret;
		}

		if (val & CS40L2X_OTP_BOOT_DONE)
			break;

		otp_timeout--;
	}

	if (otp_timeout == 0) {
		dev_err(dev, "Timed out waiting for OTP boot\n");
		return -ETIME;
	}

	ret = regmap_read(regmap, CS40L2X_IRQ1_STATUS3, &val);
	if (ret) {
		dev_err(dev, "Failed to read OTP error status\n");
		return ret;
	}

	if (val & CS40L2X_OTP_BOOT_ERR) {
		dev_err(dev, "Encountered fatal OTP error\n");
		return -EIO;
	}

	ret = regmap_read(regmap, CS40L2X_DEVID, &devid);
	if (ret) {
		dev_err(dev, "Failed to read device ID\n");
		return ret;
	}

	ret = regmap_read(regmap, CS40L2X_REVID, &revid);
	if (ret) {
		dev_err(dev, "Failed to read revision ID\n");
		return ret;
	}

	switch (devid) {
	case CS40L2X_DEVID_L20:
		part_num_index = 0;
		fw_id = CS40L2X_FW_ID_ORIG;

		if (revid != CS40L2X_REVID_A0)
			goto err_revid;

		ret = regmap_register_patch(regmap, cs40l2x_rev_a0_errata,
				ARRAY_SIZE(cs40l2x_rev_a0_errata));
		if (ret) {
			dev_err(dev, "Failed to apply revision %02X errata\n",
					revid);
			return ret;
		}

		ret = cs40l2x_otp_unpack(cs40l2x);
		if (ret)
			return ret;
		break;
	case CS40L2X_DEVID_L25:
		part_num_index = 1;
		fw_id = CS40L2X_FW_ID_ORIG;

		if (revid != CS40L2X_REVID_B0)
			goto err_revid;

		ret = regmap_register_patch(regmap, cs40l2x_rev_b0_errata,
				ARRAY_SIZE(cs40l2x_rev_b0_errata));
		if (ret) {
			dev_err(dev, "Failed to apply revision %02X errata\n",
					revid);
			return ret;
		}

		ret = cs40l2x_wseq_add_seq(cs40l2x, cs40l2x_rev_b0_errata,
				ARRAY_SIZE(cs40l2x_rev_b0_errata));
		if (ret) {
			dev_err(dev,
				"Failed to sequence revision %02X errata\n",
				revid);
			return ret;
		}
		break;
	case CS40L2X_DEVID_L25A:
	case CS40L2X_DEVID_L25B:
		part_num_index = devid - CS40L2X_DEVID_L25A + 2;
		fw_id = cs40l2x->fw_id_remap;

		if (revid < CS40L2X_REVID_B1)
			goto err_revid;

		ret = cs40l2x_basic_mode_exit(cs40l2x);
		if (ret)
			return ret;

		ret = regmap_register_patch(regmap, cs40l2x_rev_b0_errata,
				ARRAY_SIZE(cs40l2x_rev_b0_errata));
		if (ret) {
			dev_err(dev, "Failed to apply revision %02X errata\n",
					revid);
			return ret;
		}

		ret = cs40l2x_wseq_add_seq(cs40l2x, cs40l2x_rev_b0_errata,
				ARRAY_SIZE(cs40l2x_rev_b0_errata));
		if (ret) {
			dev_err(dev,
				"Failed to sequence revision %02X errata\n",
				revid);
			return ret;
		}
		break;
	default:
		dev_err(dev, "Unrecognized device ID: 0x%06X\n", devid);
		return -ENODEV;
	}

	cs40l2x->fw_desc = cs40l2x_firmware_match(cs40l2x, fw_id);
	if (!cs40l2x->fw_desc)
		return -EINVAL;

	dev_info(dev, "Cirrus Logic %s revision %02X\n",
			cs40l2x_part_nums[part_num_index], revid);
	cs40l2x->devid = devid;
	cs40l2x->revid = revid;

	return 0;
err_revid:
	dev_err(dev, "Unexpected revision ID for %s: %02X\n",
			cs40l2x_part_nums[part_num_index], revid);
	return -ENODEV;
}

static irqreturn_t cs40l2x_irq(int irq, void *data)
{
	struct cs40l2x_private *cs40l2x = (struct cs40l2x_private *)data;
	struct regmap *regmap = cs40l2x->regmap;
	struct device *dev = cs40l2x->dev;
	unsigned int asp_timeout = cs40l2x->pdata.asp_timeout;
	unsigned int event_reg, val;
	int event_count = 0;
	int ret, i;
	irqreturn_t ret_irq = IRQ_NONE;

	pm_runtime_get_sync(cs40l2x->dev);

	ret = regmap_read(regmap, CS40L2X_DSP1_SCRATCH1, &val);
	if (ret) {
		dev_err(dev, "Failed to read DSP scratch contents\n");
		goto err_exit;
	}

	if (val) {
		dev_err(dev, "Fatal runtime error with DSP scratch = %u\n",
				val);

		ret = cs40l2x_reset_recovery(cs40l2x);
		if (!ret)
			ret_irq = IRQ_HANDLED;

		goto err_exit;
	}

	for (i = 0; i < ARRAY_SIZE(cs40l2x_event_regs); i++) {
		/* skip disabled event notifiers */
		if (!(cs40l2x->event_control & cs40l2x_event_masks[i]))
			continue;

		event_reg = cs40l2x_dsp_reg(cs40l2x, cs40l2x_event_regs[i],
				CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
		if (!event_reg)
			goto err_exit;

		ret = regmap_read(regmap, event_reg, &val);
		if (ret) {
			dev_err(dev, "Failed to read %s\n",
					cs40l2x_event_regs[i]);
			goto err_exit;
		}

		/* any event handling goes here */
		switch (val) {
		case CS40L2X_EVENT_CTRL_NONE:
			continue;
		case CS40L2X_EVENT_CTRL_HARDWARE:
			ret = cs40l2x_hw_err_chk(cs40l2x);
			if (ret)
				goto err_exit;
			break;
		case CS40L2X_EVENT_CTRL_TRIG_STOP:
			queue_work(cs40l2x->vibe_workqueue,
					&cs40l2x->vibe_pbq_work);
			/* intentionally fall through */
		case CS40L2X_EVENT_CTRL_GPIO_STOP:
			if (asp_timeout > 0)
				hrtimer_start(&cs40l2x->asp_timer,
						ktime_set(asp_timeout / 1000,
							(asp_timeout % 1000)
								* 1000000),
						HRTIMER_MODE_REL);
			else
				queue_work(cs40l2x->vibe_workqueue,
						&cs40l2x->vibe_mode_work);
			/* intentionally fall through */
			complete(&cs40l2x->hap_done);
			break;
		case CS40L2X_EVENT_CTRL_TRIG_START:
		case CS40L2X_EVENT_CTRL_GPIO_START:
			if (completion_done(&cs40l2x->hap_done))
				reinit_completion(&cs40l2x->hap_done);

			break;
		case CS40L2X_EVENT_CTRL_GPIO1_FALL
			... CS40L2X_EVENT_CTRL_GPIO4_RISE:
		case CS40L2X_EVENT_CTRL_READY:
		case CS40L2X_EVENT_CTRL_TRIG_SUSP
			... CS40L2X_EVENT_CTRL_TRIG_RESM:
			dev_dbg(dev, "Found notifier %d in %s\n",
					val, cs40l2x_event_regs[i]);
			break;
		default:
			dev_err(dev, "Unrecognized notifier %d in %s\n",
					val, cs40l2x_event_regs[i]);
			goto err_exit;
		}

		ret = regmap_write(regmap, event_reg, CS40L2X_EVENT_CTRL_NONE);
		if (ret) {
			dev_err(dev, "Failed to acknowledge %s\n",
					cs40l2x_event_regs[i]);
			goto err_exit;
		}

		/*
		 * polling for acknowledgment as with other mailbox registers
		 * is unnecessary in this case and adds latency, so only send
		 * the wake-up command to complete the notification sequence
		 */
		ret = regmap_write(regmap, CS40L2X_MBOX_POWERCONTROL,
				CS40L2X_POWERCONTROL_WAKEUP);
		if (ret) {
			dev_err(dev, "Failed to free /ALERT output\n");
			goto err_exit;
		}

		event_count++;
	}

	if (event_count > 0)
		ret_irq = IRQ_HANDLED;

err_exit:
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret_irq;
}

static struct regmap_config cs40l2x_regmap = {
	.reg_bits = 32,
	.val_bits = 32,
	.reg_stride = 4,
	.reg_format_endian = REGMAP_ENDIAN_BIG,
	.val_format_endian = REGMAP_ENDIAN_BIG,
	.max_register = CS40L2X_LASTREG,
	.precious_reg = cs40l2x_precious_reg,
	.readable_reg = cs40l2x_readable_reg,
	.cache_type = REGCACHE_NONE,
};


static const struct mfd_cell cs40l2x_devs[] = {
	{ .name = "cs40l2x-codec" },
};

static int cs40l2x_i2c_probe(struct i2c_client *i2c_client,
				const struct i2c_device_id *id)
{
	int ret, i;
	struct cs40l2x_private *cs40l2x;
	struct device *dev = &i2c_client->dev;
	struct cs40l2x_platform_data *pdata = dev_get_platdata(dev);

	cs40l2x = devm_kzalloc(dev, sizeof(struct cs40l2x_private), GFP_KERNEL);
	if (!cs40l2x)
		return -ENOMEM;

	cs40l2x->dev = dev;
	dev_set_drvdata(dev, cs40l2x);
	i2c_set_clientdata(i2c_client, cs40l2x);

	mutex_init(&cs40l2x->lock);

	hrtimer_init(&cs40l2x->pbq_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	cs40l2x->pbq_timer.function = cs40l2x_pbq_timer;

	hrtimer_init(&cs40l2x->asp_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	cs40l2x->asp_timer.function = cs40l2x_asp_timer;

	cs40l2x->vibe_workqueue =
		alloc_ordered_workqueue("vibe_workqueue", WQ_HIGHPRI);
	if (!cs40l2x->vibe_workqueue) {
		dev_err(dev, "Failed to allocate workqueue\n");
		return -ENOMEM;
	}

	INIT_WORK(&cs40l2x->vibe_start_work, cs40l2x_vibe_start_worker);
	INIT_WORK(&cs40l2x->vibe_pbq_work, cs40l2x_vibe_pbq_worker);
	INIT_WORK(&cs40l2x->vibe_stop_work, cs40l2x_vibe_stop_worker);
	INIT_WORK(&cs40l2x->vibe_mode_work, cs40l2x_vibe_mode_worker);

	ret = device_init_wakeup(cs40l2x->dev, true);
	if (ret) {
		dev_err(dev, "Failed to initialize wakeup source\n");
		return ret;
	}

	INIT_LIST_HEAD(&cs40l2x->coeff_desc_head);
	INIT_LIST_HEAD(&cs40l2x->virtual_waveform_head);
	INIT_LIST_HEAD(&cs40l2x->pwle_segment_head);

	cs40l2x->regmap = devm_regmap_init_i2c(i2c_client, &cs40l2x_regmap);
	if (IS_ERR(cs40l2x->regmap)) {
		ret = PTR_ERR(cs40l2x->regmap);
		dev_err(dev, "Failed to allocate register map: %d\n", ret);
		return ret;
	}

	for (i = 0; i < ARRAY_SIZE(cs40l2x_supplies); i++)
		cs40l2x->supplies[i].supply = cs40l2x_supplies[i];

	cs40l2x->num_supplies = ARRAY_SIZE(cs40l2x_supplies);

	ret = devm_regulator_bulk_get(dev, cs40l2x->num_supplies,
			cs40l2x->supplies);
	if (ret) {
		dev_err(dev, "Failed to request core supplies: %d\n", ret);
		return ret;
	}

	if (pdata) {
		cs40l2x->pdata = *pdata;
	} else {
		pdata = devm_kzalloc(dev, sizeof(struct cs40l2x_platform_data),
				GFP_KERNEL);
		if (!pdata)
			return -ENOMEM;

		if (i2c_client->dev.of_node) {
			ret = cs40l2x_handle_of_data(i2c_client, pdata);
			if (ret)
				return ret;

		}
		cs40l2x->pdata = *pdata;
	}

	cs40l2x->cp_trailer_index = CS40L2X_INDEX_IDLE;

	cs40l2x->vpp_measured = -1;
	cs40l2x->ipp_measured = -1;

	cs40l2x->comp_enable = !pdata->comp_disable;
	cs40l2x->comp_enable_redc = !pdata->redc_comp_disable;
	cs40l2x->comp_enable_f0 = true;

	cs40l2x->dyn_f0_enable = !pdata->dyn_f0_disable;
	cs40l2x->open_wt_enable = !pdata->open_wt_disable;

	cs40l2x->autosuspend_delay = CS40L2X_AUTOSUSPEND_DELAY_MS;

	strlcpy(cs40l2x->wt_file,
			CS40L2X_WT_FILE_NAME_MISSING,
			CS40L2X_WT_FILE_NAME_LEN_MAX);
	strlcpy(cs40l2x->wt_date,
			CS40L2X_WT_FILE_DATE_MISSING,
			CS40L2X_WT_FILE_DATE_LEN_MAX);

	/* Virtual wavetable slots mem init */
	cs40l2x->updated_offsets = devm_kzalloc(dev,
		(CS40L2X_OWT_CALC_SIZE * sizeof(unsigned int)),
		GFP_KERNEL);
	if (!cs40l2x->updated_offsets)
		return -ENOMEM;

	cs40l2x->pbq_updated_fw_raw_wt = devm_kzalloc(dev,
		CS40L2X_WT_MAX_BIN_SIZE, GFP_KERNEL);
	if (!cs40l2x->pbq_updated_fw_raw_wt)
		return -ENOMEM;

	cs40l2x->wvfrm_lengths = devm_kzalloc(dev,
		(CS40L2X_OWT_CALC_SIZE * sizeof(unsigned int)),
		GFP_KERNEL);
	if (!cs40l2x->wvfrm_lengths)
		return -ENOMEM;

	cs40l2x->pbq_fw_raw_wt = devm_kzalloc(dev,
		CS40L2X_WT_MAX_BIN_SIZE, GFP_KERNEL);
	if (!cs40l2x->pbq_fw_raw_wt)
		return -ENOMEM;

	cs40l2x->ovwr_wav = devm_kzalloc(dev,
		sizeof(struct cs40l2x_ovwr_waveform), GFP_KERNEL);
	if (!cs40l2x->ovwr_wav)
		return -ENOMEM;

	cs40l2x->virtual_stored = true;
	cs40l2x->safe_save_state = true;

	switch (pdata->fw_id_remap) {
	case CS40L2X_FW_ID_ORIG:
	case CS40L2X_FW_ID_B1ROM:
	case CS40L2X_FW_ID_CAL:
		dev_err(dev, "Unexpected firmware ID: 0x%06X\n",
				pdata->fw_id_remap);
		return -EINVAL;
	case 0:
		cs40l2x->fw_id_remap = CS40L2X_FW_ID_REMAP;
		break;
	default:
		cs40l2x->fw_id_remap = pdata->fw_id_remap;
	}

	for (i = 0; i < CS40L2X_NUM_HW_ERRS; i++)
		cs40l2x->hw_err_mask |= cs40l2x_hw_errs[i].irq_mask;

	ret = regulator_bulk_enable(cs40l2x->num_supplies, cs40l2x->supplies);
	if (ret) {
		dev_err(dev, "Failed to enable core supplies: %d\n", ret);
		return ret;
	}

	cs40l2x->reset_gpio = devm_gpiod_get_optional(dev, "reset",
			GPIOD_OUT_LOW);
	if (IS_ERR(cs40l2x->reset_gpio))
		return PTR_ERR(cs40l2x->reset_gpio);

	/* satisfy reset pulse width specification (with margin) */
	usleep_range(2000, 2100);

	gpiod_set_value_cansleep(cs40l2x->reset_gpio, 1);

	/* satisfy control port delay specification (with margin) */
	usleep_range(1000, 1100);

	ret = cs40l2x_part_num_resolve(cs40l2x);
	if (ret)
		goto err;

	cs40l2x->asp_available = (cs40l2x->devid == CS40L2X_DEVID_L25A)
			&& pdata->asp_bclk_freq
			&& pdata->asp_slot_width
			&& pdata->asp_samp_width;

	init_completion(&cs40l2x->hap_done);

	if (cs40l2x->fw_desc->id != CS40L2X_FW_ID_ORIG && i2c_client->irq) {
		ret = devm_request_threaded_irq(dev, i2c_client->irq,
				NULL, cs40l2x_irq,
				IRQF_ONESHOT | IRQF_SHARED | IRQF_TRIGGER_LOW,
				i2c_client->name, cs40l2x);
		if (ret) {
			dev_err(dev, "Failed to request IRQ: %d\n", ret);
			goto err;
		}

		cs40l2x->event_control = CS40L2X_EVENT_HARDWARE_ENABLED
				| CS40L2X_EVENT_END_ENABLED;
	} else {
		cs40l2x->event_control = CS40L2X_EVENT_DISABLED;
	}

	if (!pdata->gpio_indv_enable
			|| cs40l2x->fw_desc->id == CS40L2X_FW_ID_ORIG) {
		cs40l2x->gpio_mask = CS40L2X_GPIO_BTNDETECT_GPIO1;

		if (cs40l2x->devid == CS40L2X_DEVID_L25B)
			cs40l2x->gpio_mask |= (CS40L2X_GPIO_BTNDETECT_GPIO2
					| CS40L2X_GPIO_BTNDETECT_GPIO3
					| CS40L2X_GPIO_BTNDETECT_GPIO4);
	} else {
		cs40l2x->gpio_mask = pdata->gpio_indv_enable;

		if (cs40l2x->devid == CS40L2X_DEVID_L25A)
			cs40l2x->gpio_mask &= ~CS40L2X_GPIO_BTNDETECT_GPIO2;
	}

	ret = cs40l2x_init(cs40l2x);
	if (ret)
		goto err;

	request_firmware_nowait(THIS_MODULE, FW_ACTION_HOTPLUG,
			cs40l2x->fw_desc->fw_file, dev, GFP_KERNEL, cs40l2x,
			cs40l2x_firmware_load);

	ret = devm_mfd_add_devices(dev, PLATFORM_DEVID_NONE, cs40l2x_devs,
				ARRAY_SIZE(cs40l2x_devs), NULL, 0, NULL);
	if (ret) {
		dev_err(dev, "Cannot register codec component\n");
		goto err;
	}

	return 0;
err:
	gpiod_set_value_cansleep(cs40l2x->reset_gpio, 0);

	regulator_bulk_disable(cs40l2x->num_supplies, cs40l2x->supplies);

	return ret;
}

static int cs40l2x_i2c_remove(struct i2c_client *i2c_client)
{
	struct cs40l2x_private *cs40l2x = i2c_get_clientdata(i2c_client);

	pm_runtime_disable(&i2c_client->dev);

	/* manually free irq ahead of destroying workqueue */
	if (cs40l2x->event_control != CS40L2X_EVENT_DISABLED)
		devm_free_irq(&i2c_client->dev, i2c_client->irq, cs40l2x);

	if (cs40l2x->vibe_init_success) {
#ifdef CONFIG_ANDROID_TIMED_OUTPUT
		hrtimer_cancel(&cs40l2x->vibe_timer);

		timed_output_dev_unregister(&cs40l2x->timed_dev);

		sysfs_remove_group(&cs40l2x->timed_dev.dev->kobj,
				&cs40l2x_dev_attr_group);
#else
		led_classdev_unregister(&cs40l2x->led_dev);

		sysfs_remove_group(&cs40l2x->dev->kobj,
				&cs40l2x_dev_attr_group);
#endif /* CONFIG_ANDROID_TIMED_OUTPUT */
	}

	hrtimer_cancel(&cs40l2x->pbq_timer);
	hrtimer_cancel(&cs40l2x->asp_timer);

	if (cs40l2x->vibe_workqueue) {
		cancel_work_sync(&cs40l2x->vibe_start_work);
		cancel_work_sync(&cs40l2x->vibe_pbq_work);
		cancel_work_sync(&cs40l2x->vibe_stop_work);
		cancel_work_sync(&cs40l2x->vibe_mode_work);

		destroy_workqueue(cs40l2x->vibe_workqueue);
	}

	device_init_wakeup(cs40l2x->dev, false);

	gpiod_set_value_cansleep(cs40l2x->reset_gpio, 0);

	regulator_bulk_disable(cs40l2x->num_supplies, cs40l2x->supplies);

	mutex_destroy(&cs40l2x->lock);

	return 0;
}

static int __maybe_unused cs40l2x_suspend(struct device *dev)
{
	struct cs40l2x_private *cs40l2x = dev_get_drvdata(dev);
	int ret = 0;

	mutex_lock(&cs40l2x->lock);

	if (cs40l2x->pdata.gpio1_mode == CS40L2X_GPIO1_MODE_AUTO
			&& cs40l2x->fw_desc->id != CS40L2X_FW_ID_CAL) {
		ret = regmap_write(cs40l2x->regmap,
				cs40l2x_dsp_reg(cs40l2x, "GPIO_ENABLE",
						CS40L2X_XM_UNPACKED_TYPE,
						cs40l2x->fw_desc->id),
				CS40L2X_GPIO1_ENABLED);
		if (ret) {
			dev_err(dev, "Failed to enable GPIO1 upon suspend\n");
			goto err_mutex;
		}
	}

	if (cs40l2x->pdata.hiber_enable
			&& cs40l2x->fw_desc->id != CS40L2X_FW_ID_CAL
			&& cs40l2x->fw_desc->id != CS40L2X_FW_ID_ORIG) {
		ret = cs40l2x_hiber_cmd_send(cs40l2x,
				CS40L2X_POWERCONTROL_HIBERNATE);
		if (ret)
			dev_err(dev, "Failed to hibernate upon suspend\n");
	}

err_mutex:
	mutex_unlock(&cs40l2x->lock);

	return ret;
}

static int __maybe_unused cs40l2x_resume(struct device *dev)
{
	struct cs40l2x_private *cs40l2x = dev_get_drvdata(dev);
	int ret = 0;

	mutex_lock(&cs40l2x->lock);

	if (cs40l2x->pdata.hiber_enable
			&& cs40l2x->fw_desc->id != CS40L2X_FW_ID_CAL
			&& cs40l2x->fw_desc->id != CS40L2X_FW_ID_ORIG) {
		ret = cs40l2x_hiber_cmd_send(cs40l2x,
				CS40L2X_POWERCONTROL_WAKEUP);
		if (ret) {
			dev_err(dev, "Failed to wake up upon resume\n");
			goto err_mutex;
		}
	}

	if (cs40l2x->pdata.gpio1_mode == CS40L2X_GPIO1_MODE_AUTO
			&& cs40l2x->fw_desc->id != CS40L2X_FW_ID_CAL) {
		ret = regmap_write(cs40l2x->regmap,
				cs40l2x_dsp_reg(cs40l2x, "GPIO_ENABLE",
						CS40L2X_XM_UNPACKED_TYPE,
						cs40l2x->fw_desc->id),
				CS40L2X_GPIO1_DISABLED);
		if (ret)
			dev_err(dev, "Failed to disable GPIO1 upon resume\n");
	}

err_mutex:
	mutex_unlock(&cs40l2x->lock);

	return ret;
}

static const struct dev_pm_ops cs40l2x_pm_ops = {
	SET_RUNTIME_PM_OPS(cs40l2x_suspend, cs40l2x_resume, NULL)
};

static const struct of_device_id cs40l2x_of_match[] = {
	{ .compatible = "cirrus,cs40l20" },
	{ .compatible = "cirrus,cs40l25" },
	{ .compatible = "cirrus,cs40l25a" },
	{ .compatible = "cirrus,cs40l25b" },
	{ }
};

MODULE_DEVICE_TABLE(of, cs40l2x_of_match);

static const struct i2c_device_id cs40l2x_id[] = {
	{ "cs40l20", 0 },
	{ "cs40l25", 1 },
	{ "cs40l25a", 2 },
	{ "cs40l25b", 3 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, cs40l2x_id);

static struct i2c_driver cs40l2x_i2c_driver = {
	.driver = {
		.name = "cs40l2x",
		.of_match_table = cs40l2x_of_match,
		.pm = &cs40l2x_pm_ops,
	},
	.id_table = cs40l2x_id,
	.probe = cs40l2x_i2c_probe,
	.remove = cs40l2x_i2c_remove,
};

module_i2c_driver(cs40l2x_i2c_driver);

MODULE_DESCRIPTION("CS40L20/CS40L25/CS40L25A/CS40L25B Haptics Driver");
MODULE_AUTHOR("Jeff LaBundy, Cirrus Logic Inc, <jeff.labundy@cirrus.com>");
MODULE_LICENSE("GPL");
