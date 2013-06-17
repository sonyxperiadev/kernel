/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2010  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>

#include <asm/hardware/pl330.h>
#include <plat/pl330-pdata.h>
#include <mach/dma.h>
#include <plat/dmux.h>
#include <mach/clock.h>

#ifdef CONFIG_KONA_PI_MGR
#include <plat/pi_mgr.h>
#endif

/**
 * struct pl330_chan_desc - Peripheral channel descriptor.
 */
struct pl330_chan_desc {
	int id;			/* channel ID for the client */
	struct list_head node;	/* Link to next channel desc */
	bool is_peri_mapped;	/*Is peripheral mapped?, false if mem to mem */
	char name[MAX_CHAN_NAME_LENGTH];	/* Name of the peripheral */
	int event_id;		/* ID of event/Interrupt line to notify */
	u8 peri_req_id;		/* mapped peripheral request interface(PRI) ID */
	void *pl330_chan_id;	/* PL330 channel id alloted */
	struct pl330_reqcfg rqcfg;	/* DMA req configurations */
	pl330_xfer_callback_t xfer_callback;	/* DMA callback function */
	void *client_cookie;	/* client data for callback fn */
	bool in_use;		/* is DMA channel busy */
	bool is_setup;		/* Is 'pl330_req' having valid transfer setup */
	struct pl330_req req;	/* A DMA request item */
#ifdef CONFIG_KONA_PI_MGR
	struct pi_mgr_dfs_node dfs_node;	/* dfs node for DMA */
#endif
};

/**
 * struct pl330_dmac_desc - PL330 DMAC Descriptor.
 */
struct pl330_dmac_desc {
	struct pl330_info *pi;	/*  PL330 DMAC info */
	int irq_start;		/* First PL330 Irq mapped */
	int irq_end;		/* Last Irq number mapped */
	struct list_head chan_list;	/* List of channel descriptors */
	int chan_count;		/* channel descriptors count */
	struct clk *clk;	/* clock struct for DMAC */
};

#ifdef CONFIG_KONA_PI_MGR
/* Array storing the names all the channels */
char *dma_chan[9] = { "dma_chan_0", "dma_chan_1", "dma_chan_2", "dma_chan_3",
	"dma_chan_4", "dma_chan_5", "dma_chan_6", "dma_chan_7", "dma_chan_8"
};
#endif

/* PL330 DMAC Descriptor structure */
static struct pl330_dmac_desc *dmac = NULL;	/* Allocate on platform device probe */
/* global resources lock */
static DEFINE_SPINLOCK(lock);

/* always call this function with global spinlock held */
static struct pl330_chan_desc *chan_id_to_cdesc(int id)
{
	struct pl330_chan_desc *cdesc;

	list_for_each_entry(cdesc, &dmac->chan_list, node)
	    if (cdesc->id == id)
		return cdesc;

	return NULL;
}

static void _cleanup_req(struct pl330_req *rq)
{
	struct pl330_xfer *x, *nxt;

	if (!rq)
		return;

	rq->rqtype = DEVTODEV;	/* Invalid type */

	if (rq->cfg) {
		kfree(rq->cfg);
		rq->cfg = NULL;
	}

	if (!rq->x)
		return;

	/* Free all the xfer items */
	x = rq->x;
	do {
		nxt = x->next;
		kfree(x);
		x = nxt;
	} while (x);
	rq->x = NULL;

	return;
}

static void _free_cdesc(struct pl330_chan_desc *cdesc)
{
	/* Deallocate all mapped peripherals */
	if (cdesc->is_peri_mapped) {
		dmux_dealloc_peripheral(cdesc->id);
		cdesc->is_peri_mapped = false;
	}
	/* Release the DMUX channel */
	dmux_release_channel(cdesc->id);

	/* Release PL330 channel thread */
	pl330_release_channel(cdesc->pl330_chan_id);

	list_del(&cdesc->node);
	dmac->chan_count--;
	kfree(cdesc);
}

static void pl330_req_callback(void *token, enum pl330_op_err err)
{
	struct pl330_req *r = token;
	enum pl330_xfer_status stat;
	struct pl330_chan_desc *c =
	    container_of(r, struct pl330_chan_desc, req);

	//printk("--> pl330 drv callback\n");
	if (c && c->xfer_callback) {
		switch (err) {
		case PL330_ERR_NONE:
			stat = DMA_PL330_XFER_OK;
			break;
		case PL330_ERR_ABORT:
			stat = DMA_PL330_XFER_ABORT;
			break;
		case PL330_ERR_FAIL:
			stat = DMA_PL330_XFER_ERR;
			break;
		default:
			stat = DMA_PL330_XFER_OK;
			break;
		}
		/* call client callback function */
		c->xfer_callback(c->client_cookie, stat);
	}
}

/* Function that allows dynamic change of frequency for a dma channel on
 * request by the user
 * Return - 0 on success and negative on failure*/
#ifdef CONFIG_KONA_PI_MGR
int dma_change_bus_speed(int chan, u32 opp)
{
	struct pl330_chan_desc *cdesc = NULL;
	int err = 0;

	/* Check if the mode of operation asked for is valid. DMA supports only
	 * normal and economy modes */
	if (opp != PI_OPP_ECONOMY && opp != PI_OPP_NORMAL) {
		err = -1;
		pr_err
		    ("%s : Error: invalid operation mode or mode not supported\n",
		     __func__);
		return err;
	}

	/* Get the channel descriptor for the requested channel number and check
	 * if its valid */
	cdesc = chan_id_to_cdesc(chan);
	if (cdesc == NULL) {
		err = -1;
		pr_err
		    ("%s : Error: invalid channel number. request not granted\n",
		     __func__);
		return err;
	}

	/* Request for a update on the bus speed */
	err = pi_mgr_dfs_request_update(&cdesc->dfs_node, opp);

	/* Check if the request was handled or not */
	if (err) {
		pr_err("%s : Error: could not change the bus speed\n",
		       __func__);
		return err;
	}

	return err;
}
#endif

