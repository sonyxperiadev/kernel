/*
 * Linux DHD Bus Module for PCIE
 *
 * Copyright (C) 1999-2014, Broadcom Corporation
 *
 *      Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed to you
 * under the terms of the GNU General Public License version 2 (the "GPL"),
 * available at http://www.broadcom.com/licenses/GPLv2.php, with the
 * following added to such license:
 *
 *      As a special exception, the copyright holders of this software give you
 * permission to link this software with independent modules, and to copy and
 * distribute the resulting executable under terms of your choice, provided that
 * you also meet, for each linked independent module, the terms and conditions of
 * the license of that module.  An independent module is a module which is not
 * derived from this software.  The special exception does not apply to any
 * modifications of the software.
 *
 *      Notwithstanding the above, under no circumstances may you combine this
 * software in any way with any other Broadcom software provided under a license
 * other than the GPL, without Broadcom's express prior written consent.
 *
 * $Id: dhd_pcie_linux.c 477713 2014-05-14 08:59:12Z $
 */


/* include files */
#include <typedefs.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <siutils.h>
#include <hndsoc.h>
#include <hndpmu.h>
#include <sbchipc.h>
#if defined(DHD_DEBUG)
#include <hnd_armtrap.h>
#include <hnd_cons.h>
#endif /* defined(DHD_DEBUG) */
#include <dngl_stats.h>
#include <pcie_core.h>
#include <dhd.h>
#include <dhd_bus.h>
#include <dhd_proto.h>
#include <dhd_dbg.h>
#include <dhdioctl.h>
#include <bcmmsgbuf.h>
#include <pcicfg.h>
#include <dhd_pcie.h>
#if defined (CONFIG_ARCH_MSM)
#include <mach/msm_pcie.h>
#endif

#define PCI_CFG_RETRY 		10
#define OS_HANDLE_MAGIC		0x1234abcd	/* Magic # to recognize osh */
#define BCM_MEM_FILENAME_LEN 	24		/* Mem. filename length */

#define OSL_PKTTAG_CLEAR(p) \
do { \
	struct sk_buff *s = (struct sk_buff *)(p); \
	ASSERT(OSL_PKTTAG_SZ == 32); \
	*(uint32 *)(&s->cb[0]) = 0; *(uint32 *)(&s->cb[4]) = 0; \
	*(uint32 *)(&s->cb[8]) = 0; *(uint32 *)(&s->cb[12]) = 0; \
	*(uint32 *)(&s->cb[16]) = 0; *(uint32 *)(&s->cb[20]) = 0; \
	*(uint32 *)(&s->cb[24]) = 0; *(uint32 *)(&s->cb[28]) = 0; \
} while (0)


/* user defined data structures  */

typedef struct dhd_pc_res {
	uint32 bar0_size;
	void* bar0_addr;
	uint32 bar1_size;
	void* bar1_addr;
} pci_config_res, *pPci_config_res;

typedef bool (*dhdpcie_cb_fn_t)(void *);

typedef struct dhdpcie_info
{
	dhd_bus_t	*bus;
	osl_t 			*osh;
	struct pci_dev  *dev;		/* pci device handle */
	volatile char 	*regs;		/* pci device memory va */
	volatile char 	*tcm;		/* pci device memory va */
	uint32			tcm_size;	/* pci device memory size */
	struct pcos_info *pcos_info;
	uint16		last_intrstatus;	/* to cache intrstatus */
	int	irq;
	char pciname[32];

	struct pci_saved_state* state;

} dhdpcie_info_t;


struct pcos_info {
	dhdpcie_info_t *pc;
	spinlock_t lock;
	wait_queue_head_t intr_wait_queue;
	struct timer_list tuning_timer;
	int tuning_timer_exp;
	atomic_t timer_enab;
	struct tasklet_struct tuning_tasklet;
};


