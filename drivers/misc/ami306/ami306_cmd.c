/* ami306_cmd.c - AMI-Sensor driver
 *
 * Copyright (C) 2011 AICHI STEEL CORPORATION
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
/**
 * @file	ami306_cmd.c
 * @brief	AMI306 Device Driver
 */
#ifdef __KERNEL__
#include <linux/module.h>
#include "linux/ami306_hw.h"
#include "linux/ami306_def.h"
#include "linux/ami306_cmd.h"
#include "linux/ami_sensor_pif.h"
#else
#include <stdio.h>
#include <string.h>
#include "ami306_hw.h"
#include "ami306_def.h"
#include "ami306_cmd.h"
#include "ami_sensor_pif.h"
#endif

/*-------+---------+---------+---------+---------+---------+---------+---------+
 *		Driver Information.
 *-------+---------+---------+---------+---------+---------+---------+--------*/
#define	THIS_CODE_REMARKS "ami306 20121011"
#define THIS_VER_MAJOR	1
#define THIS_VER_MIDDLE	4
#define THIS_VER_MINOR	0

/*-------+---------+---------+---------+---------+---------+---------+---------+
 *		define
 *-------+---------+---------+---------+---------+---------+---------+--------*/
#define AMI_STANDBY_MODE		0xFFFF
#define AMI_NORMAL_MODE			0
#define AMI_FORCE_MODE			1

/*-------+---------+---------+---------+---------+---------+---------+---------+
 *		struct
 *-------+---------+---------+---------+---------+---------+---------+--------*/
struct ami_stat {
	void *i2c_hnd;
	s16 mode;
	struct ami_sensor_parameter prm;
	s16 lsb[3];
	s16 dir;
	s16 polarity;
	s16 si[9];
};

/*-------+---------+---------+---------+---------+---------+---------+---------+
 *		Some routines using i2c interface
 *-------+---------+---------+---------+---------+---------+---------+--------*/
/*---------------------------------------------------------------------------*/
int AMI_i2c_send_b(void *i2c_hnd, u8 cmd, u8 buf)
{
	return AMI_i2c_send(i2c_hnd, cmd, 1, &buf);
}

/*---------------------------------------------------------------------------*/
int AMI_i2c_recv_b(void *i2c_hnd, u8 cmd, u8 *buf)
{
	return AMI_i2c_recv(i2c_hnd, cmd, 1, buf);
}

/*---------------------------------------------------------------------------*/
int AMI_i2c_send_w(void *i2c_hnd, u8 cmd, u16 buf)
{
	u8 dat[2];
	dat[0] = 0xFF & buf;
	dat[1] = 0xFF & buf >> 8;
	return AMI_i2c_send(i2c_hnd, cmd, sizeof(dat), dat);
}

/*---------------------------------------------------------------------------*/
int AMI_i2c_recv_w(void *i2c_hnd, u8 cmd, u16 *buf)
{
	u8 dat[2];
	int ret = AMI_i2c_recv(i2c_hnd, cmd, sizeof(dat), dat);
	*buf = dat[1] << 8 | dat[0];
	return ret;
}

/*-------+---------+---------+---------+---------+---------+---------+---------+
 *	Some routines, bit Up or bit Dn AMI register.
 *-------+---------+---------+---------+---------+---------+---------+--------*/
static int AMI_Up_RegisterBit8(void *i2c_hnd, u8 regiaddr, u8 bit)
{
	int res = 0;
	u8 buf;

	res = AMI_i2c_recv_b(i2c_hnd, regiaddr, &buf);
	buf |= bit;
	res = AMI_i2c_send_b(i2c_hnd, regiaddr, buf);
	return (res);		/* At read error, it may happen write error. */
}

/*---------------------------------------------------------------------------*/
static int AMI_Dn_RegisterBit8(void *i2c_hnd, u8 regiaddr, u8 maskbit)
{
	int res = 0;
	u8 buf;

	res = AMI_i2c_recv_b(i2c_hnd, regiaddr, &buf);
	buf &= (u8) ~ (maskbit);
	res = AMI_i2c_send_b(i2c_hnd, regiaddr, buf);
	return (res);		/* At read error, it may happen write error. */
}

/*-------+---------+---------+---------+---------+---------+---------+---------+
 *	Some routines ami command
 *-------+---------+---------+---------+---------+---------+---------+--------*/
/* HighSpeedMode */
int AMI_HighSpeedMea(void *i2c_hnd)
{
	return AMI_i2c_send_w(i2c_hnd, AMI_REG_CTRL4, AMI_CTRL4_HS);
}

/*---------------------------------------------------------------------------*/
/* FS1 Force & PC1 Active */
int AMI_ForceModeActive(void *i2c_hnd)
{
	int res = 0;
	res = AMI_Up_RegisterBit8(i2c_hnd, AMI_REG_CTRL1,
				  AMI_CTRL1_PC1 | AMI_CTRL1_FS1_FORCE);
	AMI_udelay(8);		/* 8microsec(Turn on time 2) */
	return (res);
}

