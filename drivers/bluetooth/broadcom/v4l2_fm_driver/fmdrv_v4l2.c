/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.


 *  Copyright (C) 2009-2014 Broadcom Corporation
 */

/************************************************************************************
 *
 *  Filename:      fmdrv_v4l2.c
 *
 *  Description:   FM Driver for Connectivity chip of Broadcom Corporation.
*  This file provides interfaces to V4L2 subsystem.
*
*  This module registers with V4L2 subsystem as Radio
*  data system interface (/dev/radio). During the registration,
*  it will expose three set of function pointers to V4L2 subsystem.
*
*    1) File operation related API (open, close, read, write, poll...etc).
*    2) Set of V4L2 IOCTL complaint API.
*
************************************************************************************/

#include "fmdrv.h"
#include "fmdrv_v4l2.h"
#include "fmdrv_main.h"
#include "fmdrv_rx.h"
#include "fm_public.h"
#include "fmdrv_config.h"
#include "../include/v4l2_target.h"
#include "../include/v4l2_logs.h"
#include <linux/ioctl.h>


/* Required to set "THIS_MODULE" during driver registration to linux system. Not required in Maguro */
#ifndef V4L2_THIS_MODULE_SUPPORT
#include <linux/export.h>
#endif

/************************************************************************************
**  Constants & Macros
************************************************************************************/
#ifndef V4L2_FM_DEBUG
#define V4L2_FM_DEBUG TRUE
#endif

/* The major device number. We can't rely on dynamic
 * registration any more, because ioctls need to know
 * it.  There is no logic behind chosing 100, it's just a random
 * number*/
#define MAJOR_NUM 100
/* Set the message of the device driver */
#define IOCTL_GET_PI_CODE _IOR(MAJOR_NUM, 0, char *)
#define IOCTL_GET_TP_CODE _IOR(MAJOR_NUM, 1, void *)
#define IOCTL_GET_PTY_CODE _IOR(MAJOR_NUM, 2, char *)
#define IOCTL_GET_TA_CODE _IOR(MAJOR_NUM, 3, char *)
#define IOCTL_GET_MS_CODE _IOR(MAJOR_NUM, 4, char *)
#define IOCTL_GET_PS_CODE _IOR(MAJOR_NUM, 5, char *)
#define IOCTL_GET_RT_MSG _IOR(MAJOR_NUM, 6, char *)
#define IOCTL_GET_CT_DATA _IOR(MAJOR_NUM, 7, char *)
#define IOCTL_GET_TMC_CHANNEL _IOR(MAJOR_NUM, 8, char *)


/*These values are set and has to be sent together.*/
/*Keep them as a set always, never try to further seperate them*/
/*These are arguments to BRCM vsc HCI command to switch the FM-I2S pins */
/*over PCM pins*/
/*I2S works in slave mode. Host side has to be master*/
unsigned char i2s_slave_on_pcm_pins[5] = {0x07, 0x19, 0x18, 0x19, 0x19};
/*I2S works in master mode. Host side has to be slave*/
unsigned char i2s_master_on_pcm_pins[5] = {0x05, 0x19, 0x18, 0x18, 0x18};

/*PCM works in slave mode. Host side has to be master*/
unsigned char bt_slave_on_pcm_pins[5] = {0x01, 0x19, 0x18, 0x19, 0x19};
/*PCM works in master mode. Host side has to be slave*/
unsigned char bt_master_on_pcm_pins[5] = {0x01, 0x19, 0x18, 0x18, 0x18 };

/* set this module parameter to enable debug info */
extern int fm_dbg_param;

/* Query control */
struct v4l2_queryctrl fmdrv_v4l2_queryctrl[] = {
    {
        .id = V4L2_CID_AUDIO_VOLUME,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .name = "Volume",
        .minimum = FM_RX_VOLUME_MIN,
        .maximum = FM_RX_VOLUME_MAX,
        .step = 1,
        .default_value = FM_DEFAULT_RX_VOLUME,
    },
    {
        .id = V4L2_CID_AUDIO_BALANCE,
        .flags = V4L2_CTRL_FLAG_DISABLED,
    },
    {
        .id = V4L2_CID_AUDIO_BASS,
        .flags = V4L2_CTRL_FLAG_DISABLED,
    },
    {
        .id = V4L2_CID_AUDIO_TREBLE,
        .flags = V4L2_CTRL_FLAG_DISABLED,
    },
    {
        .id = V4L2_CID_AUDIO_MUTE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .name = "Mute",
        .minimum = 0,
        .maximum = 2,
        .step = 1,
        .default_value = FM_MUTE_OFF,
    },
    {
        .id = V4L2_CID_AUDIO_LOUDNESS,
        .flags = V4L2_CTRL_FLAG_DISABLED,
    },
// may need private control
};


