/*****************************************************************************
* Copyright 2006 - 2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

/****************************************************************************/
/**
*  @file    bcmhana_otg_device.c
*
*  @brief   Instantiates an OTG device instance
*
*  @note    This is intended as a temporary solution until DWC OTG driver
*           can support multiple device instances.
*
*/
/****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/sched.h>

#include <mach/lm.h>
#include <mach/irqs.h>
#include <mach/io_map.h>

#include <mach/rdb/brcm_rdb_hsotg_ctrl.h>
#include <mach/rdb/brcm_rdb_hsotg.h>

/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */
/* ---- Private Function Prototypes -------------------------------------- */

#ifdef DEBUG
   #define DBG_PRINT            printk
#else
   #define DBG_PRINT(fmt...)	do {} while (0)
#endif

static void __exit dwc_otg_device_exit(void);
static int __init  dwc_otg_device_init(void);
static int __init  dwc_otg_device_register( unsigned irq, unsigned base_addr );

/* ---- Private Variables ------------------------------------------------ */

static unsigned int fshost = 0;
static unsigned int otghost = 0;
static struct lm_device *lmdev = NULL;


/*-------------------------------------------------------------------------*/

/* ==== Public Functions ================================================= */

/****************************************************************************
 * Module level definitions used to load / unload the DWC OTG Device
 ***************************************************************************/
module_param( fshost, uint, 0644 );
MODULE_PARM_DESC(fshost, "Load FSHOST device instead of HSOTG device");

module_param( otghost, uint, 0644 );
MODULE_PARM_DESC(otghost, "Force OTG host mode - Needed for FPGA v6.n images and later");

MODULE_DESCRIPTION("DWC OTG Device");
MODULE_LICENSE("GPL");
MODULE_VERSION("2.91a");

module_init(dwc_otg_device_init);
module_exit(dwc_otg_device_exit);


/* ==== Private Functions ================================================= */

/****************************************************************************
 *
 ***************************************************************************/
static void __exit dwc_otg_device_exit(void)
{
    if ( lmdev != NULL )
    {
        /* The lmdev memory will get freed as side effect of the
         * lm_device_unregister(), so don't do it here.
         */
        lm_device_unregister(lmdev);
        lmdev = NULL;
    }
}

/****************************************************************************
 *
 ***************************************************************************/
static int __init dwc_otg_device_init(void)
{
    int rc;

    if ( lmdev != NULL )
    {
        rc = -EBUSY;
    }
    else if ( fshost )
    {
        rc = dwc_otg_device_register (BCM_INT_ID_USB_FSHOST, KONA_USB_FSHOST_VA);
    }
    else
    {
        printk("\n%s: Setting up USB OTG PHY and Clock\n", __func__);
        if ( otghost )
        {
            /* Should only be doing this with FPGA images v6.n and later */
            printk(KERN_WARNING "%s: Set HSOTG_CTRL register for host mode\n", __func__);
            *(int*) (KONA_USB_HSOTG_CTRL_VA) =
               HSOTG_CTRL_USBOTGCONTROL_OTGSTAT2_MASK |
               HSOTG_CTRL_USBOTGCONTROL_OTGSTAT1_MASK |
               HSOTG_CTRL_USBOTGCONTROL_REG_OTGSTAT2_MASK |
               HSOTG_CTRL_USBOTGCONTROL_REG_OTGSTAT1_MASK |
               HSOTG_CTRL_USBOTGCONTROL_OTGSTAT_CTRL_MASK |
               HSOTG_CTRL_USBOTGCONTROL_USB_ON_MASK |
               HSOTG_CTRL_USBOTGCONTROL_PRST_N_SW_MASK |
               HSOTG_CTRL_USBOTGCONTROL_HRESET_N_SW_MASK |
               HSOTG_CTRL_USBOTGCONTROL_SOFT_PHY_RESETB_MASK |
               HSOTG_CTRL_USBOTGCONTROL_SOFT_DLDO_PDN_MASK |
               HSOTG_CTRL_USBOTGCONTROL_SOFT_ALDO_PDN_MASK;
        }
        rc = dwc_otg_device_register (BCM_INT_ID_USB_HSOTG, KONA_USB_HSOTG_VA);

        printk("\n%s: Setup USB OTG PHY and Clock Completed\n", __func__);
    }

    return (rc);
}

/****************************************************************************
 *
 ***************************************************************************/
static int __init  dwc_otg_device_register( unsigned irq, unsigned base_addr )
{
    int rc = 0;


    lmdev = kmalloc(sizeof(struct lm_device), GFP_KERNEL);

    if ( lmdev == NULL )
    {
        printk(KERN_ERR "dwc_otg_device_register(): kmalloc() failed\n");
        return (-ENOMEM);
    }

    memset( lmdev, 0 , sizeof(struct lm_device));

    lmdev->id  = -2;
    lmdev->irq = irq;
    lmdev->resource.flags = IORESOURCE_MEM;
    lmdev->resource.start = HW_IO_VIRT_TO_PHYS (base_addr);
    lmdev->resource.end   = lmdev->resource.start + SZ_64K - 1;
    DBG_PRINT(KERN_ERR "dwc_otg_device_register(): irq=%d start=0x%08x end=0x%08x\n", lmdev->irq, lmdev->resource.start, lmdev->resource.end );

    if ( (rc = lm_device_register(lmdev)) < 0 )
    {
        kfree(lmdev);
        lmdev = NULL;
    }

    return (rc);
}