/*---------------------------------------------------------------------------*/
/* FS1 Normal & PC1 Active */
#if 0				/* not use */
static int AMI_NormalModeActive(void *i2c_hnd)
{
	int res = 0;
	res = AMI_Dn_RegisterBit8(i2c_hnd, AMI_REG_CTRL1, AMI_CTRL1_FS1_FORCE);
	res = AMI_Up_RegisterBit8(i2c_hnd, AMI_REG_CTRL1,
				  AMI_CTRL1_PC1 | AMI_CTRL1_ODR1);
	AMI_udelay(8);		/* 8microsec(Turn on time 2) */
	return (res);
}
#endif

/*---------------------------------------------------------------------------*/
/* PC1 Stand-by */
static int AMI_Standby(void *i2c_hnd)
{
	int res = 0;
	res = AMI_Dn_RegisterBit8(i2c_hnd, AMI_REG_CTRL1, AMI_CTRL1_PC1);
	AMI_udelay(30);		/* 30microsec(Turn off time 1) */
	return (res);
}

/*---------------------------------------------------------------------------*/
/* DRDY enable */
int AMI_DRDY_Enable(void *i2c_hnd)
{
	return AMI_Up_RegisterBit8(i2c_hnd, AMI_REG_CTRL2, AMI_CTRL2_DREN);
}

/*---------------------------------------------------------------------------*/
/* SoftReSeT */
int AMI_SRSTset(void *i2c_hnd)
{
	int res = 0;
	res = AMI_Up_RegisterBit8(i2c_hnd, AMI_REG_CTRL3, AMI_CTRL3_SRST_BIT);
	AMI_udelay(250);	/* 250microsec(Turn on time 1) */
	return (res);
}

/*---------------------------------------------------------------------------*/
/* FORCE=1*/
static int AMI_FORCEset(void *i2c_hnd)
{
	return AMI_Up_RegisterBit8(i2c_hnd, AMI_REG_CTRL3, AMI_CTRL3_FORCE_BIT);
}

/*---------------------------------------------------------------------------*/
/* DATA X,Y,Z read */
static int AMI_Read_RawData(void *i2c_hnd, s16 dat[3])
{
	int ret = 0;
	u8 buf[6];
	ret = AMI_i2c_recv(i2c_hnd, AMI_REG_DATAX, sizeof(buf), buf);
	dat[0] = buf[1] << 8 | buf[0];
	dat[1] = buf[3] << 8 | buf[2];
	dat[2] = buf[5] << 8 | buf[4];
	return ret;
}

/*---------------------------------------------------------------------------*/
/* Temperature */
#if 0				/* not use */
static int AMI_Read_Temperature(void *i2c_hnd, u16 *dat)
{
	return AMI_i2c_recv_w(i2c_hnd, AMI_REG_TEMP, dat);
}
#endif

/*---------------------------------------------------------------------------*/
/* read fine_output */
int AMI_FineOutput(void *i2c_hnd, u16 dat[3])
{
	int ret = 0;
	ret = AMI_i2c_recv_w(i2c_hnd, AMI_REG_FINEOUTPUT_X, &dat[0]);
	ret = AMI_i2c_recv_w(i2c_hnd, AMI_REG_FINEOUTPUT_Y, &dat[1]);
	ret = AMI_i2c_recv_w(i2c_hnd, AMI_REG_FINEOUTPUT_Z, &dat[2]);
	return ret;
}

/*---------------------------------------------------------------------------*/
/* OFF X,Y,Z write */
int AMI_Write_Offset(void *i2c_hnd, u8 fine[3])
{
	int ret = 0;
	u16 dat[3];
	dat[0] = 0x7f & fine[0];
	dat[1] = 0x7f & fine[1];
	dat[2] = 0x7f & fine[2];
	ret = AMI_i2c_send_w(i2c_hnd, AMI_REG_OFFX, dat[0]);
	ret = AMI_i2c_send_w(i2c_hnd, AMI_REG_OFFY, dat[1]);
	ret = AMI_i2c_send_w(i2c_hnd, AMI_REG_OFFZ, dat[2]);
	return ret;
}

/*---------------------------------------------------------------------------*/
/* OFF X,Y,Z read */
static int AMI_Read_Offset(void *i2c_hnd, u8 fine[3])
{
	int ret = 0;
	u16 dat[3];
	ret = AMI_i2c_recv_w(i2c_hnd, AMI_REG_OFFX, &dat[0]);
	ret = AMI_i2c_recv_w(i2c_hnd, AMI_REG_OFFY, &dat[1]);
	ret = AMI_i2c_recv_w(i2c_hnd, AMI_REG_OFFZ, &dat[2]);
	fine[0] = (u8) (dat[0] & 0x007f);
	fine[1] = (u8) (dat[1] & 0x007f);
	fine[2] = (u8) (dat[2] & 0x007f);
	return ret;
}

