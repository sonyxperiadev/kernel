 /*
 * S6E63M0 AMOLED LCD panel driver.
 *
 * Author: InKi Dae  <inki.dae@samsung.com>
 * Modified by JuneSok Lee <junesok.lee@samsung.com>
 *
 * Derived from drivers/video/broadcom/lcd/smart_dimming_s6e63m0_dsi.c
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/err.h>
#include <linux/lcd.h>
#include <linux/backlight.h>
#include <linux/mutex.h>
#include <linux/fb.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/wait.h>
#include <video/kona_fb.h>
#include <linux/broadcom/mobcom_types.h>
#include "dispdrv_common.h"

#undef dev_dbg
#define dev_dbg dev_info

#define MAX_BRIGHTNESS		255
#define DEFAULT_BRIGHTNESS		190
#define DEFAULT_GAMMA_LEVEL	17 /*190cd*/

#define ELVSS_SET_START_IDX 2
#define ELVSS_SET_END_IDX 5

#define ID_VALUE_M2			0xA4
#define ID_VALUE_SM2			0xB4
#define ID_VALUE_SM2_1			0xB6

//#define ESD_OPERATION
//#define ESD_TEST
#ifdef ESD_OPERATION
#define ESD_PORT_NUM 88
#endif

extern char *get_seq(DISPCTRL_REC_T *rec);
extern void panel_write(UInt8 *buff);
extern void panel_read(UInt8 reg, UInt8 *rxBuff, UInt8 buffLen);

extern struct device *lcd_dev;

struct sc7798_dsi_lcd {
	struct device	*dev;
	struct mutex	lock;
	unsigned int	current_brightness;
	unsigned int	bl;	
	bool			panel_awake;			
	u8*			lcd_id;	
#ifdef ESD_OPERATION
	unsigned int	lcd_connected;
	unsigned int	esd_enable;
	unsigned int	esd_port;
	struct workqueue_struct	*esd_workqueue;
	struct work_struct	esd_work;
	bool	esd_processing; 
#endif
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend	earlysuspend;
#endif
#ifdef ESD_TEST
	struct timer_list               esd_test_timer;
#endif
};

#define PANEL_ID_MAX		3
struct sc7798_dsi_lcd *lcd = NULL;
u8 gPanelID[PANEL_ID_MAX];


void panel_read_id(void)
{
	u8 *pPanelID = gPanelID;	
	
	pr_info("%s\n", __func__);	

	/* Read panel ID*/
	panel_read(0xDA, pPanelID, 1); 
	printk("[LCD] gPanelID1 = 0x%02X\n", pPanelID[0]);
	panel_read(0xDB, pPanelID+1, 1);
	printk("[LCD] gPanelID2 = 0x%02X\n", pPanelID[1]);
	panel_read(0xDC, pPanelID+2, 1);	
	printk("[LCD] gPanelID3 = 0x%02X\n", pPanelID[2]);
}
EXPORT_SYMBOL(panel_read_id);

void panel_initialize(char *init_seq)
{
	u8 *pPanelID = gPanelID;
	
	pr_info("%s\n", __func__);	

	panel_write(init_seq);	

	//panel_read_id();
}
EXPORT_SYMBOL(panel_initialize);

static ssize_t show_lcd_info(struct device *dev, struct device_attribute *attr, char *buf)
{
	char temp[20];
	sprintf(temp, "SDC_AMS397GE84\n");
	strcat(buf, temp);
	return strlen(buf);
}

static DEVICE_ATTR(lcd_type, 0444, show_lcd_info, NULL);


#ifdef ESD_OPERATION

#ifdef ESD_TEST
static void esd_test_timer_func(unsigned long data)
{
	pr_info("%s\n", __func__);

	if (list_empty(&(lcd->esd_work.entry))) {
		disable_irq_nosync(gpio_to_irq(lcd->esd_port));
		queue_work(lcd->esd_workqueue, &(lcd->esd_work));
		pr_info("%s invoked\n", __func__);
	}

	mod_timer(&lcd->esd_test_timer,  jiffies + (30*HZ));
}
#endif

