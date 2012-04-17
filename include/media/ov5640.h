#ifndef _MEDIA_OV5640_H
#define _MEDIA_OV5640_H

#include <media/v4l2-subdev.h>

struct ov5640_platform_data {
	int (*s_power)(struct v4l2_subdev *subdev, int on);
};

#endif