int dma_request_chan(unsigned int *chan, const char *name)
{
	int ch, err = -1;
	enum dma_peri peri = KONA_DMACH_INVALID;
	u8 pri_id;
	void *pl330_chan_id = NULL;
	struct pl330_chan_desc *cdesc = NULL;
	unsigned long flags;
	bool is_peri = false;

	spin_lock_irqsave(&lock, flags);

	/* channel request for a 'named' peripheral, NULL if memory<->memory DMA */
	if (name) {
		/* get peripheral id from DMUX logic */
		if ((peri = dmux_name_to_id(name)) == KONA_DMACH_INVALID) {
			dev_info(dmac->pi->dev, "Unknown DMA peripheral:%s\n",
				 name);
			goto err_ret;
		}
		is_peri = true;
	}

	/* Allocate PL330 DMA channel thread first */
	pl330_chan_id = pl330_request_channel(dmac->pi);
	if (!pl330_chan_id) {
		dev_info(dmac->pi->dev,
			 "Failed to allocate PL330 channel thread!!!\n");
		goto err_1;
	}

	/*'Always' alloc dmux channel, peripherals can be attached later in case */
	if (dmux_alloc_channel(&ch) == -1) {
		dev_info(dmac->pi->dev, "Failed to allocate DMUX channel\n");
		goto err_2;
	}

	if (is_peri) {
		/* allocate the peripheral request channel */
		if (dmux_alloc_peripheral(ch, peri, &pri_id) == -1) {
			dev_info(dmac->pi->dev,
				 "Failed to allocate peripheral request, periph already taken!!!\n");
			goto err_3;
		}
	}
	spin_unlock_irqrestore(&lock, flags);

	/* Channel allocation is done, create a 'channel descriptor' for the client */
	cdesc = (struct pl330_chan_desc *)kzalloc(sizeof(*cdesc), GFP_KERNEL);

	spin_lock_irqsave(&lock, flags);
	if (!cdesc) {
		err = -ENOMEM;
		goto err_4;
	}

	/* Populate the cdesc and return channel id to client */
	cdesc->id = ch;
	cdesc->xfer_callback = NULL;
	cdesc->client_cookie = NULL;
	cdesc->is_peri_mapped = is_peri;
	cdesc->event_id = 0;	/* always use INTR/EVT line 0 */
	if (is_peri)
		cdesc->peri_req_id = pri_id;
	cdesc->pl330_chan_id = pl330_chan_id;
	cdesc->in_use = false;
	cdesc->req.rqtype = DEVTODEV;	/* set invalid type */
	if (name)
		strlcpy(cdesc->name, name, MAX_CHAN_NAME_LENGTH);

	/* Attach cdesc to DMAC channel list */
	list_add_tail(&cdesc->node, &dmac->chan_list);
	dmac->chan_count++;

#ifdef CONFIG_KONA_PI_MGR
	/* Add dfs request */
	err = pi_mgr_dfs_add_request(&cdesc->dfs_node, dma_chan[ch],
				     PI_MGR_PI_ID_ARM_SUB_SYSTEM,
				     PI_MGR_DFS_MIN_VALUE);

	if (err)
		goto err_4;
#endif

	spin_unlock_irqrestore(&lock, flags);

	/* Give the channel ID to client */
	*chan = cdesc->id;
	return 0;

      err_4:
	dmux_dealloc_peripheral(ch);
      err_3:
	dmux_release_channel(ch);
      err_2:
	pl330_release_channel(pl330_chan_id);
      err_1:
      err_ret:
	spin_unlock_irqrestore(&lock, flags);
	return err;
}

int dma_free_chan(unsigned int chan)
{
	unsigned long flags;
	struct pl330_chan_desc *cdesc;
	int ret;

	spin_lock_irqsave(&lock, flags);
	cdesc = chan_id_to_cdesc(chan);
	if (!cdesc || cdesc->in_use)	/* can free id channel is active */
		goto err;

#ifdef CONFIG_KONA_PI_MGR
	/* Remove dfs request */
	ret = pi_mgr_dfs_request_remove(&cdesc->dfs_node);
	if (ret)
		pr_debug("PL330: Failed to remove dfs node!\n");
#endif

	_cleanup_req(&cdesc->req);

	_free_cdesc(cdesc);

	spin_unlock_irqrestore(&lock, flags);

	return 0;
      err:
	spin_unlock_irqrestore(&lock, flags);
	return -1;
}

int dma_map_peripheral(unsigned int chan, const char *peri_name)
{
	struct pl330_chan_desc *c = NULL;
	unsigned long flags;
	enum dma_peri peri;
	u8 pri_id;

	if (!peri_name)
		return -1;

	spin_lock_irqsave(&lock, flags);

	c = chan_id_to_cdesc(chan);
	if (!c)
		goto err_ret;

	/*  If a peripheral is already mapped, return failure */
	if (c->is_peri_mapped) {
		dev_info(dmac->pi->dev,
			 "Already peripheral mapped, need to unmap first!!!\n");
		goto err_ret;
	}

	/* Get dmux channel id for 'named' peripheral */
	if ((peri = dmux_name_to_id(peri_name)) == KONA_DMACH_INVALID) {
		dev_info(dmac->pi->dev, "Unknown DMA peripheral:%s\n",
			 peri_name);
		goto err_ret;
	}

	/* Map the 'peripheral' to pre-allocated DMUX channel */
	if (dmux_alloc_peripheral(c->id, peri, &pri_id) == -1) {
		dev_info(dmac->pi->dev,
			 "Failed to alloc PRI, peripheral is already taken\n");
		goto err_ret;
	}

	c->peri_req_id = pri_id;
	c->is_peri_mapped = true;

	spin_unlock_irqrestore(&lock, flags);
	return 0;

      err_ret:
	spin_unlock_irqrestore(&lock, flags);
	return -1;
}

int dma_unmap_peripheral(unsigned int chan)
{
	struct pl330_chan_desc *c = NULL;
	unsigned long flags;

	spin_lock_irqsave(&lock, flags);

	c = chan_id_to_cdesc(chan);
	if (!c)
		goto err_ret;

	/* If a peripheral is already mapped or channel in use, return failure */
	if (!c->is_peri_mapped || c->in_use) {
		dev_info(dmac->pi->dev, "Peripheral is not mapped\n");
		goto err_ret;
	}

	dmux_dealloc_peripheral(c->id);
	c->is_peri_mapped = false;

	spin_unlock_irqrestore(&lock, flags);
	return 0;

      err_ret:
	spin_unlock_irqrestore(&lock, flags);
	return -1;
}

