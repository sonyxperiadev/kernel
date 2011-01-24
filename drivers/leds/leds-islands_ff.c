#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <asm/hardware/scoop.h>
#include <asm/mach-types.h>

extern int vc_gencmd(char *response, int maxlen, const char *format, ...);

static void islands_ff_led_lcdbacklight_set(struct led_classdev *led_cdev,
				enum led_brightness value)
{
   char response_buffer[32];
   vc_gencmd(response_buffer, 32,
      	     "set_backlight %i", value);
}

static void islands_ff_led_buttonbacklight_set(struct led_classdev *led_cdev,
				enum led_brightness value)
{
	
}

static void islands_ff_led_keyboardbacklight_set(struct led_classdev *led_cdev,
				enum led_brightness value)
{
	
}

static struct led_classdev islands_ff_lcdbacklight_led = {
	.name			= "lcd-backlight",
	.default_trigger	= "lcd-backlight",
	.brightness_set		= islands_ff_led_lcdbacklight_set,
};

static struct led_classdev islands_ff_buttonbacklight_led = {
	.name			= "button-backlight",
	.default_trigger	= "button-backlight",
	.brightness_set		= islands_ff_led_buttonbacklight_set,
};

static struct led_classdev islands_ff_keyboardbacklight_led = {
	.name			= "keyboard-backlight",
	.default_trigger	= "keyboard-backlight",
	.brightness_set		= islands_ff_led_keyboardbacklight_set,
};

#ifdef CONFIG_PM
static int islands_ff_led_suspend(struct platform_device *dev, pm_message_t state)
{
#ifdef CONFIG_LEDS_TRIGGERS
	if (islands_ff_lcdbacklight_led.trigger && strcmp(islands_ff_lcdbacklight_led.trigger->name,
						"sharpsl-charge"))
#endif
		led_classdev_suspend(&islands_ff_lcdbacklight_led);
	led_classdev_suspend(&islands_ff_buttonbacklight_led);
   led_classdev_suspend(&islands_ff_keyboardbacklight_led);
	return 0;
}

static int islands_ff_led_resume(struct platform_device *dev)
{
	led_classdev_resume(&islands_ff_lcdbacklight_led);
	led_classdev_resume(&islands_ff_buttonbacklight_led);
   led_classdev_resume(&islands_ff_keyboardbacklight_led);
	return 0;
}
#else
#define islands_ff_led_suspend NULL
#define islands_ff_led_resume NULL
#endif

static int islands_ff_led_probe(struct platform_device *pdev)
{
	int ret;

	ret = led_classdev_register(&pdev->dev, &islands_ff_lcdbacklight_led);

	if (ret < 0)
		return ret;

	ret = led_classdev_register(&pdev->dev, &islands_ff_buttonbacklight_led);

	if (ret < 0)
		return ret;

        led_classdev_register(&pdev->dev, &islands_ff_keyboardbacklight_led);

	return ret;
}

static int islands_ff_led_remove(struct platform_device *pdev)
{
	led_classdev_unregister(&islands_ff_lcdbacklight_led);
	led_classdev_unregister(&islands_ff_buttonbacklight_led);
        led_classdev_unregister(&islands_ff_keyboardbacklight_led);

	return 0;
}

static struct platform_driver islands_ff_led_driver = {
	.probe		= islands_ff_led_probe,
	.remove		= islands_ff_led_remove,
	.suspend	= islands_ff_led_suspend,
	.resume		= islands_ff_led_resume,
	.driver		= {
		.name		= "islands_ff-led",
		.owner		= THIS_MODULE,
	},
};

static int __init islands_ff_led_init(void)
{
	return platform_driver_register(&islands_ff_led_driver);
}

static void __exit islands_ff_led_exit(void)
{
 	platform_driver_unregister(&islands_ff_led_driver);
}

module_init(islands_ff_led_init);
module_exit(islands_ff_led_exit);
