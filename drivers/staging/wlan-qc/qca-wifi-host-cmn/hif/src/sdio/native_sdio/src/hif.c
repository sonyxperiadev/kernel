/*
 * Copyright (c) 2013-2020 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/mmc/card.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio_ids.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/sd.h>
#include <linux/version.h>
#include <linux/module.h>
#include <qdf_atomic.h>
#include <cds_utils.h>
#include <qdf_timer.h>
#include <cds_api.h>
#include <qdf_time.h>
#include "hif_sdio_dev.h"
#include "if_sdio.h"
#include "regtable_sdio.h"
#include "wma_api.h"
#include "hif_internal.h"
#include <transfer/transfer.h>

#define HIF_USE_DMA_BOUNCE_BUFFER 1
#define ATH_MODULE_NAME hif
#include "a_debug.h"

#define MAX_HIF_DEVICES 2
#ifdef HIF_MBOX_SLEEP_WAR
#define HIF_MIN_SLEEP_INACTIVITY_TIME_MS     50
#define HIF_SLEEP_DISABLE_UPDATE_DELAY 1
#define HIF_IS_WRITE_REQUEST_MBOX1_TO_3(request) \
				((request->request & HIF_SDIO_WRITE) && \
				(request->address >= 0x1000 && \
				request->address < 0x1FFFF))
#endif
unsigned int forcesleepmode;
module_param(forcesleepmode, uint, 0644);
MODULE_PARM_DESC(forcesleepmode,
		"Set sleep mode: 0-host capbility, 1-force WOW, 2-force DeepSleep, 3-force CutPower");

unsigned int forcecard;
module_param(forcecard, uint, 0644);
MODULE_PARM_DESC(forcecard,
		 "Ignore card capabilities information to switch bus mode");

unsigned int debugcccr = 1;
module_param(debugcccr, uint, 0644);
MODULE_PARM_DESC(debugcccr, "Output this cccr values");

#define dev_to_sdio_func(d)		container_of(d, struct sdio_func, dev)
#define to_sdio_driver(d)		container_of(d, struct sdio_driver, drv)
static struct hif_sdio_dev *add_hif_device(struct hif_softc *hif_ctx,
					   struct sdio_func *func);
static void del_hif_device(struct hif_sdio_dev *device);

int reset_sdio_on_unload;
module_param(reset_sdio_on_unload, int, 0644);

uint32_t nohifscattersupport = 1;

/* ------ Static Variables ------ */
static const struct sdio_device_id ar6k_id_table[] = {
#ifdef AR6002_HEADERS_DEF
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6002_BASE | 0x0))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6002_BASE | 0x1))},
#endif
#ifdef AR6003_HEADERS_DEF
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6003_BASE | 0x0))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6003_BASE | 0x1))},
#endif
#ifdef AR6004_HEADERS_DEF
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6004_BASE | 0x0))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6004_BASE | 0x1))},
#endif
#ifdef AR6320_HEADERS_DEF
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6320_BASE | 0x0))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6320_BASE | 0x1))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6320_BASE | 0x2))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6320_BASE | 0x3))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6320_BASE | 0x4))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6320_BASE | 0x5))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6320_BASE | 0x6))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6320_BASE | 0x7))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6320_BASE | 0x8))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6320_BASE | 0x9))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6320_BASE | 0xA))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6320_BASE | 0xB))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6320_BASE | 0xC))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6320_BASE | 0xD))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6320_BASE | 0xE))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6320_BASE | 0xF))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9377_BASE | 0x0))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9377_BASE | 0x1))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9377_BASE | 0x2))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9377_BASE | 0x3))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9377_BASE | 0x4))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9377_BASE | 0x5))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9377_BASE | 0x6))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9377_BASE | 0x7))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9377_BASE | 0x8))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9377_BASE | 0x9))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9377_BASE | 0xA))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9377_BASE | 0xB))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9377_BASE | 0xC))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9377_BASE | 0xD))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9377_BASE | 0xE))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9377_BASE | 0xF))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9379_BASE | 0x0))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9379_BASE | 0x1))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9379_BASE | 0x2))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9379_BASE | 0x3))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9379_BASE | 0x4))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9379_BASE | 0x5))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9379_BASE | 0x6))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9379_BASE | 0x7))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9379_BASE | 0x8))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9379_BASE | 0x9))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9379_BASE | 0xA))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9379_BASE | 0xB))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9379_BASE | 0xC))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9379_BASE | 0xD))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9379_BASE | 0xE))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9379_BASE | 0xF))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (0 | 0x0))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (0 | 0x1))},
#endif
	{ /* null */ },
};