int dma_setup_transfer(unsigned int chan,
		       dma_addr_t src_addr,
		       dma_addr_t dst_addr,
		       unsigned int xfer_size, int control, int cfg)
{
	unsigned long flags;
	enum pl330_reqtype rqtype;
	struct pl330_reqcfg *config;
	struct pl330_xfer *xfer;
	struct pl330_chan_desc *c;
	int err = -EINVAL, bl, bs, w;

	if (!xfer_size)
		goto err;

	/* DMA transfer direction */
	switch (control & DMA_DIRECTION_MASK) {
		/* Peripheral DMA is handled with PRI handshake using DMAC flow control.
		 * Peripheral flow control is not supported at PL330 microcode level
		 * in low-level PL330 driver.
		 */
	case DMA_DIRECTION_MEM_TO_MEM:
		rqtype = MEMTOMEM;
		break;
	case DMA_DIRECTION_MEM_TO_DEV_FLOW_CTRL_DMAC:
	case DMA_DIRECTION_MEM_TO_DEV_FLOW_CTRL_PERI:	/*fall back to DMAC FCTRL */
		rqtype = MEMTODEV;
		break;
	case DMA_DIRECTION_DEV_TO_MEM_FLOW_CTRL_DMAC:
	case DMA_DIRECTION_DEV_TO_MEM_FLOW_CTRL_PERI:	/*fall back to DMAC FCTRL */
		rqtype = DEVTOMEM;
		break;
	case DMA_DIRECTION_DEV_TO_DEV:
	default:
		rqtype = DEVTODEV;	/*Unsupported */
		break;
	};

	if (rqtype == DEVTODEV)
		goto err;

	/* Burst size */
	bs = cfg & DMA_CFG_BURST_SIZE_MASK;

	if (bs > DMA_CFG_BURST_SIZE_8) {
		dev_err(dmac->pi->dev,
			"Burst Length > 64 bits not supported\n");
		goto err;
	}

	switch (bs) {
	case DMA_CFG_BURST_SIZE_1:
		w = 1;
		break;
	case DMA_CFG_BURST_SIZE_2:
		w = 2;
		break;
	case DMA_CFG_BURST_SIZE_4:
		w = 4;
		break;
	case DMA_CFG_BURST_SIZE_8:
	default:
		w = 8;
		break;
	};

	/* Burst Length */
	bl = (((cfg & DMA_CFG_BURST_LENGTH_MASK) >> DMA_CFG_BURST_LENGTH_SHIFT)
	      + 1);

	/* Check for bs*bl <= 64 bytes*/
	if ((w * bl) > 64) {
		dev_err(dmac->pi->dev,
			"total burst cannot be greater than 64 bytes\n");
		goto err;
	}

	/* checking xfer size alignment */
	if ((rqtype != MEMTOMEM) && (cfg & (DMA_PERI_END_SINGLE_REQ |
					    DMA_PERI_REQ_ALWAYS_BURST))) {
		if (xfer_size % w) {
			dev_err(dmac->pi->dev,
				"xfer size not aligned to burst size\n");
			goto err;
		}
	} else {
		if (xfer_size % (bl * w)) {
			dev_err(dmac->pi->dev,
				"xfer size not aligned to burst size x burst len\n");
			goto err;
		}
	}

	/* check buffer address alignment */
	if (src_addr % w) {
		dev_err(dmac->pi->dev,
			"src buffer is not aligned to brst size\n");
		goto err;
	}
	if (dst_addr % w) {
		dev_err(dmac->pi->dev,
			"dst buffer is not aligned to brst size\n");
		goto err;
	}

	spin_lock_irqsave(&lock, flags);

	/* Get channel descriptor */
	c = chan_id_to_cdesc(chan);
	if (!c) {
		spin_unlock_irqrestore(&lock, flags);
		goto err;
	}

	if (c->in_use || c->is_setup) {
		dev_info(dmac->pi->dev,
			 "Cant setup transfer, already setup/running\n");
		spin_unlock_irqrestore(&lock, flags);
		goto err1;
	};

	if ((rqtype != MEMTOMEM) && (!c->is_peri_mapped)) {
		spin_unlock_irqrestore(&lock, flags);
		goto err1;
	}

	c->is_setup = true;	/* Mark it now, for setup */

	spin_unlock_irqrestore(&lock, flags);

	/* Allocate */
	config = (struct pl330_reqcfg *)kzalloc(sizeof(*config), GFP_KERNEL);
	if (!config) {
		err = -ENOMEM;
		goto err1;
	}

	xfer = (struct pl330_xfer *)kzalloc(sizeof(*xfer), GFP_KERNEL);
	if (!xfer) {
		err = -ENOMEM;
		goto err2;
	}

	/* configuration options */
	config->src_inc = (cfg & DMA_CFG_SRC_ADDR_INCREMENT) ? 1 : 0;
	config->dst_inc = (cfg & DMA_CFG_DST_ADDR_INCREMENT) ? 1 : 0;
	config->peri_flush_start = (cfg & PERIPHERAL_FLUSHP_START) ? 1 : 0;
	config->peri_flush_end = (cfg & PERIPHERAL_FLUSHP_END) ? 1 : 0;
	config->always_burst = (cfg & DMA_PERI_REQ_ALWAYS_BURST) ? 1 : 0;
	config->end_single_req = (cfg & DMA_PERI_END_SINGLE_REQ) ? 1 : 0;

	/* Burst size */
	config->brst_size = bs >> DMA_CFG_BURST_SIZE_SHIFT;
	/* Burst Length */
	config->brst_len = bl;

	/* default settings:  Noncacheable, nonbufferable, no swapping */
	config->scctl = SCCTRL0;
	config->dcctl = DCCTRL0;
	config->swap = SWAP_NO;

	/* TrustZone Security level for DMA transactions: AXPROT[2:0] */
	config->insnaccess = false;
	config->privileged = false;
#ifdef CONFIG_DMAC_KONA_PL330_SECURE_MODE
	config->nonsecure = false;	/* Secure Mode */
#else
	config->nonsecure = true;	/* Open Mode */
#endif

	xfer->src_addr = src_addr;
	xfer->dst_addr = dst_addr;
	xfer->bytes = xfer_size;
	xfer->next = NULL;

	spin_lock_irqsave(&lock, flags);

	/* Attach the request */
	c->req.rqtype = rqtype;

	if (rqtype != MEMTOMEM)
		c->req.peri = c->peri_req_id;

	/* callback function */
	c->req.xfer_cb = pl330_req_callback;
	c->req.token = &c->req;	/* callback data */
	/* attach configuration */
	c->req.cfg = config;
	/* attach xfer item */
	c->req.x = xfer;

	spin_unlock_irqrestore(&lock, flags);
	return 0;

      err2:
	kfree(config);
      err1:
	c->is_setup = false;
      err:
	return err;
}

