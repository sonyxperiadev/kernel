#ifndef _CAMERA_H
#define _CAMERA_H

#define CAMERA_DEBUG

#ifdef CAMERA_DEBUG

#ifdef __KERNEL__
#define refmt(fmt) "[%s]: " fmt, __func__
#define camera_print(fmt, ...) \
	printk(KERN_ERR refmt(fmt), ##__VA_ARGS__)
#else
#define refmt(fmt) "[%s]: " fmt, __func__
#define camera_print(fmt, ...) \
	printf(refmt(fmt), ##__VA_ARGS__)
#endif

#else
#define camera_print(fmt, ...)
#endif

#define CAMERA_CMD_SIZE		512
#define CAMERA_RESP_SIZE	512

/***************************************************************
					Viewfinder setup
***************************************************************/
typedef enum {
	ROT_0 = 0,
	ROT_90,
	ROT_180,
	ROT_270,
} CAMERA_VIEWFINDER_ROTATION_T;

typedef enum {
	VIREFINDER_DISABLE = 0x0,
	VIEWFINDER_ENABLE,
} CAMERA_VIEWFINDER_STATE_T;

typedef struct {
	uint32_t                        width;
	uint32_t                        height;
	uint32_t                        x_offset;
	uint32_t                        y_offset;
	CAMERA_VIEWFINDER_ROTATION_T    rotation;
} CAMERA_VIEWFINDER_REGION_S;

typedef struct {
	CAMERA_VIEWFINDER_STATE_T   state;
	CAMERA_VIEWFINDER_REGION_S  vf_region;
} CAMERA_IOCTL_SETUP_CAMERA_T;

/***************************************************************
					Capture Setup
***************************************************************/
typedef struct {
	uint32_t                        width;
	uint32_t                        height;
	uint32_t                        quality;
	uint32_t                        max_size;
	void                           *buffer;
} CAMERA_CAPTURE_S;


/***************************************************************
					Camera IOCTLS
***************************************************************/
typedef enum
{
	CAMERA_IOCTL_VIEWFINDER_SETUP_ID = 0x1,
	CAMERA_IOCTL_TAKE_PICTURE_ID,
}CAMERA_COMMAND_ID_T;

#define CAMERA_IOCTL_VIEWFINDER_SETUP _IOWR('C', CAMERA_IOCTL_VIEWFINDER_SETUP_ID, CAMERA_IOCTL_SETUP_CAMERA_T )
#define CAMERA_IOCTL_TAKE_PICTURE     _IOWR('C', CAMERA_IOCTL_TAKE_PICTURE_ID, CAMERA_CAPTURE_S )

/***************************************************************
					Public interface
***************************************************************/
int vc_camera(char *response, int maxlen, const char *format, ...);
int vc_camera_control(uint32_t enable);
int vc_camera_take_picture(uint32_t width, uint32_t height, uint32_t max_size, void *memory);
#endif