struct hif_sdio_softc *scn;

static struct hif_sdio_dev *hif_devices[MAX_HIF_DEVICES];

#if defined(WLAN_DEBUG) || defined(DEBUG)
ATH_DEBUG_INSTANTIATE_MODULE_VAR(hif,
				 "hif",
				 "(Linux MMC) Host Interconnect Framework",
				 ATH_DEBUG_MASK_DEFAULTS, 0, NULL);
#endif

/**
 * add_to_async_list() - add bus reqest to async task list
 * @device: pointer to hif device
 * @busrequest: pointer to type of bus request
 *
 * Return: None.
 */
void add_to_async_list(struct hif_sdio_dev *device,
		      struct bus_request *busrequest)
{
	struct bus_request *async;
	struct bus_request *active;

	qdf_spin_lock_irqsave(&device->asynclock);
	active = device->asyncreq;
	if (!active) {
		device->asyncreq = busrequest;
		device->asyncreq->inusenext = NULL;
	} else {
		for (async = device->asyncreq;
		     async; async = async->inusenext) {
			active = async;
		}
		active->inusenext = busrequest;
		busrequest->inusenext = NULL;
	}
	qdf_spin_unlock_irqrestore(&device->asynclock);
}

/*
 * Setup IRQ mode for deep sleep and WoW
 * Switch back to 1 bits mode when we suspend for
 * WoW in order to detect SDIO irq without clock.
 * Re-enable async 4-bit irq mode for some host controllers
 * after resume.
 */
#ifdef CONFIG_SDIO_TRANSFER_MAILBOX
static int sdio_enable4bits(struct hif_sdio_dev *device, int enable)
{
	int ret = 0;
	struct sdio_func *func = device->func;
	struct mmc_card *card = func->card;
	struct mmc_host *host = card->host;

	if (!(host->caps & (MMC_CAP_4_BIT_DATA)))
		return 0;

	if (card->cccr.low_speed && !card->cccr.wide_bus)
		return 0;

	sdio_claim_host(func);
	do {
		int setAsyncIRQ = 0;
		__u16 manufacturer_id =
			device->id->device & MANUFACTURER_ID_AR6K_BASE_MASK;

		/* Re-enable 4-bit ASYNC interrupt on AR6003x
		 * after system resume for some host controller
		 */
		if (manufacturer_id == MANUFACTURER_ID_AR6003_BASE) {
			setAsyncIRQ = 1;
			ret =
				func0_cmd52_write_byte(func->card,
					    CCCR_SDIO_IRQ_MODE_REG_AR6003,
					    enable ?
					    SDIO_IRQ_MODE_ASYNC_4BIT_IRQ_AR6003
					    : 0);
		} else if (manufacturer_id == MANUFACTURER_ID_AR6320_BASE ||
			     manufacturer_id == MANUFACTURER_ID_QCA9377_BASE ||
			     manufacturer_id == MANUFACTURER_ID_QCA9379_BASE) {
			unsigned char data = 0;

			setAsyncIRQ = 1;
			ret =
				func0_cmd52_read_byte(func->card,
					      CCCR_SDIO_IRQ_MODE_REG_AR6320,
						   &data);
			if (ret) {
				AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
					("%s: failed to read interrupt extension register %d\n",
						 __func__, ret));
				sdio_release_host(func);
				return ret;
			}
			if (enable)
				data |= SDIO_IRQ_MODE_ASYNC_4BIT_IRQ_AR6320;
			else
				data &= ~SDIO_IRQ_MODE_ASYNC_4BIT_IRQ_AR6320;
			ret =
				func0_cmd52_write_byte(func->card,
					       CCCR_SDIO_IRQ_MODE_REG_AR6320,
					       data);
		}
		if (setAsyncIRQ) {
			if (ret) {
				AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
					("%s: failed to setup 4-bit ASYNC IRQ mode into %d err %d\n",
					 __func__, enable, ret));
			} else {
				AR_DEBUG_PRINTF(ATH_DEBUG_INFO,
					("%s: Setup 4-bit ASYNC IRQ mode into %d successfully\n",
					 __func__, enable));
			}
		}
	} while (0);
	sdio_release_host(func);

	return ret;
}
#else
static int sdio_enable4bits(struct hif_sdio_dev *device, int enable)
{
	return 0;
}
#endif

/**
 * hif_sdio_probe() - configure sdio device
 * @ol_sc: HIF device context
 * @func: SDIO function context
 * @device: pointer to hif handle
 *
 * Return: QDF_STATUS
 */
