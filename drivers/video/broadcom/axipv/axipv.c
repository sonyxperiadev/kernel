#include <plat/reg_axipv.h>
#include <plat/axipv.h>
#include <linux/delay.h>
#include <linux/err.h>

#ifdef AXIPV_HAS_CLK
#include <linux/clk.h>
#endif

#define axipv_info(fmt, args...) \
		printk(KERN_INFO"%s:%d " fmt, __func__, __LINE__, ##args)
#define axipv_debug(fmt, args...) \
		printk(KERN_DEBUG"%s:%d " fmt, __func__, __LINE__, ##args)
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

static inline u32 axipv_get_free_buff_index(struct axipv_buff *buff)
{
	int i;
	for (i = 0; i < AXIPV_MAX_DISP_BUFF_SUPP; i++) {
		if (AXIPV_BUFF_AVAILABLE == buff[i].status)
			break;
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

	for (i = 0; i < AXIPV_MAX_DISP_BUFF_SUPP; i++) {
		if (AXIPV_BUFF_TXFERED == buff[i].status) {
			dev->release_cb(buff[i].addr);
			err = axipv_set_buff_status(buff, buff[i].addr,
						AXIPV_BUFF_AVAILABLE);
			if (err)
				axipv_err("couldn't set buff status err=%d\n",
					err);
		}
	}
}


int axipv_init(struct axipv_init_t *init, struct axipv_config_t **config)
{
	int ret = 0, ctrl = 0, i;
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
		goto fail;
	}

#ifdef AXIPV_HAS_CLK
	dev->clk = clk_get(NULL, init->clk_name);
	if (IS_ERR(dev->clk)) {
		axipv_err("failed to get clk");
		ret = PTR_ERR(dev->clk);
		goto fail;
	}
	axipv_clk_enable(dev);
#endif

	ctrl &= SFT_RSTN_MASK;
	/* Do an immediate soft reset and wait for completion */
	writel(ctrl, init->base_addr + REG_CTRL);
	while (!(readl(init->base_addr + REG_CTRL) & SFT_RSTN_DONE)) {
		printk("udelay\n");
		udelay(5);
	}

	axipv_clk_disable(dev);
	dev->base_addr = init->base_addr;
	dev->irq_cb = init->irq_cb;
	dev->release_cb = init->release_cb;
	INIT_WORK(&dev->irq_work, process_irq);
	INIT_WORK(&dev->release_work, process_release);
	for (i = 0; i < AXIPV_MAX_DISP_BUFF_SUPP; i++)
		dev->buff[i].status = AXIPV_BUFF_AVAILABLE;
	dev->state = AXIPV_INIT_DONE;
	g_axipv_init = true;
	*config = &dev->config;
	ret = 0;
	goto done;

fail:
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

	if (!config)
		return -EINVAL;

	dev = container_of(config, struct axipv_dev, config);
	axipv_base = dev->base_addr;

	writel_relaxed(config->buff.sync.xlen, axipv_base + REG_BYTES_PER_LINE);
	writel_relaxed(config->buff.sync.ylen,
			axipv_base + REG_LINES_PER_FRAME);
	writel_relaxed(config->buff.sync.addr, axipv_base + REG_NXT_FRAME);
	g_curr = config->buff.sync.addr;
	buff_index = axipv_get_free_buff_index(dev->buff);
	if (buff_index >= AXIPV_MAX_DISP_BUFF_SUPP) {
		axipv_err("Couldn't get free buff index\n");
	} else {
		axipv_add_new_buff_info(dev->buff, buff_index,
					config->buff.sync.addr);
	}

	writel_relaxed(config->width, axipv_base + REG_LINE_STRIDE);
	writel_relaxed(AXIPV_BURST_LEN, axipv_base + REG_BURST_LEN);
	writel_relaxed(AXIPV_TE_LINE, axipv_base + REG_TE_LINE);
	writel_relaxed(0, axipv_base + REG_INTR_EN);
	writel_relaxed(UINT_MAX, axipv_base + REG_INTR_CLR);
	dev->prev_irq_handled = 1;
	dev->irq_stat = 0;

	writel_relaxed(AXIPV_AXI_ID1, axipv_base + REG_AXI_ID_CFG_1);
	writel_relaxed(AXIPV_AXI_ID2, axipv_base + REG_AXI_ID_CFG_2);

	ctrl =  SFT_RSTN_DONE | (config->cmd ? AXIPV_CMD_MODE : 0)
		| (config->pix_fmt << PIXEL_FORMAT_SHIFT) | NUM_OUTSTDG_XFERS_8
		| AXI_ID_SYS_DUAL | AXIPV_ARPROT | AXIPV_ARCACHE
		| (config->test ? AXIPV_TESTMODE : 0);
	writel(ctrl, axipv_base + REG_CTRL);

	tx_size = config->buff.sync.xlen * config->buff.sync.ylen;
	if ((PIXEL_FORMAT_24BPP_RGB == config->pix_fmt)
		|| (PIXEL_FORMAT_24BPP_BGR == config->pix_fmt))
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
	return 0;
}

#define axipv_release_buff(addr) \
	do { \
		u32 err = axipv_set_buff_status(dev->buff, addr,\
		AXIPV_BUFF_TXFERED); \
		if (!err)\
			schedule_work(&dev->release_work);\
		else\
			axipv_debug("failed to set buff status err=%d\n", err);\
	} while (0)

static irqreturn_t axipv_isr(int err, void *dev_id)
{
	struct axipv_dev *dev = dev_id;
	int irq_stat;
	u32 axipv_base;

	axipv_base = dev->base_addr;
	irq_stat = readl(axipv_base + REG_INTR_STAT);

	if (irq_stat & FRAME_END_INT) {
		/* Do not turn off clocks right here. Even in command mode!
		 * Reason: This will only indicate that the entire frame has
		 *         been fetched into AXIPV's LB. PV needs to pull in all
		 *         the data in AXIPV's LB before we turn off clocks */
		u32 curr_reg_val = readl(axipv_base + REG_CUR_FRAME);
		if ((g_curr != curr_reg_val) ||
			(AXIPV_STOPPING == dev->state)) {
			if (g_curr)
				axipv_release_buff(g_curr);
			g_curr = curr_reg_val;
			if (AXIPV_STOPPING == dev->state)
				dev->state = AXIPV_STOPPED;
		}
		writel(FRAME_END_INT, axipv_base + REG_INTR_CLR);
		irq_stat = irq_stat & ~FRAME_END_INT;
	}
	if (irq_stat & AXIPV_DISABLED_INT) {
		irq_stat |= AXIPV_DISABLED_INT;
		if (g_curr) {
			axipv_release_buff(g_curr);
			g_curr = 0;
		}
		if (g_nxt) {
			axipv_release_buff(g_nxt);
			g_nxt = 0;
		}
		dev->state = AXIPV_STOPPED;
		writel(AXIPV_DISABLED_INT, axipv_base + REG_INTR_CLR);
		axipv_clk_disable(dev);
		irq_stat = irq_stat & ~AXIPV_DISABLED_INT;
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

	writel(config->buff.async, axipv_base + REG_NXT_FRAME);
	curr_reg_val = readl(axipv_base + REG_CUR_FRAME);
	buff_index = axipv_get_free_buff_index(dev->buff);
	if (buff_index >= AXIPV_MAX_DISP_BUFF_SUPP) {
		axipv_err("Couldn't get free buff index\n");
	} else {
		axipv_add_new_buff_info(dev->buff, buff_index,
					config->buff.async);
	}
	if ((curr_reg_val == g_curr) || (curr_reg_val == config->buff.async)) {
		/* Skipped buffer has to be released*/
		if (g_nxt) {
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
/*	unsigned long flags; */

	dev = container_of(config, struct axipv_dev, config);
	axipv_base = dev->base_addr;

	if (config->cmd) {
		u32 tx_size;
		writel_relaxed(config->buff.sync.xlen,
			axipv_base + REG_BYTES_PER_LINE);
		writel_relaxed(config->buff.sync.ylen,
			axipv_base + REG_LINES_PER_FRAME);

		tx_size = config->buff.sync.xlen * config->buff.sync.ylen;
		if ((PIXEL_FORMAT_24BPP_RGB == config->pix_fmt)
			|| (PIXEL_FORMAT_24BPP_BGR == config->pix_fmt))
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

/*	spin_lock_irqsave(&lock, flags); */
	if (config->cmd)
		writel(readl(axipv_base + REG_CTRL) | AXIPV_EN,
				axipv_base + REG_CTRL);

	/*Raghu: update state to enabled within spinlock */

	if (g_curr) {
		/* Not required, but just to be sure.
		Use a semaphore instead and signal it in ISR */
		while (config->buff.sync.addr !=
			readl(axipv_base + REG_CUR_FRAME))
			udelay(1);
	}
	if (config->cmd) {
		writel(0, axipv_base + REG_NXT_FRAME);
		writel(readl(axipv_base  + REG_CTRL) & ~AXIPV_EN,
				axipv_base + REG_CTRL);
	}
/*	spin_lock_irqrestore(&lock, flags);*/
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


int axipv_change_state(u32 event, struct axipv_config_t *config)
{
	u32 ret = 0, axipv_base;
	struct axipv_dev *dev;

	if (!config)
		return -EINVAL;

	dev = container_of(config, struct axipv_dev, config);
	axipv_base = dev->base_addr;

	switch (event) {
	case AXIPV_CONFIG:
		while (AXIPV_STOPPING == dev->state)
			usleep_range(1000, 1100);
		if ((AXIPV_INIT_DONE == dev->state)
			|| (AXIPV_STOPPED == dev->state)) {
			axipv_clk_enable(dev);
			ret = axipv_config(config);
			axipv_clk_disable(dev);
			dev->state = AXIPV_CONFIGURED;
			ret = 0;
		} else {
			return -EBUSY;
		}
		break;
	case AXIPV_START:
		if ((AXIPV_CONFIGURED == dev->state)
			|| (AXIPV_STOPPED == dev->state)) {
			unsigned long flags;

#ifdef AXIPV_HAS_CLK
			axipv_clk_enable(dev);
#endif
			spin_lock_irqsave(&lock, flags);
			writel_relaxed(UINT_MAX, axipv_base + REG_INTR_EN);
			if (config->cmd) {
				writel_relaxed(readl(axipv_base + REG_CTRL) &
				AXIPV_SINGLE_SHOT, axipv_base + REG_CTRL);
			}
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
			return -EBUSY;
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
			return -EBUSY;
		}
		break;
	case AXIPV_STOP_IMM:
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
			dev->state = AXIPV_INVALID_STATE;
			axipv_clk_disable(dev);
			spin_unlock_irqrestore(&lock, flags);
			ret = 0;
		} else {
			return -EBUSY;
		}
		break;
	case AXIPV_RESET:
		/* Take it to reset state */
		break;
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
