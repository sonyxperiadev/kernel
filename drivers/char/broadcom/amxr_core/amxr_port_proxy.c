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
/**
*
*  @file	amxr_port_proxy.c
*
*  @brief	AMXR port proxy to support user side ports.
*
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/mm_types.h>
#include <linux/mm.h>
#include <linux/sysctl.h>
#include <linux/semaphore.h>
#include <linux/list.h>
#include <linux/string.h>
#include <linux/cdev.h>
#include <linux/ioctl.h>
#include <linux/spinlock.h>
#include <linux/device.h>
#include <linux/vmalloc.h>
#include <asm/atomic.h>
#include <asm/uaccess.h>

#include <linux/broadcom/amxr.h>
#include <linux/broadcom/amxr_port.h>
#include <linux/broadcom/amxr_port_ioctl.h>

/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */

#define PERIOD_USEC		5000
#define MAX_SAMPLING_FREQ	48000

/* Number of dst and src buffers. The more buffers, the more delay in the
 * source buffer direction
 */
#define NUM_BUFS		1

/* Number of message buffers used for messaging between kernel and
 * user proxy ports
 */
#define NUM_MSGS		32

/* Number of source buffers used for low latency direct write for providing
 * source data instead of using AMXR src callbacks
 */
#define NUM_SRC_BUFS		4


/* Debug trace */
#define TRACE_ENABLED		0
#if TRACE_ENABLED
#include <linux/broadcom/knllog.h>
#define PPXY_TRACE		KNLLOG
#else
#define PPXY_TRACE(c...)
#endif

#define ENABLE_DBG_PROC		0
#if ENABLE_DBG_PROC
#define DBG_PROC_NAME		"amxrproxyport"
#endif

struct ppxy_buf
{
	spinlock_t 		lock;
	char 			*bufp;		/* Src buffers */
	int 			cnxs;		/* Flag for src cnxs exist */
	int			bufidx;
	int			frame_bytes;
};

struct ppxy_list
{
	struct list_head	list;		/* List head */
	struct mutex		mutex;		/* Protect list */
	unsigned int		total;		/* Total elements in list */
};

struct ppxy_src_buf
{
	struct list_head	lnode;		/* List node */

	atomic_t 		alloc;		/* Allocated flag */
	char 			*bufp;		/* Buffer pointer */
	int 			allocsz;	/* Allocated size in bytes */

	int 			bytes;		/* Amount of data stored in bytes */
	int 			offset;		/* Amount read */
};

/*
 * Port node definition
 *
 * Note: This structure should not exceed PAGE_SIZE as it is shared to user
 * 	 space. An alternative is just to share the memory buffers.
 */
struct ppxy_node
{
	struct list_head	lnode;		/* List node */
	AMXR_PORT_ID		portid;		/* Real port ID */
	AMXR_PORT_CB		user_cbks;	/* Copy of user callbacks */

	struct ppxy_buf		src;
	struct ppxy_buf		dst;

	spinlock_t 		msg_wlock;	/* Write lock */
	struct amxr_port_msg 	msgs[NUM_MSGS];	/* Msg buffers */
	int 			msg_readi;	/* Msg read index */
	int 			msg_writei;	/* Msg write index */
	atomic_t		msg_num;	/* Number of msgs */
	struct semaphore	msg_avail;	/* Msg available sem */
	atomic_t		msg_alloc_errs;	/* Msg alloc errors */

	struct list_head	srcbuf_list;	/* List of active src buffers */
	spinlock_t		srcbuf_list_lock;

	struct ppxy_src_buf	srcbufs[NUM_SRC_BUFS];
	struct mutex		srcbuf_alloc_mutex;
};

/* Union of IOCTL parameter structure used to reserve stack space */
union ioctl_params
{
	struct amxr_port_ioctl_createport createport;
	struct amxr_port_ioctl_setportfreq setportfreq;
	struct amxr_port_ioctl_setportchans setportchans;
};

/* ---- Private Variables ------------------------------------------------ */
static struct ppxy_list gPortList;

static dev_t gDevno;
static struct cdev gCdev;
static struct class *gClass;
static struct device *gDev;

/* ---- Private Function Prototypes -------------------------------------- */
static int ppxy_open( struct inode *inode, struct file *filp );
static int ppxy_release( struct inode *inode, struct file *filp );
static long ppxy_ioctl( struct file *file, unsigned int cmd, unsigned long arg );
static int ppxy_mmap( struct file *filp, struct vm_area_struct *vma );
static ssize_t ppxy_write( struct file *file, const char __user *buffer, size_t count, loff_t *ppos );

/* ---- Functions -------------------------------------------------------- */

static struct file_operations gfops =
{
	.owner		= THIS_MODULE,
	.open		= ppxy_open,
	.release	= ppxy_release,
	.unlocked_ioctl = ppxy_ioctl,
	.mmap		= ppxy_mmap,
	.write		= ppxy_write,
};