void sc7798_esd_recovery(struct s6e63m0_dsi_lcd *lcd)
{
	pr_info("%s\n", __func__);	
	
	/* To Do : LCD Reset & Initialize */
}
static void esd_work_func(struct work_struct *work)
{
	struct sc7798_dsi_lcd *lcd = container_of(work, struct sc7798_dsi_lcd, esd_work);
	
	pr_info(" %s \n", __func__);
       
	if (lcd->esd_enable && !lcd->esd_processing/* && !battpwroff_charging*/) {
		pr_info("ESD PORT =[%d]\n", gpio_get_value(lcd->esd_port));
		
		lcd->esd_processing = true; 
		
		mutex_lock(&lcd->lock);		
		sc7798_esd_recovery(lcd);
		mutex_unlock(&lcd->lock);			
		
		msleep(100);
		
    lcd->esd_processing = false; 
		
		/* low is normal. On PBA esd_port coule be HIGH */
		if (gpio_get_value(lcd->esd_port)) 
			pr_info(" %s esd_work_func re-armed\n", __func__);

#ifdef ESD_TEST			
		enable_irq(gpio_to_irq(lcd->esd_port));			
#endif
	}
}

static irqreturn_t esd_interrupt_handler(int irq, void *data)
{
	struct sc7798_dsi_lcd *lcd = data;
	
	dev_dbg(lcd->dev,"lcd->esd_enable :%d\n", lcd->esd_enable);
	
	if (lcd->esd_enable && !lcd->esd_processing/* && !battpwroff_charging*/) {
		if (list_empty(&(lcd->esd_work.entry)))
			queue_work(lcd->esd_workqueue, &(lcd->esd_work));
		else
			dev_dbg(lcd->dev,"esd_work_func is not empty\n" );
	}
	
	return IRQ_HANDLED;
}
#endif	/*ESD_OPERATION*/

#ifdef CONFIG_HAS_EARLYSUSPEND
static void sc7798_dsi_early_suspend(struct early_suspend *earlysuspend)
{
  int ret;
  struct sc7798_dsi_lcd *lcd = container_of(earlysuspend, struct sc7798_dsi_lcd, earlysuspend);

  pr_info(" %s function entered\n", __func__);

#ifdef ESD_OPERATION
	if (lcd->esd_enable) {
		lcd->esd_enable = 0;
		irq_set_irq_type(gpio_to_irq(lcd->esd_port), IRQF_TRIGGER_NONE);
		disable_irq_nosync(gpio_to_irq(lcd->esd_port));

		if (!list_empty(&(lcd->esd_work.entry))) {
			cancel_work_sync(&(lcd->esd_work));
			dev_dbg(lcd->dev,"cancel esd works\n");
		}

		dev_dbg(lcd->dev,"disable esd operation\n", lcd->esd_enable);
	}
#endif		  

	if (lcd->panel_awake) {
			lcd->panel_awake = false;		
			dev_dbg(lcd->dev, "panel goes to sleep\n");	
	}	
}

static void sc7798_dsi_late_resume(struct early_suspend *earlysuspend)
{
  struct sc7798_dsi_lcd *lcd = container_of(earlysuspend, struct sc7798_dsi_lcd, earlysuspend);

  pr_info(" %s function entered\n", __func__);

	if (!lcd->panel_awake) {
			lcd->panel_awake = true;
			dev_dbg(lcd->dev, "panel is ready\n");			
	}

	mutex_lock(&lcd->lock);	
	
	/* To Do : Resume LCD Device */
	
	mutex_unlock(&lcd->lock);			
	
#ifdef ESD_OPERATION
	if (lcd->lcd_connected) {
		enable_irq(gpio_to_irq(lcd->esd_port));
		irq_set_irq_type(gpio_to_irq(lcd->esd_port), IRQF_TRIGGER_RISING);
		lcd->esd_enable = 1;
		dev_dbg(lcd->dev, "change lcd->esd_enable :%d\n",lcd->esd_enable);
	}
#endif	
}
#endif

