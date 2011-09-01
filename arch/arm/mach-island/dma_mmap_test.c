/*****************************************************************************
* Copyright 2010 - 2011 Broadcom Corporation.  All rights reserved.
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
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/dma-mapping.h>
#include <linux/mm.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <asm/atomic.h>

#include <mach/sdma.h>
#include <mach/dma_mmap.h>

#define TIMEOUT_TIME        (msecs_to_jiffies(500))

#define MODULE_NAME         "DMA_MMAP_TEST"
#define MAX_PROC_BUF_SIZE   256
#define PROC_PARENT_DIR     "dma_mmap/test"
#define PROC_ENTRY_START    "start"
#define PROC_ENTRY_SETUP    "setup"

enum dma_t
{
   DMA_TYPE_SDMA, /* ARM system DMA */
   DMA_TYPE_DMA, /* Synopsys DMA */
};

enum mem_t
{
   MEM_TYPE_DMA = 0,
   MEM_TYPE_KMALLOC,
   MEM_TYPE_VMALLOC,
   MEM_TYPE_MAX,
};

struct test_cfg
{
   volatile int in_use;
   volatile int kill_thread;
   volatile int setup_is_done;

   struct task_struct *thread;

   volatile int dir;
   volatile enum dma_t dma_type;
   volatile enum mem_t mem_type;

   DMA_MMAP_CFG_T mmap_cfg;

   DMA_Device_t device;
   SDMA_Handle_t sdma_hdl;
   DMA_Handle_t dma_hdl;

   struct completion dma_done;

   void *src_ptr;
   dma_addr_t src_addr;
   enum mem_t src_type;

   void *dst_ptr;
   dma_addr_t dst_addr;
   enum mem_t dst_type;

   size_t len;
};

struct proc_dir
{
   struct proc_dir_entry *parent_dir;
};

static struct proc_dir gProc;
static struct test_cfg gCfg;

static void *alloc_mem(dma_addr_t *addr, size_t len, enum mem_t type)
{
   void *buf;

   if (type >= MEM_TYPE_MAX || len == 0)
      return NULL;

   switch (type)
   {
      case MEM_TYPE_DMA:
         buf = dma_alloc_writecombine(NULL, len, addr, GFP_KERNEL);
         if (buf == NULL)
         {
            printk(KERN_ERR "%s: dma_alloc_writecombine failed\n", __func__);
            return NULL;
         }
         break;

      case MEM_TYPE_KMALLOC:
         buf = kmalloc(len, GFP_KERNEL);
         if (buf == NULL)
         {
            printk(KERN_ERR "%s: kmalloc failed\n", __func__);
            return NULL;
         }
         break;

      case MEM_TYPE_VMALLOC:
         buf = vmalloc(len);
         if (buf == NULL)
         {
            printk(KERN_ERR "%s: vmalloc failed\n", __func__);
            return NULL;
         }
         break;

      default:
         return NULL;
   }
   return buf;
}

static void free_mem(void *buf, dma_addr_t addr, size_t len, enum mem_t type)
{
   if (buf == NULL || type >= MEM_TYPE_MAX || len == 0)
      return;

   switch (type)
   {
      case MEM_TYPE_DMA:
         dma_free_writecombine(NULL, len, buf, addr);
         break;

      case MEM_TYPE_KMALLOC:
         kfree(buf);
         break;

      case MEM_TYPE_VMALLOC:
         vfree(buf);
         break;

      default:
         return;
   }
}

static void dma_isr(DMA_Device_t dev, int reason, void *data)
{
   struct test_cfg *cfg = (struct test_cfg *)data;
   
   complete(&cfg->dma_done);
}

static int set_dev_handler(struct test_cfg *cfg)
{
   int rc;

   if (cfg->dma_type == DMA_TYPE_SDMA)
      rc = sdma_set_device_handler(cfg->device, dma_isr, (void *)cfg);
   else {
	printk(KERN_ERR "%s: only support SDMA for now\n", __func__);
	return -EINVAL;
   }

   if (rc < 0)
   {
      printk(KERN_ERR "%s: failed to set DMA handler\n", __func__);
      return rc;
   }

   return 0;
}

static unsigned char cnt = 0;

/*
 * Kernel thread that processes the update requests
 */
