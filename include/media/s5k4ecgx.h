/* include/media/s5k4ecgx.h
 *
 * Copyright (C) 2010, SAMSUNG ELECTRONICS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#define S5K4ECGX_DRIVER_NAME	"S5K4ECGX"

struct s5k4ecgx_platform_data {
	unsigned int default_width;
	unsigned int default_height;
	unsigned int pixelformat;
	int freq;	/* MCLK in Hz */

	int (*flash_onoff)(int);
	int (*af_assist_onoff)(int);
	int (*torch_onoff)(int);
};
