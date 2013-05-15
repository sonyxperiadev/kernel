/*
 * Copyright (c) 2012 Sony Mobile Communications AB.
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
#include "board-rdtags.h"

#define RDTAGS_TAG_MAGIC 0xBADFAD01

#define NBR_OF_ELEMENTS 3

struct rdtags_tag_const {
	const char const key[16];
	const char const value[32];
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

static int board_rdtags_init(void)
{
	int nbr_tags = 0;
	int i = 0;
	int nbrOfElements = sizeof(rdtags_build_info.tag_array) /
						sizeof(rdtags_build_info.tag_array[0]);

	for (i = 0; i < nbrOfElements; i++) {
		if (!rdtags_add_tag_string(
				rdtags_build_info.tag_array[i].key,
				rdtags_build_info.tag_array[i].value)) {
			nbr_tags++;
		}
	}

	return nbr_tags;
}

struct rdtags_platform_data rdtags_platdata = {
	.platform_init	= board_rdtags_init,
};
