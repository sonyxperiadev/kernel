/*****************************************************************************
*  Copyright 2001 - 2011 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/
#ifndef __BCM_HSOTGCTRL_H
#define __BCM_HSOTGCTRL_H

#define BCM_HSOTGCTRL_WAKEUP_PROCESSING_DELAY 5

struct bcm_hsotgctrl_platform_data {
	void __iomem *hsotgctrl_virtual_mem_base;
	void __iomem *chipreg_virtual_mem_base;
	int irq;
	const unsigned char *usb_ahb_clk_name;
	const unsigned char *mdio_mstr_clk_name;
};

typedef void (*send_core_event_cb_t)(void *arg);

int bcm_hsotgctrl_phy_set_vbus_stat(bool on);
int bcm_hsotgctrl_phy_set_non_driving(bool on);
int bcm_hsotgctrl_set_phy_off(bool on);
int bcm_hsotgctrl_phy_set_id_stat(bool floating);
int bcm_hsotgctrl_phy_init(bool id_device);
int bcm_hsotgctrl_phy_deinit(void);
int bcm_hsotgctrl_en_clock(bool on);
int bcm_hsotgctrl_bc_reset(void);
int bcm_hsotgctrl_bc_enable_sw_ovwr(void);
int bcm_hsotgctrl_bc_status(unsigned long *status);
int bcm_hsotgctrl_bc_vdp_src_off(void);
int bcm_hsotgctrl_reset_clk_domain(void);
int bcm_hsotgctrl_set_phy_iso(bool on);
int bcm_hsotgctrl_set_bc_iso(bool on);
int bcm_hsotgctrl_set_soft_ldo_pwrdn(bool on);
int bcm_hsotgctrl_set_aldo_pdn(bool on);
int bcm_hsotgctrl_set_phy_resetb(bool on);
int bcm_hsotgctrl_set_phy_pll_resetb(bool on);
int bcm_hsotgctrl_set_phy_clk_request(bool on);
int bcm_hsotgctrl_phy_mdio_init(void);
int bcm_hsotgctrl_set_ldo_suspend_mask(void);
int bcm_hsotgctrl_phy_wakeup_condition(bool set);
int bcm_hsotgctrl_handle_bus_suspend(void);
int bcm_hsotgctrl_register_wakeup_cb(send_core_event_cb_t wakeup_core_cb, void* arg);
int bcm_hsotgctrl_unregister_wakeup_cb(void);
int bcm_hsotgctrl_get_clk_count(void);
int bcm_hsotgctrl_is_suspend_allowed(bool *suspend_allowed);
void bcm_hsotgctrl_wakeup_core(void);

#endif /*__BCM_HSOTGCTRL_H*/