/* function declarations */
static int __devinit
dhdpcie_pci_probe(struct pci_dev *pdev, const struct pci_device_id *ent);
static void __devexit
dhdpcie_pci_remove(struct pci_dev *pdev);
static int dhdpcie_init(struct pci_dev *pdev);
static irqreturn_t dhdpcie_isr(int irq, void *arg);
/* OS Routine functions for PCI suspend/resume */

static int dhdpcie_pci_suspend(struct pci_dev *dev, pm_message_t state);
static int dhdpcie_set_suspend_resume(struct pci_dev *dev, bool state);
static int dhdpcie_pci_resume(struct pci_dev *dev);
static int dhdpcie_resume_dev(struct pci_dev *dev);
static int dhdpcie_suspend_dev(struct pci_dev *dev);
static struct pci_device_id dhdpcie_pci_devid[] __devinitdata = {
	{ vendor: 0x14e4,
	device: PCI_ANY_ID,
	subvendor: PCI_ANY_ID,
	subdevice: PCI_ANY_ID,
	class: PCI_CLASS_NETWORK_OTHER << 8,
	class_mask: 0xffff00,
	driver_data: 0,
	},
	{ 0, }
};
MODULE_DEVICE_TABLE(pci, dhdpcie_pci_devid);

static struct pci_driver dhdpcie_driver = {
	node:		{},
	name:		"pcieh",
	id_table:	dhdpcie_pci_devid,
	probe:		dhdpcie_pci_probe,
	remove:		dhdpcie_pci_remove,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
	save_state:	NULL,
#endif
	suspend:	dhdpcie_pci_suspend,
	resume:		dhdpcie_pci_resume,
};

int dhdpcie_init_succeeded = FALSE;

static void dhdpcie_pme_active(struct pci_dev *pdev, bool enable)
{
	uint16 pmcsr;

	pci_read_config_word(pdev, pdev->pm_cap + PCI_PM_CTRL, &pmcsr);
	/* Clear PME Status by writing 1 to it and enable PME# */
	pmcsr |= PCI_PM_CTRL_PME_STATUS | PCI_PM_CTRL_PME_ENABLE;
	if (!enable)
		pmcsr &= ~PCI_PM_CTRL_PME_ENABLE;

	pci_write_config_word(pdev, pdev->pm_cap + PCI_PM_CTRL, pmcsr);
}

static int dhdpcie_set_suspend_resume(struct pci_dev *pdev, bool state)
{
	int ret = 0;
	dhdpcie_info_t *pch = pci_get_drvdata(pdev);
	dhd_bus_t *bus = NULL;
	DHD_ERROR(("%s Enter with state :%x\n", __FUNCTION__, state));
	if (pch) {
		bus = pch->bus;
	}

	/* When firmware is not loaded do the PCI bus */
	/* suspend/resume only */
	if (bus && (bus->dhd->busstate == DHD_BUS_DOWN) &&
		!bus->dhd->dongle_reset) {
		ret = dhdpcie_pci_suspend_resume(bus->dev, state);
		return ret;
	}

	if (bus && ((bus->dhd->busstate == DHD_BUS_SUSPEND)||
		(bus->dhd->busstate == DHD_BUS_DATA)) &&
		(bus->suspended != state)) {

		ret = dhdpcie_bus_suspend(bus, state);
	}
	DHD_ERROR(("%s Exit with state :%d\n", __FUNCTION__, ret));
	return ret;
}

static int dhdpcie_pci_suspend(struct pci_dev * pdev, pm_message_t state)
{
	BCM_REFERENCE(state);
	DHD_ERROR(("%s Enter with event %x\n", __FUNCTION__, state.event));
	return dhdpcie_set_suspend_resume(pdev, TRUE);
}

static int dhdpcie_pci_resume(struct pci_dev *pdev)
{
	DHD_ERROR(("%s Enter\n", __FUNCTION__));
	return dhdpcie_set_suspend_resume(pdev, FALSE);
}