/*---------------------------------------------------------------------------*/
/* OFFOTP X,Y,Z read */
static int AMI_Read_OffsetOTP(void *i2c_hnd, u8 fine[3])
{
	int ret = 0;
	u16 dat[3];
	ret = AMI_i2c_recv_w(i2c_hnd, AMI_REG_OFFOTPX, &dat[0]);
	ret = AMI_i2c_recv_w(i2c_hnd, AMI_REG_OFFOTPY, &dat[1]);
	ret = AMI_i2c_recv_w(i2c_hnd, AMI_REG_OFFOTPZ, &dat[2]);
	fine[0] = (u8) (dat[0] & 0x007f);
	fine[1] = (u8) (dat[1] & 0x007f);
	fine[2] = (u8) (dat[2] & 0x007f);
	return ret;
}

/*---------------------------------------------------------------------------*/
/* SENS X,Y,Z read */
static int AMI_Read_Sense(void *i2c_hnd, u16 dat[3])
{
	int ret = 0;
	ret = AMI_i2c_recv_w(i2c_hnd, AMI_REG_SENX, &dat[0]);
	ret = AMI_i2c_recv_w(i2c_hnd, AMI_REG_SENY, &dat[1]);
	ret = AMI_i2c_recv_w(i2c_hnd, AMI_REG_SENZ, &dat[2]);
	return ret;
}

/*---------------------------------------------------------------------------*/
/* GAIN PARA X,Y,Z read */
static int AMI_Read_GainPara(void *i2c_hnd, u8 interference[6])
{
	int ret = 0;
	u16 dat[3];
	ret = AMI_i2c_recv_w(i2c_hnd, AMI_REG_GAINX, &dat[0]);
	ret = AMI_i2c_recv_w(i2c_hnd, AMI_REG_GAINY, &dat[1]);
	ret = AMI_i2c_recv_w(i2c_hnd, AMI_REG_GAINZ, &dat[2]);
	interference[0] = 0xFF & dat[0] >> 8;	/*xy */
	interference[1] = 0xFF & dat[0];	/*xz */
	interference[2] = 0xFF & dat[1] >> 8;	/*yx */
	interference[3] = 0xFF & dat[1];	/*yz */
	interference[4] = 0xFF & dat[2] >> 8;	/*zx */
	interference[5] = 0xFF & dat[2];	/*zz */
	return ret;
}

/*---------------------------------------------------------------------------*/
/* Wait DataReaDY */
static int AMI_Wait_DataReady(void *i2c_hnd, u16 secs, u16 times)
{
	s16 i;

	for (i = 0; i < times; i++) {
#ifdef USE_DRDY_PIN
		if (AMI_DRDY_Value() > 0) {
			AMI_DLOG("DRDY(DRDY PIN) ON (%d)", i);
			return 0;
		}
#else
		u8 buf = 0;
		AMI_i2c_recv_b(i2c_hnd, AMI_REG_STA1, &buf);
		if (buf & AMI_STA1_DRDY_BIT) {
			AMI_DLOG("DRDY(STA1 DRDY BIT) ON (%d)", i);
			return 0;
		}
#endif
#ifdef DELAY_MILLI_SEC
		AMI_mdelay(secs);
#else
		AMI_udelay(secs);
#endif
	}
	return AMI_ERROR;
}

/*---------------------------------------------------------------------------*/
/* read WIA */
static int AMI_WhoIam(void *i2c_hnd, u8 *dat)
{
	return AMI_i2c_recv_b(i2c_hnd, AMI_REG_WIA, dat);
}

/*---------------------------------------------------------------------------*/
/* read VERSION */
static int AMI_GetVersion(void *i2c_hnd, u16 *dat)
{
	int ret = AMI_i2c_recv_w(i2c_hnd, AMI_REG_VER, dat);
	*dat &= 0x7f;
	return ret;
}

/*---------------------------------------------------------------------------*/
/* read S/N */
static int AMI_SerialNumber(void *i2c_hnd, u16 *dat)
{
	return AMI_i2c_recv_w(i2c_hnd, AMI_REG_SN, dat);
}

/*---------------------------------------------------------------------------*/
/* read MoreInfo */
static int AMI_MoreInfo(void *i2c_hnd, u16 *dat)
{
	return AMI_i2c_recv_w(i2c_hnd, AMI_REG_INFO, dat);
}

/*-------+---------+---------+---------+---------+---------+---------+---------+
 *		Get Sensor Raw Value
 *-------+---------+---------+---------+---------+---------+---------+--------*/