#if V4L2_FM_DEBUG
#define V4L2_FM_DRV_DBG(flag, fmt, arg...) \
        do { \
            if (fm_dbg_param & flag) \
                printk(KERN_DEBUG "(v4l2fmdrv):%s  "fmt"\n" , \
                                           __func__,## arg); \
        } while(0)
#else
#define V4L2_FM_DRV_DBG(flag, fmt, arg...)
#endif
#define V4L2_FM_DRV_ERR(fmt, arg...)  printk(KERN_ERR "(v4l2fmdrv):%s  "fmt"\n" , \
                                           __func__,## arg)



/************************************************************************************
**  Static variables
************************************************************************************/

static struct video_device *gradio_dev;
static unsigned char radio_disconnected;

static atomic_t v4l2_device_available = ATOMIC_INIT(1);

/************************************************************************************
**  Forward function declarations
************************************************************************************/

static int fm_v4l2_vidioc_s_hw_freq_seek(struct file *, void *,
                    struct v4l2_hw_freq_seek *);

/************************************************************************************
**  Functions
************************************************************************************/
/*****************************************************************************
**   V4L2 RADIO (/dev/radioX) device file operation interfaces
*****************************************************************************/

/* Read RX RDS data */
static ssize_t fm_v4l2_fops_read(struct file *file, char __user * buf,
                    size_t count, loff_t *ppos)
{
    int ret, bytes_read;
    struct fmdrv_ops *fmdev;

    fmdev = video_drvdata(file);

    if (!radio_disconnected) {
        V4L2_FM_DRV_ERR("(fmdrv): FM device is already disconnected\n");
        ret = -EIO;
        return ret;
    }

    /* Copy RDS data from the cicular buffer to userspace */
    bytes_read =
        fmc_transfer_rds_from_cbuff(fmdev, file, buf, count);
    return bytes_read;
}

/* Write RDS data. Since FM TX is not supported, return EINVAL
 */
static ssize_t fm_v4l2_fops_write(struct file *file, const char __user * buf,
                    size_t count, loff_t *ppos)
{
    return -EINVAL;
}

/* Handle Poll event for "/dev/radioX" device.*/
static unsigned int fm_v4l2_fops_poll(struct file *file,
                      struct poll_table_struct *pts)
{
    int ret;
    struct fmdrv_ops *fmdev;

    V4L2_FM_DRV_DBG(V4L2_DBG_RX, "(rds) %s", __func__ );

    fmdev = video_drvdata(file);
    /* Check if RDS data is available */
    ret = fm_rx_is_rds_data_available(fmdev, file, pts);
    if (!ret)
        return POLLIN | POLLRDNORM;
    return 0;
}

static ssize_t show_fmrx_comp_scan(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct fmdrv_ops *fmdev = dev_get_drvdata(dev);

    /* Chip doesn't support complete scan for weather band */
    if (fmdev->rx.region.fm_band == FM_BAND_WEATHER)
        return -EINVAL;

    return sprintf(buf, "%d\n", fmdev->rx.no_of_chans);
}

static ssize_t store_fmrx_comp_scan(struct device *dev,
        struct device_attribute *attr, char *buf, size_t size)
{
    int ret;
    unsigned long comp_scan;
    struct fmdrv_ops *fmdev = dev_get_drvdata(dev);

    /* Chip doesn't support complete scan for weather band */
    if (fmdev->rx.region.fm_band == FM_BAND_WEATHER)
        return -EINVAL;

    if (kstrtoul(buf, 0, &comp_scan))
        return -EINVAL;

    ret = fm_rx_seek_station(fmdev, 1, 0);// FM_CHANNEL_SPACING_200KHZ, comp_scan);
    if (ret < 0)
        V4L2_FM_DRV_ERR("RX complete scan failed - %d\n", ret);

    if (comp_scan == COMP_SCAN_READ)
        return (size_t) fmdev->rx.no_of_chans;
    else
        return size;
}

static ssize_t show_fmrx_deemphasis(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct fmdrv_ops *fmdev = dev_get_drvdata(dev);

    return sprintf(buf, "%d\n", (fmdev->rx.region.deemphasis==
                FM_RX_EMPHASIS_FILTER_50_USEC) ? 50 : 75);
}

static ssize_t store_fmrx_deemphasis(struct device *dev,
        struct device_attribute *attr, char *buf, size_t size)
{
    int ret;
    unsigned long deemph_mode;
    struct fmdrv_ops *fmdev = dev_get_drvdata(dev);

    if (kstrtoul(buf, 0, &deemph_mode))
        return -EINVAL;

    ret = fm_rx_config_deemphasis(fmdev,deemph_mode);
    if (ret < 0) {
        V4L2_FM_DRV_ERR("Failed to set De-emphasis Mode\n");
        return ret;
    }

    return size;
}

static ssize_t show_fmrx_af(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct fmdrv_ops *fmdev = dev_get_drvdata(dev);

    return sprintf(buf, "%d\n", fmdev->rx.af_mode);
}

static ssize_t store_fmrx_af(struct device *dev,
        struct device_attribute *attr, char *buf, size_t size)
{
    int ret;
    unsigned long af_mode;
    struct fmdrv_ops *fmdev = dev_get_drvdata(dev);

    if (kstrtoul(buf, 0, &af_mode))
        return -EINVAL;

    if (af_mode < 0 || af_mode > 1)
        return -EINVAL;

    ret = fm_rx_set_af_switch(fmdev, af_mode);
    if (ret < 0) {
        V4L2_FM_DRV_ERR("Failed to set AF Switch\n");
        return ret;
    }

    return size;
}

static ssize_t show_fmrx_band(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct fmdrv_ops *fmdev = dev_get_drvdata(dev);

    return sprintf(buf, "%d\n", fmdev->rx.region.fm_band);
}

static ssize_t store_fmrx_band(struct device *dev,
        struct device_attribute *attr, char *buf, size_t size)
{
    int ret;
    unsigned long fm_band;
    struct fmdrv_ops *fmdev = dev_get_drvdata(dev);
    if (kstrtoul(buf, 0, &fm_band))
        return -EINVAL;
    pr_info("store_fmrx_band In  fm_band %ld",fm_band);

    if (fm_band < FM_BAND_EUROPE || fm_band >= FM_BAND_WEATHER)
        return -EINVAL;

    ret = fm_rx_set_region(fmdev, fm_band);
    if (ret < 0) {
        V4L2_FM_DRV_ERR("Failed to set FM Band\n");
        return ret;
    }

    return size;
}

static ssize_t show_fmrx_fm_audio_pins(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct fmdrv_ops *fmdev = dev_get_drvdata(dev);

    return sprintf(buf, "%s\n", fmdev->rx.current_pins);
}

static ssize_t store_fmrx_fm_audio_pins(struct device *dev,
        struct device_attribute *attr, char *buf, size_t size)
{
    int ret = 0;
    struct fmdrv_ops *fmdev = dev_get_drvdata(dev);
    if(strncmp(buf, fmdev->rx.current_pins, 3) == 0) /*I2S or PCM*/
    {
        return size;
    }

    else if(strncmp(buf, "PCM", 3) == 0) /*use PCM pins*/
    {
        //send VSC to switch I2S to PCM pins
 #if ROUTE_FM_I2S_MASTER_TO_PCM_PINS
        V4L2_FM_DRV_DBG(V4L2_DBG_OPEN, "Routing I2S audio over PCM pins in master mode");
        ret = fmc_send_cmd(fmdev, 0, i2s_master_on_pcm_pins, 5, VSC_HCI_CMD, &fmdev->maintask_completion, NULL, NULL);
        if (ret < 0)
        {
            V4L2_FM_DRV_ERR("(fmdrv): Error setting switch I2s path to PCM pins as a master");
            return ret;
        }
#endif

#if ROUTE_FM_I2S_SLAVE_TO_PCM_PINS
    V4L2_FM_DRV_DBG(V4L2_DBG_OPEN, "Routing I2S audio over PCM pins in slave mode");
    ret = fmc_send_cmd(fmdev, 0, i2s_slave_on_pcm_pins, 5, VSC_HCI_CMD, &fmdev->maintask_completion, NULL, NULL);
    if (ret < 0)
    {
        V4L2_FM_DRV_ERR("(fmdrv): Error setting switch I2s path to PCM pins as a slave");
        return ret;
    }
#endif
    sprintf(fmdev->rx.current_pins, "%s", buf);
    return size;
    }
    else if(strncmp(buf, "I2S", 3) == 0) /*use I2S pins and release PCM pins for BT SCO*/
    {
    /*send VSC to release PCM pins*/
#if ROUTE_BT_I2S_MASTER_TO_PCM_PINS
        V4L2_FM_DRV_DBG(V4L2_DBG_OPEN, "Routing I2S audio over PCM pins in master mode");
        ret = fmc_send_cmd(fmdev, 0, bt_master_on_pcm_pins, 5, VSC_HCI_CMD, &fmdev->maintask_completion, NULL, NULL);
        if (ret < 0)
        {
            V4L2_FM_DRV_ERR("(fmdrv): Error setting switch I2s path to PCM pins as a master");
            return ret;
        }
#endif

#if ROUTE_FM_I2S_SLAVE_TO_PCM_PINS
        V4L2_FM_DRV_DBG(V4L2_DBG_OPEN, "Routing I2S audio over PCM pins in slave mode");
        ret = fmc_send_cmd(fmdev, 0, bt_slave_on_pcm_pins, 5, VSC_HCI_CMD, &fmdev->maintask_completion, NULL, NULL);
        if (ret < 0)
        {
            V4L2_FM_DRV_ERR("(fmdrv): Error setting switch I2s path to PCM pins as a slave");
            return ret;
        }
#endif
        sprintf(fmdev->rx.current_pins, "%s", buf);
        return size;
    }
    else
    {
        V4L2_FM_DRV_ERR("Wrong value: either PCM or I2S\n");
        return ret;
    }
    return size;
}


static ssize_t show_fmrx_rssi_lvl(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct fmdrv_ops *fmdev = dev_get_drvdata(dev);

    return sprintf(buf, "%d\n", fmdev->rx.curr_rssi_threshold);
}

static ssize_t store_fmrx_rssi_lvl(struct device *dev,
        struct device_attribute *attr, char *buf, size_t size)
{
    int ret;
    unsigned long rssi_lvl;
    struct fmdrv_ops *fmdev = dev_get_drvdata(dev);

    if (kstrtoul(buf, 0, &rssi_lvl))
        return -EINVAL;

    ret = fm_rx_set_rssi_threshold(fmdev, rssi_lvl);
    if (ret < 0) {
        V4L2_FM_DRV_ERR("Failed to set RSSI level\n");
        return ret;
    }

    return size;
}

static ssize_t show_fmrx_snr_lvl(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct fmdrv_ops *fmdev = dev_get_drvdata(dev);

    return sprintf(buf, "%d\n", fmdev->rx.curr_snr_threshold);
}

static ssize_t store_fmrx_snr_lvl(struct device *dev,
        struct device_attribute *attr, char *buf, size_t size)
{
    int ret;
    unsigned long snr_lvl;
    struct fmdrv_ops *fmdev = dev_get_drvdata(dev);

    if (kstrtoul(buf, 0, &snr_lvl))
        return -EINVAL;

    ret = fm_rx_set_snr_threshold(fmdev, snr_lvl);
    if (ret < 0) {
        V4L2_FM_DRV_ERR("Failed to set SNR level\n");
        return ret;
    }

    return size;
}

static ssize_t show_fmrx_channel_space(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct fmdrv_ops *fmdev = dev_get_drvdata(dev);

    return sprintf(buf, "%d\n", fmdev->rx.sch_step);
}

static ssize_t store_fmrx_channel_space(struct device *dev,
        struct device_attribute *attr, char *buf, size_t size)
{
    int ret;
    unsigned long chl_spacing,chl_step;
    struct fmdrv_ops *fmdev = dev_get_drvdata(dev);

    if (kstrtoul(buf, 0, &chl_spacing))
        return -EINVAL;
    switch( chl_spacing){
        case CHL_SPACE_ONE:
            chl_step= FM_STEP_50KHZ;
            break;
        case CHL_SPACE_TWO:
            chl_step= FM_STEP_100KHZ;
            break;
        case CHL_SPACE_FOUR:
            chl_step= FM_STEP_200KHZ;
            break;
        default:
            chl_step= FM_STEP_100KHZ;
    };
    ret = fmc_set_scan_step(fmdev, chl_spacing);
    if (ret < 0) {
        V4L2_FM_DRV_ERR("Failed to set channel spacing\n");
        return ret;
    }

    return size;
}

/* structures specific for sysfs entries
 * FM GUI app belongs to group "fmradio", these sysfs entries belongs to "root",
 * but GUI app needs both read and write permissions to these sysfs entires for
 * below features, so these entries got permission "666"
 */

/* To start FM RX complete scan*/
static struct kobj_attribute v4l2_fmrx_comp_scan =
__ATTR(fmrx_comp_scan, 0666, (void *)show_fmrx_comp_scan,
        (void *)store_fmrx_comp_scan);

/* To Set De-Emphasis filter mode */
static struct kobj_attribute v4l2_fmrx_deemph_mode =
__ATTR(fmrx_deemph_mode, 0666, (void *)show_fmrx_deemphasis,
        (void *)store_fmrx_deemphasis);

/* To Enable/Disable FM RX RDS AF feature */
static struct kobj_attribute v4l2_fmrx_rds_af =
__ATTR(fmrx_rds_af, 0666, (void *)show_fmrx_af, (void *)store_fmrx_af);

/* To switch between Japan/US bands */
static struct kobj_attribute v4l2_fmrx_band =
__ATTR(fmrx_band, 0666, (void *)show_fmrx_band, (void *)store_fmrx_band);

/* To set the desired FM reception RSSI level */
static struct kobj_attribute v4l2_fmrx_rssi_lvl =
__ATTR(fmrx_rssi_lvl, 0666, (void *) show_fmrx_rssi_lvl,
        (void *)store_fmrx_rssi_lvl);

/* To set the desired FM reception SNR level */
static struct kobj_attribute v4l2_fmrx_snr_lvl =
__ATTR(fmrx_snr_lvl, 0666, (void *) show_fmrx_snr_lvl,
        (void *)store_fmrx_snr_lvl);

/* To set the desired channel spacing */
static struct kobj_attribute v4l2_fmrx_channel_space =
__ATTR(fmrx_chl_lvl, 0666, (void *) show_fmrx_channel_space,
        (void *)store_fmrx_channel_space);

/* To switch between PCM / I2S pins*/
static struct kobj_attribute v4l2_fmrx_fm_audio_pins =
__ATTR(fmrx_fm_audio_pins, 0666, (void *)show_fmrx_fm_audio_pins, (void *)store_fmrx_fm_audio_pins);

static struct attribute *v4l2_fm_attrs[] = {
    &v4l2_fmrx_comp_scan.attr,
    &v4l2_fmrx_deemph_mode.attr,
    &v4l2_fmrx_rds_af.attr,
    &v4l2_fmrx_band.attr,
    &v4l2_fmrx_rssi_lvl.attr,
    &v4l2_fmrx_snr_lvl.attr,
    &v4l2_fmrx_channel_space.attr,
    &v4l2_fmrx_fm_audio_pins.attr,
    NULL,
};
static struct attribute_group v4l2_fm_attr_grp = {
    .attrs = v4l2_fm_attrs,
};

static int notify_topology(struct video_device *radio_dev)
{
    static const char *env[] = { "ACTION=change", NULL };

    return kobject_uevent_env(&radio_dev->dev.kobj, KOBJ_CHANGE, env);
}

/* Handle open request for "/dev/radioX" device.
 * Start with FM RX mode as default.
 */
static int fm_v4l2_fops_open(struct file *file)
{
    int ret = -EINVAL;
    unsigned char option;
    struct fmdrv_ops *fmdev = NULL;
    V4L2_FM_DRV_DBG(V4L2_DBG_OPEN, "(fmdrv): fm_v4l2_fops_open");
    /* Don't allow multiple open */
    if(!atomic_dec_and_test(&v4l2_device_available))
    {
        atomic_inc(&v4l2_device_available);
        V4L2_FM_DRV_ERR("(fmdrv): FM device is already opened\n");
        return -EBUSY;
    }

    if (radio_disconnected) {
        V4L2_FM_DRV_ERR("(fmdrv): FM device is already opened\n");
        return  -EBUSY;
    }

    fmdev = video_drvdata(file);
    /* initialize the driver */
    ret = fmc_prepare(fmdev);
    if (ret < 0) {
        V4L2_FM_DRV_ERR("(fmdrv): Unable to prepare FM CORE");
        return ret;
    }

    radio_disconnected = 1;

    ret = fmc_set_mode(fmdev, FM_MODE_RX); /* As of now, support only Rx */

#if(defined(DEF_V4L2_FM_WORLD_REGION) && DEF_V4L2_FM_WORLD_REGION == FM_REGION_NA)
    option = FM_REGION_NA | FM_RBDS_BIT;
#elif(defined(DEF_V4L2_FM_WORLD_REGION) && DEF_V4L2_FM_WORLD_REGION == FM_REGION_EUR)
    option = FM_REGION_EUR | FM_RDS_BIT;
#elif(defined(DEF_V4L2_FM_WORLD_REGION) && DEF_V4L2_FM_WORLD_REGION == FM_REGION_JP)
    option = FM_REGION_JP | FM_RDS_BIT;
#else
    option = 0;
#endif

    /* Enable FM */
    V4L2_FM_DRV_DBG(V4L2_DBG_OPEN,"(fmdrv): FM Enable INIT option : %d", option);
    ret = fmc_enable(fmdev, option);
    if (ret < 0) {
        V4L2_FM_DRV_ERR("(fmdrv): Unable to enable FM");
        return ret;
    }

    /* Set Audio mode */
    V4L2_FM_DRV_DBG(V4L2_DBG_OPEN,"(fmdrv): FM Set Audio mode option : %d", DEF_V4L2_FM_AUDIO_MODE);
    ret = fmc_set_audio_mode(fmdev, DEF_V4L2_FM_AUDIO_MODE);
    if (ret < 0) {
        V4L2_FM_DRV_ERR("(fmdrv): Error setting Audio mode during FM enable operation");
        return ret;
    }

    /* Register sysfs entries */
    ret = sysfs_create_group(&fmdev->radio_dev->dev.kobj,
            &v4l2_fm_attr_grp);
    if (ret) {
        V4L2_FM_DRV_ERR("failed to create sysfs entries");
        return ret;
    }

    /* Notify new sysfs entries */
    notify_topology(fmdev->radio_dev);

    /* Set Audio path */
    V4L2_FM_DRV_DBG(V4L2_DBG_OPEN,"(fmdrv): FM Set Audio path option : %d", DEF_V4L2_FM_AUDIO_PATH);
    ret = fm_rx_config_audio_path(fmdev, DEF_V4L2_FM_AUDIO_PATH);
    if (ret < 0) {
        V4L2_FM_DRV_ERR("(fmdrv): Error setting Audio path during FM enable operation");
        return ret;
    }

#if ROUTE_FM_I2S_MASTER_TO_PCM_PINS
    V4L2_FM_DRV_DBG(V4L2_DBG_OPEN, "Routing I2S audio over PCM pins in master mode");
    ret = fmc_send_cmd(fmdev, 0, i2s_master_on_pcm_pins, 5, VSC_HCI_CMD, &fmdev->maintask_completion, NULL, NULL);
    if (ret < 0) {
        V4L2_FM_DRV_ERR("(fmdrv): Error setting switch I2s path to PCM pins as a master");
        return ret;
    }
#endif

#if ROUTE_FM_I2S_SLAVE_TO_PCM_PINS
    V4L2_FM_DRV_DBG(V4L2_DBG_OPEN, "Routing I2S audio over PCM pins in slave mode");
    ret = fmc_send_cmd(fmdev, 0, i2s_slave_on_pcm_pins, 5, VSC_HCI_CMD, &fmdev->maintask_completion, NULL, NULL);
    if (ret < 0)
    {
        V4L2_FM_DRV_ERR("(fmdrv): Error setting switch I2s path to PCM pins as a slave");
        return ret;
    }
#endif

    return 0;
}

/* Handle close request for "/dev/radioX" device.
 */
static int fm_v4l2_fops_release(struct file *file)
{
    int ret =  -EINVAL;
    struct fmdrv_ops *fmdev;
    V4L2_FM_DRV_DBG(V4L2_DBG_CLOSE, "(fmdrv): fm_v4l2_fops_release");

    fmdev = video_drvdata(file);

    if (!radio_disconnected) {
        V4L2_FM_DRV_DBG(V4L2_DBG_CLOSE, "(fmdrv):FM dev already closed, close called again?");
        return ret;
    }
    /* First set audio path to NONE */
    ret = fm_rx_config_audio_path(fmdev, FM_AUDIO_NONE);
    if (ret < 0) {
        V4L2_FM_DRV_ERR("(fmdrv): Failed to set audio path to FM_AUDIO_NONE");
        /*ret = 0;*/
    }
#if ROUTE_FM_I2S_MASTER_TO_PCM_PINS
    V4L2_FM_DRV_DBG(V4L2_DBG_CLOSE, "Routing I2S audio over PCM pins in master mode");
    ret = fmc_send_cmd(fmdev, 0, bt_master_on_pcm_pins, 5, VSC_HCI_CMD, &fmdev->maintask_completion, NULL, NULL);
    if (ret < 0)
    {
        V4L2_FM_DRV_ERR("(fmdrv): Error setting switch I2s path to PCM pins as a master");
        return ret;
    }
#endif

#if ROUTE_FM_I2S_SLAVE_TO_PCM_PINS
    V4L2_FM_DRV_DBG(V4L2_DBG_CLOSE, "Routing I2S audio over PCM pins in slave mode");
    ret = fmc_send_cmd(fmdev, 0, bt_slave_on_pcm_pins, 5, VSC_HCI_CMD, &fmdev->maintask_completion, NULL, NULL);
    if (ret < 0) {
        V4L2_FM_DRV_ERR("(fmdrv): Error setting switch I2s path to PCM pins as a slave");
        return ret;
    }
#endif

    /* Now disable FM */
    ret = fmc_turn_fm_off(fmdev);
    if(ret < 0)
    {
        V4L2_FM_DRV_ERR("(fmdrv): Error disabling FM. Continuing to release FM core..");
        ret = 0;
    }
    sysfs_remove_group(&fmdev->radio_dev->dev.kobj, &v4l2_fm_attr_grp);

    ret = fmc_release(fmdev);
    if (ret < 0)
    {
        V4L2_FM_DRV_ERR("(fmdrv): FM CORE release failed");
        return ret;
    }
    radio_disconnected = 0;
    atomic_inc(&v4l2_device_available);

    return 0;
}

/*****************************************************************************
**   V4L2 RADIO (/dev/radioX) device IOCTL interfaces
*****************************************************************************/

/*
* Function to query the driver capabilities
*/
static int fm_v4l2_vidioc_querycap(struct file *file, void *priv,
                    struct v4l2_capability *capability)
{
    struct fmdrv_ops *fmdev;

    fmdev = video_drvdata(file);

    strlcpy(capability->driver, FM_DRV_NAME, sizeof(capability->driver));
    strlcpy(capability->card, FM_DRV_CARD_SHORT_NAME,
                                    sizeof(capability->card));
    sprintf(capability->bus_info, "UART");
    capability->version = FM_DRV_RADIO_VERSION;
    capability->capabilities = fmdev->device_info.capabilities;
    return 0;
}

/*
* Function to query the driver control params
*/
static int fm_v4l2_vidioc_queryctrl(struct file *file, void *priv,
                                        struct v4l2_queryctrl *qc)
{
    int index;
    int ret = -EINVAL;

    if (qc->id < V4L2_CID_BASE)
        return ret;

    /* Search control ID and copy its properties */
    for (index = 0; index < NO_OF_ENTRIES_IN_ARRAY(fmdrv_v4l2_queryctrl);\
            index++) {
        if (qc->id && qc->id == fmdrv_v4l2_queryctrl[index].id) {
            memcpy(qc, &(fmdrv_v4l2_queryctrl[index]), sizeof(*qc));
            ret = 0;
            break;
        }
    }
    return ret;
}

/*
* Function to get the driver control params. Called
* by user-space via IOCTL call
*/
static int fm_v4l2_vidioc_g_ctrl(struct file *file, void *priv,
                    struct v4l2_control *ctrl)
{
    int ret = -EINVAL;
    unsigned short curr_vol;
    unsigned char curr_mute_mode;
    struct fmdrv_ops *fmdev;

    fmdev = video_drvdata(file);

    switch (ctrl->id) {
        case V4L2_CID_AUDIO_MUTE:    /* get mute mode */
            ret = fm_rx_get_mute_mode(fmdev, &curr_mute_mode);
            if (ret < 0)
                return ret;
            ctrl->value = curr_mute_mode;
            break;

        case V4L2_CID_AUDIO_VOLUME:    /* get volume */
            V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdrv): V4L2_CID_AUDIO_VOLUME get");
            ret = fm_rx_get_volume(fmdev, &curr_vol);
            if (ret < 0)
                return ret;
            ctrl->value = curr_vol;
            break;

       default:
           V4L2_FM_DRV_ERR("(fmdrv): Unhandled IOCTL for get Control");
           break;
    }

    return ret;
}

