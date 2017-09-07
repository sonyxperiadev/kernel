/*
 * Copyright (C) 2014 Sony Mobile Communications Inc.
 *
 * Author: Daisuke Niwa daisuke.x.niwa@sonymobile.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <bcmutils.h>
#include <dngl_stats.h>
#include <dhd.h>
#include <dhd_dbg.h>
#include <dhd_somc_custom.h>

/* miscTA items */
typedef enum somc_nv_item {
	SOMC_TA_TXPWR_2_4G = 0,
	SOMC_TA_TXPWR_5G_LOW,
	SOMC_TA_TXPWR_5G_MID,
	SOMC_TA_TXPWR_5G_HIGH,
	SOMC_TA_TXPWR_CO1_2_4G,
	SOMC_TA_TXPWR_CO1_5G_LOW,
	SOMC_TA_TXPWR_CO1_5G_MID,
	SOMC_TA_TXPWR_CO1_5G_HIGH,
} somc_nv_item_t;

/* Paths to miscTA import files */
static const char *somc_ta_paths[] = {
	"/data/etc/wlan_txpower_2_4g",
	"/data/etc/wlan_txpower_5g_low",
	"/data/etc/wlan_txpower_5g_mid",
	"/data/etc/wlan_txpower_5g_high",
	"/data/etc/wlan_txpower_co1_2_4g",
	"/data/etc/wlan_txpower_co1_5g_low",
	"/data/etc/wlan_txpower_co1_5g_mid",
	"/data/etc/wlan_txpower_co1_5g_high",
};

#define SOMC_NV_IS_5G(i) (i % 4 == 0 ? 0 : 1)
#define SOMC_NV_GET_CHAIN(i) (i < SOMC_TA_TXPWR_CO1_2_4G ? 0 : 1)

#define SOMC_MAX_TABUF_SIZE 128
#define SOMC_TXPWR_BUF_SIZE 7

/* Keys used in calibration file for tx power */
#define SOMC_CKEY_TXPWR_2_4G_11B     "cckbw202gpo"
#define SOMC_CKEY_TXPWR_2_4G_11A     "dot11agofdmhrbw202gpo"
#define SOMC_CKEY_TXPWR_2_4G_11A_2   "ofdmlrbw202gpo"
#define SOMC_CKEY_TXPWR_2_4G_11N     "mcsbw202gpo"
#define SOMC_CKEY_TXPWR_5G_LOW_11A   "mcsbw205glpo"
#define SOMC_CKEY_TXPWR_5G_LOW_11N   "mcsbw405glpo"
#define SOMC_CKEY_TXPWR_5G_LOW_11AC  "mcsbw805glpo"
#define SOMC_CKEY_TXPWR_5G_MID_11A   "mcsbw205gmpo"
#define SOMC_CKEY_TXPWR_5G_MID_11N   "mcsbw405gmpo"
#define SOMC_CKEY_TXPWR_5G_MID_11AC  "mcsbw805gmpo"
#define SOMC_CKEY_TXPWR_5G_HIGH_11A  "mcsbw205ghpo"
#define SOMC_CKEY_TXPWR_5G_HIGH_11N  "mcsbw405ghpo"
#define SOMC_CKEY_TXPWR_5G_HIGH_11AC "mcsbw805ghpo"

#define SOMC_TXPWR_MAX 127
#define SOMC_TXPWR_5G 0x20

typedef struct {
	char *key;  /* Key for tx power (see the definitions above) */
	int len;    /* Length of a unit of PPR (not incl. "0x") */
	int offset; /* Offset to PPR */
} somc_ppr_item_t;