QDF_STATUS hif_sdio_probe(struct hif_softc *ol_sc,
			  struct sdio_func *func,
			  struct hif_sdio_dev *device)
{
	int ret = 0;
	const struct sdio_device_id *id;
	uint32_t target_type;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	HIF_ENTER();
	scn = (struct hif_sdio_softc *)ol_sc;

	scn->hif_handle = device;
	spin_lock_init(&scn->target_lock);
	/*
	 * Attach Target register table. This is needed early on
	 * even before BMI since PCI and HIF initialization
	 * directly access Target registers.
	 *
	 * TBDXXX: targetdef should not be global -- should be stored
	 * in per-device struct so that we can support multiple
	 * different Target types with a single Host driver.
	 * The whole notion of an "hif type" -- (not as in the hif
	 * module, but generic "Host Interface Type") is bizarre.
	 * At first, one one expect it to be things like SDIO, USB, PCI.
	 * But instead, it's an actual platform type. Inexplicably, the
	 * values used for HIF platform types are *different* from the
	 * values used for Target Types.
	 */

#if defined(CONFIG_AR9888_SUPPORT)
	hif_register_tbl_attach(ol_sc, HIF_TYPE_AR9888);
	target_register_tbl_attach(ol_sc, TARGET_TYPE_AR9888);
	target_type = TARGET_TYPE_AR9888;
#elif defined(CONFIG_AR6320_SUPPORT)
	id = device->id;
	if (((id->device & MANUFACTURER_ID_AR6K_BASE_MASK) ==
				MANUFACTURER_ID_QCA9377_BASE) ||
			((id->device & MANUFACTURER_ID_AR6K_BASE_MASK) ==
			 MANUFACTURER_ID_QCA9379_BASE)) {
		hif_register_tbl_attach(ol_sc, HIF_TYPE_AR6320V2);
		target_register_tbl_attach(ol_sc, TARGET_TYPE_AR6320V2);
	} else if ((id->device & MANUFACTURER_ID_AR6K_BASE_MASK) ==
			MANUFACTURER_ID_AR6320_BASE) {
		int ar6kid = id->device & MANUFACTURER_ID_AR6K_REV_MASK;

		if (ar6kid >= 1) {
			/* v2 or higher silicon */
			hif_register_tbl_attach(ol_sc, HIF_TYPE_AR6320V2);
			target_register_tbl_attach(ol_sc, TARGET_TYPE_AR6320V2);
		} else {
			/* legacy v1 silicon */
			hif_register_tbl_attach(ol_sc, HIF_TYPE_AR6320);
			target_register_tbl_attach(ol_sc, TARGET_TYPE_AR6320);
		}
	}
	target_type = TARGET_TYPE_AR6320;

#endif
	scn->targetdef = ol_sc->targetdef;
	scn->hostdef = ol_sc->hostdef;
	scn->dev = &func->dev;
	ol_sc->bus_type = QDF_BUS_TYPE_SDIO;
	ol_sc->target_info.target_type = target_type;

	scn->ramdump_base =
		pld_hif_sdio_get_virt_ramdump_mem(&func->dev,
						  &scn->ramdump_size);
	if (!scn->ramdump_base || !scn->ramdump_size) {
		hf_err("Failed ramdump res alloc - base:%s, len:%lu",
			scn->ramdump_base ? "ok" : "null",
			scn->ramdump_size);
	} else {
		hif_info("ramdump base %pK size %lu",
			 scn->ramdump_base, scn->ramdump_size);
	}

	if (athdiag_procfs_init(scn) != 0) {
		status = QDF_STATUS_E_FAILURE;
		goto err_attach1;
	}

	ret = hif_dev_register_channels(device, func);

	return qdf_status_from_os_return(ret);

err_attach1:
	if (scn->ramdump_base)
		pld_hif_sdio_release_ramdump_mem(scn->ramdump_base);
	scn = NULL;
	return status;
}

/**
 * power_state_change_notify() - SDIO bus power notification handler
 * @ol_sc: HIF device context
 * @config: hif device power change type
 *
 * Return: 0 on success, error number otherwise.
 */
