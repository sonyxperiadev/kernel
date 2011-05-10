#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/miscdevice.h>
#include <linux/vmalloc.h>
#include <linux/string.h>
#include <linux/spinlock.h>
#include <linux/ipc/ipc.h>

#include <asm/bug.h>
#include <asm/io.h>
#include <asm/atomic.h>

#include <mach/media_dec_regs.h>
#include <mach/fifo.h>

#include "bcm2708_mdec.h"


#define BCM2708_MDEC_DRIVER_NAME "bcm2708_mdec"

#define BCM2708MDEC_DEBUG 0

#define bcm2708mdec_error(format, arg...) \
   printk(KERN_ERR BCM2708_MDEC_DRIVER_NAME ": %s " format, __func__, ##arg)

#if BCM2708MDEC_DEBUG
#define bcm2708mdec_dbg(format, arg...) bcm2708mdec_error(format, ##arg)
#else
#define bcm2708mdec_dbg(format, arg...) do {} while (0)
#endif

#define VC_MFS_SD_PREFIX "/mfs/sd/"   /* the path for mdeia file on VC SD card. */

#define MEDIA_DEC_REGISTER_RW(offset)   \
   (*((volatile unsigned long *)((u32)g_mdec->reg_base + (offset))))

#define START_SEQUENCE_NUM   1

typedef enum {
   AUDIO_STREAM = 0x0,
   VIDEO_STREAM,
} MEDIA_STREAM_T;

typedef enum {
   PLAYBACK_IDLE      =   0x0,
   PLAYBACK_ENABLED,
   PLAYBACK_STARTED,
   PLAYBACK_CLOSED,
} playback_state_t;

typedef enum {
        AUDIO_MASK   =       0x1<<0,
        VIDEO_MASK   =   0x1<<1,
} playback_type_t;

struct play_data_request {
   void         *buf;
   unsigned int      buf_size;
   struct completion   *cmpl;
   struct list_head   node;
};

/*
 * TODO:
 *    1. Each media control structure can be allocated at device open time and stored at filp
 *       then all the following filp operations can use it.
 *    2. Then this sturcture can be linked into the global list that resides inside the g_mdec.
 */
struct media_stream_ctl {
   struct semaphore        vc_buf_sem;
   struct semaphore        arm_buf_sem;
   spinlock_t              vc_fifo_lock;
   IPC_FIFO_T              vc_to_arm_fifo;
   spinlock_t              arm_fifo_lock;
   IPC_FIFO_T              arm_to_vc_fifo;
   atomic_t                sequence_num;
};

struct media_av_ctl {
   struct media_stream_ctl      audio_ctl;
   struct media_stream_ctl      video_ctl;
   u32            playback_type;
   playback_state_t      state;
};

/* TODO:
 *   1. need to add spinlock to make sure there is only one stream setup on-the-fly at any single moment if we do
 *      support multi-stream use case.
 */
struct bcm2708_mdec {
   u32                 irq;
   void __iomem        *reg_base;
   struct semaphore    vc_ack_sem;
   struct semaphore    mUserCallbackSem;
   struct media_av_ctl av_stream_ctl;
};

/* hacky here; needs to make a per thread buffer */
static struct bcm2708_mdec *g_mdec = NULL;

static inline void dump_vc_to_arm_fifo(struct media_stream_ctl *stream_ctl)
{
#if BCM2708MDEC_DEBUG
        printk(KERN_ERR "vc_to_arm_video_fifo: write=0x%08x           read=0x%08x\n"
                                        "base=0x%08x            size=0x%08x\n"
                                        "entry_size=0x%08x\n",
                                        (u32)stream_ctl->vc_to_arm_fifo.write, (u32)stream_ctl->vc_to_arm_fifo.read,
                                        (u32)stream_ctl->vc_to_arm_fifo.base, stream_ctl->vc_to_arm_fifo.size,
                                        stream_ctl->vc_to_arm_fifo.entry_size);
#endif
}

static inline void dump_arm_to_vc_video_fifo(struct media_stream_ctl *stream_ctl)
{
#if BCM2708MDEC_DEBUG
        printk(KERN_ERR "arm_to_vc_video_fifo: write=0x%08x           read=0x%08x\n"
                                        "base=0x%08x            size=0x%08x\n"
                                        "entry_size=0x%08x\n",
                                        (u32)stream_ctl->arm_to_vc_fifo.write, (u32)stream_ctl->arm_to_vc_fifo.read,
                                        (u32)stream_ctl->arm_to_vc_fifo.base, stream_ctl->arm_to_vc_fifo.size,
                                        stream_ctl->arm_to_vc_fifo.entry_size);
#endif
}

static inline void dump_fifo_entry(MEDIA_DEC_FIFO_ENTRY_T *entry)
{
#if BCM2708MDEC_DEBUG
        printk(KERN_ERR "fifo entry:    buffer_id=0x%08x                buffer_size=0x%08x\n"
                                        "buffer_filled=0x%08x           buffer_ptr=0x%08x\n"
                                        "timestamp=0x%08x\n",
                                        entry->buffer_id, entry->buffer_size,
                                        entry->buffer_filled_size, entry->buffer_ptr,
                                        entry->timestamp);
#endif
}

static inline int notify_vc_and_wait_for_ack(void)
{
   int ret = 0;

   ipc_notify_vc_event(g_mdec->irq);

#if 0
   ret = down_timeout(&g_mdec->vc_ack_sem, HZ * 3);
   if (ret)
      bcm2708mdec_error("Failed to acquire the semaphore, probably VC side is pegged!");
#endif
   down(&g_mdec->vc_ack_sem);

   return ret;
}


static int player_setup(bcm2708_mdec_setup_t *setup_cmd)
{
   size_t configSize;
   int ret = 0;
   void * audioConfigReg;

   g_mdec->av_stream_ctl.state = PLAYBACK_IDLE;

   sema_init(&g_mdec->vc_ack_sem, 0);
   bcm2708mdec_dbg( "   after ibit: %08x %d %p %p\n", *( (uint32_t *)(&g_mdec->vc_ack_sem.lock) ), g_mdec->vc_ack_sem.count, g_mdec->vc_ack_sem.wait_list.next, g_mdec->vc_ack_sem.wait_list.prev );

   bcm2708mdec_dbg("player setup with video_type=%d audio_type=%d\n",
            setup_cmd->video_type,
            setup_cmd->audio_type);

        /* Set up the debug mode */
        MEDIA_DEC_REGISTER_RW( MEDIA_DEC_DEBUG_MASK ) = 0;

        /* Set up the src width as 0xFFFFFFFF (ignore) */
        MEDIA_DEC_REGISTER_RW( MEDIA_DEC_SOURCE_X_OFFSET ) = 0xFFFFFFFF;
        MEDIA_DEC_REGISTER_RW( MEDIA_DEC_SOURCE_Y_OFFSET ) = 0xFFFFFFFF;
        MEDIA_DEC_REGISTER_RW( MEDIA_DEC_SOURCE_WIDTH_OFFSET ) = 0xFFFFFFFF;
        MEDIA_DEC_REGISTER_RW( MEDIA_DEC_SOURCE_HEIGHT_OFFSET ) = 0xFFFFFFFF;

        /* Set up the target codec */
        MEDIA_DEC_REGISTER_RW( MEDIA_DEC_VID_TYPE ) = setup_cmd->video_type;
        MEDIA_DEC_REGISTER_RW( MEDIA_DEC_AUD_TYPE ) = setup_cmd->audio_type;

        /* Set up audio config */
        switch( setup_cmd->audio_type )
        {
            case MEDIA_DEC_AUDIO_CodingPCM:     configSize = sizeof( MDEC_PCM_CONFIG ); break;
            case MEDIA_DEC_AUDIO_CodingDDP:     configSize = sizeof( MDEC_DDP_CONFIG ); break;
            default:                            configSize = 0; break;
        }

        // Note: the REGISTER_RW macro gives us a uint32_t pointer, but that shouldn't matter since
        // it's just going in to memcpy anyway
        audioConfigReg = (void *)( (u32)g_mdec->reg_base + (MEDIA_DEC_AUDIO_CONFIG_OFFSET) );
        if( configSize > 0 )
           memcpy( audioConfigReg, &setup_cmd->mAudioConfig, configSize );

        /* Enable the mode */
        MEDIA_DEC_REGISTER_RW( MEDIA_DEC_CONTROL_OFFSET ) = MEDIA_DEC_CONTROL_ENABLE_BIT;

        /* Clear the flags */
        MEDIA_DEC_REGISTER_RW( MEDIA_DEC_FLAGS_OFFSET ) = 0x0;

   mb();

   ret = notify_vc_and_wait_for_ack();

        while ((MEDIA_DEC_REGISTER_RW( MEDIA_DEC_STATUS_OFFSET) & 0x1) != 0x1) {
                schedule_timeout(1);
        }


#if 0
   BUG_ON((MEDIA_DEC_REGISTER_RW( MEDIA_DEC_STATUS_OFFSET) & 0x1) != 0x1);
#endif
   /*
     * Initialize the stream control structure for both audio and video streams, if any.
     *
     */

   g_mdec->av_stream_ctl.playback_type = 0;
   /* FIFO is set up on the VC side after enabl bit is set and the ARM side is doing the same. */
   if (MEDIA_DEC_VIDEO_CodingUnused != setup_cmd->video_type) {
           ipc_fifo_setup_no_reset(&g_mdec->av_stream_ctl.video_ctl.vc_to_arm_fifo,
                           &MEDIA_DEC_REGISTER_RW(MEDIA_DEC_VIDEO_OUT_WRITE_PTR_OFFSET),
                           &MEDIA_DEC_REGISTER_RW(MEDIA_DEC_VIDEO_OUT_READ_PTR_OFFSET),
                           ipc_bus_to_virt(MEDIA_DEC_REGISTER_RW(MEDIA_DEC_VIDEO_OUT_FIFO_START_OFFSET)),
                           MEDIA_DEC_REGISTER_RW(MEDIA_DEC_VIDEO_OUT_FIFO_SIZE_OFFSET),
                           MEDIA_DEC_REGISTER_RW(MEDIA_DEC_VIDEO_OUT_FIFO_ENTRY_OFFSET));

           ipc_fifo_setup_no_reset(&g_mdec->av_stream_ctl.video_ctl.arm_to_vc_fifo,
                           &MEDIA_DEC_REGISTER_RW(MEDIA_DEC_VIDEO_IN_WRITE_PTR_OFFSET),
                           &MEDIA_DEC_REGISTER_RW(MEDIA_DEC_VIDEO_IN_READ_PTR_OFFSET),
                           ipc_bus_to_virt(MEDIA_DEC_REGISTER_RW(MEDIA_DEC_VIDEO_IN_FIFO_START_OFFSET)),
                           MEDIA_DEC_REGISTER_RW(MEDIA_DEC_VIDEO_IN_FIFO_SIZE_OFFSET),
                           MEDIA_DEC_REGISTER_RW(MEDIA_DEC_VIDEO_IN_FIFO_ENTRY_OFFSET));

      sema_init(&g_mdec->av_stream_ctl.video_ctl.vc_buf_sem, 0);
      sema_init(&g_mdec->av_stream_ctl.video_ctl.arm_buf_sem, 0);
      spin_lock_init(&g_mdec->av_stream_ctl.video_ctl.vc_fifo_lock);
      spin_lock_init(&g_mdec->av_stream_ctl.video_ctl.arm_fifo_lock);
      atomic_set(&g_mdec->av_stream_ctl.video_ctl.sequence_num, 0);

      g_mdec->av_stream_ctl.playback_type |= VIDEO_MASK;
   }

   if (MEDIA_DEC_AUDIO_CodingUnused != setup_cmd->audio_type) {
                ipc_fifo_setup_no_reset(&g_mdec->av_stream_ctl.audio_ctl.vc_to_arm_fifo,
                                &MEDIA_DEC_REGISTER_RW(MEDIA_DEC_AUDIO_OUT_WRITE_PTR_OFFSET),
                                &MEDIA_DEC_REGISTER_RW(MEDIA_DEC_AUDIO_OUT_READ_PTR_OFFSET),
                                ipc_bus_to_virt(MEDIA_DEC_REGISTER_RW(MEDIA_DEC_AUDIO_OUT_FIFO_START_OFFSET)),
                                MEDIA_DEC_REGISTER_RW(MEDIA_DEC_AUDIO_OUT_FIFO_SIZE_OFFSET),
                                MEDIA_DEC_REGISTER_RW(MEDIA_DEC_AUDIO_OUT_FIFO_ENTRY_OFFSET));

                ipc_fifo_setup_no_reset(&g_mdec->av_stream_ctl.audio_ctl.arm_to_vc_fifo,
                                &MEDIA_DEC_REGISTER_RW(MEDIA_DEC_AUDIO_IN_WRITE_PTR_OFFSET),
                                &MEDIA_DEC_REGISTER_RW(MEDIA_DEC_AUDIO_IN_READ_PTR_OFFSET),
                                ipc_bus_to_virt(MEDIA_DEC_REGISTER_RW(MEDIA_DEC_AUDIO_IN_FIFO_START_OFFSET)),
                                MEDIA_DEC_REGISTER_RW(MEDIA_DEC_AUDIO_IN_FIFO_SIZE_OFFSET),
                                MEDIA_DEC_REGISTER_RW(MEDIA_DEC_AUDIO_IN_FIFO_ENTRY_OFFSET));

                sema_init(&g_mdec->av_stream_ctl.audio_ctl.vc_buf_sem, 0);
                sema_init(&g_mdec->av_stream_ctl.audio_ctl.arm_buf_sem, 0);
                spin_lock_init(&g_mdec->av_stream_ctl.audio_ctl.vc_fifo_lock);
                spin_lock_init(&g_mdec->av_stream_ctl.audio_ctl.arm_fifo_lock);
                atomic_set(&g_mdec->av_stream_ctl.audio_ctl.sequence_num, 0);

      g_mdec->av_stream_ctl.playback_type |= AUDIO_MASK;
   }

   g_mdec->av_stream_ctl.state = PLAYBACK_ENABLED;

   return ret;
}

static int player_start(void)
{
   int ret = 0;

   bcm2708mdec_dbg("player start\n");

   if (PLAYBACK_ENABLED != g_mdec->av_stream_ctl.state)
      return -EIO;

        /* start to play */
        MEDIA_DEC_REGISTER_RW(MEDIA_DEC_CONTROL_OFFSET) |= MEDIA_DEC_CONTROL_PLAY_BIT;

   mb();

        ret = notify_vc_and_wait_for_ack();

#if 0
        WARN_ON((MEDIA_DEC_REGISTER_RW( MEDIA_DEC_STATUS_OFFSET) & MEDIA_DEC_CONTROL_PLAY_BIT) != MEDIA_DEC_CONTROL_PLAY_BIT);
#endif

        while ((MEDIA_DEC_REGISTER_RW( MEDIA_DEC_STATUS_OFFSET) & MEDIA_DEC_CONTROL_PLAY_BIT) != MEDIA_DEC_CONTROL_PLAY_BIT) {
                schedule_timeout(1);
        }

   g_mdec->av_stream_ctl.state = PLAYBACK_STARTED;

   return ret;
}

static int player_send_data(bcm2708_mdec_send_data_t *send_data_cmd, MEDIA_STREAM_T stream_type)
{
   MEDIA_DEC_FIFO_ENTRY_T entry;
   unsigned long copy_bytes, total_bytes;
   void    *buf_virt, *copy_ptr;
   int ret = 0;
   struct media_stream_ctl *stream_ctl;

   if (PLAYBACK_STARTED != g_mdec->av_stream_ctl.state)
                return -EIO;

   if (AUDIO_STREAM == stream_type) {
      stream_ctl = &g_mdec->av_stream_ctl.audio_ctl;
   }
   else if (VIDEO_STREAM == stream_type) {
      stream_ctl = &g_mdec->av_stream_ctl.video_ctl;
   }
   else
      BUG();

   if (0 == send_data_cmd->data_size)
      return 0;

#if 0
   bcm2708mdec_dbg("player send %s data with buf=0x%08x and size=0x%08x\n",
            (AUDIO_STREAM == stream_type)?"audio":"video",
            (u32)send_data_cmd->data_buf,
            send_data_cmd->data_size);
#endif

   memset(&entry, 0, sizeof(&entry));
   total_bytes = send_data_cmd->data_size;
   copy_ptr = send_data_cmd->data_buf;

   /*
    * Spinlock is used to protect against simualtenious access from
    * ARM side. And once the thread is waken up, it needs to check if
    * FIFO is still empty because of potential access from other thread, or
    * the wake up interrupt is an extra one.
    */
   do {
      //spin_lock_irqsave(&stream_ctl->vc_fifo_lock, flags);
      if (!ipc_fifo_empty(&stream_ctl->vc_to_arm_fifo)) {
         ipc_fifo_read(&stream_ctl->vc_to_arm_fifo, &entry);
         BUG_ON(0 == entry.buffer_size);
         //   spin_unlock_irqrestore(&stream_ctl->vc_fifo_lock, flags);
      } else {
         //   spin_unlock_irqrestore(&stream_ctl->vc_fifo_lock, flags);
         ret = down_interruptible(&stream_ctl->vc_buf_sem);
         if (ret < 0)
            return -ERESTARTSYS;
         else {
            continue;
         }
      }
      copy_bytes = (entry.buffer_size > total_bytes)? total_bytes : entry.buffer_size;

      BUG_ON(0UL == entry.buffer_ptr);

      buf_virt = ioremap(__VC_BUS_TO_ARM_PHYS_ADDR(entry.buffer_ptr), copy_bytes);
      if (NULL == buf_virt) {
                   bcm2708mdec_error("failed to map the memory\n");
                   return -ENOMEM;
           }
#if 0
                bcm2708mdec_dbg("VC buffer bus addr=0x%08x, virt addr=0x%08x\n",
            (u32)entry.buffer_ptr, (u32)buf_virt);
#endif

      if (copy_from_user(buf_virt, copy_ptr, copy_bytes)) {
         bcm2708mdec_error("failed to copy the user data\n");
         iounmap(buf_virt);
         return -EFAULT;
      }
      entry.buffer_filled_size = copy_bytes;
      entry.sequence_number    = atomic_add_return(1, &stream_ctl->sequence_num);
      entry.flags              = send_data_cmd->flags;
      entry.timestamp          = send_data_cmd->timestamp;
      entry.callback_context   = send_data_cmd->callback_context;
      iounmap(buf_virt);

      total_bytes -= copy_bytes;
      copy_ptr = (void *)((u32)copy_ptr + copy_bytes);

      /* Now put the filled buf into output fifo; and we assume that
        * there should be a slot in the input FIFO.
        * Afterwards, tell VC about this buffer with ringing doorbell.
        */
      while (1) {
         //spin_lock_irqsave(&stream_ctl->arm_fifo_lock, flags);
         if (!ipc_fifo_full(&stream_ctl->arm_to_vc_fifo)) {
            ipc_fifo_write(&stream_ctl->arm_to_vc_fifo, &entry);
            //spin_unlock_irqrestore(&stream_ctl->arm_fifo_lock, flags);
            break;
         } else {
            //spin_unlock_irqrestore(&stream_ctl->arm_fifo_lock, flags);
            ret = down_interruptible(&stream_ctl->arm_buf_sem);
            if (ret < 0)
               return -ERESTARTSYS;
            else {
               continue;
            }
         }
      }

      mb();

      ipc_notify_vc_event(g_mdec->irq);

   } while (total_bytes > 0);

   return 0;
}

static int player_send_video_data(bcm2708_mdec_send_data_t *send_data_cmd)
{
        return player_send_data(send_data_cmd, VIDEO_STREAM);
}

static int player_send_audio_data(bcm2708_mdec_send_data_t *send_data_cmd)
{
        return player_send_data(send_data_cmd, AUDIO_STREAM);
}

static int do_playback(bcm2708_mdec_play_t *play_cmd)
{
   int ret = 0;
   u32 time = 0, prev_time = 0;
   int count;
   char *temp_name;

   BUG_ON(MEDIA_DEC_DEBUG_FILENAME_LENGTH  <= play_cmd->filename_size);

   /* Set up the debug mode */
   MEDIA_DEC_REGISTER_RW( MEDIA_DEC_DEBUG_MASK ) = 0;

   /* Set up the src width as 0xFFFFFFFF (ignore) */
   MEDIA_DEC_REGISTER_RW( MEDIA_DEC_SOURCE_X_OFFSET ) = 0xFFFFFFFF;
   MEDIA_DEC_REGISTER_RW( MEDIA_DEC_SOURCE_Y_OFFSET ) = 0xFFFFFFFF;
   MEDIA_DEC_REGISTER_RW( MEDIA_DEC_SOURCE_WIDTH_OFFSET ) = 0xFFFFFFFF;
   MEDIA_DEC_REGISTER_RW( MEDIA_DEC_SOURCE_HEIGHT_OFFSET ) = 0xFFFFFFFF;

   /* Set up the target codec */
   MEDIA_DEC_REGISTER_RW( MEDIA_DEC_VID_TYPE ) = play_cmd->video_type;
   MEDIA_DEC_REGISTER_RW( MEDIA_DEC_AUD_TYPE ) = play_cmd->audio_type;

#if BCM2708MDEC_DEBUG
   play_cmd->filename[play_cmd->filename_size] = 0;
   bcm2708mdec_dbg("filename=%s size=%d\n", play_cmd->filename, play_cmd->filename_size);
#endif
   /* If user does not provide a full ppath filename, fix it. */
   if (strncmp(play_cmd->filename, VC_MFS_SD_PREFIX, strlen(VC_MFS_SD_PREFIX))) {
      temp_name = (char *)vmalloc(play_cmd->filename_size);
      if (NULL == temp_name) {
         bcm2708mdec_error("Unable to allocate name\n");
         return -ENOMEM;
      }
      strncpy(temp_name, play_cmd->filename, play_cmd->filename_size);
      strcpy(play_cmd->filename, VC_MFS_SD_PREFIX);
      strncat(play_cmd->filename, temp_name, play_cmd->filename_size);
      vfree(temp_name);
      play_cmd->filename_size += strlen(VC_MFS_SD_PREFIX);
   }

#if BCM2708MDEC_DEBUG
        play_cmd->filename[play_cmd->filename_size] = 0;
        bcm2708mdec_dbg("filename=%s size=%d\n", play_cmd->filename, play_cmd->filename_size);
#endif

   /* Write in the filename */
   strncpy((char *)&MEDIA_DEC_REGISTER_RW( MEDIA_DEC_DEBUG_FILENAME), play_cmd->filename, play_cmd->filename_size);

   /* Enable the mode */
   MEDIA_DEC_REGISTER_RW( MEDIA_DEC_CONTROL_OFFSET ) = MEDIA_DEC_CONTROL_ENABLE_BIT | MEDIA_DEC_CONTROL_LOCAL_FILEMODE_BIT;

   ipc_notify_vc_event(g_mdec->irq);

   /* Wait for it to get ready */
   while ((MEDIA_DEC_REGISTER_RW( MEDIA_DEC_STATUS_OFFSET) & 0x1) != 0x1) {
      schedule_timeout(100);
      bcm2708mdec_dbg("slept for 1 sec in enabling playback\n");
   }

   /* start to play */
   MEDIA_DEC_REGISTER_RW(MEDIA_DEC_CONTROL_OFFSET) |= MEDIA_DEC_CONTROL_PLAY_BIT;

        ipc_notify_vc_event(g_mdec->irq);

   /* Wait for it to start */
   while ((MEDIA_DEC_REGISTER_RW( MEDIA_DEC_STATUS_OFFSET) & MEDIA_DEC_CONTROL_PLAY_BIT) != MEDIA_DEC_CONTROL_PLAY_BIT) {
      schedule_timeout(100);
      bcm2708mdec_dbg("slept for 1 sec in playback\n");
   }

   for( count = 0; count < 10; count++ ) {
      prev_time = time;
      schedule_timeout(100);
      time = MEDIA_DEC_REGISTER_RW( MEDIA_DEC_PLAYBACK_TIME );
      if (time == prev_time)
         bcm2708mdec_dbg("the playback ts is not moving\n");
   }

   return ret;
}


static int player_stop(void)
{
   int ret = 0;

   bcm2708mdec_dbg("player stops\n");

//   WARN_ON(0x1 != (0x1 & MEDIA_DEC_REGISTER_RW( MEDIA_DEC_CONTROL_OFFSET )));


   MEDIA_DEC_REGISTER_RW( MEDIA_DEC_CONTROL_OFFSET ) = 0x0;

        mb();

        ret = notify_vc_and_wait_for_ack();

   mb();


//   BUG_ON((MEDIA_DEC_REGISTER_RW( MEDIA_DEC_STATUS_OFFSET) & 0x3) != 0x0);

#if 1
   while ((MEDIA_DEC_REGISTER_RW( MEDIA_DEC_STATUS_OFFSET) & 0x3) != 0x0) {
                schedule_timeout(1);
   }
#endif

//   sema_init(&g_mdec->vc_ack_sem, 0);

   g_mdec->av_stream_ctl.state =  PLAYBACK_IDLE;

   return ret;
}

static int player_set_paused(int paused)
{
   int ret = 0;

   bcm2708mdec_dbg( "player %s\n", ( ( paused ) ? "paused" : "resumed" ) );

/*   if( paused )
      MEDIA_DEC_REGISTER_RW( MEDIA_DEC_FLAGS_OFFSET ) |= MEDIA_DEC_FLAGS_PAUSED;
   else
      MEDIA_DEC_REGISTER_RW( MEDIA_DEC_FLAGS_OFFSET ) &= ~MEDIA_DEC_FLAGS_PAUSED;
*/
   if( paused )
   {
      if( g_mdec->av_stream_ctl.state != PLAYBACK_STARTED )
         return -EIO;

      MEDIA_DEC_REGISTER_RW( MEDIA_DEC_CONTROL_OFFSET ) &= ~MEDIA_DEC_CONTROL_PLAY_BIT;

      g_mdec->av_stream_ctl.state = PLAYBACK_ENABLED;
   }
   else
   {
      if( g_mdec->av_stream_ctl.state != PLAYBACK_ENABLED )
         return -EIO;

      MEDIA_DEC_REGISTER_RW( MEDIA_DEC_CONTROL_OFFSET ) |= MEDIA_DEC_CONTROL_PLAY_BIT;

      g_mdec->av_stream_ctl.state = PLAYBACK_STARTED;
   }

   mb();

   ret = notify_vc_and_wait_for_ack();

   mb();

   return ret;
}

static int player_set_volume(bcm2708_mdec_set_volume_t * data)
{
   int ret = 0;

   bcm2708mdec_dbg( "player volume set to %d\n", data->volumeInmB );

   MEDIA_DEC_REGISTER_RW( MEDIA_DEC_VOLUME_OFFSET ) = data->volumeInmB;
   MEDIA_DEC_REGISTER_RW( MEDIA_DEC_CONTROL_OFFSET ) |= MEDIA_DEC_CONTROL_SET_VOLUME_BIT;

   mb();

   ret = notify_vc_and_wait_for_ack();

   mb();

   return ret;
}

static int player_set_muted(bcm2708_mdec_set_muted_t * data)
{
   int ret = 0;

   bcm2708mdec_dbg( "player mute setting set to %d\n", data->muted );

   if( data->muted )
   {
      MEDIA_DEC_REGISTER_RW( MEDIA_DEC_FLAGS_OFFSET ) |= MEDIA_DEC_FLAGS_MUTED;
   }
   else
   {
      MEDIA_DEC_REGISTER_RW( MEDIA_DEC_FLAGS_OFFSET ) &= ~MEDIA_DEC_FLAGS_MUTED;
   }
   MEDIA_DEC_REGISTER_RW( MEDIA_DEC_CONTROL_OFFSET ) |= MEDIA_DEC_CONTROL_SET_FLAGS_BIT;

   mb();

   ret = notify_vc_and_wait_for_ack();

   mb();

   return ret;
}

static int player_set_src_region(bcm2708_mdec_set_source_region_t * data)
{
   int ret = 0;

   bcm2708mdec_dbg( "player source region set to %d,%d: %d x %d\n", data->x, data->y, data->width, data->height );

   MEDIA_DEC_REGISTER_RW( MEDIA_DEC_SOURCE_X_OFFSET ) = data->x;
   MEDIA_DEC_REGISTER_RW( MEDIA_DEC_SOURCE_Y_OFFSET ) = data->y;
   MEDIA_DEC_REGISTER_RW( MEDIA_DEC_SOURCE_WIDTH_OFFSET ) = data->width;
   MEDIA_DEC_REGISTER_RW( MEDIA_DEC_SOURCE_HEIGHT_OFFSET ) = data->height;
   MEDIA_DEC_REGISTER_RW( MEDIA_DEC_CONTROL_OFFSET ) |= MEDIA_DEC_CONTROL_SET_SRC_REGION_BIT;

   mb();

   ret = notify_vc_and_wait_for_ack();

   mb();

   return ret;
}

static int player_set_dest_region(bcm2708_mdec_set_dest_region_t * data )
{
   int ret = 0;

   bcm2708mdec_dbg( "player dest region set to (%d,%d,%d) %d,%d: %d x %d (%d,%d)\n",
                  data->display, data->fullscreen, data->layer, data->x, data->y, data->width, data->height,
                  data->transform, data->mode );

   MEDIA_DEC_REGISTER_RW( MEDIA_DEC_TARGET_DISPLAY_OFFSET ) = data->display;
   MEDIA_DEC_REGISTER_RW( MEDIA_DEC_TARGET_FULLSCREEN_OFFSET ) = data->fullscreen;
   MEDIA_DEC_REGISTER_RW( MEDIA_DEC_TARGET_LAYER_OFFSET ) = data->layer;
   MEDIA_DEC_REGISTER_RW( MEDIA_DEC_TARGET_X_OFFSET ) = data->x;
   MEDIA_DEC_REGISTER_RW( MEDIA_DEC_TARGET_Y_OFFSET ) = data->y;
   MEDIA_DEC_REGISTER_RW( MEDIA_DEC_TARGET_WIDTH_OFFSET ) = data->width;
   MEDIA_DEC_REGISTER_RW( MEDIA_DEC_TARGET_HEIGHT_OFFSET ) = data->height;
   MEDIA_DEC_REGISTER_RW( MEDIA_DEC_TARGET_TRANSFORM_OFFSET ) = data->transform;
   MEDIA_DEC_REGISTER_RW( MEDIA_DEC_TARGET_ASPECT_MODE ) = data->mode;
   MEDIA_DEC_REGISTER_RW( MEDIA_DEC_CONTROL_OFFSET ) |= MEDIA_DEC_CONTROL_SET_TARGET_REGION_BIT;

   mb();

   ret = notify_vc_and_wait_for_ack();

   mb();

   return ret;
}

static int player_set_transparency(bcm2708_mdec_set_transparency_t * data )
{
   int ret = 0;

   bcm2708mdec_dbg( "player transparency set to %d\n", data->alpha );

   MEDIA_DEC_REGISTER_RW( MEDIA_DEC_TRANSPARENCY_OFFSET ) = data->alpha;
   MEDIA_DEC_REGISTER_RW( MEDIA_DEC_CONTROL_OFFSET ) |= MEDIA_DEC_CONTROL_SET_TRANSPARENCY_BIT;

   mb();

   ret = notify_vc_and_wait_for_ack();

   mb();

   return ret;
}

static int player_get_property(bcm2708_mdec_get_property_t * data )
{
   int ret = 0;
   uint32_t which = 0;

   bcm2708mdec_dbg( "player retrieve property %d\n", data->property_id );

   switch( data->property_id )
   {
      case MDEC_PROPERTY_VOLUME:             which = MEDIA_DEC_CONTROL_GET_VOLUME_BIT; break;
      case MDEC_PROPERTY_VIDEO_BUFFER_LEVEL: which = MEDIA_DEC_CONTROL_GET_VIDEO_LEVEL_BIT; break;
      case MDEC_PROPERTY_AUDIO_BUFFER_LEVEL: which = MEDIA_DEC_CONTROL_GET_AUDIO_LEVEL_BIT; break;
      default:
         bcm2708mdec_dbg( "unknown property id %d\n", data->property_id );
         return -1;
   }

   MEDIA_DEC_REGISTER_RW( MEDIA_DEC_CONTROL_OFFSET ) |= which;

   mb();

   ret = notify_vc_and_wait_for_ack();

   mb();

   switch( data->property_id )
   {
      case MDEC_PROPERTY_VOLUME:
         data->out_value.volume = 0;
         break;
      case MDEC_PROPERTY_VIDEO_BUFFER_LEVEL:
         data->out_value.level = MEDIA_DEC_REGISTER_RW( MEDIA_DEC_VIDEO_LEVEL_OFFSET );
         break;
      case MDEC_PROPERTY_AUDIO_BUFFER_LEVEL:
         data->out_value.level = MEDIA_DEC_REGISTER_RW( MEDIA_DEC_AUDIO_LEVEL_OFFSET );
         break;
      default:
         // Should have already caught this
         return -1;
   }

   return ret;
}

static int player_wait_for_callback(bcm2708_mdec_wait_for_callback_t * data )
{
   uint32_t tail;
   MDEC_CALLBACK_INFO info;
   int ret = 0;
   volatile MDEC_CALLBACK_INFO * ringBuffer;


   // First, check to see if the buffer has content. If it does, we don't have to bother waiting on the sem
   tail = MEDIA_DEC_REGISTER_RW( MEDIA_DEC_CALLBACKS_TAIL_OFFSET );
   if( tail == MEDIA_DEC_REGISTER_RW( MEDIA_DEC_CALLBACKS_HEAD_OFFSET ) )
   {
      // No content yet, wait for a signal that there is
      ret = down_interruptible( &g_mdec->mUserCallbackSem );
      if( ret < 0 )
      {
         // We were interrupted by a signal, better bail back to the user (they'll restart if necessary)
         return -EINTR;
      }

      mb();

      // See if the buffer now has content
      tail = MEDIA_DEC_REGISTER_RW( MEDIA_DEC_CALLBACKS_TAIL_OFFSET );
      if( tail == MEDIA_DEC_REGISTER_RW( MEDIA_DEC_CALLBACKS_HEAD_OFFSET ) )
      {
         // This is not an error; we could have simply picked up a semaphore stating callbacks were ready
         // after we already picked them up. So we return that we were interrupted by a signal. It's not
         // entirely true, but the effect is the same--our block was stopped even though no data was there
         return -EINTR;
      }
   }

   mb();

   // Grab the next one out. Note we copy it out of the register space in one copy before splitting
   // it up for the callback.

   // The sad thing is that using the pointer as an array access would be far cleaner code-wise,
   // but it can be affected by alignment assumptions by the compiler. Since we're packing these
   // together and crossing between VC and ARM, we have to avoid all such assumptions.
   ringBuffer = (volatile MDEC_CALLBACK_INFO *)( (u32)g_mdec->reg_base +
                              MEDIA_DEC_CALLBACKS_RING_BUFFER_OFFSET +
                              ( ( tail & MEDIA_DEC_CALLBACK_BUFFER_SIZE_MASK ) * ( MDEC_CALLBACK_INFO_SIZE_IN_BYTES ) ) );
   info = *ringBuffer;
   data->mReason = info.mReason;
   data->mData = info.mData;
   data->mContext = info.mContext;

   // Increment tail. Just like the head increment in VC, this is safe on several fronts:
   // 1. VC will only read the value to see if the ring buffer is full, which means worst case
   //    (race condition) it will erroneously see the buffer as full when it isn't. Hopefully
   //    that will never happen, but if we're that close, most likely we're going to actually
   //    hit a full buffer condition soon anyway.
   // 2. We don't have to wrap the incremented value because that's taken care of on the
   //    access (above) and also so we know the difference beteween full and empty (see VC side)
   // 3. We don't have to worry about overflow because the is-full math still works out correctly
   //    (also see VC side)
   tail++;
   MEDIA_DEC_REGISTER_RW( MEDIA_DEC_CALLBACKS_TAIL_OFFSET ) = tail;

   // All good!
   return 0;
}

static int player_stop_callback_dispatch( void )
{
   // Kick the callback semaphore so the userspace dispatch thread can wake up and 
   // terminate
   up( &g_mdec->mUserCallbackSem );
   
   return 0;
}

static int mdec_open( struct inode *inode, struct file *file_id)
{
   bcm2708mdec_dbg( "Device opened; initializing callback sem\n" );
   sema_init( &g_mdec->mUserCallbackSem, 0 );
   bcm2708mdec_dbg( "   %08x %d %p %p\n", *( (uint32_t *)(&g_mdec->mUserCallbackSem.lock) ), g_mdec->mUserCallbackSem.count, g_mdec->mUserCallbackSem.wait_list.next, g_mdec->mUserCallbackSem.wait_list.prev );
   return 0;
}

static int mdec_release( struct inode *inode, struct file *file_id )
{
   int ret = 0;

   if (g_mdec->av_stream_ctl.state !=  PLAYBACK_IDLE)
      ret = player_stop();

   return ret;
}

static ssize_t mdec_read( struct file *file, char *buffer, size_t count, loff_t *ppos )
{
   return -EINVAL;
}

static ssize_t mdec_write( struct file *file, const char *buffer, size_t count, loff_t *ppos )
{
        return -EINVAL;
}

/* The main dispatch for ioctl calls coming in to the driver.
   WARNING: there are multiple threads that could be calling this function, so it
   MUST BE REENTRANT! */
static int mdec_ioctl( struct inode *inode, struct file *file_id, unsigned int cmd, unsigned long arg )
{
   int ret = 0;
   unsigned long uncopied;

   // Local stack storage for the command buffer. 512 bytes should be safe on the stack...right?
   u8 ioctl_cmd_buf[ MAX_BCM2708_MDEC_IOCTL_CMD_SIZE ];
   BUG_ON(MAX_BCM2708_MDEC_IOCTL_CMD_SIZE < _IOC_SIZE(cmd));

   // Copy command buffer in if it's necessary
   if( ( ( _IOC_DIR( cmd ) & _IOC_WRITE ) != 0 ) && ( _IOC_SIZE( cmd ) > 0 ) )
   {
      uncopied = copy_from_user( ioctl_cmd_buf, (void *)arg, _IOC_SIZE( cmd ) );
      if( uncopied != 0 )
      {
         return -EFAULT;
      }
   }

   switch (cmd)
   {
      case MDEC_IOCTL_PLAYER_SETUP:
         ret = player_setup((bcm2708_mdec_setup_t *)ioctl_cmd_buf);
         break;

      case MDEC_IOCTL_PLAYER_START:
         ret = player_start();
         break;

      case MDEC_IOCTL_PLAYER_SEND_VIDEO_DATA:
         ret = player_send_video_data((bcm2708_mdec_send_data_t *)ioctl_cmd_buf);
         break;

      case MDEC_IOCTL_PLAYER_SEND_AUDIO_DATA:
         ret = player_send_audio_data((bcm2708_mdec_send_data_t *)ioctl_cmd_buf);
         break;

      case MDEC_IOCTL_PLAYER_STOP:
         ret = player_stop();
         break;

      case MDEC_IOCTL_PLAYER_LOCAL_DBG:
         do_playback((bcm2708_mdec_play_t *)ioctl_cmd_buf);
         break;

      case MDEC_IOCTL_PLAYER_PAUSE:
         ret = player_set_paused( -1 );
         break;

      case MDEC_IOCTL_PLAYER_RESUME:
         ret = player_set_paused( 0 );
         break;

      case MDEC_IOCTL_PLAYER_SET_VOLUME:
         ret = player_set_volume( (bcm2708_mdec_set_volume_t *)ioctl_cmd_buf );
         break;

      case MDEC_IOCTL_PLAYER_SET_MUTED:
         ret = player_set_muted( (bcm2708_mdec_set_muted_t *)ioctl_cmd_buf );
         break;

      case MDEC_IOCTL_PLAYER_GET_PROPERTY:
         ret = player_get_property( (bcm2708_mdec_get_property_t *)ioctl_cmd_buf );
         break;

      case MDEC_IOCTL_PLAYER_SET_SOURCE_REGION:
         ret = player_set_src_region( (bcm2708_mdec_set_source_region_t *)ioctl_cmd_buf );
         break;

      case MDEC_IOCTL_PLAYER_SET_DEST_REGION:
         ret = player_set_dest_region( (bcm2708_mdec_set_dest_region_t *)ioctl_cmd_buf );
         break;

      case MDEC_IOCTL_PLAYER_SET_TRANSPARENCY:
         ret = player_set_transparency( (bcm2708_mdec_set_transparency_t *)ioctl_cmd_buf );
         break;

      /* Note: this will be coming in from a separate thread in libmedia_dec that will then
         dispatch callbacks back to the user level. Hence why blocking here is appropriate
         (and safe). */
      case MDEC_IOCTL_PLAYER_WAIT_FOR_CALLBACK:
         ret = player_wait_for_callback( (bcm2708_mdec_wait_for_callback_t *)ioctl_cmd_buf );
         break;
         
      /* This call should be coming in from the main thread, to request that we release
         any blocking ioctl call on a separate thread for WAIT_FOR_CALLBACK */
      case MDEC_IOCTL_PLAYER_STOP_CALLBACK_DISPATCH:
         ret = player_stop_callback_dispatch();
         break;
         
      default: 
         bcm2708mdec_error("Wrong IOCTL cmd\n");
         ret = -EFAULT;
         break;
   }

   if( ret != 0 )
      return ret;

   if( ( ( _IOC_DIR( cmd ) & _IOC_READ ) != 0 ) && ( _IOC_SIZE( cmd ) > 0 ) )
   {
      uncopied = copy_to_user( (void *)arg, ioctl_cmd_buf, _IOC_SIZE( cmd ) );
      if( uncopied != 0 )
         return -EFAULT;
   }

   return ret;
}

static int mdec_mmap(struct file *filp, struct vm_area_struct *vma)
{
   return 0;
}

static struct file_operations mdec_file_ops =
{
    owner:      THIS_MODULE,
    open:       mdec_open,
    release:    mdec_release,
    read:       mdec_read,
    write:      mdec_write,
    ioctl:      mdec_ioctl,
    mmap:       mdec_mmap,
};

static struct proc_dir_entry *mdec_create_proc_entry( const char * const name,
                                                     read_proc_t *read_proc,
                                                     write_proc_t *write_proc )
{
   struct proc_dir_entry *ret = NULL;

   ret = create_proc_entry( name, 0644, NULL);

   if (ret == NULL)
   {
      remove_proc_entry( name, NULL);
      printk(KERN_ALERT "could not initialize %s", name );
   }
   else
   {
      ret->read_proc  = read_proc;
      ret->write_proc = write_proc;
      ret->mode           = S_IFREG | S_IRUGO;
      ret->uid    = 0;
      ret->gid    = 0;
      ret->size           = 37;
   }
   return ret;
}

static int mdec_dummy_read(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
   int len = 0;

   if (offset > 0)
   {
      *eof = 1;
      return 0;
   }

   *eof = 1;

   return len;
}

#define INPUT_MAX_INPUT_STR_LENGTH   256

static int mdec_proc_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
        char *init_string = NULL;
        int num;
	int timeout = 0;

        init_string = vmalloc(INPUT_MAX_INPUT_STR_LENGTH);

   if(NULL == init_string)
      return -EFAULT;

   memset(init_string, 0, INPUT_MAX_INPUT_STR_LENGTH);

   count = (count > INPUT_MAX_INPUT_STR_LENGTH) ? INPUT_MAX_INPUT_STR_LENGTH : count;

   if(copy_from_user(init_string, buffer, count))
   {
      return -EFAULT;
   }
   init_string[ INPUT_MAX_INPUT_STR_LENGTH  - 1 ] = 0;

   num = simple_strtol(init_string, 0, 0);
   bcm2708mdec_dbg("read from /proc is %d\n", num);

   //write anything to this proc entry and it will kill any instance of the media decoder running (usually the splash screen)
   //no state in the rest of the driver is checked!
   MEDIA_DEC_REGISTER_RW( MEDIA_DEC_CONTROL_OFFSET ) = 0x0;
   ipc_notify_vc_event(g_mdec->irq);
   while (MEDIA_DEC_REGISTER_RW( MEDIA_DEC_STATUS_OFFSET)) {
      schedule_timeout(10);
      timeout++;
      if (timeout > 100) {
         printk( KERN_ERR "Error disabling MDEC control!\n");
         break;
      }   
   }

   vfree(init_string);

   return count;
}

