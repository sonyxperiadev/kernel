/* arch/arm/mach-msm/include/mach/mipi_dsi_renesas.h
 *
 * Copyright (C) 2011 Sony Ericsson Mobile Communications AB.
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */


#ifndef __ARCH_ARM_MACH_MSM_MIPI_DSI_RENESAS_H
#define __ARCH_ARM_MACH_MSM_MIPI_DSI_RENESAS_H

#include <linux/types.h>

#define R63306_DEVICE_NAME "mipi_renesas_r63306"

#ifdef CONFIG_FB_MSM_MIPI_R63306_PANEL_TMD_MDV20
extern const struct panel_id tmd_video_wxga_mdv20_panel_id_00;
extern const struct panel_id tmd_video_wxga_mdv20_panel_id_01;
extern const struct panel_id tmd_video_wxga_mdv20_panel_id_02;
extern const struct panel_id tmd_video_wxga_mdv20_panel_id;
#endif /* CONFIG_FB_MSM_MIPI_R63306_PANEL_TMD_MDV20 */
#ifdef CONFIG_FB_MSM_MIPI_R63306_PANEL_TMD_MDV22
extern const struct panel_id tmd_video_wxga_mdv22_panel_id_00;
extern const struct panel_id tmd_video_wxga_mdv22_panel_id_01;
extern const struct panel_id tmd_video_wxga_mdv22_panel_id_02;
extern const struct panel_id tmd_video_wxga_mdv22_panel_id;
#endif /* CONFIG_FB_MSM_MIPI_R63306_PANEL_TMD_MDV22 */
#ifdef CONFIG_FB_MSM_MIPI_R63306_PANEL_TMD_MDW30
extern const struct panel_id tmd_video_wxga_mdw30_panel_id;
#endif /* CONFIG_FB_MSM_MIPI_R63306_PANEL_TMD_MDW30 */
#ifdef CONFIG_FB_MSM_MIPI_R63306_PANEL_SHARP_LS043K3SX01F
extern const struct panel_id sharp_ls043k3sx01_panel_id;
#endif /* CONFIG_FB_MSM_MIPI_R63306_PANEL_SHARP_LS043K3SX01F */
#ifdef CONFIG_FB_MSM_MIPI_R63306_PANEL_TMD_MDW30F
extern const struct panel_id tmd_mdw30_panel_id;
#endif /* CONFIG_FB_MSM_MIPI_R63306_PANEL_TMD_MDW30F */

#endif