/*
* Function to Set the driver control params. Called
* by user-space via IOCTL call
*/
static int fm_v4l2_vidioc_s_ctrl(struct file *file, void *priv,
                    struct v4l2_control *ctrl)
{
    int ret = -EINVAL;
    struct fmdrv_ops *fmdev;

    fmdev = video_drvdata(file);

    switch (ctrl->id) {
        case V4L2_CID_AUDIO_MUTE:    /* set mute */
            ret = fm_rx_set_mute_mode(fmdev, (unsigned char)ctrl->value);
            if (ret < 0)
                return ret;
            break;

        case V4L2_CID_AUDIO_VOLUME:    /* set volume */
            V4L2_FM_DRV_DBG(V4L2_DBG_TX,"(fmdrv): V4L2_CID_AUDIO_VOLUME set : %d", ctrl->value);
            ret = fm_rx_set_volume(fmdev, (unsigned short)ctrl->value);
            if (ret < 0)
                return ret;
            break;

        default:
            V4L2_FM_DRV_ERR("(fmdrv): Unhandled IOCTL for set Control");
            break;
    }

    return ret;
}

/*
* Function to get the driver audio params. Called
* by user-space via IOCTL call
*/
static int fm_v4l2_vidioc_g_audio(struct file *file, void *priv,
                    struct v4l2_audio *audio)
{
    memset(audio, 0, sizeof(*audio));
    audio->index = 0;
    strcpy(audio->name, "Radio");
    /* For FM Radio device, the audio capability should always return
   V4L2_AUDCAP_STEREO */
    audio->capability = V4L2_AUDCAP_STEREO;
    return 0;
}

