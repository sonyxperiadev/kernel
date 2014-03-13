#ifndef __ARM_ARCH_DEV_HAWAII_HEAT3G_CAMERA_H
#define __ARM_ARCH_DEV_HAWAII_HEAT3G_CAMERA_H

#ifdef CONFIG_VIDEO_UNICAM_CAMERA
extern struct platform_device hawaii_camera;
extern struct platform_device hawaii_camera_sub;
#endif
void __init hawaii_cam_init(void);

#endif