static QDF_STATUS
power_state_change_notify(struct hif_softc *ol_sc,
			  struct hif_sdio_dev *device,
			  enum HIF_DEVICE_POWER_CHANGE_TYPE config)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct sdio_func *func = device->func;

	AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
			("%s: config type %d\n",
			__func__, config));
	switch (config) {
	case HIF_DEVICE_POWER_DOWN:
		/* Disable 4bits to allow SDIO bus to detect
		 * DAT1 as interrupt source
		 */
		sdio_enable4bits(device, 0);
		break;
	case HIF_DEVICE_POWER_CUT:
		status = hif_disable_func(device, func, 1);
		if (!device->is_suspend) {
			device->power_config = config;
			mmc_detect_change(device->host, HZ / 3);
		}
		break;
	case HIF_DEVICE_POWER_UP:
		if (device->power_config == HIF_DEVICE_POWER_CUT) {
			if (device->is_suspend) {
				status = reinit_sdio(device);
				/* set power_config before EnableFunc to
				 * passthrough sdio r/w action when resuming
				 * from cut power
				 */
				device->power_config = config;
				if (status == QDF_STATUS_SUCCESS)
					status = hif_enable_func(ol_sc, device,
								 func, true);
			} else {
				/* device->func is bad pointer at this time */
				mmc_detect_change(device->host, 0);
				return QDF_STATUS_E_PENDING;
			}
		} else if (device->power_config == HIF_DEVICE_POWER_DOWN) {
			int ret = sdio_enable4bits(device, 1);

			status = (ret == 0) ? QDF_STATUS_SUCCESS :
						QDF_STATUS_E_FAILURE;
		}
		break;
	}
	device->power_config = config;

	AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
			("%s:\n", __func__));

	return status;
}


/**
 * hif_configure_device() - configure sdio device
 * @ol_sc: HIF device context
 * @device: pointer to hif device structure
 * @opcode: configuration type
 * @config: configuration value to set
 * @configLen: configuration length
 *
 * Return: 0 on success, error number otherwise.
 */
QDF_STATUS
hif_configure_device(struct hif_softc *ol_sc, struct hif_sdio_dev *device,
		     enum hif_device_config_opcode opcode,
		     void *config, uint32_t config_len)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	switch (opcode) {
	case HIF_DEVICE_GET_BLOCK_SIZE:
		hif_dev_get_block_size(config);
		break;

	case HIF_DEVICE_GET_FIFO_ADDR:
		hif_dev_get_fifo_address(device, config, config_len);
		break;

	case HIF_DEVICE_GET_PENDING_EVENTS_FUNC:
		hif_warn("opcode %d", opcode);
		status = QDF_STATUS_E_FAILURE;
		break;
	case HIF_DEVICE_GET_IRQ_PROC_MODE:
		*((enum hif_device_irq_mode *) config) =
			HIF_DEVICE_IRQ_SYNC_ONLY;
		break;
	case HIF_DEVICE_GET_RECV_EVENT_MASK_UNMASK_FUNC:
		hif_warn("opcode %d", opcode);
		status = QDF_STATUS_E_FAILURE;
		break;
	case HIF_CONFIGURE_QUERY_SCATTER_REQUEST_SUPPORT:
		if (!device->scatter_enabled)
			return QDF_STATUS_E_NOSUPPORT;
		status =
			setup_hif_scatter_support(device,
				  (struct HIF_DEVICE_SCATTER_SUPPORT_INFO *)
				   config);
		if (QDF_IS_STATUS_ERROR(status))
			device->scatter_enabled = false;
		break;
	case HIF_DEVICE_GET_OS_DEVICE:
		/* pass back a pointer to the SDIO function's "dev" struct */
		((struct HIF_DEVICE_OS_DEVICE_INFO *) config)->os_dev =
			&device->func->dev;
		break;
	case HIF_DEVICE_POWER_STATE_CHANGE:
		status =
		power_state_change_notify(ol_sc, device,
					  *(enum HIF_DEVICE_POWER_CHANGE_TYPE *)
					   config);
		break;
	case HIF_DEVICE_GET_IRQ_YIELD_PARAMS:
		hif_warn("opcode %d", opcode);
		status = QDF_STATUS_E_FAILURE;
		break;
	case HIF_DEVICE_SET_HTC_CONTEXT:
		device->htc_context = config;
		break;
	case HIF_DEVICE_GET_HTC_CONTEXT:
		if (!config) {
			hif_err("htc context is NULL");
			return QDF_STATUS_E_FAILURE;
		}
		*(void **)config = device->htc_context;
		break;
	case HIF_BMI_DONE:
		hif_debug("BMI_DONE");
		break;
	default:
		hif_err("Unsupported opcode: %d", opcode);
		status = QDF_STATUS_E_FAILURE;
	}

	return status;
}

/**
 * hif_sdio_shutdown() - hif-sdio shutdown routine
 * @hif_ctx: pointer to hif_softc structore
 *
 * Return: None.
 */