/*
* Function to set the driver audio params. Called
* by user-space via IOCTL call
*/
static int fm_v4l2_vidioc_s_audio(struct file *file, void *priv,
                    struct v4l2_audio *audio)
{
    int ret = 0;
    if (audio->index != 0)
        ret = -EINVAL;
    return ret;
}

/* Get tuner attributes. This IOCTL call will return attributes like tuner type,
   upper/lower frequency, audio mode, RSSI value and AF channel */
static int fm_v4l2_vidioc_g_tuner(struct file *file, void *priv,
                    struct v4l2_tuner *tuner)
{
    unsigned short curr_rssi;
    unsigned int high = 0, low = 0;
    int ret = -EINVAL;
    struct fmdrv_ops *fmdev;
    unsigned char mode = 0;

    if (tuner->index != 0)
        return ret;

    fmdev = video_drvdata(file);
    strcpy(tuner->name, "FM");
    tuner->type = fmdev->device_info.type;
    /* The V4L2 specification defines all frequencies in unit of 62.5 kHz */
    ret = fm_rx_get_band_frequencies(fmdev, &low, &high);
    tuner->rangelow = (low * 100000)/625;
    tuner->rangehigh = (high * 100000)/625;

    ret = fmc_get_audio_mode(fmdev, &mode);
    tuner->audmode =  ((mode == FM_STEREO_MODE) ?
                    V4L2_TUNER_MODE_STEREO : V4L2_TUNER_MODE_MONO);
    V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdrv) tuner->audmode:%d", tuner->audmode);
    tuner->capability = fmdev->device_info.tuner_capability;
    tuner->rxsubchans = fmdev->device_info.rxsubchans;

    ret = fm_rx_read_curr_rssi_freq(fmdev, TRUE);
    curr_rssi = fmdev->rx.curr_rssi;
    /* RSSI from controller will be in range of -128 to +127.
     But V4L2 API defines the range of 0 to 65535. So convert this value
     FM rssi is cannot be 1~128dbm normally, although range is -128 to +127 */

    tuner->signal = (128-curr_rssi) * (65535 / 128);
    ret = 0;
    return ret;
}