static int dhdpcie_suspend_dev(struct pci_dev *dev)
{
	int ret;
	dhdpcie_pme_active(dev, TRUE);
	pci_save_state(dev);
	pci_enable_wake(dev, PCI_D0, TRUE);
	if (pci_is_enabled(dev))
		pci_disable_device(dev);
	ret = pci_set_power_state(dev, PCI_D3hot);
	return ret;
}

static int dhdpcie_resume_dev(struct pci_dev *dev)
{
	int err = 0;
	pci_restore_state(dev);
	err = pci_enable_device(dev);
	if (err) {
		printf("%s:pci_enable_device error %d \n", __FUNCTION__, err);
		return err;
	}
	pci_set_master(dev);
	/*
	 * Suspend/Resume resets the PCI configuration space, so we have to
	 * re-disable the RETRY_TIMEOUT register (0x41) to keep
	 * PCI Tx retries from interfering with C3 CPU state
	 * Code taken from ipw2100 driver
	 */
	err = pci_set_power_state(dev, PCI_D0);
	if (err) {
		printf("%s:pci_set_power_state error %d \n", __FUNCTION__, err);
		return err;
	}
	dhdpcie_pme_active(dev, FALSE);
	return err;
}

int dhdpcie_pci_suspend_resume(struct pci_dev *dev, bool state)
{
	int rc;

	if (state)
		rc = dhdpcie_suspend_dev(dev);
	else
		rc = dhdpcie_resume_dev(dev);
	return rc;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
static int dhdpcie_device_scan(struct device *dev, void *data)
{
	struct pci_dev *pcidev;
	int *cnt = data;

	pcidev = container_of(dev, struct pci_dev, dev);
	if (pcidev->vendor != 0x14e4)
		return 0;

	DHD_INFO(("Found Broadcom PCI device 0x%04x\n", pcidev->device));
	*cnt += 1;
	if (pcidev->driver && strcmp(pcidev->driver->name, dhdpcie_driver.name))
		DHD_ERROR(("Broadcom PCI Device 0x%04x has allocated with driver %s\n",
			pcidev->device, pcidev->driver->name));

	return 0;
}
#endif /* LINUX_VERSION >= 2.6.0 */

int
dhdpcie_bus_register(void)
{
	int error = 0;


#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
	if (!(error = pci_module_init(&dhdpcie_driver)))
		return 0;

	DHD_ERROR(("%s: pci_module_init failed 0x%x\n", __FUNCTION__, error));
#else
	if (!(error = pci_register_driver(&dhdpcie_driver))) {
		bus_for_each_dev(dhdpcie_driver.driver.bus, NULL, &error, dhdpcie_device_scan);
		if (!error) {
			DHD_ERROR(("No Broadcom PCI device enumerated!\n"));
		} else if (!dhdpcie_init_succeeded) {
			DHD_ERROR(("%s: dhdpcie initialize failed.\n", __FUNCTION__));
		} else {
			return 0;
		}

		pci_unregister_driver(&dhdpcie_driver);
		error = BCME_ERROR;
	}
#endif /* LINUX_VERSION < 2.6.0 */

	return error;
}


void
dhdpcie_bus_unregister(void)
{
	pci_unregister_driver(&dhdpcie_driver);
}

int __devinit
dhdpcie_pci_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{

	if (dhdpcie_chipmatch (pdev->vendor, pdev->device)) {
		DHD_ERROR(("%s: chipmatch failed!!\n", __FUNCTION__));
			return -ENODEV;
	}
	printf("PCI_PROBE:  bus %X, slot %X,vendor %X, device %X"
		"(good PCI location)\n", pdev->bus->number,
		PCI_SLOT(pdev->devfn), pdev->vendor, pdev->device);

	if (dhdpcie_init (pdev)) {
		DHD_ERROR(("%s: PCIe Enumeration failed\n", __FUNCTION__));
		return -ENODEV;
	}
	/* disable async suspend */
	device_disable_async_suspend(&pdev->dev);
	DHD_TRACE(("%s: PCIe Enumeration done!!\n", __FUNCTION__));
	return 0;
}

int
dhdpcie_detach(dhdpcie_info_t *pch)
{
	osl_t *osh = pch->osh;
	if (pch) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0))
		if (!dhd_download_fw_on_driverload)
			pci_load_and_free_saved_state(pch->dev, &pch->state);
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0) */
		MFREE(osh, pch, sizeof(dhdpcie_info_t));
	}
	return 0;
}