static void test_thread(void)
{
   struct test_cfg *cfg = &gCfg;
   unsigned int i;
   int rc;
   void *virt_addr;
   enum dma_data_direction dir;
   dma_addr_t devPhysAddr;
   unsigned long time_left;
   unsigned long trials = 0;

   if (!cfg->setup_is_done)
   {
      printk(KERN_ERR "Need to set up parameters before running the test\n");
      return;
   }

   cfg->kill_thread = 0;
   cfg->in_use = 1;

   /* allocate memory for dstination and source */
   cfg->src_ptr = alloc_mem(&cfg->src_addr, cfg->len, cfg->src_type);
   if (cfg->src_ptr == NULL)
   {
      printk(KERN_ERR "alloc_mem for src failed\n");
      goto exit_free_mem;
   }

   cfg->dst_ptr = alloc_mem(&cfg->dst_addr, cfg->len, cfg->dst_type);
   if (cfg->dst_ptr == NULL)
   {
      printk(KERN_ERR "alloc_mem for dst failed\n");
      goto exit_free_mem;
   }

   /* init mmap */
   rc = dma_mmap_init_map(&cfg->mmap_cfg);
   if (rc < 0)
   {
      printk(KERN_ERR "dma_mmap_init_map failed\n");
      goto exit_free_mem;
   }

   if (cfg->dir) /* mem-to-dev, map src memory */
   {
      virt_addr = cfg->src_ptr;
      dir = DMA_TO_DEVICE;
      devPhysAddr = cfg->dst_addr;
   }
   else /* dev-to-mem, map dst memory */
   {
      virt_addr = cfg->dst_ptr;
      dir = DMA_FROM_DEVICE;
      devPhysAddr = cfg->src_addr;
   }

   daemonize(MODULE_NAME);
   allow_signal(SIGKILL);
   
   for (;;)
   {
      /* driver shutting down... let's quit the kthread */
      if (cfg->kill_thread)
         goto exit_term_mmap;

      if (signal_pending(current))
         goto exit_term_mmap;

      /* write some values into the source buffer */
      for (i = 0; i < cfg->len; i++)
      {
         ((unsigned char *)cfg->src_ptr)[i] = cnt++;
      }

      /* clear the destination buffer */
      memset(cfg->dst_ptr, 0, cfg->len);

      /* map memories */
      rc = dma_mmap_map(&cfg->mmap_cfg, virt_addr, cfg->len, dir);
      if (rc < 0)
      {
         printk(KERN_ERR "dma_mmap_map failed\n");
         goto exit_term_mmap;
      }

      /* reserve the DMA channel and set up descriptors */
      if (cfg->dma_type == DMA_TYPE_SDMA)
      {
         cfg->sdma_hdl = sdma_request_channel(cfg->device);
         if (cfg->sdma_hdl < 0)
         {
            printk(KERN_ERR "sdma_request_channel failed\n");
            goto exit_unmap;
         }

         rc = sdma_map_create_descriptor_ring(cfg->sdma_hdl, &cfg->mmap_cfg,
                                           devPhysAddr, DMA_UPDATE_MODE_INC);
         if (rc < 0)
         {
            printk(KERN_ERR "create desc ring failed\n");
            goto exit_free_dma_channel;
         }
      }
      else
      {
	 printk(KERN_ERR "only support SDMA for now\n");
	 rc = -EINVAL;
	 goto exit_unmap;
#if 0
         cfg->dma_hdl = dma_request_channel(cfg->device);
         if (cfg->dma_hdl < 0)
         {
            printk(KERN_ERR "dma_request_channel failed\n");
            goto exit_unmap;
         }

         rc = dma_map_create_descriptor_ring(cfg->device, &cfg->mmap_cfg,
                                          devPhysAddr, DMA_UPDATE_MODE_INC);
         if (rc < 0)
         {
            printk(KERN_ERR "create desc ring failed\n");
            goto exit_free_dma_channel;
         }
#endif
      }

      /* set DMA interrupt handler */
      rc = set_dev_handler(cfg);
      if (rc < 0)
      {
         printk(KERN_ERR "set_dev_handler failed\n");
         goto exit_free_dma_channel;
      }

      INIT_COMPLETION(cfg->dma_done);

      if (cfg->dma_type == DMA_TYPE_SDMA)
      {
         rc = sdma_start_transfer(cfg->sdma_hdl);
         if (rc < 0)
         {
            printk(KERN_ERR "sdma_transfer failed\n");
            goto exit_free_dma_channel;
         }
      }
#if 0
      else /* synopsys DMA */
      {
         rc = dma_start_transfer(cfg->dma_hdl, cfg->len);
         if (rc < 0)
         {
            printk(KERN_ERR "dma_start_transfer failed\n");
            goto exit_free_dma_channel;
         }
      }
#endif

      time_left = wait_for_completion_timeout(&cfg->dma_done, TIMEOUT_TIME);
      if (time_left == 0)
      {
         printk(KERN_ERR "DMA MMAP test timeout after %lu trials\n", trials);
         goto exit_free_dma_channel;
      }

      /* free DMA channel and unmap memory */
      if (cfg->dma_type == DMA_TYPE_SDMA)
         sdma_free_channel(cfg->sdma_hdl);
#if 0
      else
         dma_free_channel(cfg->dma_hdl);
#endif

      dma_mmap_unmap(&cfg->mmap_cfg, 0);

      /* verify the result */
      for (i = 0; i < cfg->len; i++)
      {
         if (((unsigned char *)cfg->src_ptr)[i] !=
             ((unsigned char *)cfg->dst_ptr)[i])
         {
            printk(KERN_ERR "src[%u]=%u != dst[%u]=%u trials=%lu\n",
                  i, ((unsigned char *)cfg->src_ptr)[i],
                  i, ((unsigned char *)cfg->dst_ptr)[i],
                  trials);
            goto exit_term_mmap;
         }
      }

      trials++;
   }

exit_free_dma_channel:
   if (cfg->dma_type == DMA_TYPE_SDMA)
      sdma_free_channel(cfg->sdma_hdl);
#if 0
   else
      dma_free_channel(cfg->dma_hdl);
#endif

exit_unmap:
   dma_mmap_unmap(&cfg->mmap_cfg, 0);

exit_term_mmap:
   dma_mmap_term_map(&cfg->mmap_cfg);

exit_free_mem:
   if (cfg->src_ptr)
   {
      free_mem(cfg->src_ptr, cfg->src_addr, cfg->len, cfg->src_type);
      cfg->src_ptr = NULL;
   }

   if (cfg->dst_ptr)
   {
      free_mem(cfg->dst_ptr, cfg->dst_addr, cfg->len, cfg->dst_type);
      cfg->dst_ptr = NULL;
   }

   cfg->thread = NULL;
   cfg->in_use = 0;
   cfg->kill_thread = 0;

   printk(KERN_INFO "Quitted test thread\n");
}