/* Set tuner attributes. This IOCTL call will set attributes like
   upper/lower frequency, audio mode.
 */
static int fm_v4l2_vidioc_s_tuner(struct file *file, void *priv,
                    struct v4l2_tuner *tuner)
{
    int ret = -EINVAL;
    struct fmdrv_ops *fmdev;
    unsigned short high_freq, low_freq;
    unsigned short mode;
    if (tuner->index != 0)
        return ret;

    fmdev = video_drvdata(file);

    /* TODO : Figure out how to set the region based on lower/upper freq */
    /* The V4L2 specification defines all frequencies in unit of 62.5 kHz.
    Hence translate the incoming tuner band frequencies to controller
    recognized values. Set only if rangelow/rangehigh is not 0*/
    if(tuner->rangelow != 0 && tuner->rangehigh != 0)
    {
        V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdrv) rangelow:%d rangehigh:%d", tuner->rangelow, tuner->rangehigh);
        low_freq = ((tuner->rangelow) * 625)/100000;
        high_freq= ((tuner->rangehigh) * 625)/100000;
        V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdrv) low_freq:%d high_freq:%d", low_freq, high_freq);
        ret = fm_rx_set_band_frequencies(fmdev, low_freq, high_freq);
        if (ret < 0)
            return ret;
    }

    /* Map V4L2 stereo/mono macro to Broadcom controller equivalent audio mode */
    mode = (tuner->audmode == V4L2_TUNER_MODE_STEREO) ?
        FM_AUTO_MODE : FM_MONO_MODE;

    ret = fmc_set_audio_mode(fmdev, mode);
    if (ret < 0)
        return ret;
    return 0;
}

