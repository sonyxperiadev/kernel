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
*  Filename:      fmdrv_main.h
*
*  Description:   Header for FM V4L2 driver
*
***********************************************************************************/

#ifndef _FMDRV_MAIN_H
#define _FMDRV_MAIN_H
#include <linux/module.h>
#include <net/bluetooth/bluetooth.h>
#include <net/bluetooth/hci_core.h>

/*******************************************************************************
**  Constants & Macros
*******************************************************************************/

#define FM_PKT_LOGICAL_CHAN_NUMBER  0x08

#define REG_RD       0x1
#define REG_WR       0x0
#define VSC_HCI_CMD  0x2

#define HCI_COMMAND  0x01
#define VSC_HCI_WRITE_PCM_PINS_OCF 0x0061
#define FM_2048_OP_CODE         0x0015
#define HCI_GRP_VENDOR_SPECIFIC 0x3F
#define BRCM_FM_VS_EVENT 0xff
#define BRCM_FM_VS_CMD_CMPL HCI_EV_CMD_COMPLETE
#define BRCM_VSE_SUBCODE_FM_INTERRUPT  0x08

/* FM RX registers */
#define FM_REG_RDS_SYS          0x00
#define FM_REG_FM_CTRL          0x01
#define FM_REG_RDS_CTL0         0x02

#define FM_REG_AUD_PAUS         0x04
#define FM_REG_AUD_CTL0         0x05
#define FM_REG_AUD_CTL1         0x06
#define FM_REG_SCH_CTL0         0x07
#define FM_SEARCH_SNR           0x08        /* search criteria for SNR */
#define FM_REG_SCH_TUNE         0x09
#define FM_REG_FM_FREQ          0x0a
#define FM_REG_FM_FREQ1         0x0b
#define FM_REG_AF_FREQ0         0x0c
#define FM_REG_AF_FREQ1         0x0d
#define FM_REG_CARRIER          0x0e
#define FM_REG_RSSI             0x0f
#define FM_REG_FM_RDS_MSK       0x10
#define FM_REG_FM_RDS_MSK1      0x11
#define FM_REG_FM_RDS_FLAG      0x12
#define FM_REG_FM_RDS_FLAG1     0x13
#define FM_REG_RDS_WLINE        0x14

#define FM_REG_BB_MAC0          0x16
#define FM_REG_BB_MAC1          0x17
#define FM_REG_BB_MSK0          0x18
#define FM_REG_BB_MSK1          0x19
#define FM_REG_PI_MAC0          0x1a
#define FM_REG_PI_MAC1          0x1b
#define FM_REG_PI_MSK0          0x1c
#define FM_REG_PI_MSK1          0x1d

#define FM_REG_RCV_ID           0x28
#define FM_REG_CFG              0x29
#define FM_REG_RDS_DATA         0x80
#define FM_REG_AF_FAILURE       0x90    /* AF jump failure reason code */
#define FM_REG_PCM_ROUTE        0x4d
#define FM_RES_PRESCAN_QUALITY  0xde    /* preset scan CO slope threshold */
#define FM_REG_SNR              0xdf    /* SNR reading of currently tuned channel */
#define FM_REG_VOLUME_CTRL      0xf8    /* audio control register */
#define FM_REG_BLEND_MUTE       0xf9
#define FM_SEARCH_BOUNDARY      0xfb    /* search boundary */
#define FM_SEARCH_METHOD        0xfc    /* scan mode for 2048B0 FW */
#define FM_REG_PRESET_MAX       0xfe
#define FM_REG_SCH_STEP         0xfd
#define FM_REG_PRESET_STA       0xff

/*******************************************************************************
**  Type definitions
*******************************************************************************/

/* SKB helpers */
struct fm_skb_cb {
    __u8 fm_opcode;
    struct completion *completion;
};

#define fm_cb(skb) ((struct fm_skb_cb *)(skb->cb))

