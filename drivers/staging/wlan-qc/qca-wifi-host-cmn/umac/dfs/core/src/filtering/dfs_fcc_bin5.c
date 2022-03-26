/*
 * Copyright (c) 2013, 2016-2020 The Linux Foundation. All rights reserved.
 * Copyright (c) 2002-2010, Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: FCC Bin5 are special type of radars because they "chirp". Basically the
 * pulses move across the frequency band and are called chirping pulses.
 * dfs_check_chirping() actually examines the FFT data contained in the PHY
 * error information to figure out whether the pulse is moving across
 * frequencies.
 */

#include "../dfs.h"
#include "wlan_dfs_mlme_api.h"
#include "../dfs_channel.h"

int dfs_bin5_check_pulse(struct wlan_dfs *dfs, struct dfs_event *re,
		struct dfs_bin5radars *br)
{
	int b5_rssithresh = br->br_pulse.b5_rssithresh;

	dfs_debug(dfs, WLAN_DEBUG_DFS_BIN5_PULSE,
		"re_dur=%d, rssi=%d, check_chirp=%d, hw_chirp=%d, sw_chirp=%d",
		 (int)re->re_dur, (int)re->re_rssi,
		!!(re->re_flags & DFS_EVENT_CHECKCHIRP),
		!!(re->re_flags & DFS_EVENT_HW_CHIRP),
		!!(re->re_flags & DFS_EVENT_SW_CHIRP));

	/* If the SW/HW chirp detection says to fail the pulse,do so. */
	if (DFS_EVENT_NOTCHIRP(re)) {
		dfs_debug(dfs, WLAN_DEBUG_DFS_BIN5,
			"rejecting chirp: ts=%llu, dur=%d, rssi=%d checkchirp=%d, hwchirp=%d, swchirp=%d",
			 (unsigned long long)re->re_full_ts,
			(int)re->re_dur, (int)re->re_rssi,
			!!(re->re_flags & DFS_EVENT_CHECKCHIRP),
			!!(re->re_flags & DFS_EVENT_HW_CHIRP),
			!!(re->re_flags & DFS_EVENT_SW_CHIRP));

		return 0;
	}

#define CHANNEL_TURBO 0x00010
	/* Adjust the filter threshold for rssi in non TURBO mode. */
	if (!(dfs->dfs_curchan->dfs_ch_flags & CHANNEL_TURBO))
		b5_rssithresh += br->br_pulse.b5_rssimargin;

	/* Check if the pulse is within duration and rssi thresholds. */
	if ((re->re_dur >= br->br_pulse.b5_mindur) &&
			(re->re_dur <= br->br_pulse.b5_maxdur) &&
			(re->re_rssi >= b5_rssithresh)) {
		dfs_debug(dfs, WLAN_DEBUG_DFS_BIN5,
			"dur=%d, rssi=%d - adding!",
			 (int)re->re_dur, (int)re->re_rssi);
		return 1;
	}

	dfs_debug(dfs, WLAN_DEBUG_DFS_BIN5,
		"too low to be Bin5 pulse tsf=%llu, dur=%d, rssi=%d",
		 (unsigned long long)re->re_full_ts,
		(int)re->re_dur, (int)re->re_rssi);

	return 0;
}

int dfs_bin5_addpulse(struct wlan_dfs *dfs,
		struct dfs_bin5radars *br,
		struct dfs_event *re,
		uint64_t thists)
{
	uint32_t index, stop;
	uint64_t tsDelta;

	/*
	 * Check if this pulse is a valid pulse in terms of repetition,
	 * if not, return without adding it to the queue. PRI : Pulse
	 * Repitetion Interval.
	 * BRI : Burst Repitetion Interval.
	 */
	if (br->br_numelems != 0) {
		index = br->br_lastelem;
		tsDelta = thists - br->br_elems[index].be_ts;
		if ((tsDelta < DFS_BIN5_PRI_LOWER_LIMIT) ||
				((tsDelta > DFS_BIN5_PRI_HIGHER_LIMIT) &&
		 (tsDelta < DFS_BIN5_BRI_LOWER_LIMIT))) {
			return 0;
		}
	}

	if (dfs->dfs_min_sidx > re->re_sidx)
		dfs->dfs_min_sidx = re->re_sidx;

	if (dfs->dfs_max_sidx < re->re_sidx)
		dfs->dfs_max_sidx = re->re_sidx;
	/* Circular buffer of size 2^n. */
	index = (br->br_lastelem + 1) & DFS_MAX_B5_MASK;
	br->br_lastelem = index;
	if (br->br_numelems == DFS_MAX_B5_SIZE)
		br->br_firstelem = (br->br_firstelem + 1) & DFS_MAX_B5_MASK;
	else
		br->br_numelems++;

	br->br_elems[index].be_ts = thists;
	br->br_elems[index].be_rssi = re->re_rssi;
	br->br_elems[index].be_dur = re->re_dur;  /* This is in u-sec */
	stop = 0;
	index = br->br_firstelem;
	while ((!stop) && (br->br_numelems - 1) > 0) {
		if ((thists - br->br_elems[index].be_ts) >
				((uint64_t)br->br_pulse.b5_timewindow)) {
			br->br_numelems--;
			br->br_firstelem =
				(br->br_firstelem + 1) & DFS_MAX_B5_MASK;
			index = br->br_firstelem;
		} else {
			stop = 1;
		}
	}

	return 1;
}

