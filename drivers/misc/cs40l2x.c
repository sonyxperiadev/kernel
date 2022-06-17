// SPDX-License-Identifier: GPL-2.0
//
// CS40L20/CS40L25/CS40L25A/CS40L25B Haptics Driver
//
// Copyright (C) 2018-2020 Cirrus Logic, Inc.

#include <linux/bitops.h>
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
#include "../../include/linux/platform_data/cs40l2x.h"
#include "../../include/linux/mfd/cs40l2x.h"
#include <linux/uaccess.h>

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

static unsigned int cs40l2x_dsp_reg(struct cs40l2x_private *cs40l2x,
			const char *coeff_name, const unsigned int block_type,
			const unsigned int algo_id);

static int cs40l2x_dsp_cache(struct cs40l2x_private *cs40l2x,
			unsigned int reg, unsigned int val);

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

static inline int cs40l2x_ground_amp(struct cs40l2x_private *cs40l2x, bool gnd)
{
	unsigned int val = CS40L2X_FORCE_SPK_FREE;

	if (!cs40l2x->amp_gnd_stby)
		return 0;

	if (gnd)
		val = CS40L2X_FORCE_SPK_GND;

	return regmap_write(cs40l2x->regmap, CS40L2X_SPK_FORCE_TST_1, val);
}

static void cs40l2x_sysfs_notify(struct cs40l2x_private *cs40l2x,
		const char *attr)
{
	struct kobject *kobj;

#ifdef CONFIG_HAPTICS_CS40L2X_INPUT
	kobj = &cs40l2x->input->dev.kobj;
#elif defined CONFIG_ANDROID_TIMED_OUTPUT
	kobj = &cs40l2x->timed_dev.dev->kobj;
#else
	kobj = &cs40l2x->dev->kobj;
#endif /* CONFIG_ANDROID_TIMED_OUTPUT */

	sysfs_notify(kobj, NULL, attr);
}

struct dspmem_chunk {
	u8 *data;
	u8 *max;
	int bytes;

	u32 cache;
	int cachebits;
};

static inline struct dspmem_chunk dspmem_chunk(void *data, int size)
{
	struct dspmem_chunk ch = {
		.data = data,
		.max = data + size,
	};

	return ch;
}

static inline bool dspmem_chunk_end(struct dspmem_chunk *ch)
{
	return ch->data == ch->max;
}

static inline int dspmem_chunk_bytes(struct dspmem_chunk *ch)
{
	return ch->bytes;
}

static inline bool dspmem_chunk_valid_addr(struct dspmem_chunk *ch, void *addr)
{
	return (u8 *)addr <= ch->max;
}

static int dspmem_chunk_write(struct dspmem_chunk *ch, int nbits, u32 val)
{
	int nwrite, i;

	nwrite = min(24 - ch->cachebits, nbits);

	ch->cache <<= nwrite;
	ch->cache |= val >> (nbits - nwrite);
	ch->cachebits += nwrite;
	nbits -= nwrite;

	if (ch->cachebits == 24) {
		if (dspmem_chunk_end(ch))
			return -ENOSPC;

		ch->cache &= 0xFFFFFF;
		for (i = 0; i < sizeof(ch->cache); i++, ch->cache <<= 8)
			*ch->data++ = (ch->cache & 0xFF000000) >> 24;

		ch->bytes += sizeof(ch->cache);
		ch->cachebits = 0;
	}

	if (nbits)
		return dspmem_chunk_write(ch, nbits, val);

	return 0;
}

static int dspmem_chunk_flush(struct dspmem_chunk *ch)
{
	if (!ch->cachebits)
		return 0;

	return dspmem_chunk_write(ch, 24 - ch->cachebits, 0);
}

static int dspmem_chunk_read(struct dspmem_chunk *ch, int nbits)
{
	int nread, i;
	u32 result;

	if (!ch->cachebits) {
		if (dspmem_chunk_end(ch))
			return -ENOSPC;

		ch->cache = 0;
		ch->cachebits = 24;

		for (i = 0; i < sizeof(ch->cache); i++, ch->cache <<= 8)
			ch->cache |= *ch->data++;

		ch->bytes += sizeof(ch->cache);
	}

	nread = min(ch->cachebits, nbits);
	nbits -= nread;

	result = ch->cache >> (32 - nread);
	ch->cache <<= nread;
	ch->cachebits -= nread;

	if (nbits)
		result = (result << nbits) | dspmem_chunk_read(ch, nbits);

	return result;
}

static int cs40l2x_write_comp(struct cs40l2x_private *cs40l2x, void *buf,
			      int size, struct wt_type10_comp *wave)
{
	struct dspmem_chunk ch = dspmem_chunk(buf, size);
	int i;

	dspmem_chunk_write(&ch, 24, wave->wlength);
	dspmem_chunk_write(&ch, 8, 0); // padding
	dspmem_chunk_write(&ch, 8, wave->nsections);
	dspmem_chunk_write(&ch, 8, wave->repeat);

	for (i = 0; i < wave->nsections; i++) {
		dspmem_chunk_write(&ch, 8, wave->sections[i].amplitude);
		dspmem_chunk_write(&ch, 8, wave->sections[i].index);
		dspmem_chunk_write(&ch, 8, wave->sections[i].repeat);
		dspmem_chunk_write(&ch, 8, wave->sections[i].flags);
		dspmem_chunk_write(&ch, 16, wave->sections[i].delay);

		if (wave->sections[i].flags & WT_T10_FLAG_DURATION) {
			dspmem_chunk_write(&ch, 8, 0); // padding
			dspmem_chunk_write(&ch, 16, wave->sections[i].duration);
		}
	}

	return dspmem_chunk_bytes(&ch);
}

static int cs40l2x_write_pwle(struct cs40l2x_private *cs40l2x, void *buf,
			      int size, struct wt_type12_pwle *wave)
{
	struct dspmem_chunk ch = dspmem_chunk(buf, size);
	int i;

	dspmem_chunk_write(&ch, 24, wave->wlength);
	dspmem_chunk_write(&ch, 8, wave->repeat);
	dspmem_chunk_write(&ch, 12, wave->wait);
	dspmem_chunk_write(&ch, 8, wave->nsections);

	for (i = 0; i < wave->nsections; i++) {
		dspmem_chunk_write(&ch, 16, wave->sections[i].time);
		dspmem_chunk_write(&ch, 12, wave->sections[i].level);
		dspmem_chunk_write(&ch, 12, wave->sections[i].frequency);
		dspmem_chunk_write(&ch, 8, wave->sections[i].flags);

		if (wave->sections[i].flags & WT_T12_FLAG_AMP_REG)
			dspmem_chunk_write(&ch, 24, wave->sections[i].vbtarget);
	}

	dspmem_chunk_flush(&ch);

	return dspmem_chunk_bytes(&ch);
}

static int cs40l2x_write_header(struct cs40l2x_private *cs40l2x, void *buf,
				int size, struct wt_entry *entry)
{
	struct dspmem_chunk ch = dspmem_chunk(buf, size);

	dspmem_chunk_write(&ch, 16, entry->flags);
	dspmem_chunk_write(&ch, 8, entry->type);
	dspmem_chunk_write(&ch, 24, entry->index);
	dspmem_chunk_write(&ch, 24, entry->size);

	return dspmem_chunk_bytes(&ch);
}

static int cs40l2x_write_wavetable(struct cs40l2x_private *cs40l2x, void *buf,
				   int size, struct wt_wavetable *table)
{
	struct dspmem_chunk ch = dspmem_chunk(buf, size);
	struct wt_entry *entry = table->waves;
	int i, index = (table->nwaves * WT_ENTRY_SIZE_WORDS) + 1;
	u32 *data = buf;

	for (i = 0; i < table->nwaves + 1; i++, entry++) {
		dspmem_chunk_write(&ch, 16, entry->flags);
		dspmem_chunk_write(&ch, 8, entry->type);

		if (entry->type == WT_TYPE_TERMINATOR)
			break;

		entry->index = index;

		dspmem_chunk_write(&ch, 24, entry->index);
		dspmem_chunk_write(&ch, 24, entry->size);

		if (!dspmem_chunk_valid_addr(&ch, data + index + entry->size))
			return -EINVAL;

		if (entry->data)
			memcpy(data + index, entry->data,
			       entry->size * sizeof(*data));
		else
			memset(data + index, 0, entry->size * sizeof(*data));

		index += entry->size;
	}

	if (i != table->nwaves)
		return -E2BIG;

	return index * sizeof(*data);
}

static int cs40l2x_read_wavetable(struct cs40l2x_private *cs40l2x, void *buf,
				  int size, struct wt_wavetable *table)
{
	struct dspmem_chunk ch = dspmem_chunk(buf, size);
	struct wt_entry *entry = table->waves;
	u32 *data = buf, *max = buf;
	int i;

	for (i = 0; i < ARRAY_SIZE(table->waves); i++, entry++) {
		entry->flags = dspmem_chunk_read(&ch, 16);
		entry->type = dspmem_chunk_read(&ch, 8);

		if (entry->type == WT_TYPE_TERMINATOR) {
			table->nwaves = i;
			table->bytes = max(dspmem_chunk_bytes(&ch),
					   (int)((void *)max - buf));

			return table->bytes;
		}

		entry->index = dspmem_chunk_read(&ch, 24);
		entry->size = dspmem_chunk_read(&ch, 24);
		entry->data = data + entry->index;

		if (data + entry->index + entry->size > max) {
			max = data + entry->index + entry->size;

			if (!dspmem_chunk_valid_addr(&ch, max))
				return -EINVAL;
		}

		dev_info(cs40l2x->dev,
			"Wave %03d (Type: 0x%02x, Flags: 0x%04x, Index: % 6d, Size: % 4d, Data: 0x%06x)\n",
			i, entry->type, entry->flags, entry->index, entry->size,
			be32_to_cpu(*((unsigned int *)entry->data)));
	}

	return -E2BIG;
}

static inline struct wt_entry *
	cs40l2x_get_wave(struct cs40l2x_private *cs40l2x, int index)
{
	if (index < cs40l2x->wt_xm.nwaves)
		return &cs40l2x->wt_xm.waves[index];
	else
		return &cs40l2x->wt_ym.waves[index - cs40l2x->wt_xm.nwaves];
}

static int cs40l2x_get_wlength(struct cs40l2x_private *cs40l2x, int index)
{
	struct wt_entry *entry = cs40l2x_get_wave(cs40l2x, index);
	struct dspmem_chunk ch;

	switch (entry->type) {
	case WT_TYPE_V6_PCM_F0_REDC:
	case WT_TYPE_V6_PCM_F0_REDC_VAR:
	case WT_TYPE_V6_PWLE:
		break;
	case WT_TYPE_V6_COMPOSITE:
		dev_err(cs40l2x->dev, "Nested composites not allowed\n");
		return -EINVAL;
	default:
		dev_err(cs40l2x->dev, "Can't size waveform: %x\n", entry->type);
		return -EINVAL;
	}

	ch = dspmem_chunk(entry->data, sizeof(u32));

	return dspmem_chunk_read(&ch, 24);
}

static void cs40l2x_set_state(struct cs40l2x_private *cs40l2x, bool state)
{
	if (cs40l2x->vibe_state != state) {
		cs40l2x->vibe_state = state;
		cs40l2x_sysfs_notify(cs40l2x, "vibe_state");
	}
}

