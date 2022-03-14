/*
 *  lowmemorykiller_oom interface
 *
 *  Author: Peter Enderborg <peter.enderborg@sonymobile.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
/*
 * Copyright (C) 2018 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/oom.h>
#include "lowmemorykiller.h"
#include "lowmemorykiller_stats.h"
#include "lowmemorykiller_tasks.h"

int __init lowmemorykiller_register_oom_notifier(void);