/**
 * dfs_calculate_bursts_for_same_rssi() - Calculate bursts for same rssi.
 * @dfs: Pointer to wlan_dfs structure.
 * @br: Pointer to dfs_bin5radars structure.
 * @bursts: Bursts.
 * @numevents: Number of events.
 * @prev: prev index.
 * @this: index to br_elems[].
 * @index: index array.
 */
static inline void dfs_calculate_bursts_for_same_rssi(
		struct wlan_dfs *dfs,
		struct dfs_bin5radars *br,
		uint32_t *bursts,
		uint32_t *numevents,
		uint32_t prev,
		uint32_t this,
		int *index)
{
	uint32_t rssi_diff;

	if (br->br_elems[this].be_rssi >= br->br_elems[prev].be_rssi)
		rssi_diff = (br->br_elems[this].be_rssi -
				br->br_elems[prev].be_rssi);
	else
		rssi_diff = (br->br_elems[prev].be_rssi -
				br->br_elems[this].be_rssi);

	if (rssi_diff <= DFS_BIN5_RSSI_MARGIN) {
		(*bursts)++;
		/*
		 * Save the indexes of this pair for later
		 * width variance check.
		 */
		if ((*numevents) >= 2) {
			/*
			 * Make sure the event is not duplicated, possible in
			 * a 3 pulse burst.
			 */
			if (index[(*numevents)-1] != prev)
				index[(*numevents)++] = prev;
		} else {
			index[(*numevents)++] = prev;
		}

		index[(*numevents)++] = this;
	} else {
		dfs_debug(dfs, WLAN_DEBUG_DFS_BIN5,
				"Bin5 rssi_diff=%d", rssi_diff);
	}
}

void bin5_rules_check_internal(struct wlan_dfs *dfs,
		struct dfs_bin5radars *br,
		uint32_t *bursts,
		uint32_t *numevents,
		uint32_t prev,
		uint32_t i,
		uint32_t this,
		int *index)
{
	uint64_t pri = 0;
	uint32_t width_diff = 0;

	/* Rule 1: 1000 <= PRI <= 2000 + some margin. */
	if (br->br_elems[this].be_ts >= br->br_elems[prev].be_ts) {
		pri = br->br_elems[this].be_ts - br->br_elems[prev].be_ts;
	} else {
		/* Roll over case */
		pri = br->br_elems[this].be_ts;
	}
	dfs_debug(dfs, WLAN_DEBUG_DFS_BIN5,
		" pri=%llu this.ts=%llu this.dur=%d this.rssi=%d prev.ts=%llu",
		(uint64_t)pri,
		(uint64_t) br->br_elems[this].be_ts,
		(int) br->br_elems[this].be_dur,
		(int) br->br_elems[this].be_rssi,
		(uint64_t)br->br_elems[prev].be_ts);

	if (((pri >= DFS_BIN5_PRI_LOWER_LIMIT) &&
		    /*pri: pulse repitition interval in us. */
		    (pri <= DFS_BIN5_PRI_HIGHER_LIMIT))) {
		/*
		 * Rule 2: pulse width of the pulses in the
		 * burst should be same (+/- margin).
		 */
		if (br->br_elems[this].be_dur >= br->br_elems[prev].be_dur) {
			width_diff = (br->br_elems[this].be_dur
					- br->br_elems[prev].be_dur);
		} else {
			width_diff = (br->br_elems[prev].be_dur
					- br->br_elems[this].be_dur);
		}

		if (width_diff <= DFS_BIN5_WIDTH_MARGIN)
			/*
			 * Rule 3: RSSI of the pulses in the
			 * burst should be same (+/- margin)
			 */
			dfs_calculate_bursts_for_same_rssi(dfs, br, bursts,
					numevents, prev, this, index);
		 else
			dfs_debug(dfs, WLAN_DEBUG_DFS_BIN5,
				"Bin5 width_diff=%d", width_diff);
	} else if ((pri >= DFS_BIN5_BRI_LOWER_LIMIT) &&
			(pri <= DFS_BIN5_BRI_UPPER_LIMIT)) {
		/* Check pulse width to make sure it is in range of bin 5. */
		(*bursts)++;
	} else{
		dfs_debug(dfs, WLAN_DEBUG_DFS_BIN5,
			"Bin5 PRI check fail pri=%llu", (uint64_t)pri);
	}
}

