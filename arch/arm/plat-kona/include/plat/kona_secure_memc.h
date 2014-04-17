/******************************************************************************/
/*                                                                            */
/*  Copyright 2012  Broadcom Corporation                                      */
/*                                                                            */
/*     Unless you and Broadcom execute a separate written software license    */
/*     agreement governing use of this software, this software is licensed    */
/*     to you under the terms of the GNU General Public License version 2     */
/*     (the GPL), available at                                                */
/*                                                                            */
/*          http://www.broadcom.com/licenses/GPLv2.php                        */
/*                                                                            */
/*     with the following added to such license:                              */
/*                                                                            */
/*     As a special exception, the copyright holders of this software give    */
/*     you permission to link this software with independent modules, and     */
/*     to copy and distribute the resulting executable under terms of your    */
/*     choice, provided that you also meet, for each linked independent       */
/*     module, the terms and conditions of the license of that module. An     */
/*     independent module is a module which is not derived from this          */
/*     software.  The special exception does not apply to any modifications   */
/*     of the software.                                                       */
/*                                                                            */
/*     Notwithstanding the above, under no circumstances may you combine      */
/*     this software in any way with any other Broadcom software provided     */
/*     under a license other than the GPL, without Broadcom's express prior   */
/*     written consent.                                                       */
/*                                                                            */
/******************************************************************************/
#ifndef __KONA_SECURE_MEMC_H__
#define __KONA_SECURE_MEMC_H__

#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/kobject.h>
#include <linux/klist.h>
#include <linux/circ_buf.h>
#include <plat/kona_secure_memc_settings.h>


#define MASTER_A7					"a7"
#define MASTER_COMMS					"comms"
#define MASTER_FABRIC					"fabric"
#define MASTER_MM					"mm"
#define INVALID						"invalid"

#define DELETE_MASTER					"del"

#define MEM_TYPE_USR					0x3
#define MEM_TYPE_VC_SUPERVISOR				0x2
#define MEM_TYPE_VC_SECURE				0x1
#define MEM_TYPE_TZ_TRUSTED				0x0

#define REGION_GROUP_MASK				0xFF0000


enum region_parms {
	START_ADDR,
	END_ADDR,
	MEM_TYPE,
	GROUP_MASK,
	REGION_EN,
};

#define	NOT_CONFIGURED					0x00
#define	GROUP_CONFIGURED				0x02
#define	REGION_CONFIGURED				0x3C
#define	REGION_START_CONFIGURED				0x04
#define	REGION_END_CONFIGURED				0x08
#define	REGION_MEM_TYPE_CONFIGURED			0x10
#define	REGION_GROUP_MASK_CONFIGURED			0x20
#define	COMPLETE_REGION_CONFIGURED			0x40


#define REGION_MEM_TYPE_SHIFT				1
#define RGN_GID_SANDBOX_EN_SHIFT			16
#define RGN_GID_SANDBOX_EN_MASK				0x00FF0000


#define REGION_NUM_MASK					0xFF000000
#define REGION_NUM_SHIFT				24

#define CONFIGURED_REGION_NUM_MASK			0x00FF0000
#define CONFIGURED_REGION_NUM_SHIFT			16


#define MEMC_NOT_ACTIVATED				0x00
#define MEMC_WAS_ACTIVATED				0x02
#define MEMC_IS_ACTIVATED				0x01


#define PORT0_GIDCMP0_OFFSET				0x100
#define REGION0_CTL_OFFSET				16
#define REGION0_START_OFFSET				4
#define REGION0_END_OFFSET				8

#define MAX_MASTER_LENGTH				10
#define VIOLATION_INFO_LEN				10
#define VIOLATION_POLL_DELAY				10000

struct kona_secure_memc;


struct default_master_map {
	char master[MAX_MASTER_LENGTH];
	u32 axi_val;
	u32 axi_mask;
};

struct kona_memc_regions {
	u32 region_id;
	u32 start_address;
	u32 end_address;
	u32 mem_type;
	u32 group_mask;
	int (*write_regionx)(struct kona_secure_memc*, u32,
		enum region_parms, u32);
	int (*read_regionx)(struct kona_secure_memc*, u32);
	struct kobject kobj;
	struct list_head group_region_list;
	struct kona_secure_memc *memc_dev;
};

struct kona_memc_master {
	char *name;
	u32 axi_id;
	struct list_head group_master_list;
};


struct access_control {
	u32 region_access[NUM_OF_ALLOWED_MASTERS];
};

struct kona_memc_group {
	u32 group_id;
	struct access_control access;
	struct list_head master_list;
	struct list_head region_list;
};


struct kona_memc_port {
	u32 port_id;
	struct kobject kobj;
	char *default_master;
	struct kona_memc_group **group;
	struct kona_secure_memc *memc_dev;
	int (*write_portx_gidx_cmp)(struct kona_secure_memc*, u32, u32, char*);
	int (*read_portx_gidx_cmp)(struct kona_secure_memc*, u32);
};

struct kona_memc_bitmaps {
	u32 port_map[NUM_OF_MEMC_PORTS][NUM_OF_GROUPS];
	u32 group_bitmap;
	u32 port_bitmap;
	u32 region_bitmap;
	u32 is_memc_activated;
};

struct kona_memc_logging {
	char *memc_log;
	u32 memc_log_index;
	u32 violation_info[VIOLATION_INFO_LEN];
	u32 violation_index;
	struct kobject kobj;
	struct kona_secure_memc *memc_dev;
};

struct kona_secure_memc {
	struct device *dev;
	struct kobject *kobj;
	struct kobject *region_kobj;
	struct kona_memc_port **port;
	struct kona_memc_regions **region;
	struct kona_memc_logging *logging;
	struct kona_secure_memc_pdata *pdata;
	struct kona_memc_bitmaps memc_config_status;
	struct notifier_block sec_memc_panic_block;
	struct notifier_block sec_memc_reboot_block;
	struct delayed_work memc_work;
	char **masters;
	u32 memc_state;
	u32 handle;
	int (*init_static_config_memc)(struct kona_secure_memc *);
};

#endif /*__KONA_SECURE_MEMC_H__*/