static int sc7798_panel_probe(struct platform_device *pdev)
{
	int ret = 0;

	lcd = kzalloc(sizeof(struct sc7798_dsi_lcd), GFP_KERNEL);
	if (!lcd)
		return -ENOMEM;

	lcd->dev = &pdev->dev;
	lcd->bl = DEFAULT_GAMMA_LEVEL;
	lcd->lcd_id = gPanelID;
	lcd->current_brightness	= 255;
	lcd->panel_awake = true;	

	dev_info(lcd->dev, "%s function entered\n", __func__);			
	
	{
		int n = 0;

		dev_info(lcd->dev, "panelID : [0x%02X], [0x%02X], [0x%02X]\n", gPanelID[0], gPanelID[1], gPanelID[2]);		

	}	
	
	platform_set_drvdata(pdev, lcd);	

	mutex_init(&lcd->lock);

#ifdef CONFIG_LCD_CLASS_DEVICE
	ret = device_create_file(lcd_dev, &dev_attr_lcd_type);
	if (ret < 0)
		printk("Failed to add lcd_type sysfs entries, %d\n",	__LINE__);		
#endif			
	
#ifdef ESD_OPERATION
	lcd->esd_workqueue = create_singlethread_workqueue("esd_workqueue");
	if (!lcd->esd_workqueue) {
		dev_info(lcd->dev, "esd_workqueue create fail\n");
		return 0;
	}
	
	INIT_WORK(&(lcd->esd_work), esd_work_func);
	
	lcd->esd_port = ESD_PORT_NUM;
	
	if (request_threaded_irq(gpio_to_irq(lcd->esd_port), NULL,
			esd_interrupt_handler, IRQF_TRIGGER_RISING, "esd_interrupt", lcd)) {
			dev_info(lcd->dev, "esd irq request fail\n");
			free_irq(gpio_to_irq(lcd->esd_port), NULL);
			lcd->lcd_connected = 0;
	}
	    
#ifdef ESD_TEST
	setup_timer(&lcd->esd_test_timer, esd_test_timer_func, 0);
	mod_timer(&lcd->esd_test_timer,  jiffies + (30*HZ));
#endif

	lcd->esd_processing = false; 
	lcd->lcd_connected = 1;	
	lcd->esd_enable = 1;		
#endif	
	
#ifdef CONFIG_HAS_EARLYSUSPEND
  lcd->earlysuspend.level   = EARLY_SUSPEND_LEVEL_DISABLE_FB - 1;
  lcd->earlysuspend.suspend = sc7798_dsi_early_suspend;
  lcd->earlysuspend.resume  = sc7798_dsi_late_resume;
  register_early_suspend(&lcd->earlysuspend);
#endif	

	return 0;
}

static int sc7798_panel_remove(struct platform_device *pdev)
{
	struct sc7798_dsi_lcd *lcd = platform_get_drvdata(pdev);

	dev_dbg(lcd->dev, "%s function entered\n", __func__);

#ifdef CONFIG_HAS_EARLYSUSPEND
  unregister_early_suspend(&lcd->earlysuspend);
#endif

	kfree(lcd);

	return 0;
}

static struct platform_driver sc7798_panel_driver = {
	.driver		= {
		.name	= "SC7798",
		.owner	= THIS_MODULE,
	},
	.probe		= sc7798_panel_probe,
	.remove		= sc7798_panel_remove,
};
static int __init sc7798_panel_init(void)
{
	return platform_driver_register(&sc7798_panel_driver);
}

static void __exit sc7798_panel_exit(void)
{
	platform_driver_unregister(&sc7798_panel_driver);
}

late_initcall_sync(sc7798_panel_init);
module_exit(sc7798_panel_exit);

MODULE_DESCRIPTION("sc7798 panel control driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:sc7798");