int dma_setup_transfer_sg(unsigned int chan,
			  struct scatterlist *sgl,
			  unsigned int sg_len,
			  dma_addr_t hw_addr, int control, int cfg)
{
	unsigned long flags;
	enum pl330_reqtype rqtype;
	struct pl330_reqcfg *config;
	struct pl330_xfer *xfer_front, *nxt = NULL, *priv = NULL;
	struct pl330_chan_desc *c;
	struct scatterlist *sg;
	int err = -EINVAL, i, bl, bs, w;

	if (unlikely(!sgl || !sg_len) || (control == DMA_DIRECTION_MEM_TO_MEM))
		goto err;

	/* DMA transfer direction */
	switch (control & DMA_DIRECTION_MASK) {
		/* Peripheral transfers are always handled with DMAC flow control.
		 * Peripheral flow control is not supported at PL330 microcode level
		 * in low-level PL330 driver.
		 */
	case DMA_DIRECTION_MEM_TO_DEV_FLOW_CTRL_DMAC:
	case DMA_DIRECTION_MEM_TO_DEV_FLOW_CTRL_PERI:	/*fall back to DMAC FCTRL */
		rqtype = MEMTODEV;
		break;
	case DMA_DIRECTION_DEV_TO_MEM_FLOW_CTRL_DMAC:
	case DMA_DIRECTION_DEV_TO_MEM_FLOW_CTRL_PERI:	/*fall back to DMAC FCTRL */
		rqtype = DEVTOMEM;
		break;
	case DMA_DIRECTION_DEV_TO_DEV:
	default:
		rqtype = DEVTODEV;	/*Unsupported */
		break;
	};

	if (rqtype == DEVTODEV)
		goto err;

	/* Burst size */
	bs = cfg & DMA_CFG_BURST_SIZE_MASK;

	if (bs > DMA_CFG_BURST_SIZE_8) {
		dev_err(dmac->pi->dev,
			"Burst Length > 64 bits not supported\n");
		goto err;
	}

	switch (bs) {
	case DMA_CFG_BURST_SIZE_1:
		w = 1;
		break;
	case DMA_CFG_BURST_SIZE_2:
		w = 2;
		break;
	case DMA_CFG_BURST_SIZE_4:
		w = 4;
		break;
	case DMA_CFG_BURST_SIZE_8:
	default:
		w = 8;
		break;
	};

	/* Burst Length */
	bl = (((cfg & DMA_CFG_BURST_LENGTH_MASK) >> DMA_CFG_BURST_LENGTH_SHIFT)
	      + 1);

	/* Check for bs*bl <= 64 bytes*/
	if ((w * bl) > 64) {
		dev_err(dmac->pi->dev,
			"total burst cannot be greater than 64 bytes\n");
		goto err;
	}

	/* check buffer address alignment */
	if (hw_addr % w) {
		dev_err(dmac->pi->dev,
			"hw address is not aligned to brst size\n");
		goto err;
	}

	spin_lock_irqsave(&lock, flags);

	/* Get channel descriptor */
	c = chan_id_to_cdesc(chan);
	if (!c) {
		spin_unlock_irqrestore(&lock, flags);
		goto err;
	}

	if (c->in_use || c->is_setup) {
		dev_info(dmac->pi->dev,
			 "Cant setup transfer, already setup/running\n");
		spin_unlock_irqrestore(&lock, flags);
		goto err1;
	};

	if ((rqtype != MEMTOMEM) && (!c->is_peri_mapped)) {
		spin_unlock_irqrestore(&lock, flags);
		goto err1;
	}

	c->is_setup = true;	/* Mark it now, for setup */

	spin_unlock_irqrestore(&lock, flags);

	/* Allocate  config strcuture */
	config = (struct pl330_reqcfg *)kzalloc(sizeof(*config), GFP_KERNEL);
	if (!config) {
		err = -ENOMEM;
		goto err1;
	}

	/* configuration options */
	config->src_inc = (cfg & DMA_CFG_SRC_ADDR_INCREMENT) ? 1 : 0;
	config->dst_inc = (cfg & DMA_CFG_DST_ADDR_INCREMENT) ? 1 : 0;
	config->peri_flush_start = (cfg & PERIPHERAL_FLUSHP_START) ? 1 : 0;
	config->peri_flush_end = (cfg & PERIPHERAL_FLUSHP_END) ? 1 : 0;
	config->always_burst = (cfg & DMA_PERI_REQ_ALWAYS_BURST) ? 1 : 0;
	config->end_single_req = (cfg & DMA_PERI_END_SINGLE_REQ) ? 1 : 0;

	/* Burst size */
	config->brst_size = bs >> DMA_CFG_BURST_SIZE_SHIFT;
	/* Burst Length */
	config->brst_len = bl;

	/* default settings:  Noncacheable, nonbufferable, no swapping */
	config->scctl = SCCTRL0;
	config->dcctl = DCCTRL0;
	config->swap = SWAP_NO;

	/* TrustZone Security level for DMA transactions: AXPROT[2:0] */
	config->insnaccess = false;
	config->privileged = false;
#ifdef CONFIG_DMAC_KONA_PL330_SECURE_MODE
	config->nonsecure = false;	/* Secure Mode */
#else
	config->nonsecure = true;	/* Open Mode */
#endif

	/* Generate xfer list based on scatterlist passed */
	xfer_front = NULL;

	for_each_sg(sgl, sg, sg_len, i) {

		if (!sg_dma_len(sg))
			continue;

		/* checking xfer size alignment */
		if ((rqtype != MEMTOMEM) && (cfg &
			(DMA_PERI_END_SINGLE_REQ |
		    DMA_PERI_REQ_ALWAYS_BURST))) {
			if (sg_dma_len(sg) % w) {
				dev_err(dmac->pi->dev,
					"LLI xfer size not aligned to burst size\n");
				goto err2;
			}
		} else {
			if (sg_dma_len(sg) % (bl * w)) {
				dev_err(dmac->pi->dev,
					"LLI xfer size not aligned to burst size x burst len\n");
				goto err2;
			}
		}

		/* check buffer address alignment */
		if (sg_dma_address(sg) % w) {
			dev_err(dmac->pi->dev,
				"sg buffer address not aligned to brst size\n");
			goto err2;
		}

		nxt = (struct pl330_xfer *)kzalloc(sizeof(*nxt), GFP_KERNEL);
		if (!nxt) {
			err = -ENOMEM;
			goto err2;
		}

		if (rqtype == MEMTODEV) {
			nxt->src_addr = sg_dma_address(sg);
			nxt->dst_addr = hw_addr;
		} else {
			nxt->src_addr = hw_addr;
			nxt->dst_addr = sg_dma_address(sg);
		}

		nxt->bytes = sg_dma_len(sg);
		nxt->next = NULL;

		if (!xfer_front) {
			xfer_front = nxt;	/* This is the first Item */
			priv = xfer_front;	/* On next iteration, attach to here  */
		} else {
			priv->next = nxt;	/* Add to the tail */
			priv = priv->next;	/* On next iteration, attach to here  */
		}

		/* nxt is added to xfer list, make it NULL before next LLI */
		nxt = NULL;
	}

	spin_lock_irqsave(&lock, flags);

	/* Attach the request */
	c->req.rqtype = rqtype;

	if (rqtype != MEMTOMEM)
		c->req.peri = c->peri_req_id;

	/* callback function */
	c->req.xfer_cb = pl330_req_callback;
	c->req.token = &c->req;	/* callback data */
	/* attach configuration */
	c->req.cfg = config;
	/* attach xfer item list */
	c->req.x = xfer_front;

	spin_unlock_irqrestore(&lock, flags);
	return 0;

      err2:
	/* Free all allocated xfer items */
	if (xfer_front) {
		priv = xfer_front;
		do {
			/* Get nxt item */
			nxt = priv->next;
			/* free current item */
			kfree(priv);
			/* load nxt item to current */
			priv = nxt;
		} while (priv != NULL);
	}
	/* Free config struct */
	kfree(config);
      err1:
	c->is_setup = false;
      err:
	return err;
}

