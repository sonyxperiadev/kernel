/*
 * linux/drivers/video/backlight/tps61158_pwm_bl.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

 #include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/backlight.h>
#include <linux/err.h>
#include <linux/tps61158_pwm_bl.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/broadcom/lcd.h>
#include <linux/spinlock.h>
#include <linux/broadcom/PowerManager.h>
#include <linux/rtc.h>

int current_intensity;

static int backlight_mode=1;

#define DIMMING_VALUE		5
#define MAX_BRIGHTNESS_VALUE	255
#define MIN_BRIGHTNESS_VALUE	20
#define BACKLIGHT_DEBUG 1
#define BACKLIGHT_SUSPEND 0
#define BACKLIGHT_RESUME 1

#if BACKLIGHT_DEBUG
#define BLDBG(fmt, args...) printk(fmt, ## args)
#else
#define BLDBG(fmt, args...)
#endif

struct tps61158_bl_data {
	struct platform_device *pdev;
	unsigned int ctrl_pin;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend_desc;
#endif
};

struct brt_value{
	int level;				// Platform setting values
	int tune_level;			// Chip Setting values
};

#if defined (CONFIG_MACH_RHEA_SS_LUCAS)
struct brt_value brt_table[] = {
   { MIN_BRIGHTNESS_VALUE,  32 }, // Min pulse 32
   { 32,  31 },
   { 46,  30 },
   { 60,  29 },  
   { 73,  28 }, 
   { 86,  27 }, 
   { 98,  26 }, 
   { 105,  25 }, 
   { 110,  24 }, 
   { 115,  23 }, 
   { 120,  22 }, 
   { 125,  21 }, 
   { 130,  20 }, 
   { 140,  19 },//default value  
   { 155,  18 },   
   { 165,  17 },
   { 176,  16 }, 
   { 191,  15 }, 
   { 207,  14 },
   { 214,  13 },
   { 221,  12 },
   { 228,  10 },
   { 235,  8 },
   { 242,  7 },
   { 249,  5 },
   { MAX_BRIGHTNESS_VALUE,  5 }, // Max pulse 1
};
#else
struct brt_value brt_table[] = {
   { 20,  5 }, 
   { 25,  8 },
   { 30,  12 },
   { 35,  15 },  
   { 40,  19 }, 
   { 45,  23 }, 
   { 50,  27 },
   { 55,  30 }, 
   { 60,  34 },
   { 65,  37 }, 
   { 70,  41 },
   { 75,  45 },
   { 80,  49 }, 
   { 85,  53 }, 
   { 90,  56 }, 
   { 95,  58 }, 
   { 100,  61 },   
   { 105,  64 },
   { 110,  67 }, 
   { 115,  70 }, 
   { 120,  73 },
   { 125,  76 },
   { 130,  79 },
   { 135,  82 },
   { 140,  85 },
   { 145,  87 },  
   { 150,  91 },
   { 155,  95 },
   { 160,  99 }, /* default 160 */
   { 165,  104 },
   { 170,  109 },
   { 175,  114 },
   { 180,  118 },
   { 185,  122 }, 
   { 190,  125 },
   { 195,  129 },
   { 200,  133 },
   { 205,  137 },
   { 210,  142 },
   { 215,  146 },
   { 220,  150 },
   { 225,  155 },
   { 230,  160 },
   { 235,  165 },
   { 240,  170 },
   { 245,  175 },
   { 250,  180 },
   { 255,  190 }, 
};
#endif

#define MAX_BRT_STAGE (int)(sizeof(brt_table)/sizeof(struct brt_value))

extern void panel_write(u8 *buff);

static u8 set_bl_seq[] = {
	2, 0x51, 0xFF, 0x00 /* The Last 0x00 : Sequence End Mark */
};

void backlight_control(int brigtness)
{
	set_bl_seq[2] = brigtness;
	panel_write(set_bl_seq);
}


/* input: intensity in percentage 0% - 100% */
static int tps61158_backlight_update_status(struct backlight_device *bd)
{
	int user_intensity = bd->props.brightness;
 	int tune_level = 0;
	int i;
  
	BLDBG("[BACKLIGHT] tps61158_backlight_update_status ==> user_intensity  : %d\n", user_intensity);
		
	if (bd->props.power != FB_BLANK_UNBLANK)
		user_intensity = 0;

	if (bd->props.fb_blank != FB_BLANK_UNBLANK)
		user_intensity = 0;

	if (bd->props.state & BL_CORE_SUSPENDED)
		user_intensity = 0;
		
	if(backlight_mode != BACKLIGHT_RESUME)
	{
		BLDBG("[BACKLIGHT] Returned with invalid backlight mode %d\n", backlight_mode);
		return 0;
	}

	if(user_intensity > 0) {
		if(user_intensity < MIN_BRIGHTNESS_VALUE) {
			tune_level = DIMMING_VALUE; //DIMMING
		} else if (user_intensity == MAX_BRIGHTNESS_VALUE) {
			tune_level = brt_table[MAX_BRT_STAGE-1].tune_level;
		} else {
			for(i = 0; i < MAX_BRT_STAGE; i++) {
				if(user_intensity <= brt_table[i].level ) {
					tune_level = brt_table[i].tune_level;
					break;
				}
			}
		}
	}
		
	BLDBG("[BACKLIGHT] tps61158_backlight_update_status ==> tune_level : %d\n", tune_level);
	
	backlight_control(tune_level);
	
	return 0;
}

static int tps61158_backlight_get_brightness(struct backlight_device *bl)
{
	BLDBG("[BACKLIGHT] tps61158_backlight_get_brightness\n");
    
	return current_intensity;
}