#ifdef DELAY_MILLI_SEC
#define AMI_DRDY_RETRY		5	/* retry times */
#define AMI_DRDY_WAIT		1	/* m(milli) sec */
#else
#define AMI_DRDY_RETRY		50	/* retry times */
#define AMI_DRDY_WAIT		100	/* u(micro) sec */
#endif
/*---------------------------------------------------------------------------*/
static int AMI_ForceMesurement(void *i2c_hnd, s16 ver[3])
{
	int res;
	/* Write CNTL3:FORCE = 1 */
	res = AMI_FORCEset(i2c_hnd);
	if (0 > res)
		return res;

	/* Wait DRDY high */
	res = AMI_Wait_DataReady(i2c_hnd, AMI_DRDY_WAIT, AMI_DRDY_RETRY);
	if (0 > res)
		return res;

	/* READ DATA X,Y,Z */
	res = AMI_Read_RawData(i2c_hnd, ver);
	if (0 > res)
		return res;

	return 0;
}

/*---------------------------------------------------------------------------*/
int AMI_Mea(void *i2c_hnd, s16 val[3])
{
	int res = AMI_ForceMesurement(i2c_hnd, val);
	if (0 > res)
		return res;

	return res;
}

/*-------+---------+---------+---------+---------+---------+---------+---------+
 *		Read parameter
 *-------+---------+---------+---------+---------+---------+---------+--------*/
static int AMI_ReadWinParam(void *i2c_hnd, struct ami_win_parameter *win)
{
	int res = 0;
	u16 m_fineout[3];
	u8 fine[3];

	res = AMI_FineOutput(i2c_hnd, m_fineout);	/*fine output */
	if (0 > res)
		return res;

	win->fine_output[0] = m_fineout[0];
	win->fine_output[1] = m_fineout[1];
	win->fine_output[2] = m_fineout[2];

	res = AMI_Read_Offset(i2c_hnd, fine);
	if (0 > res)
		return res;

	win->fine[0] = fine[0];
	win->fine[1] = fine[1];
	win->fine[2] = fine[2];

	res = AMI_Read_OffsetOTP(i2c_hnd, fine);	/* OFFOTP */
	if (0 > res)
		return res;

	win->zero_gauss_fine[0] = fine[0];
	win->zero_gauss_fine[1] = fine[1];
	win->zero_gauss_fine[2] = fine[2];

	return res;
}

/*---------------------------------------------------------------------------*/
static int AMI_ReadMagParam(void *i2c_hnd, struct ami_mag_parameter *prm)
{
	int res = 0;
	u16 sensitivity[3];

	res = AMI_Read_Sense(i2c_hnd, sensitivity);
	if (0 > res)
		return res;

	prm->sensitivity[0] = sensitivity[0];
	prm->sensitivity[1] = sensitivity[1];
	prm->sensitivity[2] = sensitivity[2];

	res = AMI_Read_GainPara(i2c_hnd, prm->interference);
	if (0 > res)
		return res;

	return res;
}

/*---------------------------------------------------------------------------*/
static int AMI_ChipInformation(void *i2c_hnd, struct ami_chipinfo *chip)
{
	int res;
	res = AMI_WhoIam(i2c_hnd, &chip->wia);
	if (res < 0)
		return res;

	res = AMI_MoreInfo(i2c_hnd, &chip->info);
	if (res < 0)
		return res;

	res = AMI_GetVersion(i2c_hnd, &chip->ver);
	if (res < 0)
		return res;

	res = AMI_SerialNumber(i2c_hnd, &chip->sn);
	if (res < 0)
		return res;

	return 0;
}

/*-------+---------+---------+---------+---------+---------+---------+---------+
 *		SearchOffset
 *-------+---------+---------+---------+---------+---------+---------+--------*/
#define AMI_FINE_MIN	1
#define AMI_FINE_MAX	95

#define SEH_RANGE_MIN	(-1948)
#define SEH_RANGE_MAX	(1902)
#define SEH_RANGE	(SEH_RANGE_MAX - SEH_RANGE_MIN)

#define AMI_WIN_MAX	1800
#define AMI_WIN_MIN	-1800

#define SEARCH_COUNT	3

/*---------------------------------------------------------------------------*/
static int AMI_CalcNewOffset(u8 *fine, s16 val, u8 win_range_fine,
			     s16 fine_output)
{
	int res = 0;		/* continue */
	int new_fine = *fine;
	/* At the case of less low limit. */
	if (val < SEH_RANGE_MIN) {
		new_fine -= win_range_fine;
		AMI_DLOG("min : fine=%d diff=%d", new_fine, win_range_fine);
	}
	/* At the case of over high limit. */
	if (val > SEH_RANGE_MAX) {
		new_fine += win_range_fine;
		AMI_DLOG("max : fine=%d diff=%d", new_fine, win_range_fine);
	}
	/* In the current window. */
	if (SEH_RANGE_MIN <= val && val <= SEH_RANGE_MAX) {
		s16 quo = val / fine_output;
		s16 rem = val % fine_output;
		rem = fine_output < 2 ? 0 : rem / (fine_output / 2);
		new_fine += (quo + rem);
		AMI_DLOG("mid : fine=%d (%d,%d)", new_fine, quo, rem);
		res = 1;	/* search end */
	}

	if (new_fine < AMI_FINE_MIN)
		*fine = AMI_FINE_MIN;
	else if (new_fine > AMI_FINE_MAX)
		*fine = AMI_FINE_MAX;
	else
		*fine = (u8)new_fine;

	return res;
}

