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
*  @file    gist.c
*
*  @brief   Generic I/O Streaming implementation.
*
****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/module.h>
#include <linux/version.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/completion.h>
#include <linux/jiffies.h>
#include <linux/workqueue.h>
#include <linux/timer.h>
#include <linux/broadcom/knllog.h>
#include <linux/slab.h>

#include <linux/broadcom/gist.h>


/* ---- Public Variables ------------------------------------------------- */

/* ---- Private Constants and Types -------------------------------------- */
#define GIST_DEFAULT_BUFFER_SIZE (20 * 1024)
typedef struct gist_circ_buffer
{
   char *bufp;           /* Buffer base */
   char *bufendp;        /* Buffer end + 1 */
   int   bufsize;        /* Buffer size */
   char *bufwritep;      /* Next write point to buffer */
   char *bufreadp;       /* Next read point from buffer */
} GIST_CIRC_BUFFER;

/* Define threshold at which the workers start reading or writing data to file */
#define GIST_BUFFER_READ_THRESHOLD(bufp) ((bufp)->bufsize >> 1)
#define GIST_BUFFER_WRITE_THRESHOLD(bufp) ((bufp)->bufsize >> 2)

/* Define time delay before workers read/write before threshold is reached */
#define GIST_WORKER_DELAY_MSECS 3000

typedef enum
{
   GIST_READER,
   GIST_WRITER
} GIST_WORKER_TYPE;

typedef enum
{
   GIST_OK = 0,
   GIST_DONE,
   GIST_CLOSED,
   GIST_ERROR
} GIST_WORKER_STATUS;

typedef struct gist_worker
{
   GIST_WORKER_TYPE type;      /* Reader or Writer */

   union
   {
      GIST_READER_PARAMS reader_params;
      GIST_WRITER_PARAMS writer_params;
   };                          /* Parameters */

   struct file *file;          /* File handle */
   fl_owner_t  file_owner;     /* File owner */

   GIST_CIRC_BUFFER buffer;              /* Circular buffer */
   struct workqueue_struct *workqueue;   /* Work queue */
   struct work_struct init_exit_work;    /* Work entry for init/exit work */
   struct work_struct work;              /* Work entry for read/write work */
   struct delayed_work delayed_work;     /* Delayed work entry for read/write work */
   struct completion completion;         /* Completion signal */
   GIST_WORKER_STATUS status;            /* Current work status */

   struct gist_worker *prev;
   struct gist_worker *next;
} GIST_WORKER;

#define GIST_MODE_RESERVED 0x80
#define GIST_MODE_MASK     ~(GIST_MODE_RESERVED)

/* ---- Private Variables ------------------------------------------------ */

static char gBanner[] __initdata = KERN_INFO "BROADCOM Generic I/O Streaming\n";

static GIST_WORKER *gWorkers = NULL;

/* Circular buffer routines */
static int gist_circ_buffer_alloc(GIST_CIRC_BUFFER *buffer, unsigned int size);
static void gist_circ_buffer_free(GIST_CIRC_BUFFER *buffer);
static unsigned int gist_circ_buffer_in_use(GIST_CIRC_BUFFER *buffer);
static unsigned int gist_circ_buffer_room(GIST_CIRC_BUFFER *buffer);

static char * gist_circ_buffer_get_contiguous_read_buffer(GIST_CIRC_BUFFER *buffer, unsigned int *size);
static char * gist_circ_buffer_get_contiguous_write_buffer(GIST_CIRC_BUFFER *buffer, unsigned int *size);

static void gist_circ_buffer_advance_read_pointer(GIST_CIRC_BUFFER *buffer, unsigned int size);
static void gist_circ_buffer_advance_write_pointer(GIST_CIRC_BUFFER *buffer, unsigned int size);

static int gist_circ_buffer_read(GIST_CIRC_BUFFER *buffer, char *output, unsigned int size);
static int gist_circ_buffer_write(GIST_CIRC_BUFFER *buffer, const char *input, unsigned int size);

static int gist_circ_buffer_sum_sint16(GIST_CIRC_BUFFER *buffer, char *output, unsigned int size);

/* File helper routines */
static int gist_open_file(const char *filename, int flags, struct file **filep, fl_owner_t *owner);
static void gist_close_file(struct file *file, fl_owner_t owner);