void hif_sdio_shutdown(struct hif_softc *hif_ctx)
{
	struct hif_sdio_softc *scn = HIF_GET_SDIO_SOFTC(hif_ctx);
	struct hif_sdio_dev *hif_device = scn->hif_handle;

	AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
			("%s: Enter\n", __func__));
	if (hif_device) {
		AR_DEBUG_ASSERT(hif_device->power_config == HIF_DEVICE_POWER_CUT
				|| hif_device->func);
	} else {
		int i;
		/* since we are unloading the driver anyways,
		 * reset all cards in case the SDIO card is
		 * externally powered and we are unloading the SDIO
		 * stack. This avoids the problem when the SDIO stack
		 * is reloaded and attempts are made to re-enumerate
		 * a card that is already enumerated
		 */
		for (i = 0; i < MAX_HIF_DEVICES; ++i) {
			if (hif_devices[i] && !hif_devices[i]->func) {
				AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
				("%s: Remove pending hif_device %pK\n",
					 __func__, hif_devices[i]));
				del_hif_device(hif_devices[i]);
				hif_devices[i] = NULL;
			}
		}
	}
	AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
			("%s: Exit\n", __func__));
}

/**
 * hif_device_inserted() - hif-sdio driver probe handler
 * @ol_sc: HIF device context
 * @func: pointer to sdio_func
 * @id: pointer to sdio_device_id
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS hif_device_inserted(struct hif_softc *ol_sc,
				      struct sdio_func *func,
				      const struct sdio_device_id *id)
{
	int i, count;
	QDF_STATUS ret = QDF_STATUS_SUCCESS;
	struct hif_sdio_dev *device = NULL;

	hif_info("F%X, VID: 0x%X, DevID: 0x%X, block size: 0x%X/0x%X",
		 func->num, func->vendor, id->device,
		 func->max_blksize, func->cur_blksize);

	/* dma_mask should be populated here. Use the parent device's setting */
	func->dev.dma_mask = mmc_dev(func->card->host)->dma_mask;

	for (i = 0; i < MAX_HIF_DEVICES; ++i) {
		struct hif_sdio_dev *hifdevice = hif_devices[i];

		if (hifdevice &&
		    hifdevice->power_config == HIF_DEVICE_POWER_CUT &&
		    hifdevice->host == func->card->host) {
			device = get_hif_device(ol_sc, func);
			hifdevice->func = func;
			hifdevice->power_config = HIF_DEVICE_POWER_UP;
			hif_sdio_set_drvdata(ol_sc, func, hifdevice);

			if (device->is_suspend) {
				hif_info("Resume from suspend");
				ret = reinit_sdio(device);
			}
			break;
		}
	}

	/* If device not found, then it is a new insertion, alloc and add it */
	if (!device) {
		if (!add_hif_device(ol_sc, func))
			return QDF_STATUS_E_FAILURE;

		device = get_hif_device(ol_sc, func);

		for (i = 0; i < MAX_HIF_DEVICES; ++i) {
			if (!hif_devices[i]) {
				hif_devices[i] = device;
				break;
			}
		}
		if (i == MAX_HIF_DEVICES) {
			hif_err("No more slots");
			goto del_hif_dev;
		}

		device->id = id;
		device->host = func->card->host;
		device->is_disabled = true;
		/* TODO: MMC SDIO3.0 Setting should also be modified in ReInit()
		 * function when Power Manage work.
		 */
		sdio_claim_host(func);

		hif_sdio_quirk_force_drive_strength(ol_sc, func);

		hif_sdio_quirk_write_cccr(ol_sc, func);

		ret = hif_sdio_set_bus_speed(ol_sc, func);

		ret = hif_sdio_set_bus_width(ol_sc, func);
		if (debugcccr)
			hif_dump_cccr(device);

		sdio_release_host(func);
	}

	qdf_spinlock_create(&device->lock);

	qdf_spinlock_create(&device->asynclock);

	DL_LIST_INIT(&device->scatter_req_head);

	if (!nohifscattersupport) {
		/* try to allow scatter operation on all instances,
		 * unless globally overridden
		 */
		device->scatter_enabled = true;
	} else
		device->scatter_enabled = false;

	/* Initialize the bus requests to be used later */
	qdf_mem_zero(device->bus_request, sizeof(device->bus_request));
	for (count = 0; count < BUS_REQUEST_MAX_NUM; count++) {
		sema_init(&device->bus_request[count].sem_req, 0);
		hif_free_bus_request(device, &device->bus_request[count]);
	}
	sema_init(&device->sem_async, 0);

	ret = hif_enable_func(ol_sc, device, func, false);
	if ((ret == QDF_STATUS_SUCCESS || ret == QDF_STATUS_E_PENDING))
		return QDF_STATUS_SUCCESS;
	ret = QDF_STATUS_E_FAILURE;