static irqreturn_t bcm2708_mdec_isr(int irq, void *dev_id)
{
   uint32_t numCallbacksWaiting;

   bcm2708mdec_dbg("The MDEC device rxed one interrupt");

   /* There are three reasons we could have received an interrupt:
      1. Command acknowledgement, in which case the register bitfield for that will be set
      2. Data buffer was consumed and can be filled again (no direct knowledge of that)
      3. One or more user-level callbacks are ready to be propagated up (the callback ring
         buffer will be non-empty)
      Since #2 can't be immediately determined, we kick that thread no matter what,
      and it'll take care of checking to see if any buffers are actually waiting to be refilled.
      The other two, we actually check and only kick their semaphores if their conditions have
      been met. */

   if( MEDIA_DEC_REGISTER_RW( MEDIA_DEC_COMMAND_ACK_OFFSET ) != 0 )
   {
      /* See the register doc for this, but it's safe from race conditions and such */
      MEDIA_DEC_REGISTER_RW( MEDIA_DEC_COMMAND_ACK_OFFSET ) = 0;

      bcm2708mdec_dbg( "  Propagating command acknowledgement");
      up( &g_mdec->vc_ack_sem );
   }

   /* Kick the data threads no matter what. Ideally, we should actually check first... */
   if (g_mdec->av_stream_ctl.playback_type & AUDIO_MASK)
   {
      up(&g_mdec->av_stream_ctl.audio_ctl.vc_buf_sem);
      up(&g_mdec->av_stream_ctl.audio_ctl.arm_buf_sem);
   }
   if (g_mdec->av_stream_ctl.playback_type & VIDEO_MASK)
   {
      up(&g_mdec->av_stream_ctl.video_ctl.vc_buf_sem);
      up(&g_mdec->av_stream_ctl.video_ctl.arm_buf_sem);
   }

   /* Handle user-level callbacks. Note that the ring buffer is actually
      emptied by the ioctl call waiting on the callback; we just signal for every
      callback that is waiting so the blocking ioctl call will properly wake up
      and pop-and-return each one. For proof this math works, check the VC side documentation
      that writes to these registers. */
   numCallbacksWaiting = MEDIA_DEC_REGISTER_RW( MEDIA_DEC_CALLBACKS_HEAD_OFFSET ) -
                                  MEDIA_DEC_REGISTER_RW( MEDIA_DEC_CALLBACKS_TAIL_OFFSET );
   if( numCallbacksWaiting == MEDIA_DEC_CALLBACK_BUFFER_SIZE )
      bcm2708mdec_error( "  Propagating %d user-level callback notifications to sem %p (RING BUFFER FULL WARNING)", (int)numCallbacksWaiting, &g_mdec->mUserCallbackSem );
   if( numCallbacksWaiting > 0 )
   {
      bcm2708mdec_dbg( "  Propagating %d user-level callback notifications to sem %p", (int)numCallbacksWaiting, &g_mdec->mUserCallbackSem );
      up( &g_mdec->mUserCallbackSem );
   }

   return IRQ_HANDLED;
}