/* Get tuner or modulator radio frequency */
static int fm_v4l2_vidioc_g_frequency(struct file *file, void *priv,
                    struct v4l2_frequency *freq)
{
    int ret;
    struct fmdrv_ops *fmdev;

    fmdev = video_drvdata(file);
    ret = fmc_get_frequency(fmdev, &freq->frequency);
    /* Translate the controller frequency to V4L2 specific frequency
        (frequencies in unit of 62.5 Hz):
        x = (y * 100) * 1000/62.5  = y * 160 */
    freq->frequency = (freq->frequency * 160);
    if (ret < 0)
        return ret;
    return 0;
}

/* Set tuner or modulator radio frequency, this is tune channel */
static int fm_v4l2_vidioc_s_frequency(struct file *file, void *priv,
                    struct v4l2_frequency *freq)
{
    int ret = 0;
    struct fmdrv_ops *fmdev;
    struct v4l2_frequency fq;

    fmdev = video_drvdata(file);
    /* Translate the incoming tuner band frequencies
    (frequencies in unit of 62.5 Hz to controller
    recognized values. x = y * (62.5/1000000) * 100 = y / 160 */
    fq.frequency = (freq->frequency/160);
    ret = fmc_set_frequency(fmdev, fq.frequency);
    if (ret < 0)
        return ret;
    return 0;
}

