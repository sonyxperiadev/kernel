/*****************************************************************************
 * Copyright 2011 Broadcom Corporation.  All rights reserved.
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
 * Platform device driver to control TFT Panel
 */
 
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/broadcom/tft_panel.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif /* CONFIG_HAS_EARLYSUSPEND */

/* pointer to platform data */
static struct tft_panel_platform_data *g_plt_data;

static void close_gpios(struct tft_panel_platform_data *plt_data)
{
    if (plt_data->gpio_lcd_pwr_en >= 0)
        gpio_free(plt_data->gpio_lcd_pwr_en);
    if (plt_data->gpio_lcd_reset >= 0)
        gpio_free(plt_data->gpio_lcd_reset);
    if (plt_data->gpio_bl_en >= 0)
        gpio_free(plt_data->gpio_bl_en);
    if (plt_data->gpio_bl_pwr_en >= 0)
        gpio_free(plt_data->gpio_bl_pwr_en);
    if (plt_data->gpio_bl_pwm >= 0)
        gpio_free(plt_data->gpio_bl_pwm);
}

static int setup_gpios(struct tft_panel_platform_data *plt_data)
{
    int rc;
   
    if (plt_data->gpio_lcd_pwr_en >= 0) {
        rc = gpio_request_one(plt_data->gpio_lcd_pwr_en, 
                              GPIOF_OUT_INIT_HIGH, "LCD Power Enable");
        printk(KERN_INFO "%s() LCD Power Enable GPIO requested: %d, status: %d\n", 
               __FUNCTION__, plt_data->gpio_lcd_pwr_en, rc);

        if (rc) {
            close_gpios(plt_data);
            return rc;
        }   
    }
    if (plt_data->gpio_lcd_reset >= 0) {
        rc = gpio_request_one(plt_data->gpio_lcd_reset, 
                              GPIOF_OUT_INIT_HIGH, "LCD Reset");
        printk(KERN_INFO "%s() LCD Reset GPIO requested: %d, status: %d\n", 
               __FUNCTION__, plt_data->gpio_lcd_reset, rc);
        if (rc) {
            close_gpios(plt_data);
            return rc;
        }   
    }
    if (plt_data->gpio_bl_en >= 0) {
        rc = gpio_request_one(plt_data->gpio_bl_en, 
                              GPIOF_OUT_INIT_HIGH, "Backlight Enable");
        printk(KERN_INFO "%s() Backlight Enable GPIO requested: %d, status: %d\n", 
               __FUNCTION__, plt_data->gpio_bl_en, rc);
        if (rc) {
            close_gpios(plt_data);
            return rc;
        }   
    }
    if (plt_data->gpio_bl_pwr_en >= 0) {
        rc = gpio_request_one(plt_data->gpio_bl_pwr_en, 
                              GPIOF_OUT_INIT_HIGH, "Backlight Power Enable");
        printk(KERN_INFO "%s() Backlight Power Enable requested: %d, status: %d\n", 
               __FUNCTION__, plt_data->gpio_bl_pwr_en, rc);
        if (rc) {
            close_gpios(plt_data);
            return rc;
        }   
    }
    if (plt_data->gpio_bl_pwm >= 0) {
        rc = gpio_request_one(plt_data->gpio_bl_pwm, 
                              GPIOF_OUT_INIT_HIGH, "Backlight PWM");
        printk(KERN_INFO "%s() Backlight PWM GPIO requested: %d, status: %d\n", 
               __FUNCTION__, plt_data->gpio_bl_pwm, rc);
        if (rc) {
            close_gpios(plt_data);
            return rc;
        }   
    }

    return 0;
}

static void power_off(struct tft_panel_platform_data *plt_data)
{
    gpio_set_value(plt_data->gpio_lcd_pwr_en, 0);
    gpio_set_value(plt_data->gpio_lcd_reset, 0);
    gpio_set_value(plt_data->gpio_bl_en, 0);
    gpio_set_value(plt_data->gpio_bl_pwr_en, 0);
    gpio_set_value(plt_data->gpio_bl_pwm, 0);
}

static void power_on(struct tft_panel_platform_data *plt_data)
{
    gpio_set_value(plt_data->gpio_lcd_pwr_en, 1);
    gpio_set_value(plt_data->gpio_lcd_reset, 1);
    gpio_set_value(plt_data->gpio_bl_en, 1);
    gpio_set_value(plt_data->gpio_bl_pwr_en, 1);
    gpio_set_value(plt_data->gpio_bl_pwm, 1);
}

#ifdef CONFIG_PM
static int tft_panel_suspend(struct platform_device *p_dev, pm_message_t state)
{
    power_off(g_plt_data);
    return 0;
}

static int tft_panel_resume(struct platform_device *p_dev)
{
    power_on(g_plt_data);
    return 0;
}
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static void tft_panel_early_suspend(struct early_suspend *h)
{
    power_off(g_plt_data);
}

static void tft_panel_late_resume(struct early_suspend *h)
{
    power_on(g_plt_data);
}

/* we want early suspend handler to be called after
   EARLY_SUSPEND_LEVEL_BLANK_SCREEN handler is called, so we increase priority
   by 5 */ 
static struct early_suspend tft_panel_early_suspend_desc = {
    .level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 5,
    .suspend = tft_panel_early_suspend,
    .resume = tft_panel_late_resume,
};

#endif /* CONFIG_HAS_EARLYSUSPEND */

static int __devinit tft_panel_probe(struct platform_device *plt_dev)
{
    int ret;

    printk(KERN_INFO "Broadcom TFT Panel Control Driver");

    if (plt_dev->dev.platform_data == NULL) {  
        printk(KERN_ERR "%s() platform data is not present\n", __FUNCTION__);
        return -ENODATA;
    }
   
    /* store pointer to platform data */
    g_plt_data = (struct tft_panel_platform_data *)plt_dev->dev.platform_data;

    /* set up GPIOs */
    if ((ret = setup_gpios(g_plt_data)) != 0){
        printk(KERN_ERR "%s() setup_gpios() returned %d\n", __FUNCTION__, ret);
        return ret;
    }

#ifdef CONFIG_HAS_EARLYSUSPEND
    register_early_suspend(&tft_panel_early_suspend_desc);
    printk(KERN_INFO "%s() register for early suspend\n", __FUNCTION__);
#endif /* CONFIG_HAS_EARLYSUSPEND */

    printk(KERN_INFO "%s() TFT Panel Control Driver probed\n", __FUNCTION__);
    return 0;
}

static int __devexit tft_panel_remove(struct platform_device *plt_dev)
{
    /* close GPIOs */
    close_gpios(g_plt_data);
    return 0;
}

static struct platform_driver tft_panel_driver = 
{
    .driver.name = TFT_PANEL_DRIVER_NAME,
    .driver.owner = THIS_MODULE,
    .probe = tft_panel_probe,
    .remove = __devexit_p(tft_panel_remove),
#ifdef CONFIG_PM
    .suspend = tft_panel_suspend,
    .resume = tft_panel_resume,
#endif
};

static int __init tft_panel_init(void)
{
    int ret;

    ret = platform_driver_register(&tft_panel_driver);
    if (ret) 
    {
        printk(KERN_ERR "%s(): platform_driver_register failed %d\n",
               __FUNCTION__, ret);
        return ret;
    }
    return 0;
}

static void __exit tft_panel_exit(void)
{
    platform_driver_unregister(&tft_panel_driver);
}

module_init(tft_panel_init);
module_exit(tft_panel_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Broadcom TFT Panel Control Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