static void cs40l2x_set_gpio_event(struct cs40l2x_private *cs40l2x, bool value)
{
	if (cs40l2x->gpio_event != value) {
		cs40l2x->gpio_event = value;
		cs40l2x_sysfs_notify(cs40l2x, "gpio_event");
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
	/* If YM exists, must add to end of YM to keep index order */
	if (!cs40l2x->wt_ym.nwaves && size <= wt_get_space(&cs40l2x->wt_xm)) {
		cs40l2x->xm_append = true;

		return 0;
	}

	if (size <= wt_get_space(&cs40l2x->wt_ym)) {
		cs40l2x->xm_append = false;

		return 0;
	}

	return -ENOSPC;
}

static int cs40l2x_update_existing_block(struct cs40l2x_private *cs40l2x,
					 unsigned int comp_size, bool is_xm)
{
	struct wt_entry template = {
		.type = WT_TYPE_V6_COMPOSITE,
		.flags = 0,
		.index = 0,
		.size = comp_size / CS40L2X_WT_NUM_VIRT_SLOTS / sizeof(u32),
	};
	struct wt_wavetable *wt = &cs40l2x->wt_ym;
	int i, ret;

	if (is_xm)
		wt = &cs40l2x->wt_xm;

	for (i = 0; i < CS40L2X_WT_NUM_VIRT_SLOTS; i++, wt->nwaves++)
		wt->waves[wt->nwaves] = template;

	wt->waves[wt->nwaves].type = WT_TYPE_TERMINATOR;
	wt->waves[wt->nwaves].flags = WT_FLAG_TERMINATOR;

	ret = cs40l2x_write_wavetable(cs40l2x, cs40l2x->pbq_updated_fw_raw_wt,
				      CS40L2X_WT_MAX_BIN_SIZE, wt);
	if (ret < 0) {
		dev_err(cs40l2x->dev, "Failed to write new wavetable: %d\n", ret);
		return ret;
	}

	cs40l2x->updated_block_size = ret;

	return 0;
}

static int cs40l2x_write_virtual_waveform(struct cs40l2x_private *cs40l2x,
					  unsigned int index, bool is_gpio,
					  bool is_rise, bool over_write)
{
	unsigned int size_in_bytes = 0;
	struct cs40l2x_virtual_waveform *virtual_wav;
	unsigned int wtype, wflags;
	int hreg, dreg;
	struct wt_entry *entry;
	uint8_t raw_header[WT_ENTRY_SIZE_BYTES];
	char *raw_waveform_data;
	bool is_xm;
	int ret;

	if (over_write) {
		is_xm = cs40l2x->ovwr_wav->is_xm;
		wtype = cs40l2x->ovwr_wav->wvfrm_type;
		wflags = cs40l2x->ovwr_wav->wvfrm_feature;
		size_in_bytes = cs40l2x->ovwr_wav->data_len;
		raw_waveform_data = cs40l2x->ovwr_wav->data;
	} else {
		list_for_each_entry(virtual_wav,
				    &cs40l2x->virtual_waveform_head, list) {
			if (virtual_wav->index != index)
				continue;

			is_xm = virtual_wav->is_xm;
			wtype = virtual_wav->wvfrm_type;
			wflags = virtual_wav->wvfrm_feature;
			size_in_bytes = virtual_wav->data_len;
			raw_waveform_data = virtual_wav->data;
			break;
		}

		if (size_in_bytes == 0) {
			dev_err(cs40l2x->dev,
				"Unable to find index in virtual list\n");
			return -EINVAL;
		}
	}

	if (is_gpio) {
		if (is_rise) {
			cs40l2x->loaded_gpio_index[CS40L2X_GPIO_RISE] = index;
			index = cs40l2x->virtual_gpio1_rise_slot;
		} else {
			cs40l2x->loaded_gpio_index[CS40L2X_GPIO_FALL] = index;
			index = cs40l2x->virtual_gpio1_fall_slot;
		}
	} else {
		cs40l2x->loaded_virtual_index = index;
		index = cs40l2x->virtual_slot_index;
	}

	if (is_xm) {
		entry = cs40l2x->wt_xm.waves;

		hreg = cs40l2x_dsp_reg(cs40l2x, "WAVETABLE",
				       CS40L2X_XM_UNPACKED_TYPE,
				       CS40L2X_ALGO_ID_VIBE);
	} else {
		entry = cs40l2x->wt_ym.waves;
		index -= cs40l2x->wt_xm.nwaves;

		hreg = cs40l2x_dsp_reg(cs40l2x, "WAVETABLEYM",
				       CS40L2X_YM_UNPACKED_TYPE,
				       CS40L2X_ALGO_ID_VIBE);
	}

	if (!hreg) {
		dev_err(cs40l2x->dev, "Failed to locate wavetable\n");
		return -EINVAL;
	}

	dreg = hreg;
	hreg += index * WT_ENTRY_SIZE_BYTES;
	dreg += entry[index].index * sizeof(u32);

	entry[index].size = size_in_bytes / sizeof(u32);
	entry[index].type = wtype;
	entry[index].flags = wflags;
	entry[index].data = raw_waveform_data;

	cs40l2x_write_header(cs40l2x, raw_header, ARRAY_SIZE(raw_header),
			     &entry[index]);

	ret = cs40l2x_raw_write(cs40l2x, hreg, raw_header,
				ARRAY_SIZE(raw_header), CS40L2X_MAX_WLEN);
	if (ret) {
		dev_err(cs40l2x->dev, "Failed to sync header: %d\n", ret);
		return ret;
	}

	ret = cs40l2x_raw_write(cs40l2x, dreg, entry[index].data,
				entry[index].size * sizeof(u32),
				CS40L2X_MAX_WLEN);
	if (ret) {
		dev_err(cs40l2x->dev, "Failed to sync waveform: %d\n", ret);
		return ret;
	}

	return 0;
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

static int cs40l2x_add_wt_slots(struct cs40l2x_private *cs40l2x,
	unsigned int *is_xm)
{
	unsigned int wt_open_bytes = CS40L2X_PACKED_BYTES_MAX +
		(WT_ENTRY_SIZE_BYTES * CS40L2X_WT_NUM_VIRT_SLOTS);
	unsigned int comp_size;
	int no_space = -ENOSPC;

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

	cs40l2x->num_waves = cs40l2x->wt_xm.nwaves + cs40l2x->wt_ym.nwaves;

	cs40l2x->virtual_slot_index = ((cs40l2x->num_waves +
		CS40L2X_WT_NUM_VIRT_SLOTS) - 1);
	cs40l2x->virtual_gpio1_fall_slot =
		cs40l2x->virtual_slot_index - 1;
	cs40l2x->virtual_gpio1_rise_slot =
		cs40l2x->virtual_gpio1_fall_slot - 1;

	comp_size = (wt_open_bytes -
		(WT_ENTRY_SIZE_BYTES * CS40L2X_WT_NUM_VIRT_SLOTS));
	cs40l2x->comp_bytes = comp_size;

	cs40l2x->display_pwle_segs =
		(((cs40l2x->comp_bytes / CS40L2X_WT_NUM_VIRT_SLOTS) -
		CS40L2X_PWLE_NON_SEG_BYTES) / CS40L2X_PWLE_MAX_SEG_BYTES);

	*is_xm = cs40l2x->xm_append;

	return cs40l2x_update_existing_block(cs40l2x, comp_size, *is_xm);
}

static int cs40l2x_convert_and_save_comp_data(struct cs40l2x_private *cs40l2x,
					      bool over_write)
{
	unsigned int comp_size;
	char *raw_composite_data;
	int ret = 0;

	raw_composite_data = kzalloc(CS40L2X_SINGLE_PACKED_MAX, GFP_KERNEL);
	if (!raw_composite_data)
		return -ENOMEM;

	comp_size = cs40l2x_write_comp(cs40l2x, raw_composite_data,
				       CS40L2X_SINGLE_PACKED_MAX,
				       &cs40l2x->pbq_comp);
	if (comp_size < 0) {
		ret = comp_size;
		goto err_free;
	}

	if (comp_size > (cs40l2x->comp_bytes / CS40L2X_WT_NUM_VIRT_SLOTS)) {
		dev_err(cs40l2x->dev, "Waveform size exceeds available space\n");
		return -ENOSPC;
	}

	if (over_write)
		cs40l2x_save_waveform_to_ovwr_struct(cs40l2x,
			CS40L2X_WT_TYPE_10_COMP_FILE, 0, comp_size,
			raw_composite_data);
	else
		ret = cs40l2x_add_waveform_to_virtual_list(cs40l2x,
			CS40L2X_WT_TYPE_10_COMP_FILE, 0,
			comp_size, raw_composite_data);

err_free:
	kfree(raw_composite_data);

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

static ssize_t cs40l2x_cp_trigger_index_impl(struct cs40l2x_private *cs40l2x,
					     unsigned int index)
{
	bool gpio_pol, gpio_rise = false;
	int ret = 0;
	unsigned int reg;
	unsigned int gpio_index = CS40L2X_GPIO_FALL;
	unsigned int gpio_slot = cs40l2x->virtual_gpio1_fall_slot;

	if ((index == CS40L2X_INDEX_PBQ_SAVE) ||
		(index == CS40L2X_INDEX_OVWR_SAVE) ||
		(index == CS40L2X_INDEX_GP1F_OVWR) ||
		(index == CS40L2X_INDEX_GP1R_OVWR)) {
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
	}

	switch (index) {
	case CS40L2X_INDEX_PBQ_SAVE:
		ret = cs40l2x_convert_and_save_comp_data(cs40l2x, false);
		if (ret)
			dev_err(cs40l2x->dev, "Unable to save virtual waveform.\n");
		/* After save or save attempt, reset flag */
		cs40l2x->queue_stored = false;
		break;
	case CS40L2X_INDEX_OVWR_SAVE:
		if (cs40l2x->last_type_entered ==
			CS40L2X_WT_TYPE_10_COMP_FILE) {
			ret = cs40l2x_convert_and_save_comp_data(cs40l2x, true);
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
	case CS40L2X_INDEX_GP1R_OVWR:
		gpio_rise = true;
		gpio_index = CS40L2X_GPIO_RISE;
		gpio_slot = cs40l2x->virtual_gpio1_rise_slot;
		/* Intentional fall through */
	case CS40L2X_INDEX_GP1F_OVWR:
		if (cs40l2x->last_type_entered ==
			CS40L2X_WT_TYPE_10_COMP_FILE) {
			ret = cs40l2x_convert_and_save_comp_data(cs40l2x, true);
			if (ret) {
				dev_err(cs40l2x->dev, "Unable to convert waveform.\n");
				cs40l2x->queue_stored = false;
				break;
			}
		}
		ret = cs40l2x_write_virtual_waveform(cs40l2x, index,
			true, gpio_rise, true);
		if (ret) {
			dev_err(cs40l2x->dev, "Unable to write waveform.\n");
			cs40l2x->queue_stored = false;
			break;
		}

		cs40l2x->virtual_gpio_index[gpio_index] = index;

		gpio_pol = cs40l2x->pdata.gpio_indv_pol &
			(1 << (CS40L2X_INDEXBUTTONPRESS1 >> 2));
		reg = cs40l2x_dsp_reg(cs40l2x,
				gpio_pol ^ gpio_rise ?
					"INDEXBUTTONPRESS" :
					"INDEXBUTTONRELEASE",
				CS40L2X_XM_UNPACKED_TYPE,
				cs40l2x->fw_desc->id);
		if (!reg) {
			dev_err(cs40l2x->dev, "Failed to find control.\n");
			cs40l2x->queue_stored = false;
			break;
		}

		ret = regmap_write(cs40l2x->regmap, reg, gpio_slot);
		if (ret) {
			dev_err(cs40l2x->dev, "Unable to set GPIO1 index.\n");
			cs40l2x->queue_stored = false;
			break;
		}

		ret = cs40l2x_dsp_cache(cs40l2x, reg, gpio_slot);
		if (ret)
			dev_err(cs40l2x->dev, "GPIO1 index cache failed.\n");
		/* After save or save attempt, reset queue_stored flag */
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
		if (cs40l2x->fw_desc->id == CS40L2X_FW_ID_CAL) {
			if (cs40l2x->cal_disabled_owt) {
				cs40l2x->open_wt_enable = true;
				cs40l2x->cal_disabled_owt = false;
			}
			ret = cs40l2x_firmware_swap(cs40l2x,
					cs40l2x->fw_id_remap);
		}
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

		if (cs40l2x->fw_desc->id == CS40L2X_FW_ID_CAL) {
			if (cs40l2x->cal_disabled_owt) {
				cs40l2x->open_wt_enable = true;
				cs40l2x->cal_disabled_owt = false;
			}
			ret = cs40l2x_firmware_swap(cs40l2x,
					cs40l2x->fw_id_remap);
		}
	}
	if (ret)
		goto err_exit;

	cs40l2x->cp_trigger_index = index;

err_exit:
	if (!cs40l2x->virtual_stored)
		cs40l2x->virtual_stored = true;

	return ret;
}

static ssize_t cs40l2x_cp_trigger_index_store(struct device *dev,
					      struct device_attribute *attr,
					      const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	unsigned int index;
	int ret;

	ret = kstrtou32(buf, 10, &index);
	if (ret)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_cp_trigger_index_impl(cs40l2x, index);

	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	if (ret)
		return ret;

	return count;
}

static ssize_t cs40l2x_cp_trigger_queue_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	struct wt_type10_comp_section *section = cs40l2x->pbq_comp.sections;
	int i, len = 0;

	mutex_lock(&cs40l2x->lock);

	for (i = 0; i < cs40l2x->pbq_comp.nsections; i++, section++) {
		if (section->repeat == WT_REPEAT_LOOP_MARKER)
			len += snprintf(buf + len, PAGE_SIZE - len, "!!, ");

		if (section->amplitude)
			len += snprintf(buf + len, PAGE_SIZE - len, "%d.%d, ",
					section->index, section->amplitude);

		if (section->delay)
			len += snprintf(buf + len, PAGE_SIZE - len, "%d, ",
					section->delay);

		if (section->repeat && section->repeat != WT_REPEAT_LOOP_MARKER)
			len += snprintf(buf + len, PAGE_SIZE - len, "%d!!, ",
					section->repeat);
	}

	switch (cs40l2x->pbq_comp.repeat) {
	case WT_REPEAT_LOOP_MARKER:
		len += snprintf(buf + len, PAGE_SIZE - len, "~\n");
		break;
	case 0:
		len -= 2; // Remove ", " from end of string
		len += snprintf(buf + len, PAGE_SIZE - len, "\n");
		break;
	default:
		len += snprintf(buf + len, PAGE_SIZE - len, "%d!\n",
				cs40l2x->pbq_comp.repeat);
	}

	mutex_unlock(&cs40l2x->lock);

	return len;
}

static int cs40l2x_comp_finalise_section(struct cs40l2x_private *cs40l2x)
{
	struct wt_type10_comp *comp = &cs40l2x->pbq_comp;
	struct wt_type10_comp_section *sec = &comp->sections[comp->nsections];
	int slen = 0;

	if (sec->index) {
		slen = cs40l2x_get_wlength(cs40l2x, sec->index);
		if (slen < 0)
			return slen;

		if (slen & WT_WAVELEN_INDEFINITE) {
			if (!(sec->flags & WT_T10_FLAG_DURATION)) {
				dev_err(cs40l2x->dev, "Indefinite PBQ entry needs duration\n");
				return -EINVAL;
			}

			slen = WT_WAVELEN_MAX;
		} else {
			slen &= WT_WAVELEN_MAX;
		}
	}

	slen += sec->delay * 8;

	if (sec->flags & WT_T10_FLAG_DURATION)
		slen = min(slen, 2 * sec->duration);

	comp->wlength += slen;
	comp->nsections++;

	if (comp->nsections == WT_MAX_SECTIONS)
		return -E2BIG;

	return slen;
}

static ssize_t cs40l2x_cp_trigger_queue_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	struct i2c_client *i2c_client = to_i2c_client(cs40l2x->dev);
	struct wt_type10_comp *comp = &cs40l2x->pbq_comp;
	struct wt_type10_comp_section *section;
	char *pbq_str, *token, *cur;
	unsigned int num_waves = cs40l2x->num_waves;
	unsigned int index, amp, val;
	unsigned int inner_samples = 0;
	bool inner_loop = false;
	int ret;

	pbq_str = kstrndup(buf, count, GFP_KERNEL);
	if (!pbq_str)
		return -ENOMEM;

	disable_irq(i2c_client->irq);

	mutex_lock(&cs40l2x->lock);

	cs40l2x->queue_stored = false;

	if (cs40l2x->virtual_bin)
		num_waves = cs40l2x->num_waves - CS40L2X_WT_NUM_VIRT_SLOTS;

	if (!cs40l2x->virtual_stored) {
		dev_err(dev, "Unsafe condition encountered.\n");
		ret = -EINVAL;
		goto err_mutex;
	}

	memset(comp, 0, sizeof(*comp));
	section = comp->sections;

	cur = pbq_str;

	while ((token = strsep(&cur, ","))) {
		token = strim(token);

		/* loop specifier */
		if (!strcmp(token, "~")) {
			if (comp->repeat) {
				dev_err(cs40l2x->dev, "Duplicate outer loop specifier\n");
				ret = -EINVAL;
				goto err_mutex;
			}

			comp->repeat = WT_REPEAT_LOOP_MARKER;

		/* inner loop start: "!!" */
		} else if (!strcmp(token, "!!")) {
			if (inner_loop) {
				dev_err(cs40l2x->dev, "Nested inner loop specifier\n");
				ret = -EINVAL;
				goto err_mutex;
			}

			if (section->amplitude || section->delay) {
				ret = cs40l2x_comp_finalise_section(cs40l2x);
				if (ret < 0)
					goto err_mutex;
				section++;
			}

			section->repeat = WT_REPEAT_LOOP_MARKER;
			inner_loop = true;

		/* inner loop stop: "n!!" */
		} else if (strstr(token, "!!")) {
			if (!inner_loop) {
				dev_err(cs40l2x->dev, "Inner loop with no start\n");
				ret = -EINVAL;
				goto err_mutex;
			}

			ret = kstrtou32(strsep(&token, "!"), 10, &val);
			if (ret) {
				dev_err(cs40l2x->dev,
					"Failed to parse inner loop specifier: %d\n",
					ret);
				goto err_mutex;
			}

			section->repeat = val;
			ret = cs40l2x_comp_finalise_section(cs40l2x);
			if (ret < 0)
				goto err_mutex;
			section++;

			if (inner_loop)
				comp->wlength += (inner_samples + ret) * val;

			inner_loop = false;
			inner_samples = 0;

		/* repetition specifier */
		} else if (strchr(token, '!')) {
			if (comp->repeat) {
				dev_err(cs40l2x->dev, "Duplicate outer loop specifier\n");
				ret = -EINVAL;
				goto err_mutex;
			}

			ret = kstrtou32(strsep(&token, "!"), 10, &val);
			if (ret) {
				dev_err(cs40l2x->dev,
					"Failed to parse outer loop specifier: %d\n",
					ret);
				goto err_mutex;
			}

			comp->repeat = val;

		/* waveform specifier */
		} else if (strchr(token, '.')) {
			ret = sscanf(token, "%u.%u.%u", &index, &amp, &val);

			if (ret < 2) {
				dev_err(cs40l2x->dev,
					"Failed to parse waveform: %d\n", ret);
				ret = -EINVAL;
				goto err_mutex;
			}

			if ((ret == 2 && index == 0) || index >= num_waves) {
				dev_err(cs40l2x->dev, "Invalid waveform index\n");
				ret = -EINVAL;
				goto err_mutex;
			}
			if (amp == 0 || amp > CS40L2X_PBQ_SCALE_MAX) {
				dev_err(cs40l2x->dev, "Invalid waveform amplitude\n");
				ret = -EINVAL;
				goto err_mutex;
			}

			if (ret == 3) {
				if (!cs40l2x->comp_dur_min_fw) {
					dev_err(cs40l2x->dev, "Composite duration not supported by firmware\n");
					ret = -EINVAL;
					goto err_mutex;
				}

				if (val != CS40L2X_PWLE_INDEF_TIME_VAL) {
					if (val > CS40L2X_PWLE_MAX_TIME_VAL) {
						dev_err(cs40l2x->dev, "Invalid duration: 0 to 16383ms, or 65535\n");
						ret = -EINVAL;
						goto err_mutex;
					}

					val *= 4; /* Time stored in 1/4 ms */
				}

				section->flags |= WT_T10_FLAG_DURATION;
			} else {
				val = 0;
			}

			if (section->amplitude || section->delay) {
				ret = cs40l2x_comp_finalise_section(cs40l2x);
				if (ret < 0)
					goto err_mutex;
				section++;

				if (inner_loop)
					inner_samples += ret;
			}

			section->index = index;
			section->amplitude = amp;
			section->duration = val;

		/* delay specifier */
		} else {
			ret = kstrtou32(token, 10, &val);
			if (ret) {
				dev_err(cs40l2x->dev,
					"Failed to parse duration: %d\n", ret);
				goto err_mutex;
			}

			if (val > CS40L2X_PBQ_DELAY_MAX) {
				dev_err(cs40l2x->dev, "Delay too long\n");
				ret = -EINVAL;
				goto err_mutex;
			}

			if (section->delay) {
				ret = cs40l2x_comp_finalise_section(cs40l2x);
				if (ret < 0)
					goto err_mutex;
				section++;

				if (inner_loop)
					inner_samples += ret;
			}

			section->delay = val;
		}
	}

	if (section->amplitude || section->delay) {
		ret = cs40l2x_comp_finalise_section(cs40l2x);
		if (ret < 0)
			goto err_mutex;
	}

	if (comp->repeat == WT_REPEAT_LOOP_MARKER) {
		comp->wlength = WT_WAVELEN_INDEFINITE;
	} else {
		comp->wlength *= comp->repeat + 1;
		clamp_t(unsigned int, comp->wlength, 0, WT_WAVELEN_MAX);
	}
	comp->wlength |= WT_WAVELEN_CALCULATED;

	cs40l2x->last_type_entered = CS40L2X_WT_TYPE_10_COMP_FILE;
	cs40l2x->queue_stored = true;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);

	kfree(pbq_str);

	enable_irq(i2c_client->irq);

	return ret;
}

static int cs40l2x_save_packed_pwle_data(struct cs40l2x_private *cs40l2x,
					 struct wt_type12_pwle *pwle,
					 unsigned int feature, bool save)
{
	unsigned int zero_pad_size;
	char *zero_pad_data;
	int ret;

	zero_pad_data = kzalloc(CS40L2X_PWLE_BYTES_MAX, GFP_KERNEL);
	if (!zero_pad_data)
		return -ENOMEM;

	ret = cs40l2x_write_pwle(cs40l2x, zero_pad_data,
				 CS40L2X_PWLE_BYTES_MAX, pwle);

	if (ret > (cs40l2x->comp_bytes / CS40L2X_WT_NUM_VIRT_SLOTS)) {
		dev_err(cs40l2x->dev, "PWLE size exceeds available space\n");
		return -ENOSPC;
	}

	zero_pad_size = ret;

	if (save) {
		ret = cs40l2x_add_waveform_to_virtual_list(cs40l2x,
				CS40L2X_WT_TYPE_12_PWLE_FILE,
				feature, zero_pad_size, zero_pad_data);
		if (!ret)
			cs40l2x->num_virtual_pwle_waves++;
	} else {
		cs40l2x_save_waveform_to_ovwr_struct(cs40l2x,
				CS40L2X_WT_TYPE_12_PWLE_FILE,
				feature, zero_pad_size, zero_pad_data);
		ret = 0;
	}

	kfree(zero_pad_data);

	return ret;
}

static int cs40l2x_pwle_repeat_entry(struct cs40l2x_private *cs40l2x,
				     char *token, struct wt_type12_pwle *pwle)
{
	unsigned int val;
	int ret;

	ret = kstrtou32(token, 10, &val);
	if (ret)
		return ret;

	if (val > CS40L2X_PWLE_MAX_RP_VAL) {
		dev_err(cs40l2x->dev, "Valid Repeat: 0 to 255\n");
		return -EINVAL;
	}

	pwle->repeat = val;

	return ret;
}

static int cs40l2x_parse_float(char *frac, int *result, int nfracdigits,
			       int min, int max)
{
	char *inte, convert[] = "000000000";
	int ninte, nfrac = 0;
	int ret;

	if (strlen(frac) > strlen(convert))
		return -EOVERFLOW;

	inte = strsep(&frac, ".");

	ninte = strlen(inte);
	if (frac)
		nfrac = strlen(frac);

	memcpy(convert, inte, ninte);
	memcpy(convert + ninte, frac, min(nfrac, nfracdigits));
	convert[ninte + nfracdigits] = 0;

	ret = kstrtoint(convert, 10, result);
	if (ret)
		return ret;

	if (*result < min || *result > max)
		return -ERANGE;

	return 0;
}

static int cs40l2x_pwle_wait_time_entry(struct cs40l2x_private *cs40l2x,
					char *token, struct wt_type12_pwle *pwle)
{
	int val, ret;

	/* Valid values, as per spec, 0mS - 1023.75mS */
	ret = cs40l2x_parse_float(token, &val, 2, 0, 102375);
	if (ret) {
		dev_err(cs40l2x->dev, "Failed to parse wait time: %d\n", ret);
		return ret;
	}

	pwle->wait = val / (100 / 4);
	pwle->wlength += pwle->wait;

	return ret;
}

static int cs40l2x_pwle_time_entry(struct cs40l2x_private *cs40l2x, char *token,
				   struct wt_type12_pwle *pwle,
				   struct wt_type12_pwle_section *section,
				   bool *indef)
{
	int val, ret;

	/* Valid values, as per spec, 0mS - 16383.5mS, 16383.75mS = infinite */
	ret = cs40l2x_parse_float(token, &val, 2, 0, 1638375);
	if (ret) {
		dev_err(cs40l2x->dev, "Failed to parse time: %d\n", ret);
		return ret;
	}

	section->time = val / (100 / 4);

	if (val == CS40L2X_PWLE_INDEF_TIME_VAL)
		*indef = true;
	else
		pwle->wlength += section->time;

	return ret;
}

static int cs40l2x_pwle_level_entry(struct cs40l2x_private *cs40l2x, char *token,
				    struct wt_type12_pwle_section *section)
{
	int val, ret;

	/* Valid values, as per spec, -1 - 0.9995118 */
	ret = cs40l2x_parse_float(token, &val, 7, -10000000, 9995118);
	if (ret) {
		dev_err(cs40l2x->dev, "Failed to parse level: %d\n", ret);
		return ret;
	}

	section->level = val / (10000000 / 2048);

	return ret;
}

static int cs40l2x_pwle_frequency_entry(struct cs40l2x_private *cs40l2x,
					char *token,
					struct wt_type12_pwle_section *section)
{
	int val, ret;
	/* Valid values, as per spec, 50.125Hz - 561.875Hz */
	int min = 50125, max = 561875;

	if (cs40l2x->ext_freq_min_fw) {
		/* Valid values, as per spec, 0.25Hz - 1023.75Hz */
		min = 250;
		max = 1023750;
	}

	ret = cs40l2x_parse_float(token, &val, 3, min, max);
	if (ret) {
		dev_err(cs40l2x->dev, "Failed to parse frequency: %d\n", ret);
		return ret;
	}

	if (cs40l2x->ext_freq_min_fw)
		section->frequency = (val / (1000 / 4));
	else
		section->frequency = (val / (1000 / 8)) - 400;

	return ret;
}

static int cs40l2x_pwle_vb_target_entry(struct cs40l2x_private *cs40l2x,
					char *token,
					struct wt_type12_pwle_section *section)
{
	int val, ret;

	/*
	 * We don't pass a scale value as we will scale locally, valid values,
	 * as per spec, are 0 - 1.
	 */
	ret = cs40l2x_parse_float(token, &val, 6, 0, 1000000);
	if (ret) {
		dev_err(cs40l2x->dev, "Failed to parse frequency: %d\n", ret);
		return ret;
	}

	/* Approximation to scaling to 999999/0x7fffff without overflowing */
	val = (val * 1770) / 211;
	clamp(val, 0, 0x7FFFFF);

	section->vbtarget = val;

	return ret;
}

static ssize_t cs40l2x_pwle_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	struct i2c_client *i2c_client = to_i2c_client(cs40l2x->dev);
	struct wt_type12_pwle *pwle;
	struct wt_type12_pwle_section *section;
	char *pwle_str, *cur, *token, *type;
	unsigned int num_vals = 0, num_segs = 0, feature = 0;
	unsigned int val;
	bool indef = false, save_pwle = false;
	bool t = false, l = false, f = false, c = false, b = false;
	bool a = false, v = false;
	int ret;


	pwle_str = kzalloc(count, GFP_KERNEL);
	if (!pwle_str)
		return -ENOMEM;

	pwle = kzalloc(sizeof(*pwle), GFP_KERNEL);
	if (!pwle) {
		kfree(pwle_str);
		return -ENOMEM;
	}

	disable_irq(i2c_client->irq);

	mutex_lock(&cs40l2x->lock);

	cs40l2x->queue_stored = false;

	if (!cs40l2x->virtual_stored) {
		dev_err(dev, "Unsafe condition encountered.\n");
		ret = -EINVAL;
		goto err_exit;
	}

	section = pwle->sections;

	strlcpy(pwle_str, buf, count);

	cur = pwle_str;

	while ((token = strsep(&cur, ","))) {
		token = strim(token);

		if (num_vals >= CS40L2X_PWLE_TOTAL_VALS) {
			ret = -E2BIG;
			goto err_exit;
		}

		type = strsep(&token, ":");
		token = strim(token);

		if (type[0] == 'S') {
			if (num_vals != 0) {
				dev_err(cs40l2x->dev,
					"Malformed PWLE, missing Save entry\n");
				ret = -EINVAL;
				goto err_exit;
			}

			ret = kstrtou32(token, 10, &val);
			if (ret)
				goto err_exit;

			if (val > 1) {
				dev_err(cs40l2x->dev, "Valid Save: 0 or 1\n");
				ret = -EINVAL;
				goto err_exit;
			}

			if (val)
				save_pwle = true;
		} else if (!strncmp(type, "WF", 2)) {
			if (num_vals != 1) {
				dev_err(cs40l2x->dev,
					"Malformed PWLE, missing Feature entry\n");
				ret = -EINVAL;
				goto err_exit;
			}

			ret = kstrtou32(token, 10, &val);
			if (ret)
				goto err_exit;

			if (val > CS40L2X_PWLE_MAX_WVFRM_FEAT || (val % 4) != 0) {
				dev_err(cs40l2x->dev, "Valid Waveform Feature: 0, 4, 8, 12\n");
				ret = -EINVAL;
				goto err_exit;
			}

			feature = val << CS40L2X_PWLE_WVFRM_FT_SHFT;
		} else if (!strncmp(type, "RP", 2)) {
			if (num_vals != 2) {
				dev_err(cs40l2x->dev,
					"Malformed PWLE, missing Repeat entry\n");
				ret = -EINVAL;
				goto err_exit;
			}

			ret = cs40l2x_pwle_repeat_entry(cs40l2x, token, pwle);
			if (ret)
				goto err_exit;
		} else if (!strncmp(type, "WT", 2)) {
			if (num_vals != 3) {
				dev_err(cs40l2x->dev,
					"Malformed PWLE, WaitTime follows Repeat\n");
				ret = -EINVAL;
				goto err_exit;
			}

			ret = cs40l2x_pwle_wait_time_entry(cs40l2x, token, pwle);
			if (ret)
				goto err_exit;
		} else if (type[0] == 'T') {
			if (num_vals > 4) {
				/* Verify complete previous segment */
				if (!t || !l || !f || !c || !b || !a || !v) {
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

			ret = cs40l2x_pwle_time_entry(cs40l2x, token, pwle,
						      section, &indef);
			if (ret)
				goto err_exit;

			t = true;
		} else if (type[0] == 'L') {
			ret = cs40l2x_pwle_level_entry(cs40l2x, token, section);
			if (ret)
				goto err_exit;

			l = true;
		} else if (type[0] == 'F') {
			ret = cs40l2x_pwle_frequency_entry(cs40l2x, token,
							   section);
			if (ret)
				goto err_exit;

			f = true;
		} else if (type[0] == 'C') {
			ret = kstrtou32(token, 10, &val);
			if (ret)
				goto err_exit;

			if (val > 1) {
				dev_err(cs40l2x->dev,
					"Valid Chirp: 0 or 1\n");
				ret = -EINVAL;
				goto err_exit;
			}

			if (val)
				section->flags |= WT_T12_FLAG_CHIRP;

			c = true;
		} else if (type[0] == 'B') {
			ret = kstrtou32(token, 10, &val);
			if (ret)
				goto err_exit;

			if (val > 1) {
				dev_err(cs40l2x->dev,
					"Valid Braking: 0 or 1\n");
				ret = -EINVAL;
				goto err_exit;
			}

			if (val)
				section->flags |= WT_T12_FLAG_BRAKE;

			b = true;
		} else if (!strncmp(type, "AR", 2)) {
			ret = kstrtou32(token, 10, &val);
			if (ret)
				goto err_exit;

			if (val > 1) {
				dev_err(cs40l2x->dev,
					"Valid Amplitude Regulation: 0 or 1\n");
				ret = -EINVAL;
				goto err_exit;
			}

			if (val)
				section->flags |= WT_T12_FLAG_AMP_REG;

			a = true;

		} else if (type[0] == 'V') {
			if (section->flags & WT_T12_FLAG_AMP_REG) {
				ret = cs40l2x_pwle_vb_target_entry(cs40l2x,
								   token,
								   section);
				if (ret)
					goto err_exit;
			}

			v = true;
			num_segs++;
			section++;
		}

		num_vals++;
	}

	/* Verify last segment was complete */
	if (!t || !l || !f || !c || !b || !a || !v) {
		dev_err(cs40l2x->dev,
			"Malformed PWLE. Missing entry in seg %d\n",
			(num_segs - 1));
		ret = -EINVAL;
		goto err_exit;
	}

	pwle->nsections = num_segs;

	strlcpy(cs40l2x->pwle_str, buf, count);
	cs40l2x->pwle_str_size = count;

	pwle->wlength *= pwle->repeat + 1;
	/* Firmware doesn't count the last wait since it is just dead time */
	pwle->wlength -= pwle->wait;

	/* Convert from 1/4mS's to samples at 8kHz for waveform length */
	pwle->wlength *= 2;

	if (indef)
		pwle->wlength |= WT_WAVELEN_INDEFINITE;

	pwle->wlength |= WT_WAVELEN_CALCULATED;

	ret = cs40l2x_save_packed_pwle_data(cs40l2x, pwle, feature, save_pwle);
	if (ret) {
		dev_err(cs40l2x->dev,
			"Malformed PWLE. No segments found.\n");
		ret = -EINVAL;
		goto err_exit;
	}

	ret = count;

	if (!save_pwle)
		cs40l2x->last_type_entered = CS40L2X_WT_TYPE_12_PWLE_FILE;
	cs40l2x->queue_stored = true;

err_exit:
	mutex_unlock(&cs40l2x->lock);

	kfree(pwle);
	kfree(pwle_str);

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

	mutex_lock(&cs40l2x->lock);
	index = cs40l2x->display_pwle_segs;
	mutex_unlock(&cs40l2x->lock);

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

static int cs40l2x_wait_for_pwrmgt_sts(struct cs40l2x_private *cs40l2x)
{
	unsigned int sts;
	int i, ret;

	for (i = 0; i < CS40L2X_STATUS_RETRIES; i++) {
		ret = regmap_read(cs40l2x->regmap, CS40L2X_PWRMGT_STS, &sts);
		if (ret)
			dev_err(cs40l2x->dev,
				"Failed to read PWRMGT_STS: %d\n", ret);
		else if (!(sts & CS40L2X_WR_PEND_STS_MASK))
			return 0;
	}

	dev_err(cs40l2x->dev, "Timed out reading PWRMGT_STS\n");
	return -ETIMEDOUT;
}

static int cs40l2x_apply_hibernate_errata(struct cs40l2x_private *cs40l2x)
{
	int ret;

	dev_warn(cs40l2x->dev, "Retry hibernate\n");

	cs40l2x_wait_for_pwrmgt_sts(cs40l2x);

	ret = regmap_write(cs40l2x->regmap, CS40L2X_WAKESRC_CTL,
			   (CS40L2X_WKSRC_EN_SDA << CS40L2X_WKSRC_EN_SHIFT) |
			   (CS40L2X_WKSRC_POL_SDA << CS40L2X_WKSRC_POL_SHIFT));
	if (ret)
		dev_err(cs40l2x->dev, "Failed to set WAKESRC: %d\n", ret);

	cs40l2x_wait_for_pwrmgt_sts(cs40l2x);

	ret = regmap_write(cs40l2x->regmap, CS40L2X_WAKESRC_CTL,
			   CS40L2X_UPDT_WKCTL_MASK |
			   (CS40L2X_WKSRC_EN_SDA << CS40L2X_WKSRC_EN_SHIFT) |
			   (CS40L2X_WKSRC_POL_SDA << CS40L2X_WKSRC_POL_SHIFT));
	if (ret)
		dev_err(cs40l2x->dev, "Failed to enable WAKESRC: %d\n", ret);

	cs40l2x_wait_for_pwrmgt_sts(cs40l2x);

	/*
	 * This write may force the device into hibernation before the ACK is
	 * returned, so ignore the return value.
	 */
	regmap_write(cs40l2x->regmap, CS40L2X_PWRMGT_CTL,
		     (1 << CS40L2X_MEM_RDY_SHIFT) |
		     (1 << CS40L2X_TRIG_HIBER_SHIFT));

	return 0;
}

static int cs40l2x_wake_from_hibernate(struct cs40l2x_private *cs40l2x)
{
	unsigned int pwr_reg = cs40l2x_dsp_reg(cs40l2x, "POWERSTATE",
					       CS40L2X_XM_UNPACKED_TYPE,
					       cs40l2x->fw_desc->id);
	unsigned int val;
	int ret, i;

	dev_dbg(cs40l2x->dev, "Attempt wake from hibernate\n");

	ret = cs40l2x_ack_write(cs40l2x, CS40L2X_MBOX_POWERCONTROL,
				CS40L2X_POWERCONTROL_WAKEUP,
				CS40L2X_POWERCONTROL_NONE);
	if (ret) {
		if (ret == -ETIME)
			cs40l2x_apply_hibernate_errata(cs40l2x);

		return ret;
	}

	for (i = 0; i < CS40L2X_STATUS_RETRIES; i++) {
		ret = regmap_read(cs40l2x->regmap, pwr_reg, &val);
		if (ret) {
			dev_err(cs40l2x->dev, "Failed to read POWERSTATE: %d\n",
				ret);
			return ret;
		}

		dev_dbg(cs40l2x->dev, "Read POWERSTATE: %d\n", val);

		switch (val) {
		case CS40L2X_POWERSTATE_ACTIVE:
		case CS40L2X_POWERSTATE_STANDBY:
			dev_dbg(cs40l2x->dev, "Woke from hibernate\n");
			return 0;
		case CS40L2X_POWERSTATE_HIBERNATE:
			break;
		default:
			dev_err(cs40l2x->dev, "Invalid POWERSTATE: %x\n", val);
			break;
		}

		usleep_range(5000, 5100);
	}

	dev_err(cs40l2x->dev, "Timed out waiting for POWERSTATE: %d\n", val);

	cs40l2x_apply_hibernate_errata(cs40l2x);

	return -ETIMEDOUT;
}

static int cs40l2x_hiber_cmd_send(struct cs40l2x_private *cs40l2x,
				  unsigned int hiber_cmd)
{
	int i;

	switch (hiber_cmd) {
	case CS40L2X_POWERCONTROL_NONE:
	case CS40L2X_POWERCONTROL_FRC_STDBY:
		return cs40l2x_ack_write(cs40l2x, CS40L2X_MBOX_POWERCONTROL,
					 hiber_cmd, CS40L2X_POWERCONTROL_NONE);

	case CS40L2X_POWERCONTROL_HIBERNATE:
		/*
		 * The control port is unavailable immediately after this write,
		 * so don't poll for acknowledgment.
		 */
		return regmap_write(cs40l2x->regmap, CS40L2X_MBOX_POWERCONTROL,
				    hiber_cmd);

	case CS40L2X_POWERCONTROL_WAKEUP:
		/*
		 * The first several transactions are expected to be NAK'd, so
		 * retry multiple times in rapid succession.
		 */
		for (i = 0; i < CS40L2X_WAKEUP_RETRIES; i++) {
			if (!cs40l2x_wake_from_hibernate(cs40l2x))
				return 0;

			usleep_range(1000, 1100);
		}

		return -ETIMEDOUT;

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
		if (*index == cs40l2x->virtual_gpio1_fall_slot) {
			*index =
			cs40l2x->virtual_gpio_index[CS40L2X_GPIO_FALL];
		} else if (*index == cs40l2x->virtual_gpio1_rise_slot) {
			*index =
			cs40l2x->virtual_gpio_index[CS40L2X_GPIO_RISE];
		}
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
					cs40l2x->virtual_gpio1_rise_slot;
					if (cs40l2x->virtual_gpio_index[r] !=
						cs40l2x->loaded_gpio_index[r])
						cs40l2x_write_virtual_waveform(
						cs40l2x,
						cs40l2x->virtual_gpio_index[r],
						true, true, false);
					/* else virtual wvfrm already loaded */
				} else {
					index =
					cs40l2x->virtual_gpio1_fall_slot;
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

static ssize_t cs40l2x_bemf_rec_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	struct regmap *regmap = cs40l2x->regmap;
	unsigned int reg, val;
	ssize_t len = 0;
	int ret, i;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "BEMF_BUFFER",
				CS40L2X_XM_UNPACKED_TYPE,
				CS40L2X_ALGO_ID_PAR);
	if (!reg) {
		dev_err(dev, "Cannot get the register for the bemf buffer\n");
		ret = -EINVAL;
		goto err_bemf_rec;
	}

	for (i = 0; i < CS40L2X_BEMF_BUF_MAX; i++) {
		ret = regmap_read(regmap, reg + (i*4), &val);
		if (ret)
			goto err_bemf_rec;
		len += snprintf(buf + len, PAGE_SIZE - len, "%d ", val);
	}
	len += snprintf(buf + len, PAGE_SIZE - len, "\n");

	ret = len;

err_bemf_rec:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);
	return ret;

}

static ssize_t cs40l2x_bemf_rec_en_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	struct regmap *regmap = cs40l2x->regmap;
	int ret;
	unsigned int reg, val;

	ret = kstrtou32(buf, 10, &val);
	if (ret || val > 1)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "MEASURE_BEMF_ONLY",
				CS40L2X_XM_UNPACKED_TYPE,
				CS40L2X_ALGO_ID_PAR);
	if (!reg) {
		dev_err(dev, "Cannot get the register for bemf only\n");
		ret = -EINVAL;
		goto err_bemf_rec_en;
	}

	ret = regmap_write(regmap, reg, val);
	if (ret)
		goto err_bemf_rec_en;

	ret = count;

err_bemf_rec_en:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);
	return ret;
}