void __devexit
dhdpcie_pci_remove(struct pci_dev *pdev)
{
	osl_t *osh = NULL;
	dhdpcie_info_t *pch = NULL;
	dhd_bus_t *bus = NULL;

	DHD_TRACE(("%s Enter\n", __FUNCTION__));
	pch = pci_get_drvdata(pdev);
	bus = pch->bus;
	osh = pch->osh;

	dhdpcie_bus_release(bus);
	pci_disable_device(pdev);
	/* pcie info detach */
	dhdpcie_detach(pch);
	/* osl detach */
	osl_detach(osh);

	dhdpcie_init_succeeded = FALSE;

	DHD_TRACE(("%s Exit\n", __FUNCTION__));

	return;
}

/* Free Linux irq */
int
dhdpcie_request_irq(dhdpcie_info_t *dhdpcie_info)
{
	dhd_bus_t *bus = dhdpcie_info->bus;
	struct pci_dev *pdev = dhdpcie_info->bus->dev;

	snprintf(dhdpcie_info->pciname, sizeof(dhdpcie_info->pciname),
	    "dhdpcie:%s", pci_name(pdev));
	if (request_irq(pdev->irq, dhdpcie_isr, IRQF_SHARED,
	                dhdpcie_info->pciname, bus) < 0) {
			DHD_ERROR(("%s: request_irq() failed\n", __FUNCTION__));
			return -1;
	}

	DHD_TRACE(("%s %s\n", __FUNCTION__, dhdpcie_info->pciname));


	return 0; /* SUCCESS */
}

#ifdef CONFIG_PHYS_ADDR_T_64BIT
#define PRINTF_RESOURCE	"0x%016llx"
#else
#define PRINTF_RESOURCE	"0x%08x"
#endif

/*

Name:  osl_pci_get_resource

Parametrs:

1: struct pci_dev *pdev   -- pci device structure
2: pci_res                       -- structure containing pci configuration space values


Return value:

int   - Status (TRUE or FALSE)

Description:
Access PCI configuration space, retrieve  PCI allocated resources , updates in resource structure.

 */
int dhdpcie_get_resource(dhdpcie_info_t *dhdpcie_info)
{
	phys_addr_t  bar0_addr, bar1_addr;
	ulong bar1_size;
	struct pci_dev *pdev = NULL;
	pdev = dhdpcie_info->dev;
	do {
		if (pci_enable_device(pdev)) {
			printf("%s: Cannot enable PCI device\n", __FUNCTION__);
			break;
		}
		pci_set_master(pdev);
		bar0_addr = pci_resource_start(pdev, 0);	/* Bar-0 mapped address */
		bar1_addr = pci_resource_start(pdev, 2);	/* Bar-1 mapped address */

		/* read Bar-1 mapped memory range */
		bar1_size = pci_resource_len(pdev, 2);

		if ((bar1_size == 0) || (bar1_addr == 0)) {
			printf("%s: BAR1 Not enabled for this device  size(%ld),"
				" addr(0x"PRINTF_RESOURCE")\n",
				__FUNCTION__, bar1_size, bar1_addr);
			goto err;
		}

		dhdpcie_info->regs = (volatile char *) REG_MAP(bar0_addr, DONGLE_REG_MAP_SIZE);
		dhdpcie_info->tcm = (volatile char *) REG_MAP(bar1_addr, DONGLE_TCM_MAP_SIZE);
		dhdpcie_info->tcm_size = DONGLE_TCM_MAP_SIZE;

		if (!dhdpcie_info->regs || !dhdpcie_info->tcm) {
			DHD_ERROR(("%s:ioremap() failed\n", __FUNCTION__));
			break;
		}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0))
		if (!dhd_download_fw_on_driverload) {
			/* Backup PCIe configuration so as to use Wi-Fi on/off process
			 * in case of built in driver
			 */
			pci_save_state(pdev);
			dhdpcie_info->state = pci_store_saved_state(pdev);

			if (dhdpcie_info->state == NULL) {
				DHD_ERROR(("%s pci_store_saved_state returns NULL\n",
					__FUNCTION__));
				REG_UNMAP(dhdpcie_info->regs);
				REG_UNMAP(dhdpcie_info->tcm);
				pci_disable_device(pdev);
				break;
			}
		}
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0) */

		DHD_TRACE(("%s:Phys addr : reg space = %p base addr 0x"PRINTF_RESOURCE" \n",
			__FUNCTION__, dhdpcie_info->regs, bar0_addr));
		DHD_TRACE(("%s:Phys addr : tcm_space = %p base addr 0x"PRINTF_RESOURCE" \n",
			__FUNCTION__, dhdpcie_info->tcm, bar1_addr));

		return 0; /* SUCCESS  */
	} while (0);
