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
#ifndef __KONA_TMON_H__
#define __KONA_TMON_H__

#if defined(DEBUG)
#define tmon_dbg printk
#else
#define tmon_dbg(log_typ, format...)					\
	do {								\
		if ((log_typ) == TMON_LOG_ERR)				\
			pr_err(format);					\
		else if (tmon_dbg_mask & (log_typ))			\
			pr_info(format);				\
	} while (0)
#endif

/*flags*/
enum {
	TMON_NOTIFY = 1,
	TMON_HW_SHDWN = (1 << 1),
	TMON_SW_SHDWN = (1 << 2),
};

enum {
	TMON_PVTMON = 1,
	TMON_VTMON = (1 << 1),
	TMON_SUSPEND_POWEROFF = (1 << 2),
};

enum {
	RAW_VAL,
	CELCIUS,
};
struct tmon_state {
	int rising;
	int flags;
};

struct kona_tmon_pdata {
	void __iomem *base_addr;
	int irq;
	struct tmon_state *thold;
	int thold_size;
	int poll_rate_ms;
	int hysteresis;
	void __iomem *chipreg_addr;
	u32 interval_ms;
	int flags;
	char const *tmon_apb_clk;
	char const *tmon_1m_clk;
	int falling; /* fall back temperarure offset*/
};

int tmon_register_notifier(struct notifier_block *notifier);
int tmon_unregister_notifier(struct notifier_block *notifier);
long tmon_get_current_temp(bool celcius, bool avg);
void tmon_set_suspend_poweroff(int poweroff);
int tmon_get_suspend_poweroff(void);

#endif /*__KONA_TMON_H__*/