static ssize_t cs40l2x_bemf_rec_en_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	struct regmap *regmap = cs40l2x->regmap;
	unsigned int reg, val;
	int ret;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "MEASURE_BEMF_ONLY",
				CS40L2X_XM_UNPACKED_TYPE,
				CS40L2X_ALGO_ID_PAR);
	if (!reg) {
		dev_err(dev, "Cannot get the register for bemf only\n");
		ret = -EINVAL;
		goto err_bemf_rec_en;
	}

	ret = regmap_read(regmap, reg, &val);
	if (ret)
		goto err_bemf_rec_en;

	ret = snprintf(buf, PAGE_SIZE, "%d\n", val);

err_bemf_rec_en:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);
	return ret;
}

static ssize_t cs40l2x_bemf_shift_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	struct regmap *regmap = cs40l2x->regmap;
	unsigned int reg, val;
	int ret;

	ret = kstrtou32(buf, 10, &val);
	if (ret)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "BEMF_SHIFT",
			      CS40L2X_XM_UNPACKED_TYPE, CS40L2X_ALGO_ID_PAR);
	if (!reg) {
		dev_err(cs40l2x->dev, "Unable to get bemf shift register\n");
		ret = -EPERM;
		goto err_exit;
	}

	ret = regmap_write(regmap, reg, val);
	if (ret)
		goto err_exit;

	ret = count;

