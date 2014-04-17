/*
 *  include/linux/mmc/bcm_sdiowl.h
 *
 *  Copyright (C) 2011-2014 Broadcom, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 */

#ifndef MMC_BCM_SDIOWL_H
#define MMC_BCM_SDIOWL_H

extern void bcm_sdiowl_reset_b(int onoff);
extern int bcm_sdiowl_rescan(void);

extern int bcm_sdiowl_init(void);
extern void bcm_sdiowl_term(void);

#endif