int AMI_SearchOffsetProc(void *i2c_hnd, u8 fine[3])
{
	int res = AMI_ERROR;
	s16 axis;
	u8 run_flg = 0;
	u8 win_range_fine[3];
	s16 fine_output[3];
	s16 val[3];
	u16 cnt = 0;

	AMI_DLOG("--- Search Offset ---");

	res = AMI_FineOutput(i2c_hnd, (u16 *) fine_output);
	if (0 > res) {
		AMI_LOG("AMI_FineOutput ERROR(%d)", res);
		return res;
	}
	AMI_DLOG("fine_output (%d, %d, %d)",
		 fine_output[0], fine_output[1], fine_output[2]);

	for (axis = 0; axis < 3; ++axis) {
		if (fine_output[axis] == 0) {
			AMI_LOG("fine_output=0 axis:%d ", axis);
			return AMI_SYSTEM_ERR;
		}
		/* fines per a window */
		win_range_fine[axis] = SEH_RANGE / fine_output[axis];
	}
	AMI_DLOG("win_range_fine (%d, %d, %d)",
		 win_range_fine[0], win_range_fine[1], win_range_fine[2]);

	/* get current fine */
	res = AMI_Read_Offset(i2c_hnd, fine);
	if (0 > res) {
		AMI_LOG("AMI_Read_Offset ERROR(%d)", res);
		return res;
	}

	while (run_flg != 0x7) {

		if (cnt > SEARCH_COUNT) {
			AMI_LOG("Search Offset Count Over");
			return 0;
		}
		cnt++;

		res = AMI_Mea(i2c_hnd, val);
		if (0 > res) {
			AMI_LOG("AMI_Mea ERROR(%d)", res);
			return res;
		}
		AMI_DLOG("fine (%d, %d, %d)", fine[0], fine[1], fine[2]);
		AMI_DLOG("val  (%d, %d, %d)", val[0], val[1], val[2]);

		/* X-axis is reversed */
		val[0] *= -1;

		for (axis = 0; axis < 3; ++axis) {

			res = AMI_CalcNewOffset(&fine[axis], val[axis],
						win_range_fine[axis],
						fine_output[axis]);
			if (res == 1)
				run_flg |= (0x1 << axis);
		}
		/* set current fine */
		res = AMI_Write_Offset(i2c_hnd, fine);
		if (0 > res) {
			AMI_LOG("AMI_Write_Offset ERROR(%d)", res);
			return res;
		}
	}
	AMI_DLOG("-- fine (%d, %d, %d) --", fine[0], fine[1], fine[2]);
	return 0;
}

/*---------------------------------------------------------------------------*/
static int AMI_CheckWindow(s16 val[3])
{
	s16 check = 0;
	if (val[0] > AMI_WIN_MAX)
		check |= 0x1;
	if (val[0] < AMI_WIN_MIN)
		check |= 0x2;
	if (val[1] > AMI_WIN_MAX)
		check |= 0x4;
	if (val[1] < AMI_WIN_MIN)
		check |= 0x8;
	if (val[2] > AMI_WIN_MAX)
		check |= 0x10;
	if (val[2] < AMI_WIN_MIN)
		check |= 0x20;
	return check;
}

/*-------+---------+---------+---------+---------+---------+---------+---------+
 *	convert magnetic sensor value
 *-------+---------+---------+---------+---------+---------+---------+--------*/
/**
 * magnetic field value conversion from sensor-axis to device-axis
 *
 * @param dir		direction
 * @param polarity	polarity
 * @param mag		magnetic value
 */
static void AMI_Sen2Dev(s16 dir, s16 polarity, s16 mag[3])
{
	s16 x, y, z;
	s16 work[3];

	work[0] = mag[0];
	work[1] = mag[1];
	work[2] = mag[2];
	/* sensor-axis --> device-axis convert */
	x = (dir & 0x30) >> 4;
	y = (dir & 0x0C) >> 2;
	z = dir & 0x03;
	/* polarity convert and axis swap */
	mag[0] = polarity & 0x4 ? work[x] : -work[x];
	mag[1] = polarity & 0x2 ? work[y] : -work[y];
	mag[2] = polarity & 0x1 ? work[z] : -work[z];
}

/**
 * magnetic field (x,y,z) interference calculation
 *
 * @param inter		interference
 * @param sensitivity	magnetic sensor sensitivity
 * @param mag		magnetic value(mGauss)
 */