int dfs_bin5_check(struct wlan_dfs *dfs)
{
	struct dfs_bin5radars *br;
	uint32_t n = 0, i = 0, i1 = 0, this = 0, prev = 0;
	uint32_t bursts = 0, total_diff = 0, average_diff = 0;
	uint32_t total_width = 0, average_width = 0, numevents = 0;
	int index[DFS_MAX_B5_SIZE];

	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return 1;
	}

	for (n = 0; n < dfs->dfs_rinfo.rn_numbin5radars; n++) {
		br = &(dfs->dfs_b5radars[n]);
		dfs_debug(dfs, WLAN_DEBUG_DFS_BIN5, "Num elems = %d",
				br->br_numelems);

		/* Find a valid bin 5 pulse and use it as reference. */
		for (i1 = 0; i1 < br->br_numelems; i1++) {
			this = ((br->br_firstelem + i1) & DFS_MAX_B5_MASK);
			if ((br->br_elems[this].be_dur >= MIN_BIN5_DUR_MICROSEC)
				&& (br->br_elems[this].be_dur <=
				    MAX_BIN5_DUR_MICROSEC)) {
				break;
			}
		}

		prev = this;
		for (i = i1 + 1; i < br->br_numelems; i++) {
			this = ((br->br_firstelem + i) & DFS_MAX_B5_MASK);
			/*
			 * First make sure it is a bin 5 pulse by checking
			 * the duration.
			 */
			if ((br->br_elems[this].be_dur < MIN_BIN5_DUR_MICROSEC)
				|| (br->br_elems[this].be_dur >
				    MAX_BIN5_DUR_MICROSEC)) {
				continue;
			}
			bin5_rules_check_internal(dfs, br, &bursts, &numevents,
					prev, i, this, index);
			prev = this;
		}

		dfs_debug(dfs, WLAN_DEBUG_DFS_BIN5,
			  "bursts=%u numevents=%u", bursts, numevents);
		if (bursts >= br->br_pulse.b5_threshold) {
			if ((br->br_elems[br->br_lastelem].be_ts -
					br->br_elems[br->br_firstelem].be_ts) <
					3000000)
				return 0;

			dfs_debug(dfs, WLAN_DEBUG_DFS_BIN5,
				  "bursts=%u numevents=%u total_width=%d average_width=%d total_diff=%d average_diff=%d",
				   bursts, numevents, total_width,
				   average_width, total_diff,
				   average_diff);
			dfs_debug(dfs, WLAN_DEBUG_DFS_ALWAYS,
				  "bin 5 radar detected, bursts=%d",
				   bursts);
			return 1;
		}
	}

	return 0;
}

/**
 * dfs_check_chirping_sowl() - Chirp detection for Sowl/Howl.
 * @dfs: Pointer to wlan_dfs structure.
 * @buf: Phyerr buffer.
 * @datalen: Phyerr buf length
 * @is_ctl: detected on primary channel.
 * @is_ext: detected on extension channel.
 * @slope: Slope
 * @is_dc: DC found
 *
 * Return: Return TRUE if chirping pulse, FALSE if not. Decision is made
 * based on processing the FFT data included with the PHY error.
 * Calculate the slope using the maximum bin index reported in
 * the FFT data. Calculate slope between FFT packet 0 and packet
 * n-1. Also calculate slope between packet 1 and packet n. If a
 * pulse is chirping, a slope of 5 and greater is seen.
 * Non-chirping pulses have slopes of 0, 1, 2 or 3.
 */
