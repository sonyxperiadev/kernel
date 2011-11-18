/*
 * V4L2 Driver for unicam/rhea camera host
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/platform_device.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>

#include <linux/videodev2.h>
#include <media/v4l2-common.h>
#include <media/v4l2-dev.h>
#include <media/videobuf2-dma-contig.h>
#include <media/soc_camera.h>
#include <media/soc_mediabus.h>

#define UNICAM_CAM_DRV_NAME		"unicam-camera"

enum unicam_cam_memresource {
	UNICAM_CAM_CSI0_REGS,
	UNICAM_NUM_RSRC
};

struct unicam_camera_dev {
	/* soc and vb3 rleated */
	struct soc_camera_device	*icd;
	struct vb2_alloc_ctx		*alloc_ctx;
	struct soc_camera_host		soc_host;
	/* generic driver related */
	struct resource				*res[UNICAM_NUM_RSRC];
	unsigned int				irq;
	struct device				*dev;
	void __iomem				*csi_base;
	/* data structure needed to support streaming */
	int							sequence;
	spinlock_t					lock;
	struct vb2_buffer			*active;
	struct list_head			capture;
	u8							streaming;
};

struct unicam_camera_buffer {
	struct vb2_buffer vb;
	struct list_head queue;
};


static struct unicam_camera_buffer *to_unicam_camera_vb(struct vb2_buffer *vb)
{
	return container_of(vb, struct unicam_camera_buffer, vb);
}

/* videobuf operations */

static int unicam_videobuf_setup(struct vb2_queue *vq,
		unsigned int *count, unsigned int *numplanes,
		unsigned long sizes[], void *alloc_ctxs[])
{
	struct soc_camera_device *icd = soc_camera_from_vb2q(vq);
	struct soc_camera_host *ici = to_soc_camera_host(icd->dev.parent);
	struct unicam_camera_dev *unicam_dev = (struct unicam_camera_dev *)ici->priv;
	int bytes_per_line = soc_mbus_bytes_per_line(icd->user_width,
			icd->current_fmt->host_fmt);

	if (bytes_per_line < 0)
		return bytes_per_line;

	*numplanes = 1;

	unicam_dev->sequence = 0;

	sizes[0] = bytes_per_line * icd->user_height;
	alloc_ctxs[0] = unicam_dev->alloc_ctx;

	if(!*count)
		*count = 2;

	dev_dbg(icd->dev.parent, "count=%d, size=%lu\n", *count, sizes[0]);

	return 0;
}

static int unicam_videobuf_prepare(struct vb2_buffer *vb)
{
	struct soc_camera_device *icd = soc_camera_from_vb2q(vb->vb2_queue);
	struct unicam_camera_buffer *buf;
	int bytes_per_line = soc_mbus_bytes_per_line(icd->user_width,
			icd->current_fmt->host_fmt);
	unsigned long size;

	if (bytes_per_line < 0)
		return bytes_per_line;

	buf = to_unicam_camera_vb(vb);

	dev_dbg(icd->dev.parent, "%s (vb=0x%p) 0x%p %lu\n", __func__,
			vb, vb2_plane_vaddr(vb, 0), vb2_get_plane_payload(vb, 0));

	size = icd->user_height * bytes_per_line;

	if (vb2_plane_size(vb, 0) < size) {
		dev_err(icd->dev.parent, "Buffer too small (%lu < %lu)\n",
				vb2_plane_size(vb, 0), size);
		return -ENOBUFS;
	}
	vb2_set_plane_payload(vb, 0, size);

	return 0;
}

static void unicam_videobuf_queue(struct vb2_buffer *vb)
{
	struct soc_camera_device *icd = soc_camera_from_vb2q(vb->vb2_queue);
	struct soc_camera_host *ici = to_soc_camera_host(icd->dev.parent);
	struct unicam_camera_dev *unicam_dev = ici->priv;
	struct unicam_camera_buffer *buf = to_unicam_camera_vb(vb);
	unsigned long flags;

	dev_dbg(icd->dev.parent, "%s (vb=0x%p) 0x%p %lu\n", __func__,
			vb, vb2_plane_vaddr(vb,0), vb2_get_plane_payload(vb, 0));

	spin_lock_irqsave(&unicam_dev->lock, flags);
	list_add_tail(&buf->queue, &unicam_dev->capture);

	if (!unicam_dev->active) {
		unicam_dev->active = vb;
		/* use this buffer to trigger capture */
	}
	spin_unlock_irqrestore(&unicam_dev->lock, flags);
}

