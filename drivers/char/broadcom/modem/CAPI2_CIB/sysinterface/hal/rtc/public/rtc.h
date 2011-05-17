/****************************************************************************
*
*     Copyright (c) 2007 Broadcom Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           Broadcom Corporation
*           5300 California Avenue
*           Irvine, California 92617
*
*     All information contained in this document is Broadcom Corporation
*     company private, proprietary, and trade secret.
*
****************************************************************************/
/**
*
*   @file   rtc.h
*
*   @brief  This file defines the low level interface to the Real Time Clock
*
****************************************************************************/
/**

*   @defgroup   RTCGroup   RTC Group
*   @ingroup    HALGroup
*
*   @brief      This group defines the low level interfaces and the utilities related to
				Real Time Clock.
*	
****************************************************************************/

/* Requires the following header files before its inclusion in a c file
*/
#ifndef _RTC_H_
#define _RTC_H_

/**
Minimum & Maximum values for TimeZone expressed as quarter of an hour related
to Greenwich Mean time
**/
#define MIN_TIMEZONE_VALUE	(-47)	//Those values should match the values of sTimeZoneTable
#define MAX_TIMEZONE_VALUE	48

/**
Value to indicate TimeZone has not been initialized
**/
#define INVALID_TIMEZONE_VALUE	(MAX_TIMEZONE_VALUE + 1)

/**
Minimum & Maximum values for DST (Daylight Saving Time). See Section 10.5.3.12 of 3GPP 24.008. 
**/
#define MIN_DST_VALUE 0
#define MAX_DST_VALUE 2

/**
Value to indicate DST has not been initialized
**/
#define INVALID_DST_VALUE	(MAX_DST_VALUE + 1)

/**
 * @addtogroup RTCGroup
 * @{
 */

#if defined( PMU_BCM59001 ) || defined( PMU_BCM59035 ) || defined(PMU_BCM59036) || defined(PMU_BCM59038) || defined(PMU_BCM59055) || defined(PMU_MAX8986)
	#define RTC_YEARBASE		2000			///< Base Year
#else
	#define RTC_YEARBASE		1970
#endif

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

/* Maximum number of years after our beginning year: 100 -----> (RTC_YEARBASE + 99). 
 * For "RTC_YEARBASE" defined to be 2000, our coverage spans from 2000 - 2099.
 *
 * Important:
 * Our PMU uses a 4-byte counter (1s unit) to keep the RTC time. Thus 0xFFFFFFFF/365/24/60/60 = 136.19 years.
 * The maximum span we can support is 136 years. Do not change "RTC_MAX_ELAPSE_YEAR" to more than 136!!!
 */
#if defined(PMU_MAX8986)
#define RTC_MAX_ELAPSE_YEAR 100     //maxim's range is 0-99 for year register
#else
#define RTC_MAX_ELAPSE_YEAR 130
#endif
#define RTC_COUNTER_MASK	( (UInt32)0xffffffff )	// bit 31-0 ( 32 bits )

/**
Real time clock alarm call back
**/

typedef void (*RTCAlarmCBF_T)(
	RTCTime_t	*time
	);

/**
Real time clock time call back
**/
typedef void (*RTCTimeCBF_T)(
	RTCTime_t	*time
	);

//***************************************************************************************
/**
    Function to report real time clock counter
	@return		Unsigned 32 bit value. Counter value
**/

UInt32 RTC_GetCounter(
	void
	);


//***************************************************************************************
/**
	This function calculates a new date in "RTCTime_t" format based upon another date in 
	"RTCTime_t" format and the offset in seconds.
	
	@param		*pNewTime (in) Pointer to new time in RTCTime_t format
	@param		*pStartTime (in) Pointer to start time in RTCTime_t format
	@param		rtc_difference (in) time difference between "pNewTime" and "pStartTime". 
	@note
	When "rtc_difference" is positive, pNewTime is later than "pStartTime", otherwise pNewTime is earlier.					 
**/

void RTC_OffsetToDate(
	RTCTime_t	*pNewTime,
	RTCTime_t	*pStartTime,
	Int32      rtc_difference
	);


