/*
 * Copyright (c) 2012-2013 Sony Mobile Communications AB.
 *
 * Author: Nilsson, Stefan 2 <stefan2.nilsson@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/rdtags.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <asm/setup.h>
#include "board-rdtags.h"
#ifdef CONFIG_KALLSYMS_ALL
#include <linux/kallsyms.h>
#include <asm/memory.h>
#endif

#define RDTAGS_TAG_MAGIC 0xBADFAD01

#define NBR_OF_ELEMENTS 3

struct rdtags_tag_const {
	const char const key[16];
	const char const value[64];
};

struct rdtags_build_tags {
	const unsigned int magic;
	const unsigned int size;
	const struct rdtags_tag_const tag_array[NBR_OF_ELEMENTS];
};

static const struct rdtags_build_tags rdtags_build_info = {
	RDTAGS_TAG_MAGIC,
	sizeof(rdtags_build_info),
	{
		{
			"build_product",
			INFO_PRODUCT
		},
		{
			"build_variant",
			INFO_VARIANT
		},
		{
			"build_id",
			INFO_BUILDID
		}
	}
};


/*     change NR_RDTAGS incase addding new tags
       Current Tags __log_buf,log_end,log_buf_len,xtime
       [0]     struct clocksource *clock
       [4]     u32 mult
       [8]     int shift
       [16]    cycle_t cycle_interval
       [24]    u64     xtime_interval
       [32]    s64     xtime_remainder
       [40]    u32     raw_interval
       [48]    u64     xtime_nsec
       [56]    s64     ntp_error
       [64]    int     ntp_error_shift
       [68]    struct timespec xtime
 */

/*Add rdtag for mem descriptors*/
static int rdtags_get_meminfo(void)
{
	int ret = 0;
	char tmp_data[11];

	memset(tmp_data, 0, sizeof(tmp_data));
	snprintf(tmp_data, sizeof(tmp_data), "0x%08x",
		(unsigned int)virt_to_phys(&meminfo));

	ret = rdtags_add_tag_string("meminfo", tmp_data);
	if (ret) {
		pr_err("Could not create RDTAG meminfo: %d",
						ret);
		return ret;
	}

	return 0;
}

#ifdef CONFIG_KALLSYMS_ALL
#define XTIME_OFFSET 68
static int rdtags_get_xtime_info(void)
{
	unsigned long timekeeper_addr, xtime_addr;
	char *timekeeper_sym = "timekeeper";
	char tmp_data[11];
	timekeeper_addr = kallsyms_lookup_name(timekeeper_sym);
	if (!timekeeper_addr) {
		pr_err("Unable to lookup symbol %s\n",
		       timekeeper_sym);
		return -EINVAL;
	}
	xtime_addr = (timekeeper_addr + XTIME_OFFSET);
	snprintf(tmp_data, sizeof(tmp_data), "0x%08x",
		 (unsigned int)virt_to_phys((void *)xtime_addr));
	rdtags_add_tag_string("xtime", tmp_data);
	return 0;
}

# define NR_LOGBUF_TAGS 3
static int rdtags_get_logbuf_info(void)
{
	unsigned long rdtags_logbuf_addr[NR_LOGBUF_TAGS];
	int i;
	char *rdtags_logbuf_name[] = {
		"__log_buf",
		"log_end",
		"log_buf_len",
	};
	char tmp_data[11];
	/* lookup the virtual address of __log_buf,log_end,log_buf_len,xtime
	   from kallsyms and tag them.
	 */
	for (i = 0; i < NR_LOGBUF_TAGS; i++) {
		rdtags_logbuf_addr[i] =
			kallsyms_lookup_name(rdtags_logbuf_name[i]);
		if (!rdtags_logbuf_addr[i]) {
			pr_err("Unable to lookup symbol %s\n",
			       rdtags_logbuf_name[i]);
			return -EINVAL;
		}
		snprintf(tmp_data, sizeof(tmp_data), "0x%08x",
			 (unsigned int)virt_to_phys(
				    (void *)rdtags_logbuf_addr[i]));
		rdtags_add_tag_string(rdtags_logbuf_name[i], tmp_data);
	}
	return 0;
}
#endif

static int board_rdtags_init(void)
{
	int nbr_tags = 0;
	int i = 0;
	int nr_elements = sizeof(rdtags_build_info.tag_array) /
		sizeof(rdtags_build_info.tag_array[0]);

	for (i = 0; i < nr_elements; i++) {
		if (!rdtags_add_tag_string(
				   rdtags_build_info.tag_array[i].key,
				   rdtags_build_info.tag_array[i].value)) {
			nbr_tags++;
		}
	}

#ifdef CONFIG_KALLSYMS_ALL
	/* log_buf and xtime physical addresses are Taged here*/
	rdtags_get_logbuf_info();
	rdtags_get_xtime_info();
	nbr_tags += NR_LOGBUF_TAGS + 1;
#endif
	if (!rdtags_get_meminfo())
		nbr_tags++;

	return nbr_tags;
}

struct rdtags_platform_data rdtags_platdata = {
	.platform_init	= board_rdtags_init,
};