static void unicam_videobuf_release(struct vb2_buffer *vb)
{
	struct soc_camera_device *icd = soc_camera_from_vb2q(vb->vb2_queue);
	struct soc_camera_host *ici = to_soc_camera_host(icd->dev.parent);
	struct unicam_camera_dev *unicam_dev = ici->priv;
	struct unicam_camera_buffer *buf = to_unicam_camera_vb(vb);
	unsigned long flags;

	spin_lock_irqsave(&unicam_dev->lock, flags);

	list_del_init(&buf->queue);

	spin_unlock_irqrestore(&unicam_dev->lock, flags);

}

static int unicam_videobuf_init(struct vb2_buffer *vb)
{
	INIT_LIST_HEAD(&to_unicam_camera_vb(vb)->queue);
	return 0;
}

int unicam_videobuf_start_streaming(struct vb2_queue *q)
{
	struct soc_camera_device *icd = soc_camera_from_vb2q(q);
	struct soc_camera_host *ici = to_soc_camera_host(icd->dev.parent);
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	struct unicam_camera_dev *unicam_dev = ici->priv;

	/*enable clocks */

	/* wait for hw to initialize */

	/* enable h/w interrupts */

	/* lane configuration */

	unicam_dev->streaming = 1;
	return 0;
}

int unicam_videobuf_stop_streaming(struct vb2_queue *q)
{
	struct soc_camera_device *icd = soc_camera_from_vb2q(q);
	struct soc_camera_host *ici = to_soc_camera_host(icd->dev.parent);
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	struct unicam_camera_dev *unicam_dev = ici->priv;

	/* disable csi2 interface */

	unicam_dev->active = NULL;

	/* Disable h/w interrupts */

	/* disable clocks */

	unicam_dev->streaming = 0;
	return 0;
}


static struct vb2_ops unicam_videobuf_ops = {
	.queue_setup		= unicam_videobuf_setup,
	.buf_prepare		= unicam_videobuf_prepare,
	.buf_queue			= unicam_videobuf_queue,
	.buf_cleanup		= unicam_videobuf_release,
	.buf_init			= unicam_videobuf_init,
	.start_streaming	= unicam_videobuf_start_streaming,
	.stop_streaming		= unicam_videobuf_stop_streaming,
	.wait_prepare	= soc_camera_unlock,
	.wait_finish	= soc_camera_lock
};

static int unicam_camera_init_videobuf(struct vb2_queue *q,
				     struct soc_camera_device *icd)
{
	q->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	q->io_modes = VB2_MMAP | VB2_USERPTR | VB2_READ;
	q->drv_priv = icd;
	q->ops = &unicam_videobuf_ops;
	q->mem_ops = &vb2_dma_contig_memops;
	q->buf_struct_size = sizeof(struct unicam_camera_buffer);
	return vb2_queue_init(q);
}

static int unicam_camera_set_bus_param(struct soc_camera_device *icd, __u32 pixfmt)
{
	return 0;
}

static int unicam_camera_querycap(struct soc_camera_host *ici,
			       struct v4l2_capability *cap)
{

	/* cap->name is set by the firendly caller:-> */
	strlcpy(cap->card, "Unicam Camera", sizeof(cap->card));
	cap->version = KERNEL_VERSION(0, 1, 0);
	cap->capabilities = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING;

	return 0;
}

static unsigned int unicam_camera_poll(struct file *file, poll_table *pt)
{
	struct soc_camera_device *icd = file->private_data;

	return vb2_poll(&icd->vb2_vidq, file, pt);
}

static int unicam_camera_try_fmt(struct soc_camera_device *icd,
			      struct v4l2_format *f)
{
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	const struct soc_camera_format_xlate *xlate;
	struct v4l2_pix_format *pix = &f->fmt.pix;
	struct v4l2_mbus_framefmt mf;
	__u32 pixfmt = pix->pixelformat;
	int ret;

	xlate = soc_camera_xlate_by_fourcc(icd, pixfmt);
	if (!xlate) {
		dev_warn(icd->dev.parent, "Format %x not found\n", pixfmt);
		return -EINVAL;
	}