//***************************************************************************************
/**
	This function calculates the difference in seconds for two dates passed in "RTCTime_t" format.

	@param		*pRefTime (in) Pointer to new time in RTCTime_t format
	@param		*pTime (in) Pointer to start time in RTCTime_t format
	@return		Signed 32 bit value. Difference in seconds 
	@note
	If pRefTime is earlier than pTime, the return value is positive. Otherwise it is negative 
**/

Int32 RTC_DateToOffset(RTCTime_t *pRefTime, RTCTime_t *pTime);


//***************************************************************************************
/**
	This function is used to return day of the week 

	@param		*time (in) Pointer to time in RTCTime_t format
	@return		Unsigned 8 bit value. Day of the week :0-6 == sun-sat
	@note
**/

UInt8 RTC_CalcWeekDay(
	RTCTime_t	*time
	);


//***************************************************************************************
/**
	Function to check time 

	@param		*time (in) Pointer to time in RTCTime_t format
	@return		Boolean value.  TRUE if passed in time is accurate. 
	@note
**/

Boolean RTC_CheckTime(
	RTCTime_t	*time
	);


//***************************************************************************************
/**
	This function is to initialize RTC. It shall be called during powerup 
	only if PMU exists.	
**/

void RTC_Init(
	void
	);

//***************************************************************************************
/**
	This function is to initialize RTC without PMU. It shall be called during powerup 
	only if PMU does not exist. This is to let the phone keep the current time in SRAM, 
	which will be reset once powered off. 
**/

void RTC_InitNoPmu(
	void
	);

//***************************************************************************************
/**
	This function is used to register a call back function to receive the minute interrupt event

	@param		RTCTimeCB (in) RTC time callback
	@note
	This function should be called together with RTC_EnableMinuteInt(). 

**/

void RTC_TimeRegister(
	RTCTimeCBF_T	RTCTimeCB
	);

//***************************************************************************************
/**
	This function is used to register a call back function to receive the second interrupt event

	@param		RTCTimeCB (in) RTC time callback
	@note
	This function should be called together with RTC_EnableSecondInt(). 

**/

void RTC_TimeSecondRegister(
	RTCTimeCBF_T	RTCTimeCB
	);

//***************************************************************************************
/**
	This function is used to enable the minute interrupt in PMU.
	When minute interrupt is enabled, the RTC module will call the registered call back function
	when the PMU triggers the minute interrupt at the end of every minute.
	This function should be called only if hardware PMU exists.
**/

void RTC_EnableMinuteInt(
	void
	);

//***************************************************************************************
/**
	This function is used to disable the mimute interrupt in the PMU.
	It should be called only if hardware PMU exists.	
**/

void RTC_DisableMinuteInt(
	void
	);

//***************************************************************************************
/**
	This function is used to enable the second interrupt in PMU.
	When second interrupt is enabled, the RTC module will call the registered call back function
	when the PMU triggers the second interrupt at the end of every second.
	This function should be called only if hardware PMU exists.
**/

void RTC_EnableSecondInt(
	void
	);

//***************************************************************************************
/**
	This function is used to disable the second interrupt in the PMU.
	It should be called only if hardware PMU exists.	
**/

void RTC_DisableSecondInt(
	void
	);

//***************************************************************************************
/**
	This function is used to get Time Zone

	@return		Int8 value of timezone (range from -47 to +48) or INVALID_TIMEZONE_VALUE. 
				INVALID_TIMEZONE_VALUE means no Time Zone information is received
				from network in MM Information or GMM Information messages. 
	@note
**/

Int8 RTC_GetTimeZone(void);

//***************************************************************************************
/**
	This function is used to set Time Zone

	@param		time_zone (in) Time Zone
	@note
	time_zone indicates the difference, expressed in quarters of an hour, 
	between the local time and Greenwich Mean Time (GMT). Value 
	ranges from -47 to +48. If INVALID_TIMEZONE_VALUE is passed, this function will 
	reset the timezone value as if it had never been initialized. 
**/

void RTC_SetTimeZone(Int8 time_zone);

//***************************************************************************************
/**
	This function is used to get DST (DayLight Saving Time) information. 

	@return		UInt8 value of DST (range from 0-2, see Section 10.5.3.12 of 3GPP 24.008)
				or INVALID_DST_VALUE. INVALID_DST_VALUE means no DST information is received
				from the network in MM Information or GMM Information messages.  
	@note
**/

UInt8 RTC_GetDST(void);