/* List of ppr item handled by somc_txpower_calibrate() */
static const somc_ppr_item_t somc_ppr_items[] = {
	{ SOMC_CKEY_TXPWR_2_4G_11B,     4, 0},
	{ SOMC_CKEY_TXPWR_2_4G_11A,     4, 0},
	{ SOMC_CKEY_TXPWR_2_4G_11A_2,   4, 2},
	{ SOMC_CKEY_TXPWR_2_4G_11N,     8, 0},
	{ SOMC_CKEY_TXPWR_5G_LOW_11A,   8, 0},
	{ SOMC_CKEY_TXPWR_5G_LOW_11N,   8, 0},
	{ SOMC_CKEY_TXPWR_5G_LOW_11AC,  8, 0},
	{ SOMC_CKEY_TXPWR_5G_MID_11A,   8, 0},
	{ SOMC_CKEY_TXPWR_5G_MID_11N,   8, 0},
	{ SOMC_CKEY_TXPWR_5G_MID_11AC,  8, 0},
	{ SOMC_CKEY_TXPWR_5G_HIGH_11A,  8, 0},
	{ SOMC_CKEY_TXPWR_5G_HIGH_11N,  8, 0},
	{ SOMC_CKEY_TXPWR_5G_HIGH_11AC, 8, 0},
};

/* {2.4GHz: {chain0, chain1}, 5GHz: {chain0, chain1}} */
static int somc_txpower_min_deltas[2][2] = {{0, 0},{0, 0}};

static int
somc_txpower_get_min_delta(int band5g, int chain)
{
	int min_delta = somc_txpower_min_deltas[band5g][chain];
	return (min_delta == INT_MAX) ? 0 : min_delta;
}

static void
somc_txpower_update_min_delta(const int *delta, int num, int band5g, int chain)
{
	int *min_delta = &somc_txpower_min_deltas[band5g][chain];
	while (num > 0) {
		num--;
		if (band5g && (num == 0)) /* ignore 11b delta value for 5GHz */
			continue;
		if (!band5g && (num > 2)) /* ignore 11n-40/11ac delta values for 2.4GHz */
			continue;
		*min_delta = MIN(*min_delta, delta[num]);
	}
}

static int
somc_read_file(const char *path, unsigned char *buf, int buf_len)
{
	int ret = -1;
	int len;
	struct file *fp = NULL;

	if (!path || !buf)
		goto err;

	fp = filp_open(path, O_RDONLY, 0);
	if (IS_ERR(fp)) {
		DHD_ERROR(("%s: file open error: %s\n", __FUNCTION__, path));
		if (PTR_ERR(fp) == -ENOENT) {
			DHD_ERROR(("%s: file does not exist: %s\n", __FUNCTION__, path));
			ret = -2;
		}
		fp = NULL;
		goto err;
	}

	len = dhd_os_get_image_block(buf, buf_len, (void *)fp);
	if (len <= 0 || buf_len <= len) {
		DHD_ERROR(("%s: file read error: %s\n", __FUNCTION__, path));
		goto err;
	}
	buf[len] = '\0';

	ret = 0;
err:
	if (fp)
		filp_close(fp, NULL);
	return ret;
}

static int
somc_read_ta(somc_nv_item_t item, unsigned char *buf, int buf_len)
{
	char ta_buf[SOMC_MAX_TABUF_SIZE] = {0};
	int *d, ret;

	ret = somc_read_file(somc_ta_paths[item], ta_buf, sizeof(ta_buf));
	if (ret != 0)
		return ret;

	switch (item) {
	case SOMC_TA_TXPWR_2_4G:
	case SOMC_TA_TXPWR_5G_LOW:
	case SOMC_TA_TXPWR_5G_MID:
	case SOMC_TA_TXPWR_5G_HIGH:
	case SOMC_TA_TXPWR_CO1_2_4G:
	case SOMC_TA_TXPWR_CO1_5G_LOW:
	case SOMC_TA_TXPWR_CO1_5G_MID:
	case SOMC_TA_TXPWR_CO1_5G_HIGH:
		if (buf_len < SOMC_TXPWR_BUF_SIZE)
			return -1;
		d = (int *)buf;
		if (sscanf(ta_buf, "%d:%d:%d:%d:%d:%d:%d",
			   &d[0], &d[1], &d[2], &d[3], &d[4], &d[5], &d[6]) != 7) {
			DHD_ERROR(("%s: tx power parse error: %s\n",
				   __FUNCTION__, somc_ta_paths[item]));
			return -1;
		}

		printk("%s: tx power in miscTA(%s),\n 11b:11a:11n(20MHz):11n(40MHz):"
			"11ac(20MHz):11ac(40MHz):11ac(80MHz)=,\n  %d:%d:%d:%d:%d:%d:%d\n",
			__FUNCTION__, somc_ta_paths[item],
			d[0], d[1], d[2], d[3], d[4], d[5], d[6]);
		break;
	default:
		return -1;
	}

	return 0;
}