/***************************************************************************/
/**
*  Allocate port message buffer used for sending control data to user
*  client.
*
*  @return 	Pointer to alloc'd message buffer, otherwise NULL
*/
static inline struct amxr_port_msg *alloc_msg( struct ppxy_node *nodep )
{
	unsigned long flags;
	struct amxr_port_msg *msgp = NULL;

	if ( atomic_read( &nodep->msg_num ) >= NUM_MSGS )
	{
		atomic_inc( &nodep->msg_alloc_errs );
		return NULL;
	}

	atomic_inc( &nodep->msg_num );
	spin_lock_irqsave( &nodep->msg_wlock, flags );
	msgp = &nodep->msgs[nodep->msg_writei];
	nodep->msg_writei = (nodep->msg_writei + 1) % NUM_MSGS;
	spin_unlock_irqrestore( &nodep->msg_wlock, flags );

	return msgp;
}

/***************************************************************************/
/**
*  Helper to send message
*
*  @return 	Pointer to alloc'd message buffer, otherwise NULL
*/
static void send_msg(
	struct ppxy_node *nodep,
	enum amxr_port_msgid msgid,
	long buf_pgoff,
	long bytes
)
{
	struct amxr_port_msg *msgp;
	PPXY_TRACE( "msgid=0x%x buf_pgoff=0x%x bytes=%i", msgid, buf_pgoff, bytes );

	msgp = alloc_msg( nodep );
	if ( msgp == NULL )
	{
		PPXY_TRACE( "Failed to alloc msg" );
		/* Cannot allocate message. Drop frame */
		return;
	}
	msgp->msgid = msgid;
	msgp->buf_pgoff = buf_pgoff;
	msgp->bytes = bytes;
	up( &nodep->msg_avail );
}

/***************************************************************************/
/**
*  Allocate a buffer used to store source data for kernel proxy port.
*/
struct ppxy_src_buf *alloc_src_buf( struct ppxy_node *nodep, int bytes )
{
	int i;
	struct ppxy_src_buf *srcbufp = nodep->srcbufs;
	struct ppxy_src_buf *newsrcbufp = NULL;

	mutex_lock( &nodep->srcbuf_alloc_mutex );
	for ( i = 0; i < NUM_SRC_BUFS; i++, srcbufp++ )
	{
		if ( !atomic_read( &srcbufp->alloc ))
		{
			if ( srcbufp->allocsz < bytes )
			{
				/* Re-allocate buffer */
				void *newbufp = vmalloc( bytes );
				if ( newbufp )
				{
					vfree( srcbufp->bufp );
					srcbufp->bufp = newbufp;
					srcbufp->allocsz = bytes;
				}
				else
				{
					/* Insufficient memory */
					break;
				}
			}
			atomic_set( &srcbufp->alloc, 1 );
			srcbufp->bytes = bytes;
			srcbufp->offset = 0;
			newsrcbufp = srcbufp;
			break;
		}
	}
	mutex_unlock( &nodep->srcbuf_alloc_mutex );

	return newsrcbufp;
}

/***************************************************************************/
/**
*  Lazy free of source buffer. Actual memory is not freed, but instead
*  buffer is marked to allow subsequent reuse. Typically called within
*  an atomic context.
*/
static void free_src_buf( struct ppxy_src_buf *srcbufp )
{
	atomic_set( &srcbufp->alloc, 0 );
}

/***************************************************************************/
/**
*  Callback for getting source data from low latency ports
*
*  @return
*     NULL     - non-matching frame size or non-existent buffer
*     ptr      - pointer to source buffer
*/
static int16_t *getsrc_cb_lowlatency(
	int	bytes,		/**< (i) frame size in bytes */
	void 	*privdata	/**< (i) private data  */
)
{
	struct ppxy_node *nodep = privdata;
	struct ppxy_src_buf *srcbufp;

	PPXY_TRACE( "bytes=%i", bytes );

	if ( !list_empty( &nodep->srcbuf_list ))
	{
		srcbufp = list_first_entry( &nodep->srcbuf_list,
				struct ppxy_src_buf, lnode );
	}
	else
	{
		PPXY_TRACE( "list empty" );
		return NULL;
	}

	if ( bytes > ( srcbufp->bytes - srcbufp->offset ))
	{
		/* Sanity check */
		PPXY_TRACE( "insufficient samples" );
		return NULL;
	}

	PPXY_TRACE( "bytes=%i offset=%i", bytes, srcbufp->offset );

	return (int16_t *)(srcbufp->bufp + srcbufp->offset);
}