del_hif_dev:
	del_hif_device(device);
	for (i = 0; i < MAX_HIF_DEVICES; ++i) {
		if (hif_devices[i] == device) {
			hif_devices[i] = NULL;
			break;
		}
	}
	if (i == MAX_HIF_DEVICES) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
			("%s: No hif_devices[] slot for %pK",
			__func__, device));
	}
	return ret;
}

/**
 * hif_ack_interrupt() - Acknowledge hif device irq
 * @device: pointer to struct hif_sdio_dev
 *
 * This should translate to an acknowledgment to the bus driver indicating that
 * the previous interrupt request has been serviced and the all the relevant
 * sources have been cleared. HTC is ready to process more interrupts.
 * This should prevent the bus driver from raising an interrupt unless the
 * previous one has been serviced and acknowledged using the previous API.
 *
 * Return: None.
 */
void hif_ack_interrupt(struct hif_sdio_dev *device)
{
	AR_DEBUG_ASSERT(device);

	/* Acknowledge our function IRQ */
}

/**
 * hif_sdio_configure_pipes - Configure pipes for the lower layer bus
 * @pdev - HIF layer object
 * @func - SDIO bus function object
 *
 * Return - error in case of failure to configure, else success
 */
int hif_sdio_configure_pipes(struct hif_sdio_dev *dev, struct sdio_func *func)
{
	return hif_dev_configure_pipes(dev, func);
}

/**
 * hif_allocate_bus_request() - Allocate hif bus request
 * @device: pointer to struct hif_sdio_dev
 *
 *
 * Return: pointer to struct bus_request structure.
 */
struct bus_request *hif_allocate_bus_request(struct hif_sdio_dev *device)
{
	struct bus_request *busrequest;

	qdf_spin_lock_irqsave(&device->lock);
	busrequest = device->bus_request_free_queue;
	/* Remove first in list */
	if (busrequest)
		device->bus_request_free_queue = busrequest->next;

	/* Release lock */
	qdf_spin_unlock_irqrestore(&device->lock);
	AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
			("%s: hif_allocate_bus_request: 0x%pK\n",
			__func__, busrequest));

	return busrequest;
}

/**
 * hif_free_bus_request() - Free hif bus request
 * @device: pointer to struct hif_sdio_dev
 *
 *
 * Return: None.
 */
void hif_free_bus_request(struct hif_sdio_dev *device,
			  struct bus_request *busrequest)
{
	AR_DEBUG_ASSERT(busrequest);
	/* Acquire lock */
	qdf_spin_lock_irqsave(&device->lock);

	/* Insert first in list */
	busrequest->next = device->bus_request_free_queue;
	busrequest->inusenext = NULL;
	device->bus_request_free_queue = busrequest;

	/* Release lock */
	qdf_spin_unlock_irqrestore(&device->lock);
}

int hif_device_suspend(struct hif_softc *ol_sc, struct device *dev)
{
	struct sdio_func *func = dev_to_sdio_func(dev);
	struct hif_sdio_dev *device = get_hif_device(ol_sc, func);
	mmc_pm_flag_t pm_flag = 0;
	enum HIF_DEVICE_POWER_CHANGE_TYPE config;
	struct mmc_host *host = func->card->host;

	host = device->func->card->host;

	device->is_suspend = true;

	switch (forcesleepmode) {
		case 0: /* depend on sdio host pm capbility */
			pm_flag = sdio_get_host_pm_caps(func);
			break;
		case 1: /* force WOW */
			pm_flag |= MMC_PM_KEEP_POWER | MMC_PM_WAKE_SDIO_IRQ;
			break;
		case 2: /* force DeepSleep */
			pm_flag &= ~MMC_PM_WAKE_SDIO_IRQ;
			pm_flag |= MMC_PM_KEEP_POWER;
			break;
		case 3: /* force CutPower */
			pm_flag &=
				~(MMC_PM_WAKE_SDIO_IRQ | MMC_PM_WAKE_SDIO_IRQ);
			break;
	}

	if (!(pm_flag & MMC_PM_KEEP_POWER)) {
		/* setting power_config before hif_configure_device to
		 * skip sdio r/w when suspending with cut power
		 */
		hif_info("Power cut");
		config = HIF_DEVICE_POWER_CUT;
		device->power_config = config;

		hif_configure_device(ol_sc, device,
				     HIF_DEVICE_POWER_STATE_CHANGE,
				     &config,
				     sizeof(config));
		hif_mask_interrupt(device);
		device->device_state = HIF_DEVICE_STATE_CUTPOWER;
		return 0;
	}

	if (sdio_set_host_pm_flags(func, MMC_PM_KEEP_POWER)) {
		hif_err("set pm_flags failed");
		return -EINVAL;
	}

	if (pm_flag & MMC_PM_WAKE_SDIO_IRQ) {
		hif_info("WOW mode");
		config = HIF_DEVICE_POWER_DOWN;
		hif_configure_device(ol_sc, device,
				     HIF_DEVICE_POWER_STATE_CHANGE,
				     &config,
				     sizeof(config));

		if (sdio_set_host_pm_flags(func, MMC_PM_WAKE_SDIO_IRQ)) {
			hif_err("set pm_flags failed");
			return -EINVAL;
		}
		hif_mask_interrupt(device);
		device->device_state = HIF_DEVICE_STATE_WOW;
		return 0;
	} else {
		hif_info("deep sleep enter");
		msleep(100);
		hif_mask_interrupt(device);
		device->device_state = HIF_DEVICE_STATE_DEEPSLEEP;
		return 0;
	}

	return 0;
}