int dma_setup_transfer_list(unsigned int chan, struct list_head *head,
			    int control, int cfg)
{
	unsigned long flags;
	enum pl330_reqtype rqtype;
	struct pl330_reqcfg *config;
	struct pl330_xfer *xfer_front, *nxt = NULL, *priv = NULL;
	struct pl330_chan_desc *c;
	struct dma_transfer_list *lli;
	int err = -EINVAL, bl, bs, w;

	if (!head)
		goto err;

	/* DMA transfer direction */
	switch (control & DMA_DIRECTION_MASK) {
		/* Peripheral transfers are always handled with DMAC flow control.
		 * Peripheral flow control is not supported at PL330 microcode level
		 * in low-level PL330 driver.
		 */
	case DMA_DIRECTION_MEM_TO_MEM:
		rqtype = MEMTOMEM;
		break;
	case DMA_DIRECTION_MEM_TO_DEV_FLOW_CTRL_DMAC:
	case DMA_DIRECTION_MEM_TO_DEV_FLOW_CTRL_PERI:	/*fall back to DMAC FCTRL */
		rqtype = MEMTODEV;
		break;
	case DMA_DIRECTION_DEV_TO_MEM_FLOW_CTRL_DMAC:
	case DMA_DIRECTION_DEV_TO_MEM_FLOW_CTRL_PERI:	/*fall back to DMAC FCTRL */
		rqtype = DEVTOMEM;
		break;
	case DMA_DIRECTION_DEV_TO_DEV:
	default:
		rqtype = DEVTODEV;	/*Unsupported */
		break;
	};

	if (rqtype == DEVTODEV)
		goto err;

	/* Burst size */
	bs = cfg & DMA_CFG_BURST_SIZE_MASK;

	if (bs > DMA_CFG_BURST_SIZE_8) {
		dev_err(dmac->pi->dev,
			"Burst Length > 64 bits not supported\n");
		goto err;
	}

	switch (bs) {
	case DMA_CFG_BURST_SIZE_1:
		w = 1;
		break;
	case DMA_CFG_BURST_SIZE_2:
		w = 2;
		break;
	case DMA_CFG_BURST_SIZE_4:
		w = 4;
		break;
	case DMA_CFG_BURST_SIZE_8:
	default:
		w = 8;
		break;
	};

	/* Burst Length */
	bl = (((cfg & DMA_CFG_BURST_LENGTH_MASK) >> DMA_CFG_BURST_LENGTH_SHIFT)
	      + 1);

	/* Check for bs*bl <= 64 bytes*/
	if ((w * bl) > 64) {
		dev_err(dmac->pi->dev,
			"total burst cannot be greater than 64 bytes\n");
		goto err;
	}

	spin_lock_irqsave(&lock, flags);

	/* Get channel descriptor */
	c = chan_id_to_cdesc(chan);
	if (!c) {
		spin_unlock_irqrestore(&lock, flags);
		goto err;
	}

	if (c->in_use || c->is_setup) {
		dev_info(dmac->pi->dev,
			 "Cant setup transfer, already setup/running\n");
		spin_unlock_irqrestore(&lock, flags);
		goto err1;
	};

	if ((rqtype != MEMTOMEM) && (!c->is_peri_mapped)) {
		spin_unlock_irqrestore(&lock, flags);
		goto err1;
	}

	c->is_setup = true;	/* Mark it now, for setup */

	spin_unlock_irqrestore(&lock, flags);

	/* Allocate  config strcuture */
	config = (struct pl330_reqcfg *)kzalloc(sizeof(*config), GFP_KERNEL);
	if (!config) {
		err = -ENOMEM;
		goto err1;
	}

	/* configuration options */
	config->src_inc = (cfg & DMA_CFG_SRC_ADDR_INCREMENT) ? 1 : 0;
	config->dst_inc = (cfg & DMA_CFG_DST_ADDR_INCREMENT) ? 1 : 0;
	config->peri_flush_start = (cfg & PERIPHERAL_FLUSHP_START) ? 1 : 0;
	config->peri_flush_end = (cfg & PERIPHERAL_FLUSHP_END) ? 1 : 0;
	config->always_burst = (cfg & DMA_PERI_REQ_ALWAYS_BURST) ? 1 : 0;
	config->end_single_req = (cfg & DMA_PERI_END_SINGLE_REQ) ? 1 : 0;

	/* Burst size */
	config->brst_size = bs >> DMA_CFG_BURST_SIZE_SHIFT;
	/* Burst Length */
	config->brst_len = bl;

	/* default settings:  Noncacheable, nonbufferable, no swapping */
	config->scctl = SCCTRL0;
	config->dcctl = DCCTRL0;
	config->swap = SWAP_NO;

	/* TrustZone Security level for DMA transactions: AXPROT[2:0] */
	config->insnaccess = false;
	config->privileged = false;
#ifdef CONFIG_DMAC_KONA_PL330_SECURE_MODE
	config->nonsecure = false;	/* Secure Mode */
#else
	config->nonsecure = true;	/* Open Mode */
#endif

	/* Generate xfer list based on linked list passed */
	xfer_front = NULL;
	list_for_each_entry(lli, head, next) {

		if (!lli->xfer_size)
			continue;

		/* checking xfer size alignment */
		if ((rqtype != MEMTOMEM) &&
			(cfg & (DMA_PERI_END_SINGLE_REQ |
			DMA_PERI_REQ_ALWAYS_BURST))) {
			if (lli->xfer_size % w) {
				dev_err(dmac->pi->dev,
					"LLI xfer size not aligned to burst size\n");
				goto err2;
			}
		} else {
			if (lli->xfer_size % (bl * w)) {
				dev_err(dmac->pi->dev,
					"LLI xfer size not aligned to burst size x burst len\n");
				goto err2;
			}
		}

		/* check buffer address alignment */
		if (lli->srcaddr % w) {
			dev_err(dmac->pi->dev,
				"src buffer is not aligned to brst size\n");
			goto err2;
		}

		if (lli->dstaddr % w) {
			dev_err(dmac->pi->dev,
				"dst buffer is not aligned to brst size\n");
			goto err2;
		}

		nxt = (struct pl330_xfer *)kzalloc(sizeof(*nxt), GFP_KERNEL);
		if (!nxt) {
			err = -ENOMEM;
			goto err2;
		}
		nxt->src_addr = lli->srcaddr;
		nxt->dst_addr = lli->dstaddr;
		nxt->bytes = lli->xfer_size;
		nxt->next = NULL;

		if (!xfer_front) {
			xfer_front = nxt;	/* This is the first Item */
			priv = xfer_front;	/* On next iteration, attach to here  */
		} else {
			priv->next = nxt;	/* Add to the tail */
			priv = priv->next;	/* On next iteration, attach to here  */
		}

		/* nxt is added to xfer list, make it NULL before next LLI */
		nxt = NULL;
	}

	spin_lock_irqsave(&lock, flags);

	/* Attach the request */
	c->req.rqtype = rqtype;

	if (rqtype != MEMTOMEM)
		c->req.peri = c->peri_req_id;

	/* callback function */
	c->req.xfer_cb = pl330_req_callback;
	c->req.token = &c->req;	/* callback data */
	/* attach configuration */
	c->req.cfg = config;
	/* attach xfer item list */
	c->req.x = xfer_front;
	c->is_setup = true;	/* Mark the xfer item as valid */

	spin_unlock_irqrestore(&lock, flags);
	return 0;

      err2:
	/* Free all allocated xfer items */
	if (xfer_front) {
		priv = xfer_front;
		do {
			/* Get nxt item */
			nxt = priv->next;
			/* free current item */
			kfree(priv);
			/* load nxt item to current */
			priv = nxt;
		} while (priv != NULL);
	}
	/* Free config struct */
	kfree(config);
      err1:
	c->is_setup = false;
      err:
	return err;
}