/***************************************************************************/
/**
*  Callback for source data complete for low latency ports
*/
static void srcdone_cb_lowlatency(
	int   bytes,		/**< (i) size of the buffer in bytes */
	void *privdata		/**< (i) user supplied data */
)
{
	struct ppxy_node *nodep = privdata;
	struct ppxy_src_buf *srcbufp;

	PPXY_TRACE( "bytes=%i ", bytes );

	if ( !list_empty( &nodep->srcbuf_list ))
	{
		srcbufp = list_first_entry( &nodep->srcbuf_list,
				struct ppxy_src_buf, lnode );

		PPXY_TRACE( "offset=%i ", srcbufp->offset );
		srcbufp->offset += bytes;

		if ( srcbufp->offset >= srcbufp->bytes )
		{
			spin_lock( &nodep->srcbuf_list_lock );
			list_del( &srcbufp->lnode );
			spin_unlock( &nodep->srcbuf_list_lock );

			free_src_buf( srcbufp );
		}
	}

	if ( nodep->user_cbks.srcdone )
	{
		PPXY_TRACE( "user srcdone callback signaled" );
		send_msg( nodep, AMXR_PORT_SRCDONE_MSG,
				-1 /* no shared buffers */, bytes );
	}
}

/***************************************************************************/
/**
*  Callback for getting source data for standard proxy ports.
*
*  @return
*     NULL     - non-matching frame size or non-existent buffer
*     ptr      - pointer to source buffer
*/
static int16_t *getsrc_cb(
	int	bytes,		/**< (i) frame size in bytes */
	void 	*privdata	/**< (i) private data  */
)
{
	struct ppxy_node *nodep = privdata;
	struct ppxy_buf *bp = &nodep->src;
	bp->cnxs = 1;
	if ( bytes > bp->frame_bytes )
	{
		/* Sanity check */
		return NULL;
	}
	PPXY_TRACE( "bytes=%i idx=%i", bytes, bp->bufidx );
#if NUM_BUFS > 1
	return (int16_t *)(bp->bufp + (bp->bufidx * bp->frame_bytes));
#else
	return (int16_t *)bp->bufp;
#endif
}

/***************************************************************************/
/**
*  Callback for source data complete for standard proxy ports
*/
static void srcdone_cb(
	int   bytes,		/**< (i) size of the buffer in bytes */
	void *privdata		/**< (i) user supplied data */
)
{
	struct ppxy_node *nodep = privdata;
	long buf_pgoff;

	if ( nodep->src.cnxs && nodep->src.bufp )
	{
#if NUM_BUFS > 1
		buf_pgoff = nodep->src.bufidx * nodep->src.frame_bytes;
		nodep->src.bufidx = (nodep->src.bufidx + 1) % NUM_BUFS;
#else
		buf_pgoff = 0;
#endif
		nodep->src.cnxs = 0;
	}
	else
	{
		/* Indicate no buffers needed */
		buf_pgoff = -1;
	}

	PPXY_TRACE( "bytes=%i buf_pgoff=0x%x", bytes, buf_pgoff );

	send_msg( nodep, AMXR_PORT_SRCDONE_MSG, buf_pgoff, bytes );
}

/***************************************************************************/
/**
*  Callback for getting destination data
*
*  @return
*     NULL     - non-matching frame size or non-existent buffer
*     ptr      - pointer to destination buffer
*/
static int16_t *getdst_cb(
	int	bytes,		/**< (i) frame size in bytes */
	void 	*privdata	/**< (i) private data  */
)
{
	struct ppxy_node *nodep = privdata;
	struct ppxy_buf *bp = &nodep->dst;
	bp->cnxs = 1;
	if ( bytes > bp->frame_bytes )
	{
		/* Sanity check */
		return NULL;
	}
	PPXY_TRACE( "bytes=%i idx=%i", bytes, bp->bufidx );
#if NUM_BUFS > 1
	return (int16_t *)(bp->bufp +
			bp->frame_bytes * bp->bufidx);
#else
	return (int16_t *)bp->bufp;
#endif
}

/***************************************************************************/
/**
*  Callback for destination data complete
*/
static void dstdone_cb(
	int   bytes,		/**< (i) size of the buffer in bytes */
	void *privdata		/**< (i) user supplied data */
)
{
	struct ppxy_node *nodep = privdata;
	long buf_pgoff;

	if ( nodep->dst.cnxs && nodep->dst.bufp )
	{
#if NUM_BUFS > 1
		buf_pgoff = nodep->dst.bufidx * nodep->dst.frame_bytes;
		nodep->dst.bufidx = (nodep->dst.bufidx + 1) % NUM_BUFS;
#else
		buf_pgoff = 0;
#endif
		nodep->dst.cnxs = 0;
	}
	else
	{
		/* Indicate no buffers needed */
		buf_pgoff = -1;
	}

	PPXY_TRACE( "bytes=%i buf_pgoff=0x%x", bytes, buf_pgoff );

	send_msg( nodep, AMXR_PORT_DSTDONE_MSG, buf_pgoff, bytes );
}