err_exit:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_bemf_shift_show(struct device *dev,
				       struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	struct regmap *regmap = cs40l2x->regmap;
	unsigned int reg, val;
	int ret;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "BEMF_SHIFT",
			      CS40L2X_XM_UNPACKED_TYPE, CS40L2X_ALGO_ID_PAR);
	if (!reg) {
		dev_err(cs40l2x->dev, "Unable to get bemf shift register\n");
		ret = -EINVAL;
		goto err_exit;
	}

	ret = regmap_read(regmap, reg, &val);
	if (ret)
		goto err_exit;

	ret = snprintf(buf, PAGE_SIZE, "%d\n", val);

err_exit:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

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
	int ret, i, loc = -1, index;

	ret = kstrtou32(buf, 10, &val);
	if (ret)
		return -EINVAL;

	if (val > CS40L2X_DYN_F0_MASK) {
		dev_err(dev, "Invalid f0 value %d\n", val);
		return -EINVAL;
	}

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "DYN_F0_TABLE", CS40L2X_XM_UNPACKED_TYPE,
				CS40L2X_ALGO_ID_DYN_F0);
	if (!reg) {
		dev_err(dev, "Cannot get the register for the f0 table\n");
		ret = -EINVAL;
		goto err_mutex;
	}

	index = cs40l2x->dynamic_f0_index;

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
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_dyn_f0_val_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret = 0, i, index;

	mutex_lock(&cs40l2x->lock);

	index = cs40l2x->dynamic_f0_index;

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

	ret = regmap_read(cs40l2x->regmap, CS40L2X_AMP_CTRL, &val);
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

	ret = regmap_read(cs40l2x->regmap, CS40L2X_AMP_CTRL, &val);
	if (ret)
		return ret;

	val &= ~CS40L2X_AMP_VOL_PCM_MASK;
	val |= CS40L2X_AMP_VOL_PCM_MASK &
			(((CS40L2X_DIG_SCALE_ZERO - dig_scale)
			& CS40L2X_DIG_SCALE_MASK) << CS40L2X_AMP_VOL_PCM_SHIFT);

	ret = regmap_write(cs40l2x->regmap, CS40L2X_AMP_CTRL, val);
	if (ret)
		return ret;

	return cs40l2x_wseq_replace(cs40l2x, CS40L2X_AMP_CTRL, val);
}

static ssize_t cs40l2x_dig_scale_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int dig_scale;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	/*
	 * this operation is agnostic to the variable firmware ID and may
	 * therefore be performed without mutex protection
	 */
	ret = cs40l2x_dig_scale_get(cs40l2x, &dig_scale);

	mutex_unlock(&cs40l2x->lock);
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
	unsigned int val, reg;
	int ret;

	reg = cs40l2x_dsp_reg(cs40l2x, "GAIN_CONTROL",
			      CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
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
	unsigned int val, reg;
	int ret;

	reg = cs40l2x_dsp_reg(cs40l2x, "GAIN_CONTROL",
			      CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
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
	unsigned int val, reg;
	int ret;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "GPIO_GAIN",
			      CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	reg += gpio_offs;

	if (!(cs40l2x->gpio_mask & (1 << (gpio_offs >> 2)))) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_read(cs40l2x->regmap, reg, &val);
	if (ret)
		goto err_mutex;

	*dig_scale = (val & (gpio_pol ^ gpio_rise ?
			CS40L2X_GPIO_GAIN_RISE_MASK :
			CS40L2X_GPIO_GAIN_FALL_MASK)) >>
				(gpio_pol ^ gpio_rise ?
					CS40L2X_GPIO_GAIN_RISE_SHIFT :
					CS40L2X_GPIO_GAIN_FALL_SHIFT);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static int cs40l2x_gpio_edge_dig_scale_set(struct cs40l2x_private *cs40l2x,
			unsigned int dig_scale,
			unsigned int gpio_offs, bool gpio_rise)
{
	bool gpio_pol = cs40l2x->pdata.gpio_indv_pol & (1 << (gpio_offs >> 2));
	unsigned int val, reg;
	int ret;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "GPIO_GAIN",
			      CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	reg += gpio_offs;

	if (!(cs40l2x->gpio_mask & (1 << (gpio_offs >> 2)))) {
		ret = -EPERM;
		goto err_mutex;
	}

	if (dig_scale == CS40L2X_DIG_SCALE_RESET
			|| dig_scale > CS40L2X_DIG_SCALE_MAX) {
		ret = -EINVAL;
		goto err_mutex;
	}

	ret = regmap_read(cs40l2x->regmap, reg, &val);
	if (ret)
		goto err_mutex;

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
		goto err_mutex;

	ret = cs40l2x_dsp_cache(cs40l2x, reg, val);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_gpio1_rise_dig_scale_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int dig_scale;

	ret = cs40l2x_gpio_edge_dig_scale_get(cs40l2x, &dig_scale,
			CS40L2X_INDEXBUTTONPRESS1, CS40L2X_GPIO_RISE);
	if (ret)
		return ret;

	return snprintf(buf, PAGE_SIZE, "%u\n", dig_scale);
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

	ret = cs40l2x_gpio_edge_dig_scale_set(cs40l2x, dig_scale,
			CS40L2X_INDEXBUTTONPRESS1, CS40L2X_GPIO_RISE);
	if (ret)
		return ret;

	return count;
}

static ssize_t cs40l2x_gpio1_fall_dig_scale_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int dig_scale;

	ret = cs40l2x_gpio_edge_dig_scale_get(cs40l2x, &dig_scale,
			CS40L2X_INDEXBUTTONRELEASE1, CS40L2X_GPIO_FALL);
	if (ret)
		return ret;

	return snprintf(buf, PAGE_SIZE, "%u\n", dig_scale);
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

	ret = cs40l2x_gpio_edge_dig_scale_set(cs40l2x, dig_scale,
			CS40L2X_INDEXBUTTONRELEASE1, CS40L2X_GPIO_FALL);
	if (ret)
		return ret;

	return count;
}

static ssize_t cs40l2x_gpio2_rise_dig_scale_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int dig_scale;

	ret = cs40l2x_gpio_edge_dig_scale_get(cs40l2x, &dig_scale,
			CS40L2X_INDEXBUTTONPRESS2, CS40L2X_GPIO_RISE);
	if (ret)
		return ret;

	return snprintf(buf, PAGE_SIZE, "%u\n", dig_scale);
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

	ret = cs40l2x_gpio_edge_dig_scale_set(cs40l2x, dig_scale,
			CS40L2X_INDEXBUTTONPRESS2, CS40L2X_GPIO_RISE);
	if (ret)
		return ret;

	return count;
}