/* FM Channel-8 command message format */
struct fm_cmd_msg_hdr {
    __u8 header;        /* Logical Channel-8 */
    __u16 cmd;          /* vendor specific command */
    __u8 len;           /* Number of bytes follows */
    __u8 fm_opcode;     /* FM Opcode */
    __u8 rd_wr;         /* Read/Write command */
} __attribute__ ((packed));


/* FM Channel-8 command message format */
struct fm_vsc_hci_cmd_msg_hdr {
    __u8 header;        /* Logical Channel-8 */
    __u16 cmd;          /* vendor specific command */
    __u8 len;           /* Number of bytes follows */
} __attribute__ ((packed));


#define FM_CMD_MSG_HDR_SIZE    6    /* sizeof(struct fm_cmd_msg_hdr) */
#define FM_VSC_HCI_CMD_MSG_HDR_SIZE    4



/* FM Channel-8 event messgage format */
struct fm_event_msg_hdr {
    __u8 header;        /* Logical Channel-8 */
    __u8 event_id;      /* event identification */
    __u8 len;           /* Number of bytes follows */
} __attribute__ ((packed));

#define FM_EVT_MSG_HDR_SIZE     3 /* sizeof(struct fm_event_msg_hdr) */
struct fm_cmd_complete_hdr {
    struct hci_ev_cmd_complete hci_cmd_complete;
    __u8 status;
    __u8 fm_opcode;        /* FM Opcode */
    __u8 rd_wr;
} __attribute__ ((packed));

#define FM_CMD_COMPLETE_HDR_SIZE     6 /* sizeof(struct fm_cmd_complete_hdr ) */

/* Converts little endian to big endian */
#define FM_STORE_LE16_TO_BE16(data, value)   \
    (data = ((value >> 8) | ((value & 0xFF) << 8)))
#define FM_LE16_TO_BE16(value)   (((value >> 8) | ((value & 0xFF) << 8)))

#define FM_STORE_LE32_TO_BE32(data, value)   \
    (data = (((value & 0xFF) >> 24) | ((value & 0xFF00) >> 8) \
             | ((value & 0xFF0000) << 8) | ((value & 0xFF000000) << 24)))
#define FM_LE32_TO_BE32(value)   (((value & 0xFF) >> 24) | ((value & 0xFF00) >> 8) \
             | ((value & 0xFF0000) << 8) | ((value & 0xFF000000) << 24))


/* Converts big endian to little endian */
#define FM_STORE_BE16_TO_LE16(data, value)   \
    (data = ((value & 0xFF) << 8) | ((value >> 8)))
#define FM_BE16_TO_LE16(value)   (((value & 0xFF) << 8) | ((value >> 8)))

#define FM_STORE_BE32_TO_LE32(data, value)   \
    (data = (((value & 0xFF) >> 24) | ((value & 0xFF00) >> 8) \
             | ((value & 0xFF0000) << 8) | ((value & 0xFF000000) << 24)))
#define FM_BE32_TO_LE32(value)   (((value & 0xFF000000) << 24) | ((value & 0xFF0000) << 8)  \
    | ((value & 0xFF00) >> 8) | ((value & 0xFF) >> 24))

#define STREAM_TO_UINT8(u8, p)   {u8 = (unsigned int)(*(p)); (p) += 1;}

#define FM_CHECK_SEND_CMD_STATUS(ret)  \
    if (ret < 0) {\
        return ret;\
    }

/* Seek directions */
#define FM_SEARCH_DIRECTION_DOWN    0
#define FM_SEARCH_DIRECTION_UP      1

/* Frequency scanning direction */
#define FM_SCAN_DOWN 0x00 /* bit 7 = 0 : Scan towards lower frequency */
#define FM_SCAN_UP 0x80 /* bit 7 = 1 : Scan towards upper frequency */

#define FM_SCAN_DIRECT_MASK     0xf0 /* high 4 bits for search direction */

