/*
 * Broadcom specific v4l2 extension
 * This file should replace the contents in videodev2_samsung.h
 */

#ifndef __LINUX_VIDEODEV2_BRCM_H
#define __LINUX_VIDEODEV2_BRCM_H

#define VIDIOC_THUMB_SUPPORTED		_IOR('V', BASE_VIDIOC_PRIVATE+0, int)
#define VIDIOC_THUMB_G_FMT			_IOR('V', BASE_VIDIOC_PRIVATE+1, struct v4l2_format)
#define VIDIOC_THUMB_S_FMT			_IOWR('V', BASE_VIDIOC_PRIVATE+2, struct v4l2_format)

#endif /* __LINUX_VIDEODEV2_BRCM_H */