static void AMIL_MagInterference(u8 inter[6], s16 sens[3], s16 mag[3])
{
	s32 wk[3];
	/* sens compensate */
	wk[0] = mag[0]
	      - (mag[1] * 100 / sens[1]) * (inter[2] - 127) * sens[0] / 100000
	      - (mag[2] * 100 / sens[2]) * (inter[4] - 127) * sens[0] / 100000;
	wk[1] = mag[1]
	      - (mag[0] * 100 / sens[0]) * (inter[0] - 127) * sens[1] / 100000
	      - (mag[2] * 100 / sens[2]) * (inter[5] - 127) * sens[1] / 100000;
	wk[2] = mag[2]
	      - (mag[0] * 100 / sens[0]) * (inter[1] - 127) * sens[2] / 100000
	      - (mag[1] * 100 / sens[1]) * (inter[3] - 127) * sens[2] / 100000;

	mag[0] = wk[0];
	mag[1] = wk[1];
	mag[2] = wk[2];
}

/**
 * Soft Iron
 *
 * @param si	soft iron parameter
 * @param mag		magnetic value(mGauss)
 */
static void AMIL_SoftIron(s16 si[9], s16 mag[3])
{
	s32 wk[3];

	wk[0] = mag[0] * si[0] / 1000 + mag[1] * si[3] / 1000
	      + mag[2] * si[6] / 1000;
	wk[1] = mag[0] * si[1] / 1000 + mag[1] * si[4] / 1000
	      + mag[2] * si[7] / 1000;
	wk[2] = mag[0] * si[2] / 1000 + mag[1] * si[5] / 1000
	      + mag[2] * si[8] / 1000;

	mag[0] = (s16)wk[0];
	mag[1] = (s16)wk[1];
	mag[2] = (s16)wk[2];
}

/**
 * magnetic field (x,y,z) calculation
 *
 * @param val		G2-Sensor measurement value (voltage ADC value )
 * @param prm		calibration parameter
 * @param mag		magnetic value(mGauss)
 */
static void AMI_ConvertMag(s16 val[3], struct ami_sensor_parameter *prm,
			   s16 si[9], s16 mag[3])
{
	s16 sens[3];
	s32 v[3];

	v[0] = val[0] - (prm->win.fine_output[0] *
	       (prm->win.fine[0] - prm->win.zero_gauss_fine[0]));
	v[1] = val[1] + (prm->win.fine_output[1] *
	       (prm->win.fine[1] - prm->win.zero_gauss_fine[1]));
	v[2] = val[2] + (prm->win.fine_output[2] *
	       (prm->win.fine[2] - prm->win.zero_gauss_fine[2]));

#if 0
printf("convert %d,%d,%d  %d,%d,%d  %d,%d,%d  %d,%d,%d  %d,%d,%d  %d,%d,%d\r",
v[0], v[1], v[2],
val[0], val[1], val[2],
prm->win.fine[0], prm->win.fine[1], prm->win.fine[2],
prm->win.zero_gauss_fine[0], prm->win.zero_gauss_fine[1],
prm->win.zero_gauss_fine[2],
prm->win.fine_output[0], prm->win.fine_output[1], prm->win.fine_output[2],
prm->mag.sensitivity[0], prm->mag.sensitivity[1], prm->mag.sensitivity[2]);
#endif

	sens[0] = prm->mag.sensitivity[0] == 0 ? 600 : prm->mag.sensitivity[0];
	sens[1] = prm->mag.sensitivity[1] == 0 ? 600 : prm->mag.sensitivity[1];
	sens[2] = prm->mag.sensitivity[2] == 0 ? 600 : prm->mag.sensitivity[2];

	mag[0] = (s16)(v[0] * 1000 / sens[0]);
	mag[1] = (s16)(v[1] * 1000 / sens[1]);
	mag[2] = (s16)(v[2] * 1000 / sens[2]);

	AMIL_MagInterference(prm->mag.interference, sens, mag);

	AMIL_SoftIron(si, mag);
}

/*-------+---------+---------+---------+---------+---------+---------+---------+
 *	Device Driver APIs
 *-------+---------+---------+---------+---------+---------+---------+--------*/
#ifdef USER_MEMORY
/**
 * Device Driver work memory size
 *
 * @return work memory size
 *
 */
int AMI_GetMemSize(void)
{
	AMI_LOG("AMI_GetMemSize");
	return sizeof(struct ami_stat);
}

/**
 * Initialize Device Driver
 *
 * @param mem		Work memory(memory size returned by AMI_GetMemSize())
 * @param comm_handle	Communication handle
 * @return handle
 */
void *AMI_InitDriver(void *mem, void *comm_handle,
		int ami_dir, int ami_polarity)
{
	int res = 0;
	struct ami_stat *stat = mem;
	memset(stat, 0, sizeof(struct ami_stat));

	stat->i2c_hnd = comm_handle;
	stat->mode = AMI_STANDBY_MODE;
	stat->dir = ami_dir;
	stat->polarity = ami_polarity;
	stat->si[0] = stat->si[4] = stat->si[8] = 1000;

	res = AMI_ReadParameter(stat, &stat->prm);
	if (res < 0)
		return NULL;

	AMI_LOG("AMI_InitDriver");
	return mem;
}
#else
/**
 * Initialize Device Driver
 *
 * @param comm_handle	Communication handle
 * @return handle
 */
