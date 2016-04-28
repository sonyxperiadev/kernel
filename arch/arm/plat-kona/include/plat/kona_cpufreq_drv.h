/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
*       @file   arch/arm/mach-bcm215xx/kona_cpufreq_drv.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#ifndef KONA_CPUFREQ_DRV_H
#define KONA_CPUFREQ_DRV_H

#include <linux/plist.h>

struct kona_freq_tbl {
	u32 cpu_freq;		/* in MHz */
	int opp;		/* Operating point eg: ECONOMY, NORMAL, TURBO */
	long max_temp;		/* max temperature supported in celcius */
};

/* Helper to initialize array of above structures */
#define FTBL_INIT(freq, __opp, temp)  \
{                              \
	.cpu_freq    = freq,   \
	.opp = __opp,   \
	.max_temp = temp, \
}

enum {
	KONA_CPUFREQ_UPDATE_LPJ = 1,
	KONA_CPUFREQ_TMON = (1 << 1),
};

#define DEFAULT_LIMIT   (-1)
#define MIN_LIMIT       (0)
#define CURRENT_FREQ    (1)
#define MAX_LIMIT       (2)
#define TEMP_DONT_CARE  0xFFFFFFFF
/* Platform data for Kona cpufreq driver */
struct kona_cpufreq_drv_pdata {
	/* Number of cpus */
	u32 flags;
	/* Table of cpu frequencies and voltages supported for a cpu */
	struct kona_freq_tbl *freq_tbl;
	/* Number of entries in the DVFS table */
	int num_freqs;
	/* PROC CCU PI id */
	int pi_id;
	/* CPU Frequency transition latency in ns */
	u32 latency;

	/*Init callback - can be NULL */
	void (*cpufreq_init) (void);
};

struct cpufreq_lmt_node {
	char *name;
	struct plist_node node;
	int lmt;
	int lmt_typ;
	bool valid;
};

int cpufreq_add_lmt_req(struct cpufreq_lmt_node *lmt_node,
	char *client_name, int lmt, int lmt_typ);
int cpufreq_del_lmt_req(struct cpufreq_lmt_node *lmt_node);
int cpufreq_update_lmt_req(struct cpufreq_lmt_node *lmt_node,
	int lmt);

int get_cpufreq_limit(unsigned int *val, int limit_type);
int set_cpufreq_limit(unsigned int val, int limit_type);
u32 get_cpu_freq_from_opp(int opp);

#endif /* BCM_CPUFREQ_DRV_H */
