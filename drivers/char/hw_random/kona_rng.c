/*****************************************************************************
 * Copyright 2006 - 2011 Broadcom Corporation.  All rights reserved.
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

/*
 * Frameworks:
 *
 *    - SMP:          Not done.
 *    - GPIO:         Fully supported.    No GPIOs are used.
 *    - MMU:          Fully supported.    Platform model with ioremap used.
 *    - Dynamic /dev: Fully supported.    Uses HW random number framework
 *    - Suspend:      Not done.
 *    - Clocks:       Not done.           Awaiting clock framework to be completed.
 *    - Power:        Not done.
 *
 */

/*
 * DESCRIPTION: The BCMHANA random number generator (RNG) driver
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/hw_random.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/io.h>
#include <mach/rdb/brcm_rdb_rng.h>

DEFINE_MUTEX(lock); /* lock for data access */

static atomic_t bus_is_probed;

static void * baseAddr;

static void ioclrbit32(void *addr, unsigned int bits)
{
    iowrite32(ioread32(addr) & ~bits, addr);
}

static void iosetbit32(void *addr, unsigned int bits)
{
    iowrite32(ioread32(addr) | bits,  addr);
}

/**
*  @brief  set warmup cycle
*  @param  none
*  @return none
*  @note
*****************************************************************************/
static inline void chal_rng_inline_warmup(uint32_t cycles)
{
    iowrite32(RNG_STATUS_RNG_WARM_CNT_MASK - (cycles & RNG_STATUS_RNG_WARM_CNT_MASK), baseAddr + RNG_STATUS_OFFSET);
    while ( ( ioread32(baseAddr + RNG_STATUS_OFFSET) & RNG_STATUS_RNG_WARM_CNT_MASK ) != RNG_STATUS_RNG_WARM_CNT_MASK );
}

/**
*  @brief  Stop RNG block
*  @param  none
*  @return none
*  @note
*****************************************************************************/
static inline void chal_rng_inline_enable(void)
{
     iosetbit32(baseAddr + RNG_CTRL_OFFSET, RNG_CTRL_RNG_RBGEN_MASK);
}
/**
*  @brief  Stop RNG block
*  @param  none
*  @return none
*  @note
*****************************************************************************/
static inline void chal_rng_inline_disable(void)
{
    ioclrbit32(baseAddr + RNG_CTRL_OFFSET, RNG_CTRL_RNG_RBGEN_MASK);
}
/**
*  @brief  Enable/Disable RNG RBG2X
*  @param  enable   (in) 1 to enable, 0 to disable
*  @return none
*  @note
*****************************************************************************/
static inline void chal_rng_inline_rbg2x_enable(void)
{
        iosetbit32(baseAddr + RNG_CTRL_OFFSET, RNG_CTRL_RNG_RBG2X_MASK);
}
static inline void chal_rng_inline_rbg2x_disable(void)
{
        ioclrbit32(baseAddr + RNG_CTRL_OFFSET, RNG_CTRL_RNG_RBG2X_MASK);
}
/**
*  @brief  Configure RNG FF THRESH
*  @param  ff_thresh   (in) ff threshold
*  @return none
*  @note
*****************************************************************************/
static inline void chal_rng_inline_ff_thresh(uint8_t ff_thresh)
{
        uint32_t val = ioread32(baseAddr + RNG_FF_THRES_OFFSET);
        val &= ~RNG_FF_THRES_RNG_FF_THRESH_MASK;
        iowrite32(val | ff_thresh, baseAddr + RNG_FF_THRES_OFFSET);
}
/**
*  @brief  Enable/Disable RNG INTERRUPT
*  @param  enable   (in) 1 to enable, 0 to disable
*  @return none
*  @note
*****************************************************************************/
static inline void chal_rng_inline_int_enable(void)
{
        ioclrbit32(baseAddr + RNG_INT_MASK_OFFSET, RNG_INT_MASK_RNG_INT_OFF_MASK);
}
static inline void chal_rng_inline_int_disable(void)
{
        iosetbit32(baseAddr + RNG_INT_MASK_OFFSET, RNG_INT_MASK_RNG_INT_OFF_MASK);
}

