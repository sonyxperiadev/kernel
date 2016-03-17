/************************************************************************/
/*                                                                      */
/*  Copyright 2012  Broadcom Corporation                                */
/*                                                                      */
/* Unless you and Broadcom execute a separate written software license  */
/* agreement governing use of this software, this software is licensed  */
/* to you under the terms of the GNU General Public License version 2   */
/* (the GPL), available at						*/
/*                                                                      */
/*          http://www.broadcom.com/licenses/GPLv2.php                  */
/*                                                                      */
/*  with the following added to such license:                           */
/*                                                                      */
/*  As a special exception, the copyright holders of this software give */
/*  you permission to link this software with independent modules, and  */
/*  to copy and distribute the resulting executable under terms of your */
/*  choice, provided that you also meet, for each linked independent    */
/*  module, the terms and conditions of the license of that module. An  */
/*  independent module is a module which is not derived from this       */
/*  software.  The special   exception does not apply to any            */
/*  modifications of the software.					*/
/*									*/
/*  Notwithstanding the above, under no circumstances may you combine	*/
/*  this software in any way with any other Broadcom software provided	*/
/*  under a license other than the GPL, without Broadcom's express	*/
/*  prior written consent.						*/
/*									*/
/************************************************************************/
#ifndef _LOWMEMORYKILLER_H
#define _LOWMEMORYKILLER_H

struct lmk_op {
	int op;
};
/*
 * Register callback to LMK to report various statistics,
 * to be considered in the LMK operation.
 * 'op' will be passed as '0', to get the number of pages,
 * currently used by others through the allocator.
 */
struct reg_lmk {
	int (*cbk)(struct reg_lmk *reg_lmk, struct lmk_op *op);

	/* Only to be used by LMK*/
	struct list_head list;
};
extern void register_lmk(struct reg_lmk *);
extern void unregister_lmk(struct reg_lmk *);
#endif