/* Tunner modes */
#define FM_TUNER_NORMAL_SCAN_MODE       0
#define FM_TUNER_PRESET_MODE            1
#define FM_TUNER_SEEK_MODE 2
#define FM_TUNER_AF_JUMP_MODE           3

#define FM_SCAN_FULL        (FM_SCAN_UP | FM_TUNER_NORMAL_SCAN_MODE |0x02)       /* full band scan */
#define FM_FAST_SCAN        (FM_SCAN_UP | FM_TUNER_PRESET_MODE)                 /* use preset scan */
#define FM_SCAN_NONE         0xff

/* Min and Max volume */
#define FM_RX_VOLUME_MIN    0
#define FM_RX_VOLUME_MAX    255 /* Max volume 0x100 */

/*TI GUI app is setting max volume as 16383
16383/255=63*/
#define FM_RX_VOLUME_RATIO 63

#define COMP_SCAN_START         1
#define COMP_SCAN_READ          2
#define COMP_SCAN_STOP          3

/* FM RX De-emphasis filter modes */
#define FM_RX_EMPHASIS_FILTER_50_USEC   0
#define FM_RX_EMPHASIS_FILTER_75_USEC   1

/* RSSI threshold min and max */
#define FM_RX_RSSI_THRESHOLD_MIN    0   /* 0 dBuV */
#define FM_RX_RSSI_THRESHOLD_MAX    127 /* 191.1477 dBuV */

/*SNR threshold min and max*/
#define FM_RX_SNR_THRESHOLD_MIN    0
#define FM_RX_SNR_THRESHOLD_MAX    31


/* AF on/off */
#define FM_RX_RDS_AF_SWITCH_MODE_ON 1
#define FM_RX_RDS_AF_SWITCH_MODE_OFF 0

/* Band types */
#define FM_BAND_EUROPE      0
#define FM_BAND_JAPAN       1
#define FM_BAND_NA          2
#define FM_BAND_RUSSIAN     3
#define FM_BAND_CHINA       4
#define FM_BAND_ITALY       5
#define FM_BAND_WEATHER     6

/* noise floor estimation */
#define     FM_NFE_DEFAILT      93      /* default Noise floor value */
#define     FM_NFE_THRESH       0x32    /* default NFE threshold 53 db */
#define     FM_NFE_SNR_STEREO   19      /* Stereo audio mode SNR level above NFL */
#define     FM_NFE_SNR_MONO     10      /* Mono audio mode SNR level above NFL */

/* Mute modes */
#define    FM_MUTE_OFF         0
#define    FM_MUTE_ON          1
#define    FM_MUTE_ATTENUATE   2

/* RF dependent mute mode */
#define FM_RX_RF_DEPENDENT_MUTE_ON  1
#define FM_RX_RF_DEPENDENT_MUTE_OFF 0

/* SNR threshold max */
#define FM_RX_SNR_MAX      31

/* FM RDS modes */
#define FM_RDS_DISABLE    0
#define FM_RDS_ENABLE    1

/* FM RBDS modes */
#define FM_RDBS_DISABLE 0
#define FM_RDBS_ENABLE  1

/* RDS system type (RDS/RBDS) */
#define FM_RDS_SYSTEM_NONE  0
#define FM_RDS_SYSTEM_RDS   1
#define FM_RDS_SYSTEM_RBDS  2

/* Default RX mode configuration. Chip will be configured
 * with this default values after loading RX firmware.
 */
#define FM_DEFAULT_RX_VOLUME        150

/* length of RDS tuple retreived from RDS FIFO */
#define FM_RDS_TUPLE_LENGTH          3    /* 3 bytes */

/* max number of bytes to be read from RDS FIFO */
#define FM_RDS_FIFO_MAX               240
/* length of a RDS group based on RDS tuple */
#define FM_RDS_GROUP_LEN            12
/* 1st byte of a RDS tuple index */
#define FM_RDS_TUPLE_BYTE1         0
/* 2nd byte of a RDS tuple index */
#define FM_RDS_TUPLE_BYTE2         1
/* 3rd byte of a RDS tuple index */
#define FM_RDS_TUPLE_BYTE3         2