struct miscdevice mdec_misc_dev = {
    .minor =    MISC_DYNAMIC_MINOR,
    .name =     BCM2708_MDEC_DRIVER_NAME,
    .fops =     &mdec_file_ops
};

static int bcm2708_mdec_probe(struct platform_device *pdev)
{
        int ret = -ENXIO;
        struct resource *r;
   struct bcm2708_mdec *bcm_mdec = NULL;

        bcm_mdec = kzalloc(sizeof(struct bcm2708_mdec), GFP_KERNEL);
        if (bcm_mdec == NULL) {
                bcm2708mdec_error("Unable to allocate mdec structure\n");
                ret = -ENOMEM;
                goto err_mdec_alloc_failed;
        }
   g_mdec = bcm_mdec;
        platform_set_drvdata(pdev, bcm_mdec);

   r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
        if(r == NULL) {
                bcm2708mdec_error("Unable to get mdec memory resource\n");
                ret = -ENODEV;
                goto err_no_io_base;
        }
        bcm2708mdec_dbg("MDEC registers start-end (0x%08x)-(0x%08x)\n", r->start, r->end);
   bcm_mdec->reg_base = (void __iomem *)r->start;

   bcm_mdec->irq = platform_get_irq(pdev, 0);
        if(bcm_mdec->irq < 0) {
                bcm2708mdec_error("Unable to get mdec irq resource\n");
                ret = -ENODEV;
                goto err_no_irq;
        }

        ret = request_irq(bcm_mdec->irq, bcm2708_mdec_isr, IRQF_DISABLED,
                                "bcm2708 mdec interrupt", (void *)bcm_mdec);
        if (ret < 0) {
                bcm2708mdec_error("Unable to register Interrupt for bcm2708 MDEC\n");
                goto err_no_irq;
        }

   // TODO: Initing this twice seems bad?
   sema_init(&g_mdec->vc_ack_sem, 0);

//   sema_init( &g_mdec->mUserCallbackSem, 0 );

   ret = misc_register(&mdec_misc_dev);
   if (ret < 0) {
      bcm2708mdec_error("failed to register char device\n");
      goto err_reg_chrdev;
   }

    mdec_create_proc_entry("bcm2835_mdec", mdec_dummy_read, mdec_proc_write);

    bcm2708mdec_dbg("The MDEC device is probed successfully");

   return 0;

err_reg_chrdev:
   free_irq(bcm_mdec->irq, NULL);
err_no_irq:
err_no_io_base:
   kfree(bcm_mdec);
err_mdec_alloc_failed:
   return ret;

}

