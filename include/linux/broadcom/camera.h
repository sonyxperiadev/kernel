/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/broadcom/camera.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/*
*
*****************************************************************************
*
*  camera.h
*
*  PURPOSE:
*
*  Definitions for the camera driver.
*
*  NOTES:
*
*****************************************************************************/

#if !defined(LINUX_CAMERA_H)
#define LINUX_CAMERA_H

/* ---- Include Files ---------------------------------------------------- */

#include <linux/ioctl.h>

/* ---- Constants and Types ---------------------------------------------- */

#define BCM_CAM_MAGIC   'C'

#define CAM_CMD_FIRST               0x80

#define CAM_CMD_ENABLE              0x80
#define CAM_CMD_ENABLE_INTS         0x81
#define CAM_CMD_READ_REG            0x82
#define CAM_CMD_WRITE_REG           0x83
#define CAM_CMD_SET_FPS             0x84
#define CAM_CMD_ENABLE_LCD          0x85
#define CAM_CMD_GET_FRAME           0x86

#define CAM_CMD_LAST                0x86

typedef struct {
	unsigned char reg;
	unsigned char val;

} CAM_Reg_t;

typedef enum {
	CAM_FPS_5,		/* 5 frames per second */
	CAM_FPS_10,		/* 10 frames per second */
	CAM_FPS_15		/* 15 frames per second */
} CAM_fps_t;

typedef struct {
	unsigned short *buffer;
	unsigned int len;
} CAM_Frame_t;
sads
#define CAM_IOCTL_ENABLE        _IO(BCM_CAM_MAGIC, CAM_CMD_ENABLE)	/* arg is int */
#define CAM_IOCTL_ENABLE_INTS   _IO(BCM_CAM_MAGIC, CAM_CMD_ENABLE_INTS)	/* arg is int */
#define CAM_IOCTL_READ_REG      _IOWR(BCM_CAM_MAGIC, CAM_CMD_READ_REG, CAM_Reg_t)	/* arg is CAM_Reg_t * */
#define CAM_IOCTL_WRITE_REG     _IOW(BCM_CAM_MAGIC, CAM_CMD_WRITE_REG, CAM_Reg_t)	/* arg is CAM_Reg_t * */
#define CAM_IOCTL_SET_FPS       _IO(BCM_CAM_MAGIC, CAM_CMD_SET_FPS)	/* arg is CAM_fps_t */
#define CAM_IOCTL_ENABLE_LCD    _IO(BCM_CAM_MAGIC, CAM_CMD_ENABLE_LCD)	/* arg is int */
#define CAM_IOCTL_GET_FRAME     _IOWR(BCM_CAM_MAGIC, CAM_CMD_GET_FRAME, CAM_Frame_t)	/* arg is CAM_Frame_t * */
/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */
#ifdef __KERNEL__
/* ---- Constants and Types ---------------------------------------------- */
typedef void (*CAM_fncptr) (void *, void *);	/* callback function pointer type */

typedef enum {
	CAM_RGB565,		/* 16 bits per pixel RGB 5:6:5 */
	CAM_YUV422		/* 16 bits per pixel YCrCb 4:2:2 */
} CAM_Format_t;

typedef struct {
	CAM_fncptr frameFunc;	/* callback to run when frame is complete */
	void *frameFuncArg;	/* callback argument */
	CAM_fps_t fps;		/* frames per second */
	CAM_Format_t imageFormat;	/* image format */
	int enable;		/* enable level */
	int enableInts;		/* interrupts enable level */
	int dmaChannel;		/* DMA channel number */

} CAM_Parm_t;

/* ---- Function Prototypes ---------------------------------------------- */
void camEnableInts(int level);
void camEnable(int level);
void camGetParm(CAM_Parm_t *parmp);
void camSetParm(CAM_Parm_t *parmp);

#endif /* __KERNEL */
#endif /* LINUX_CAMERA_H */
