/*******************************************************************************
* Copyright 2012 Broadcom Corporation.  All rights reserved.
*
* @file rtcbcm_rtc_cal.h
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

#ifndef _BCM_RTC_CAL_H_
#define _BCM_RTC_CAL_H_

#define RTC_CAL_QOS_DEFAULT_VALUE  0xFFFFFFFF
#define RTC_CAL_DFS_MIN_VALUE  0
#define RTC_OPP_UNSUPPORTED  0xFFFF

#define RTC_CAL_ACTIVE_STATE_INX  0

/**
Real Time Clock Time structure
**/
struct RTCTime_t {
	UInt8 Sec;
	UInt8 Min;
	UInt8 Hour;
	UInt8 Week;
	UInt8 Day;
	UInt8 Month;
	UInt16 Year;
};

/**
Real Time Clock Time structure
**/

struct rtctime_t {
	u8 sec;
	u8 min;
	u8 hour;
	u8 week;
	u8 mday;
	u8 mon;
	u16 year;
};

/**
RTC Time Calibration structure
**/
struct rtc_cal_TC_t {

	unsigned long time_next;

	u64 slippage_acc;

	unsigned long time_hw_time;
	unsigned long time_hz_acc;
	u64 time_err_acc;
	bool flag_err_acc;

	unsigned long time_hz_curr;
	unsigned long time_hz_prev;
	unsigned long time_hz_interval;
	u64 time_err_curr;
	bool flag_err_curr;

	unsigned long time_ratio_curr;
	bool flag_ratio_curr;
	unsigned long time_hz_delt;
	unsigned long slippage_ms;
	bool slippage_flag;
	int err_read1;
	int err_read2;
	int err_write;
	struct rtc_time tm_read;
	struct rtc_time tm_write;

};

struct rtccal_pdata {
	const char *device;
	unsigned long nom_ratio;
	unsigned long min_ratio;
	unsigned long max_ratio;
	unsigned long range_ppm;
	unsigned long cal_thold;
	unsigned long cal_meas_int;
	unsigned long calc_interval;
	unsigned long update_interval;
	unsigned long slippage_err;
	unsigned long method;
};

struct rtc_cal_info_t {
	unsigned long ratio;
	unsigned int temp;
};

enum rtc_cal_msg_id {
	RTCCAL_ID_SETTIME,
	RTCCAL_ID_CALEVENT,
	RTCCAL_ID_CONFIG,
	RTCCAL_ID_REQCALLBACK,
	RTCCAL_ID_UPDATETIME,
	RTCCAL_ID_TIMERCB,
	RTCCAL_ID_TIMECORR,
	RTCCAL_ID_GETTIME,
	RTCCAL_ID_MAX
};

enum RTCCAL_Parm {
	RTCCAL_PARM_RATIO_NORM = 0,
	RTCCAL_PARM_RATIO_MIN = 1,
	RTCCAL_PARM_RATIO_MAX = 2,
	RTCCAL_PARM_RANGE = 3,
	RTCCAL_PARM_THRESHOLD = 4,
	RTCCAL_PARM_TIME_MEASURE = 5,
	RTCCAL_PARM_TIME_CALC = 6,
	RTCCAL_PARM_SLIPPAGE = 7,
	RTCCAL_PARM_PERIODICUPDATE = 8,
	RTCCAL_PARM_MAX
};

struct rtc_cal_msg_t {
	unsigned char id;
	struct rtctime_t time;
	unsigned long val1;
	unsigned long val2;
	unsigned long val3;
	unsigned long val4;
};

enum {
	RTC_CAL_ID_MM,
	RTC_CAL_ID_HUB_SWITCHABLE,
	RTC_CAL_ID_HUB_AON,
	RTC_CAL_ID_ARM_CORE,
	RTC_CAL_ID_ARM_SUB_SYSTEM,
	RTC_CAL_ID_MODEM,
	RTC_CAL_ID_MAX
};

extern uint32_t __div64_32(uint64_t *dividend, uint32_t divisor);

void bcm_rtc_cal_ratio(unsigned long val1, unsigned long val2);

/**
	This function is used to get RTCTime from PMU for startup

	@param		void
	@note

**/
void RTCCal_GetRTCTime(void);

/**
	This function is used to init task queue

	@note
	@return		Boolean

**/
bool RTCCal_Init(void);

/**
	This function is used to run RTC

	@param		void
	@note

**/
void RTCCal_Run(void);

/**
	Function used to update current time

	@param		time  in sec format
	@note
**/
void RTCCal_UpdateTime(unsigned long time);

/**
	Function used to get time from RTC CAL for upper layer usage

	@param		rtc_ptr  in RTCTime_t pointer
	@note
**/
void RTCCal_GetTime(struct rtctime_t *rtc_ptr);
#endif /* _BCM_RTC_CAL_H_ */