static int dfs_check_chirping_sowl(struct wlan_dfs *dfs,
		void *buf,
		uint16_t datalen,
		int is_ctl,
		int is_ext,
		int *slope,
		int *is_dc)
{
#define FFT_LEN 70
#define FFT_LOWER_BIN_MAX_INDEX_BYTE 66
#define FFT_UPPER_BIN_MAX_INDEX_BYTE 69
#define MIN_CHIRPING_SLOPE 4
	int is_chirp = 0;
	int p, num_fft_packets = 0;
	int ctl_slope = 0, ext_slope = 0;
	int ctl_high0 = 0, ctl_low0 = 0, ctl_slope0 = 0;
	int ext_high0 = 0, ext_low0 = 0, ext_slope0 = 0;
	int ctl_high1 = 0, ctl_low1 = 0, ctl_slope1 = 0;
	int ext_high1 = 0, ext_low1 = 0, ext_slope1 = 0;
	uint8_t *fft_data_ptr;

	*slope = 0;
	*is_dc = 0;
	num_fft_packets = datalen / FFT_LEN;
	fft_data_ptr = (uint8_t *)buf;

	/* DEBUG - Print relevant portions of the FFT data. */
	for (p = 0; p < num_fft_packets; p++) {
		dfs_debug(dfs, WLAN_DEBUG_DFS_BIN5_FFT,
			"fft_data_ptr=0x%pK\t", fft_data_ptr);
		dfs_debug(dfs, WLAN_DEBUG_DFS_BIN5_FFT,
			"[66]=%d [69]=%d",
			*(fft_data_ptr + FFT_LOWER_BIN_MAX_INDEX_BYTE) >> 2,
			*(fft_data_ptr + FFT_UPPER_BIN_MAX_INDEX_BYTE) >> 2);
		fft_data_ptr += FFT_LEN;
	}

	dfs_debug(dfs, WLAN_DEBUG_DFS_BIN5_FFT,
		"datalen=%d num_fft_packets=%d", datalen, num_fft_packets);

	/*
	 * There is not enough FFT data to figure out whether the pulse
	 * is chirping or not.
	 */
	if (num_fft_packets < 4)
		return 0;

	fft_data_ptr = (uint8_t *)buf;

	if (is_ctl) {
		fft_data_ptr = (uint8_t *)buf;
		ctl_low0 = *(fft_data_ptr + FFT_LOWER_BIN_MAX_INDEX_BYTE) >>  2;
		fft_data_ptr += FFT_LEN;
		ctl_low1 = *(fft_data_ptr + FFT_LOWER_BIN_MAX_INDEX_BYTE) >>  2;

		/* Last packet with first packet. */
		fft_data_ptr =
		    (uint8_t *)buf + (FFT_LEN * (num_fft_packets - 1));
		ctl_high1 = *(fft_data_ptr + FFT_LOWER_BIN_MAX_INDEX_BYTE) >> 2;

		/* Second last packet with 0th packet. */
		fft_data_ptr =
		    (uint8_t *)buf + (FFT_LEN * (num_fft_packets - 2));
		ctl_high0 = *(fft_data_ptr + FFT_LOWER_BIN_MAX_INDEX_BYTE) >> 2;

		ctl_slope0 = ctl_high0 - ctl_low0;
		if (ctl_slope0 < 0)
			ctl_slope0 *= (-1);

		ctl_slope1 = ctl_high1 - ctl_low1;
		if (ctl_slope1 < 0)
			ctl_slope1 *= (-1);

		ctl_slope =
		    ((ctl_slope0 > ctl_slope1) ? ctl_slope0 : ctl_slope1);
		*slope = ctl_slope;

		dfs_debug(dfs, WLAN_DEBUG_DFS_BIN5_FFT,
			"ctl_slope0=%d ctl_slope1=%d ctl_slope=%d",
			ctl_slope0, ctl_slope1, ctl_slope);
	} else if (is_ext) {
		fft_data_ptr = (uint8_t *)buf;
		ext_low0 = *(fft_data_ptr + FFT_UPPER_BIN_MAX_INDEX_BYTE) >>  2;

		fft_data_ptr += FFT_LEN;
		ext_low1 = *(fft_data_ptr + FFT_UPPER_BIN_MAX_INDEX_BYTE) >>  2;

		fft_data_ptr =
		    (uint8_t *)buf + (FFT_LEN * (num_fft_packets - 1));
		ext_high1 = *(fft_data_ptr + FFT_UPPER_BIN_MAX_INDEX_BYTE) >> 2;
		fft_data_ptr =
		    (uint8_t *)buf + (FFT_LEN * (num_fft_packets - 2));

		ext_high0 = *(fft_data_ptr + FFT_UPPER_BIN_MAX_INDEX_BYTE) >> 2;

		ext_slope0 = ext_high0 - ext_low0;
		if (ext_slope0 < 0)
			ext_slope0 *= (-1);

		ext_slope1 = ext_high1 - ext_low1;
		if (ext_slope1 < 0)
			ext_slope1 *= (-1);

		ext_slope = ((ext_slope0 > ext_slope1) ?
				ext_slope0 : ext_slope1);
		*slope = ext_slope;
		dfs_debug(dfs, WLAN_DEBUG_DFS_BIN5_FFT | WLAN_DEBUG_DFS_BIN5,
			"ext_slope0=%d ext_slope1=%d ext_slope=%d",
			ext_slope0, ext_slope1, ext_slope);
	} else
		return 0;

	if ((ctl_slope >= MIN_CHIRPING_SLOPE) ||
			(ext_slope >= MIN_CHIRPING_SLOPE)) {
		is_chirp = 1;
		dfs_debug(dfs, WLAN_DEBUG_DFS_BIN5 | WLAN_DEBUG_DFS_BIN5_FFT |
			WLAN_DEBUG_DFS_PHYERR_SUM, "is_chirp=%d is_dc=%d",
			is_chirp, *is_dc);
	}

	return is_chirp;

#undef FFT_LEN
#undef FFT_LOWER_BIN_MAX_INDEX_BYTE
#undef FFT_UPPER_BIN_MAX_INDEX_BYTE
#undef MIN_CHIRPING_SLOPE
}

