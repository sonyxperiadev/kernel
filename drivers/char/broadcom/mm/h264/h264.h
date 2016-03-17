/*******************************************************************************
  Copyright 2010 Broadcom Corporation.  All rights reserved.

  Unless you and Broadcom execute a separate written software license agreement
  governing use of this software, this software is licensed to you under the
  terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/
#ifndef _H264_H_
#define _H264_H_

#if defined(CONFIG_MM_SECURE_DRIVER)

#define H264_SUBDEV_COUNT 16

#else /* CONFIG_MM_SECURE_DRIVER */

#define H264_SUBDEV_COUNT 4

#endif /* CONFIG_MM_SECURE_DRIVER */

/*CME*/
void cme_update_virt(void *virt);
int	cme_init(MM_CORE_HW_IFC *core_param);
void	cme_deinit(void);

/*MCIN*/
void	mcin_update_virt(void *virt);
int	mcin_init(MM_CORE_HW_IFC *core_param);
void	mcin_deinit(void);

/*CABAC*/
void	cabac_update_virt(void *virt);
int	cabac_init(MM_CORE_HW_IFC *core_param);
void	cabac_deinit(void);

/*VCE*/
void	h264_vce_update_virt(void *virt);
int	h264_vce_init(MM_CORE_HW_IFC *core_param);
void	h264_vce_deinit(void);

#if defined(CONFIG_MM_SECURE_DRIVER)

/*MCIN*/
int	mcin_secure_init(MM_CORE_HW_IFC *core_param);
void	mcin_secure_deinit(void);

/*CABAC*/
int	cabac_secure_init(MM_CORE_HW_IFC *core_param);
void	cabac_secure_deinit(void);

/*VCE*/
int	h264_vce_secure_init(MM_CORE_HW_IFC *core_param);
void	h264_vce_secure_deinit(void);

/*OL*/
int	h264_ol_secure_init(MM_CORE_HW_IFC *core_param);
void	h264_ol_secure_deinit(void);

#endif /* CONFIG_MM_SECURE_DRIVER */

#endif