int hif_device_resume(struct hif_softc *ol_sc, struct device *dev)
{
	struct sdio_func *func = dev_to_sdio_func(dev);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	enum HIF_DEVICE_POWER_CHANGE_TYPE config;
	struct hif_sdio_dev *device;

	device = get_hif_device(ol_sc, func);
	if (!device) {
		hif_err("hif object is null");
		return -EINVAL;
	}

	if (device->device_state == HIF_DEVICE_STATE_CUTPOWER) {
		config = HIF_DEVICE_POWER_UP;
		hif_configure_device(ol_sc, device,
				     HIF_DEVICE_POWER_STATE_CHANGE,
				     &config,
				     sizeof(config));
		hif_enable_func(ol_sc, device, func, true);
	} else if (device->device_state == HIF_DEVICE_STATE_DEEPSLEEP) {
		hif_un_mask_interrupt(device);
	} else if (device->device_state == HIF_DEVICE_STATE_WOW) {
		/*TODO:WOW support */
		hif_un_mask_interrupt(device);
	}

	device->is_suspend = false;
	device->device_state = HIF_DEVICE_STATE_ON;

	return QDF_IS_STATUS_SUCCESS(status) ? 0 : status;
}

/**
 * hif_sdio_remove() - remove sdio device
 * @conext: sdio device context
 * @hif_handle: pointer to sdio function
 *
 * Return: 0 for success and non-zero for failure
 */
static A_STATUS hif_sdio_remove(void *context, void *hif_handle)
{
	HIF_ENTER();

	if (!scn) {
		QDF_TRACE(QDF_MODULE_ID_HIF, QDF_TRACE_LEVEL_ERROR,
			  "Global SDIO context is NULL");
		return A_ERROR;
	}

	athdiag_procfs_remove();

#ifndef TARGET_DUMP_FOR_NON_QC_PLATFORM
	iounmap(scn->ramdump_base);
#endif

	HIF_EXIT();

	return 0;
}

static void hif_device_removed(struct hif_softc *ol_sc, struct sdio_func *func)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct hif_sdio_dev *device;
	int i;

	AR_DEBUG_ASSERT(func);
	HIF_ENTER();
	device = get_hif_device(ol_sc, func);

	if (device->power_config == HIF_DEVICE_POWER_CUT) {
		device->func = NULL;    /* func will be free by mmc stack */
		return;         /* Just return for cut-off mode */
	}
	for (i = 0; i < MAX_HIF_DEVICES; ++i) {
		if (hif_devices[i] == device)
			hif_devices[i] = NULL;
	}

	hif_sdio_remove(device->claimed_ctx, device);

	hif_mask_interrupt(device);

	if (device->is_disabled)
		device->is_disabled = false;
	else
		status = hif_disable_func(device, func,
					  reset_sdio_on_unload ? true : false);


	del_hif_device(device);
	if (status != QDF_STATUS_SUCCESS)
		AR_DEBUG_PRINTF(ATH_DEBUG_WARN,
		  ("%s: Unable to disable sdio func\n",
		   __func__));

	HIF_EXIT();
}

static struct hif_sdio_dev *add_hif_device(struct hif_softc *ol_sc,
					   struct sdio_func *func)
{
	struct hif_sdio_dev *hifdevice = NULL;
	int ret = 0;