/* Worker routines */
static void gist_worker_init(struct work_struct *work);
static void gist_worker_exit(struct work_struct *work);
static void gist_worker_read(struct work_struct *work);
static void gist_worker_delayed_read(struct work_struct *work);
static void gist_worker_write(struct work_struct *work);
static void gist_worker_delayed_write(struct work_struct *work);

/* ---- Functions -------------------------------------------------------- */

/***************************************************************************/
/**
*  Allocate circular buffer
*
*  @return
*     0           - success
*     -ve         - failure code
*/
static int gist_circ_buffer_alloc(GIST_CIRC_BUFFER *buffer, unsigned int size)
{
   char *bufp;

   bufp = kmalloc(size, GFP_KERNEL);
   if (!bufp)
   {
      return -ENOMEM;
   }

   /* Initialize buffer pointers and clear memory */
   buffer->bufsize = size;
   buffer->bufp = bufp;
   buffer->bufendp = bufp + size;
   buffer->bufreadp = bufp;
   buffer->bufwritep = bufp;
   memset(bufp, 0, size);

   return 0;
}


/***************************************************************************/
/**
*  Free circular buffer
*
*  @return
*     0           - success
*     -ve         - failure code
*/
static void gist_circ_buffer_free(GIST_CIRC_BUFFER *buffer)
{
   kfree(buffer->bufp);
   buffer->bufsize = 0;
   buffer->bufp = NULL;
   buffer->bufendp = NULL;
   buffer->bufreadp = NULL;
   buffer->bufwritep = NULL;
}


/***************************************************************************/
/**
*  Get amount of circular capture buffer currently in use.
*
*  @return
*     number of bytes in use
*/
static unsigned int gist_circ_buffer_in_use(GIST_CIRC_BUFFER *buffer)
{
   int bytes;

   bytes = buffer->bufwritep - buffer->bufreadp;

   if (bytes < 0)
   {
      bytes += buffer->bufsize;
   }
   return bytes;
}


/***************************************************************************/
/**
* Get amount of circular capture buffer currently free.
*
*  @return
*     number of bytes free
*/
static unsigned int gist_circ_buffer_room(GIST_CIRC_BUFFER *buffer)
{
   /* Return 1 byte less than the full buffer size to avoid wrap around condition */
   return buffer->bufsize - gist_circ_buffer_in_use(buffer) - 1;
}


/***************************************************************************/
/**
* Get a contiguous read buffer.
*
*  @return
*     number of bytes free
*/
static char * gist_circ_buffer_get_contiguous_read_buffer(GIST_CIRC_BUFFER *buffer, unsigned int *size)
{
   char *writep;

   /* Take a snapshot of write pointer in case it starts moving */
   writep = buffer->bufwritep;

   if (writep == buffer->bufreadp)
   {
      *size = 0;
   }
   else if (writep > buffer->bufreadp)
   {
      *size = writep - buffer->bufreadp;
   }
   else
   {
      *size = buffer->bufendp - buffer->bufreadp;
   }
   return buffer->bufreadp;
}


/***************************************************************************/
/**
* Get a contiguous write buffer.
*
*  @return
*     number of bytes free
*/
static char * gist_circ_buffer_get_contiguous_write_buffer(GIST_CIRC_BUFFER *buffer, unsigned int *size)
{
   char *readp;

   /* Take a snapshot of read pointer in case it starts moving */
   readp = buffer->bufreadp;

   if (buffer->bufwritep >= readp)
   {
      *size = buffer->bufendp - buffer->bufwritep;
      if (readp == buffer->bufp)
      {
         *size -= 1;
      }
   }
   else
   {
      *size = readp - buffer->bufwritep - 1;
   }
   return buffer->bufwritep;
}


/***************************************************************************/
/**
* Advance read pointer within the circular buffer
*
*  @return
*     none
*/
static void gist_circ_buffer_advance_read_pointer(GIST_CIRC_BUFFER *buffer, unsigned int size)
{
   unsigned int tail_size = (buffer->bufendp - buffer->bufreadp);

   if (tail_size > size)
   {
      buffer->bufreadp += size;
   }
   else
   {
      buffer->bufreadp = buffer->bufp + (size - tail_size);
   }
}