err:
	return -1;  /* FAILURE */
}

int dhdpcie_scan_resource(dhdpcie_info_t *dhdpcie_info)
{

	DHD_TRACE(("%s: ENTER\n", __FUNCTION__));

	do {
		/* define it here only!! */
		if (dhdpcie_get_resource (dhdpcie_info)) {
			DHD_ERROR(("%s: Failed to get PCI resources\n", __FUNCTION__));
			break;
		}
		DHD_TRACE(("%s:Exit - SUCCESS \n",
			__FUNCTION__));

		return 0; /* SUCCESS */

	} while (0);

	DHD_TRACE(("%s:Exit - FAILURE \n", __FUNCTION__));

	return -1; /* FAILURE */

}
#ifdef MSM_PCIE_LINKDOWN_RECOVERY
void dhdpcie_linkdown_cb(struct msm_pcie_notify *noti)
{
	struct pci_dev *pdev = (struct pci_dev *)noti->user;
	dhdpcie_info_t *pch;
	dhd_bus_t *bus;
	dhd_pub_t *dhd;
	if (pdev && (pch = pci_get_drvdata(pdev))) {
		if ((bus = pch->bus) && (dhd = bus->dhd)) {
			DHD_ERROR(("%s: Event HANG send up "
				"due to PCIe linkdown\n", __FUNCTION__));
			bus->islinkdown = TRUE;
			DHD_OS_WAKE_LOCK_CTRL_TIMEOUT_ENABLE(dhd, DHD_EVENT_TIMEOUT_MS);
			dhd_os_check_hang(dhd, 0, -ETIMEDOUT);
		}
	}
}
#endif /* MSM_PCIE_LINKDOWN_RECOVERY */
int dhdpcie_init(struct pci_dev *pdev)
{

	osl_t 				*osh = NULL;
	dhd_bus_t 			*bus = NULL;
	dhdpcie_info_t		*dhdpcie_info =  NULL;
	DHD_ERROR(("%s enter\n", __FUNCTION__));
	do {
		/* osl attach */
		if (!(osh = osl_attach(pdev, PCI_BUS, FALSE))) {
			DHD_ERROR(("%s: osl_attach failed\n", __FUNCTION__));
			break;
		}

		/*  allocate linux spcific pcie structure here */
		if (!(dhdpcie_info = MALLOC(osh, sizeof(dhdpcie_info_t)))) {
			DHD_ERROR(("%s: MALLOC of dhd_bus_t failed\n", __FUNCTION__));
			break;
		}
		bzero(dhdpcie_info, sizeof(dhdpcie_info_t));
		dhdpcie_info->osh = osh;
		dhdpcie_info->dev = pdev;

		/* Find the PCI resources, verify the  */
		/* vendor and device ID, map BAR regions and irq,  update in structures */
		if (dhdpcie_scan_resource(dhdpcie_info)) {
			DHD_ERROR(("%s: dhd_Scan_PCI_Res failed\n", __FUNCTION__));

			break;
		}

		/* Bus initialization */
		bus = dhdpcie_bus_attach(osh, dhdpcie_info->regs, dhdpcie_info->tcm);
		if (!bus) {
			DHD_ERROR(("%s:dhdpcie_bus_attach() failed\n", __FUNCTION__));
			break;
		}

		dhdpcie_info->bus = bus;
		dhdpcie_info->bus->dev = pdev;
#ifdef MSM_PCIE_LINKDOWN_RECOVERY
		bus->islinkdown = FALSE;
		bus->pcie_event.events = MSM_PCIE_EVENT_LINKDOWN;
		bus->pcie_event.user = pdev;
		bus->pcie_event.mode = MSM_PCIE_TRIGGER_CALLBACK;
		bus->pcie_event.callback = dhdpcie_linkdown_cb;
		bus->pcie_event.options = MSM_PCIE_CONFIG_NO_RECOVERY;
		msm_pcie_register_event(&bus->pcie_event);
#endif /* MSM_PCIE_LINKDOWN_RECOVERY */

		if (bus->intr) {
			/* Register interrupt callback, but mask it (not operational yet). */
			DHD_INTR(("%s: Registering and masking interrupts\n", __FUNCTION__));
			dhdpcie_bus_intr_disable(bus);

			if (dhdpcie_request_irq(dhdpcie_info)) {
				DHD_ERROR(("%s: request_irq() failed\n", __FUNCTION__));
				break;
			}
		} else {
			bus->pollrate = 1;
			DHD_INFO(("%s: PCIe interrupt function is NOT registered "
				"due to polling mode\n", __FUNCTION__));
		}


		/* set private data for pci_dev */
		pci_set_drvdata(pdev, dhdpcie_info);
		/* Attach to the OS network interface */
		DHD_TRACE(("%s(): Calling dhd_register_if() \n", __FUNCTION__));
		if(dhd_register_if(bus->dhd, 0, TRUE)) {
			DHD_ERROR(("%s(): ERROR.. dhd_register_if() failed\n", __FUNCTION__));
			break;
		}
		if (dhd_download_fw_on_driverload) {
			if (dhd_bus_start(bus->dhd)) {
				DHD_ERROR(("%s: dhd_bud_start() failed\n", __FUNCTION__));
				break;
			}
		}

		dhdpcie_init_succeeded = TRUE;

		DHD_ERROR(("%s:Exit - SUCCESS \n", __FUNCTION__));
		return 0;  /* return  SUCCESS  */

	} while (0);
	/* reverse the initialization in order in case of error */

	if (bus)
		dhdpcie_bus_release(bus);

	if (dhdpcie_info)
		dhdpcie_detach(dhdpcie_info);
	pci_disable_device(pdev);
	if (osh)
		osl_detach(osh);

	dhdpcie_init_succeeded = FALSE;

	DHD_TRACE(("%s:Exit - FAILURE \n", __FUNCTION__));

	return -1; /* return FAILURE  */
}

