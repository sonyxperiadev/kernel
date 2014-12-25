/*
 * Generic GPIO card-detect helper header
 *
 * Copyright (C) 2011, Guennadi Liakhovetski <g.liakhovetski@gmx.de>
 * Copyright (c) 2013 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef MMC_SLOT_GPIO_H
#define MMC_SLOT_GPIO_H

struct mmc_host;

int mmc_gpio_get_ro(struct mmc_host *host);
int mmc_gpio_request_ro(struct mmc_host *host, unsigned int gpio);
void mmc_gpio_free_ro(struct mmc_host *host);

int mmc_gpio_get_cd(struct mmc_host *host);
int mmc_gpio_request_cd(struct mmc_host *host, unsigned int gpio);
void mmc_gpio_free_cd(struct mmc_host *host);
int mmc_gpio_get_status(struct mmc_host *host);

#ifdef CONFIG_MMC_BLOCK_DEFERRED_RESUME
void mmc_gpio_prepare_suspend(struct mmc_host *host, bool pending_detect);
bool mmc_gpio_is_pending_detect(struct mmc_host *host);
#endif
#endif