/* Set hardware frequency seek. This is scanning radio stations. */
static int fm_v4l2_vidioc_s_hw_freq_seek(struct file *file, void *priv,
                    struct v4l2_hw_freq_seek *seek)
{
    int ret = -EINVAL;
    struct fmdrv_ops *fmdev;

    fmdev = video_drvdata(file);

    V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdrv) direction:%d wrap:%d", \
        seek->seek_upward, seek->wrap_around);

    ret = fmc_seek_station(fmdev, seek->seek_upward, seek->wrap_around);

    if (ret < 0)
        return ret;
    return 0;
}


/* This function is called whenever a process tries to
 * do an ioctl on this radio device. We get two extra
 * parameters (additional to the inode and file
 * structures, which all device functions get): the number
 * of the ioctl called and the parameter given to the
 * ioctl function.
 *
 * If the ioctl is write or read/write (meaning output
 * is returned to the calling process), the ioctl call
 * returns the output of this function.
 * Very important is that this is the Private IOCTL function to handle  RDA psrser
 * IOCTL commands. V4L2 framework also issues some IOCTLs which we have to route
 * them to video_ioctl2 function. So all the IOCTL other than Private ones are
 * passed to video_ioctl2 function.
 * And another important thing is that, it's good to return from this function as soon as
* we handle Private IOCTL so return statement is used insteadof break.
 */
long rds_info_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    /* Switch according to the ioctl called */
    long ret = 0;