/**
 * dfs_check_chirping_merlin() - Merlin (and Osprey, etc) chirp radar chirp
 *                               detection.
 * @dfs: Pointer to wlan_dfs structure.
 * @buf: Phyerr buffer
 * @datalen: Phyerr buf length
 * @is_ctl: detected on primary channel.
 * @is_ext: detected on extension channel.
 * @slope: Slope
 * @is_dc: DC found
 */
static int dfs_check_chirping_merlin(struct wlan_dfs *dfs,
		void *buf,
		uint16_t datalen,
		int is_ctl,
		int is_ext,
		int *slope,
		int *is_dc)
{
#define ABS_DIFF(_x, _y) ((int)_x > (int)_y ? (int)_x - (int)_y : \
		(int)_y - (int)_x)
#define ABS(_x) ((int)_x > 0 ? (int)_x : -(int)_x)
	/* This should be between 1 and 3. Default is 1. */
#define DELTA_STEP 1
	/* Number of Diffs to compute. valid range is 2-4. */
#define NUM_DIFFS  3
	/* Threshold for difference of delta peaks. */
#define MAX_DIFF   2
	/* Max. number of strong bins for narrow band. */
#define BIN_COUNT_MAX 6

	/* Dynamic 20/40 mode FFT packet format related definition. */
#define NUM_FFT_BYTES_HT40      70
#define NUM_BIN_BYTES_HT40      64
#define NUM_SUBCHAN_BINS_HT40   64
#define LOWER_INDEX_BYTE_HT40   66
#define UPPER_INDEX_BYTE_HT40   69
#define LOWER_WEIGHT_BYTE_HT40  64
#define UPPER_WEIGHT_BYTE_HT40  67
#define LOWER_MAG_BYTE_HT40     65
#define UPPER_MAG_BYTE_HT40     68

	/* Static 20 mode FFT packet format related definition. */
