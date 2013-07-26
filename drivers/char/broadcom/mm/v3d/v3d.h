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

 #ifndef __V3D_H__
 #define __V3D_H__

 #define V3D_SUBDEV_COUNT 2

 /* Bin and Render Core */
void	v3d_bin_render_update_virt(void *virt);
int	v3d_bin_render_init(MM_CORE_HW_IFC *core_param);
void	v3d_bin_render_deinit(void);

/* User Core */
void    v3d_user_update_virt(void *virt);
int	v3d_user_init(MM_CORE_HW_IFC *core_param);
void	v3d_user_deinit(void);

 #endif