/* Process if it is private IOCTL*/
    switch (cmd) {
        case IOCTL_GET_PI_CODE:
           V4L2_FM_DRV_DBG(V4L2_DBG_RX, "IOCTL_GET_PI_CODE");
           get_rds_element_value(GET_PI_CODE, (char *)arg);
           return 1;
        case IOCTL_GET_TP_CODE:
           V4L2_FM_DRV_DBG(V4L2_DBG_RX, "IOCTL_GET_TP_CODE");
           get_rds_element_value(GET_TP_CODE, (char *)arg);
           return 1;
        case IOCTL_GET_PTY_CODE:
           V4L2_FM_DRV_DBG(V4L2_DBG_RX, "IOCTL_GET_PTY_CODE");
           get_rds_element_value(GET_PTY_CODE, (char *)arg);
           return 1;
        case IOCTL_GET_TA_CODE:
           V4L2_FM_DRV_DBG(V4L2_DBG_RX, "IOCTL_GET_TA_CODE");
           get_rds_element_value(GET_TA_CODE, (char *)arg);
           return 1;
        case IOCTL_GET_MS_CODE:
           V4L2_FM_DRV_DBG(V4L2_DBG_RX, "IOCTL_GET_MS_CODE");
            get_rds_element_value(GET_MS_CODE, (char *)arg);
           return 1;
        case IOCTL_GET_PS_CODE:
           V4L2_FM_DRV_DBG(V4L2_DBG_RX, "IOCTL_GET_PS_CODE");
           get_rds_element_value(GET_PS_CODE, (char *)arg);
           return 1;
        case IOCTL_GET_RT_MSG:
           V4L2_FM_DRV_DBG(V4L2_DBG_RX, "IOCTL_GET_RT_MSG");
           get_rds_element_value(GET_RT_MSG, (char *)arg);
           return 1;
        case IOCTL_GET_CT_DATA:
           V4L2_FM_DRV_DBG(V4L2_DBG_RX, "IOCTL_GET_CT_DATA");
           get_rds_element_value(GET_CT_DATA, (char *)arg);
           return 1;
        case IOCTL_GET_TMC_CHANNEL:
           V4L2_FM_DRV_DBG(V4L2_DBG_RX, "IOCTL_GET_TMC_CHANNEL");
           get_rds_element_value(GET_TMC_CHANNEL, (char *)arg);
           return 1;
        default:
           V4L2_FM_DRV_DBG(V4L2_DBG_RX, "Invalid IOCTL");
           break;
    }
/* If anything other than private will be passed to V4L2 framework */
    ret = video_ioctl2(file, cmd, arg);
    return ret;
}

static const struct v4l2_file_operations fm_drv_fops = {
    .owner = THIS_MODULE,
    .read = fm_v4l2_fops_read,
    .write = fm_v4l2_fops_write,
    .poll = fm_v4l2_fops_poll,
/*This is the private IOCTL to handle RDS parser related IOCTLs*/
    .unlocked_ioctl = rds_info_ioctl,
    .open = fm_v4l2_fops_open,
    .release = fm_v4l2_fops_release,
};

static const struct v4l2_ioctl_ops fm_drv_ioctl_ops = {
    .vidioc_querycap = fm_v4l2_vidioc_querycap,
    .vidioc_queryctrl = fm_v4l2_vidioc_queryctrl,
    .vidioc_g_ctrl = fm_v4l2_vidioc_g_ctrl,
    .vidioc_s_ctrl = fm_v4l2_vidioc_s_ctrl,
    .vidioc_g_audio = fm_v4l2_vidioc_g_audio,
    .vidioc_s_audio = fm_v4l2_vidioc_s_audio,
    .vidioc_g_tuner = fm_v4l2_vidioc_g_tuner,
    .vidioc_s_tuner = fm_v4l2_vidioc_s_tuner,
    .vidioc_g_frequency = fm_v4l2_vidioc_g_frequency,
    .vidioc_s_frequency = fm_v4l2_vidioc_s_frequency,
    .vidioc_s_hw_freq_seek = fm_v4l2_vidioc_s_hw_freq_seek
};

/* V4L2 RADIO device parent structure */
static struct video_device fm_viddev_template = {
    .fops = &fm_drv_fops,
    .ioctl_ops = &fm_drv_ioctl_ops,
    .name = FM_DRV_NAME,
    .release = video_device_release,
    .vfl_type = VFL_TYPE_RADIO,
};

int fm_v4l2_init_video_device(struct fmdrv_ops *fmdev, int radio_nr)
{
    int ret = -ENOMEM;

    gradio_dev = NULL;
    /* Allocate new video device */
    gradio_dev = video_device_alloc();
    if (NULL == gradio_dev) {
        pr_err("(fmdrv): Can't allocate video device");
        return -ENOMEM;
    }

    /* Setup FM driver's V4L2 properties */
    memcpy(gradio_dev, &fm_viddev_template, sizeof(fm_viddev_template));

    video_set_drvdata(gradio_dev, fmdev);

    /* Register with V4L2 subsystem as RADIO device */
    if (video_register_device(gradio_dev, VFL_TYPE_RADIO, radio_nr)) {
        video_device_release(gradio_dev);
        V4L2_FM_DRV_ERR("(fmdrv): Could not register video device");
        return -EINVAL;
    }

    fmdev->radio_dev = gradio_dev;
    V4L2_FM_DRV_DBG(V4L2_DBG_INIT,"(fmdrv) registered with video device");
    ret = 0;

    return ret;
}

void *fm_v4l2_deinit_video_device(void)
{
    struct fmdrv_ops *fmdev;

    fmdev = video_get_drvdata(gradio_dev);
    /* Unregister RADIO device from V4L2 subsystem */
    video_unregister_device(gradio_dev);

    return fmdev;
}