static ssize_t cs40l2x_gpio2_fall_dig_scale_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int dig_scale;

	ret = cs40l2x_gpio_edge_dig_scale_get(cs40l2x, &dig_scale,
			CS40L2X_INDEXBUTTONRELEASE2, CS40L2X_GPIO_FALL);
	if (ret)
		return ret;

	return snprintf(buf, PAGE_SIZE, "%u\n", dig_scale);
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

	ret = cs40l2x_gpio_edge_dig_scale_set(cs40l2x, dig_scale,
			CS40L2X_INDEXBUTTONRELEASE2, CS40L2X_GPIO_FALL);
	if (ret)
		return ret;

	return count;
}

static ssize_t cs40l2x_gpio3_rise_dig_scale_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int dig_scale;

	ret = cs40l2x_gpio_edge_dig_scale_get(cs40l2x, &dig_scale,
			CS40L2X_INDEXBUTTONPRESS3, CS40L2X_GPIO_RISE);
	if (ret)
		return ret;

	return snprintf(buf, PAGE_SIZE, "%u\n", dig_scale);
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

	ret = cs40l2x_gpio_edge_dig_scale_set(cs40l2x, dig_scale,
			CS40L2X_INDEXBUTTONPRESS3, CS40L2X_GPIO_RISE);
	if (ret)
		return ret;

	return count;
}

static ssize_t cs40l2x_gpio3_fall_dig_scale_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int dig_scale;

	ret = cs40l2x_gpio_edge_dig_scale_get(cs40l2x, &dig_scale,
			CS40L2X_INDEXBUTTONRELEASE3, CS40L2X_GPIO_FALL);
	if (ret)
		return ret;

	return snprintf(buf, PAGE_SIZE, "%u\n", dig_scale);
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

	ret = cs40l2x_gpio_edge_dig_scale_set(cs40l2x, dig_scale,
			CS40L2X_INDEXBUTTONRELEASE3, CS40L2X_GPIO_FALL);
	if (ret)
		return ret;

	return count;
}

static ssize_t cs40l2x_gpio4_rise_dig_scale_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int dig_scale;

	ret = cs40l2x_gpio_edge_dig_scale_get(cs40l2x, &dig_scale,
			CS40L2X_INDEXBUTTONPRESS4, CS40L2X_GPIO_RISE);
	if (ret)
		return ret;

	return snprintf(buf, PAGE_SIZE, "%u\n", dig_scale);
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

	ret = cs40l2x_gpio_edge_dig_scale_set(cs40l2x, dig_scale,
			CS40L2X_INDEXBUTTONPRESS4, CS40L2X_GPIO_RISE);
	if (ret)
		return ret;

	return count;
}

static ssize_t cs40l2x_gpio4_fall_dig_scale_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int dig_scale;

	ret = cs40l2x_gpio_edge_dig_scale_get(cs40l2x, &dig_scale,
			CS40L2X_INDEXBUTTONRELEASE4, CS40L2X_GPIO_FALL);
	if (ret)
		return ret;

	return snprintf(buf, PAGE_SIZE, "%u\n", dig_scale);
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

	ret = cs40l2x_gpio_edge_dig_scale_set(cs40l2x, dig_scale,
			CS40L2X_INDEXBUTTONRELEASE4, CS40L2X_GPIO_FALL);
	if (ret)
		return ret;

	return count;
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

static ssize_t cs40l2x_fw_id_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	unsigned int fw_id;

	mutex_lock(&cs40l2x->lock);
	fw_id = cs40l2x->fw_desc->id;
	mutex_unlock(&cs40l2x->lock);

	return snprintf(buf, PAGE_SIZE, "0x%06X\n", fw_id);
}

static ssize_t cs40l2x_fw_swap_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	unsigned int fw_id;
	int ret;

	ret = kstrtou32(buf, 16, &fw_id);
	if (ret)
		return -EINVAL;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	if (fw_id == cs40l2x->fw_desc->id)
		goto err_exit;

	if (fw_id == CS40L2X_FW_ID_ORIG || fw_id == CS40L2X_FW_ID_B1ROM) {
		ret = -EINVAL;
		goto err_exit;
	}

	if (cs40l2x->revid < CS40L2X_REVID_B1) {
		ret = -EPERM;
		goto err_exit;
	}

	ret = cs40l2x_firmware_swap(cs40l2x, fw_id);
	if (ret) {
		dev_err(cs40l2x->dev, "Failed to swap firmware: %d\n", ret);
		goto err_exit;
	}

	cs40l2x->fw_id_remap = fw_id;

	cs40l2x->dsp_cache_depth = 0;

	if (cs40l2x->pbq_state != CS40L2X_PBQ_STATE_IDLE) {
		ret = cs40l2x_cp_dig_scale_set(cs40l2x, cs40l2x->pbq_cp_dig_scale);
		if (ret)
			goto err_exit;

		cs40l2x->pbq_state = CS40L2X_PBQ_STATE_IDLE;
	}

	if (cs40l2x->cp_trigger_index == cs40l2x->virtual_slot_index)
		cs40l2x_write_virtual_waveform(cs40l2x,
					       cs40l2x->loaded_virtual_index,
					       false, false, false);

	dev_info(cs40l2x->dev, "Successfully swapped firmware to 0x%06X\n",
		 fw_id);

err_exit:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	if (ret)
		return ret;

	return count;
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
	if (!reg) {
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

	if (cs40l2x->vibe_state == CS40L2X_VIBE_STATE_RUNNING) {
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

static ssize_t cs40l2x_imon_offs_enable_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "VMON_IMON_OFFSET_ENABLE",
			CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
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

static ssize_t cs40l2x_imon_offs_enable_store(struct device *dev,
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

	reg = cs40l2x_dsp_reg(cs40l2x, "VMON_IMON_OFFSET_ENABLE",
			CS40L2X_XM_UNPACKED_TYPE, cs40l2x->fw_desc->id);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_write(cs40l2x->regmap, reg,
			val ? CS40L2X_IMON_OFFS_CALC_EN :
			CS40L2X_IMON_OFFS_CALC_DIS);
	if (ret)
		goto err_mutex;

	ret = cs40l2x_dsp_cache(cs40l2x, reg,
			val ? CS40L2X_IMON_OFFS_CALC_EN :
			CS40L2X_IMON_OFFS_CALC_DIS);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
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

	pm_runtime_get_sync(cs40l2x->dev);
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
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

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

	pm_runtime_get_sync(cs40l2x->dev);
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
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_par_gain_comp_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	pm_runtime_get_sync(cs40l2x->dev);
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
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

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

	pm_runtime_get_sync(cs40l2x->dev);
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
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_vibe_state_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;

	mutex_lock(&cs40l2x->lock);
	ret = snprintf(buf, PAGE_SIZE, "%u\n", cs40l2x->vibe_state);
	mutex_unlock(&cs40l2x->lock);

	return ret;
}

static ssize_t cs40l2x_gpio_event_show(struct device *dev,
				       struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;

	mutex_lock(&cs40l2x->lock);
	ret = snprintf(buf, PAGE_SIZE, "%u\n", cs40l2x->gpio_event);
	mutex_unlock(&cs40l2x->lock);

	return ret;
}

static ssize_t cs40l2x_gpio_event_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int val;

	ret = kstrtou32(buf, 10, &val);
	if (ret || val != 0)
		return -EINVAL;

	mutex_lock(&cs40l2x->lock);
	cs40l2x_set_gpio_event(cs40l2x, false);
	mutex_unlock(&cs40l2x->lock);

	return count;
}

static ssize_t cs40l2x_safe_save_state_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;

	mutex_lock(&cs40l2x->lock);
	ret = snprintf(buf, PAGE_SIZE, "%u\n", cs40l2x->safe_save_state);
	mutex_unlock(&cs40l2x->lock);

	return ret;
}

static ssize_t cs40l2x_max_back_emf_show(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	pm_runtime_get_sync(cs40l2x->dev);
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
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

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

	pm_runtime_get_sync(cs40l2x->dev);
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
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_die_temp_show(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	unsigned int val;
	s32 temp;
	int ret;

	pm_runtime_get_sync(cs40l2x->dev);

	/* Zero value could mean that the device is in standby */
	ret = regmap_read(cs40l2x->regmap, CS40L2X_ENABLES_AND_CODES_DIG, &val);
	if (!ret) {
		temp = sign_extend32(val & CS40L2X_TEMP_RESULT_UNFILT_MASK,
				     fls(CS40L2X_TEMP_RESULT_UNFILT_MASK) - 1);

		if (temp < CS40L2X_TEMP_RESULT_MIN ||
		    temp > CS40L2X_TEMP_RESULT_MAX) {
			dev_err(cs40l2x->dev, "Read invalid die temperature\n");
			ret = -EINVAL;
		} else {
			ret = snprintf(buf, PAGE_SIZE, "%d\n", temp);
		}
	}

	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_vbst_avg_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	struct regmap *regmap = cs40l2x->regmap;
	int ret;
	unsigned int reg, val;

	pm_runtime_get_sync(cs40l2x->dev);

	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "VBST_AVG",
			CS40L2X_XM_UNPACKED_TYPE,
			cs40l2x->fw_desc->id);
	if (!reg) {
		ret = -EINVAL;
		goto err_vbst_avg;
	}

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

static ssize_t cs40l2x_pwle_ramp_down_show(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	struct cs40l2x_private *cs40l2x = cs40l2x_get_private(dev);
	int ret;
	unsigned int reg, val;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "RAMPDOWN_COEFF",
			      CS40L2X_XM_UNPACKED_TYPE,
				cs40l2x->fw_desc->id);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	ret = regmap_read(cs40l2x->regmap, reg, &val);
	if (ret)
		goto err_mutex;

	/* Q0.24 format */
	ret = snprintf(buf, PAGE_SIZE, "%u\n", val);

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
}

static ssize_t cs40l2x_pwle_ramp_down_store(struct device *dev,
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

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	reg = cs40l2x_dsp_reg(cs40l2x, "RAMPDOWN_COEFF",
			      CS40L2X_XM_UNPACKED_TYPE,
				cs40l2x->fw_desc->id);
	if (!reg) {
		ret = -EPERM;
		goto err_mutex;
	}

	/* Q0.24 format */
	/* Zero value means that PWLE Ramp down is off */
	ret = regmap_write(cs40l2x->regmap, reg, val);
	if (ret)
		goto err_mutex;

	ret = count;

err_mutex:
	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);

	return ret;
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
static DEVICE_ATTR(bemf_measured, 0660, cs40l2x_bemf_measured_show, NULL);
static DEVICE_ATTR(bemf_rec, 0660, cs40l2x_bemf_rec_show, NULL);
static DEVICE_ATTR(bemf_rec_en, 0660, cs40l2x_bemf_rec_en_show,
		cs40l2x_bemf_rec_en_store);
static DEVICE_ATTR(bemf_shift, 0660, cs40l2x_bemf_shift_show,
		cs40l2x_bemf_shift_store);
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
static DEVICE_ATTR(fw_id, 0660, cs40l2x_fw_id_show, NULL);
static DEVICE_ATTR(fw_swap, 0660, NULL, cs40l2x_fw_swap_store);
static DEVICE_ATTR(vpp_measured, 0660, cs40l2x_vpp_measured_show, NULL);
static DEVICE_ATTR(ipp_measured, 0660, cs40l2x_ipp_measured_show, NULL);
static DEVICE_ATTR(vbatt_max, 0660, cs40l2x_vbatt_max_show,
		cs40l2x_vbatt_max_store);
static DEVICE_ATTR(vbatt_min, 0660, cs40l2x_vbatt_min_show,
		cs40l2x_vbatt_min_store);
static DEVICE_ATTR(exc_enable, 0660, cs40l2x_exc_enable_show,
		cs40l2x_exc_enable_store);
static DEVICE_ATTR(hw_err_count, 0660, cs40l2x_hw_err_count_show,
		cs40l2x_hw_err_count_store);
static DEVICE_ATTR(hw_reset, 0660, cs40l2x_hw_reset_show,
		cs40l2x_hw_reset_store);
static DEVICE_ATTR(wt_file, 0660, cs40l2x_wt_file_show, cs40l2x_wt_file_store);
static DEVICE_ATTR(wt_date, 0660, cs40l2x_wt_date_show, NULL);
static DEVICE_ATTR(vmon_imon_offs_enable, 0660, cs40l2x_imon_offs_enable_show,
		cs40l2x_imon_offs_enable_store);
static DEVICE_ATTR(clab_enable, 0660, cs40l2x_clab_enable_show,
		cs40l2x_clab_enable_store);
static DEVICE_ATTR(clab_peak, 0660, cs40l2x_clab_peak_show,
		cs40l2x_clab_peak_store);
static DEVICE_ATTR(pwle_regulation_enable, 0660, cs40l2x_par_enable_show,
		cs40l2x_par_enable_store);
static DEVICE_ATTR(gain_compensation_enable, 0660, cs40l2x_par_gain_comp_show,
		cs40l2x_par_gain_comp_store);
static DEVICE_ATTR(vibe_state, 0660, cs40l2x_vibe_state_show, NULL);
static DEVICE_ATTR(gpio_event, 0660, cs40l2x_gpio_event_show,
		   cs40l2x_gpio_event_store);
