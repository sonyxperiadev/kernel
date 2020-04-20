/*
 * Copyright (C) 2010 Trusted Logic S.A.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/******************************************************************************
 *
 *  The original Work has been changed by NXP Semiconductors.
 *
 *  Copyright (C) 2013-2014 NXP Semiconductors
 *   *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ******************************************************************************/
#ifndef _PN553_H_
#define _PN553_H_
#define PN544_MAGIC 0xE9

/*
 * PN544 power control via ioctl
 * PN544_SET_PWR(0): power off
 * PN544_SET_PWR(1): power on
 * PN544_SET_PWR(2): reset and power on with firmware download enabled
 */
#define PN544_SET_PWR    _IOW(PN544_MAGIC, 0x01, long)

/*
 * SPI Request NFCC to enable p61 power, only in param
 * Only for SPI
 * level 1 = Enable power
 * level 0 = Disable power
 */
#define P61_SET_SPI_PWR    _IOW(PN544_MAGIC, 0x02, long)

/* SPI or DWP can call this ioctl to get the current
 * power state of P61
 *
*/
#define P61_GET_PWR_STATUS    _IOR(PN544_MAGIC, 0x03, long)

/* DWP side this ioctl will be called
 * level 1 = Wired access is enabled/ongoing
 * level 0 = Wired access is disalbed/stopped
*/
#define P61_SET_WIRED_ACCESS _IOW(PN544_MAGIC, 0x04, long)

/*
  NFC Init will call the ioctl to register the PID with the i2c driver
*/
#define P544_SET_NFC_SERVICE_PID _IOW(PN544_MAGIC, 0x05, long)

/*
  NFC and SPI will call the ioctl to get the i2c/spi bus access
*/
#define P544_GET_ESE_ACCESS _IOW(PN544_MAGIC, 0x06, long)
/*
  NFC and SPI will call the ioctl to update the power scheme
*/
#define P544_SET_POWER_SCHEME _IOW(PN544_MAGIC, 0x07, long)

/*
  NFC will call the ioctl to release the svdd protection
*/
#define P544_REL_SVDD_WAIT _IOW(PN544_MAGIC, 0x08, long)

/* SPI or DWP can call this ioctl to get the current
 * power state of P61
 *
*/
#define PN544_SET_DWNLD_STATUS    _IOW(PN544_MAGIC, 0x09, long)
/*
  NFC will call the ioctl to release the dwp on/off protection
*/
#define P544_REL_DWPONOFF_WAIT _IOW(PN544_MAGIC, 0x0A, long)

/*
  NFC will call the ioctl to start Secure Timer
*/

#define P544_SECURE_TIMER_SESSION _IOW(PN544_MAGIC, 0x0B, long)

#define MAX_ESE_ACCESS_TIME_OUT_MS 200 /*100 milliseconds*/

typedef enum p61_access_state{
    P61_STATE_INVALID = 0x0000,
    P61_STATE_IDLE = 0x0100, /* p61 is free to use */
    P61_STATE_WIRED = 0x0200,  /* p61 is being accessed by DWP (NFCC)*/
    P61_STATE_SPI = 0x0400, /* P61 is being accessed by SPI */
    P61_STATE_DWNLD = 0x0800, /* NFCC fw download is in progress */
    P61_STATE_SPI_PRIO = 0x1000, /*Start of p61 access by SPI on priority*/
    P61_STATE_SPI_PRIO_END = 0x2000, /*End of p61 access by SPI on priority*/
    P61_STATE_SPI_END = 0x4000,
    P61_STATE_JCP_DWNLD = 0x8000,/* JCOP downlad in progress */
    P61_STATE_SECURE_MODE = 0x100000, /* secure mode state*/
    P61_STATE_SPI_SVDD_SYNC_START = 0x0001, /*ESE_VDD Low req by SPI*/
    P61_STATE_SPI_SVDD_SYNC_END = 0x0002, /*ESE_VDD is Low by SPI*/
    P61_STATE_DWP_SVDD_SYNC_START = 0x0004, /*ESE_VDD  Low req by Nfc*/
    P61_STATE_DWP_SVDD_SYNC_END = 0x0008, /*ESE_VDD is Low by Nfc*/
    P61_STATE_SPI_FAILED = 0x0010 /*SPI open/close failed*/
}p61_access_state_t;

typedef enum chip_type_pwr_scheme{
    PN67T_PWR_SCHEME = 0x01,
    PN80T_LEGACY_PWR_SCHEME,
    PN80T_EXT_PMU_SCHEME,
}chip_pwr_scheme_t;

typedef enum {
    STATUS_FAILED = -1,
    STATUS_SUCCESS = (0x0000),
} STATUS;

typedef enum jcop_dwnld_state{
    JCP_DWNLD_IDLE = P61_STATE_JCP_DWNLD,   /* jcop dwnld is ongoing*/
    JCP_DWNLD_INIT=0x8010,                         /* jcop dwonload init state*/
    JCP_DWNLD_START=0x8020,                        /* download started */
    JCP_SPI_DWNLD_COMPLETE=0x8040,                 /* jcop download complete in spi interface*/
    JCP_DWP_DWNLD_COMPLETE=0x8080,                 /* jcop download complete */
} jcop_dwnld_state_t;

struct pn544_i2c_platform_data {
    unsigned int irq_gpio;
    unsigned int ven_gpio;
    unsigned int firm_gpio;
    unsigned int ese_pwr_gpio; /* gpio to give power to p61, only TEE should use this */
    unsigned int iso_rst_gpio; /* gpio used for ISO hard reset P73*/
};

struct hw_type_info {
    /*
     * Response of get_version_cmd will be stored in data
     * byte structure :
     * byte 0-1     : Header
     * byte 2       : Status
     * byte 3       : Hardware Version
     * byte 4       : ROM code
     * byte 5       : 0x00 constant
     * byte 6-7     : Protected data version
     * byte 8-9     : Trim data version
     * byte 10-11   : FW version
     * byte 12-13   : CRC
     * */
    char data[20];
    int len;
};
#endif