#define NUM_FFT_BYTES_HT20      31
#define NUM_BIN_BYTES_HT20      28
#define NUM_SUBCHAN_BINS_HT20   56
#define LOWER_INDEX_BYTE_HT20   30
#define UPPER_INDEX_BYTE_HT20   30
#define LOWER_WEIGHT_BYTE_HT20  28
#define UPPER_WEIGHT_BYTE_HT20  28
#define LOWER_MAG_BYTE_HT20     29
#define UPPER_MAG_BYTE_HT20     29

	int num_fft_packets; /* number of FFT packets reported to software */
	int num_fft_bytes;
	int num_bin_bytes;
	int num_subchan_bins;
	int lower_index_byte;
	int upper_index_byte;
	int lower_weight_byte;
	int upper_weight_byte;
	int lower_mag_byte;
	int upper_mag_byte;
	int max_index_lower[DELTA_STEP + NUM_DIFFS];
	int max_index_upper[DELTA_STEP + NUM_DIFFS];
	int max_mag_lower[DELTA_STEP + NUM_DIFFS];
	int max_mag_upper[DELTA_STEP + NUM_DIFFS];
	int bin_wt_lower[DELTA_STEP + NUM_DIFFS];
	int bin_wt_upper[DELTA_STEP + NUM_DIFFS];
	int max_mag_sel[DELTA_STEP + NUM_DIFFS];
	int max_mag[DELTA_STEP + NUM_DIFFS];
	int max_index[DELTA_STEP + NUM_DIFFS];
	int max_d[] = {10, 19, 28};
	int min_d[] = {1, 2, 3};
	uint8_t *ptr; /* pointer to FFT data */
	int i;
	int fft_start;
	int chirp_found;
	int delta_peak[NUM_DIFFS];
	int j;
	int bin_count;
	int bw_mask;
	int delta_diff;
	int same_sign;
	int temp;

	if (WLAN_IS_CHAN_11N_HT40(dfs->dfs_curchan)) {
		num_fft_bytes = NUM_FFT_BYTES_HT40;
		num_bin_bytes = NUM_BIN_BYTES_HT40;
		num_subchan_bins = NUM_SUBCHAN_BINS_HT40;
		lower_index_byte = LOWER_INDEX_BYTE_HT40;
		upper_index_byte = UPPER_INDEX_BYTE_HT40;
		lower_weight_byte = LOWER_WEIGHT_BYTE_HT40;
		upper_weight_byte = UPPER_WEIGHT_BYTE_HT40;
		lower_mag_byte = LOWER_MAG_BYTE_HT40;
		upper_mag_byte = UPPER_MAG_BYTE_HT40;

		/* If we are in HT40MINUS then swap primary and extension. */
		if (WLAN_IS_CHAN_11N_HT40MINUS(dfs->dfs_curchan)) {
			temp = is_ctl;
			is_ctl = is_ext;
			is_ext = temp;
		}
	} else {
		num_fft_bytes = NUM_FFT_BYTES_HT20;
		num_bin_bytes = NUM_BIN_BYTES_HT20;
		num_subchan_bins = NUM_SUBCHAN_BINS_HT20;
		lower_index_byte = LOWER_INDEX_BYTE_HT20;
		upper_index_byte = UPPER_INDEX_BYTE_HT20;
		lower_weight_byte = LOWER_WEIGHT_BYTE_HT20;
		upper_weight_byte = UPPER_WEIGHT_BYTE_HT20;
		lower_mag_byte = LOWER_MAG_BYTE_HT20;
		upper_mag_byte = UPPER_MAG_BYTE_HT20;
	}

	ptr = (uint8_t *)buf;
	/* Sanity check for FFT buffer. */
	if (!ptr || (datalen == 0)) {
		dfs_debug(dfs, WLAN_DEBUG_DFS_BIN5_FFT,
			"FFT buffer pointer is null or size is 0");
		return 0;
	}

	num_fft_packets = (datalen - 3) / num_fft_bytes;
	if (num_fft_packets < (NUM_DIFFS + DELTA_STEP)) {
		dfs_debug(dfs, WLAN_DEBUG_DFS_BIN5_FFT,
			"datalen = %d, num_fft_packets = %d, too few packets... (exiting)",
			datalen, num_fft_packets);
		return 0;
	}

	if ((((datalen - 3) % num_fft_bytes) == 2) &&
			(datalen > num_fft_bytes)) {
		ptr += 2;
		datalen -= 2;
	}

	for (i = 0; i < (NUM_DIFFS + DELTA_STEP); i++) {
		fft_start = i * num_fft_bytes;
		bin_wt_lower[i] = ptr[fft_start + lower_weight_byte] & 0x3f;
		bin_wt_upper[i] = ptr[fft_start + upper_weight_byte] & 0x3f;
		max_index_lower[i] = ptr[fft_start + lower_index_byte] >> 2;
		max_index_upper[i] = (ptr[fft_start + upper_index_byte] >> 2) +
			num_subchan_bins;

		if (!WLAN_IS_CHAN_11N_HT40(dfs->dfs_curchan)) {
			/* For HT20 mode indices are 6 bit signed number. */
			max_index_lower[i] ^= 0x20;
			max_index_upper[i] = 0;
		}

		/*
		 * Reconstruct the maximum magnitude for each sub-channel.
		 * Also select and flag the max overall magnitude between
		 * the two sub-channels.
		 */

		max_mag_lower[i] =
		    ((ptr[fft_start + lower_index_byte] & 0x03) << 8) +
			ptr[fft_start + lower_mag_byte];
		max_mag_upper[i] =
		    ((ptr[fft_start + upper_index_byte] & 0x03) << 8) +
			ptr[fft_start + upper_mag_byte];
		bw_mask = ((bin_wt_lower[i] == 0) ? 0 : is_ctl) +
			(((bin_wt_upper[i] == 0) ? 0 : is_ext) << 1);

		/*
		 * Limit the max bin based on channel bandwidth
		 * If the upper sub-channel max index is stuck at '1',
		 * the signal is dominated * by residual DC
		 * (or carrier leak) and should be ignored.
		 */

		if (bw_mask == 1) {
			max_mag_sel[i] = 0;
			max_mag[i] = max_mag_lower[i];
			max_index[i] = max_index_lower[i];
		} else if (bw_mask == 2) {
			max_mag_sel[i] = 1;
			max_mag[i] = max_mag_upper[i];
			max_index[i] = max_index_upper[i];
		} else if (max_index_upper[i] == num_subchan_bins) {
			max_mag_sel[i] = 0;  /* Ignore DC bin. */
			max_mag[i] = max_mag_lower[i];
			max_index[i] = max_index_lower[i];
		} else {
			if (max_mag_upper[i] > max_mag_lower[i]) {
				max_mag_sel[i] = 1;
				max_mag[i] = max_mag_upper[i];
				max_index[i] = max_index_upper[i];
			} else {
				max_mag_sel[i] = 0;
				max_mag[i] = max_mag_lower[i];
				max_index[i] = max_index_lower[i];
			}
		}
		dfs_debug(dfs, WLAN_DEBUG_DFS_BIN5_FFT,
			"i=%d, max_index[i]=%d, max_index_lower[i]=%d, max_index_upper[i]=%d",
			i, max_index[i], max_index_lower[i],
			max_index_upper[i]);
	}

	chirp_found = 1;
	delta_diff = 0;
	same_sign = 1;

	/*
	 * delta_diff computation -- look for movement in peak.
	 * make sure that the chirp direction (i.e. sign) is
	 * always the same, i.e. sign of the two peaks should
	 * be same.
	 */
	for (i = 0; i < NUM_DIFFS; i++) {
		delta_peak[i] = max_index[i + DELTA_STEP] - max_index[i];
		if (i > 0) {
			delta_diff = delta_peak[i] - delta_peak[i-1];
			same_sign = !((delta_peak[i] & 0x80) ^
				(delta_peak[i-1] & 0x80));
		}
		chirp_found &=
			(ABS(delta_peak[i]) >= min_d[DELTA_STEP - 1]) &&
			(ABS(delta_peak[i]) <= max_d[DELTA_STEP - 1]) &&
			same_sign && (ABS(delta_diff) <= MAX_DIFF);
		dfs_debug(dfs, WLAN_DEBUG_DFS_BIN5_FFT,
			"i=%d, delta_peak[i]=%d, delta_diff=%d",
			i, delta_peak[i], delta_diff);
	}

	if (chirp_found) {
		dfs_debug(dfs, WLAN_DEBUG_DFS_BIN5_FFT,
			"CHIRPING_BEFORE_STRONGBIN_YES");
	} else {
		dfs_debug(dfs, WLAN_DEBUG_DFS_BIN5_FFT,
			"CHIRPING_BEFORE_STRONGBIN_NO");
	}

	/*
	 * Work around for potential hardware data corruption bug.
	 * Check for wide band signal by counting strong bins
	 * indicated by bitmap flags. This check is done if
	 * chirp_found is true. We do this as a final check to
	 * weed out corrupt FFTs bytes. This looks expensive but
	 * in most cases it will exit early.
	 */

	for (i = 0; (i < (NUM_DIFFS + DELTA_STEP)) &&
			(chirp_found == 1); i++) {
		bin_count = 0;
		/*
		 * Point to the start of the 1st byte of the selected
		 * sub-channel.
		 */
		fft_start = (i * num_fft_bytes) + (max_mag_sel[i] ?
				(num_subchan_bins >> 1) : 0);
		for (j = 0; j < (num_subchan_bins >> 1); j++) {
			/*
			 * If either bin is flagged "strong", accumulate
			 * the bin_count. It's not accurate, but good
			 * enough...
			 */
			bin_count += (ptr[fft_start + j] & 0x88) ? 1 : 0;
		}
		chirp_found &= (bin_count > BIN_COUNT_MAX) ? 0 : 1;
		dfs_debug(dfs, WLAN_DEBUG_DFS_BIN5_FFT,
			"i=%d, computed bin_count=%d",
			i, bin_count);
	}

	if (chirp_found) {
		dfs_debug(dfs, WLAN_DEBUG_DFS_BIN5_FFT |
			WLAN_DEBUG_DFS_PHYERR_SUM,
			"CHIRPING_YES");
	} else {
		dfs_debug(dfs, WLAN_DEBUG_DFS_BIN5_FFT |
			WLAN_DEBUG_DFS_PHYERR_SUM,
			"CHIRPING_NO");
	}

	return chirp_found;