/***************************************************************************/
/**
* Advance write pointer within the circular buffer
*
*  @return
*     none
*/
static void gist_circ_buffer_advance_write_pointer(GIST_CIRC_BUFFER *buffer, unsigned int size)
{
   unsigned int tail_size = (buffer->bufendp - buffer->bufwritep);

   if (tail_size > size)
   {
      buffer->bufwritep += size;
   }
   else
   {
      buffer->bufwritep = buffer->bufp + (size - tail_size);
   }
}


/***************************************************************************/
/**
* Read data from circular buffer
*
*  @return
*     none
*/
static int gist_circ_buffer_read(GIST_CIRC_BUFFER *buffer, char *output, unsigned int size)
{
   int bytes_to_read = size;
   char *readp;
   int bytes_available;

   do
   {
      /* Get contiguous read buffer */
      readp = gist_circ_buffer_get_contiguous_read_buffer(buffer, &bytes_available);

      /* Stop if empty */
      if (bytes_available == 0)
      {
         break;
      }

      /* Copy data */
      if (bytes_available > bytes_to_read)
      {
         bytes_available = bytes_to_read;
      }
      memcpy(output, readp, bytes_available);
      gist_circ_buffer_advance_read_pointer(buffer, bytes_available);
      bytes_to_read -= bytes_available;
      output += bytes_available;

   } while (bytes_to_read);

   return (size - bytes_to_read);
}


/***************************************************************************/
/**
* Write data to circular buffer
*
*  @return
*     none
*/
static int gist_circ_buffer_write(GIST_CIRC_BUFFER *buffer, const char *input, unsigned int size)
{
   int bytes_to_write = size;
   char *writep;
   int bytes_available;

   do
   {
      /* Get contiguous read buffer */
      writep = gist_circ_buffer_get_contiguous_write_buffer(buffer, &bytes_available);

      /* Stop if full */
      if (bytes_available == 0)
      {
         break;
      }

      /* Copy data */
      if (bytes_available > bytes_to_write)
      {
         bytes_available = bytes_to_write;
      }
      memcpy(writep, input, bytes_available);
      gist_circ_buffer_advance_write_pointer(buffer, bytes_available);
      bytes_to_write -= bytes_available;
      input += bytes_available;

   } while (bytes_to_write);

   return (size - bytes_to_write);
}


/***************************************************************************/
/**
* Sum signed int16 data from circular buffer
*
*  @return
*     none
*/
static int16_t temp_buffer[2048];
static int gist_circ_buffer_sum_sint16(GIST_CIRC_BUFFER *buffer, char *output, unsigned int size)
{
   int rc;

   /* Size and alignment checks */
   if ( (size & 1) || (size > sizeof(temp_buffer)) || ((int)output & 1) )
      return 0;

   /* Read data input temporary buffer */
   rc = gist_circ_buffer_read(buffer, (char *)temp_buffer, size);
   if (rc < size)
      return 0;

   /* Sum data into output buffer */
   {
      int i;
      int length = size >> 1;
      int16_t *output_buffer = (int16_t *)output;

      for (i = 0; i < length; i++)
      {
         output_buffer[i] += temp_buffer[i];
      }
   }

   return size;
}


/***************************************************************************/
/**
*  Open a file for I/O
*
*  @return
*     0           - success
*     -ve         - failure code
*/
static int gist_open_file(const char *filename, int flags, struct file **filep, fl_owner_t *owner)
{
   struct file *file;
   mm_segment_t old_fs = get_fs();

   if (!filep)
   {
      return -EINVAL;
   }

   /* Open file */
   set_fs(KERNEL_DS);
   file = filp_open(filename, flags, 0644);
   set_fs(old_fs);

   if (IS_ERR(file))
   {
      return -EINVAL;
   }

   /* When opening for reading, do not allow 0 sized files */
   if ( flags == O_RDONLY )
   {
      struct kstat stat;
      int rc;

      /* Check file size */
      set_fs(KERNEL_DS);
      rc = vfs_getattr(file->f_path.mnt, file->f_path.dentry, &stat);
      set_fs(old_fs);
      if ( rc )
      {
         printk(KERN_ERR "Failed to get size of file %s. (%d)\n", filename, rc);
         return rc;
      }
      if ( stat.size == 0 )
      {
         printk(KERN_ERR "Do not allow reading from a zero-lengthed file %s.\n", filename);
         return -EINVAL;
      }
   }

   *filep = file;

   /* Save owner */
   if (owner)
   {
      *owner = current->files;
   }
   return 0;
}