static int
somc_txpower_apply_delta(const unsigned char *key, int len, int offset,
			int delta, unsigned char *nvram, int nvram_len)
{
	unsigned char *end = nvram + nvram_len - 1;
	unsigned char *k, *v, *t;
	unsigned int power_h, power_l;
	int i, j, v_len = len * 2 + 5; /* e.g. "0x5555,0x1111" (= 4 * 2 + 5) */
	const unsigned char *fmt;

	if (!key || !nvram || offset >= len || (len != 4 && len != 8))
		return -1;

	fmt = (len == 4) ? "0x%04x,0x%04x" : "0x%08x,0x%08x";

	/* look up key in nvram */
	if ((k = strnstr(nvram, key, nvram_len)) == NULL) {
		DHD_ERROR(("%s: key not found: %s\n", __FUNCTION__, key));
		return -1;
	}

	/* extract value */
	v = k + strlen(key);
	if (v > end || *v != '=') {
		DHD_ERROR(("%s: value parse error: %s\n", __FUNCTION__, key));
		return -1;
	}
	v += 1;

	if (v > end || (t = strnchr(v, end - v + 1, '\n')) == NULL ||
	    t - v != v_len) {
		DHD_ERROR(("%s: value parse error: %s\n", __FUNCTION__, key));
		return -1;
	}

	/* extract each values */
	if (sscanf(v, fmt, &power_l, &power_h) != 2) {
		DHD_ERROR(("%s: power offset values parse error: %s\n", __FUNCTION__, key));
		return -1;
	}

	/* convert unit since nvram uses 1/2dB step, miscTA uses 1/100dB step */
	if (delta < 0 && delta % 50 != 0)
		delta = delta / 50 - 1;
	else
		delta = delta / 50;

	for (i = 0, j = len - offset; i < j; i++) {
		int val, sft = i * 4;
		/* combine low and high 4 bits into a value(8bit) */
		val = ((power_l >> sft) & 0xf) | (((power_h >> sft) & 0xf) << 4);
		/* apply delta */
		val -= delta;
		val = (val > 0xff) ? 0xff : (val < 0x00) ? 0x00 : val;
		/* separate power to low and high again */
		power_l &= ~(0xf << sft);
		power_l |= (val & 0x0f) << sft;
		power_h &= ~(0xf << sft);
		power_h |= ((val & 0xf0) >> 4) << sft;
	}

	snprintf(v, v_len + 1, fmt, power_l, power_h);
	v[v_len] = '\n';

	return 0;
}