int dma_setup_transfer_list_multi_sg(unsigned int chan,
				     struct list_head *head,
				     dma_addr_t hw_addr, int control, int cfg)
{
	unsigned long flags;
	enum pl330_reqtype rqtype;
	struct pl330_reqcfg *config;
	struct pl330_xfer *xfer_front, *nxt = NULL, *priv = NULL;
	struct pl330_chan_desc *c;
	struct dma_transfer_list_sg *lli;
	struct scatterlist *sg;
	int err = -EINVAL, i, bl, bs, w;

	if (!head)
		goto err;

	/* DMA transfer direction */
	switch (control & DMA_DIRECTION_MASK) {

	case DMA_DIRECTION_MEM_TO_MEM:
		rqtype = MEMTOMEM;
		break;
	case DMA_DIRECTION_MEM_TO_DEV_FLOW_CTRL_DMAC:
	case DMA_DIRECTION_MEM_TO_DEV_FLOW_CTRL_PERI:	/*fall back*/
		rqtype = MEMTODEV;
		break;
	case DMA_DIRECTION_DEV_TO_MEM_FLOW_CTRL_DMAC:
	case DMA_DIRECTION_DEV_TO_MEM_FLOW_CTRL_PERI:	/*fall back*/
		rqtype = DEVTOMEM;
		break;
	case DMA_DIRECTION_DEV_TO_DEV:
	default:
		rqtype = DEVTODEV;	/*Unsupported */
		break;
	};

	if (rqtype == DEVTODEV)
		goto err;

	/* Burst size */
	bs = cfg & DMA_CFG_BURST_SIZE_MASK;

	if (bs > DMA_CFG_BURST_SIZE_8) {
		dev_err(dmac->pi->dev,
			"Burst Length > 64 bits not supported\n");
		goto err;
	}

	switch (bs) {
	case DMA_CFG_BURST_SIZE_1:
		w = 1;
		break;
	case DMA_CFG_BURST_SIZE_2:
		w = 2;
		break;
	case DMA_CFG_BURST_SIZE_4:
		w = 4;
		break;
	case DMA_CFG_BURST_SIZE_8:
	default:
		w = 8;
		break;
	};

	/* Burst Length */
	bl = (((cfg & DMA_CFG_BURST_LENGTH_MASK) >> DMA_CFG_BURST_LENGTH_SHIFT)
	      + 1);

	/* Check for bs*bl <= 64 bytes*/
	if ((w * bl) > 64) {
		dev_err(dmac->pi->dev,
			"total burst cannot be greater than 64 bytes\n");
		goto err;
	}

	/* check buffer address alignment */
	if (hw_addr % w) {
		dev_err(dmac->pi->dev,
			"hw address is not aligned to brst size\n");
		goto err;
	}

	spin_lock_irqsave(&lock, flags);

	/* Get channel descriptor */
	c = chan_id_to_cdesc(chan);
	if (!c) {
		spin_unlock_irqrestore(&lock, flags);
		goto err;
	}

	if (c->in_use || c->is_setup) {
		dev_info(dmac->pi->dev,
			 "Cant setup transfer, already setup/running\n");
		spin_unlock_irqrestore(&lock, flags);
		goto err1;
	};

	if ((rqtype != MEMTOMEM) && (!c->is_peri_mapped)) {
		spin_unlock_irqrestore(&lock, flags);
		goto err1;
	}

	c->is_setup = true;	/* Mark it now, for setup */

	spin_unlock_irqrestore(&lock, flags);

	/* Allocate  config strcuture */
	config = kzalloc(sizeof(*config), GFP_KERNEL);
	if (!config) {
		err = -ENOMEM;
		goto err1;
	}

	/* configuration options */
	config->src_inc = (cfg & DMA_CFG_SRC_ADDR_INCREMENT) ? 1 : 0;
	config->dst_inc = (cfg & DMA_CFG_DST_ADDR_INCREMENT) ? 1 : 0;
	config->peri_flush_start = (cfg & PERIPHERAL_FLUSHP_START) ? 1 : 0;
	config->peri_flush_end = (cfg & PERIPHERAL_FLUSHP_END) ? 1 : 0;
	config->always_burst = (cfg & DMA_PERI_REQ_ALWAYS_BURST) ? 1 : 0;
	config->end_single_req = (cfg & DMA_PERI_END_SINGLE_REQ) ? 1 : 0;

	/* Burst size */
	config->brst_size = bs >> DMA_CFG_BURST_SIZE_SHIFT;
	/* Burst Length */
	config->brst_len = bl;

	/* default settings:  Noncacheable, nonbufferable, no swapping */
	config->scctl = SCCTRL0;
	config->dcctl = DCCTRL0;
	config->swap = SWAP_NO;

	/* TrustZone Security level for DMA transactions: AXPROT[2:0] */
	config->insnaccess = false;
	config->privileged = false;
#ifdef CONFIG_DMAC_KONA_PL330_SECURE_MODE
	config->nonsecure = false;	/* Secure Mode */
#else
	config->nonsecure = true;	/* Open Mode */
#endif

	/* Generate xfer list by parsing each sg list */
	xfer_front = NULL;
	list_for_each_entry(lli, head, next) {
		/* If sg list is invalid, skip this */
		if (unlikely(!lli->sgl || !lli->sg_len) ||
		    (control == DMA_DIRECTION_MEM_TO_MEM))
			continue;

		/* Parse thru the sg list */
		for_each_sg(lli->sgl, sg, lli->sg_len, i) {
			if (!sg_dma_len(sg))
				continue;

			/* checking xfer size alignment */
			if ((rqtype != MEMTOMEM) && (cfg &
			     (DMA_PERI_END_SINGLE_REQ |
			      DMA_PERI_REQ_ALWAYS_BURST))) {
				if (sg_dma_len(sg) % w) {
					dev_err(dmac->pi->dev,
						"LLI xfer size not aligned to burst size\n");
					goto err2;
				}
			} else {
				if (sg_dma_len(sg) % (bl * w)) {
					dev_err(dmac->pi->dev,
						"LLI xfer size not aligned to burst size x burst len\n");
					goto err2;
				}
			}

			/* check buffer address alignment */
			if (sg_dma_address(sg) % w) {
				dev_err(dmac->pi->dev,
					"sg buffer address not aligned to brst size\n");
				goto err2;
			}

			nxt = kzalloc(sizeof(*nxt), GFP_KERNEL);

			if (!nxt) {
				err = -ENOMEM;
				goto err2;
			}

			if (rqtype == MEMTODEV) {
				nxt->src_addr = sg_dma_address(sg);
				nxt->dst_addr = hw_addr;
			} else {
				nxt->src_addr = hw_addr;
				nxt->dst_addr = sg_dma_address(sg);
			}

			nxt->bytes = sg_dma_len(sg);
			nxt->next = NULL;

			if (!xfer_front) {
				xfer_front = nxt;	/* 1st Item */
				priv = xfer_front;
			} else {
				priv->next = nxt;
				priv = priv->next;
			}
			/* make nxt NULL before next cycle */
			nxt = NULL;
		}
	}

	spin_lock_irqsave(&lock, flags);

	/* Attach the request */
	c->req.rqtype = rqtype;

	if (rqtype != MEMTOMEM)
		c->req.peri = c->peri_req_id;

	/* callback function */
	c->req.xfer_cb = pl330_req_callback;
	c->req.token = &c->req;	/* callback data */
	/* attach configuration */
	c->req.cfg = config;
	/* attach xfer item list */
	c->req.x = xfer_front;
	c->is_setup = true;	/* Mark the xfer item as valid */

	spin_unlock_irqrestore(&lock, flags);
	return 0;

err2:
	/* Free all allocated xfer items */
	if (xfer_front) {
		priv = xfer_front;
		do {
			/* Get nxt item */
			nxt = priv->next;
			/* free current item */
			kfree(priv);
			/* load nxt item to current */
			priv = nxt;
		} while (priv != NULL);
	}
	/* Free config struct */
	kfree(config);
err1:
	c->is_setup = false;
err:
	return err;
}

