#include <linux/input.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <asm/io.h>
#include <asm/delay.h>

#define MODNAME "dummy_touchscreen"

#define DUMMY_TOUCHSCREEN_ABS_X_MIN	0
#define DUMMY_TOUCHSCREEN_ABS_X_MAX	1023
#define DUMMY_TOUCHSCREEN_ABS_Y_MIN	0
#define DUMMY_TOUCHSCREEN_ABS_Y_MAX	1023

#define INTERVAL (HZ/100)
#warning HZ

static struct input_dev *dummy_touchscreen_dev;
struct timer_list timer1;
char phys[32];

void timer1_routine(unsigned long data)
{
   unsigned touchbits = *(unsigned *)__io_address(ST_BASE+0x18);
   int touched = (touchbits >> 31) & 1;
   int absx = (touchbits >>  0) & DUMMY_TOUCHSCREEN_ABS_X_MAX;
   int absy = (touchbits >> 16) & DUMMY_TOUCHSCREEN_ABS_Y_MAX;
   //static int count;
   //printk(KERN_ERR "DTS: softint %x,%x\n", ST_BASE+0x18, (unsigned)IO_ADDRESS(ST_BASE+0x18));
   //printk(KERN_ERR "DTS: softint %x,%x\n", (unsigned)__io_address(ST_BASE+0x18), *(unsigned *)__io_address(ST_BASE+0x18));
   /*if (count < 20 || (count & 1023) == 0) *///printk(KERN_ERR "DTS: softint %x,%d,%d,%d\n", touchbits,absx,absy,touched);
   if (touched) {
      input_report_key(dummy_touchscreen_dev, BTN_TOUCH, 1);
      input_report_abs(dummy_touchscreen_dev, ABS_X, absx);
      input_report_abs(dummy_touchscreen_dev, ABS_Y, absy);
   } else {
      input_report_key(dummy_touchscreen_dev, BTN_TOUCH, 0);
   }
   input_sync(dummy_touchscreen_dev);
   mod_timer(&timer1, jiffies + INTERVAL); /* restarting timer */
}


static int __init dummy_touchscreen_init(void)
{
   int err;
   printk(KERN_ERR "DTS: init. HZ=%d\n", HZ);
   dummy_touchscreen_dev = input_allocate_device();
   if (!dummy_touchscreen_dev)
      return -ENOMEM;

   dummy_touchscreen_dev->evbit[0] = BIT_MASK(EV_ABS) | BIT_MASK(EV_KEY);
   dummy_touchscreen_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);

   input_set_abs_params(dummy_touchscreen_dev, ABS_X, DUMMY_TOUCHSCREEN_ABS_X_MIN, DUMMY_TOUCHSCREEN_ABS_X_MAX, 0, 0);
   input_set_abs_params(dummy_touchscreen_dev, ABS_Y, DUMMY_TOUCHSCREEN_ABS_Y_MIN, DUMMY_TOUCHSCREEN_ABS_Y_MAX, 0, 0);

   dummy_touchscreen_dev->name = "Broadcom dummy touchscreen";

   //snprintf(phys, sizeof(phys), "%s/input0", dev_name(&client->dev));
   snprintf(phys, sizeof(phys), "%s/input0", "/dev/input"); //dev_name(&client->dev));
   dummy_touchscreen_dev->phys = phys;
   dummy_touchscreen_dev->id.bustype = BUS_RS232;
   dummy_touchscreen_dev->id.vendor = 0x2708;
   dummy_touchscreen_dev->id.product = 0x0dc4;
   dummy_touchscreen_dev->id.version = 0x0100;

   init_timer(&timer1);

   timer1.function = timer1_routine;
   timer1.data = 1;
   timer1.expires = jiffies + HZ; /* 1 second */
   add_timer(&timer1); /* Starting the timer */

   printk(KERN_ALERT"Timer Module loaded\n");
   err = input_register_device(dummy_touchscreen_dev);
   if (err) goto fail;
   printk(KERN_ALERT"DTS: init ok\n");
   return 0;

fail:
   input_free_device(dummy_touchscreen_dev);
   printk(KERN_ALERT"DTS: init error %d\n", err);
   return err;
}

static void __exit dummy_touchscreen_exit(void)
{
   printk(KERN_ERR "DTS: exit\n");
   del_timer_sync(&timer1); /* Deleting the timer */
   input_unregister_device(dummy_touchscreen_dev);
}

module_init(dummy_touchscreen_init);
module_exit(dummy_touchscreen_exit);

MODULE_AUTHOR("Andriy Skulysh, askulysh@image.kiev.ua");
MODULE_DESCRIPTION("Broadcom dummy touchscreen driver");
MODULE_LICENSE("GPL");