static int __devexit bcm2708_mdec_remove(struct platform_device *pdev)
{
        struct bcm2708_mdec *bcm_mdec = platform_get_drvdata(pdev);

   free_irq(bcm_mdec->irq, NULL);
   misc_deregister(&mdec_misc_dev);
        kfree(bcm_mdec);
        bcm2708mdec_dbg("BCM2708 MDEC device removed!!\n");

        return 0;
}


static struct platform_driver bcm2708_mdec_driver = {
        .probe          = bcm2708_mdec_probe,
        .remove         = __devexit_p(bcm2708_mdec_remove),
        .driver = {
                .name = "bcm2835_MEDD"
        }
};

static int __init bcm2708_mdec_init(void)
{
        int ret;

        ret = platform_driver_register(&bcm2708_mdec_driver);
        if (ret)
                printk(KERN_ERR BCM2708_MDEC_DRIVER_NAME "%s : Unable to register BCM2708 MDEC driver\n", __func__);

        printk(KERN_INFO BCM2708_MDEC_DRIVER_NAME "Init %s !\n", ret ? "FAILED" : "OK");

        return ret;
}

static void __exit bcm2708_mdec_exit(void)
{
        /* Clean up .. */
        platform_driver_unregister(&bcm2708_mdec_driver);

        printk(KERN_INFO BCM2708_MDEC_DRIVER_NAME "BRCM MDEC driver exit OK\n");
}

module_init(bcm2708_mdec_init);
module_exit(bcm2708_mdec_exit);
