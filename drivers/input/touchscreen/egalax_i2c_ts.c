/*
 *
 * Touch Screen I2C Driver for EETI Controller
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/freezer.h>
#include <linux/proc_fs.h>
#include <linux/clk.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <linux/poll.h>
#include <linux/kfifo.h>
#include <linux/version.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <linux/i2c/egalax_i2c_ts.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif /* CONFIG_HAS_EARLYSUSPEND */

#define MAX_I2C_LEN          10
#define MAX_SUPPORT_POINT    4
#define REPORTID_MOUSE       0x01
#define REPORTID_VENDOR      0x03
#define REPORTID_MTOUCH      0x04

#ifdef CONFIG_TOUCHSCREEN_SCALE

#define X_ORIG 1366
#define Y_ORIG 768
static volatile int g_x_new = X_ORIG;
static volatile int g_y_new = Y_ORIG;

#define X_ORIG_OFF(x_new)  ((X_ORIG - (x_new)) / 2)
#define Y_ORIG_OFF(y_new)  ((Y_ORIG - (y_new)) / 2)

#define X_OFF(x_new)  ((X_ORIG_OFF(x_new) * 2048) / X_ORIG)
#define Y_OFF(y_new)  ((Y_ORIG_OFF(y_new) * 2048) / Y_ORIG)

#define X_SCALED(x, x_new)  ((x_new) ? ((((x) - X_OFF(x_new)) * X_ORIG) / (x_new)) : (x))
#define Y_SCALED(y, y_new)  ((y_new) ? ((((y) - Y_OFF(y_new)) * Y_ORIG) / (y_new)) : (y))

#define X_MIN(x_new)  X_OFF(x_new)
#define X_MAX(x_new)  (((X_ORIG_OFF(x_new) + (x_new)) * 2048) / X_ORIG)

#define Y_MIN(y_new)  Y_OFF(y_new)
#define Y_MAX(y_new)  (((Y_ORIG_OFF(y_new) + (y_new)) * 2048) / Y_ORIG)

#endif /* CONFIG_TOUCHSCREEN_SCALE */

#ifdef CONFIG_CP_TS_SOFTKEY
#define BTN_HOME         2
#define BTN_BACKK        1
#define BTN_MENU         3
#define BTN_SEARCH       0
#define MAX_BTN          4
#define BTN_REPORT_MOD   5

static unsigned short BtnState[MAX_BTN];
static struct input_dev *softkey_input_dev;
#endif /* CONFIG_CP_TS_SOFTKEY */

#define MAX_PROC_BUF_SIZE         256
#define PROC_PARENT_DIR           "touchscreen"
#define PROC_ENTRY_DEBUG          "debug"
#define PROC_ENTRY_CMD            "cmd"
#define PROC_ENTRY_WAKEUP         "wakeup"
#define PROC_ENTRY_X_SCALE        "x-scale"
#define PROC_ENTRY_Y_SCALE        "y-scale"

static const u8 cmd_str_query_firmware[MAX_I2C_LEN]={0x03, 0x03, 0x0A, 0x01, 0x44, 0, 0, 0, 0, 0};
static const u8 cmd_str_idle[MAX_I2C_LEN]={0x03, 0x06, 0x0A, 0x04, 0x36, 0x3F, 0x01, 0x05, 0, 0};
static const u8 cmd_str_sleep[MAX_I2C_LEN]={0x03, 0x05, 0x0A, 0x03, 0x36, 0x3F, 0x02, 0, 0, 0};