/**
*  @brief  RNG get number of valid words available
*  @return Number of words available
*  @note
*****************************************************************************/
static inline uint32_t chal_rng_inline_get_valid_words(void)
{
    return ((ioread32(baseAddr + RNG_STATUS_OFFSET) & RNG_STATUS_RND_VAL_MASK) >> RNG_STATUS_RND_VAL_SHIFT);
}

/**
*  @brief  RNG get value
*  @param  pBuffer (in) Buffer to read data into
*  @param  len (in) bytes of the data to be read
*  @return Number of bytes actually read
*  @note
*****************************************************************************/
static inline uint32_t chal_rng_inline_get_random_number( void )
{
   while ( chal_rng_inline_get_valid_words() == 0 );

   return ioread32(baseAddr + RNG_DATA_OFFSET);
}

/**
 *  @brief  Start RNG block
 *  @param  none
 *  @return none
 *  @note
 *****************************************************************************/
static void chal_rng_inline_start(void)
{
    chal_rng_inline_enable();
    chal_rng_inline_rbg2x_enable();
    chal_rng_inline_int_disable();
    chal_rng_inline_warmup(0xfff);
}

static int rng_data_present(struct hwrng *rng, int wait)
{
    int data, i;

    for (i = 0; i < 20; i++) {
        data = chal_rng_inline_get_valid_words() ? 1 : 0;
        if (data || !wait)
            break;
        /*
         * RNG produces data fast enough.  We *could* use the RNG IRQ, but
         * that'd be higher overhead ... so why bother?
         */
        udelay(10);
    }

    return data;
}

static int rng_data_read(struct hwrng *rng, u32 *data)
{
    if (!atomic_read(&bus_is_probed))
        return -ENODEV;

    /* lock it here since other kernel drivers can access it */
    mutex_lock(&lock);
    *data = chal_rng_inline_get_random_number();
    mutex_unlock(&lock);
    return 4;
}

static struct hwrng rng_ops = {
    .name = "bcmkona",
    .data_present = rng_data_present,
    .data_read = rng_data_read,
};

static int __init rng_probe(struct platform_device *pdev)
{
    struct resource *res;
    int ret;

    /* We only accept one device, and it must have an id of -1 */
    if (pdev->id != -1)
        return -ENODEV;

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res) {
        ret = -ENODEV;
        goto err_out;
    }

    baseAddr = ioremap(res->start, resource_size(res));
    if (!baseAddr) {
        ret = -ENOMEM;
        goto err_out;
    }

    atomic_set(&bus_is_probed, 0);

    chal_rng_inline_start();

    /* register to the Linux RNG framework */
    ret = hwrng_register(&rng_ops);
    if (ret)
        goto err_register;

    printk(KERN_INFO "RNG: Driver initialized\n");

    atomic_set(&bus_is_probed, 1);

    return 0;

err_register:
    iounmap(baseAddr);
err_out:
    return ret;
}

static int __devexit rng_remove(struct platform_device *pdev)
{
    atomic_set(&bus_is_probed, 0);
    hwrng_unregister(&rng_ops);
    return 0;
}

// #ifdef CONFIG_PM
static int rng_suspend(struct platform_device *dev, pm_message_t msg)
{
    chal_rng_inline_disable();
    return 0;
}

static int rng_resume(struct platform_device *dev)
{
    chal_rng_inline_enable();
    return 0;
}
// #else
// #define rng_suspend    NULL
// #define rng_resume     NULL
// #endif

static struct platform_driver rng_driver = {
    .driver = {
        .name = "kona_rng",
        .owner = THIS_MODULE,
    },
    .suspend   = rng_suspend,
    .resume    = rng_resume,
    .remove    = __devexit_p(rng_remove),
};

static char banner[] __initdata = KERN_INFO "Broadcom KONA RNG Driver\n";

static int __init rng_init(void)
{
    printk(banner);
    return platform_driver_probe(&rng_driver, rng_probe);
}

static void __exit rng_exit(void)
{
    platform_driver_unregister(&rng_driver);
}

module_init(rng_init);
module_exit(rng_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("RNG Device Driver");
MODULE_LICENSE("GPL");