/***************************************************************************/
/**
*  Callback for removing all connections from destination
*/
static void dstcnxsremoved_cb(
	void *privdata		/**< (i) user supplied data */
)
{
	struct ppxy_node *nodep = privdata;
	PPXY_TRACE( "removed" );
	send_msg( nodep, AMXR_PORT_DSTCNXSREMOVED_MSG, 0, 0 );
}

/***************************************************************************/
/**
*  Get node pointer from handle
*
*  @return
*     0        On success
*     -ve      Error code
*/
static inline struct ppxy_node *getNodep( AMXR_PORT_ID portid )
{
	return portid;
}

/***************************************************************************/
/**
*  Helper to reallocate buffers
*/
static int realloc_data_buf( struct ppxy_buf *bp, int new_bytes )
{
	if ( bp->frame_bytes < new_bytes )
	{
		unsigned long flags;
		char *new_bufp, *old_bufp;
		int size;

		/* Rounded up to next page size for page alignment */
		size = (new_bytes * NUM_BUFS + PAGE_SIZE-1 ) & PAGE_MASK;
		new_bufp = kmalloc( size, GFP_KERNEL );
		if ( new_bufp == NULL )
		{
			return -ENOMEM;
		}
		if ( (long)new_bufp & ~PAGE_MASK )
		{
			printk( KERN_ERR "%s: memory is not aligned 0x%lx size=%i\n",
					__FUNCTION__, (unsigned long)new_bufp, size );
			kfree( new_bufp );
			return -ENOMEM;
		}

		old_bufp = bp->bufp;

		spin_lock_irqsave( &bp->lock, flags );
		bp->bufp = new_bufp;
		bp->frame_bytes = new_bytes;
		spin_unlock_irqrestore( &bp->lock, flags );

		if ( old_bufp ) kfree( old_bufp );
	}
	return 0;
}

/***************************************************************************/
/**
*  Helper to free buffers
*
*  @return
*     0        On success
*/
static void free_data_buf( struct ppxy_buf *bp )
{
	if ( bp->bufp )
	{
		kfree( bp->bufp );
		bp->bufp = NULL;
	}
}

/***************************************************************************/
/**
*  Create a user-side mixer port.
*
*  @return
*     0        On success
*     -ve      Error code
*/
int amxrCreatePortProxy(
	const char *name,	/**< (i) Name string */
	AMXR_PORT_CB *cb,	/**< (i) Callbacks */
	int dst_hz,		/**< (i) Current destination sampling frequency in Hz */
	int dst_chans,		/**< (i) Number of channels, i.e. mono = 1 */
	int dst_bytes,		/**< (i) Destination period size in bytes */
	int src_hz,		/**< (i) Current source sampling frequency in Hz */
	int src_chans,		/**< (i) Number of channels, i.e. mono = 1 */
	int src_bytes,		/**< (i) Source period size in bytes */
	AMXR_PORT_ID *portidp	/**< (o) Ptr to port ID */
)
{
	struct ppxy_node *nodep;
	AMXR_PORT_CB callbacks;
	int rc;
	int i;
	struct ppxy_src_buf *tmpbufp;

	/* Alloc is rounded up to next page size for page alignment */
	nodep = kmalloc( (sizeof(*nodep)+PAGE_SIZE-1) & PAGE_MASK, GFP_KERNEL );
	if ( nodep == NULL )
	{
		return -ENOMEM;
	}
	memset( nodep, 0, sizeof(*nodep) );

	spin_lock_init( &nodep->src.lock );
	spin_lock_init( &nodep->dst.lock );
	spin_lock_init( &nodep->msg_wlock );
	atomic_set( &nodep->msg_num, 0 );
	atomic_set( &nodep->msg_alloc_errs, 0 );
	sema_init( &nodep->msg_avail, 0 );

	mutex_init( &nodep->srcbuf_alloc_mutex );
	INIT_LIST_HEAD( &nodep->srcbuf_list );
	spin_lock_init( &nodep->srcbuf_list_lock );

	memcpy( &nodep->user_cbks, cb, sizeof(nodep->user_cbks) );
	memset( &callbacks, 0, sizeof(callbacks) );

	if ( cb->getsrc && src_bytes )
	{
		/* Allocate shared source buffer */
		rc = realloc_data_buf( &nodep->src, src_bytes );
		if ( rc < 0 )
		{
			goto out_free_mem;
		}
		*nodep->src.bufp = 0x55; /* debug magic */

		callbacks.getsrc = getsrc_cb;
	}
	else if ( src_bytes )
	{
		/* Low-latency src buffer mode choosen since user getsrc
		 * callback is not specified, but src channel properties are
		 * non-zero.
		 *
		 * Pre-allocate low latency source buffers
		 */
		tmpbufp = nodep->srcbufs;
		for ( i = 0; i < NUM_SRC_BUFS; i++, tmpbufp++ )
		{
			atomic_set( &tmpbufp->alloc, 0 );
			tmpbufp->bufp = vmalloc( PAGE_SIZE );
			if ( tmpbufp->bufp == NULL )
			{
				rc = -ENOMEM;
				goto out_free_mem;
			}
			tmpbufp->allocsz = PAGE_SIZE;
		}

		callbacks.getsrc = getsrc_cb_lowlatency;
		callbacks.srcdone = srcdone_cb_lowlatency;
	}

	if ( cb->getdst && dst_bytes )
	{
		/* Allocate shared dst buffer */
		rc = realloc_data_buf( &nodep->dst, dst_bytes );
		if ( rc < 0 )
		{
			goto out_free_mem2;
		}
		*nodep->dst.bufp = 0xdd; /* debug magic */

	}

	if ( callbacks.srcdone == NULL && ( cb->getsrc || cb->srcdone ))
	{
		/* Only init srcdone callback if not already set to
		 * not overwrite low latency setup
		 */
		callbacks.srcdone = srcdone_cb;
	}

	callbacks.getdst = cb->getdst ? getdst_cb : NULL;
	callbacks.dstdone = ( cb->getdst || cb->dstdone ) ? dstdone_cb : NULL;
	callbacks.dstcnxsremoved = cb->dstcnxsremoved ? dstcnxsremoved_cb : NULL;

	rc = amxrCreatePort( name, &callbacks, nodep /* kernel priv data */,
			dst_hz, dst_chans, dst_bytes, src_hz,
			src_chans, src_bytes, &nodep->portid );
	if ( rc )
	{
		goto out_free_mem3;
	}

	mutex_lock( &gPortList.mutex );
	list_add_tail( &nodep->lnode, &gPortList.list );
	gPortList.total++;
	mutex_unlock( &gPortList.mutex );

	*portidp = nodep;
	return 0;

out_free_mem3:
	free_data_buf( &nodep->dst );
out_free_mem2:
	free_data_buf( &nodep->src );
out_free_mem:
	tmpbufp = nodep->srcbufs;
	for ( i = 0; i < NUM_SRC_BUFS; i++, tmpbufp++ )
	{
		if ( tmpbufp->bufp )
		{
			vfree( tmpbufp->bufp );
		}
	}
	kfree( nodep );
	return rc;
}

