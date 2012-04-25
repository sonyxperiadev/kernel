/*****************************************************************************
* Copyright 2010 - 2011 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#ifndef _SDHCI_PLTFM_H_
#define _SDHCI_PLTFM_H_

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mmc/sdhci.h>
#include <mach/sdio_platform.h>

int sdio_dev_is_initialized(enum sdio_devtype devtype);
struct mmc_card *sdio_get_mmc_card(enum sdio_devtype devtype);
struct sdio_wifi_gpio_cfg *sdio_get_wifi_gpio(enum sdio_devtype devtype);
int sdio_card_emulate(enum sdio_devtype devtype, int insert);
int sdio_stop_clk(enum sdio_devtype devtype, int insert);
int sdio_reset_comm(struct mmc_card *card);

#endif /* _SDHCI_PLTFM_H_ */