/* Free Linux irq */
void
dhdpcie_free_irq(dhd_bus_t *bus)
{
	struct pci_dev *pdev = NULL;

	DHD_TRACE(("%s: freeing up the IRQ\n", __FUNCTION__));
	if (bus) {
		pdev = bus->dev;
		free_irq(pdev->irq, bus);
	}
	DHD_TRACE(("%s: Exit\n", __FUNCTION__));
	return;
}

/*

Name:  dhdpcie_isr

Parametrs:

1: IN int irq   -- interrupt vector
2: IN void *arg      -- handle to private data structure

Return value:

Status (TRUE or FALSE)

Description:
Interrupt Service routine checks for the status register,
disable interrupt and queue DPC if mail box interrupts are raised.
*/


irqreturn_t
dhdpcie_isr(int irq, void *arg)
{
	dhd_bus_t *bus = (dhd_bus_t*)arg;
	if (dhdpcie_bus_isr(bus))
		return TRUE;
	else
		return FALSE;
}

int
dhdpcie_start_host_pcieclock(dhd_bus_t *bus)
{
	int ret=0;
	int options = 0;
	DHD_TRACE(("%s Enter:\n", __FUNCTION__));

	if(bus == NULL)
		return BCME_ERROR;

	if(bus->dev == NULL)
		return BCME_ERROR;

#ifdef CONFIG_ARCH_MSM
#ifdef MSM_PCIE_LINKDOWN_RECOVERY
	if (bus->islinkdown)
		options = MSM_PCIE_CONFIG_NO_CFG_RESTORE;
#endif /* MSM_PCIE_LINKDOWN_RECOVERY */

	ret = msm_pcie_pm_control(MSM_PCIE_RESUME, bus->dev->bus->number,
		bus->dev, NULL, options);
#ifdef MSM_PCIE_LINKDOWN_RECOVERY
	if (bus->islinkdown && !ret) {
		msm_pcie_recover_config(bus->dev);
		if (bus->dhd)
			DHD_OS_WAKE_UNLOCK(bus->dhd);
		bus->islinkdown = FALSE;
	}
#endif /* MSM_PCIE_LINKDOWN_RECOVERY */

	if (ret) {
		DHD_ERROR(("%s Failed to bring up PCIe link\n", __FUNCTION__));
	}
#endif /* CONFIG_ARCH_MSM */
	DHD_TRACE(("%s Exit:\n", __FUNCTION__));
	return ret;
}