void *AMI_InitDriver(void *comm_handle, int ami_dir, int ami_polarity)
{
	int res = 0;
	static struct ami_stat stat;
	memset(&stat, 0, sizeof(stat));

	stat.i2c_hnd = comm_handle;
	stat.mode = AMI_STANDBY_MODE;
	stat.dir = ami_dir;
	stat.polarity = ami_polarity;
	stat.si[0] = stat.si[4] = stat.si[8] = 1000;

	res = AMI_ReadParameter(&stat, &stat.prm);
	if (res < 0)
		return NULL;

	AMI_LOG("AMI_InitDriver");
	return &stat;
}
#endif

/**
 * Start Sensor
 *
 * @param handle	handle(returned by AMI_InitDriver())
 * @return result
 */
int AMI_StartSensor(void *handle)
{
	int res = 0;
	struct ami_stat *stat = handle;

	/* Step 1 */
	res = AMI_ForceModeActive(stat->i2c_hnd);
	if (0 > res) {
		AMI_LOG("AMI_ForceModeActive ERROR(%d)", res);
		return res;
	}
	/* Step 2 */
	res = AMI_DRDY_Enable(stat->i2c_hnd);
	if (0 > res) {
		AMI_LOG("AMI_DRDY_Enable ERROR(%d)", res);
		return res;
	}
	/* Step 3 */
	res = AMI_HighSpeedMea(stat->i2c_hnd);
	if (0 > res) {
		AMI_LOG("AMI_HighSpeedMea ERROR(%d)", res);
		return res;
	}

	stat->mode = AMI_FORCE_MODE;
	AMI_LOG("AMI_StartSensor");

	return 0;
}

/**
 * Stop Sensor
 *
 * @param handle	handle(returned by AMI_InitDriver())
 * @return result
 */
int AMI_StopSensor(void *handle)
{
	struct ami_stat *stat = handle;
	int res = AMI_Standby(stat->i2c_hnd);
	if (0 > res) {
		AMI_LOG("AMI_Standby ERROR(%d)", res);
		return res;
	}

	stat->mode = AMI_STANDBY_MODE;
	AMI_LOG("AMI_StopSensor");

	return 0;
}

/**
 * Get Sensor Value
 *
 * @param handle	handle(returned by AMI_InitDriver())
 * @param val		Sensor Value
 * @return result
 */
int AMI_GetValue(void *handle, struct ami_sensor_value *val)
{
	int res = 0;
	int win_flg = 0;
	s16 lsb[3];
	struct ami_stat *stat = handle;

	if (stat->mode != AMI_FORCE_MODE) {
		AMI_LOG("mode != AMI_FORCE_MODE");
		return AMI_SEQ_ERR;
	}

	res = AMI_Mea(stat->i2c_hnd, lsb);
	if (0 > res) {
		AMI_LOG("AMI_Mea ERROR(%d)", res);
		return res;
	}

	win_flg = AMI_CheckWindow(lsb);
	if (win_flg != 0) {
		res = AMI_SearchOffset(handle);
		if (0 > res)
			return res;

		res = AMI_Mea(stat->i2c_hnd, lsb);
		if (0 > res)
			return res;
	}

	/* convert LSB to mGauss */
	AMI_ConvertMag(lsb, &stat->prm, stat->si, val->mag);

	memcpy(stat->lsb, lsb, sizeof(lsb));
	AMI_DLOG("%d, %d, %d, %d, %d, %d",
		 lsb[0], lsb[1], lsb[2],
		 val->mag[0], val->mag[1], val->mag[2]);

	AMI_Sen2Dev(stat->dir, stat->polarity, val->mag);

	return 0;
}

/**
 * Search Offset(fine)
 *
 * @param handle	handle(returned by AMI_InitDriver())
 * @return result
 */
int AMI_SearchOffset(void *handle)
{
	struct ami_stat *stat = handle;
	int res = 0;
	u8 fine[3];

	if (stat->mode != AMI_FORCE_MODE) {
		AMI_LOG("mode != AMI_FORCE_MODE");
		return AMI_SEQ_ERR;
	}

	res = AMI_SearchOffsetProc(stat->i2c_hnd, fine);
	if (res < 0) {
		AMI_LOG("AMI_SearchOffsetProc ERROR(%d)", res);
		return res;
	}
	stat->prm.win.fine[0] = fine[0];
	stat->prm.win.fine[1] = fine[1];
	stat->prm.win.fine[2] = fine[2];

	return 0;
}

/**
 * Write Offset(fine)
 *
 * @param handle	handle(returned by AMI_InitDriver())
 * @param offset	sensor offset
 * @return result
 */
