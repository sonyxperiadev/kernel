/*****************************************************************************
* Copyright 2009 Broadcom Corporation.  All rights reserved.
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



/**
*
*  @file    gist_test.c
*
*  @brief   Generic I/O Streaming tester.
*
****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/module.h>
#include <linux/version.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/broadcom/knllog.h>

#include <linux/broadcom/gist.h>

/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */
static char gBanner[] __initdata = KERN_INFO "BROADCOM Generic I/O Streaming Tester\n";

/* ---- Private Variables ------------------------------------------------ */

static int reader_handle = 0;
static int writer_handle = 0;
struct timer_list timer;

/* ---- Private Function Prototypes -------------------------------------- */
/* ---- Functions -------------------------------------------------------- */

static char buffer[500];
static void timer_func(unsigned long data)
{
   int rc;

   if (reader_handle && writer_handle)
   {
      rc = gist_read(reader_handle, 500, buffer);
      if (rc > 0)
      {
         rc = gist_write(writer_handle, rc, buffer);

         timer.expires = jiffies + 1;
         add_timer(&timer);
      }
   }

}

/****************************************************************************
*
*  gist_test_init
*
*     Called to perform module initialization when the module is loaded
*
***************************************************************************/
int __init gist_test_init(void)
{
   int rc = 0;
   GIST_READER_PARAMS reader_params;
   GIST_WRITER_PARAMS writer_params;

   printk(gBanner);

   strcpy(reader_params.filename, "/tmp/input.txt");
   reader_params.mode = GIST_READ_MODE_ONCE;
   reader_params.buffer_size = GIST_BUFFER_SIZE_USE_DEFAULT;

   rc = gist_alloc_reader(&reader_params);
   printk("gist_alloc_reader(%s)=%d\n", reader_params.filename, rc);
   reader_handle = rc;


   strcpy(writer_params.filename, "/tmp/output.txt");
   writer_params.mode = GIST_WRITE_MODE_TRUNCATE;
   writer_params.buffer_size = GIST_BUFFER_SIZE_USE_DEFAULT;

   rc = gist_alloc_writer(&writer_params);
   printk("gist_alloc_writer(%s)=%d\n", writer_params.filename, rc);
   writer_handle = rc;

   {

      init_timer(&timer);

      timer.data = 0;
      timer.function = timer_func;
      timer.expires = jiffies + 1;

      add_timer(&timer);



   }
   return 0;
}

/****************************************************************************
*
*  gist_test_exit
*
*       Called to perform module cleanup when the module is unloaded.
*
***************************************************************************/

void __exit gist_test_exit(void)
{
   int temp;
   if (reader_handle)
   {
      temp = reader_handle;
      reader_handle = 0;
      gist_free_reader(temp);
   }
   if (writer_handle)
   {
      temp = writer_handle;
      writer_handle = 0;
      gist_free_writer(temp);
   }
   del_timer(&timer);
}



module_init(gist_test_init);
module_exit(gist_test_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Broadcom Generic I/O Streaming Utility");
MODULE_LICENSE("GPL v2");

