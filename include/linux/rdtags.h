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

#ifndef __RAMDUMP_TAGS_H_
#define __RAMDUMP_TAGS_H_

#include <linux/types.h>

int rdtags_remove_tag(const char *name);
int rdtags_add_tag(const char *name, const unsigned char *data,
		   const uint32_t size);
unsigned char *rdtags_get_tag_data(const char *name, uint32_t *size);
void rdtags_clear_tags(void);

#define rdtags_add_tag_string(x, y) rdtags_add_tag(x, y, sizeof(y))
#define rdtags_add_tag_type(x, y) \
	rdtags_add_tag(x, (unsigned char *)&y, sizeof(y))

/**
 * struct rdtags_platform_data - Ramdump Tags platform data
 * @platform_init: Function that will be called after rdtags is loaded
 */
struct rdtags_platform_data {
	int (*platform_init)(void);
};
#endif