int
dhdpcie_stop_host_pcieclock(dhd_bus_t *bus)
{
	int ret = 0;
	int options = 0;
	DHD_TRACE(("%s Enter:\n", __FUNCTION__));

	if (bus == NULL)
		return BCME_ERROR;

	if (bus->dev == NULL)
		return BCME_ERROR;

#ifdef CONFIG_ARCH_MSM
#ifdef MSM_PCIE_LINKDOWN_RECOVERY
	if (bus->islinkdown)
		options = MSM_PCIE_CONFIG_NO_CFG_RESTORE | MSM_PCIE_CONFIG_LINKDOWN;
#endif /* MSM_PCIE_LINKDOWN_RECOVERY */

	ret = msm_pcie_pm_control(MSM_PCIE_SUSPEND, bus->dev->bus->number,
		bus->dev, NULL, options);

	if (ret) {
		DHD_ERROR(("Failed to stop PCIe link\n"));
	}
#endif /* CONFIG_ARCH_MSM */
	DHD_TRACE(("%s Exit:\n", __FUNCTION__));
	return ret;
}

int
dhdpcie_disable_device(dhd_bus_t *bus)
{
	if (bus == NULL)
		return BCME_ERROR;

	if (bus->dev == NULL)
		return BCME_ERROR;

	pci_disable_device(bus->dev);

	return 0;
}

int
dhdpcie_enable_device(dhd_bus_t *bus)
{
	int ret = BCME_ERROR;
	dhdpcie_info_t *pch;

	DHD_TRACE(("%s Enter:\n", __FUNCTION__));

	if(bus == NULL)
		return BCME_ERROR;

	if(bus->dev == NULL)
		return BCME_ERROR;

	pch = pci_get_drvdata(bus->dev);
	if(pch == NULL)
		return BCME_ERROR;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0))
	if (pci_load_saved_state(bus->dev, pch->state))
		pci_disable_device(bus->dev);
	else {
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0) */
		pci_restore_state(bus->dev);
		ret = pci_enable_device(bus->dev);
		if(!ret)
			pci_set_master(bus->dev);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0))
	}
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0) */

	if(ret)
		pci_disable_device(bus->dev);

	return ret;
}