static int
proc_start_write(struct file *file, const char __user *buffer,
		unsigned long count, void *data)
{
   int rc, start;
   struct test_cfg *cfg = &gCfg;
   unsigned char kbuf[MAX_PROC_BUF_SIZE];

   if (count > MAX_PROC_BUF_SIZE)
      count = MAX_PROC_BUF_SIZE;

   rc = copy_from_user(kbuf, buffer, count);
   if (rc) {
      printk(KERN_ERR "copy_from_user failed status=%d", rc);
      return -EFAULT;
   }

   if (sscanf(kbuf, "%d", &start) != 1) {
      printk(KERN_ERR "echo <start> > /proc/%s/%s\n",
            PROC_PARENT_DIR, PROC_ENTRY_START);
      return count;
   }

   if (start)
   {
      if (cfg->in_use)
      {
         printk(KERN_INFO "Test thread is already running\n");
      }
      else /* not already in use */
      {
         if (!cfg->setup_is_done)
         {
            printk(KERN_ERR "Need to set up the memory before starting the test thread\n");
         }
         else
         {
            cfg->thread = kthread_run((void *)test_thread, NULL, MODULE_NAME);
            if (IS_ERR(cfg->thread))
            {
               printk(KERN_ERR "kthread_run failed\n");
            }
         }
      }
   }
   else /* disable */
   {
      if (cfg->in_use)
      {
         cfg->kill_thread = 1;
      }
   }

   return count;
}

static int
proc_start_read(char *buffer, char **start, off_t off, int count,
		int *eof, void *data)
{
   unsigned int len = 0;
   struct test_cfg *cfg = &gCfg;

   if (off > 0)
      return 0;

   len += sprintf(buffer + len, "DMA MMAP Test Kthread is %s\n",
         (cfg->in_use) ? "RUNNING" : "NOT RUNNING");

   return len;
}