/***************************************************************************/
/**
*  Remove an existing mixer port. All connections to this port will also
*  be removed.
*
*  @return
*     0           On success
*     -ve         On general failure
*
*  @remarks
*     Typically ports are not removed unless the entire user is removed.
*     Although it is conceivable that the API supports dynamic creation and
*     deletion of ports, it is not practical for most applications since
*     applications will have to constantly query what ports still exists
*     and keep track of valid port IDs.
*/
int amxrRemovePortProxy(
	AMXR_PORT_ID portid		/**< (i) Port to remove */
)
{
	struct ppxy_node *nodep = getNodep( portid );
	struct ppxy_src_buf *tmpbufp;
	int rc;
	int i;

	mutex_lock( &gPortList.mutex );
	list_del( &nodep->lnode );
	gPortList.total--;
	mutex_unlock( &gPortList.mutex );

	rc = amxrRemovePort( nodep->portid );

	free_data_buf( &nodep->dst );
	free_data_buf( &nodep->src );

	tmpbufp = nodep->srcbufs;
	for ( i = 0; i < NUM_SRC_BUFS; i++, tmpbufp++ )
	{
		if ( tmpbufp->bufp )
		{
			vfree( tmpbufp->bufp );
		}
	}
	kfree( nodep );

	return rc;
}

/***************************************************************************/
/**
*  Set destination port sampling frequency. Connections using the
*  port are also updated accordingly.
*
*  @return
*     0           On success
*     -ve         On general failure
*/
int amxrSetPortDstFreqProxy(
	AMXR_PORT_ID	portid,		/**< (i) Destination port id */
	int		dst_hz,		/**< (i) Destination sampling frequency in Hz */
	int		dst_bytes	/**< (i) Destination period size in bytes */
)
{
	struct ppxy_node *nodep = getNodep( portid );
	int rc = realloc_data_buf( &nodep->dst, dst_bytes );
	if ( rc < 0 )
	{
		return rc;
	}
	return amxrSetPortDstFreq( nodep->portid, dst_hz, dst_bytes );
}

