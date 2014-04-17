#ifndef BRCM_VTQINIT_PRIV_H
#define BRCM_VTQINIT_PRIV_H

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

extern int vtq_configure(struct vtq_vce *,
			 /* These three should be vtq_progmemoffset_t
			  * ? */
			 uint32_t loader_base,
			 uint32_t loader_run,
			 uint32_t loadimage_entrypoint,
			 uint32_t *loader_text,
			 size_t loader_textsz,
			 /* the next four should be
			  * vtq_datamemoffset_t really */
			 uint32_t datamem_reservation,
			 uint32_t writepointer_locn,
			 uint32_t readpointer_locn,
			 uint32_t fifo_offset,
			 size_t fifo_length,
			 size_t fifo_entry_size,
			 uint32_t semaphore_id);

extern int vtq_onloadhook(struct vtq_vce *,
		uint32_t pc,
		uint32_t r,
		uint32_t r2,
		uint32_t r3,
		uint32_t r4,
		uint32_t r5,
		uint32_t r6);

#endif
