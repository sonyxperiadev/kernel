/* Copyright (c) 2015-2018, The Linux Foundation. All rights reserved.
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

#ifndef _CNSS_QMI_H
#define _CNSS_QMI_H

struct cnss_plat_data;

#ifdef CONFIG_CNSS2_QMI
#include "wlan_firmware_service_v01.h"

int cnss_qmi_init(struct cnss_plat_data *plat_priv);
void cnss_qmi_deinit(struct cnss_plat_data *plat_priv);
unsigned int cnss_get_qmi_timeout(void);
int cnss_wlfw_server_arrive(struct cnss_plat_data *plat_priv);
int cnss_wlfw_server_exit(struct cnss_plat_data *plat_priv);
int cnss_wlfw_respond_mem_send_sync(struct cnss_plat_data *plat_priv);
int cnss_wlfw_tgt_cap_send_sync(struct cnss_plat_data *plat_priv);
int cnss_wlfw_bdf_dnld_send_sync(struct cnss_plat_data *plat_priv);
int cnss_wlfw_m3_dnld_send_sync(struct cnss_plat_data *plat_priv);
int cnss_wlfw_wlan_mode_send_sync(struct cnss_plat_data *plat_priv,
				  enum cnss_driver_mode mode);
int cnss_wlfw_wlan_cfg_send_sync(struct cnss_plat_data *plat_priv,
				 struct cnss_wlan_enable_cfg *config,
				 const char *host_version);
int cnss_wlfw_athdiag_read_send_sync(struct cnss_plat_data *plat_priv,
				     u32 offset, u32 mem_type,
				     u32 data_len, u8 *data);
int cnss_wlfw_athdiag_write_send_sync(struct cnss_plat_data *plat_priv,
				      u32 offset, u32 mem_type,
				      u32 data_len, u8 *data);
int cnss_wlfw_ini_send_sync(struct cnss_plat_data *plat_priv,
			    u8 fw_log_mode);
#else
#define QMI_WLFW_TIMEOUT_MS		10000

static inline int cnss_qmi_init(struct cnss_plat_data *plat_priv)
{
	return 0;
}

static inline void cnss_qmi_deinit(struct cnss_plat_data *plat_priv)
{
}

static inline unsigned int cnss_get_qmi_timeout(void)
{
	return QMI_WLFW_TIMEOUT_MS;
}

static inline int cnss_wlfw_server_arrive(struct cnss_plat_data *plat_priv)
{
	return 0;
}

static inline int cnss_wlfw_server_exit(struct cnss_plat_data *plat_priv)
{
	return 0;
}

static inline
int cnss_wlfw_respond_mem_send_sync(struct cnss_plat_data *plat_priv)
{
	return 0;
}

static inline int cnss_wlfw_tgt_cap_send_sync(struct cnss_plat_data *plat_priv)
{
	return 0;
}

static inline int cnss_wlfw_bdf_dnld_send_sync(struct cnss_plat_data *plat_priv)
{
	return 0;
}

static inline int cnss_wlfw_m3_dnld_send_sync(struct cnss_plat_data *plat_priv)
{
	return 0;
}

static inline
int cnss_wlfw_wlan_mode_send_sync(struct cnss_plat_data *plat_priv,
				  enum cnss_driver_mode mode)
{
	return 0;
}

static inline
int cnss_wlfw_wlan_cfg_send_sync(struct cnss_plat_data *plat_priv,
				 struct cnss_wlan_enable_cfg *config,
				 const char *host_version)
{
	return 0;
}

static inline
int cnss_wlfw_athdiag_read_send_sync(struct cnss_plat_data *plat_priv,
				     u32 offset, u32 mem_type,
				     u32 data_len, u8 *data)
{
	return 0;
}

static inline
int cnss_wlfw_athdiag_write_send_sync(struct cnss_plat_data *plat_priv,
				      u32 offset, u32 mem_type,
				      u32 data_len, u8 *data)
{
	return 0;
}

static inline
int cnss_wlfw_ini_send_sync(struct cnss_plat_data *plat_priv,
			    u8 fw_log_mode)
{
	return 0;
}
#endif /* CONFIG_CNSS2_QMI */

#endif /* _CNSS_QMI_H */