int AMI_WriteOffset(void *handle, u8 offset[3])
{
	int res = 0;
	struct ami_stat *stat = handle;
	int i = 0;

	for (i = 0; i < 3; i++) {
		if (offset[i] < AMI_FINE_MIN || offset[i] > AMI_FINE_MAX)
			return AMI_PARAM_ERR;
	}

	/* set current fine */
	res = AMI_Write_Offset(stat->i2c_hnd, offset);
	if (0 > res) {
		AMI_LOG("AMI_Write_Offset ERROR(%d)", res);
		return res;
	}
	stat->prm.win.fine[0] = offset[0];
	stat->prm.win.fine[1] = offset[1];
	stat->prm.win.fine[2] = offset[2];
	return 0;
}

/**
 * Read Offset(fine)
 *
 * @param handle	handle(returned by AMI_InitDriver())
 * @param offset	sensor offset
 * @return result
 */
int AMI_ReadOffset(void *handle, u8 offset[3])
{
	struct ami_stat *stat = handle;
	int res = 0;

	res = AMI_Read_Offset(stat->i2c_hnd, offset);
	if (0 > res) {
		AMI_LOG("AMI_Read_Offset ERROR(%d)", res);
		return res;
	}
	return 0;
}

/**
 * Set Soft iron parameter
 *
 * @param handle	handle(returned by AMI_InitDriver())
 * @param si	soft iron parameter
 * @return result
 */
int AMI_SetSoftIron(void *handle, s16 si[9])
{
	struct ami_stat *stat = handle;
	memcpy(stat->si, si, sizeof(s16)*9);
	return 0;
}

/**
 * Get Soft iron parameter
 *
 * @param handle	handle(returned by AMI_InitDriver())
 * @param si	soft iron parameter
 * @return result
 */
int AMI_GetSoftIron(void *handle, s16 si[9])
{
	struct ami_stat *stat = handle;
	memcpy(si, stat->si, sizeof(s16)*9);
	return 0;
}

/**
 * Set Sensor Direction Information
 *
 * @param handle	handle(returned by AMI_InitDriver())
 * @param dir	direction
 * @param polarity	polarity
 * @return result
 */
int AMI_SetDirection(void *handle, s16 dir, s16 polarity)
{
	struct ami_stat *stat = handle;
	stat->dir = dir;
	stat->polarity = polarity;
	return 0;
}

/**
 * Get Sensor Direction Information
 *
 * @param handle	handle(returned by AMI_InitDriver())
 * @param dir	direction
 * @param polarity	polarity
 * @return result
 */
int AMI_GetDirection(void *handle, s16 *dir, s16 *polarity)
{
	struct ami_stat *stat = handle;
	*dir = stat->dir;
	*polarity = stat->polarity;
	return 0;
}

/**
 * Read Sensor Parameter Information
 *
 * @param handle	handle(returned by AMI_InitDriver())
 * @param prm		Sensor Parameter Information
 * @return result
 */
int AMI_ReadParameter(void *handle, struct ami_sensor_parameter *prm)
{
	struct ami_stat *stat = handle;

	int res = AMI_ReadWinParam(stat->i2c_hnd, &prm->win);
	if (0 > res) {
		AMI_LOG("AMI_ReadWinParam ERROR(%d)", res);
		return res;
	}

	res = AMI_ReadMagParam(stat->i2c_hnd, &prm->mag);
	if (0 > res) {
		AMI_LOG("AMI_ReadMagParam ERROR(%d)", res);
		return res;
	}

	res = AMI_ChipInformation(stat->i2c_hnd, &prm->chip);
	if (0 > res) {
		AMI_LOG("AMI_ChipInformation ERROR(%d)", res);
		return res;
	}
	return 0;
}

/**
 * Read Device Driver Information
 *
 * @param handle	handle(returned by AMI_InitDriver())
 * @param drv		Device Driver Information
 * @return result
 */
int AMI_DriverInformation(void *handle, struct ami_driverinfo *drv)
{
	drv->ver_major = THIS_VER_MAJOR;
	drv->ver_middle = THIS_VER_MIDDLE;
	drv->ver_minor = THIS_VER_MINOR;
	sprintf((char *)drv->datetime, "%s %s", __DATE__, __TIME__);
	sprintf((char *)drv->remarks, "%s", THIS_CODE_REMARKS);
	return 0;
}

/**
 * Get Raw Value (LSB)
 *
 * @param handle	handle(returned by AMI_InitDriver())
 * @param val		Sensor Raw Value(LSB)
 * @return result
 */
int AMI_GetRawValue(void *handle, struct ami_sensor_value *raw_val)
{
	struct ami_stat *stat = handle;
	raw_val->mag[0] = stat->lsb[0];
	raw_val->mag[1] = stat->lsb[1];
	raw_val->mag[2] = stat->lsb[2];
	return 0;
}
