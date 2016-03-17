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

#include <plat/pv.h>
#include <plat/reg_pv.h>
#include <linux/delay.h>
#include <linux/err.h>

#ifdef PV_HAS_CLK
#include <linux/clk.h>
#endif

#define MAX_PV_INST 1
#define FIFO_SIZE 64
#define FIFO_THRE_TO_FETCH (FIFO_SIZE - 32)

/*
 * These two parameters are being tested by ASIC team and need to be followed
 * up by end of May-2012.
 * Ideally HVS_PIXRC1 should be enabled and should be handled in AXIPV
 */

#undef HVS_PIXRC0_ENABLE
#undef HVS_PIXRC1_ENABLE

#if 0
#define pv_info(fmt, arg...) \
		printk(KERN_INFO "%s:%d " fmt, __func__, __LINE__, ##arg)
#define pv_debug(fmt, arg...) \
		printk(KERN_DEBUG "%s:%d " fmt, __func__, __LINE__, ##arg)
#else
#define pv_info(fmt, arg...)
#define pv_debug(fmt, arg...)
#endif

#define pv_err(fmt, arg...) \
		printk(KERN_ERR "%s:%d " fmt, __func__, __LINE__, ##arg)

static int pv_vid_config(struct pv_config_t *config);
static irqreturn_t pv_isr(int irq, void *dev);

struct pv_dev {
	u32 base_addr;
	u32 irq;  /*Interrupt ID*/
	u8 id; /* 0:PV0, 1:PV1, 2:PV2 */
	volatile u8 state;
	int irq_stat;
#ifdef PV_HAS_CLK
	struct clk *apb_clk;
	struct clk *pix_clk;
#endif
	struct work_struct err_work;
	struct work_struct eof_work;
	void (*err_cb)(int err); /* Error Callback- run in ISR context */
	void (*eof_cb)(void); /* End-Of-Frame Callback- run in ISR context */
	struct pv_config_t vid_config;
};

#ifdef PV_HAS_CLK
static inline void pv_clk_enable(struct pv_dev *dev)
{
	if (clk_enable(dev->apb_clk))
		pv_err("failed to enable apb clk\n");
	if (clk_enable(dev->pix_clk))
		pv_err("failed to enable pix clk\n");
}

static inline void pv_clk_disable(struct pv_dev *dev)
{
	clk_disable(dev->pix_clk);
	clk_disable(dev->apb_clk);
}
#else
static inline void pv_clk_enable(struct pv_dev *dev)
{
}

static inline void pv_clk_disable(struct pv_dev *dev)
{
}
#endif

static bool g_pv_init[MAX_PV_INST];

static DEFINE_SPINLOCK(lock);

static void err_cb(struct work_struct *work)
{
	unsigned long flags;
	int irq_stat;
	struct pv_dev *dev = container_of(work, struct pv_dev, err_work);

	spin_lock_irqsave(&lock, flags);
	irq_stat = dev->irq_stat;
	dev->irq_stat = 0;
	spin_unlock_irqrestore(&lock, flags);

	if (dev->err_cb)
		dev->err_cb(irq_stat);
}

static void eof_cb(struct work_struct *work)
{
	struct pv_dev *dev = container_of(work, struct pv_dev, eof_work);
	if (PV_STOPPED == dev->state) {
		u32 pv_base = dev->base_addr;
		writel(0, pv_base + REG_PV_INTEN);
		writel(readl(pv_base + REG_PV_C) & ~PVEN, pv_base + REG_PV_C);
		pv_clk_disable(dev);
	}
	if (dev->eof_cb)
		dev->eof_cb();
}

int pv_init(struct pv_init_t *init, struct pv_config_t **config)
{
	struct pv_dev *dev;
	int ret;

	if (!init || (init->id >= MAX_PV_INST) || !init->irq
#ifdef PV_HAS_CLK
		|| !init->apb_clk_name || !init->pix_clk_name
#endif
		|| !init->base_addr || !init->err_cb || !init->eof_cb) {
		pv_err("Invalid input parameters\n");
		ret = -EINVAL;
		goto done;
	}
	if (g_pv_init[init->id]) {
		pv_err("All instances of PV have already been initialised\n");
		ret = -EPERM;
		goto done;
	}

	dev = kzalloc(sizeof(struct pv_dev), GFP_KERNEL);
	if (!dev) {
		pv_err("couldn't allocate memory for pv_data\n");
		ret = -ENOMEM;
		goto done;
	}
	ret = request_irq(init->irq, pv_isr, IRQF_TRIGGER_HIGH, "PV", dev);
	if (ret < 0) {
		pv_err("failed to get irq\n");
		goto fail;
	}
#ifdef CONFIG_SMP
	irq_set_affinity(init->irq, cpumask_of(0));
#endif
#ifdef PV_HAS_CLK
	dev->apb_clk = clk_get(NULL, init->apb_clk_name);
	if (IS_ERR(dev->apb_clk)) {
		pv_err("failed to get %s\n", init->apb_clk_name);
		ret = PTR_ERR(dev->apb_clk);
		goto fail;
	}

	dev->pix_clk = clk_get(NULL, init->pix_clk_name);
	if (IS_ERR(dev->pix_clk)) {
		pv_err("failed to get %s\n", init->pix_clk_name);
		ret = PTR_ERR(dev->pix_clk);
		goto fail;
	}
#endif
	dev->id = init->id;
	dev->irq = init->irq;
	dev->base_addr = init->base_addr;
	dev->err_cb = init->err_cb;
	dev->eof_cb = init->eof_cb;
	INIT_WORK(&dev->err_work, err_cb);
	INIT_WORK(&dev->eof_work, eof_cb);
	dev->state = PV_INIT_DONE;
	if (!g_display_enabled)
		dev->state = PV_INIT_DONE;
	else{
		if (readl(dev->base_addr + REG_PV_VC) & CMD_MODE)
			dev->state = PV_STOPPED;
		else {
			pv_clk_enable(dev);
			dev->state = PV_ENABLED;
		}

	}
	g_pv_init[init->id] = true;
	*config = &dev->vid_config;
	ret = 0;
	goto done;

fail:
	free_irq(init->irq, NULL);
	kfree(dev);
done:
	return ret;
}

static irqreturn_t pv_isr(int irq, void *dev_data)
{
	u32 pv_base, irq_stat;
	struct pv_dev *dev = dev_data;
	u32 schedule_wk = 0;

	pv_base = dev->base_addr;
	irq_stat = readl(pv_base + REG_PV_INTSTAT);

	if (irq_stat & VFP_END) {
#if 0
		if (!config->cont) {
			/*Since the PV will be disabled at the frame boundary,
			this will not be required. In case ASIC verification
			turns out to be negative, then we need to enable this*/
			pv_info("stopping PV\n");
			writel((readl(pv_base + REG_PV_C) | FIFOCLRC)
				& ~PVEN, pv_base + REG_PV_C);
			writel(readl(pv_base + REG_PV_VC) & ~VIDEN,
					pv_base + REG_PV_VC);
		}
#else
		if (PV_STOPPING == dev->state) {
			/*Change PV state to Stopped*/
			dev->state = PV_STOPPED;
			schedule_wk = 1;
		}
#endif
		writel(VFP_END, pv_base + REG_PV_INTSTAT);
		irq_stat = irq_stat & ~VFP_END;
		//disable_irq(dev->irq);
	}


	if (irq_stat & OF_UF) {
		dev->irq_stat |= OF_UF;
		schedule_work(&dev->err_work);
		writel(HVS_UF | HVS_OF | PV_UF, pv_base + REG_PV_STAT);
		writel(OF_UF, pv_base + REG_PV_INTSTAT);
		irq_stat = irq_stat & ~OF_UF;
	}

	if (irq_stat) {
		writel(irq_stat, pv_base + REG_PV_INTSTAT);
	}

	if (schedule_wk)
		schedule_work(&dev->eof_work);
	return IRQ_HANDLED;
}


static int pv_vid_config(struct pv_config_t *vid_config)
{
	u32 pv_base;
	u32 pv_c = 0, pv_vc = 0;
	struct pv_dev *dev;

	/*Check bounds*/
	if ((vid_config->pclk_sel >= PCLK_SEL_TYPE_MAX)
		|| (vid_config->pix_fmt >= PIX_FMT_TYPE_MAX)
		|| ((0 == vid_config->vbp) && !vid_config->cmd)
		|| (vid_config->vsyncd & ~((1 << 17) - 1))) /* PV_VC[22:6]*/
		return -EINVAL;

	if (vid_config->cont
		&& (DSI_VIDEO_CMD_18_24BPP == vid_config->pix_fmt)) {
		vid_config->hbp = (vid_config->hbp > 5) ? vid_config->hbp : 5;
		vid_config->hfp = (vid_config->hfp > 5) ? vid_config->hfp : 5;
		vid_config->hs = (vid_config->hs > 1) ? vid_config->hs : 1;
	}

	dev = container_of(vid_config, struct pv_dev, vid_config);
	pv_base = dev->base_addr;

	/*Clear the FIFO and disable the PV*/
	writel(FIFO_CLR, pv_base + REG_PV_C);
	writel(0, pv_base + REG_PV_INTEN);
	writel(UINT_MAX, pv_base + REG_PV_INTSTAT); /*Ack previous interrupts*/
	writel_relaxed(vid_config->hs | (vid_config->hbp << HBP_SHIFT),
			pv_base + REG_PV_HORZA);
	writel_relaxed((vid_config->hact + vid_config->hbllp) |
			(vid_config->hfp << HFP_SHIFT),
			pv_base + REG_PV_HORZB);
	writel_relaxed((vid_config->hact), pv_base + REG_PV_DSI_HACT_ACT);
	writel_relaxed((vid_config->vs + 1) |
		((vid_config->vbp - 1) << VBP_SHIFT), pv_base + REG_PV_VERTA);
	writel_relaxed(vid_config->vact | (vid_config->vfp << VFP_SHIFT),
			pv_base + REG_PV_VERTB);

	/* Issues are being fixed by ASIC team. Enable this later*/
//	writel_relaxed(OF_UF, pv_base + REG_PV_INTEN);

	pv_c = (vid_config->pix_stretch << PIX_STRETCH_SHIFT) |
		   (vid_config->pclk_sel << PCLK_SEL_SHIFT) |
		   (vid_config->pix_fmt << PIX_F_SHIFT) |
		   (FIFO_THRE_TO_FETCH << FIFO_FULL_LEVEL_SHIFT);

#ifdef HVS_PIXRC0_ENABLE
	pv_c |= (HVS_PIXRC0);
#endif
#ifdef HVS_PIXRC1_ENABLE
	pv_c |= (HVS_PIXRC1);
#endif
	writel_relaxed(pv_c, pv_base + REG_PV_C);

	pv_vc = (vid_config->cont ? FRAMEC : 0) |
			(vid_config->cmd ? CMD_MODE : DSI_VMODE) |
			(vid_config->interlaced ? INT_MODE : 0) |
			(vid_config->vsyncd << VSYNCD_SHIFT);

	writel(pv_vc, pv_base + REG_PV_VC);

	return 0;
}


int pv_change_state(int event, struct pv_config_t *vid_config)
{
	u32 pv_base;
	struct pv_dev *dev;
	int ret = -EINVAL;

	if (!vid_config)
		return -EINVAL;

	dev = container_of(vid_config, struct pv_dev, vid_config);
	pv_base = dev->base_addr;

	switch (event) {
	case PV_VID_CONFIG:
#if 0
		int cnt = 100;
		while ((PV_STOPPING == dev->state) && cnt-- ) {
			if (readl(pv_base + REG_PV_STAT) & VID_IDLE_STAT)
				dev->state = PV_STOPPED;
			else
				usleep_range(1000, 1100);
		}
#endif
		if ((PV_STOPPED == dev->state)
			|| (PV_INIT_DONE == dev->state)) {
			pv_clk_enable(dev);
			ret = pv_vid_config(vid_config);
			pv_clk_disable(dev);
			dev->state = PV_CONFIGURED;
			ret = 0;
		} else {
			return -EBUSY;
		}
		break;
	case PV_START:
		if ((PV_STOPPED == dev->state)
			|| (PV_CONFIGURED == dev->state)) {
			pv_clk_enable(dev);
			writel((readl(pv_base + REG_PV_INTEN)
				& ~VFP_END), pv_base + REG_PV_INTEN);
			writel(readl(pv_base + REG_PV_C) | PVEN,
				pv_base + REG_PV_C);
			writel(readl(pv_base + REG_PV_VC) | VIDEN,
					pv_base + REG_PV_VC);
			//enable_irq(dev->irq);
			if (!vid_config->cont) {
				writel((readl(pv_base + REG_PV_INTEN)
				| VFP_END), pv_base + REG_PV_INTEN);
				dev->state = PV_STOPPING;
			} else {
				dev->state = PV_ENABLED;
			}
			ret = 0;
		} else {
			return -EBUSY;
		}
		break;
	case PV_RESUME_STREAM:
		if (PV_STOPPED == dev->state) {
			writel((readl(pv_base + REG_PV_INTEN)
				& ~VFP_END), pv_base + REG_PV_INTEN);
			writel(readl(pv_base + REG_PV_C) | PVEN,
				pv_base + REG_PV_C);
			writel(readl(pv_base + REG_PV_VC) | VIDEN,
					pv_base + REG_PV_VC);
			if (!vid_config->cont) {
				writel((readl(pv_base + REG_PV_INTEN)
				| VFP_END), pv_base + REG_PV_INTEN);
				dev->state = PV_STOPPING;
			} else {
				dev->state = PV_ENABLED;
			}
			ret = 0;
		} else {
			return -EBUSY;
		}
		break;
	case PV_PAUSE_STREAM_SYNC:
		if (PV_ENABLED == dev->state) {
			unsigned long flags;
			if (vid_config->cont) {
				writel(readl(pv_base + REG_PV_VC) & ~VIDEN,
					pv_base + REG_PV_VC);
				spin_lock_irqsave(&lock, flags);
				dev->state = PV_STOPPING;
				spin_unlock_irqrestore(&lock, flags);
			}
		}

		/* In case of !cont mode, PV would be in stopping state */
		if (PV_STOPPING == dev->state) {
			unsigned long flags;
			int retry = 1000; /*50ms */
			while (retry > 0) {
				if (readl(pv_base + REG_PV_STAT)
					& VID_IDLE_STAT)
					break;
				usleep_range(50, 60);
				retry--;
			}
			if (retry <= 0)
				pv_err("PV_PAUSE_STREAM_SYNC fail in PV_STOPPING\n");

			spin_lock_irqsave(&lock, flags);
			dev->state = PV_STOPPED;
			spin_unlock_irqrestore(&lock, flags);
		}
		ret = 0;
		break;
	case PV_STOP_EOF_ASYNC:
		if (PV_ENABLED == dev->state) {
			unsigned long flags;
			if (vid_config->cont) {
				spin_lock_irqsave(&lock, flags);
				writel(readl(pv_base + REG_PV_VC)
					& ~VIDEN,
					pv_base + REG_PV_VC);
				writel((readl(pv_base + REG_PV_INTEN)
					| VFP_END),
					pv_base + REG_PV_INTEN);
				dev->state = PV_STOPPING;
				spin_unlock_irqrestore(&lock, flags);
			} else {
				spin_lock_irqsave(&lock, flags);
				/* Stop after the currrent frame is txferd.
				Need to find out if there would be an
				interrupt at VFP_END in single-shot/cmd mode.
				+ VFP_END will be enabled for !cont mode.*/
				writel((readl(pv_base + REG_PV_INTEN)
					| VFP_END),
					pv_base + REG_PV_INTEN);
				dev->state = PV_STOPPING;
				spin_unlock_irqrestore(&lock, flags);
			}
			ret = 0;
		} else {
			return -EBUSY;
		}
		break;
	case PV_STOP_IMM:
		if ((PV_STOPPING == dev->state)
			|| (PV_ENABLED == dev->state)) {
			unsigned long flags;
			spin_lock_irqsave(&lock, flags);
			writel(readl(pv_base + REG_PV_VC) & ~VIDEN,
					pv_base + REG_PV_VC);
			writel(readl(pv_base + REG_PV_C) & ~PVEN,
					pv_base + REG_PV_C);
			/* Todo: Clear FIFO? */
			//disable_irq(dev->irq);
			pv_clk_disable(dev);
			dev->state = PV_STOPPED;
			spin_unlock_irqrestore(&lock, flags);
			ret = 0;
		} else {
			return -EBUSY;
		}
		break;
	case PV_RESET:
		/* Need to reset the PV block and take it to hardware
		reset state*/
		break;
	default:
		pv_info("Invalid option %d\n", dev->state);
		break;
	}
	return ret;
}


int pv_get_state(struct pv_config_t *config)
{
	struct pv_dev *dev;

	if (!config)
		return -EINVAL;

	dev = container_of(config, struct pv_dev, vid_config);

	return dev->state;
}


int check_pv_state(int event, struct pv_config_t *config)
{
	u32 pv_base, stat, istat, ien;
	struct pv_dev *dev;

	if (!config)
		return -EINVAL;

	dev = container_of(config, struct pv_dev, vid_config);
	pv_base = dev->base_addr;
	istat = readl(pv_base + REG_PV_INTSTAT);
	stat = readl(pv_base + REG_PV_STAT);
	ien = readl(pv_base + REG_PV_INTEN);
	pv_err("event=%d state=%d istat=0x%x stat=0x%x ien=0x%x\n",
		event, dev->state, istat, stat, ien);

	if (istat & ien) {
		writel(istat, pv_base + REG_PV_INTSTAT);
		return 0;
	} else {
		return -1;
	}
}