	HIF_ENTER();
	AR_DEBUG_ASSERT(func);
	hifdevice = (struct hif_sdio_dev *) qdf_mem_malloc(sizeof(
							struct hif_sdio_dev));
	AR_DEBUG_ASSERT(hifdevice);
	if (!hifdevice)
		return NULL;

#if HIF_USE_DMA_BOUNCE_BUFFER
	hifdevice->dma_buffer = qdf_mem_malloc(HIF_DMA_BUFFER_SIZE);
	AR_DEBUG_ASSERT(hifdevice->dma_buffer);
	if (!hifdevice->dma_buffer) {
		qdf_mem_free(hifdevice);
		return NULL;
	}
#endif
	hifdevice->func = func;
	hifdevice->power_config = HIF_DEVICE_POWER_UP;
	hifdevice->device_state = HIF_DEVICE_STATE_ON;
	ret = hif_sdio_set_drvdata(ol_sc, func, hifdevice);
	hif_info("status %d", ret);

	return hifdevice;
}

static void del_hif_device(struct hif_sdio_dev *device)
{
	AR_DEBUG_ASSERT(device);
	AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
			("%s: deleting hif device 0x%pK\n",
				__func__, device));
	if (device->dma_buffer)
		qdf_mem_free(device->dma_buffer);

	qdf_mem_free(device);
}

QDF_STATUS hif_attach_htc(struct hif_sdio_dev *device,
				struct htc_callbacks *callbacks)
{
	if (device->htc_callbacks.context)
		/* already in use! */
		return QDF_STATUS_E_FAILURE;
	device->htc_callbacks = *callbacks;

	return QDF_STATUS_SUCCESS;
}

void hif_detach_htc(struct hif_opaque_softc *hif_ctx)
{
	struct hif_sdio_softc *scn = HIF_GET_SDIO_SOFTC(hif_ctx);
	struct hif_sdio_dev *hif_device = scn->hif_handle;

	qdf_mem_zero(&hif_device->htc_callbacks,
			  sizeof(hif_device->htc_callbacks));
}

int func0_cmd52_write_byte(struct mmc_card *card,
			   unsigned int address,
			   unsigned char byte)
{
	struct mmc_command io_cmd;
	unsigned long arg;
	int status = 0;

	memset(&io_cmd, 0, sizeof(io_cmd));
	SDIO_SET_CMD52_WRITE_ARG(arg, 0, address, byte);
	io_cmd.opcode = SD_IO_RW_DIRECT;
	io_cmd.arg = arg;
	io_cmd.flags = MMC_RSP_R5 | MMC_CMD_AC;
	status = mmc_wait_for_cmd(card->host, &io_cmd, 0);

	if (status)
		hif_err("mmc_wait_for_cmd returned %d", status);

	return status;
}

int func0_cmd52_read_byte(struct mmc_card *card,
			  unsigned int address,
			  unsigned char *byte)
{
	struct mmc_command io_cmd;
	unsigned long arg;
	int32_t err;

	memset(&io_cmd, 0, sizeof(io_cmd));
	SDIO_SET_CMD52_READ_ARG(arg, 0, address);
	io_cmd.opcode = SD_IO_RW_DIRECT;
	io_cmd.arg = arg;
	io_cmd.flags = MMC_RSP_R5 | MMC_CMD_AC;

	err = mmc_wait_for_cmd(card->host, &io_cmd, 0);

	if ((!err) && (byte))
		*byte = io_cmd.resp[0] & 0xFF;

	if (err)
		hif_err("mmc_wait_for_cmd returned %d", err);

	return err;
}

void hif_dump_cccr(struct hif_sdio_dev *hif_device)
{
	unsigned int i;
	uint8_t cccr_val;
	uint32_t err;

	if (!hif_device || !hif_device->func ||
				!hif_device->func->card) {
		hif_err("Incorrect input");
		return;
	}

	for (i = 0; i <= 0x16; i++) {
		err = func0_cmd52_read_byte(hif_device->func->card,
						i, &cccr_val);
		if (err)
			hif_err("Reading CCCR 0x%02X failed: %d",
				i, (unsigned int)err);
		else
			hif_err("%X(%X) ", i, (unsigned int)cccr_val);
	}
}

QDF_STATUS hif_sdio_device_inserted(struct hif_softc *ol_sc,
				    struct device *dev,
				    const struct sdio_device_id *id)
{
	struct sdio_func *func = dev_to_sdio_func(dev);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	hif_debug("Enter");
	status = hif_device_inserted(ol_sc, func, id);
	hif_debug("Exit: status: %d", status);

	return status;
}

void hif_sdio_device_removed(struct hif_softc *ol_sc, struct sdio_func *func)
{
	hif_device_removed(ol_sc, func);
}