static DEVICE_ATTR(safe_save_state, 0660, cs40l2x_safe_save_state_show, NULL);
static DEVICE_ATTR(max_back_emf, 0660, cs40l2x_max_back_emf_show,
		cs40l2x_max_back_emf_store);
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
static DEVICE_ATTR(die_temp, 0660, cs40l2x_die_temp_show, NULL);
static DEVICE_ATTR(vbst_avg, 0660, cs40l2x_vbst_avg_show, NULL);
static DEVICE_ATTR(pwle_ramp_down, 0660, cs40l2x_pwle_ramp_down_show,
	cs40l2x_pwle_ramp_down_store);

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
	&dev_attr_bemf_measured.attr,
	&dev_attr_bemf_rec.attr,
	&dev_attr_bemf_rec_en.attr,
	&dev_attr_bemf_shift.attr,
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
	&dev_attr_fw_id.attr,
	&dev_attr_fw_swap.attr,
	&dev_attr_vpp_measured.attr,
	&dev_attr_ipp_measured.attr,
	&dev_attr_vbatt_max.attr,
	&dev_attr_vbatt_min.attr,
	&dev_attr_exc_enable.attr,
	&dev_attr_hw_err_count.attr,
	&dev_attr_hw_reset.attr,
	&dev_attr_wt_file.attr,
	&dev_attr_wt_date.attr,
	&dev_attr_vmon_imon_offs_enable.attr,
	&dev_attr_clab_enable.attr,
	&dev_attr_clab_peak.attr,
	&dev_attr_pwle_regulation_enable.attr,
	&dev_attr_gain_compensation_enable.attr,
	&dev_attr_vibe_state.attr,
	&dev_attr_gpio_event.attr,
	&dev_attr_safe_save_state.attr,
	&dev_attr_max_back_emf.attr,
	&dev_attr_pwle.attr,
	&dev_attr_num_virtual_composite.attr,
	&dev_attr_num_virtual_pwle.attr,
	&dev_attr_virtual_composite_indexes.attr,
	&dev_attr_virtual_pwle_indexes.attr,
	&dev_attr_available_pwle_segments.attr,
	&dev_attr_die_temp.attr,
	&dev_attr_vbst_avg.attr,
	&dev_attr_pwle_ramp_down.attr,
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
	unsigned int classh = WT_FLAG_CLAB | WT_FLAG_F0;
	int ret, i;

	for (i = 0; i < CS40L2X_MAX_WAVEFORMS; i++)
		if (cs40l2x_get_wave(cs40l2x, i)->flags & classh)
			break;

	if (i == CS40L2X_MAX_WAVEFORMS)
		return 0;

	ret = regmap_update_bits(regmap, CS40L2X_VBST_CTL_2,
					CS40L2X_BST_CTL_SEL_MASK,
					CS40L2X_BST_CTL_SEL_CLASSH);
	if (ret)
		return ret;

	ret = regmap_update_bits(regmap, CS40L2X_BLOCK_ENABLES2,
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
	unsigned int val;
	int ret;

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

	ret = regmap_read(regmap, cs40l2x_dsp_reg(cs40l2x, "STATUS",
			CS40L2X_XM_UNPACKED_TYPE, CS40L2X_ALGO_ID_VIBE), &val);
	if (ret) {
		dev_err(dev, "Failed to capture playback status\n");
		goto err_exit;
	}

	if (val != CS40L2X_STATUS_IDLE)
		goto err_exit;

	if (cs40l2x->a2h_enable) {
		ret = regmap_update_bits(regmap, CS40L2X_VBST_CTL_2,
					CS40L2X_BST_CTL_SEL_MASK,
					CS40L2X_BST_CTL_SEL_CLASSH);
		if (ret)
			goto err_exit;

		ret = regmap_update_bits(regmap, CS40L2X_BLOCK_ENABLES2,
				CS40L2X_CLASSH_EN_MASK,
				1 << CS40L2X_CLASSH_EN_SHIFT);
		if (ret)
			goto err_exit;

		ret = cs40l2x_ack_write(cs40l2x, CS40L2X_MBOX_USER_CONTROL,
					CS40L2X_USER_CTRL_REINIT_A2H,
					CS40L2X_USER_CTRL_SUCCESS);
		if (ret)
			goto err_exit;

		ret = cs40l2x_ack_write(cs40l2x, CS40L2X_MBOX_USER_CONTROL,
					CS40L2X_A2H_I2S_START,
					CS40L2X_USER_CTRL_SUCCESS);
		if (ret)
			goto err_exit;

	} else {
		/* haptic-mode teardown */
		if (cs40l2x->vibe_state == CS40L2X_VIBE_STATE_STOPPED
				|| cs40l2x->pbq_state != CS40L2X_PBQ_STATE_IDLE)
			goto err_exit;

		ret = cs40l2x_ground_amp(cs40l2x, true);
		if (ret) {
			dev_err(dev, "Failed to ground amplifier outputs\n");
			goto err_exit;
		}
	}
	cs40l2x_set_state(cs40l2x, CS40L2X_VIBE_STATE_STOPPED);
	cs40l2x_wl_relax(cs40l2x);

	if (cs40l2x->dyn_f0_enable) {
		ret = cs40l2x_read_dyn_f0_table(cs40l2x);
		if (ret) {
			dev_err(dev, "Failed to read f0 table %d\n", ret);
			goto err_exit;
		}
	}

	if (cs40l2x->cond_class_h_en) {
		ret = cs40l2x_enable_classh(cs40l2x);
		if (ret)
			goto err_exit;
	}

err_exit:
	mutex_unlock(&cs40l2x->lock);
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
		cs40l2x_wl_relax(cs40l2x);
		break;
	default:
		return -EINVAL;
	}

	cs40l2x->pbq_state = CS40L2X_PBQ_STATE_IDLE;
	cs40l2x->cp_trailer_index = CS40L2X_INDEX_IDLE;

	return 0;
}

static int cs40l2x_pbq_play(struct cs40l2x_private *cs40l2x,
			    struct wt_type10_comp_section *section)
{
	unsigned int cp_dig_scale = cs40l2x->pbq_cp_dig_scale;
	int ret;

	cp_dig_scale += cs40l2x_pbq_dig_scale[section->amplitude];
	clamp_t(unsigned int, cp_dig_scale, 0, CS40L2X_DIG_SCALE_MAX);

	ret = cs40l2x_cp_dig_scale_set(cs40l2x, cp_dig_scale);
	if (ret)
		return ret;

	ret = cs40l2x_ground_amp(cs40l2x, false);
	if (ret)
		return ret;

	ret = cs40l2x_ack_write(cs40l2x, CS40L2X_MBOX_TRIGGERINDEX,
				section->index, CS40L2X_MBOX_TRIGGERRESET);
	if (ret)
		return ret;

	cs40l2x->pbq_state = CS40L2X_PBQ_STATE_PLAYING;

	if (cs40l2x->event_control & CS40L2X_EVENT_END_ENABLED)
		return 0;

	hrtimer_start(&cs40l2x->pbq_timer, ktime_set(0, CS40L2X_PBQ_POLL_NS),
		      HRTIMER_MODE_REL);

	return 0;
}