static struct backlight_ops tps61158_backlight_ops = {
	.update_status	= tps61158_backlight_update_status,
	.get_brightness	= tps61158_backlight_get_brightness,
};

#ifdef CONFIG_HAS_EARLYSUSPEND
static void tps61158_backlight_earlysuspend(struct early_suspend *desc)
{
	struct timespec ts;
	struct rtc_time tm;
	
	backlight_mode=BACKLIGHT_SUSPEND; 

	getnstimeofday(&ts);
	rtc_time_to_tm(ts.tv_sec, &tm);
	printk("[%02d:%02d:%02d.%03lu][BACKLIGHT] earlysuspend\n", tm.tm_hour, tm.tm_min, tm.tm_sec, ts.tv_nsec);
}

static void tps61158_backlight_earlyresume(struct early_suspend *desc)
{
	struct tps61158_bl_data *tps61158 = container_of(desc, struct tps61158_bl_data, early_suspend_desc);
	struct backlight_device *bl = platform_get_drvdata(tps61158->pdev);
	struct timespec ts;
	struct rtc_time tm;
	
	getnstimeofday(&ts);
	rtc_time_to_tm(ts.tv_sec, &tm);
	backlight_mode = BACKLIGHT_RESUME;
	printk("[%02d:%02d:%02d.%03lu][BACKLIGHT] earlyresume\n", tm.tm_hour, tm.tm_min, tm.tm_sec, ts.tv_nsec);

	backlight_update_status(bl);
}
#else
#ifdef CONFIG_PM
static int tps61158_backlight_suspend(struct platform_device *pdev,
					pm_message_t state)
{
	struct backlight_device *bl = platform_get_drvdata(pdev);
	struct tps61158_bl_data *tps61158 = dev_get_drvdata(&bl->dev);
	  
	BLDBG("[BACKLIGHT] tps61158_backlight_suspend\n");
	      
	return 0;
}
static int tps61158_backlight_resume(struct platform_device *pdev)
{
	struct backlight_device *bl = platform_get_drvdata(pdev);
  BLDBG("[BACKLIGHT] tps61158_backlight_resume\n");
	    
	backlight_update_status(bl);
	    
	return 0;
}
#else
#define tps61158_backlight_suspend  NULL
#define tps61158_backlight_resume   NULL
#endif /* CONFIG_PM */
#endif /* CONFIG_HAS_EARLYSUSPEND */

static int tps61158_backlight_probe(struct platform_device *pdev)
{
	struct platform_tps61158_backlight_data *data = pdev->dev.platform_data;
	struct backlight_device *bl;
	struct tps61158_bl_data *tps61158;
	struct backlight_properties props;
	int ret;
	
	BLDBG("[BACKLIGHT] tps61158_backlight_probe\n");
	
	if (!data) {
		dev_err(&pdev->dev, "failed to find platform data\n");
		return -EINVAL;
	}
	
	tps61158 = kzalloc(sizeof(*tps61158), GFP_KERNEL);
	if (!tps61158) {
		dev_err(&pdev->dev, "no memory for state\n");
		ret = -ENOMEM;
		goto err_alloc;
	}
	
	//tps61158->ctrl_pin = data->ctrl_pin;
    
	memset(&props, 0, sizeof(struct backlight_properties));
	props.max_brightness = data->max_brightness;
	props.type = BACKLIGHT_PLATFORM;
	bl = backlight_device_register(pdev->name, &pdev->dev,
			tps61158, &tps61158_backlight_ops, &props);
	if (IS_ERR(bl)) {
		dev_err(&pdev->dev, "failed to register backlight\n");
		ret = PTR_ERR(bl);
		goto err_bl;
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	tps61158->pdev = pdev;
	tps61158->early_suspend_desc.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
	tps61158->early_suspend_desc.suspend = tps61158_backlight_earlysuspend;
	tps61158->early_suspend_desc.resume = tps61158_backlight_earlyresume;
	register_early_suspend(&tps61158->early_suspend_desc);
#endif

	bl->props.max_brightness = data->max_brightness;
	bl->props.brightness = data->dft_brightness;
	platform_set_drvdata(pdev, bl);
  //tps61158_backlight_update_status(bl);
    
	return 0;
	
err_bl:
	kfree(tps61158);
err_alloc:
	return ret;
}

static int tps61158_backlight_remove(struct platform_device *pdev)
{
	struct backlight_device *bl = platform_get_drvdata(pdev);
	struct tps61158_bl_data *tps61158 = dev_get_drvdata(&bl->dev);

	backlight_device_unregister(bl);

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&tps61158->early_suspend_desc);
#endif

	kfree(tps61158);

	return 0;
}

static int tps61158_backlight_shutdown(struct platform_device *pdev)
{
	printk("[BACKLIGHT] tps61158_backlight_shutdown\n");
	return 0;
}

static struct platform_driver tps61158_backlight_driver = {
	.driver		= {
		.name	= "panel",
		.owner	= THIS_MODULE,
	},
	.probe		= tps61158_backlight_probe,
	.remove		= tps61158_backlight_remove,
	.shutdown      = tps61158_backlight_shutdown,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend        = tps61158_backlight_suspend,
	.resume         = tps61158_backlight_resume,
#endif
};

static int __init tps61158_backlight_init(void)
{
	return platform_driver_register(&tps61158_backlight_driver);
}

module_init(tps61158_backlight_init);

static void __exit tps61158_backlight_exit(void)
{
	platform_driver_unregister(&tps61158_backlight_driver);
}

module_exit(tps61158_backlight_exit);
MODULE_DESCRIPTION("tps61158 based Backlight Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:tps61158-backlight");