static int
proc_setup_write(struct file *file, const char __user *buffer,
		unsigned long count, void *data)
{
   int rc;
   struct test_cfg *cfg = &gCfg;
   int dma_type, mem_type, dir;
   unsigned int bytes;
   unsigned char kbuf[MAX_PROC_BUF_SIZE];

   if (count > MAX_PROC_BUF_SIZE)
      count = MAX_PROC_BUF_SIZE;

   if (cfg->in_use)
   {
      printk(KERN_ERR "Test thread is runnig. Please stop it first before setup\n");
      return count;
   }

   rc = copy_from_user(kbuf, buffer, count);
   if (rc) {
      printk(KERN_ERR "copy_from_user failed status=%d", rc);
      return -EFAULT;
   }

   if (sscanf(kbuf, "%d %d %d %u", &dma_type, &mem_type, &dir, &bytes) != 4) {
      printk(KERN_ERR "echo <dma_type> <mem_type> <dir> <bytes> > /proc/%s/%s\n",
            PROC_PARENT_DIR, PROC_ENTRY_SETUP);
      return count;
   }

   cfg->dma_type = dma_type;
   cfg->mem_type = mem_type;
   cfg->dir = dir;
   if (cfg->dir) /* mem-to-dev */
   {
      cfg->src_type = mem_type;
      cfg->dst_type = MEM_TYPE_DMA;
   }
   else /* dev-to-mem */
   {
      cfg->src_type = MEM_TYPE_DMA;
      cfg->dst_type = mem_type;
   }
   cfg->len = bytes;
   cfg->setup_is_done = 1;

   return count;
}

static int
proc_setup_read(char *buffer, char **start, off_t off, int count,
		int *eof, void *data)
{
   unsigned int len = 0;
   struct test_cfg *cfg = &gCfg;

   if (off > 0)
      return 0;

   len += sprintf(buffer + len, "dma_type=%d mem_type=%d dir=%d len=%u\n",
         cfg->dma_type, cfg->mem_type, cfg->dir, cfg->len);

   return len;
}

static int proc_init(void)
{
   int rc;
   struct proc_dir_entry *proc_start;
   struct proc_dir_entry *proc_setup;

   gProc.parent_dir = proc_mkdir(PROC_PARENT_DIR, NULL);

   proc_start = create_proc_entry(PROC_ENTRY_START, 0644, gProc.parent_dir);
   if (proc_start == NULL) {
      rc = -ENOMEM;
      goto err_exit;
   }
   proc_start->read_proc = proc_start_read;
   proc_start->write_proc = proc_start_write;
   proc_start->data = NULL;

   proc_setup = create_proc_entry(PROC_ENTRY_SETUP, 0644, gProc.parent_dir);
   if (proc_setup == NULL) {
      rc = -ENOMEM;
      goto err_del_start;
   }
   proc_setup->read_proc = proc_setup_read;
   proc_setup->write_proc = proc_setup_write;
   proc_setup->data = NULL;

   return 0;

err_del_start:
   remove_proc_entry(PROC_ENTRY_START, gProc.parent_dir);

err_exit:
   remove_proc_entry(PROC_PARENT_DIR, NULL);
   return rc;
}

static void proc_term(void)
{
   remove_proc_entry(PROC_ENTRY_SETUP, gProc.parent_dir);
   remove_proc_entry(PROC_ENTRY_START, gProc.parent_dir);
   remove_proc_entry(PROC_PARENT_DIR, NULL);
}

static int __init test_init(void)
{
   int rc;
   struct test_cfg *cfg = &gCfg;

   memset(cfg, 0, sizeof(*cfg));

   cfg->device = DMA_DEVICE_MEM_TO_MEM;
   init_completion(&cfg->dma_done);

   rc = proc_init();
   if (rc < 0)
   {
      printk(KERN_ERR "DMA MMAP test proc_init failed\n");
      return rc;
   }
   
   return 0;
}

static void __exit test_exit(void)
{
   struct test_cfg *cfg = &gCfg;

   proc_term();

   if (cfg->in_use)
   {
      int cnt = 0;
      cfg->kill_thread = 1;
      while (cfg->in_use)
      {
         msleep(100);
         if (++cnt > 5)
         {
            printk(KERN_ERR "Unable to kill kthread\n");
            break;
         }
      }
   }

   return;
}

module_init(test_init);
module_exit(test_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Broadcom DMA Memory Map Test");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