#define FM_RDS_BLOCK_SIZE             3    /* 3 bytes */

/* RDS block types */
#define FM_RDS_BLOCK_A                0x00
#define FM_RDS_BLOCK_B                0x10
#define FM_RDS_BLOCK_C                0x20
#define FM_RDS_BLOCK_Ctag            0x40
#define FM_RDS_BLOCK_D                0x30
#define FM_RDS_BLOCK_E_RBDS            0x50
#define FM_RDS_BLOCK_E                0x60

#define FM_RDS_BLOCK_INDEX_A            0
#define FM_RDS_BLOCK_INDEX_B            1
#define FM_RDS_BLOCK_INDEX_C            2
#define FM_RDS_BLOCK_INDEX_D            3
#define FM_RDS_BLOCK_INDEX_Ctag            4
#define FM_RDS_BLOCK_INDEX_UNKNOWN      0xF0

#define BRCM_RDS_BIT_0                              (0x01 << 0)
#define BRCM_RDS_BIT_1                              (0x01 << 1)
#define BRCM_RDS_BIT_2                              (0x01 << 2)
#define BRCM_RDS_BIT_3                              (0x01 << 3)
#define BRCM_RDS_BIT_4                              (0x01 << 4)
#define BRCM_RDS_BIT_5                              (0x01 << 5)
#define BRCM_RDS_BIT_6                              (0x01 << 6)
#define BRCM_RDS_BIT_7                              (0x01 << 7)

#define FM_RDS_STATUS_ERROR_MASK        0x30
#define BRCM_RDS_GRP_TYPE_MASK               0xF0 /* Group type mask */
#define BRCM_RDS_GRP_QLTY_MASK               (BRCM_RDS_BIT_2 | BRCM_RDS_BIT_3) /* Group Quality mask */

/* RDS block data quality */
enum
{
        BRCM_RDS_NO_ERR,        /* 0x00 */
        BRCM_RDS_2BIT_ERR,     /* 0x01 */
        BRCM_RDS_3BIT_ERR,     /* 0x02 */
        BRCM_RDS_UNRECOVER  /* 0x03 */
};

typedef unsigned char tBRCM_RDS_QUALITY;

/*******************************************************************************
**  Functions
*******************************************************************************/

/* Functions exported by FM common sub-module */
int fmc_prepare(struct fmdrv_ops *);
int fmc_release(struct fmdrv_ops *);

void fmc_update_region_info(struct fmdrv_ops *, unsigned char);
int fmc_send_cmd(struct fmdrv_ops *, unsigned char, void *, int,unsigned char,
            struct completion *, void *, int *);

int fmc_set_frequency(struct fmdrv_ops *, unsigned int);
int fmc_set_mode(struct fmdrv_ops *, unsigned char);
int fmc_set_region(struct fmdrv_ops *, unsigned char);
int fmc_set_audio_mode(struct fmdrv_ops *, unsigned char);
int fmc_seek_station(struct fmdrv_ops *, unsigned char, unsigned char);

int fmc_get_frequency(struct fmdrv_ops *, unsigned int *);
int fmc_get_region(struct fmdrv_ops *, unsigned char *);
int fmc_get_audio_mode(struct fmdrv_ops *fmdev, unsigned char *audio_mode);
int fmc_get_mode(struct fmdrv_ops *, unsigned char *);
int fmc_enable (struct fmdrv_ops *, unsigned char);
int fmc_turn_fm_off(struct fmdrv_ops *);
int fmc_turn_fm_on (struct fmdrv_ops *, unsigned char);
int fmc_set_scan_step(struct fmdrv_ops *, unsigned char);
int fmc_transfer_rds_from_cbuff(struct fmdrv_ops *, struct file *,
                    char __user *, size_t);
void fmc_reset_rds_cache(struct fmdrv_ops *);
void get_rds_element_value(int ioctl_num, char __user *ioctl_value);

#endif