static int cs40l2x_pbq_pair_launch(struct cs40l2x_private *cs40l2x)
{
	struct wt_type10_comp_section *section;
	int ret;

	while (cs40l2x->pbq_index < (cs40l2x->pbq_comp.nsections << 1)) {
		section = &cs40l2x->pbq_comp.sections[cs40l2x->pbq_index >> 1];

		if (!(cs40l2x->pbq_index & 0x1)) {
			cs40l2x->pbq_index++;

			if (section->amplitude)
				return cs40l2x_pbq_play(cs40l2x, section);
		}

		if (section->delay) {
			ret = cs40l2x_ground_amp(cs40l2x, true);
			if (ret)
				return ret;

			hrtimer_start(&cs40l2x->pbq_timer,
				      ktime_set(section->delay / 1000,
						(section->delay % 1000) *
						 1000000),
				      HRTIMER_MODE_REL);

			cs40l2x->pbq_state = CS40L2X_PBQ_STATE_SILENT;
		}

		/* Handle inner loops */
		if (section->repeat == WT_REPEAT_LOOP_MARKER) {
			cs40l2x->pbq_inner_mark = cs40l2x->pbq_index & ~0x1;
		} else if (section->repeat) {
			if (++cs40l2x->pbq_inner_loop <= section->repeat) {
				if (cs40l2x->pbq_inner_mark >= 0)
					cs40l2x->pbq_index = cs40l2x->pbq_inner_mark;
				else
					cs40l2x->pbq_index &= ~0x1;
				continue;
			}

			cs40l2x->pbq_inner_mark = -1;
			cs40l2x->pbq_inner_loop = 0;
		}

		/* Handle outer loops */
		if (++cs40l2x->pbq_index == (cs40l2x->pbq_comp.nsections << 1)) {
			if (cs40l2x->pbq_comp.repeat != WT_REPEAT_LOOP_MARKER)
				cs40l2x->pbq_outer_loop++;

			if (cs40l2x->pbq_outer_loop > cs40l2x->pbq_comp.repeat)
				cs40l2x->pbq_outer_loop = 0;
			else
				cs40l2x->pbq_index = 0;
		}

		if (section->delay)
			return 0;
	}

	cs40l2x->pbq_state = CS40L2X_PBQ_STATE_IDLE;
	return cs40l2x_pbq_cancel(cs40l2x);
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
	mutex_lock(&cs40l2x->lock);

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
	mutex_unlock(&cs40l2x->lock);
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
	bool wl_pending = (cs40l2x->vibe_state == CS40L2X_VIBE_STATE_RUNNING);
	unsigned int fw_id_restore;
	int ret, i;

	if (cs40l2x->revid < CS40L2X_REVID_B1)
		return -EPERM;

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
	int ret;

	ret = regmap_read(cs40l2x->regmap, CS40L2X_DSP1RX2_INPUT, &val);
	if (ret) {
		dev_err(cs40l2x->dev, "Failed to read known register\n");
		return ret;
	}

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

	ret = regmap_update_bits(regmap, CS40L2X_VBST_CTL_1,
			CS40L2X_BST_CTL_MASK,
			bst_ctl_scaled << CS40L2X_BST_CTL_SHIFT);
	if (ret) {
		dev_err(dev, "Failed to write VBST limit\n");
		return ret;
	}

	ret = regmap_update_bits(regmap, CS40L2X_VBST_CTL_2,
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

	if (enable && cs40l2x_get_wave(cs40l2x, index)->flags & WT_FLAG_F0) {
		boost = cs40l2x->pdata.boost_ctl;
		disable_classh = true;
	}

	reg = cs40l2x_dsp_reg(cs40l2x, "CLAB_ENABLED",
			CS40L2X_XM_UNPACKED_TYPE, CS40L2X_ALGO_ID_CLAB);

	if (reg) {

		ret = regmap_read(regmap, reg, &enable);
		if (ret)
			return ret;

		if (enable &&
		    cs40l2x_get_wave(cs40l2x, index)->flags & WT_FLAG_CLAB) {
			boost = cs40l2x->pdata.boost_clab;
			disable_classh = true;
		}
	}

	if (disable_classh) {
		ret = cs40l2x_set_boost_voltage(cs40l2x, boost);
		if (ret)
			return ret;

		ret = regmap_update_bits(regmap, CS40L2X_VBST_CTL_2,
						CS40L2X_BST_CTL_SEL_MASK,
						CS40L2X_BST_CTL_SEL_CP_VAL);
		if (ret)
			return ret;

		ret = regmap_update_bits(regmap, CS40L2X_BLOCK_ENABLES2,
				CS40L2X_CLASSH_EN_MASK,
				0 << CS40L2X_CLASSH_EN_SHIFT);
		if (ret)
			return ret;
	} else {
		ret = regmap_update_bits(regmap, CS40L2X_VBST_CTL_2,
						CS40L2X_BST_CTL_SEL_MASK,
						CS40L2X_BST_CTL_SEL_CLASSH);
		if (ret)
			return ret;

		ret = regmap_update_bits(regmap, CS40L2X_BLOCK_ENABLES2,
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

	pm_runtime_get_sync(cs40l2x->dev);
	mutex_lock(&cs40l2x->lock);

#ifdef CONFIG_HAPTICS_CS40L2X_INPUT
	if (cs40l2x->effect) {
		i = cs40l2x->trigger_indices[cs40l2x->effect->id];
		cs40l2x_cp_trigger_index_impl(cs40l2x, i);
		cs40l2x->effect = NULL;
	}
#endif

	if (!cs40l2x->virtual_stored) {
		dev_warn(dev, "Unsafe condition encountered.\n");
		goto err_mutex;
	}

	cs40l2x_set_safe_save_state(cs40l2x, CS40L2X_SAVE_UNSAFE);

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
	/* intentional fall through */
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
#endif /* CONFIG_ANDROID_TIMED_OUTPUT */
	/* intentional fall through */
	case CS40L2X_INDEX_PBQ:
		if (cs40l2x->vibe_state != CS40L2X_VIBE_STATE_RUNNING)
			cs40l2x_wl_apply(cs40l2x);
		cs40l2x_set_state(cs40l2x, CS40L2X_VIBE_STATE_RUNNING);
		break;

	case CS40L2X_INDEX_CLICK_MIN ... CS40L2X_INDEX_CLICK_MAX:
		if (!(cs40l2x->event_control & CS40L2X_EVENT_END_ENABLED))
			break;

		if (cs40l2x->vibe_state != CS40L2X_VIBE_STATE_RUNNING)
			cs40l2x_wl_apply(cs40l2x);
		cs40l2x_set_state(cs40l2x, CS40L2X_VIBE_STATE_RUNNING);
		break;
	}

	if (cs40l2x->cp_trailer_index != CS40L2X_INDEX_PBQ) {
		ret = cs40l2x_ground_amp(cs40l2x, false);
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
				cs40l2x->cp_trailer_index & CS40L2X_INDEX_MASK);
			if (ret) {
				dev_err(dev, "Conditional ClassH failed\n");
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
		cs40l2x_wl_relax(cs40l2x);
	}

err_mutex:
	mutex_unlock(&cs40l2x->lock);
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
	mutex_lock(&cs40l2x->lock);

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

	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_put_autosuspend(cs40l2x->dev);
}

#ifdef CONFIG_HAPTICS_CS40L2X_INPUT
static int cs40l2x_playback_effect(struct input_dev *dev, int effect_id, int val)
{
	struct cs40l2x_private *cs40l2x = input_get_drvdata(dev);
	struct ff_effect *effect;

	mutex_lock(&cs40l2x->lock);

	effect = &dev->ff->effects[effect_id];
	if (!effect) {
		dev_err(cs40l2x->dev, "No such effect\n");
		return -EINVAL;
	}

	cs40l2x->effect = effect;

	mutex_unlock(&cs40l2x->lock);

	if (val > 0)
		queue_work(cs40l2x->vibe_workqueue, &cs40l2x->vibe_start_work);
	else
		queue_work(cs40l2x->vibe_workqueue, &cs40l2x->vibe_stop_work);

	return 0;
}

static int cs40l2x_upload_effect(struct input_dev *dev,
				 struct ff_effect *effect,
				 struct ff_effect *old)
{
	struct cs40l2x_private *cs40l2x = input_get_drvdata(dev);
	unsigned int data_length;
	s16 *raw_custom_data = NULL;
	int ret = 0;

	switch (effect->type) {
	case FF_PERIODIC:
		if (effect->u.periodic.waveform != FF_CUSTOM) {
			dev_err(cs40l2x->dev,
				"Waveform type must be FF_CUSTOM\n");
			return -EINVAL;
		}

		if (effect->replay.length < 0 ||
		    effect->replay.length > CS40L2X_TIMEOUT_MS_MAX) {
			dev_err(cs40l2x->dev,
				"Invalid playback duration %d ms\n",
				effect->replay.length);
			return -EINVAL;
		}

		data_length = effect->u.periodic.custom_len;

		raw_custom_data = kmalloc_array(data_length,
						sizeof(*raw_custom_data),
						GFP_KERNEL);
		if (!raw_custom_data)
			return -ENOMEM;

		if (copy_from_user(raw_custom_data,
				   effect->u.periodic.custom_data,
				   sizeof(*raw_custom_data) * data_length)) {
			dev_err(cs40l2x->dev, "Failed to get user data\n");
			ret = -EFAULT;
			goto err_free;
		}

		mutex_lock(&cs40l2x->lock);

		if (raw_custom_data[1] >= cs40l2x->num_waves) {
			dev_err(cs40l2x->dev, "Index out of bounds\n");
			ret = -EINVAL;
			goto err_free;
		}

		cs40l2x->trigger_indices[effect->id] = raw_custom_data[1];

		mutex_unlock(&cs40l2x->lock);
		break;
	default:
		dev_err(cs40l2x->dev, "Effect type 0x%X not supported\n",
			effect->type);
		ret = -EINVAL;
	}

err_free:
	kfree(raw_custom_data);
	return ret;
}

static int cs40l2x_create_input_ff(struct cs40l2x_private *cs40l2x)
{
	struct device *dev = cs40l2x->dev;
	int ret;

	cs40l2x->input = devm_input_allocate_device(cs40l2x->dev);
	if (!cs40l2x->input)
		return -ENOMEM;

	cs40l2x->input->name = "cs40l25_input";
	cs40l2x->input->id.product = cs40l2x->devid;
	cs40l2x->input->id.version = cs40l2x->revid;

	input_set_drvdata(cs40l2x->input, cs40l2x);
	input_set_capability(cs40l2x->input, EV_FF, FF_PERIODIC);
	input_set_capability(cs40l2x->input, EV_FF, FF_CUSTOM);

	ret = input_ff_create(cs40l2x->input, FF_MAX_EFFECTS);
	if (ret) {
		dev_err(dev, "Failed to create FF device: %d\n", ret);
		return ret;
	}

	/* input_ff_create() automatically sets FF_RUMBLE capabilities
	 * We want to restrict this to be only FF_PERIODIC
	 */
	__clear_bit(FF_RUMBLE, cs40l2x->input->ffbit);

	cs40l2x->input->ff->upload = cs40l2x_upload_effect;
	cs40l2x->input->ff->playback = cs40l2x_playback_effect;

	ret = input_register_device(cs40l2x->input);
	if (ret) {
		dev_err(dev, "Cannot register input device: %d\n", ret);
		return ret;
	}

	ret = sysfs_create_group(&cs40l2x->input->dev.kobj,
				 &cs40l2x_dev_attr_group);
	if (ret)
		dev_err(dev, "Failed to create sysfs group: %d\n", ret);

	return ret;
}
#elif defined CONFIG_ANDROID_TIMED_OUTPUT
/* vibration callback for timed output device */
static void cs40l2x_vibe_enable(struct timed_output_dev *sdev, int timeout)
{
	struct cs40l2x_private *cs40l2x =
		container_of(sdev, struct cs40l2x_private, timed_dev);

	if (timeout > 0) {
		mutex_lock(&cs40l2x->lock);
		cs40l2x->vibe_timeout = timeout;
		mutex_unlock(&cs40l2x->lock);

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
	struct led_init_data init_data = {
		.fwnode = cs40l2x->dev->fwnode,
		.devicename = "cs40l25",
		.default_label = CS40L2X_DEVICE_NAME,
	};

	led_dev->max_brightness = LED_FULL;
	led_dev->brightness_set = cs40l2x_vibe_brightness_set;
	led_dev->default_trigger = "transient";

	ret = led_classdev_register_ext(dev, led_dev, &init_data);
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
					cs40l2x->wt_xm.byteslimit =
						(cs40l2x->algo_info[i].xm_size
						- coeff_desc->block_offset) * 4;
				break;
			case CS40L2X_YM_UNPACKED_TYPE:
				coeff_desc->reg = CS40L2X_DSP1_YMEM_UNPACK24_0
					+ cs40l2x->algo_info[i].ym_base * 4
					+ coeff_desc->block_offset * 4;
				if (!strncmp(coeff_desc->name, "WAVETABLEYM",
						CS40L2X_COEFF_NAME_LEN_MAX))
					cs40l2x->wt_ym.byteslimit =
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

	if ((cs40l2x->algo_info[0].rev >= CS40L2X_COND_CLSH_MIN_REV) &&
		cs40l2x->pdata.cond_classh)
		cs40l2x->cond_class_h_en = true;

	if (cs40l2x->algo_info[0].rev >= CS40L2X_PBQ_DUR_MIN_REV)
		cs40l2x->comp_dur_min_fw = true;

	if (cs40l2x->algo_info[0].rev >= CS40L2X_PWLE_FRQ_MIN_REV)
		cs40l2x->ext_freq_min_fw = true;

	if (cs40l2x->algo_info[0].rev < CS40L2X_OWT_MIN_REV)
		cs40l2x->open_wt_enable = false;

	if (cs40l2x->algo_info[0].rev >= CS40L2X_A2H_REINIT_MIN_REV)
		cs40l2x->a2h_reinit_min_fw = true;

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
		ret = regmap_update_bits(regmap, CS40L2X_BLOCK_ENABLES,
				CS40L2X_BST_EN_MASK,
				CS40L2X_BST_DISABLED << CS40L2X_BST_EN_SHIFT);
		if (ret) {
			dev_err(dev, "Failed to disable boost converter\n");
			return ret;
		}
	}

	ret = regmap_write(regmap, CS40L2X_ERROR_RELEASE, 0);
	if (ret) {
		dev_err(dev, "Failed to cycle error release (step 1 of 3)\n");
		return ret;
	}

	ret = regmap_write(regmap, CS40L2X_ERROR_RELEASE,
			cs40l2x_hw_errs[i].rls_mask);
	if (ret) {
		dev_err(dev, "Failed to cycle error release (step 2 of 3)\n");
		return ret;
	}

	ret = regmap_write(regmap, CS40L2X_ERROR_RELEASE, 0);
	if (ret) {
		dev_err(dev, "Failed to cycle error release (step 3 of 3)\n");
		return ret;
	}

	if (cs40l2x_hw_errs[i].bst_cycle) {
		ret = regmap_update_bits(regmap, CS40L2X_BLOCK_ENABLES,
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

	ret = regmap_read(regmap, CS40L2X_IRQ2_EINT_1, &val);
	if (ret) {
		dev_err(dev, "Failed to read hardware error status\n");
		return ret;
	}

	for (i = 0; i < CS40L2X_NUM_HW_ERRS; i++) {
		if (!(val & cs40l2x_hw_errs[i].irq_mask))
			continue;

		dev_crit(dev, "Encountered %s error\n",
				cs40l2x_hw_errs[i].err_name);

		ret = regmap_write(regmap, CS40L2X_IRQ2_EINT_1,
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
	{CS40L2X_IRQ2_MASK_1,		0xFFFFFFFF},
	{CS40L2X_IRQ2_MASK_2,		0xFFFFFFFF},
	{CS40L2X_IRQ2_MASK_3,		0xFFFF87FF},
	{CS40L2X_IRQ2_MASK_4,		0xFEFFFFFF},
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
			if (cs40l2x_irq2_masks[i].reg == CS40L2X_IRQ2_MASK_1)
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
	{0x02BC2020,			0x00000000},
	{0x02BC20E0,			0x00000000},
	{CS40L2X_DSP1_SAMPLE_RATE_RX1,	0x00000001},
	{CS40L2X_DSP1_SAMPLE_RATE_RX2,	0x00000001},
	{CS40L2X_DSP1_SAMPLE_RATE_RX3,	0x00000001},
	{CS40L2X_DSP1_SAMPLE_RATE_RX4,	0x00000001},
	{CS40L2X_DSP1_SAMPLE_RATE_RX5,	0x00000001},
	{CS40L2X_DSP1_SAMPLE_RATE_RX6,	0x00000001},
	{CS40L2X_DSP1_SAMPLE_RATE_RX7,	0x00000001},
	{CS40L2X_DSP1_SAMPLE_RATE_RX8,	0x00000001},
	{CS40L2X_DSP1_SAMPLE_RATE_TX1,	0x00000001},
	{CS40L2X_DSP1_SAMPLE_RATE_TX2,	0x00000001},
	{CS40L2X_DSP1_SAMPLE_RATE_TX3,	0x00000001},
	{CS40L2X_DSP1_SAMPLE_RATE_TX4,	0x00000001},
	{CS40L2X_DSP1_SAMPLE_RATE_TX5,	0x00000001},
	{CS40L2X_DSP1_SAMPLE_RATE_TX6,	0x00000001},
	{CS40L2X_DSP1_SAMPLE_RATE_TX7,	0x00000001},
	{CS40L2X_DSP1_SAMPLE_RATE_TX8,	0x00000001},
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
		ret = regmap_update_bits(regmap, CS40L2X_GLOBAL_ENABLES,
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

	ret = regmap_update_bits(regmap, CS40L2X_DSP1_CCM_CORE_CONTROL,
				 CS40L2X_DSP1_CCM_CORE_RESET_MASK |
				 CS40L2X_DSP1_CCM_CORE_EN_MASK,
				 (1 << CS40L2X_DSP1_CCM_CORE_RESET_SHIFT) |
				 (1 << CS40L2X_DSP1_CCM_CORE_EN_SHIFT));
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
		/* A NACK is expected when waking from hibernate */
		if (reg != CS40L2X_MBOX_POWERCONTROL ||
		    write_val != CS40L2X_POWERCONTROL_WAKEUP)
			dev_err(dev, "Failed to write register 0x%08X: %d\n",
				reg, ret);
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
	int i = 0;

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

			if (((algo_rev >> 8) & CS40L2X_ALGO_REV_MASK) !=
			    (cs40l2x->algo_info[i].rev & CS40L2X_ALGO_REV_MASK)) {
				dev_warn(dev, "Algorithm revision mismatch: %d.%d.%d\n",
					 (algo_rev & 0xFF000000) >> 24,
					 (algo_rev & 0xFF0000) >> 16,
					 (algo_rev & 0xFF00) >> 8);
			}

			switch (algo_id) {
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
				if (block_length > cs40l2x->wt_xm.byteslimit) {
					dev_err(dev,
						"Wvtbl too big: %d bytes XM\n",
						block_length / 4 * 3);
					ret = -EINVAL;
					goto err_rls_fw;
				} else {
					if (wt_found) {
						memcpy(cs40l2x->pbq_fw_raw_wt,
							&fw->data[0],
							fw->size);
						ret = cs40l2x_read_wavetable(cs40l2x,
								(void *)&cs40l2x->pbq_fw_raw_wt[pos],
								block_length,
								&cs40l2x->wt_xm);
					}
				}
			}

			break;
		case CS40L2X_YM_UNPACKED_TYPE:
			reg = CS40L2X_DSP1_YMEM_UNPACK24_0 + block_offset
					+ cs40l2x->algo_info[i].ym_base * 4;

			if (reg == cs40l2x_dsp_reg(cs40l2x,
						"WAVETABLEYM",
						CS40L2X_YM_UNPACKED_TYPE,
						CS40L2X_ALGO_ID_VIBE)) {
				if (block_length > cs40l2x->wt_ym.byteslimit) {
					dev_err(dev,
						"Wvtbl too big: %d bytes YM\n",
						block_length / 4 * 3);
					ret = -EINVAL;
					goto err_rls_fw;
				} else {
					if (wt_found) {
						ret = cs40l2x_read_wavetable(cs40l2x,
								(void *)&cs40l2x->pbq_fw_raw_wt[pos],
								block_length,
								&cs40l2x->wt_ym);
					}
				}
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

#ifdef CONFIG_HAPTICS_CS40L2X_INPUT
	ret = cs40l2x_create_input_ff(cs40l2x);
#elif defined CONFIG_ANDROID_TIMED_OUTPUT
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

	dev_info(cs40l2x->dev, "Firmware ID 0x%06X\n",
		cs40l2x->algo_info[0].id);

	dev_info(cs40l2x->dev,
			"Max. wavetable size: %d bytes (XM), %d bytes (YM)\n",
			cs40l2x->wt_xm.byteslimit / 4 * 3,
			cs40l2x->wt_ym.byteslimit / 4 * 3);

	mutex_unlock(&cs40l2x->lock);
	pm_runtime_mark_last_busy(cs40l2x->dev);
	pm_runtime_set_active(dev);
	pm_runtime_enable(dev);

	pm_runtime_set_autosuspend_delay(dev, CS40L2X_AUTOSUSPEND_DELAY_MS);
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

	mutex_lock(&cs40l2x->lock);

	ret = cs40l2x_firmware_parse(cs40l2x, fw);
	if (ret)
		goto err_mutex;

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

err_mutex:
	mutex_unlock(&cs40l2x->lock);
}

static int cs40l2x_firmware_swap(struct cs40l2x_private *cs40l2x,
			unsigned int fw_id)
{
	const struct firmware *fw;
	struct regmap *regmap = cs40l2x->regmap;
	struct device *dev = cs40l2x->dev;
	int ret, i;

	if (cs40l2x->vibe_state == CS40L2X_VIBE_STATE_RUNNING)
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

	cs40l2x->dsp_reg = NULL;

	ret = regmap_update_bits(regmap, CS40L2X_DSP1_CCM_CORE_CONTROL,
				 CS40L2X_DSP1_CCM_CORE_EN_MASK, 0);
	if (ret) {
		dev_err(dev, "Failed to stop DSP\n");
		return ret;
	}

	cs40l2x_coeff_free(cs40l2x);

	if (fw_id == CS40L2X_FW_ID_CAL) {
		cs40l2x->diag_state = CS40L2X_DIAG_STATE_INIT;
		cs40l2x->dsp_cache_depth = 0;
		if (cs40l2x->open_wt_enable) {
			cs40l2x->open_wt_enable = false;
			cs40l2x->virtual_bin = false;
			cs40l2x->cal_disabled_owt = true;
		}
	}

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

		for (i = 0; i < cs40l2x->pbq_comp.nsections; i++) {
			tag = cs40l2x->pbq_comp.sections[i].index;

			if (tag >= cs40l2x->num_waves)
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

	ret = regmap_update_bits(regmap, CS40L2X_VBST_CTL_2,
			CS40L2X_BST_CTL_SEL_MASK,
			CS40L2X_BST_CTL_SEL_CP_VAL
				<< CS40L2X_BST_CTL_SEL_SHIFT);
	if (ret) {
		dev_err(dev, "Failed to change VBST target selection\n");
		return ret;
	}

	ret = regmap_update_bits(regmap, CS40L2X_GLOBAL_ENABLES,
			CS40L2X_GLOBAL_EN_MASK, 1 << CS40L2X_GLOBAL_EN_SHIFT);
	if (ret) {
		dev_err(dev, "Failed to enable device\n");
		return ret;
	}

	usleep_range(10000, 10100);

	ret = regmap_read(regmap, CS40L2X_IRQ1_EINT_1, &val);
	if (ret) {
		dev_err(dev, "Failed to read boost converter error status\n");
		return ret;
	}

	if (val & CS40L2X_BST_SHORT_ERR) {
		dev_err(dev, "Encountered fatal boost converter short error\n");
		return -EIO;
	}

	ret = regmap_update_bits(regmap, CS40L2X_GLOBAL_ENABLES,
			CS40L2X_GLOBAL_EN_MASK, 0 << CS40L2X_GLOBAL_EN_SHIFT);
	if (ret) {
		dev_err(dev, "Failed to disable device\n");
		return ret;
	}

	ret = regmap_update_bits(regmap, CS40L2X_VBST_CTL_2,
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

	ret = regmap_update_bits(regmap, CS40L2X_BST_LOOP_COEFF,
			CS40L2X_BST_K1_MASK,
			cs40l2x_bst_k1_table[bst_lbst_val][bst_cbst_range]
				<< CS40L2X_BST_K1_SHIFT);
	if (ret) {
		dev_err(dev, "Failed to write boost K1 coefficient\n");
		return ret;
	}

	ret = regmap_update_bits(regmap, CS40L2X_BST_LOOP_COEFF,
			CS40L2X_BST_K2_MASK,
			cs40l2x_bst_k2_table[bst_lbst_val][bst_cbst_range]
				<< CS40L2X_BST_K2_SHIFT);
	if (ret) {
		dev_err(dev, "Failed to write boost K2 coefficient\n");
		return ret;
	}

	ret = cs40l2x_wseq_add_reg(cs40l2x, CS40L2X_BST_LOOP_COEFF,
			(cs40l2x_bst_k2_table[bst_lbst_val][bst_cbst_range]
				<< CS40L2X_BST_K2_SHIFT) |
			(cs40l2x_bst_k1_table[bst_lbst_val][bst_cbst_range]
				<< CS40L2X_BST_K1_SHIFT));
	if (ret) {
		dev_err(dev, "Failed to sequence boost K1/K2 coefficients\n");
		return ret;
	}

	ret = regmap_update_bits(regmap, CS40L2X_LBST_SLOPE,
			CS40L2X_BST_SLOPE_MASK,
			cs40l2x_bst_slope_table[bst_lbst_val]
				<< CS40L2X_BST_SLOPE_SHIFT);
	if (ret) {
		dev_err(dev, "Failed to write boost slope coefficient\n");
		return ret;
	}

	ret = regmap_update_bits(regmap, CS40L2X_LBST_SLOPE,
			CS40L2X_BST_LBST_VAL_MASK,
			bst_lbst_val << CS40L2X_BST_LBST_VAL_SHIFT);
	if (ret) {
		dev_err(dev, "Failed to write boost inductor value\n");
		return ret;
	}

	ret = cs40l2x_wseq_add_reg(cs40l2x, CS40L2X_LBST_SLOPE,
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

	ret = regmap_update_bits(regmap, CS40L2X_BST_IPK_CTL,
			CS40L2X_BST_IPK_MASK,
			bst_ipk_scaled << CS40L2X_BST_IPK_SHIFT);
	if (ret) {
		dev_err(dev, "Failed to write boost inductor peak current\n");
		return ret;
	}

	ret = cs40l2x_wseq_add_reg(cs40l2x, CS40L2X_BST_IPK_CTL,
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

	ret = regmap_update_bits(regmap, CS40L2X_VBST_CTL_1,
			CS40L2X_BST_CTL_MASK,
			bst_ctl_scaled << CS40L2X_BST_CTL_SHIFT);
	if (ret) {
		dev_err(dev, "Failed to write VBST limit\n");
		return ret;
	}

	ret = cs40l2x_wseq_add_reg(cs40l2x, CS40L2X_VBST_CTL_1,
			bst_ctl_scaled << CS40L2X_BST_CTL_SHIFT);
	if (ret) {
		dev_err(dev, "Failed to sequence VBST limit\n");
		return ret;
	}

	switch (boost_ovp) {
	case 0:
		break;
	case 9000 ... 12875:
		bst_ovp_scaled = ((boost_ovp - 9000) / 125) * 2;

		ret = regmap_update_bits(regmap, CS40L2X_VBST_OVP,
				CS40L2X_BST_OVP_THLD_MASK,
				bst_ovp_scaled << CS40L2X_BST_OVP_THLD_SHIFT);
		if (ret) {
			dev_err(dev, "Failed to write OVP threshold\n");
			return ret;
		}

		ret = cs40l2x_wseq_add_reg(cs40l2x, CS40L2X_VBST_OVP,
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
	case CS40L2X_VPBR_CONFIG:
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

	case CS40L2X_VBBR_CONFIG:
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

	ret = regmap_read(regmap, CS40L2X_BLOCK_ENABLES2, &val);
	if (ret) {
		dev_err(dev, "Failed to read VPBR/VBBR enable controls\n");
		return ret;
	}

	val |= br_en_mask;

	ret = regmap_write(regmap, CS40L2X_BLOCK_ENABLES2, val);
	if (ret) {
		dev_err(dev, "Failed to write VPBR/VBBR enable controls\n");
		return ret;
	}

	ret = cs40l2x_wseq_replace(cs40l2x, CS40L2X_BLOCK_ENABLES2, val);
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

static int cs40l2x_classh_config(struct cs40l2x_private *cs40l2x)
{
	struct regmap *regmap = cs40l2x->regmap;
	unsigned int val_en, val_ctl;
	int ret;

	ret = regmap_read(regmap, CS40L2X_VBST_CTL_2, &val_ctl);
	if (ret)
		return ret;

	ret = regmap_read(regmap, CS40L2X_BLOCK_ENABLES2, &val_en);
	if (ret)
		return ret;

	val_ctl &= ~CS40L2X_BST_CTL_SEL_MASK;
	val_ctl |= 1 << CS40L2X_BST_CTL_LIM_EN_SHIFT;

	if (!cs40l2x->pdata.cond_classh) {
		val_en &= ~(1 << CS40L2X_CLASSH_EN_SHIFT);
	} else {
		val_en |= 1 << CS40L2X_CLASSH_EN_SHIFT;
		val_ctl |= CS40L2X_BST_CTL_SEL_CLASSH;
	}

	ret = regmap_write(regmap, CS40L2X_VBST_CTL_2, val_ctl);
	if (ret)
		return ret;

	ret = cs40l2x_wseq_add_reg(cs40l2x, CS40L2X_VBST_CTL_2,
				val_ctl);
	if (ret)
		return ret;

	ret = regmap_write(regmap, CS40L2X_BLOCK_ENABLES2, val_en);
	if (ret)
		return ret;

	return cs40l2x_wseq_add_reg(cs40l2x, CS40L2X_BLOCK_ENABLES2, val_en);
}

static const struct reg_sequence cs40l2x_mpu_config[] = {
	{CS40L2X_DSP1_MPU_LOCK_CONFIG,		CS40L2X_MPU_UNLOCK_CODE1},
	{CS40L2X_DSP1_MPU_LOCK_CONFIG,		CS40L2X_MPU_UNLOCK_CODE2},
	{CS40L2X_DSP1_MPU_XM_ACCESS_0,		0xFFFFFFFF},
	{CS40L2X_DSP1_MPU_YM_ACCESS_0,		0xFFFFFFFF},
	{CS40L2X_DSP1_MPU_WINDOW_ACCESS_0,	0xFFFFFFFF},
	{CS40L2X_DSP1_MPU_XREG_ACCESS_0,	0xFFFFFFFF},
	{CS40L2X_DSP1_MPU_YREG_ACCESS_0,	0xFFFFFFFF},
	{CS40L2X_DSP1_MPU_WINDOW_ACCESS_1,	0xFFFFFFFF},
	{CS40L2X_DSP1_MPU_XREG_ACCESS_1,	0xFFFFFFFF},
	{CS40L2X_DSP1_MPU_YREG_ACCESS_1,	0xFFFFFFFF},
	{CS40L2X_DSP1_MPU_WINDOW_ACCESS_2,	0xFFFFFFFF},
	{CS40L2X_DSP1_MPU_XREG_ACCESS_2,	0xFFFFFFFF},
	{CS40L2X_DSP1_MPU_YREG_ACCESS_2,	0xFFFFFFFF},
	{CS40L2X_DSP1_MPU_WINDOW_ACCESS_3,	0xFFFFFFFF},
	{CS40L2X_DSP1_MPU_XREG_ACCESS_3,	0xFFFFFFFF},
	{CS40L2X_DSP1_MPU_YREG_ACCESS_3,	0xFFFFFFFF},
	{CS40L2X_DSP1_MPU_LOCK_CONFIG,		0x00000000}
};

static const struct reg_sequence cs40l2x_pcm_routing[] = {
	{CS40L2X_DACPCM1_INPUT,		CS40L2X_DACPCM1_SRC_DSP1TX1},
	{CS40L2X_DSP1RX1_INPUT,		CS40L2X_DSP1_RXn_SRC_ASPRX1},
	{CS40L2X_DSP1RX2_INPUT,		CS40L2X_DSP1_RXn_SRC_VMON},
	{CS40L2X_DSP1RX3_INPUT,		CS40L2X_DSP1_RXn_SRC_IMON},
	{CS40L2X_DSP1RX4_INPUT,		CS40L2X_DSP1_RXn_SRC_VPMON},
	{CS40L2X_DSP1RX5_INPUT,		CS40L2X_DSP1_RXn_SRC_ASPRX2},
	{CS40L2X_DSP1RX8_INPUT,		CS40L2X_DSP1_RXn_SRC_VBSTMON},
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

		ret = regmap_update_bits(regmap, CS40L2X_REFCLK_INPUT,
				CS40L2X_PLL_REFCLK_SEL_MASK,
				CS40L2X_PLL_REFCLK_SEL_MCLK
					<< CS40L2X_PLL_REFCLK_SEL_SHIFT);
		if (ret) {
			dev_err(dev, "Failed to select clock source\n");
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

	ret = cs40l2x_wseq_add_reg(cs40l2x, CS40L2X_AMP_CTRL,
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

	if (cs40l2x->pdata.dcm_disable) {
		ret = regmap_write(regmap, CS40L2X_BST_DCM_CTL,
				CS40L2X_DCM_DISABLE);
		if (ret)
			return ret;

		ret = cs40l2x_wseq_add_reg(cs40l2x, CS40L2X_BST_DCM_CTL,
					CS40L2X_DCM_DISABLE);
		if (ret) {
			dev_err(dev, "Failed to sequence DCM Control\n");
			return ret;
		}
	}

	ret = cs40l2x_classh_config(cs40l2x);
	if (ret)
		return ret;

	ret =  cs40l2x_brownout_config(cs40l2x, CS40L2X_VPBR_CONFIG);
	if (ret)
		return ret;

	return cs40l2x_brownout_config(cs40l2x, CS40L2X_VBBR_CONFIG);
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

	ret = of_property_read_u32(np, "cirrus,asp-slot-num", &out_val);
	if (!ret) {
		if (out_val > CS40L2X_ASP_RX1_SLOT_MAX)
			dev_warn(dev, "Invalid ASP slot number: %d\n",
				 pdata->asp_slot_num);
		else
			pdata->asp_slot_num = out_val;
	}


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

	pdata->dcm_disable = of_property_read_bool(np,
			"cirrus,dcm-disable");

	pdata->cond_classh = of_property_read_bool(np,
			"cirrus,cond-classh");

	return 0;
}

static const struct reg_sequence cs40l2x_basic_mode_revert[] = {
	{CS40L2X_GLOBAL_ENABLES,	0x00000000},
	{CS40L2X_BLOCK_ENABLES,		0x00003321},
	{CS40L2X_LRCK_PAD_CONTROL,	0x00000007},
	{CS40L2X_SDIN_PAD_CONTROL,	0x00000007},
	{CS40L2X_AMP_CTRL,		0x00008000},
	{CS40L2X_IRQ2_MASK_1,		0xFFFFFFFF},
	{CS40L2X_IRQ2_MASK_2,		0xFFFFFFFF},
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
	{0x3008,			0x000C1837},
	{0x3014,			0x03008E0E},
	{CS40L2X_BST_DCM_CTL,		0x00000051},
	{0x0054,			0x00000004},
	{CS40L2X_IRQ1_DB_3,		0x00000000},
	{CS40L2X_IRQ2_DB_3,		0x00000000},
	{CS40L2X_TEST_KEY_CTL,		CS40L2X_TEST_KEY_UNLOCK_CODE1},
	{CS40L2X_TEST_KEY_CTL,		CS40L2X_TEST_KEY_UNLOCK_CODE2},
	{0x4100,			0x00000000},
	{0x4310,			0x00000000},
	{CS40L2X_TEST_KEY_CTL,		CS40L2X_TEST_KEY_RELOCK_CODE1},
	{CS40L2X_TEST_KEY_CTL,		CS40L2X_TEST_KEY_RELOCK_CODE2},
	{0x4400,			0x00000000},
};

static const struct reg_sequence cs40l2x_rev_b0_errata[] = {
	{0x3008,			0x000C1837},
	{0x3014,			0x03008E0E},
	{CS40L2X_TEST_KEY_CTL,		CS40L2X_TEST_KEY_UNLOCK_CODE1},
	{CS40L2X_TEST_KEY_CTL,		CS40L2X_TEST_KEY_UNLOCK_CODE2},
	{CS40L2X_TEST_LBST,		CS40L2X_EXPL_MODE_EN},
	{CS40L2X_OTP_TRIM_12,		0x002F0065},
	{CS40L2X_OTP_TRIM_13,		0x00002B4F},
	{0x4100,			0x00000000},
	{0x4310,			0x00000000},
	{CS40L2X_TEST_KEY_CTL,		CS40L2X_TEST_KEY_RELOCK_CODE1},
	{CS40L2X_TEST_KEY_CTL,		CS40L2X_TEST_KEY_RELOCK_CODE2},
	{0x4400,			0x00000000},
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

		ret = regmap_read(regmap, CS40L2X_IRQ1_EINT_4, &val);
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

	ret = regmap_read(regmap, CS40L2X_IRQ1_EINT_3, &val);
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
	mutex_lock(&cs40l2x->lock);

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
			if (val == CS40L2X_EVENT_CTRL_GPIO_STOP)
				cs40l2x_set_gpio_event(cs40l2x, true);
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
	mutex_unlock(&cs40l2x->lock);
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

	cs40l2x->wt_xm.waves[0].type = WT_TYPE_TERMINATOR;
	cs40l2x->wt_xm.waves[0].flags = WT_FLAG_TERMINATOR;
	cs40l2x->wt_ym.waves[0].type = WT_TYPE_TERMINATOR;
	cs40l2x->wt_ym.waves[0].flags = WT_FLAG_TERMINATOR;

	strlcpy(cs40l2x->wt_file,
			CS40L2X_WT_FILE_NAME_MISSING,
			CS40L2X_WT_FILE_NAME_LEN_MAX);
	strlcpy(cs40l2x->wt_date,
			CS40L2X_WT_FILE_DATE_MISSING,
			CS40L2X_WT_FILE_DATE_LEN_MAX);

	/* Virtual wavetable slots mem init */
	cs40l2x->pbq_updated_fw_raw_wt = devm_kzalloc(dev,
		CS40L2X_WT_MAX_BIN_SIZE, GFP_KERNEL);
	if (!cs40l2x->pbq_updated_fw_raw_wt)
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

	ret = devm_mfd_add_devices(dev, PLATFORM_DEVID_AUTO, cs40l2x_devs,
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
#ifdef CONFIG_HAPTICS_CS40L2X_INPUT
		input_unregister_device(cs40l2x->input);
		sysfs_remove_group(&cs40l2x->input->dev.kobj,
				   &cs40l2x_dev_attr_group);
#elif defined CONFIG_ANDROID_TIMED_OUTPUT
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
			dev_err(dev, "Failed to enable GPIO1 upon suspend: %d\n",
				ret);
			goto err_mutex;
		}
	}

	if (cs40l2x->pdata.hiber_enable
			&& cs40l2x->fw_desc->id != CS40L2X_FW_ID_CAL
			&& cs40l2x->fw_desc->id != CS40L2X_FW_ID_ORIG) {
		ret = cs40l2x_hiber_cmd_send(cs40l2x,
				CS40L2X_POWERCONTROL_HIBERNATE);
		if (ret)
			dev_err(dev, "Failed to hibernate upon suspend: %d\n",
				ret);
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
			dev_err(dev, "Failed to wake up upon resume: %d\n",
				ret);
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
			dev_err(dev, "Failed to disable GPIO1 upon resume; %d\n",
				ret);
	}

err_mutex:
	mutex_unlock(&cs40l2x->lock);

	return ret;
}

static int cs40l2x_sys_suspend(struct device *dev)
{
	struct cs40l2x_private *cs40l2x = dev_get_drvdata(dev);
	struct i2c_client *i2c_client = to_i2c_client(dev);

	dev_dbg(cs40l2x->dev, "System suspend, disabling IRQ\n");
	disable_irq(i2c_client->irq);

	return 0;
}

static int cs40l2x_sys_suspend_noirq(struct device *dev)
{
	struct cs40l2x_private *cs40l2x = dev_get_drvdata(dev);
	struct i2c_client *i2c_client = to_i2c_client(dev);

	dev_dbg(cs40l2x->dev, "Late system suspend, reenabling IRQ\n");
	enable_irq(i2c_client->irq);

	return 0;
}

static int cs40l2x_sys_resume_noirq(struct device *dev)
{
	struct cs40l2x_private *cs40l2x = dev_get_drvdata(dev);
	struct i2c_client *i2c_client = to_i2c_client(dev);

	dev_dbg(cs40l2x->dev, "Early system resume, disabling IRQ\n");
	disable_irq(i2c_client->irq);

	return 0;
}

static int cs40l2x_sys_resume(struct device *dev)
{
	struct cs40l2x_private *cs40l2x = dev_get_drvdata(dev);
	struct i2c_client *i2c_client = to_i2c_client(dev);

	dev_dbg(cs40l2x->dev, "System resume, reenabling IRQ\n");
	enable_irq(i2c_client->irq);

	return 0;
}

static const struct dev_pm_ops cs40l2x_pm_ops = {
	SET_RUNTIME_PM_OPS(cs40l2x_suspend, cs40l2x_resume, NULL)

	SET_SYSTEM_SLEEP_PM_OPS(cs40l2x_sys_suspend, cs40l2x_sys_resume)
	SET_NOIRQ_SYSTEM_SLEEP_PM_OPS(cs40l2x_sys_suspend_noirq,
				      cs40l2x_sys_resume_noirq)
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