/***************************************************************************/
/**
*  Set source port sampling frequency. Connections using the port
*  are also updated accordingly.
*
*  @return
*     0           On success
*     -ve         On general failure
*/
int amxrSetPortSrcFreqProxy(
	AMXR_PORT_ID	portid,		/**< (i) Source port id */
	int		src_hz,		/**< (i) Source sampling frequency in Hz */
	int		src_bytes	/**< (i) Source period size in bytes */
)
{
	struct ppxy_node *nodep = getNodep( portid );
	int rc = realloc_data_buf( &nodep->src, src_bytes );
	if ( rc < 0 )
	{
		return rc;
	}
	return amxrSetPortSrcFreq( nodep->portid, src_hz, src_bytes );
}

/***************************************************************************/
/**
*  Set the channel configuration for a destination port.
*  Connections using the port are updated, where appropriate.
*
*  @return
*     0           On success
*     -ve         On general failure
*
*  @remarks    Not all existing connections with this source port
*              will be maintained after changing the number of channels.
*/
int amxrSetPortDstChannelsProxy(
	AMXR_PORT_ID portid,	/**< (i) Destination port id */
	int dst_chans,		/**< (i) Number of channels: 1 for mono, 2 for stereo, etc */
	int dst_bytes		/**< (i) Destination period size in bytes */
)
{
	struct ppxy_node *nodep = getNodep( portid );
	int rc = realloc_data_buf( &nodep->dst, dst_bytes );
	if ( rc < 0 )
	{
		return rc;
	}
	return amxrSetPortDstChannels( nodep->portid, dst_chans, dst_bytes );
}

/***************************************************************************/
/**
*  Set the channel configuration for a source port.
*  Connections using the port are updated, where appropriate.
*
*  @return
*     0           On success
*     -ve         On general failure
*
*  @remarks    Not all existing connections with this source port
*              will be maintained after changing the number of channels.
*/
int amxrSetPortSrcChannelsProxy(
	AMXR_PORT_ID   portid,           /**< (i) Source port id */
	int            src_chans,        /**< (i) Number of src channels: 1 for mono, 2 for stereo, etc. */
	int            src_bytes         /**< (i) Source period size in bytes */
)
{
	struct ppxy_node *nodep = getNodep( portid );
	int rc = realloc_data_buf( &nodep->src, src_bytes );
	if ( rc < 0 )
	{
		return rc;
	}
	return amxrSetPortSrcChannels( nodep->portid, src_chans, src_bytes );
}

/***************************************************************************/
/**
*  Dequeue and free message
*
*  @return
*
*  @remarks	Assume single thread reader because each user port
*  		should only be serviced by a single thread.
*/
int getMsg(
	AMXR_PORT_ID portid,		/**< (i) Source port id */
	struct amxr_port_msg *msgp	/**< (o) Ptr to store msg */
)
{
	struct ppxy_node *nodep = getNodep( portid );
	struct amxr_port_msg *portmsgp;
	int rc;

	rc = down_interruptible( &nodep->msg_avail );
	if ( rc < 0 )
	{
		return -ERESTARTSYS;
	}

	if ( atomic_read( &nodep->msg_num ) == 0 )
	{
		/* No messages, probably signalling to quit */
		return -EPERM;
	}

	portmsgp = &nodep->msgs[nodep->msg_readi];
	nodep->msg_readi = ( nodep->msg_readi + 1 ) % NUM_MSGS;

	memcpy( msgp, portmsgp, sizeof(*msgp) );
	atomic_dec( &nodep->msg_num );

	return 0;
}

/***************************************************************************/
/**
*  Driver open method
*/
static int ppxy_open( struct inode *inode, struct file *filp )
{
	return 0;
}

/***************************************************************************/
/**
*  Driver close method
*/
static int ppxy_release( struct inode *inode, struct file *filp )
{
	int rc = 0;
	if ( filp->private_data )
	{
		rc = amxrRemovePortProxy( filp->private_data );
		filp->private_data = NULL;
	}
	return rc;
}