/***************************************************************************/
/**
*  Close a file
*
*  @return
*     None
*/
static void gist_close_file(struct file *file, fl_owner_t owner)
{
   mm_segment_t old_fs = get_fs();

   set_fs(KERNEL_DS);
   filp_close(file, owner);
   set_fs(old_fs);
}


/***************************************************************************/
/**
*  Initialize worker
*
*  @return
*     None
*/
static void gist_worker_init(struct work_struct *work)
{
   GIST_WORKER *worker = container_of(work, GIST_WORKER, init_exit_work);
   int rc;
   const char *filename;
   int flags;

   /* Setup parameters */
   if (worker->type == GIST_READER)
   {
      filename = worker->reader_params.filename;
      flags = O_RDONLY;
   }
   else
   {
      filename = worker->writer_params.filename;
      if ((worker->writer_params.mode & GIST_MODE_MASK) == GIST_WRITE_MODE_APPEND)
      {
         flags = O_APPEND | O_WRONLY | O_CREAT;
      }
      else
      {
         flags = O_TRUNC | O_WRONLY | O_CREAT;
      }
   }

   /* Open file */
   rc = gist_open_file(filename, flags, &worker->file, &worker->file_owner);
   if (rc)
   {
      printk(KERN_ERR "Failed to open file %s. (%d)\n", filename, rc);
      worker->status = GIST_ERROR;
   }

   complete(&worker->completion);
   return;
}


/***************************************************************************/
/**
*  Stop worker
*
*  @return
*     None
*/
static void gist_worker_exit(struct work_struct *work)
{
   GIST_WORKER *worker = container_of(work, GIST_WORKER, init_exit_work);

   /* Write remaining data for writer */
   if (worker->type == GIST_WRITER)
   {
      gist_worker_write(&worker->work);
   }
   worker->status = GIST_DONE;

   /* Close file */
   gist_close_file(worker->file, worker->file_owner);
   worker->status = GIST_CLOSED;

   complete(&worker->completion);
   return;
}

/***************************************************************************/
/**
*  Read from file
*
*  @return
*     None
*/
static void gist_worker_delayed_read(struct work_struct *work)
{
   struct delayed_work *delayed_work = container_of(work, struct delayed_work, work);
   GIST_WORKER *worker = container_of(delayed_work, GIST_WORKER, delayed_work);
   gist_worker_read(&worker->work);
}
static void gist_worker_read(struct work_struct *work)
{
   GIST_WORKER *worker = container_of(work, GIST_WORKER, work);
   mm_segment_t old_fs = get_fs();

   /* Sanity check */
   if (worker->type != GIST_READER)
   {
      printk(KERN_ERR "Cannot read from a writer.\n");
      return;
   }

   /* Keep reading until finished or buffer full */
   while ((worker->status == GIST_OK) && gist_circ_buffer_room(&worker->buffer))
   {
      char *buffer;
      int size;

      /* Get contiguous buffer */
      buffer = gist_circ_buffer_get_contiguous_write_buffer(&worker->buffer, &size);

      if (!buffer)
      {
         worker->status = GIST_ERROR;
      }
      else
      {
         int bytes_read;

         /* Read data */
         set_fs(KERNEL_DS);
         bytes_read = vfs_read(worker->file, buffer, size, &worker->file->f_pos);
         set_fs(old_fs);

         if (bytes_read > 0)
         {
            /* Advance circular buffer */
            gist_circ_buffer_advance_write_pointer(&worker->buffer, bytes_read);
         }
         else if (bytes_read == 0)
         {
            /* Reached end of file */
            if ((worker->reader_params.mode & GIST_MODE_MASK) == GIST_READ_MODE_REPEAT)
            {
               /* Seek to beginning of file */
               printk(KERN_INFO "Repeat reading file %s.\n", worker->reader_params.filename);
               set_fs(KERNEL_DS);
               vfs_llseek(worker->file, 0, SEEK_SET);
               set_fs(old_fs);
            }
            else
            {
               /* Not repeating, mark worker as done */
               printk(KERN_INFO "Finished reading file %s.\n", worker->reader_params.filename);
               worker->status = GIST_DONE;
            }
         }
         else
         {
            /* Error reading file, stop reading. */
            printk(KERN_ERR "Error reading file %s. (%d)\n", worker->reader_params.filename, bytes_read);
            worker->status = GIST_ERROR;
         }
      }
   }

   /* Schedule delayed work to read more data */
   if (worker->status == GIST_OK)
   {
      cancel_delayed_work(&worker->delayed_work);
      queue_delayed_work(worker->workqueue, &worker->delayed_work, msecs_to_jiffies(GIST_WORKER_DELAY_MSECS));
   }

   return;
}