	if (pix->bytesperline < 0)
		return pix->bytesperline;

	pix->sizeimage = pix->height * pix->bytesperline;

	/* limit to sensor capabilities */
	mf.width	= pix->width;
	mf.height	= pix->height;
	mf.field	= pix->field;
	mf.colorspace	= pix->colorspace;
	mf.code		= xlate->code;

	ret = v4l2_subdev_call(sd, video, try_mbus_fmt, &mf);
	if (ret < 0)
		return ret;

	pix->width	= mf.width;
	pix->height	= mf.height;
	pix->colorspace	= mf.colorspace;

	switch (mf.field) {
	case V4L2_FIELD_ANY:
	case V4L2_FIELD_NONE:
		pix->field	= V4L2_FIELD_NONE;
		break;
	default:
		dev_err(icd->dev.parent, "Field type %d unsupported.\n",
			mf.field);
		return -EINVAL;
	}

	/* what format can unicam support */
	switch (mf.code) {
	case V4L2_MBUS_FMT_YUYV8_2X8:
	case V4L2_MBUS_FMT_UYVY8_2X8:
		/* Above formats are supported */
		break;
	default:
		dev_err(icd->dev.parent, "Sensor format code %d unsupported.\n",
			mf.code);
		return -EINVAL;
	}

	return ret;
}

static int unicam_camera_set_fmt(struct soc_camera_device *icd,
			      struct v4l2_format *f)
{
	struct device *dev = icd->dev.parent;
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	struct soc_camera_host *ici = to_soc_camera_host(icd->dev.parent);
	struct unicam_dev *unicam_dev = ici->priv;
	const struct soc_camera_format_xlate *xlate = NULL;
	struct v4l2_pix_format *pix = &f->fmt.pix;
	struct v4l2_mbus_framefmt mf;
	int ret;

	xlate = soc_camera_xlate_by_fourcc(icd, pix->pixelformat);
	if (!xlate) {
		dev_warn(dev, "Format %x not found\n", pix->pixelformat);
		return -EINVAL;
	}

	mf.width	= pix->width;
	mf.height	= pix->height;
	mf.field	= pix->field;
	mf.colorspace	= pix->colorspace;
	mf.code		= xlate->code;

	ret = v4l2_subdev_call(sd, video, s_mbus_fmt, &mf);

	if (mf.code != xlate->code)
		return -EINVAL;

	if (ret < 0) {
		dev_warn(dev, "Failed to configure for format %x\n",
				pix->pixelformat);
		return ret;
	}
	/*TODO limit here any maximum size */

	pix->width = mf.width;
	pix->height = mf.height;
	pix->field = mf.field;
	pix->colorspace = mf.colorspace;
	icd->current_fmt = xlate;

	return ret;
}

static int unicam_camera_add_device(struct soc_camera_device *icd)
{
	struct soc_camera_host *ici = to_soc_camera_host(icd->dev.parent);
	struct unicam_camera_dev *unicam_dev = ici->priv;

	if (unicam_dev->icd)
		return -EBUSY;

	unicam_dev->icd = icd;

	dev_info(icd->dev.parent, "Unicam Camera driver attached to camera %d\n",
			icd->devnum);

	return 0;
}

static void unicam_camera_remove_device(struct soc_camera_device *icd)
{
	struct soc_camera_host *ici = to_soc_camera_host(icd->dev.parent);
	struct unicam_camera_dev *unicam_dev = ici->priv;
	
	BUG_ON(icd != unicam_dev->icd);

	unicam_dev->icd = NULL;

	if (unicam_dev->streaming) {
		/* stop streaming */
		/* we should call streamoff from queue operations */
		unicam_videobuf_stop_streaming(&icd->vb2_vidq);
	}


	dev_info(icd->dev.parent, "Unicam Camera driver detached from camera %d\n",
			icd->devnum);

}

static struct soc_camera_host_ops unicam_soc_camera_host_ops = {
	.owner			= THIS_MODULE,
	.add			= unicam_camera_add_device,
	.remove			= unicam_camera_remove_device,
	.set_fmt		= unicam_camera_set_fmt,
	.try_fmt		= unicam_camera_try_fmt,
	.init_videobuf2	= unicam_camera_init_videobuf,
	.poll			= unicam_camera_poll,
	.querycap		= unicam_camera_querycap,
	.set_bus_param	= unicam_camera_set_bus_param,
};