#undef ABS_DIFF
#undef ABS
#undef DELTA_STEP
#undef NUM_DIFFS
#undef MAX_DIFF
#undef BIN_COUNT_MAX

#undef NUM_FFT_BYTES_HT40
#undef NUM_BIN_BYTES_HT40
#undef NUM_SUBCHAN_BINS_HT40
#undef LOWER_INDEX_BYTE_HT40
#undef UPPER_INDEX_BYTE_HT40
#undef LOWER_WEIGHT_BYTE_HT40
#undef UPPER_WEIGHT_BYTE_HT40
#undef LOWER_MAG_BYTE_HT40
#undef UPPER_MAG_BYTE_HT40

#undef NUM_FFT_BYTES_HT40
#undef NUM_BIN_BYTES_HT40
#undef NUM_SUBCHAN_BINS_HT40
#undef LOWER_INDEX_BYTE_HT40
#undef UPPER_INDEX_BYTE_HT40
#undef LOWER_WEIGHT_BYTE_HT40
#undef UPPER_WEIGHT_BYTE_HT40
#undef LOWER_MAG_BYTE_HT40
#undef UPPER_MAG_BYTE_HT40
}

int dfs_check_chirping(struct wlan_dfs *dfs,
		void *buf,
		uint16_t datalen,
		int is_ctl,
		int is_ext,
		int *slope,
		int *is_dc)
{
	if (dfs->dfs_caps.wlan_dfs_use_enhancement) {
		return dfs_check_chirping_merlin(dfs, buf, datalen, is_ctl,
			is_ext, slope, is_dc);
	} else {
		return dfs_check_chirping_sowl(dfs, buf, datalen, is_ctl,
			is_ext, slope, is_dc);
	}
}