/***************************************************************************/
/**
*  Write to file
*
*  @return
*     None
*/
static void gist_worker_delayed_write(struct work_struct *work)
{
   struct delayed_work *delayed_work = container_of(work, struct delayed_work, work);
   GIST_WORKER *worker = container_of(delayed_work, GIST_WORKER, delayed_work);
   gist_worker_write(&worker->work);
}
static void gist_worker_write(struct work_struct *work)
{
   GIST_WORKER *worker = container_of(work, GIST_WORKER, work);
   mm_segment_t old_fs = get_fs();

   /* Sanity check */
   if (worker->type != GIST_WRITER)
   {
      printk(KERN_ERR "Cannot write to a reader.\n");
      return;
   }

   /* Keep writing until finished or buffer is empty */
   while ((worker->status == GIST_OK) && gist_circ_buffer_in_use(&worker->buffer))
   {
      char *buffer;
      int size;

      /* Get contiguous buffer */
      buffer = gist_circ_buffer_get_contiguous_read_buffer(&worker->buffer, &size);

      if (!buffer)
      {
         worker->status = GIST_ERROR;
      }
      else
      {
         int bytes_written;

         /* Write data */
         set_fs(KERNEL_DS);
         bytes_written = vfs_write(worker->file, buffer, size, &worker->file->f_pos);
         set_fs(old_fs);

         if (bytes_written > 0)
         {
            /* Advance circular buffer */
            gist_circ_buffer_advance_read_pointer(&worker->buffer, bytes_written);
         }
         else if (bytes_written == 0)
         {
            /* No bytes were written, stop writing. */
            printk(KERN_ERR "Error writing file %s.\n", worker->writer_params.filename);
            worker->status = GIST_ERROR;
         }
         else
         {
            /* Error writing file, stop writing. */
            printk(KERN_ERR "Error writing file %s. (%d)\n", worker->writer_params.filename, bytes_written);
            worker->status = GIST_ERROR;
         }
      }
   }

   /* Schedule delayed work to write more data */
   if (worker->status == GIST_OK)
   {
      cancel_delayed_work(&worker->delayed_work);
      queue_delayed_work(worker->workqueue, &worker->delayed_work, msecs_to_jiffies(GIST_WORKER_DELAY_MSECS));
   }

   return;
}