/***************************************************************************/
/**
*  Driver ioctl method
*/
static long ppxy_ioctl( struct file *filp, unsigned int cmd, unsigned long arg )
{
	int rc;
	union ioctl_params parm;
	unsigned int cmdnr, size;

	cmdnr = _IOC_NR( cmd );
	size = _IOC_SIZE( cmd );

	if ( size > sizeof(parm) )
	{
		return -ENOMEM;
	}

	if ( size )
	{
		if (( rc = copy_from_user( &parm, (void *)arg, size )) != 0 )
		{
			printk( KERN_ERR "%s: (%d) Failed copy_from_user rc=%d\n",
					__FUNCTION__, cmdnr, rc );
			return rc;
		}
	}

	rc = -EPERM;
	switch ( cmdnr )
	{
		case AMXR_PORT_CMD_CREATE_PORT:
		{
			AMXR_PORT_ID portid;
			struct amxr_port_ioctl_createport *parmp = &parm.createport;
			if ( filp->private_data != NULL )
			{
				printk( KERN_ERR "%s: Port already created\n",
						__FUNCTION__ );
				return -EINVAL;
			}
			rc = amxrCreatePortProxy( parmp->name, &parmp->cb,
					parmp->dst_hz, parmp->dst_chans, parmp->dst_bytes,
					parmp->src_hz, parmp->src_chans, parmp->src_bytes,
					&portid );
			if ( rc == 0 )
			{
				filp->private_data = portid;
			}
		}
		break;

		case AMXR_PORT_CMD_SET_PORT_DST_FREQ:
		case AMXR_PORT_CMD_SET_PORT_SRC_FREQ:
		{
			struct amxr_port_ioctl_setportfreq *parmp;
			if ( filp->private_data == NULL )
			{
				printk( KERN_ERR "%s: Non-existent port\n",
						__FUNCTION__ );
				return -EINVAL;
			}
			parmp = &parm.setportfreq;
			if ( cmdnr == AMXR_PORT_CMD_SET_PORT_DST_FREQ )
			{
				rc = amxrSetPortDstFreqProxy(
						filp->private_data,
						parmp->hz, parmp->bytes );
			}
			else
			{
				rc = amxrSetPortSrcFreqProxy(
						filp->private_data,
						parmp->hz, parmp->bytes );
			}
		}
		break;

		case AMXR_PORT_CMD_SET_PORT_DST_CHANS:
		case AMXR_PORT_CMD_SET_PORT_SRC_CHANS:
		{
			struct amxr_port_ioctl_setportchans *parmp;
			if ( filp->private_data == NULL )
			{
				printk( KERN_ERR "%s: Non-existent port\n",
						__FUNCTION__ );
				return -EINVAL;
			}
			parmp = &parm.setportchans;
			if ( cmdnr == AMXR_PORT_CMD_SET_PORT_DST_CHANS )
			{
				rc = amxrSetPortDstChannelsProxy(
						filp->private_data,
						parmp->chans, parmp->bytes );
			}
			else
			{
				rc = amxrSetPortSrcChannelsProxy(
						filp->private_data,
						parmp->chans, parmp->bytes );
			}
		}
		break;

		case AMXR_PORT_CMD_GET_MSG:
		{
			struct amxr_port_msg msg;
			if ( filp->private_data == NULL )
			{
				printk( KERN_ERR "%s: Non-existent port\n",
						__FUNCTION__ );
				return -EINVAL;
			}
			rc = getMsg( filp->private_data, &msg );
			if ( rc == 0 )
			{
				rc = copy_to_user( (void *)arg, &msg,
						sizeof(msg) );
			}
		}
		break;

		case AMXR_PORT_CMD_RELEASE:
		{
			struct ppxy_node *nodep;

			/* Used to release message lock to gracefully quit */
			if ( filp->private_data == NULL )
			{
				printk( KERN_ERR "%s: Non-existent port\n",
						__FUNCTION__ );
				return -EINVAL;
			}
			nodep = getNodep( filp->private_data );
			up( &nodep->msg_avail );
			rc = 0;
		}
		break;

		default:
		{
			rc = -EINVAL;
		}
	}

	return rc;
}

/***************************************************************************/
/**
*  Driver memory map method
*/
static int ppxy_mmap( struct file *filp, struct vm_area_struct *vma )
{
	int rc;
	struct ppxy_node *nodep = filp->private_data;
	unsigned long length = vma->vm_end - vma->vm_start;

	if ( !filp->private_data || ( (long)filp->private_data & ~PAGE_MASK ))
	{
		/* Port has not been created yet */
		return -EINVAL;
	}

	if ( vma->vm_pgoff == 0 )
	{
		/* Map src memory buffer */
		rc = remap_pfn_range( vma, vma->vm_start,
				virt_to_phys(nodep->src.bufp) >> PAGE_SHIFT,
				length, vma->vm_page_prot);
	}
	else
	{
		/* Map dst memory buffer */
		rc = remap_pfn_range( vma, vma->vm_start,
				virt_to_phys(nodep->dst.bufp) >> PAGE_SHIFT,
				length, vma->vm_page_prot);
	}
	return rc;
}

/***************************************************************************/
/**
*  Driver write method
*/
static ssize_t ppxy_write( struct file *filp,
		const char __user *buffer, size_t count, loff_t *ppos )
{
	struct ppxy_node *nodep = filp->private_data;
	struct ppxy_src_buf *srcbufp;
	unsigned long flags;
	int rc;

	PPXY_TRACE( "count=%i\n", count );

	if ( nodep == NULL || nodep->user_cbks.getsrc )
	{
		/* Need to have created a port or low latency method not
		 * initially configured.
		 */
		return -EPERM;
	}

	srcbufp = alloc_src_buf( nodep, count );
	if ( srcbufp == NULL )
	{
		return -ENOMEM;
	}

	rc = copy_from_user( srcbufp->bufp, buffer, count );
	if ( rc < 0 )
	{
		free_src_buf( srcbufp );
		return rc;
	}

	PPXY_TRACE( "alloc'd srcbufp=0x%x and added to list", (unsigned)srcbufp );

	spin_lock_irqsave( &nodep->srcbuf_list_lock, flags );
	list_add_tail( &srcbufp->lnode, &nodep->srcbuf_list );
	spin_unlock_irqrestore( &nodep->srcbuf_list_lock, flags );

	return count;
}

