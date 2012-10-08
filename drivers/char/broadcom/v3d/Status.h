/*******************************************************************************
Copyright 2012 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#ifndef V3D_STATUS_H_
#define V3D_STATUS_H_

#include <linux/types.h>
#include <linux/proc_fs.h>


extern struct proc_dir_entry *ProcEntry_Create(const char *Name, int Permission, struct proc_dir_entry *Directory, void *Context);
extern void ProcEntry_Delete(const char *Name, struct proc_dir_entry *Directory);


#endif /* ifndef V3D_STATUS_H_ */