uint8_t dfs_retain_bin5_burst_pattern(struct wlan_dfs *dfs,
		uint32_t diff_ts,
		uint8_t old_dur)
{
	/*
	 * Pulses may get split into 2 during chirping, this print
	 * is only to show that it happened, we do not handle this
	 * condition if we cannot detect the chirping.
	 */
	/* SPLIT pulses will have a time stamp difference of < 50 */
	if (diff_ts < 50) {
		dfs_debug(dfs, WLAN_DEBUG_DFS_BIN5,
			"SPLIT pulse diffTs=%u dur=%d (old_dur=%d)",
			 diff_ts,
			dfs->dfs_rinfo.dfs_last_bin5_dur, old_dur);
	}

	/*
	 * Check if this is the 2nd or 3rd pulse in the same burst,
	 * PRI will be between 1000 and 2000 us.
	 */
	if (((diff_ts >= DFS_BIN5_PRI_LOWER_LIMIT) &&
				(diff_ts <= DFS_BIN5_PRI_HIGHER_LIMIT))) {
		/*
		 * This pulse belongs to the same burst as the pulse before,
		 * so return the same random duration for it.
		 */
		dfs_debug(dfs, WLAN_DEBUG_DFS_BIN5,
			"this pulse belongs to the same burst as before, give it same dur=%d (old_dur=%d)",
			 dfs->dfs_rinfo.dfs_last_bin5_dur, old_dur);
		return dfs->dfs_rinfo.dfs_last_bin5_dur;
	}

	/* This pulse does not belong to this burst, return unchanged duration*/
	return old_dur;
}

int dfs_get_random_bin5_dur(struct wlan_dfs *dfs,
		uint64_t tstamp)
{
	uint8_t new_dur = MIN_BIN5_DUR;
	int range;

	get_random_bytes(&new_dur, sizeof(uint8_t));
	range = (MAX_BIN5_DUR - MIN_BIN5_DUR + 1);
	new_dur %= range;
	new_dur += MIN_BIN5_DUR;

	return new_dur;
}