/***************************************************************************/
/**
*  Allocate worker
*
*  @return
*     None
*/
static int gist_alloc_worker(GIST_WORKER_TYPE type, const void *params)
{
   int rc;
   GIST_WORKER *worker;
   int buffer_size;

   /* Allocate control structure */
   worker = kmalloc(sizeof(GIST_WORKER), GFP_KERNEL);
   if (!worker)
   {
      printk(KERN_ERR "Not enough memory for worker.\n");
      goto err_worker_nomem;
   }
   worker->type = type;

   /* Save parameters */
   if (type == GIST_READER)
   {
      memcpy(&worker->reader_params, params, sizeof(GIST_READER_PARAMS));
      if (worker->reader_params.buffer_size == GIST_BUFFER_SIZE_USE_DEFAULT)
      {
         buffer_size = GIST_DEFAULT_BUFFER_SIZE;
         worker->reader_params.buffer_size = GIST_DEFAULT_BUFFER_SIZE;
      }
   }
   else
   {
      memcpy(&worker->writer_params, params, sizeof(GIST_WRITER_PARAMS));
      if (worker->writer_params.buffer_size == GIST_BUFFER_SIZE_USE_DEFAULT)
      {
         buffer_size = GIST_DEFAULT_BUFFER_SIZE;
         worker->writer_params.buffer_size = GIST_DEFAULT_BUFFER_SIZE;
      }
   }
   worker->status = GIST_OK;

   /* Allocate circular buffer */
   rc = gist_circ_buffer_alloc(&worker->buffer, buffer_size);
   if (rc)
   {
      printk(KERN_ERR "Not enough memory for circular buffer.\n");
      goto err_buffer_nomem;
   }

   /* Create work queue */
   worker->workqueue = create_singlethread_workqueue("gist_worker");
   if (!worker->workqueue)
   {
      printk(KERN_ERR "Failed to create work queue\n");
      goto err_workqueue_failed;
   }

   /* Schedule work structures */
   init_completion(&worker->completion);
   INIT_WORK(&worker->init_exit_work, gist_worker_init);
   if (type == GIST_READER)
   {
      INIT_WORK(&worker->work, gist_worker_read);
      INIT_DELAYED_WORK(&worker->delayed_work, gist_worker_delayed_read);
   }
   else
   {
      INIT_WORK(&worker->work, gist_worker_write);
      INIT_DELAYED_WORK(&worker->delayed_work, gist_worker_delayed_write);
   }

   /* Schedule initialization work */
   queue_work(worker->workqueue, &worker->init_exit_work);
   wait_for_completion(&worker->completion);
   if (worker->status != GIST_OK)
   {
      printk(KERN_ERR "Failed to initialize worker\n");
      goto err_init_failed;
   }

   /* Successfully allocated, begin reading */
   if (type == GIST_READER)
   {
      queue_work(worker->workqueue, &worker->work);
   }
   else
   {
      /* Nothing to do for writer yet */
   }

   return (int)worker;

err_init_failed:
   flush_workqueue(worker->workqueue);
   destroy_workqueue(worker->workqueue);
   worker->workqueue = NULL;

err_workqueue_failed:
   gist_circ_buffer_free(&worker->buffer);

err_buffer_nomem:
   kfree(worker);
   worker = NULL;

err_worker_nomem:

   return GIST_INVALID_HANDLE;
}


/***************************************************************************/
/**
*  Free worker
*
*  @return
*     None
*/
static void gist_free_worker(int handle)
{
   GIST_WORKER *worker = (GIST_WORKER *)handle;

   /* Schedule shutdown work */
   flush_workqueue(worker->workqueue);
   PREPARE_WORK(&worker->init_exit_work, gist_worker_exit);
   queue_work(worker->workqueue, &worker->init_exit_work);
   wait_for_completion(&worker->completion);

   /* Free all resources */
   cancel_delayed_work(&worker->delayed_work);
   flush_workqueue(worker->workqueue);
   destroy_workqueue(worker->workqueue);
   worker->workqueue = NULL;
   gist_circ_buffer_free(&worker->buffer);
   kfree(worker);
   worker = NULL;
   return;
}


/***************************************************************************/
/**
*  Allocate reader
*
*  @return
*     None
*/
int gist_alloc_reader(const GIST_READER_PARAMS *params)
{
   /* Validate parameters */
   if (((params->mode & GIST_MODE_MASK) != GIST_READ_MODE_ONCE) &&
       ((params->mode & GIST_MODE_MASK) != GIST_READ_MODE_REPEAT))
   {
      return GIST_INVALID_HANDLE;
   }
   if (params->buffer_size < 0)
   {
      return GIST_INVALID_HANDLE;
   }
   return (gist_alloc_worker(GIST_READER, params));
}
EXPORT_SYMBOL(gist_alloc_reader);


/***************************************************************************/
/**
*  Allocate writer
*
*  @return
*     None
*/
int gist_alloc_writer(const GIST_WRITER_PARAMS *params)
{
   /* Validate parameters */
   if (((params->mode & GIST_MODE_MASK) != GIST_WRITE_MODE_TRUNCATE) &&
       ((params->mode & GIST_MODE_MASK) != GIST_WRITE_MODE_APPEND))
   {
      return GIST_INVALID_HANDLE;
   }
   if (params->buffer_size < 0)
   {
      return GIST_INVALID_HANDLE;
   }
   return (gist_alloc_worker(GIST_WRITER, params));
}
EXPORT_SYMBOL(gist_alloc_writer);