static irqreturn_t unicam_camera_isr(int irq, void *arg)
{
	struct unicam_camera_dev *unicam_dev = (struct unicam_camera_dev *)arg;
	return IRQ_HANDLED;
}

static int __devinit unicam_camera_probe(struct platform_device *pdev)
{
	struct unicam_camera_dev *unicam_dev;
	struct soc_camera_host *soc_host;
	int irq;
	int err = 0;
	int i;
	struct resource *res[UNICAM_NUM_RSRC];

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		err = -ENODEV;
		goto edev;
	}

	for (i = 0; i < UNICAM_NUM_RSRC; i++) {
		res[i] = platform_get_resource(pdev, IORESOURCE_MEM, i);
		if (!res[i]) {
			err = -ENODEV;
			goto edev;
		}

		if (!request_mem_region(res[i]->start, resource_size(res[i]),
					UNICAM_CAM_DRV_NAME)) {
			err = -EBUSY;
			goto edev;
		}
	}

	unicam_dev = vzalloc(sizeof(*unicam_dev));
	if (!unicam_dev) {
		dev_err(&pdev->dev, "Could not allocate unicam camera object\n");
		err = -ENOMEM;
		goto ealloc;
	}

	INIT_LIST_HEAD(&unicam_dev->capture);
	spin_lock_init(&unicam_dev->lock);

	for (i = 0; i < UNICAM_NUM_RSRC; i++)
		unicam_dev->res[i] = res[i];

	unicam_dev->csi_base = ioremap(res[UNICAM_CAM_CSI0_REGS]->start,
					resource_size(res[UNICAM_CAM_CSI0_REGS]));
	if (!unicam_dev->csi_base) {
		dev_err(&pdev->dev, "Unable to map csi0 regs\n");
		err = -ENOMEM;
		goto ecsioregs;
	}

	unicam_dev->irq			= irq;
	soc_host				= &unicam_dev->soc_host;
	soc_host->drv_name		= UNICAM_CAM_DRV_NAME;
	soc_host->ops			= &unicam_soc_camera_host_ops;
	soc_host->priv			= unicam_dev;
	soc_host->v4l2_dev.dev	= &pdev->dev;
	soc_host->nr			= pdev->id;

	/* register irq */
	err = request_irq(unicam_dev->irq, unicam_camera_isr, IRQF_DISABLED | IRQF_SHARED,
			UNICAM_CAM_DRV_NAME, unicam_dev);
	if (err) {
		dev_err(&pdev->dev, "cound not install irq %d\n", unicam_dev->irq);
		err = -ENODEV;
		goto eirq;
	}

	unicam_dev->alloc_ctx = vb2_dma_contig_init_ctx(&pdev->dev);
	if (IS_ERR(unicam_dev->alloc_ctx)) {
		err = PTR_ERR(unicam_dev->alloc_ctx);
		goto eallocctx;
	}

	err = soc_camera_host_register(soc_host);
	if (err) 
		goto ecamhostreg;

	return 0;

ecamhostreg:
	vb2_dma_contig_cleanup_ctx(unicam_dev->alloc_ctx);
eallocctx:
	free_irq(unicam_dev->irq, unicam_dev);
eirq:
	iounmap(unicam_dev->csi_base);
ecsioregs:
	vfree(unicam_dev);
ealloc:
	for (i = 0; i < UNICAM_NUM_RSRC; i++) {
		if(res[i])
			release_mem_region(res[i]->start, resource_size(res[i]));
	}
edev:
	return err;
}

static int __devexit unicam_camera_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver unicam_camera_driver = {
	.driver		= {
		.name = UNICAM_CAM_DRV_NAME,
	},
	.probe		= unicam_camera_probe,
	.remove		= __devexit_p(unicam_camera_remove),
};

static int __init unicam_camera_init(void)
{
	return platform_driver_register(&unicam_camera_driver);
}

static void __exit unicam_camera_exit(void)
{
	platform_driver_unregister(&unicam_camera_driver);
}

module_init(unicam_camera_init);
module_exit(unicam_camera_exit);

MODULE_DESCRIPTION("Unicam Camera Host driver");
MODULE_AUTHOR("Pradeep Sawlani <spradeep@broadcom.com>");