#if ENABLE_DBG_PROC
/***************************************************************************/
/**
*  Debug proc read callback
*
*  @return  Number of characters to print
*/
static int read_proc( char *buf, char **start, off_t offset,
		int count, int *eof, void *data )
{
	int len;
	struct ppxy_node *nodep;

	len = 0;
	len += sprintf( buf+len, "User proxy ports: total=%i\n", gPortList.total );

	mutex_lock( &gPortList.mutex );
	list_for_each_entry( nodep, &gPortList.list, lnode )
	{
		len += sprintf( buf+len, "src: cnxs=%i idx=%i bytes=%i\n", nodep->src.cnxs,
				nodep->src.bufidx, nodep->src.frame_bytes );
		len += sprintf( buf+len, "dst: cnxs=%i idx=%i bytes=%i\n", nodep->dst.cnxs,
				nodep->dst.bufidx, nodep->dst.frame_bytes );
		len += sprintf( buf+len, "msgs: readi=%i writei=%i num=%i alloc_errs=%i\n",
				nodep->msg_readi, nodep->msg_writei, atomic_read( &nodep->msg_num ),
				atomic_read( &nodep->msg_alloc_errs ));
	}
	mutex_unlock( &gPortList.mutex );

	*eof = 1;
	return len+1;
}

/***************************************************************************/
/**
*  Initialize debug interfaces
*/
static void __init debug_init( void )
{
	create_proc_read_entry( DBG_PROC_NAME, 0, NULL, read_proc, NULL );
}

/***************************************************************************/
/**
*  Destroy debug interfaces
*/
static void __exit debug_exit( void )
{
	remove_proc_entry( DBG_PROC_NAME, NULL );
}
#endif

/***************************************************************************/
/**
*  Module load constructor
*/
static int __init amxr_ppxy_init( void )
{
	int rc;

	INIT_LIST_HEAD( &gPortList.list );
	mutex_init( &gPortList.mutex );

	/* Allocate character device major number */
	rc = alloc_chrdev_region( &gDevno, 0, 1, "amxr_port" );
	if ( rc < 0 )
	{
		printk( KERN_ERR "%s: cannot alloc major number, rc=%i\n",
				__FUNCTION__, rc );
		goto out;
	}

	cdev_init( &gCdev, &gfops );
	gCdev.owner = THIS_MODULE;
	rc = cdev_add( &gCdev, gDevno, 1 );
	if ( rc < 0 )
	{
		printk( KERN_ERR "%s: cannot alloc chrdev, rc=%i\n",
				__FUNCTION__, rc );
		goto out_unalloc_region;
	}

	gClass = class_create( THIS_MODULE, "amxr" );
	if ( IS_ERR( gClass ))
	{
		printk( KERN_ERR "%s: Class create failed\n", __FUNCTION__ );
		rc = PTR_ERR( gClass );
		goto out_del_cdevice;
	}

	gDev = device_create( gClass, NULL, gDevno, NULL, "amxr_port" );
	if ( IS_ERR( gDev ))
	{
		printk( KERN_ERR "%s: Device create failed\n", __FUNCTION__ );
		rc = PTR_ERR( gDev );
		goto out_class_destroy;
	}

#if ENABLE_DBG_PROC
	debug_init();
#endif

	return 0;

out_class_destroy:
	class_destroy( gClass );
out_del_cdevice:
	cdev_del( &gCdev );
out_unalloc_region:
	unregister_chrdev_region( gDevno, 1 );
out:
	return rc;
}

/***************************************************************************/
/**
*  Module unload destructor
*/
static void __exit amxr_ppxy_exit( void )
{
	struct ppxy_node *nodep, *tmpnodep;

#if ENABLE_DBG_PROC
	debug_exit();
#endif

	device_destroy( gClass, gDevno );
	class_destroy( gClass );

	cdev_del( &gCdev );
	unregister_chrdev_region( gDevno, 1 );

	list_for_each_entry_safe( nodep, tmpnodep, &gPortList.list, lnode )
	{
		amxrRemovePortProxy( nodep );
	}
}

module_init( amxr_ppxy_init );
module_exit( amxr_ppxy_exit );
MODULE_DESCRIPTION( "Audio Mixer Port Proxy Driver" );
MODULE_AUTHOR( "Broadcom" );
MODULE_LICENSE( "GPL" );