/* flag to turn on debug prints */
static volatile int gDbg = 0;
#define TS_DEBUG(format, args...) \
   do { if (gDbg) printk(KERN_WARNING "[egalax_i2c]: " format, ## args); } while (0)
#define DBG() \
   do { if (gDbg) printk("[%s]:%d => \n", __FUNCTION__, __LINE__); } while (0)

#define TS_ERR(fmt, args...)   printk(KERN_ERR "[egalax_i2c]: " fmt, ## args)
#define TS_INFO(fmt, args...)  printk(KERN_INFO "[egalax_i2c]: " fmt, ## args)

struct proc_dir
{
   struct proc_dir_entry *parent;
};

struct point_data
{
   short Status;
   short X;
   short Y;
};

struct _egalax_i2c {
   struct workqueue_struct *ktouch_wq;
   struct work_struct work;
   struct mutex mutex_wq;
   struct i2c_client *client;
   struct egalax_i2c_ts_cfg hw_cfg;
#if defined(CONFIG_PM) || defined(CONFIG_HAS_EARLYSUSPEND)
   atomic_t is_suspended;
#endif
   struct proc_dir proc;
};

static struct input_dev *input_dev;
static struct _egalax_i2c *p_egalax_i2c_dev;
static struct point_data PointBuf[MAX_SUPPORT_POINT];

static int wakeup_controller(int gpio)
{
   gpio_direction_output(gpio, 0);
   msleep(250);
   gpio_direction_input(gpio);

   TS_INFO("controller wake up done\n");

   return 0;
}

#ifdef CONFIG_CP_TS_SOFTKEY
static void report_softkey(int status, int btn_id)
{
   int keyValue = 0;

   switch (btn_id)
   {
      case BTN_HOME: 
         keyValue = KEY_HOME;
         break;
      case BTN_BACKK:
         keyValue = KEY_BACK;
         break;
      case BTN_MENU: 
         keyValue = KEY_MENU;
         break;
      case BTN_SEARCH: 
         keyValue = KEY_SEARCH;
         break;	
      default:
         return;	
   }
   TS_DEBUG("report soft button key:0x%02x status:%d\n", keyValue, status);
   input_report_key(softkey_input_dev, keyValue, status);
   input_sync(softkey_input_dev);
}
#endif

static int LastUpdateID = 0;
static void ProcessReport(unsigned char *buf, int buflen)
{
   int i;
   short X = 0, Y = 0, ContactID = 0, Status = 0;

   if (buflen != MAX_I2C_LEN || buf[0] != REPORTID_MTOUCH)
   {
      TS_ERR("I2C incorrect buflen=%d\n", buflen);
      return;
   }

   Status = buf[1] & 0x01;
   ContactID = (buf[1] & 0x7C) >> 2;
   X = ((buf[3] << 8) + buf[2]) >> 4;
   Y = ((buf[5] << 8) + buf[4]) >> 4;

   /* TODO: error if contactID > MAX_SUPPORT_POINT */
   if (ContactID >= MAX_SUPPORT_POINT)
   {
      TS_ERR("Invalid contact id = %d\n", ContactID);
      return;
   }

#ifdef CONFIG_TOUCHSCREEN_SCALE
   if ( !((g_x_new == X_ORIG) && (g_y_new = Y_ORIG)) && !((g_x_new == 0) && (g_y_new == 0)) )
   {
      if (X < X_MIN(g_x_new) || X > X_MAX(g_x_new) || Y < Y_MIN(g_y_new) || Y > Y_MAX(g_y_new))
         return;
      X = X_SCALED(X, g_x_new);
      Y = Y_SCALED(Y, g_y_new);
   }
#endif

   PointBuf[ContactID].Status = Status;
   PointBuf[ContactID].X = X;
   PointBuf[ContactID].Y = Y;

   TS_DEBUG("got touch point[%d]: status=%d X=%d Y=%d\n", ContactID, Status, X, Y);

   if (!Status || (ContactID <= LastUpdateID))
   {
      for (i = 0; i < MAX_SUPPORT_POINT; i++)
      {
         if (PointBuf[i].Status > 0)
         {
            input_report_abs(input_dev, ABS_MT_TRACKING_ID, i);         
            input_report_abs(input_dev, ABS_MT_TOUCH_MAJOR, PointBuf[i].Status);
            input_report_abs(input_dev, ABS_MT_WIDTH_MAJOR, 0);
            input_report_abs(input_dev, ABS_MT_POSITION_X, PointBuf[i].X);
            input_report_abs(input_dev, ABS_MT_POSITION_Y, PointBuf[i].Y);
            TS_DEBUG("input sync point data [%d]!\n", i);
         }
         input_mt_sync(input_dev);

         if (PointBuf[i].Status == 0)
            PointBuf[i].Status--;
      }
      input_sync(input_dev);
      TS_DEBUG("input sync point data done!\n");
   }

   LastUpdateID = ContactID;
}

static int egalax_i2c_measure(struct i2c_client *client)
{
   u8 x_buf[MAX_I2C_LEN];
   int count, loop = 3;
#ifdef CONFIG_CP_TS_SOFTKEY
   int btn_id, btn_state;
#endif
   
   DBG();

   do
   {
      count = i2c_master_recv(client, x_buf, MAX_I2C_LEN);
   } while (count == -EAGAIN && --loop);

   if (count < 0 || (x_buf[0] != REPORTID_VENDOR && x_buf[0] != REPORTID_MTOUCH))
   {
      /*
       * TODO: Ignore invalid data for now w/o printing it to the console until
       * this gets sorted out with EETI/Compal
       */
      //TS_ERR("I2C read error data with Len=%d header=%d\n", count, x_buf[0]);
      return -EFAULT;
   }

   TS_DEBUG("read data with len=%d\n", count);

   if (x_buf[0] == REPORTID_VENDOR)
   {
      TS_DEBUG("received command packet: "
             "0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
              x_buf[0], x_buf[1], x_buf[2], x_buf[3], x_buf[4],
              x_buf[5], x_buf[6], x_buf[7], x_buf[8], x_buf[9]);
   }

   if (count > 0 && x_buf[0] == REPORTID_MTOUCH)
   {
      ProcessReport(x_buf, count);
      return count;
   }

#ifdef CONFIG_CP_TS_SOFTKEY
   if (count > 0 && x_buf[0] == REPORTID_VENDOR && x_buf[1]==6 && x_buf[4]==0x36 && x_buf[5]==0x2F) 
   {
      TS_DEBUG("got virtual_key report: id=%d state=%d\n", x_buf[7], x_buf[6]);
      btn_state = x_buf[6] & 0x01;
      btn_id = x_buf[7];
      if (btn_id >= 0 && btn_id < MAX_BTN)
      {
         if (btn_state == 0 && BtnState[btn_id])
         {
            report_softkey(btn_state, btn_id);
            memset(BtnState, 0, sizeof(short) * MAX_BTN);
         }
         else if (btn_state)
         {
            if ((++BtnState[btn_id]) % BTN_REPORT_MOD == 0)
               report_softkey(btn_state, btn_id);
         }
      }
      return count;
   }
#endif

   return count;
}

static void egalax_i2c_wq(struct work_struct *work)
{
   struct _egalax_i2c *egalax_i2c = container_of(work, struct _egalax_i2c, work);
   struct i2c_client *client = egalax_i2c->client;
   int gpio = irq_to_gpio(client->irq);

   mutex_lock(&egalax_i2c->mutex_wq);
   TS_DEBUG("egalax_i2c_wq run\n");

   /* continue recv data while GPIO is pulled low */
   while (!gpio_get_value(gpio))
   {
      egalax_i2c_measure(client);
      schedule();
   }

   TS_DEBUG("egalax_i2c_wq leave\n");
   mutex_unlock(&egalax_i2c->mutex_wq);
}

static irqreturn_t egalax_i2c_interrupt(int irq, void *dev_id)
{
   struct _egalax_i2c *egalax_i2c = (struct _egalax_i2c *)dev_id;

   TS_DEBUG("egalax_i2c_interrupt with irq:%d\n", irq);

   /* postpone I2C transactions to the workqueue as it may block */
   queue_work(egalax_i2c->ktouch_wq, &egalax_i2c->work);

   return IRQ_HANDLED;
}

static int
proc_debug_write(struct file *file, const char __user *buffer,
      unsigned long count, void *data)
{
   int rc;
   unsigned int debug;
   unsigned char kbuf[MAX_PROC_BUF_SIZE];

   if (count > MAX_PROC_BUF_SIZE)
      count = MAX_PROC_BUF_SIZE;

   rc = copy_from_user(kbuf, buffer, count);
   if (rc)
   {
      printk(KERN_ERR "copy_from_user failed status=%d", rc);
      return -EFAULT;
   }

   if (sscanf(kbuf, "%u", &debug) != 1)
   {
      printk(KERN_ERR "echo <debug> > /proc/%s/%s\n",
            PROC_PARENT_DIR, PROC_ENTRY_DEBUG);
      return count;
   }

   if (debug)
      gDbg = 1;
   else
      gDbg = 0;

   return count;
}

static int
proc_debug_read(char *buffer, char **start, off_t off, int count,
      int *eof, void *data)
{
   unsigned int len = 0;

   if (off > 0)
      return 0;

   len += sprintf(buffer + len, "Debug print is %s\n", gDbg ? "enabled" : "disabled");

   return len;
}

static int
proc_cmd_write(struct file *file, const char __user *buffer,
      unsigned long count, void *data)
{
   int rc;
   unsigned int cmd;
   u8 idle_interval, cmdbuf[MAX_I2C_LEN];
   unsigned char kbuf[MAX_PROC_BUF_SIZE];
   unsigned char cmd_str[3][10] = { "query", "idle", "sleep" };

   if (count > MAX_PROC_BUF_SIZE)
      count = MAX_PROC_BUF_SIZE;

   rc = copy_from_user(kbuf, buffer, count);
   if (rc)
   {
      printk(KERN_ERR "copy_from_user failed status=%d", rc);
      return -EFAULT;
   }

   rc = sscanf(kbuf, "%u %c", &cmd, &idle_interval);
   if (rc != 1 && rc != 2)
   {
      printk(KERN_ERR "echo <0=Query, 1=Idle, 2=Sleep> <idle_interval> > /proc/%s/%s\n",
             PROC_PARENT_DIR, PROC_ENTRY_CMD);
      return count;
   }

   switch (cmd)
   {
      case 0:
         memcpy(cmdbuf, cmd_str_query_firmware, MAX_I2C_LEN);
         break;

      case 1:
         memcpy(cmdbuf, cmd_str_idle, MAX_I2C_LEN);
         if (rc == 2 && idle_interval < 10)
            cmdbuf[7] = idle_interval;
         break;

      case 2:
         memcpy(cmdbuf, cmd_str_sleep, MAX_I2C_LEN);
         break;

      default:
         printk(KERN_ERR "echo <0=Query, 1=Idle, 2=Sleep> <idle_interval> > /proc/%s/%s\n",
             PROC_PARENT_DIR, PROC_ENTRY_CMD);
         return count;
   }

   printk(KERN_INFO "[egalax_i2c] About to send [%s] command\n", cmd_str[cmd]);
   if (cmd == 1)
   {
      printk(KERN_INFO "[egalax_i2c] idle interval is %u ms\n",
            (cmdbuf[7] + 1) * 50);
   }

   if (MAX_I2C_LEN == i2c_master_send(p_egalax_i2c_dev->client, cmdbuf,
            MAX_I2C_LEN))
   {
      printk(KERN_INFO "[egalax_i2c]: Command string "
             "0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x "
             "sent successfully\n",
              cmdbuf[0], cmdbuf[1], cmdbuf[2], cmdbuf[3], cmdbuf[4],
              cmdbuf[5], cmdbuf[6], cmdbuf[7], cmdbuf[8], cmdbuf[9]);
             
   }
   else
      printk(KERN_ERR "[egalax_i2c]: Unable to send command over\n");
   
   return count;
}

static int
proc_wakeup_write(struct file *file, const char __user *buffer,
      unsigned long count, void *data)
{
   int rc;
   unsigned int wakeup;
   unsigned char kbuf[MAX_PROC_BUF_SIZE];

   if (count > MAX_PROC_BUF_SIZE)
      count = MAX_PROC_BUF_SIZE;

   rc = copy_from_user(kbuf, buffer, count);
   if (rc)
   {
      printk(KERN_ERR "copy_from_user failed status=%d", rc);
      return -EFAULT;
   }

   if (sscanf(kbuf, "%u", &wakeup) != 1)
   {
      printk(KERN_ERR "echo <wakeup> > /proc/%s/%s\n",
            PROC_PARENT_DIR, PROC_ENTRY_WAKEUP);
      return count;
   }

   if (wakeup)
      wakeup_controller(p_egalax_i2c_dev->hw_cfg.gpio.event);

   printk(KERN_ERR "[egalax_i2c]: Toggled GPIO %d to wake up the controller\n",
         p_egalax_i2c_dev->hw_cfg.gpio.event);

   return count;
}

#ifdef CONFIG_TOUCHSCREEN_SCALE
static int
proc_x_scale_write(struct file *file, const char __user *buffer,
      unsigned long count, void *data)
{
   int rc;
   unsigned int x_scale;
   unsigned char kbuf[MAX_PROC_BUF_SIZE];

   if (count > MAX_PROC_BUF_SIZE)
      count = MAX_PROC_BUF_SIZE;

   rc = copy_from_user(kbuf, buffer, count);
   if (rc)
   {
      printk(KERN_ERR "copy_from_user failed status=%d", rc);
      return -EFAULT;
   }

   if (sscanf(kbuf, "%u", &x_scale) != 1)
   {
      printk(KERN_ERR "echo <x-scale> > /proc/%s/%s\n",
            PROC_PARENT_DIR, PROC_ENTRY_X_SCALE);
      return count;
   }

   g_x_new = x_scale;

   return count;
}

static int
proc_x_scale_read(char *buffer, char **start, off_t off, int count,
      int *eof, void *data)
{
   unsigned int len = 0;

   if (off > 0)
      return 0;

   len += sprintf( buffer + len,
                   "x-axis %sscaled (scale = %d)\n",
                   ((g_x_new != 0) ? "non" : ""),
                   g_x_new );

   return len;
}

static int
proc_y_scale_write(struct file *file, const char __user *buffer,
      unsigned long count, void *data)
{
   int rc;
   unsigned int y_scale;
   unsigned char kbuf[MAX_PROC_BUF_SIZE];

   if (count > MAX_PROC_BUF_SIZE)
      count = MAX_PROC_BUF_SIZE;

   rc = copy_from_user(kbuf, buffer, count);
   if (rc)
   {
      printk(KERN_ERR "copy_from_user failed status=%d", rc);
      return -EFAULT;
   }

   if (sscanf(kbuf, "%u", &y_scale) != 1)
   {
      printk(KERN_ERR "echo <y-scale> > /proc/%s/%s\n",
            PROC_PARENT_DIR, PROC_ENTRY_Y_SCALE);
      return count;
   }

   g_y_new = y_scale;

   return count;
}

static int
proc_y_scale_read(char *buffer, char **start, off_t off, int count,
      int *eof, void *data)
{
   unsigned int len = 0;

   if (off > 0)
      return 0;

   len += sprintf( buffer + len,
                   "y-axis %sscaled (scale = %d)\n",
                   ((g_y_new != 0) ? "non" : ""),
                   g_y_new );

   return len;
}
#endif

static int proc_init(struct _egalax_i2c *egalax_i2c)
{
   struct proc_dir *proc = &egalax_i2c->proc;
   int rc;
   struct proc_dir_entry *proc_debug, *proc_cmd, *proc_wakeup;
#ifdef CONFIG_TOUCHSCREEN_SCALE
   struct proc_dir_entry *proc_x_scale, *proc_y_scale;
#endif

   proc->parent = proc_mkdir(PROC_PARENT_DIR, NULL);

   proc_debug = create_proc_entry(PROC_ENTRY_DEBUG, 0644, proc->parent);
   if (proc_debug == NULL)
   {
      rc = -ENOMEM;
      goto err_del_parent;
   }
   proc_debug->read_proc = proc_debug_read;
   proc_debug->write_proc = proc_debug_write;
   proc_debug->data = NULL;

   proc_cmd = create_proc_entry(PROC_ENTRY_CMD, 0644, proc->parent);
   if (proc_cmd == NULL)
   {
      rc = -ENOMEM;
      goto err_del_debug;
   }
   proc_cmd->read_proc = NULL;
   proc_cmd->write_proc = proc_cmd_write;
   proc_cmd->data = NULL;

   proc_wakeup = create_proc_entry(PROC_ENTRY_WAKEUP, 0644, proc->parent);
   if (proc_wakeup == NULL)
   {
      rc = -ENOMEM;
      goto err_del_cmd;
   }
   proc_wakeup->read_proc = NULL;
   proc_wakeup->write_proc = proc_wakeup_write;
   proc_wakeup->data = NULL;

#ifdef CONFIG_TOUCHSCREEN_SCALE
   proc_x_scale = create_proc_entry(PROC_ENTRY_X_SCALE, 0644, proc->parent);
   if (proc_x_scale == NULL)
   {
      rc = -ENOMEM;
      goto err_del_wakeup;
   }
   proc_x_scale->read_proc = proc_x_scale_read;
   proc_x_scale->write_proc = proc_x_scale_write;
   proc_x_scale->data = NULL;

   proc_y_scale = create_proc_entry(PROC_ENTRY_Y_SCALE, 0644, proc->parent);
   if (proc_y_scale == NULL)
   {
      rc = -ENOMEM;
      goto err_del_x_scale;
   }
   proc_y_scale->read_proc = proc_y_scale_read;
   proc_y_scale->write_proc = proc_y_scale_write;
   proc_y_scale->data = NULL;
#endif

   return 0;

#ifdef CONFIG_TOUCHSCREEN_SCALE
err_del_x_scale:
   remove_proc_entry(PROC_ENTRY_X_SCALE, proc->parent);
err_del_wakeup:
   remove_proc_entry(PROC_ENTRY_WAKEUP, proc->parent);
#endif
err_del_cmd:
   remove_proc_entry(PROC_ENTRY_CMD, proc->parent);
err_del_debug:
   remove_proc_entry(PROC_ENTRY_DEBUG, proc->parent);
err_del_parent:
   remove_proc_entry(PROC_PARENT_DIR, NULL);
   return rc;
}

static int proc_term(struct _egalax_i2c *egalax_i2c)
{
   struct proc_dir *proc = &egalax_i2c->proc;

#ifdef CONFIG_TOUCHSCREEN_SCALE
   remove_proc_entry(PROC_ENTRY_X_SCALE, proc->parent);
   remove_proc_entry(PROC_ENTRY_Y_SCALE, proc->parent);
#endif
   remove_proc_entry(PROC_ENTRY_WAKEUP, proc->parent);
   remove_proc_entry(PROC_ENTRY_CMD, proc->parent);
   remove_proc_entry(PROC_ENTRY_DEBUG, proc->parent);
   remove_proc_entry(PROC_PARENT_DIR, NULL);
   return 0;
}

#if defined(CONFIG_PM) || defined(CONFIG_HAS_EARLYSUSPEND)
static int device_suspend(struct i2c_client *client)
{
	int ret;
	struct _egalax_i2c *egalax_i2c = i2c_get_clientdata(client);

	/* already suspended */
	if (atomic_read(&egalax_i2c->is_suspended))
		return 0;

	atomic_set(&egalax_i2c->is_suspended, 1);

	/* send the command to put the controller into sleep */
	ret = i2c_master_send(client, cmd_str_sleep, MAX_I2C_LEN);
	if (ret != MAX_I2C_LEN)
	{
		TS_ERR("failed to send sleep command ret=%d\n", ret);
		ret = -EFAULT;
		atomic_set(&egalax_i2c->is_suspended, 0);
		return ret;
	}

	TS_DEBUG("sleep command sent successfully\n");

	/* disable interrupt */
	disable_irq(client->irq);

	/*
	 * flush the workqueue to make sure all outstanding work items are
	 * done
	 */
	flush_workqueue(egalax_i2c->ktouch_wq);

	TS_INFO("device suspended\n");

	return 0;
}

static int device_resume(struct i2c_client *client)
{
	struct _egalax_i2c *egalax_i2c = i2c_get_clientdata(client);

	/* already resumed */
	if (atomic_read(&egalax_i2c->is_suspended) == 0)
		return 0;

	enable_irq(client->irq);
	wakeup_controller(irq_to_gpio(client->irq));

	atomic_set(&egalax_i2c->is_suspended, 0);

	TS_INFO("device resumed\n");

	return 0;
}

#endif

#ifdef CONFIG_PM
static int egalax_i2c_suspend(struct i2c_client *client, pm_message_t mesg)
{
	return device_suspend(client);
}

static int egalax_i2c_resume(struct i2c_client *client)
{
	return device_resume(client);
}
#else
#define egalax_i2c_suspend       NULL
#define egalax_i2c_resume        NULL
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static void egalax_i2c_early_suspend(struct early_suspend *h)
{
	device_suspend(p_egalax_i2c_dev->client);
}

static void egalax_i2c_late_resume(struct early_suspend *h)
{
	device_resume(p_egalax_i2c_dev->client);
}

/* we early suspend handler to be called after EARLY_SUSPEND_LEVEL_BLANK_SCREEN
   handler is called, so we increase priority by 10 */
static struct early_suspend egalax_i2c_early_suspend_desc = {
	.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 10,
	.suspend = egalax_i2c_early_suspend,
	.resume = egalax_i2c_late_resume,
};

#endif /* CONFIG_HAS_EARLYSUSPEND */

static struct input_dev *allocate_Input_Dev(void)
{
   int ret;
   struct input_dev *pInputDev = NULL;

   pInputDev = input_allocate_device();
   if (pInputDev == NULL)
   {
      TS_ERR("Failed to allocate input device\n");
      return NULL;
   }

   pInputDev->name = "eGalax Touch Screen";
   pInputDev->phys = "I2C";
   pInputDev->id.bustype = BUS_I2C;
   pInputDev->id.vendor = 0x0EEF;
   pInputDev->id.product = 0x0020;
   pInputDev->id.version = 0x0000;
   
   set_bit(EV_ABS, pInputDev->evbit);

   input_set_abs_params(pInputDev, ABS_MT_POSITION_X, 0, 2047, 0, 0);
   input_set_abs_params(pInputDev, ABS_MT_POSITION_Y, 0, 2047, 0, 0);
   input_set_abs_params(pInputDev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
   input_set_abs_params(pInputDev, ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);
   input_set_abs_params(pInputDev, ABS_MT_TRACKING_ID, 0, 10, 0, 0);

   ret = input_register_device(pInputDev);
   if (ret) 
   {
      TS_ERR("Unable to register input device\n");
      input_free_device(pInputDev);
      pInputDev = NULL;
      return NULL;
   }

#ifdef CONFIG_CP_TS_SOFTKEY
   softkey_input_dev = input_allocate_device();
   if (softkey_input_dev == NULL)
   {
      TS_ERR("Failed to allocate input device for softkeys\n");
      goto input_done;
   }

   softkey_input_dev->name = "softkey";
   softkey_input_dev->phys = "what?";
   softkey_input_dev->id.bustype = BUS_I2C;
   softkey_input_dev->id.vendor = 0x0001;
   softkey_input_dev->id.product = 0x0002;
   softkey_input_dev->id.version = 0x0100;

   set_bit(EV_KEY, softkey_input_dev->evbit);
   __set_bit(KEY_SEARCH, softkey_input_dev->keybit);
   __set_bit(KEY_MENU, softkey_input_dev->keybit);
   __set_bit(KEY_BACK, softkey_input_dev->keybit);
   __set_bit(KEY_HOME, softkey_input_dev->keybit);

   ret = input_register_device(softkey_input_dev);
   if (ret)
   {
      TS_ERR("Unable to register input device for soft keys\n");
      input_free_device(softkey_input_dev);
      softkey_input_dev = NULL;
      goto input_done;
   }
input_done:
#endif
   
   return pInputDev;
}

static int __devinit egalax_i2c_probe(struct i2c_client *client,
      const struct i2c_device_id *id)
{
   int ret, cnt;
   u8 x_buf[MAX_I2C_LEN]= {0};
   int gpio = irq_to_gpio(client->irq);
   struct egalax_i2c_ts_cfg *cfg;
   
   DBG();

   if (client->dev.platform_data == NULL)
   {
      TS_ERR("Missing board dependent confgiurations\n");
      ret = -EFAULT;
      goto err_exit;
   }

   cfg = (struct egalax_i2c_ts_cfg *)client->dev.platform_data;

   if (cfg->gpio.event < 0)
   {
      TS_ERR("Invalid GPIO: event=%d\n", cfg->gpio.event);
      ret = -EFAULT;
      goto err_exit;
   }

   p_egalax_i2c_dev = (struct _egalax_i2c *)kzalloc(sizeof(struct _egalax_i2c),
         GFP_KERNEL);
   if (!p_egalax_i2c_dev) 
   {
      TS_ERR("Unable to request memory for device\n");
      ret = -ENOMEM;
      goto err_exit;
   }

   /* store hardware/board dependent data */
   memcpy(&p_egalax_i2c_dev->hw_cfg, cfg, sizeof(p_egalax_i2c_dev->hw_cfg));

   p_egalax_i2c_dev->client = client;
   mutex_init(&p_egalax_i2c_dev->mutex_wq);
#if defined(CONFIG_PM) || defined(CONFIG_HAS_EARLYSUSPEND)
   atomic_set(&p_egalax_i2c_dev->is_suspended, 0);
#endif

   p_egalax_i2c_dev->ktouch_wq = create_workqueue("egalax_touch_wq");
   if (p_egalax_i2c_dev->ktouch_wq == NULL)
   {
      TS_ERR("Unable to create workqueue\n");
      ret = -ENOMEM;
      goto err_free_dev;
   }

   INIT_WORK(&p_egalax_i2c_dev->work, egalax_i2c_wq);
   i2c_set_clientdata(client, p_egalax_i2c_dev);

   /* reserve GPIO for touchscreen event interrupt */
   ret = gpio_request(cfg->gpio.event, "egalax i2c ts event");
   if (ret < 0)
   {
      TS_ERR("Unable to request gpio=%d\n", cfg->gpio.event);
      goto err_del_wq;
   }
   gpio_direction_input(cfg->gpio.event);

   /* reserve GPIO for touchscreen controller reset */
   if (cfg->gpio.reset >= 0)
   {
      ret = gpio_request(cfg->gpio.reset, "egalax i2c ts reset");
      if (ret < 0)
      {
         TS_ERR("Unable to request GPIO pin %d\n", cfg->gpio.reset);
         goto err_free_event_gpio;
      }
      gpio_direction_output(cfg->gpio.reset, 1);
   
      /* now reset the touchscreen controller */
      gpio_set_value(cfg->gpio.reset, 1);
      msleep(250);
      gpio_set_value(cfg->gpio.reset, 0);
      msleep(250);
      gpio_set_value(cfg->gpio.reset, 1);
   }
   
   /*
    * initiate an I2C read to put the touchscreen controller into a known
    * state
    */
   {
      int i;

      ret = i2c_master_recv(client, x_buf, MAX_I2C_LEN);
      if (ret > 0)
      {
         printk(KERN_INFO "eGalax I2C touchscreen message:\n");
         for (i = 0; i < MAX_I2C_LEN; i++)
            printk("%2.2x ", x_buf[i]);
         printk("\n");
      }
      else
      {
         TS_ERR("unable to talk to the touchscreen controller\n");
         ret = -ENODEV;
         goto err_free_reset_gpio;
      }
   }

   /* reserve the irq line */
   ret = request_irq(client->irq, egalax_i2c_interrupt, IRQF_TRIGGER_FALLING,
         client->name, p_egalax_i2c_dev);
   if (ret) 
   {
      TS_ERR("request_irq(%d) failed\n", client->irq);
      goto err_free_reset_gpio;
   }

   /* drain the FIFO so the INT line can go back to high */
   cnt = 0;
   while (!gpio_get_value(gpio))
   {
      ret = i2c_master_recv(client, x_buf, MAX_I2C_LEN);
      if (ret < 0)
         cnt++;
      else
         printk(".");

      if (cnt >= 3)
         break;
   }

   printk("\n");

   ret = proc_init(p_egalax_i2c_dev);
   if (ret)
   {
      TS_ERR("proc fs install failed\n");
      goto err_free_irq;
   }

#ifdef CONFIG_HAS_EARLYSUSPEND
   register_early_suspend(&egalax_i2c_early_suspend_desc);
   TS_INFO("register for early suspend\n");
#endif /* CONFIG_HAS_EARLYSUSPEND */

   TS_INFO("eGalax I2C touchscreen driver probed\n");
   TS_INFO("reset=%d event=%d irq=%d\n", cfg->gpio.reset, cfg->gpio.event,
         client->irq);

   return 0;

err_free_irq:
   free_irq(client->irq, p_egalax_i2c_dev);

err_free_reset_gpio:
   if (cfg->gpio.reset >= 0)
      gpio_free(cfg->gpio.reset);

err_free_event_gpio:
   gpio_free(cfg->gpio.event);

err_del_wq:
   i2c_set_clientdata(client, NULL);
   if (p_egalax_i2c_dev->ktouch_wq) 
   {
      destroy_workqueue(p_egalax_i2c_dev->ktouch_wq); 
   }

err_free_dev:
   kfree(p_egalax_i2c_dev);
   p_egalax_i2c_dev = NULL;

err_exit:
   return ret;
}

static int __devexit egalax_i2c_remove(struct i2c_client *client)
{
   struct _egalax_i2c *egalax_i2c = i2c_get_clientdata(client);

   DBG();

   proc_term(egalax_i2c);

   if (p_egalax_i2c_dev->ktouch_wq) 
   {
      destroy_workqueue(p_egalax_i2c_dev->ktouch_wq); 
   }
   
   free_irq(client->irq, egalax_i2c);

   if (egalax_i2c->hw_cfg.gpio.reset >= 0)
      gpio_free(egalax_i2c->hw_cfg.gpio.reset);
   gpio_free(egalax_i2c->hw_cfg.gpio.event);

   i2c_set_clientdata(client, NULL);
   kfree(egalax_i2c);
   p_egalax_i2c_dev = NULL;

#ifdef CONFIG_HAS_EARLYSUSPEND
   unregister_early_suspend(&egalax_i2c_early_suspend_desc);
   TS_INFO("unregister for early suspend\n");
#endif /* CONFIG_HAS_EARLYSUSPEND */
   return 0;
}

static struct i2c_device_id egalax_i2c_idtable[] = { 
   { "egalax_i2c", 0 }, 
   { } 
}; 

MODULE_DEVICE_TABLE(i2c, egalax_i2c_idtable);

static struct i2c_driver egalax_i2c_driver = {
   .driver = {
	   .owner = THIS_MODULE,	
      .name = "egalax_i2c",
   },
   .id_table = egalax_i2c_idtable,
   .class = I2C_CLASS_TOUCHSCREEN,
   .probe = egalax_i2c_probe,
   .remove = __devexit_p(egalax_i2c_remove),
   .suspend = egalax_i2c_suspend,
   .resume = egalax_i2c_resume,
};

static void egalax_i2c_ts_exit(void)
{
   DBG();

   i2c_del_driver(&egalax_i2c_driver);

   if (input_dev)
   {
      input_unregister_device(input_dev);
      input_free_device(input_dev);
      input_dev = NULL;
      TS_INFO("Input device unregistered\n");
   }

#ifdef CONFIG_CP_TS_SOFTKEY
   if (softkey_input_dev)
   {
      input_unregister_device(softkey_input_dev);
      input_free_device(softkey_input_dev);
      softkey_input_dev = NULL;
   }
#endif
}

static int egalax_i2c_ts_init(void)
{
   int result;

   DBG();

   input_dev = allocate_Input_Dev();
   if (input_dev == NULL)
   {
      TS_ERR("allocate_Input_Dev failed\n");
      result = -ENOMEM;
      goto fail;
   }

   TS_INFO("Input device registered\n");

   memset(PointBuf, 0, sizeof(struct point_data) * MAX_SUPPORT_POINT);
#ifdef CONFIG_CP_TS_SOFTKEY
   memset(BtnState, 0, sizeof(short) * MAX_BTN);
#endif

   return i2c_add_driver(&egalax_i2c_driver);

fail:   
   egalax_i2c_ts_exit();
   return result;
}

module_init(egalax_i2c_ts_init);
module_exit(egalax_i2c_ts_exit);

MODULE_DESCRIPTION("egalax touch screen i2c driver");
MODULE_LICENSE("GPL");
