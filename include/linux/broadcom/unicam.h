/*******************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/
#ifndef _UNICAM_H_
#define _UNICAM_H_
#include <linux/ioctl.h>

#define UNICAM_DEV_NAME	"bcm_unicam"
#define BCM_UNICAM_MAGIC	'U'

typedef struct {
    void *ptr;    // virtual address
    unsigned int addr; //physical address
    unsigned int size;
} mem_t;	

typedef struct {
    unsigned int sensor_id;
    unsigned int enable;
} sensor_ctrl_t;	

typedef struct {
    unsigned int rx_status;
    unsigned int image_intr;
    unsigned int dropped_frames;
} cam_isr_reg_status_st_t;	

enum {
    UNICAM_CMD_WAIT_IRQ = 0x80,
    UNICAM_CMD_OPEN_CSI0,	
    UNICAM_CMD_CLOSE_CSI0,	
    UNICAM_CMD_OPEN_CSI1,	
    UNICAM_CMD_CLOSE_CSI1,
    UNICAM_CMD_CONFIG_SENSOR,
	UNICAM_CMD_RETURN_IRQ,	
    UNICAM_CMD_LAST
};

#define UNICAM_IOCTL_WAIT_IRQ        _IOR(BCM_UNICAM_MAGIC, UNICAM_CMD_WAIT_IRQ, unsigned int)
#define UNICAM_IOCTL_OPEN_CSI0       _IOR(BCM_UNICAM_MAGIC, UNICAM_CMD_OPEN_CSI0, unsigned int)
#define UNICAM_IOCTL_CLOSE_CSI0      _IOR(BCM_UNICAM_MAGIC, UNICAM_CMD_CLOSE_CSI0, unsigned int)
#define UNICAM_IOCTL_OPEN_CSI1       _IOR(BCM_UNICAM_MAGIC, UNICAM_CMD_OPEN_CSI1, unsigned int)
#define UNICAM_IOCTL_CLOSE_CSI1	     _IOR(BCM_UNICAM_MAGIC, UNICAM_CMD_CLOSE_CSI1, unsigned int)
#define UNICAM_IOCTL_CONFIG_SENSOR   _IOR(BCM_UNICAM_MAGIC, UNICAM_CMD_CONFIG_SENSOR, unsigned int)
#define UNICAM_IOCTL_RETURN_IRQ		 _IOR(BCM_UNICAM_MAGIC, UNICAM_CMD_RETURN_IRQ, unsigned int)
#endif