//***************************************************************************************
/**
	This function is used to set DST (DayLight Saving Time) information. 

	@param		dst (in) DST value, UInt8 value of DST (range from 0-2, see Section 
				10.5.3.12 of 3GPP 24.008) or INVALID_TIMEZONE_VALUE. INVALID_TIMEZONE_VALUE 
				means no Time Zone information is received from network in MM Information 
				or GMM Information messages. 
	@note
	
**/

void RTC_SetDST(UInt8 dst);

//***************************************************************************************
/**
	Function used to set current time

	@param		*time (in) Pointer to time in RTCTime_t format
	@note
	RTCTime_t month and day are 1-based.
	The function should be called only if PMU exists in HW.
**/

void RTC_SetTime(
	RTCTime_t	*time
	);


//***************************************************************************************
/**
	Function used to get current time

	@param		*pRtc (in) Pointer to time in RTCTime_t format
	@note
	RTCTime_t month and day are 1-based.
	The function should be called only if PMU exists in HW.
**/

void RTC_GetTime(
	RTCTime_t	*pRtc
	);


//***************************************************************************************
/**
	Return the offset in seconds of the current time since our 
	base time (Jan, 1, RTC_YEARBASE, 00:00:00)

	@return		Unsigned 32 bit value providing Offset in seconds
**/

UInt32 RTC_GetSecond(void);


//***************************************************************************************
/**
	Function to calculate the total seconds from RTC_YEARBASE.Converts RTCTime_t format 
	to 32-bit value (in unit of seconds)

	@param		pSrc (in) Time to convert from.
	@return		Unsigned 32 bit value in unit of second counted from YEAR_BASE.
	@note
	RTCTime_t year is bigger than RTC_YEARBASE.
**/

UInt32 RTC_Convert_RTCTime_t_To_Second(
	const RTCTime_t* const pSrc
	);

//***************************************************************************************
/**
	Function to calculate the total seconds from RTC_YEARBASE.Converts RTCTime_t format 
	to 32-bit value (in unit of seconds)

	@param		pDest   Converted RTC Time
	@param		time	Unsigned 32 bit value in unit of second counted from YEAR_BASE.
	@note		RTCTime_t year is bigger than RTC_YEARBASE.
**/

void RTC_Convert_Second_To_RTCTime_t(
	RTCTime_t *pDest, 
	UInt32 time
	);

//***************************************************************************************
/**
	This function is used to register a callback to receive RTC Alarm event.

	@param		RTCAlarmCB (in) RTC Alarm callback
	@note
	This function should be called together with RTC_EnableAlarm and RTC_SetAlarmTime.
**/

void RTC_AlarmRegister(
	RTCAlarmCBF_T	RTCAlarmCB
	);


//***************************************************************************************
/**
	Function to enable the alarm interrupt in PMU.
	When the alarm interrupt is enabled, the RTC module will call the registered call back function 
	when the  PMU triggers the alarm interrupt at the time set by RTC_SetAlarmTime.
	This function should be called only if hardware PMU exists.	
**/

void RTC_EnableAlarm(
	void
	);


//***************************************************************************************
/**
	Function to disable the alarm interrupt in PMU. This must 
	be called only if hardware PMU exists.	
**/

void RTC_DisableAlarm(
	void
	);

//***************************************************************************************
/**
	This function is used to set RTC Alarm time

	@param		*time (in) Pointer to RTC Alarm time
	@note
	The function should be called only if PMU exists in HW.

**/

void RTC_SetAlarmTime(
	RTCTime_t	*time
	);


//***************************************************************************************
/**
	This function is used to get RTC Alarm time

	@param		*pRtc (in) Pointer to RTC Alarm time
	@note
	The function should be called only if PMU exists in HW.

**/

void RTC_GetAlarmTime(
	RTCTime_t	*pRtc
	);


//***************************************************************************************
/**
	This function is used to get the number of days in a particular month of a year
	
	@param		year (in) Year to calculated against
	@param		month (in) Month to be checked
	@note
	Pass 2001 in "year" and 2 in "month" for Feb, 2001.In a leap year, 
	the month of Feb has 29 days instead of 28.

**/

UInt8 RTC_GetNumOfDaysInMon(
	UInt16 year, 
	UInt8 month
	);

/** @} */

#endif	// _DEV_RTC_H_

