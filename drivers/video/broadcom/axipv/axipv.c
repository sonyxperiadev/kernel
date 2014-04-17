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

#include <plat/reg_axipv.h>
#include <plat/axipv.h>

#ifdef AXIPV_HAS_CLK
#include <linux/clk.h>
#endif

#if defined(CONFIG_MACH_BCM_FPGA) || defined(CONFIG_MACH_BCM_FPGA_E)
static struct timeval prof_tv1, prof_tv2;
#endif

#ifdef DEBUG_FPS
static int fps;
static struct timeval isr_tv1, isr_tv2;
#endif


#if 0
#define axipv_info(fmt, args...) \
		printk(KERN_INFO"%s:%d " fmt, __func__, __LINE__, ##args)
#define axipv_debug(fmt, args...) \
		printk(KERN_DEBUG"%s:%d " fmt, __func__, __LINE__, ##args)
#else
#define axipv_info(fmt, args...)
#define axipv_debug(fmt, args...)
#endif

#define axipv_err(fmt, args...) \
		printk(KERN_ERR"%s:%d " fmt, __func__, __LINE__, ##args)

/* can use axipv_set_buff_status alternatively */
#define axipv_add_new_buff_info(buff, idx, fb_addr) do {\
		buff[idx].addr = fb_addr;\
		buff[idx].status = AXIPV_BUFF_SCHEDULED;\
		} while (0)


static irqreturn_t axipv_isr(int err, void *dev_id);

/*Todo: Use atomic read and write on this variable*/
static bool g_axipv_init;
static volatile u32 g_nxt, g_curr;
static DEFINE_SPINLOCK(lock);

struct axipv_buff {
	u32 addr;
	u32 status;
};

enum {
	AXIPV_BUFF_UNKNOWN,
	AXIPV_BUFF_AVAILABLE,
	AXIPV_BUFF_SCHEDULED,
	AXIPV_BUFF_TXFERED,
};

struct axipv_dev {
	u32 base_addr;
	volatile u8 state;
	int irq_stat;
	int prev_irq_handled;
	int done_buff;
#ifdef AXIPV_HAS_CLK
	struct clk *clk;
#endif
	struct axipv_buff buff[AXIPV_MAX_DISP_BUFF_SUPP];
	struct axipv_config_t config;
	struct work_struct irq_work;
	struct work_struct release_work;
	void (*irq_cb)(int err);
	void (*release_cb)(u32 free_buf);
	void (*vsync_cb)(void);
};

#ifdef AXIPV_HAS_CLK
static inline void axipv_clk_enable(struct axipv_dev *dev)
{
	if (clk_enable(dev->clk))
		axipv_err("failed to enable clk\n");
}

static inline void axipv_clk_disable(struct axipv_dev *dev)
{
	clk_disable(dev->clk);
}
#else
static inline void axipv_clk_enable(struct axipv_dev *dev)
{
}

static inline void axipv_clk_disable(struct axipv_dev *dev)
{
}
#endif

static inline u32 axipv_get_buff_status(struct axipv_buff *buff, u32 addr)
{
	int i, ret = AXIPV_BUFF_UNKNOWN;
	for (i = 0; i < AXIPV_MAX_DISP_BUFF_SUPP; i++) {
		if (addr == buff[i].addr) {
			ret = buff[i].status;
			break;
		}
	}
	return ret;
}

static inline void axipv_dump_buff_status(struct axipv_buff *buff)
{
	int i;
	for (i = 0; i < AXIPV_MAX_DISP_BUFF_SUPP; i++)
		axipv_err("0x%x state=%d\n", buff[i].addr, buff[i].status);
}

static inline u32 axipv_get_free_buff_index(struct axipv_buff *buff)
{
	int i;
	for (i = 0; i < AXIPV_MAX_DISP_BUFF_SUPP; i++) {
		if (AXIPV_BUFF_AVAILABLE == buff[i].status)
			break;
	}

	if (unlikely(AXIPV_MAX_DISP_BUFF_SUPP == i)) {
		axipv_dump_buff_status(buff);
	}
	return i;
}

static inline u32 axipv_set_buff_status(struct axipv_buff *buff,
					u32 addr, u32 status)
{
	int i, ret = 1;
	for (i = 0; i < AXIPV_MAX_DISP_BUFF_SUPP; i++) {
		if (addr == buff[i].addr) {
			switch (status) {
			case AXIPV_BUFF_AVAILABLE:
				if ((AXIPV_BUFF_TXFERED == buff[i].status) ||
				(AXIPV_BUFF_SCHEDULED == buff[i].status)) {
					buff[i].status = status;
					ret = 0;
				} else {
					ret = 2;
				}
				break;
			case AXIPV_BUFF_SCHEDULED:
				if (AXIPV_BUFF_AVAILABLE == buff[i].status) {
					buff[i].status = status;
					ret = 0;
				} else {
					ret = 2;
				}
				break;
			case AXIPV_BUFF_TXFERED:
				if (AXIPV_BUFF_SCHEDULED == buff[i].status) {
					buff[i].status = status;
					ret = 0;
				} else {
					ret = 2;
				}
				break;
			default:
				ret = 3;
			}
			if (!ret)
				break;
		}
	}
	/*
	 * ret value
	 * 0 => Success
	 * 1 => Invalid address
	 * 2 => Invalid current state
	 * 3 => Invalid new state
	 */
	return ret;
}


static void process_irq(struct work_struct *work)
{
	struct axipv_dev *dev = container_of(work, struct axipv_dev, irq_work);
	int irq_stat;
	unsigned long flags;

	spin_lock_irqsave(&lock, flags);
	irq_stat = dev->irq_stat;
	dev->irq_stat = 0;
	dev->prev_irq_handled = 1;
	spin_unlock_irqrestore(&lock, flags);

	dev->irq_cb(irq_stat);
}

static void process_release(struct work_struct *work)
{
	int i, err;
	struct axipv_dev *dev = container_of(work, struct axipv_dev,
						release_work);
	struct axipv_buff *buff =  dev->buff;
	unsigned long flags;

	spin_lock_irqsave(&lock, flags);
	for (i = 0; i < AXIPV_MAX_DISP_BUFF_SUPP; i++) {
		if (AXIPV_BUFF_TXFERED == buff[i].status) {
			axipv_debug("releasing %p\n", (void *) buff[i].addr);
			err = axipv_set_buff_status(buff, buff[i].addr,
						AXIPV_BUFF_AVAILABLE);
			dev->release_cb(buff[i].addr);
			if (err)
				axipv_err("couldn't set buff status err=%d\n",
					err);
		}
	}
	spin_unlock_irqrestore(&lock, flags);
}


int axipv_init(struct axipv_init_t *init, struct axipv_config_t **config)
{
	int ret = 0, ctrl = 0, i, tries;
	struct axipv_dev *dev;

	if (!init || !init->irq || !init->base_addr
#ifdef AXIPV_HAS_CLK
		|| !init->clk_name
#endif
		|| !init->release_cb || !init->irq_cb) {
		ret = -EINVAL;
		goto done;
	}
	if (g_axipv_init) {
		ret = -EPERM;
		goto done;
	}

	dev = kzalloc(sizeof(struct axipv_dev), GFP_KERNEL);
	if (!dev) {
		axipv_err("couldn't allocate memmory for axipv dev");
		ret = -ENOMEM;
		goto done;
	}
	ret = request_irq(init->irq, axipv_isr, IRQF_TRIGGER_HIGH,
			"axipv", dev);
	if (ret < 0) {
		axipv_err("failed to irq\n");
		goto fail_irq;
	}
#ifdef CONFIG_SMP
	irq_set_affinity(init->irq, cpumask_of(0));
#endif

#ifdef AXIPV_HAS_CLK
	dev->clk = clk_get(NULL, init->clk_name);
	if (IS_ERR(dev->clk)) {
		axipv_err("failed to get clk");
		ret = PTR_ERR(dev->clk);
		goto fail_clk;
	}
#endif
	axipv_clk_enable(dev);
	if (!g_display_enabled) {

		ctrl &= SFT_RSTN_MASK;
		/* Do an immediate soft reset and wait for completion */
		writel(ctrl, init->base_addr + REG_CTRL);
		tries = 10;
		while (tries && !(readl(init->base_addr + REG_CTRL)
			& SFT_RSTN_DONE)) {
			axipv_debug("waiting for reset to complete\n");
			udelay(5);
			--tries;
		}
		if (!tries) {
			axipv_err("AXIPV couldn't be reset! Aborting\n");
			ret = -ENODEV;
			goto fail_reset;
		}

		axipv_clk_disable(dev);
	}
	dev->base_addr = init->base_addr;
	dev->irq_cb = init->irq_cb;
	dev->release_cb = init->release_cb;
	dev->vsync_cb = init->vsync_cb;
	INIT_WORK(&dev->irq_work, process_irq);
	INIT_WORK(&dev->release_work, process_release);
	for (i = 0; i < AXIPV_MAX_DISP_BUFF_SUPP; i++)
		dev->buff[i].status = AXIPV_BUFF_AVAILABLE;
	if (!g_display_enabled) {
		dev->state = AXIPV_INIT_DONE;
	} else {
		dev->prev_irq_handled = 1;
		dev->irq_stat = 0;
		if (readl(dev->base_addr + REG_CTRL) & AXIPV_CMD_MODE) {
			axipv_clk_disable(dev);
			dev->state = AXIPV_STOPPED;
		} else
			dev->state = AXIPV_ENABLED;
		writel_relaxed(UINT_MAX, dev->base_addr + REG_INTR_CLR);
		writel_relaxed(UINT_MAX, dev->base_addr + REG_INTR_EN);
	}
	g_axipv_init = true;
	*config = &dev->config;
#ifdef DEBUG_FPS
	do_gettimeofday(&isr_tv1);
	do_gettimeofday(&isr_tv2);
#endif
	ret = 0;
	goto done;

fail_reset:
	axipv_clk_disable(dev);
fail_irq:
fail_clk:
	free_irq(init->irq, NULL);
	kfree(dev);
done:
	return ret;
}


static inline int axipv_config(struct axipv_config_t *config)
{
	int ctrl = 0;
	struct axipv_dev *dev;
	u32 axipv_base;
	u32 tx_size;
	int buff_index;
	u32 arcache = AXIPV_ARCACHE;
	struct axipv_sync_buf_t *buff;
	u32 align_chk;

	if (!config)
		return -EINVAL;

	buff = &config->buff.sync;
	/* Buffer address, line-stride and bytes-per-line have to be
	 * 8 or 16 byte aligned */
	align_chk = buff->addr | buff->xlen | config->width;
	if (unlikely(align_chk & 0x7)) {
		axipv_err("Unaligned addr=0x%x xlen=0x%x width=0x%x\n",
				buff->addr, buff->xlen, config->width);
		return -EINVAL;
	}
	/* If any of buffer address, line-stride or bytes-per-line is not
	 * 16byte aligned, then ARCACHE has to be set to 0 */
	if (unlikely(align_chk & 0xf))
		arcache = 0;

	dev = container_of(config, struct axipv_dev, config);
	axipv_base = dev->base_addr;

	writel_relaxed(buff->xlen, axipv_base + REG_BYTES_PER_LINE);
	writel_relaxed(buff->ylen, axipv_base + REG_LINES_PER_FRAME);
	writel_relaxed(buff->addr, axipv_base + REG_NXT_FRAME);
	axipv_debug("posting %p\n", (void *) buff->addr);
	g_nxt = buff->addr;
	g_curr = buff->addr;
	buff_index = axipv_get_free_buff_index(dev->buff);
	if (buff_index >= AXIPV_MAX_DISP_BUFF_SUPP)
		axipv_err("Couldn't get free buff index\n");
	else
		axipv_add_new_buff_info(dev->buff, buff_index, buff->addr);

	writel_relaxed(config->width, axipv_base + REG_LINE_STRIDE);
	writel_relaxed(AXIPV_BURST_LEN, axipv_base + REG_BURST_LEN);
	writel_relaxed(config->height - 1, axipv_base + REG_TE_LINE);
	writel_relaxed(0, axipv_base + REG_INTR_EN);
	writel_relaxed(UINT_MAX, axipv_base + REG_INTR_CLR);
	dev->prev_irq_handled = 1;
	dev->irq_stat = 0;

	writel_relaxed(AXIPV_AXI_ID1, axipv_base + REG_AXI_ID_CFG_1);
	writel_relaxed(AXIPV_AXI_ID2, axipv_base + REG_AXI_ID_CFG_2);

	ctrl =  SFT_RSTN_DONE | (config->cmd ? AXIPV_CMD_MODE : 0)
		| NUM_OUTSTDG_XFERS_8 | AXI_ID_SYS_DUAL | AXIPV_ARPROT
		| arcache | (config->test ? AXIPV_TESTMODE : 0);

	if (config->bypassPV)
		ctrl = ctrl | (1 << 8) | (1 << 29);

	ctrl |= (config->pix_fmt << PIXEL_FORMAT_SHIFT);

	tx_size = buff->xlen * buff->ylen;
	if ((AXIPV_PIXEL_FORMAT_24BPP_RGB == config->pix_fmt)
		|| (AXIPV_PIXEL_FORMAT_24BPP_BGR == config->pix_fmt))
		tx_size *= 4;
	else
		tx_size *= 2;
	if (tx_size > AXIPV_PV_THRES) {
		writel_relaxed(AXIPV_PV_THRES, axipv_base + REG_PV_THRESH);
		writel_relaxed(AXIPV_W_LVL_1, axipv_base + REG_W_LVL_1);
		writel_relaxed(AXIPV_W_LVL_2, axipv_base + REG_W_LVL_2);
		writel_relaxed(AXIPV_LB_EMPTY_THRES, axipv_base
				+ REG_LB_EMPTY_THRES);
	} else if (tx_size >= AXIPV_PV_THRES_MIN) {
		writel_relaxed(tx_size, axipv_base + REG_PV_THRESH);
		writel_relaxed(AXIPV_W_LVL_1_MIN, axipv_base + REG_W_LVL_1);
		writel_relaxed(AXIPV_W_LVL_2_MIN, axipv_base + REG_W_LVL_2);
		writel_relaxed(AXIPV_LB_EMPTY_THRES_MIN, axipv_base
				+ REG_LB_EMPTY_THRES);
	} else {
		axipv_err("tx_size=%d is too less to be sent via axipv\n",
			tx_size);
	}

	if (config->bypassPV) {
		int pv_start_thre = readl(axipv_base + REG_PV_THRESH);
		if (!((config->pix_fmt == AXIPV_PIXEL_FORMAT_24BPP_RGB) ||
			(config->pix_fmt == AXIPV_PIXEL_FORMAT_24BPP_BGR) ||
			(config->pix_fmt == AXIPV_PIXEL_FORMAT_16BPP_PACKED) ||
			(config->pix_fmt == AXIPV_PIXEL_FORMAT_16BPP_UNPACKED)))
			axipv_err("Unsupported format=%d!\n",config->pix_fmt);
		writel_relaxed(pv_start_thre + 1, axipv_base + REG_W_LVL_1);
		writel_relaxed(pv_start_thre / 4, axipv_base + REG_W_LVL_2);
		writel_relaxed(AXIPV_LB_EMPTY_THRES_MIN,
					axipv_base + REG_LB_EMPTY_THRES);
	}
	writel(ctrl , axipv_base + REG_CTRL);

	return 0;
}

#define axipv_release_buff(addr) \
	do { \
		u32 err = axipv_set_buff_status(dev->buff, addr,\
		AXIPV_BUFF_TXFERED); \
		if (!err)\
			schedule_work(&dev->release_work);\
	} while (0)

static bool __is_fifo_draining_eof(struct axipv_dev *dev)
{
	u32 axipv_base, curr_line_num;

	axipv_base = dev->base_addr;
	curr_line_num = readl(axipv_base + REG_AXIPV_STATUS) >> 16;
	if (!curr_line_num || ((dev->config.height - curr_line_num)
			< (AXIPV_LB_SIZE / (dev->config.width >> 2))))
		return true;
	return false;
}

static irqreturn_t axipv_isr(int err, void *dev_id)
{
	struct axipv_dev *dev = dev_id;
	int irq_stat;
	u32 axipv_base;
	bool disable_axipv = false, disable_clk = false;
	u32 curr_reg_val;
	unsigned long flags;

	axipv_base = dev->base_addr;

	spin_lock_irqsave(&lock, flags);
	irq_stat = readl(axipv_base + REG_INTR_STAT);
	axipv_debug("irq_stat=0x%x\n", irq_stat);

	if (!irq_stat) {
		/* If clock is disabled due to wrong state transitions */
		axipv_clk_enable(dev);
		irq_stat = readl(axipv_base + REG_INTR_STAT);
		axipv_err("irq stat=0x%x, stat=0x%x, curr=0x%x ctrl=0x%x\n",
		irq_stat, readl(axipv_base + REG_AXIPV_STATUS),
		readl(axipv_base + REG_CUR_FRAME),
		readl(axipv_base + REG_CTRL));
		disable_clk = true;
	}
	if ((irq_stat & PV_START_THRESH_INT) && (dev->config.cmd)) {
		/* Change PV_START_THRESH_INT to (WC - 1) where
		 * WC = WordCount or packetsize chosen in DSI.
		 * WC = DSI FIFO size as of now. */
		int pv_thresh;
		if ((AXIPV_PIXEL_FORMAT_24BPP_RGB == dev->config.pix_fmt) ||
			(AXIPV_PIXEL_FORMAT_24BPP_BGR == dev->config.pix_fmt))
			pv_thresh = CM_PKT_SIZE_B / 6;
		else
			pv_thresh = CM_PKT_SIZE_B / 8;
		axipv_debug("changing pv_start_thresh from 0x%x to 0x%x\n",
		readl(axipv_base + REG_PV_THRESH), pv_thresh);

		writel(pv_thresh, axipv_base + REG_PV_THRESH);
	}
	if (irq_stat & WATER_LVL2_INT) {
		if ((AXIPV_STOPPED == dev->state)
			|| ((AXIPV_STOPPING == dev->state) &&
			__is_fifo_draining_eof(dev))) {
			irq_stat = irq_stat & ~WATER_LVL2_INT;
			writel(WATER_LVL2_INT, axipv_base + REG_INTR_CLR);
		}
	}

	if (irq_stat & FRAME_END_INT) {
		/* Do not turn off clocks right here. Even in command mode!
		 * Reason: This will only indicate that the entire frame has
		 *         been fetched into AXIPV's LB. PV needs to pull in all
		 *         the data in AXIPV's LB before we turn off clocks */
#ifdef DEBUG_FPS
		++fps;
		do_gettimeofday(&isr_tv1);
		if ((isr_tv1.tv_sec - isr_tv2.tv_sec) > 1) {
			axipv_err("axipv fps = %d\n", fps/((int)(isr_tv1.tv_sec
				- isr_tv2.tv_sec)));
			memcpy(&isr_tv2, &isr_tv1, sizeof(isr_tv1));
			fps = 0;
		}
#endif
		curr_reg_val = readl(axipv_base + REG_CUR_FRAME);
		if (g_curr != curr_reg_val) {
			if (g_curr) {
				axipv_debug("isr %p\n", (void *) g_curr);
				axipv_release_buff(g_curr);
			}
			g_curr = curr_reg_val;
			if (AXIPV_STOPPING == dev->state)
				dev->state = AXIPV_STOPPED;
		}
		writel(FRAME_END_INT, axipv_base + REG_INTR_CLR);
		if (AXIPV_STOPPED != dev->state)
			irq_stat = irq_stat & ~FRAME_END_INT;
	}
	if (irq_stat & AXIPV_DISABLED_INT) {
		uint32_t ctrl;
		if (g_curr) {
			axipv_release_buff(g_curr);
			g_curr = 0;
		}
		if (g_nxt) {
			axipv_release_buff(g_nxt);
			g_nxt = 0;
		}
		disable_axipv = true;
		disable_clk = true;
		/* Client needs to be informed of the disabled state */
		/* irq_stat = irq_stat & ~AXIPV_DISABLED_INT; */
#if defined(CONFIG_MACH_BCM_FPGA) || defined(CONFIG_MACH_BCM_FPGA_E)
		do_gettimeofday(&prof_tv2);
		axipv_info("axipv tx time(us)=%d\n",
			(prof_tv2.tv_sec - prof_tv1.tv_sec) * 1000000 +
			prof_tv2.tv_usec - prof_tv1.tv_usec);
#endif
		/* DSI's PIXD FIFO can be accessed either by AXIPV or by any of
		 * CPU and MMDMA. Bits 8 and 29 are the MUX select */
		ctrl = readl(axipv_base+REG_CTRL);
		ctrl = ctrl & ~(1 << 8);
		ctrl = ctrl & ~(1 << 29);
		writel(ctrl, axipv_base + REG_CTRL);
	}
	if ((irq_stat & TE_INT) && dev->vsync_cb) {
		dev->vsync_cb();
		writel(TE_INT, axipv_base + REG_INTR_CLR);
		irq_stat &= ~TE_INT;
	}

	if (irq_stat) {
		if (dev->prev_irq_handled) {
			dev->irq_stat = irq_stat;
			dev->prev_irq_handled = 0;
		} else {
			dev->irq_stat |= irq_stat;
		}
		schedule_work(&dev->irq_work);
		writel(irq_stat, axipv_base + REG_INTR_CLR);
	}
	if (disable_axipv) {
		dev->state = AXIPV_STOPPED;
		writel(AXIPV_DISABLED_INT, axipv_base + REG_INTR_CLR);
	}
	spin_unlock_irqrestore(&lock, flags);

	if (disable_clk)
		axipv_clk_disable(dev);

	return IRQ_HANDLED;
}

static inline int post_async(struct axipv_config_t *config)
{
	u32 curr_reg_val;
	struct axipv_dev *dev;
	u32 axipv_base;
	int buff_index;

	dev = container_of(config, struct axipv_dev, config);
	axipv_base = dev->base_addr;

	axipv_debug("new buff posted 0x%x\n", config->buff.async);

	/* Handle the unusal case by bypassing the hardware since the hardware
	 * is transferring the same buffer */
	if ((config->buff.async == g_curr) || (config->buff.async == g_nxt)) {
		axipv_err("Likely tearing on screen posted:0x%x curr= 0x%x nxt=0x%x\n",
		config->buff.async, g_curr, g_nxt);
		axipv_dump_buff_status(dev->buff);
		dev->release_cb(config->buff.async);
		return 0;
	}

	writel(config->buff.async, axipv_base + REG_NXT_FRAME);
	curr_reg_val = readl(axipv_base + REG_CUR_FRAME);
	buff_index = axipv_get_free_buff_index(dev->buff);
	if (buff_index >= AXIPV_MAX_DISP_BUFF_SUPP) {
		axipv_err("Couldn't get free buff index\n");
	} else {
		axipv_add_new_buff_info(dev->buff, buff_index,
					config->buff.async);
	}
	/* if ((curr_reg_val == g_curr)
		|| (curr_reg_val == config->buff.async)) {*/
	if (curr_reg_val != g_nxt) {
		/* Skipped buffer has to be released*/
		if (g_nxt) {
			axipv_debug("skipped buff=0x%x\n", g_nxt);
			axipv_release_buff(g_nxt);
			g_nxt = 0;
		}
	}
	g_nxt = config->buff.async;

	return 0;
}

static inline int post_sync(struct axipv_config_t *config)
{
	int buff_index;
	struct axipv_dev *dev;
	u32 axipv_base;
	unsigned long flags;

	dev = container_of(config, struct axipv_dev, config);
	axipv_base = dev->base_addr;

	if (config->cmd) {
		u32 tx_size;
		writel_relaxed(config->buff.sync.xlen,
			axipv_base + REG_BYTES_PER_LINE);
		writel_relaxed(config->buff.sync.ylen,
			axipv_base + REG_LINES_PER_FRAME);

		tx_size = config->buff.sync.xlen * config->buff.sync.ylen;
		if ((AXIPV_PIXEL_FORMAT_24BPP_RGB == config->pix_fmt)
			|| (AXIPV_PIXEL_FORMAT_24BPP_BGR == config->pix_fmt))
			tx_size *= 4;
		else
			tx_size *= 2;
		tx_size /= 8; /* AXIPV_PV_THRES is in multiples of 8bytes*/
		if (tx_size > AXIPV_PV_THRES) {
			writel_relaxed(AXIPV_PV_THRES, axipv_base
					+ REG_PV_THRESH);
			writel_relaxed(AXIPV_W_LVL_1, axipv_base + REG_W_LVL_1);
			writel_relaxed(AXIPV_W_LVL_2, axipv_base + REG_W_LVL_2);
			writel_relaxed(AXIPV_LB_EMPTY_THRES, axipv_base
					+ REG_LB_EMPTY_THRES);
		} else {
			writel_relaxed(tx_size, axipv_base + REG_PV_THRESH);
			writel_relaxed(AXIPV_W_LVL_1_MIN, axipv_base
					+ REG_W_LVL_1);
			writel_relaxed(AXIPV_W_LVL_2_MIN, axipv_base
					+ REG_W_LVL_2);
			writel_relaxed(AXIPV_LB_EMPTY_THRES_MIN, axipv_base
					+ REG_LB_EMPTY_THRES);
		}

	}
	writel(config->buff.sync.addr, axipv_base + REG_NXT_FRAME);
	buff_index = axipv_get_free_buff_index(dev->buff);
	if (buff_index >= AXIPV_MAX_DISP_BUFF_SUPP) {
		axipv_err("Couldn't get free buff index\n");
	} else {
		axipv_add_new_buff_info(dev->buff, buff_index,
					config->buff.sync.addr);
	}

	spin_lock_irqsave(&lock, flags);
	if (config->cmd)
		writel(readl(axipv_base + REG_CTRL) | AXIPV_EN,
				axipv_base + REG_CTRL);

	/*Raghu: update state to enabled within spinlock */

	if (g_curr) {
		/* Not required, but just to be sure */
		while (config->buff.sync.addr !=
			readl(axipv_base + REG_CUR_FRAME))
			udelay(1);
	}
	if (config->cmd) {
		writel(0, axipv_base + REG_NXT_FRAME);
		writel(readl(axipv_base  + REG_CTRL) & ~AXIPV_EN,
				axipv_base + REG_CTRL);
	}
	spin_unlock_irqrestore(&lock, flags);
	g_nxt = config->buff.sync.addr;
	return 0;
}

int axipv_post(struct axipv_config_t *config)
{
	struct axipv_dev *dev;
	if (!config)
		return -EINVAL;

	dev = container_of(config, struct axipv_dev, config);
	if (AXIPV_ENABLED != dev->state)
		return -ENODEV;

	if (dev->config.async)
		return post_async(config);
	else
		return post_sync(config);
}


void dump_debug_info(struct axipv_dev *dev)
{
	u32 axipv_base;
	axipv_clk_enable(dev);
	axipv_base = dev->base_addr;
	axipv_err("state=%d, gcurr=%p, gnxt=%p\n", dev->state, (void *)
			g_curr, (void *) g_nxt);
	axipv_err("%p %p %p %p %p %p %p\n",
	(void *) readl(axipv_base+REG_CUR_FRAME),
	(void *) readl(axipv_base+REG_NXT_FRAME),
	(void *) readl(axipv_base+REG_CTRL),
	(void *) readl(axipv_base+REG_AXIPV_STATUS),
	(void *) readl(axipv_base+REG_INTR_STAT),
	(void *) readl(axipv_base+REG_LB_WPTR),
	(void *) readl(axipv_base+REG_LB_RPTR));
	axipv_clk_disable(dev);
}

void axipv_release_pixdfifo_ownership(struct axipv_config_t *config)
{
	u32 axipv_base;
	struct axipv_dev *dev;
	int cnt = 10;
	uint32_t ctrl;

	if (!config) {
		axipv_err("config NULL\n");
		goto done;
	}

	dev = container_of(config, struct axipv_dev, config);
	axipv_base = dev->base_addr;

	if (!config->cmd || !config->bypassPV) {
		axipv_debug("No explicit access required\n");
		goto done;
	}

	axipv_clk_enable(dev);
	while ((AXIPV_STOPPED != dev->state) && cnt--) {
		axipv_err("PIXD Fifo busy, wait\n");
		udelay(1000);
	}
	if (!cnt) {
		ctrl = readl(axipv_base + REG_CTRL);
		if (ctrl & AXIPV_ACTIVE) {
			axipv_err("failed to get access\n");
			goto err_active;
		}
	}

	ctrl = readl(axipv_base + REG_CTRL);
	ctrl = ctrl & ~(1 << 8);
	ctrl = ctrl & ~(1 << 29);
	writel(ctrl, axipv_base + REG_CTRL);

err_active:
	axipv_clk_disable(dev);
done:
	return;
}

int axipv_change_state(u32 event, struct axipv_config_t *config)
{
	u32 ret = 0, axipv_base, cnt;
	struct axipv_dev *dev;

	axipv_debug("event=%d\n", event);
	if (!config)
		return -EINVAL;

	dev = container_of(config, struct axipv_dev, config);
	axipv_base = dev->base_addr;

	switch (event) {
	case AXIPV_CONFIG:
		cnt = 10;
		while (cnt-- && (AXIPV_STOPPING == dev->state))
			usleep_range(1000, 1100);
		if ((AXIPV_INIT_DONE == dev->state)
			|| (AXIPV_STOPPED == dev->state)) {
			axipv_clk_enable(dev);
			ret = axipv_config(config);
			axipv_clk_disable(dev);
			dev->state = AXIPV_CONFIGURED;
			ret = 0;
		} else {
			axipv_err("Frame transfer in progress\n");
			ret = -EBUSY;
		}
		break;
	case AXIPV_START:
		if ((AXIPV_CONFIGURED == dev->state)
			|| (AXIPV_STOPPED == dev->state)) {
			unsigned long flags, intr_en;

			axipv_clk_enable(dev);
#if defined(CONFIG_MACH_BCM_FPGA) || defined(CONFIG_MACH_BCM_FPGA_E)
			do_gettimeofday(&prof_tv1);
#endif
			spin_lock_irqsave(&lock, flags);
			intr_en = UINT_MAX;
			if (config->cmd) {
				writel_relaxed(readl(axipv_base + REG_CTRL) |
				AXIPV_SINGLE_SHOT, axipv_base + REG_CTRL);
			}
			writel_relaxed(intr_en, axipv_base + REG_INTR_EN);
			writel(readl(axipv_base + REG_CTRL) | AXIPV_EN,
					axipv_base + REG_CTRL);

			if (!config->cmd) {
				dev->state = AXIPV_ENABLED;
			} else {
				writel(readl(axipv_base + REG_CTRL) & ~AXIPV_EN,
					axipv_base + REG_CTRL);
				dev->state = AXIPV_STOPPING;
			}
			spin_unlock_irqrestore(&lock, flags);
			ret = 0;
		} else {
			ret = -EBUSY;
		}
		break;
	case AXIPV_STOP_EOF:
		if (AXIPV_ENABLED == dev->state) {
			unsigned long flags;
			spin_lock_irqsave(&lock, flags);
			writel(readl(axipv_base + REG_CTRL) & ~AXIPV_EN,
					axipv_base + REG_CTRL);
			dev->state = AXIPV_STOPPING;
			spin_unlock_irqrestore(&lock, flags);
			ret = 0;
		} else {
			ret = -EBUSY;
		}
		break;
	case AXIPV_STOP_IMM:
		dump_debug_info(dev);
		if ((AXIPV_ENABLED == dev->state)
			|| (AXIPV_STOPPING == dev->state)) {
			u32 ctrl = 0, count = 0;
			unsigned long flags;
			ctrl &= SFT_RSTN_MASK;
			spin_lock_irqsave(&lock, flags);
			/* Do an immediate soft reset and wait for completion */
			writel(ctrl, dev->base_addr + REG_CTRL);
			while ((!(readl(dev->base_addr + REG_CTRL)
				& SFT_RSTN_DONE)) && (count < 5)) {
				udelay(1);
				count++;
			}
			if (count == 5)
				axipv_err("Soft reset failed\n");
			dev->state = AXIPV_STOPPED;
			/* Set all buffers to transferred state */
			if (g_curr) {
				axipv_err("0x%x marked as available\n", g_curr);
				axipv_set_buff_status(dev->buff, g_curr,
						AXIPV_BUFF_AVAILABLE);
				dev->release_cb(g_curr);
			}
			if (g_nxt && (g_nxt != g_curr)) {
				axipv_err("0x%x marked as available\n", g_nxt);
				axipv_set_buff_status(dev->buff, g_nxt,
						AXIPV_BUFF_AVAILABLE);
				dev->release_cb(g_nxt);
			}
			g_curr = 0;
			g_nxt = 0;
			dump_debug_info(dev);
			axipv_dump_buff_status(dev->buff);
			axipv_clk_disable(dev);
			spin_unlock_irqrestore(&lock, flags);
			ret = 0;
		} else {
			axipv_err("AXIPV_STOP_IMM ignored, curr_state=%d\n",
				dev->state);
			ret = -EBUSY;
		}
		break;
	case AXIPV_RESET:
		/* Take it to reset state */
		break;
	case AXIPV_WAIT_INTR:
	{
		uint32_t ctrl, intr;
		unsigned long flags;
		axipv_clk_enable(dev);
		do {
			udelay(1);
			ctrl = readl(axipv_base + REG_CTRL);
			intr = readl(axipv_base + REG_INTR_STAT);
			intr &= PV_START_THRESH_INT;
			ctrl &= AXIPV_ACTIVE;
		/* Loop until ctrl=0 or any of intr=1*/
		} while (ctrl && !intr);
		writel(intr, axipv_base + REG_INTR_CLR);

		if (!ctrl) {
			spin_lock_irqsave(&lock, flags);
			dev->state = AXIPV_STOPPED;
			if (g_curr)
				axipv_release_buff(g_curr);
			if (g_nxt && (g_nxt != g_curr))
				axipv_release_buff(g_nxt);
			g_curr = 0;
			g_nxt = 0;
			spin_unlock_irqrestore(&lock, flags);
			process_release(&dev->release_work);
		}
		axipv_clk_disable(dev);
		break;
	}
	default:
		axipv_err("Invalid event:%d\n", event);
		break;
	}
	return ret;
}


int axipv_get_state(struct axipv_config_t *config)
{
	struct axipv_dev *dev;

	if (!config)
		return -EINVAL;

	dev = container_of(config, struct axipv_dev, config);

	return dev->state;
}

int axipv_check_completion(u32 event, struct axipv_config_t *config)
{
	/*
	 * 1. Interrupt is disabled on the core which is supposed to
	 *	service axipv_isr => Read INT_STATUS and ack
	 * 2. Workqueue doesn't run. process_release and if
	 *	dev->irq_stat is valid, then process_irq
	 * 3. Unknown case -> Halt AXIPV immediately
	 */
	u32 ret = -1, axipv_base;
	struct axipv_dev *dev;
	unsigned long flags;
	u32 int_stat, int_en;
	u32 pcsr[CONFIG_NR_CPUS];
	int i;

	axipv_debug("event=%d\n", event);
	if (!config)
		return -EINVAL;

	dev = container_of(config, struct axipv_dev, config);
	axipv_base = dev->base_addr;

	spin_lock_irqsave(&lock, flags);
	int_stat = readl(axipv_base + REG_INTR_STAT);
	int_en = readl(axipv_base + REG_INTR_EN);

#ifdef CONFIG_ARCH_HAWAII
#define CPU0_BASE HW_IO_PHYS_TO_VIRT(A9CPU0_BASE_ADDR)
#define CPU1_OFFSET (A9CPU1_BASE_ADDR - A9CPU0_BASE_ADDR)
#elif defined(CONFIG_ARCH_JAVA)
#define CPU0_BASE HW_IO_PHYS_TO_VIRT(A7CPU0_BASE_ADDR)
#define CPU1_OFFSET (A7CPU1_BASE_ADDR - A7CPU0_BASE_ADDR)
#endif
	for (i = 0; i < CONFIG_NR_CPUS; i++)
		pcsr[i] = readl(CPU0_BASE + i * CPU1_OFFSET);

	switch (event) {
	case AXIPV_START:
		if (int_stat & PV_START_THRESH_INT) {
			axipv_err("ISR was pending stat 0x%x en 0x%x\n",
			int_stat, int_en);
			writel(PV_START_THRESH_INT, axipv_base + REG_INTR_CLR);
			ret = 0;
		} else if (dev->irq_stat & PV_START_THRESH_INT) {
			axipv_err("WQ was pending 0x%x\n", dev->irq_stat);
			dev->irq_stat &= (~PV_START_THRESH_INT);
			ret = 0;
		} else {
			axipv_err("%d wasn't triggered\n", PV_START_THRESH_INT);
			ret = -1;
		}
		break;
	case AXIPV_STOP_EOF:
		if (int_stat & FRAME_END_INT) {
			axipv_err("ISR was pending stat 0x%x en 0x%x\n",
			int_stat, int_en);
			writel(FRAME_END_INT, axipv_base + REG_INTR_CLR);
			ret = 0;
		} else if (dev->irq_stat & FRAME_END_INT) {
			axipv_err("WQ was pending 0x%x\n", dev->irq_stat);
			dev->irq_stat &= (~FRAME_END_INT);
			ret = 0;
		} else {
			axipv_err("%d wan't triggered\n", FRAME_END_INT);
			ret = -1;
		}
		if (0 == ret) {
			u32 curr_buff = readl(axipv_base + REG_CUR_FRAME);
			axipv_err("marking 0x%x as available\n", curr_buff);
			axipv_set_buff_status(dev->buff, curr_buff,
						AXIPV_BUFF_AVAILABLE);

			if (curr_buff == g_curr)
				g_curr = 0;
			if (curr_buff == g_nxt)
				g_nxt = 0;
		}
		break;
	default:
		axipv_err("%d unsupported\n", event);
		break;
	}
	spin_unlock_irqrestore(&lock, flags);
	for (i = 0; i < CONFIG_NR_CPUS; i++)
		axipv_err("PCSR[%d] = 0x%x\n", i, pcsr[i]);
	return ret;
}

/***************************** DEBUG API **************************************/
/* These APIs are to be used to enable/disable the clock while debugging ONLY */
void enable_axipv_clk_debug(struct axipv_config_t *config)
{
	u32 axipv_base;
	struct axipv_dev *dev;

	if (!config) {
		axipv_err("config NULL\n");
		return;
	}

	dev = container_of(config, struct axipv_dev, config);
	axipv_base = dev->base_addr;

	axipv_clk_enable(dev);
}

void disable_axipv_clk_debug(struct axipv_config_t *config)
{
	u32 axipv_base;
	struct axipv_dev *dev;

	if (!config) {
		axipv_err("config NULL\n");
		return;
	}

	dev = container_of(config, struct axipv_dev, config);
	axipv_base = dev->base_addr;

	axipv_clk_disable(dev);
}
/***************************** DEBUG API **************************************/