int somc_txpower_calibrate(char *nvram, int nvram_len)
{
	int v[4][SOMC_TXPWR_BUF_SIZE]; /* tx power offset(s) */
	int i, j, ta_num, delta[sizeof(somc_ppr_items) / sizeof(somc_ppr_items[0])];

#ifdef SOMC_MIMO
	ta_num = SOMC_TA_TXPWR_CO1_5G_HIGH + 1;
#else
	ta_num = SOMC_TA_TXPWR_5G_HIGH + 1;
#endif
	/* initialize minimum delta (used for tx power trim) */
	for (i = 0; i < 4; i++) {
		for (j = 0; j < SOMC_TXPWR_BUF_SIZE; j++) {
			v[i][j] = INT_MAX;
		}
	}

	/* initialize minimum delta (used for tx power back-off) */
	somc_txpower_min_deltas[0][0 /* chain0 */] = INT_MAX;
	somc_txpower_min_deltas[1][0 /* chain0 */] = INT_MAX;
	somc_txpower_min_deltas[0][1 /* chain1 */] = INT_MAX;
	somc_txpower_min_deltas[1][1 /* chain1 */] = INT_MAX;

	for (i = 0; i < ta_num; i++) {
		int ret;
		int tv[SOMC_TXPWR_BUF_SIZE] = {0};

		/* no need to update delta if miscTA doesn't exist(ret == 2) */
		ret = somc_read_ta(i, (char *)tv, sizeof(tv));
		if (ret == 0) {
			somc_txpower_update_min_delta(tv, SOMC_TXPWR_BUF_SIZE,
				SOMC_NV_IS_5G(i), SOMC_NV_GET_CHAIN(i));
			/* select minimum delta between chain0 and chain1 */
			for (j = 0; j < SOMC_TXPWR_BUF_SIZE; j++)
				v[i % 4][j] = MIN(v[i % 4][j], tv[j]);
		} else if (ret != -2)
			return BCME_OK;
	}

	/* construct delta data structure for 2.4GHz
	 *  TA value "v[0]" consists of the following format.
	 *   {11b}:{11a}:{11n-20}
	 */
	delta[0] = v[0][0]; /* {11b} */
	delta[1] = v[0][1]; /* {11a} */
	delta[2] = v[0][1]; /* {11a} */
	delta[3] = v[0][2]; /* {11n-20} */

	/* construct delta data structure for 5GHz(low/mid/high)
	 *  TA values "v[1],v[2],v[3]" consist of the following format.
	 *   {11b}:{11a}:{11n-20}:{11n-40}:{11ac-20}:{11ac-40}:{11ac-80}
	 */
	for (i = 1; i < 4; i++) {
		/* minimum delta between {11a}, {11n-20}, {11ac-20} for 20MHz */
		delta[i * 3 + 1] = MIN(MIN(v[i][1], v[i][2]), v[i][4]);
		/* minimum delta between {11n-40}, {11ac-40} for 40MHz */
		delta[i * 3 + 2] = MIN(v[i][3], v[i][5]);
		/* {11ac-80} for 80MHz */
		delta[i * 3 + 3] = v[i][6];
	}

	/* Apply delta (tx power trim) */
	for (i = 0; i < sizeof(somc_ppr_items) / sizeof(somc_ppr_items[0]); i++) {
		if (delta[i] == INT_MAX)
			continue;
		if (somc_txpower_apply_delta(somc_ppr_items[i].key, somc_ppr_items[i].len,
		    somc_ppr_items[i].offset, delta[i], nvram, nvram_len) != 0)
			return BCME_ERROR;
	}

	return BCME_OK;
}

int somc_update_qtxpower(char *buf, char band, int chain)
{
	int in_qdbm, power;
	int delta = somc_txpower_get_min_delta((band & SOMC_TXPWR_5G) != 0, chain);

	in_qdbm = *buf;

	if (in_qdbm < 0 || SOMC_TXPWR_MAX < in_qdbm)
		return -1;

	/* convert unit for calculation since 'delta' uses 1/100dB step */
	power = in_qdbm + delta / (100 / 4);
	if (power > 0) {
		power = (power > SOMC_TXPWR_MAX) ? SOMC_TXPWR_MAX : power;
	} else {
		power = 0;
	}
	*buf = (char)power;

	printk("%s: Set max tx power: %d qdBm (delta=%d)\n",
		  __FUNCTION__, power, delta);

	return 0;
}
