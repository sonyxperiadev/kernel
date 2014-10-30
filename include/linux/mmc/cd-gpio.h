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

#ifndef MMC_CD_GPIO_H
#define MMC_CD_GPIO_H

struct mmc_host;
int mmc_cd_gpio_request(struct mmc_host *host, unsigned int gpio);
void mmc_cd_gpio_free(struct mmc_host *host);
int mmc_cd_get_status(struct mmc_host *host);

#ifdef CONFIG_MMC_BLOCK_DEFERRED_RESUME
void mmc_cd_prepare_suspend(struct mmc_host *host, bool pending_detect);
bool mmc_cd_is_pending_detect(struct mmc_host *host);
#endif
#endif