int dma_start_transfer(unsigned int chan)
{
	unsigned long flags;
	struct pl330_chan_desc *c;
	int ret;

	spin_lock_irqsave(&lock, flags);

	c = chan_id_to_cdesc(chan);

	if (!c || !c->is_setup || c->in_use)
		goto err;

#ifdef CONFIG_KONA_PI_MGR
	/* Request for a update on the bus speed */
	ret = pi_mgr_dfs_request_update(&c->dfs_node, PI_OPP_ECONOMY);

	/* Check if the request was handled or not */
	if (ret) {
		pr_err("%s : Error: could not change the bus speed\n",
		       __func__);
		goto err;
	}
#endif

#ifndef CONFIG_MACH_BCM_FPGA
	/* Enable the clock before the transfer */
	ret = clk_enable(dmac->clk);
	if (ret)
		goto err1;
#endif

	if (pl330_submit_req(c->pl330_chan_id, &c->req) != 0)
		goto err2;

	/*
	 * Acquire DMUX semaphore while microcode loading
	 *
	 * This HW semaphore call is redundant as PL330 driver always
	 * serializes the microcode loading in an atomic conext.
	 *
	 */
	dmux_sema_protect();

	/* Start DMA channel thread */
	if (pl330_chan_ctrl(c->pl330_chan_id, PL330_OP_START) != 0) {
		goto err2;
	}

	c->in_use = true;
	dmux_sema_unprotect();
	spin_unlock_irqrestore(&lock, flags);

	return 0;
      err2:
	dmux_sema_unprotect();
#ifndef CONFIG_MACH_BCM_FPGA
	clk_disable(dmac->clk);
#endif
      err1:
#ifdef CONFIG_KONA_PI_MGR
	ret = pi_mgr_dfs_request_update(&c->dfs_node, PI_MGR_DFS_MIN_VALUE);
	/* Check if the request was handled or not */
	if (ret)
		pr_err("%s : Error: could not change the bus speed\n",
		       __func__);
#endif
      err:
	spin_unlock_irqrestore(&lock, flags);
	return -1;
}

int dma_stop_transfer(unsigned int chan)
{
	unsigned long flags;
	struct pl330_chan_desc *c;
	int ret;

	spin_lock_irqsave(&lock, flags);

	c = chan_id_to_cdesc(chan);
	if (!c)
		goto err;

	dmux_sema_protect();
	pl330_chan_ctrl(c->pl330_chan_id, PL330_OP_FLUSH);
	dmux_sema_unprotect();

	/* Free the completed transfer req */
	c->in_use = false;
	c->is_setup = false;
	/* free memory allocated for this request */
	_cleanup_req(&c->req);

#ifndef CONFIG_MACH_BCM_FPGA
	/* Disable clock after transfer */
	clk_disable(dmac->clk);
#endif

#ifdef CONFIG_KONA_PI_MGR
	/* Request for a update on the bus speed */
	ret = pi_mgr_dfs_request_update(&c->dfs_node, PI_MGR_DFS_MIN_VALUE);

	/* Check if the request was handled or not */
	if (ret) {
		pr_err("%s : Error: could not change the bus speed\n",
		       __func__);
		goto err;
	}
#endif
	spin_unlock_irqrestore(&lock, flags);
	return 0;

      err:
	spin_unlock_irqrestore(&lock, flags);
	return -1;
}

int dma_register_callback(unsigned int chan,
			  pl330_xfer_callback_t callback, void *private_data)
{
	unsigned long flags;
	struct pl330_chan_desc *c;

	spin_lock_irqsave(&lock, flags);

	c = chan_id_to_cdesc(chan);
	if (!c)
		goto err;

	c->xfer_callback = callback;
	c->client_cookie = private_data;

	spin_unlock_irqrestore(&lock, flags);
	return 0;

      err:
	spin_unlock_irqrestore(&lock, flags);
	return -1;
}