/***************************************************************************/
/**
*  Free reader
*
*  @return
*     None
*/
int gist_free_reader(int handle)
{
   gist_free_worker(handle);
   return 0;
}
EXPORT_SYMBOL(gist_free_reader);


/***************************************************************************/
/**
*  Free writer
*
*  @return
*     None
*/
int gist_free_writer(int handle)
{
   gist_free_worker(handle);
   return 0;
}
EXPORT_SYMBOL(gist_free_writer);


/***************************************************************************/
/**
*  Read data from reader
*
*  @return
*     None
*/
int gist_read(int handle, int length, void *bufp)
{
   GIST_WORKER *worker = (GIST_WORKER *)handle;
   int rc;

   if (worker->type != GIST_READER)
   {
      return 0;
   }

   if (!(worker->reader_params.mode & GIST_MODE_RESERVED))
   {
      rc = gist_circ_buffer_read(&worker->buffer, bufp, length);
   }
   else
   {
      rc = gist_circ_buffer_sum_sint16(&worker->buffer, bufp, length);
   }

   /* Refill buffer if necessary */
   if ((worker->status == GIST_OK) &&
       (gist_circ_buffer_in_use(&worker->buffer) <= GIST_BUFFER_READ_THRESHOLD(&worker->buffer)))
   {
      queue_work(worker->workqueue, &worker->work);
   }

   return rc;
}
EXPORT_SYMBOL(gist_read);


/***************************************************************************/
/**
*  Write data to writer
*
*  @return
*     None
*/
int gist_write(int handle, int length, const void *bufp)
{
   GIST_WORKER *worker = (GIST_WORKER *)handle;
   int rc;

   if (worker->type != GIST_WRITER)
   {
      return 0;
   }

   rc = gist_circ_buffer_write(&worker->buffer, bufp, length);

   /* Refill buffer if necessary */
   if ((worker->status == GIST_OK) &&
       (gist_circ_buffer_in_use(&worker->buffer) >= GIST_BUFFER_WRITE_THRESHOLD(&worker->buffer)))
   {
      queue_work(worker->workqueue, &worker->work);
   }

   return rc;
}
EXPORT_SYMBOL(gist_write);


/***************************************************************************/
/**
*  Get reader's parameters
*
*  @return
*     None
*/
int gist_get_reader_params(int handle, GIST_READER_PARAMS *params)
{
   GIST_WORKER *worker = (GIST_WORKER *)handle;

   if (!params || (worker->type != GIST_READER))
   {
      return -EINVAL;
   }
   memcpy(params, &worker->reader_params, sizeof(GIST_READER_PARAMS));

   return 0;
}
EXPORT_SYMBOL(gist_get_reader_params);


/***************************************************************************/
/**
*  Get writer's parameters
*
*  @return
*     None
*/
int gist_get_writer_params(int handle, GIST_WRITER_PARAMS *params)
{
   GIST_WORKER *worker = (GIST_WORKER *)handle;

   if (!params || (worker->type != GIST_WRITER))
   {
      return -EINVAL;
   }
   memcpy(params, &worker->writer_params, sizeof(GIST_READER_PARAMS));

   return 0;
}
EXPORT_SYMBOL(gist_get_writer_params);


/****************************************************************************
*
*  gist_init
*
*     Called to perform module initialization when the module is loaded
*
***************************************************************************/
int __init gist_init(void)
{
   int rc = 0;

   printk(gBanner);

   return rc;
}

/****************************************************************************
*
*  gist_exit
*
*       Called to perform module cleanup when the module is unloaded.
*
***************************************************************************/

void __exit gist_exit(void)
{
   while (gWorkers)
   {
      printk(KERN_INFO "GIST: Waiting for readers and writers to close.\n");
      /* Wait for readers and writers to close */
/*todo */
   }

}



module_init(gist_init);
module_exit(gist_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Broadcom Generic I/O Streaming Utility");
MODULE_LICENSE( "GPL" );

