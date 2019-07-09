/*
 * Linux mainline API clocks implementation for MSM8998
 * Graphics Processing Unit Clock Controller (GPUCC) driver
 * Copyright (C) 2018, AngeloGioacchino Del Regno <kholk11@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define GPU_PLL0_PLL		0
#define GPU_PLL0_PLL_OUT_EVEN	1
#define GPU_PLL0_PLL_OUT_ODD	2
#define GFX3D_CLK_SRC		3
#define RBBMTIMER_CLK_SRC	4
#define GFX3D_ISENSE_CLK_SRC	5
#define GPUCC_RBBMTIMER_CLK	6
#define GPUCC_GFX3D_ISENSE_CLK	7
#define GPUCC_GFX3D_CLK		8


/* EARLY */
#define GPUCC_XO		0
#define RBCPR_CLK_SRC		1
#define GPUCC_RBCPR_CLK		2
	
