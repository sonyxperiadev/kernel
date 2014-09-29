/* Copyright (c) 2013, The Linux Foundation. All rights reserved.
 * Copyright (c) 2013 Sony Mobile Communications AB.
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

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <asm/sections.h>
#include <asm/memory.h>

#include <mach/scm.h>

#define ERR_FATAL_ENABLE 0x0
#define ERR_FATAL_DISABLE 0x1
#define ERR_FATAL_READ 0x2
#define XPU_ERR_FATAL 0xe

#define XPU_PROTECT_AREA 0x3

static int __init xpu_err_fatal_init(void)
{
	int ret, response;
	struct {
		unsigned int config;
		unsigned int spare;
	} cmd;
	cmd.config = ERR_FATAL_ENABLE;
	cmd.spare = 0;

	ret = scm_call(SCM_SVC_MP, XPU_ERR_FATAL, &cmd, sizeof(cmd), &response,
			sizeof(response));

	if (ret != 0)
		pr_warn("Failed to set XPU violations as fatal errors: %d\n",
			ret);
	else
		pr_info("Configuring XPU violations to be fatal errors\n");

	return ret;
}
early_initcall(xpu_err_fatal_init);

static int __init xpu_protect_init(void)
{
	int ret, response;
	struct {
		unsigned int start;
		unsigned int size;
	} cmd;
	cmd.start = __pa(_stext);
	cmd.size = PAGE_ALIGN((u32)_etext - (u32)_stext);

	ret = scm_call(SCM_SVC_OEM, XPU_PROTECT_AREA, &cmd, sizeof(cmd),
			&response, sizeof(response));

	if (ret != 0)
		pr_warn("Failed to XPU protect text area 0x%08x--0x%08x: %d\n",
			cmd.start, cmd.start + cmd.size - 1, ret);
	else
		pr_info("Configured XPU protection in region 0x%08x--0x%08x\n",
			cmd.start, cmd.start + cmd.size - 1);

	return ret;
}
late_initcall(xpu_protect_init);