int dma_free_callback(unsigned int chan)
{
	unsigned long flags;
	struct pl330_chan_desc *c;

	spin_lock_irqsave(&lock, flags);

	c = chan_id_to_cdesc(chan);
	if (!c)
		goto err;

	c->xfer_callback = NULL;
	c->client_cookie = NULL;

	spin_unlock_irqrestore(&lock, flags);
	return 0;

      err:
	spin_unlock_irqrestore(&lock, flags);
	return -1;

}

static irqreturn_t pl330_irq_handler(int irq, void *data)
{
	if (pl330_update(data))
		return IRQ_HANDLED;
	else
		return IRQ_NONE;
}

static int pl330_probe(struct platform_device *pdev)
{
	struct pl330_dmac_desc *pd;
	struct kona_pl330_data *pl330_pdata;
	struct pl330_info *pl330_info;
	int ret, i, irq_start, irq;

	pl330_pdata = pdev->dev.platform_data;

	/* Only One Pl330 device is supported,
	 * since PL330 is closely bound to DMUX logic
	 */
	if (dmac) {
		dev_err(&pdev->dev, "Multiple devices are not supported!!!\n");
		ret = -ENODEV;
		goto probe_err1;
	}

	/* Platform data is required */
	if (!pl330_pdata) {
		dev_err(&pdev->dev, "platform data missing!\n");
		ret = -ENODEV;
		goto probe_err1;
	}

	if (dmux_init() != 0) {
		dev_err(&pdev->dev, "DMUX initialisation failed!\n");
		ret = -ENODEV;
		goto probe_err1;
	}

	pl330_info = kzalloc(sizeof(*pl330_info), GFP_KERNEL);
	if (!pl330_info) {
		ret = -ENOMEM;
		goto probe_err1;
	}

	pl330_info->pl330_data = NULL;
	pl330_info->client_data = NULL;
	pl330_info->dev = &pdev->dev;

	/* DMAC APB base address, for secure and open modes */
#ifdef CONFIG_DMAC_KONA_PL330_SECURE_MODE
	pl330_info->base = (void __iomem *)pl330_pdata->dmac_s_base;
#else
	pl330_info->base = (void __iomem *)pl330_pdata->dmac_ns_base;
#endif
	ret = pl330_add(pl330_info);
	if (ret)
		goto probe_err2;

	/*  Get the first IRQ line */
	irq_start = pl330_pdata->irq_base;
	irq = irq_start;

	for (i = 0; i < pl330_pdata->irq_line_count; i++) {
		irq = irq_start + i;
		ret = request_irq(irq, pl330_irq_handler, 0,
				  dev_name(&pdev->dev), pl330_info);
		if (ret) {
			irq--;
			goto probe_err3;
		}
	}

	/* Allocate DMAC descriptor */
	pd = kmalloc(sizeof(*pd), GFP_KERNEL);
	if (!pd) {
		ret = -ENOMEM;
		goto probe_err3;
	}

#ifndef CONFIG_MACH_BCM_FPGA
	/* Get the clock struct */
	pd->clk = clk_get(NULL, DMAC_MUX_APB_BUS_CLK_NAME_STR);
	if (IS_ERR_OR_NULL(pd->clk)) {
		ret = -ENOENT;
		goto probe_err4;
	}
#else
	pd->clk = NULL;
#endif

	/* Hook the info */
	pd->pi = pl330_info;
	pd->irq_start = irq_start;
	pd->irq_end = irq;
	/* init channel desc list, channels are added during dma_request_chan() */
	pd->chan_list.next = pd->chan_list.prev = &pd->chan_list;
	pd->chan_count = 0;

	/* Assign the DMAC descriptor */
	dmac = pd;

	printk(KERN_INFO
	       "Loaded driver for PL330 DMAC-%d %s\n", pdev->id, pdev->name);
	printk(KERN_INFO
	       "\tDBUFF-%ux%ubytes Num_Chans-%u Num_Peri-%u Num_Events-%u\n",
	       pl330_info->pcfg.data_buf_dep,
	       pl330_info->pcfg.data_bus_width / 8, pl330_info->pcfg.num_chan,
	       pl330_info->pcfg.num_peri, pl330_info->pcfg.num_events);

	return 0;

      probe_err4:
	kfree(pd);
      probe_err3:
	while (irq >= irq_start) {
		free_irq(irq, pl330_info);
		irq--;
	}
	pl330_del(pl330_info);

      probe_err2:
	kfree(pl330_info);
      probe_err1:
	return ret;
}

static int pl330_remove(struct platform_device *pdev)
{
	unsigned long flags;
	struct pl330_chan_desc *cdesc, *temp;
	int irq;

	spin_lock_irqsave(&lock, flags);
	/* Free all channel descriptors first */
	list_for_each_entry_safe(cdesc, temp, &dmac->chan_list, node) {
		/* free requests */
		_cleanup_req(&cdesc->req);
		/* Free channel desc */
		_free_cdesc(cdesc);
	}

	/* Free interrupt resource */
	for (irq = dmac->irq_start; irq <= dmac->irq_end; irq++)
		free_irq(irq, dmac->pi);

	pl330_del(dmac->pi);
	/* free PL330 info handle */
	kfree(dmac->pi);
	/* Free dmac descriptor */
	kfree(dmac);
	dmac = NULL;

	/* De-initialise DMUX */
	dmux_exit();

	spin_unlock_irqrestore(&lock, flags);

	return 0;
}

static struct platform_driver pl330_driver = {
	.driver = {
		   .owner = THIS_MODULE,
		   .name = "kona-dmac-pl330",
		   },
	.probe = pl330_probe,
	.remove = pl330_remove,
};

static int __init pl330_init(void)
{
	return platform_driver_register(&pl330_driver);
}

arch_initcall(pl330_init);

static void __exit pl330_exit(void)
{
	platform_driver_unregister(&pl330_driver);
	return;
}

module_exit(pl330_exit);

EXPORT_SYMBOL(dma_request_chan);
EXPORT_SYMBOL(dma_free_chan);
EXPORT_SYMBOL(dma_map_peripheral);
EXPORT_SYMBOL(dma_unmap_peripheral);
EXPORT_SYMBOL(dma_setup_transfer);
EXPORT_SYMBOL(dma_setup_transfer_sg);
EXPORT_SYMBOL(dma_setup_transfer_list_multi_sg);
EXPORT_SYMBOL(dma_setup_transfer_list);
EXPORT_SYMBOL(dma_start_transfer);
EXPORT_SYMBOL(dma_stop_transfer);
EXPORT_SYMBOL(dma_register_callback);
EXPORT_SYMBOL(dma_free_callback);
#ifdef CONFIG_KONA_PI_MGR
EXPORT_SYMBOL(dma_change_bus_speed);
#endif
