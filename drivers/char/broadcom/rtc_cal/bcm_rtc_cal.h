/*******************************************************************************
* Copyright 2012 Broadcom Corporation.  All rights reserved.
*
* 	@file	 rtcbcm_rtc_cal.h
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


#define RTC_CAL_QOS_DEFAULT_VALUE 	0xFFFFFFFF
#define RTC_CAL_DFS_MIN_VALUE 	0
#define RTC_OPP_UNSUPPORTED		0xFFFF

#define RTC_CAL_ACTIVE_STATE_INX		0

/**
Real Time Clock Time structure
**/
typedef struct{
	UInt8  Sec;		///< 0-59 seconds
	UInt8  Min;		///< 0-59 minutes
	UInt8  Hour;	///< 0-23 hours
	UInt8  Week;	///< 0-6==sun-sat week
	UInt8  Day;		///< 1-31 day
	UInt8  Month;	///< 1-12 Month
	UInt16 Year;	///< (RTC_YEARBASE) - (RTC_YEARBASE + 99)
} RTCTime_t;

/**
Real Time Clock Time structure
**/

typedef struct{
	u8  sec;		///< 0-59 seconds
	u8  min;		///< 0-59 minutes
	u8  hour;	///< 0-23 hours
	u8  week;	///< 0-6==sun-sat week
	u8  mday;		///< 1-31 day
	u8  mon;	///< 1-12 Month
	u16 year;	///< (RTC_YEARBASE) - (RTC_YEARBASE + 99)
} rtctime_t;

/**
RTC Time Calibration structure
**/
typedef struct{

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

} rtc_cal_TC_t;

struct rtccal_pdata 
{
	const char *device;
	unsigned long		nom_ratio;
	unsigned long		min_ratio;
	unsigned long		max_ratio;
	unsigned long		range_ppm;
	unsigned long		cal_thold;
	unsigned long		cal_meas_int;
	unsigned long		calc_interval;
	unsigned long		update_interval;
	unsigned long		slippage_err;
	unsigned long		method;
};

typedef unsigned char timer_id_type;

typedef struct{
	unsigned long ratio;
	unsigned int temp;
} rtc_cal_info_t;

typedef enum
{
	RTCCAL_ID_SETTIME,
	RTCCAL_ID_CALEVENT,
	RTCCAL_ID_CONFIG,
	RTCCAL_ID_REQCALLBACK,
	RTCCAL_ID_UPDATETIME,
	RTCCAL_ID_TIMERCB,
	RTCCAL_ID_TIMECORR,
	RTCCAL_ID_GETTIME,
	RTCCAL_ID_MAX
} rtc_cal_msg_id_t;

typedef enum
{
	RTCCAL_PARM_RATIO_NORM = 0, // clock ratio
	RTCCAL_PARM_RATIO_MIN = 1, // clock ratio
	RTCCAL_PARM_RATIO_MAX = 2, // clock ratio
	RTCCAL_PARM_RANGE = 3, // in ppm
	RTCCAL_PARM_THRESHOLD = 4, // in ppb
	RTCCAL_PARM_TIME_MEASURE = 5, // in milisecond
	RTCCAL_PARM_TIME_CALC = 6, // in milisecond
	RTCCAL_PARM_SLIPPAGE = 7,	// in milisecond
	RTCCAL_PARM_PERIODICUPDATE = 8,  // periodic update in mimnute
	RTCCAL_PARM_MAX		// reserved for all other indexes
} RTCCAL_Parm_t;

typedef struct
{
	rtc_cal_msg_id_t id;
	rtctime_t time;
	unsigned long val1;
	unsigned long val2;
	unsigned long val3;
	unsigned long val4;
} rtc_cal_msg_t;

enum
{
	RTC_CAL_ID_MM,
	RTC_CAL_ID_HUB_SWITCHABLE,
	RTC_CAL_ID_HUB_AON,
	RTC_CAL_ID_ARM_CORE,
	RTC_CAL_ID_ARM_SUB_SYSTEM,
	RTC_CAL_ID_MODEM,
	RTC_CAL_ID_MAX
};



//***************************************************************************************
/**
	This function is used to get RTCTime from PMU for startup
	
	@param		void
	@note

**/
void RTCCal_GetRTCTime(void);

//***************************************************************************************
/**
	This function is used to init task queue
	
	@note
	@return		Boolean

**/
bool RTCCal_Init(
	void
	);


//***************************************************************************************
/**
	This function is used to run RTC
	
	@param		void
	@note

**/
void RTCCal_Run(
	void
	);

//***************************************************************************************
/**
	Function used to update current time

	@param		time  in sec format
	@note
**/
void RTCCal_UpdateTime(
	unsigned long	time
	);

//***************************************************************************************
/**
	Function used to get time from RTC CAL for upper layer usage

	@param		rtc_ptr  in RTCTime_t pointer
	@note
**/
void RTCCal_GetTime(rtctime_t *rtc_ptr);
#endif // _RTC_CAL_H_
