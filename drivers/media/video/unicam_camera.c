/*
 * V4L2 Driver for unicam/rhea camera host
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/videodev2.h>
#include <linux/vmalloc.h>
#include <linux/platform_device.h>

#include <media/v4l2-common.h>
#include <media/v4l2-dev.h>
#include <media/videobuf2-dma-contig.h>
#include <media/soc_camera.h>

struct unicam_camera_dev {
	struct soc_camera_device *icd;
	struct vb2_alloc_ctx *alloc_ctx;
	struct soc_camera_host soc_host;
};

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

static int unicam_camera_reqbufs(struct soc_camera_device *icd,
			      struct v4l2_requestbuffers *p)
{
	return 0;
}
static int unicam_camera_init_videobuf(struct vb2_queue *q,
				     struct soc_camera_device *icd)
{

	return 0;
}

static int unicam_camera_get_formats(struct soc_camera_device *icd, unsigned int idx,
				  struct soc_camera_format_xlate *xlate)
{
	return 0;
}

static int unicam_camera_try_fmt(struct soc_camera_device *icd,
			      struct v4l2_format *f)
{
	return 0;
}

static int unicam_camera_set_fmt(struct soc_camera_device *icd,
			      struct v4l2_format *f)
{
	return 0;
}
		

static int unicam_camera_set_crop(struct soc_camera_device *icd,
			       struct v4l2_crop *a)
{

	return 0;
}

static int unicam_camera_add_device(struct soc_camera_device *icd)
{
	struct soc_camera_host *ici = to_soc_camera_host(icd->dev.parent);
	struct unicam_camera_dev *uni_cam = ici->priv;

	if (uni_cam->icd)
		return -EBUSY;

	uni_cam->icd = icd;

	dev_info(icd->dev.parent, "Unicam Camera driver attached to camera %d\n",
			icd->devnum);

	return 0;
}

static void unicam_camera_remove_device(struct soc_camera_device *icd)
{
	struct soc_camera_host *ici = to_soc_camera_host(icd->dev.parent);
	struct unicam_camera_dev *uni_cam = ici->priv;
	
	BUG_ON(icd != uni_cam->icd);

	uni_cam->icd = NULL;

	dev_info(icd->dev.parent, "Unicam Camera driver detached from camera %d\n",
			icd->devnum);
}

static struct soc_camera_host_ops	unicam_soc_camera_host_ops = {
	.owner			= THIS_MODULE,
	.add			= unicam_camera_add_device,
	.remove			= unicam_camera_remove_device,
	.set_crop		= unicam_camera_set_crop,
	.set_fmt		= unicam_camera_set_fmt,
	.try_fmt		= unicam_camera_try_fmt,
	.get_formats	= unicam_camera_get_formats,
	.init_videobuf2	= unicam_camera_init_videobuf,
	.reqbufs		= unicam_camera_reqbufs,
	.poll			= unicam_camera_poll,
	.querycap		= unicam_camera_querycap,
	.set_bus_param	= unicam_camera_set_bus_param,
};

static int __devinit unicam_camera_probe(struct platform_device *pdev)
{
	struct unicam_camera_dev *uni_cam;
	struct soc_camera_host *soc_host;
	int err = 0;

	uni_cam = vzalloc(sizeof(*uni_cam));
	if (!uni_cam) {
		dev_err(&pdev->dev, "Could not allocate unicam camera object\n");
		err = -ENOMEM;
		goto ealloc;
	}

	soc_host				= &uni_cam->soc_host;
	soc_host->drv_name		= "unicam-camera";
	soc_host->ops			= &unicam_soc_camera_host_ops;
	soc_host->priv			= uni_cam;
	soc_host->v4l2_dev.dev	= &pdev->dev;
	soc_host->nr			= pdev->id;

	uni_cam->alloc_ctx = vb2_dma_contig_init_ctx(&pdev->dev);
	if (IS_ERR(uni_cam->alloc_ctx)) {
		err = PTR_ERR(uni_cam->alloc_ctx);
		goto eallocctx;
	}

	err = soc_camera_host_register(soc_host);
	if (err) 
		goto ecamhostreg;

	return 0;

ecamhostreg:
	vb2_dma_contig_cleanup_ctx(uni_cam->alloc_ctx);

eallocctx:
	vfree(uni_cam);
ealloc:
	return err;
}

static int __devexit unicam_camera_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver unicam_camera_driver = {
	.driver		= {
		.name = "unicam-camera",
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
