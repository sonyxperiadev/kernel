/*******************************************************************************
* Copyright 2012 Broadcom Corporation.  All rights reserved.
*
* @file bcm_rtc_cal.c
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
#include <linux/version.h>
#include <linux/signal.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/math64.h>
#include <linux/atomic.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/rtc.h>
#include <linux/bcd.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/percpu.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/syscore_ops.h>
#include <linux/clocksource.h>
#include <linux/jiffies.h>
#include <linux/time.h>
#include <linux/tick.h>
#include <linux/stop_machine.h>
#include <linux/bitops.h>
#include <asm/vfp.h>
#include <../../vfp/vfpinstr.h>
#include <../../vfp/vfp.h>
#include <linux/mfd/bcmpmu.h>
#include <linux/io.h>
#include "mobcom_types.h"
#include "taskmsgs.h"
#include "msconsts.h"
#include <linux/delay.h>
#include <linux/syscalls.h>
#include <linux/kfifo.h>
#include <linux/semaphore.h>
#include <linux/wait.h>

#include <linux/broadcom/bcm_major.h>
#include <linux/broadcom/bcm_rpc.h>
#include <linux/broadcom/ipcinterface.h>
#include <linux/broadcom/ipcproperties.h>
#include "bcm_rtc_cal.h"
#include "rtc_sc.h"

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#endif

#define RTC_YEARBASE		2000
#define RTC_MAX_ELAPSE_YEAR 130

#define RTC_QUEUESIZE 32
#define RTC_TC_ACCURACY ((double)0.5)	/* 500ms polling */
#define RTC_ONE_SEC  1000

#define RTC_TC_1312M  (8666667) /* 13/12Mhz */
#define RTC_TC_26M (52000000) /* 26Mhz */

#define RTC_TC_BASERATIO  (RTC_TC_1312M) /* based on Athena */

#define RTC_RATIO  (1000000)
#define RTC_RESET_SLIPPAGE  (0)
#define RTC_PERIODICUPDATE  (HZ * 60)

#define RTC_HZ2FPU  (1000000)
#define RTC_RES  (1000000)
#define RTC_MULF  (10)
#define RTCINIT_0  (0)
#define RTCINIT_NULL  (NULL)
#define RTCINIT_FALSE	(false)


#define pr_rtcdebug(fmt, ...) \
	pr_debug(pr_fmt(fmt), ##__VA_ARGS__)

/* The Task Structure */
struct TaskStruct_t {
	struct workqueue_struct *wrk_q;
	struct work_struct wrk;
};

static struct work_struct rtccal_work;
static struct workqueue_struct *rtccal_workqueue = RTCINIT_NULL;
static void *rtccal_semaphore = RTCINIT_NULL;

struct kfifo rtccal_kfifo;
spinlock_t rtccal_flock;
static wait_queue_head_t rtccal_q;

static int rtc_cal_debug = RTCINIT_0;

static u32 rtc_cal_test_ratio = RTCINIT_0;
static u32 rtc_cal_cp_ratio = RTCINIT_0;

static struct rtc_cal_TC_t sRTC_TC = {RTCINIT_0};

static struct rtc_cal_TC_t *pRTC_TC = RTCINIT_NULL;

static bool  IsRTCInit = RTCINIT_FALSE;
static bool  IsRTCRun = RTCINIT_FALSE;

static unsigned long RTCCAL_Ratio = RTCINIT_0;
static unsigned long RTCCAL_Ratio_min = RTCINIT_0;
static unsigned long RTCCAL_Ratio_max = RTCINIT_0;
static unsigned long RTCCAL_Range = RTCINIT_0;
static unsigned long RTCCAL_Threshold = RTCINIT_0;
static unsigned long RTCCAL_Int_Measure = RTCINIT_0;
static unsigned long RTCCAL_Int_Calc = RTCINIT_0;
static unsigned long RTCCAL_Slippage = RTCINIT_0;
static unsigned long RTCCAL_PeriodicUpdate = RTCINIT_0;
static unsigned long RTCCAL_Method = RTCINIT_0;

static s64 rtccal_period = RTCINIT_0;
static bool rtc_correction = RTCINIT_FALSE;
static struct timer_list rtccal_tm_update;
static struct timer_list rtccal_tm_correct;

static Boolean IsTimerRun = RTCINIT_FALSE;
static Boolean IsCorrectionTimerRun = RTCINIT_FALSE;

static unsigned long rtc_jiffies;
static unsigned long rtc_jiffies_prev;

struct bcmpmu_rtc {
	struct rtc_device *rtc;
	struct bcmpmu *bcmpmu;
	wait_queue_head_t wait;
	struct mutex lock;
	int alarm_irq_enabled;
	int update_irq_enabled;
};

static struct bcmpmu_rtc *rtc_cal_bcmpmu = RTCINIT_NULL;

#if defined(CONFIG_MFD_BCM59055)
struct rtccal_pdata rtccal_data = {
	.device = "BCM59055",
	.nom_ratio = 8666667,
	.min_ratio = 8663200,
	.max_ratio = 8670134,
	.range_ppm = 400,
	.cal_thold = 0,
	.cal_meas_int = 0,
	.calc_interval = 600,
	.update_interval = 0,
	.slippage_err = 500,
	.method = 0,
};
#else /* default values */
struct rtccal_pdata rtccal_data = {
	.device = "BCM590xx",
	.nom_ratio = 8666667,
	.min_ratio = 8663200,
	.max_ratio = 8670134,
	.range_ppm = 400,
	.cal_thold = 0,
	.cal_meas_int = 0,
	.calc_interval = 600,
	.update_interval = 0,
	.slippage_err = 500,
	.method = 0,
};

#endif


#ifdef CONFIG_DEBUG_FS


#endif /*CONFIG_DEBUG_FS*/


/* Utility APIs for RTC time conversion */

#define TIME_ZONE_BASIC_BLOCK_IN_MIN  15

static const unsigned char sYearCode[100] = {
	0, 1, 2, 3, 5, 6, 0, 1, 3, 4, 5, 6, 1, 2, 3, 4, 6, 0, 1, 2,\
	4, 5, 6, 0, 2, 3, 4, 5, 0, 1, 2, 3, 5, 6, 0, 1, 3, 4, 5, 6,\
	1, 2, 3, 4, 6, 0, 1, 2, 4, 5, 6, 0, 2, 3, 4, 5, 0, 1, 2, 3,\
	5, 6, 0, 1, 3, 4, 5, 6, 1, 2, 3, 4, 6, 0, 1, 2, 4, 5, 6, 0,\
	2, 3, 4, 5, 0, 1, 2, 3, 5, 6, 0, 1, 3, 4, 5, 6, 1, 2, 3, 4};
static const unsigned char sMonthCode[12] = {
	0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5};

static uint64_t div64(uint64_t v, uint32_t d)
{
	uint64_t n;
	uint32_t rem;

	n = v;
	rem = __div64_32(&n, d);

	return n;
}

static void rtc_cal_pr_vars(void)
{
	pr_rtcdebug("\t%s:internal variables", __func__);
	pr_rtcdebug(
		"\t\thw_time:%lu, \tthz_acc:%lu, \tslip_acc:%llu, \terr_acc:%llu, \tflag_err_acc:%d",
		pRTC_TC->time_hw_time, pRTC_TC->time_hz_acc,
		pRTC_TC->slippage_acc, pRTC_TC->time_err_acc,
		pRTC_TC->flag_err_acc);
	pr_rtcdebug(
		"\t\thz_curr:%lu, \thz_prev:%lu, \thz_intv:%lu, \terr_curr:%llu, \tflag_err_curr:%d",
		pRTC_TC->time_hz_curr, pRTC_TC->time_hz_prev,
		pRTC_TC->time_hz_interval, pRTC_TC->time_err_curr,
		pRTC_TC->flag_err_curr);
	pr_rtcdebug(
		"\t\tratio_curr:%lu, \tflag_ratio:%d, \ttime_next:%lu, \thz_delt:%lu",
		pRTC_TC->time_ratio_curr, pRTC_TC->flag_ratio_curr,
		pRTC_TC->time_next, pRTC_TC->time_hz_delt);
}

/**************************************************************************
 * Function Name:	rtc_util_isleapyear
 *
 * Description:
 *    This function returns TRUE if the passed year is a leap year in
 *    which the month of Feb contains 29 days and the year contains 366 days.
 *
 * unsigned int year - Year in number. For example, pass 2008 for Year 2008.
 *
 ***************************************************************************/
bool rtc_util_isleapyear(unsigned int year)
{
	return
		(((year % 4) == 0) && ((year % 100) != 0))
		|| ((year % 400) == 0) ;
}


/**************************************************************************
 * Function Name:	rtc_util_get_numofdaysinyear
 *
 * Description: This function returns the number of days in a particular year
 *
 * unsigned int year - Year in number. For example, pass 2008 for Year 2008.
 *
 **************************************************************************/
unsigned int rtc_util_get_numofdaysinyear(unsigned int year)
{
	return rtc_util_isleapyear(year) ? 366 : 365;
}

/**************************************************************************
 *
 * Function Name: rtc_util_calcweekday
 *
 * Description:
 *
 *
 * Notes:         Return WeekDay: 0-6 == sun-sat
 *
 **************************************************************************/
unsigned char rtc_util_calcweekday(struct rtctime_t	*time)
{

	unsigned int year = time->year;
	unsigned char month = time->mon;
	unsigned char day = time->mday;
	char leapCode = 0;
	unsigned char weekday;

	if (rtc_util_isleapyear(year) && ((month == 1) || (month == 2)))
		leapCode = -1;

	weekday = (6 + sYearCode[year-RTC_YEARBASE]
		+ sMonthCode[month-1] + day + leapCode) % 7;

	return weekday;

}

/**************************************************************************
 * Function Name:	rtc_util_getnumofdaysinmon
 *
 * Description:
 *    This function returns the number of days in a particular month of a year
 *
 * unsigned int year - Year in number. For example, pass 2008 for Year 2008.
 * unsigned char month - Month in number from 1 to 12.
 *
 **************************************************************************/
unsigned char rtc_util_getnumofdaysinmon(
	unsigned int year, unsigned char month)
{
	static const unsigned char num_of_days_in_month[] = {
		31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	if ((month == 2) && rtc_util_isleapyear(year))
		return 29;
	else
		return num_of_days_in_month[month - 1];

}


/**************************************************************************
 * Function Name:	rtc_util_convert_rtctime_t2sec
 *
 * Description:		This function converts rtctime_t value to second value
 *
 * rtctime_t pTimeSrc - rtctime_t value
 *
 **************************************************************************/
unsigned long rtc_util_convert_rtctime_t2sec(struct rtctime_t *pTimeSrc)
{
	unsigned char x;
	unsigned long days = 0;

	pr_debug("%s", __func__);

	for (x = 0; x < pTimeSrc->year - RTC_YEARBASE; x++)
		days += rtc_util_get_numofdaysinyear(RTC_YEARBASE + x);

	for (x = 1; x < pTimeSrc->mon; x++)
		days += rtc_util_getnumofdaysinmon(pTimeSrc->year, x);

	return (days + pTimeSrc->mday - 1) * 24 * 60 * 60
		+ (unsigned long)(pTimeSrc->hour) * 60 * 60
		+ (unsigned long)(pTimeSrc->min) * 60
		+ pTimeSrc->sec;
}

/**************************************************************************
 *
 * Function Name:	rtc_util_getmaxu32time
 *
 * Description:
 *    Return the maximum unsigned long time in seconds allowed according to our
 *    covered range (Year 2100, Jan, Day 1, Hour 0, Min 0, Sec 0).
 *
 *
 **************************************************************************/
static unsigned long rtc_util_getmaxu32time(void)
{
	struct rtctime_t end_time;

	end_time.sec   = 0;	/* 0-59 */
	end_time.min   = 0;	/* 0-59 */
	end_time.hour  = 0;	/* 0-23 */
	end_time.mday   = 1; /* 1-31 */
	end_time.mon = 1; /* 1-12 */
	end_time.year  = RTC_YEARBASE + RTC_MAX_ELAPSE_YEAR;
	end_time.week  = rtc_util_calcweekday(&end_time);

	return rtc_util_convert_rtctime_t2sec(&end_time) - 1;
}


/**************************************************************************
 * Function Name:	rtc_util_convert_sec2rtctime_t
 *
 * Description:		This function converts second value to rtctime_t value
 *
 * rtctime_t pDest - rtctime_t value
 * unsigned long time - second value
 *
 **************************************************************************/
void rtc_util_convert_sec2rtctime_t(struct rtctime_t *pDest, unsigned long time)
{
	unsigned long  max_time = rtc_util_getmaxu32time();
	unsigned long  days;
	unsigned long  y_days;
	unsigned char   y;
	unsigned char   m;
	unsigned char	m_days;

	pr_debug("%s", __func__);

	if (time > max_time) {
		pr_debug("%s:over max_time", __func__);
		time = max_time;
	}

	days = time / (24 * 60 * 60);

	time %= (24 * 60 * 60);
	pDest->hour = (unsigned char)(time / (60 * 60));
	time %= (60 * 60);
	pDest->min = (unsigned char)(time / 60);
	pDest->sec = (unsigned char)(time % 60);

	for (y = 0; y < RTC_MAX_ELAPSE_YEAR; y++) {
		y_days = rtc_util_get_numofdaysinyear(RTC_YEARBASE + y);

		if (days > y_days)
			days -= y_days;
		else if (days < y_days)
			break;
		else {
			pDest->year = y + 1 + RTC_YEARBASE;
			pDest->mon = (unsigned char)1;
			pDest->mday = 1;
			rtc_util_calcweekday(pDest);
			return;
		}
	}

	for (m = 1; m <= 12 ; m++) {
		m_days = rtc_util_getnumofdaysinmon(RTC_YEARBASE + y, m);

		if (days > m_days) {
			days -= m_days;
		} else if (days < m_days) {
			break;
		} else {
			pDest->year = y + RTC_YEARBASE;
			pDest->mon = (unsigned char)(m + 1);
			pDest->mday = 1;
			rtc_util_calcweekday(pDest);
			return;
		}
	}

	pDest->year = y + RTC_YEARBASE;
	pDest->mon = (unsigned char) m;

	pDest->mday =
		(days >= rtc_util_getnumofdaysinmon(RTC_YEARBASE + y, m))
		? 1 : (days + 1);
	rtc_util_calcweekday(pDest);
}


/**
*	This function is used to check runing flag of RTC Calibration
**/
bool rtc_cal_isrun(void)
{
	return IsRTCRun;
}


/**************************************************************************
 * Function Name:	rtc_util_convert_tm2rtctime_t
 *
 * Description:		This function converts second value to rtctime_t value
 *
 * rtctime_t pDest - rtctime_t value
 * unsigned long time - second value
 *
 ***************************************************************************/
void rtc_util_convert_tm2rtctime_t(struct rtctime_t *pDest, struct rtc_time *tm)
{
	pDest->year = tm->tm_year + RTC_YEARBASE - 100;
	pDest->mon = tm->tm_mon + 1;
	pDest->mday = tm->tm_mday;
	pDest->week = tm->tm_wday;
	pDest->hour = tm->tm_hour;
	pDest->min = tm->tm_min;
	pDest->sec = tm->tm_sec;
	pr_rtcdebug("%s:IN:Y=%d,M/M/W=%d/%d/%d,H:M:S=%d:%d:%d",
		__func__, tm->tm_year, tm->tm_mon, tm->tm_mday,
		tm->tm_wday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	pr_rtcdebug("%s:OUT:Y=%d,M/M/W=%d/%d/%d,H:M:S=%d:%d:%d",
		__func__, pDest->year, pDest->mon, pDest->mday,
		pDest->week, pDest->hour, pDest->min, pDest->sec);
}

/**************************************************************************
 * Function Name:	rtc_util_convert_rtctime_t2tm
 *
 * Description:		This function converts second value to rtctime_t value
 *
 * rtctime_t pDest - rtctime_t value
 * unsigned long time - second value
 *
 **************************************************************************/
void rtc_util_convert_rtctime_t2tm(struct rtc_time *pDest, struct rtctime_t *tm)
{
	pDest->tm_year = tm->year + 100 - RTC_YEARBASE;
	pDest->tm_mon = tm->mon - 1;
	pDest->tm_mday = tm->mday;
	pDest->tm_wday = tm->week;
	pDest->tm_hour = tm->hour;
	pDest->tm_min = tm->min;
	pDest->tm_sec = tm->sec;
	pr_rtcdebug("%s:IN:Y=%d,M/M/W=%d/%d/%d,H:M:S=%d:%d:%d",
		__func__, tm->year, tm->mon, tm->mday,
		tm->week, tm->hour, tm->min, tm->sec);
	pr_rtcdebug("%s:OUT:Y=%d,M/M/W=%d/%d/%d,H:M:S=%d:%d:%d",
		__func__, pDest->tm_year, pDest->tm_mon,
		pDest->tm_mday, pDest->tm_wday, pDest->tm_hour,
		pDest->tm_min, pDest->tm_sec);
}


/* RTC CAL Messages */

/**
	This function is used to send task message to RTC Cal

	@param		id (in) Task Message ID
	@param		val1 (in) val1 parameter
	@param		val2 (in) val2 parameter
	@param		val3 (in) val3 parameter
	@param		val4 (in) val4 parameter
	@note
	@return		Boolean

**/
bool rtc_cal_msg(
	unsigned char id, unsigned long val1, unsigned long val2,
	unsigned long val3, unsigned long val4)
{

	struct rtc_cal_msg_t	msg;
	static unsigned long count = RTCINIT_0;
	unsigned int len;

	msg.id = id;
	msg.val1 = val1;
	msg.val2 = val2;
	msg.val3 = val3;
	msg.val4 = val4;

	if (!rtc_cal_isrun()) {
		pr_rtcdebug("rtc_cal_msg : not running yet");
		return false;
	}

	count++;

	len = kfifo_in_locked(&rtccal_kfifo, (unsigned char *)&msg,
		sizeof(struct rtc_cal_msg_t), &rtccal_flock);
	if (len != sizeof(struct rtc_cal_msg_t))
		pr_err("%s len=%u", __func__, len);

	wake_up(&rtccal_q);

	pr_info(
	"%s : msg.id = %u(%lu), val1=%lu, val2=%lu, val3=%lu, val4=%lu,len=%u",
	__func__, (unsigned int)msg.id, count, val1, val2, val3, val4, len);

	return true;
}

/**
	This function is used to send calibration ratio event

	@param		ratio (in) UInt32 for ratio
	@param		temp (in) UInt16 for temp
	@note

**/
void rtc_cal_msg_ratio_event(unsigned long ratio, unsigned int temp)
{
	pr_debug("%s : ratio=%lu, temp=%u", __func__, ratio, temp);
	rtc_cal_msg(RTCCAL_ID_CALEVENT, ratio, (unsigned long) temp, 0, 0);
}

/**
	This function is used to send SetTime event

	@param		time (in) unsigned long for time input
	@note

**/
void rtc_cal_msg_set_time(unsigned long time)
{
	pr_debug("%s : time = %lu", __func__, time);
	rtc_cal_msg(RTCCAL_ID_SETTIME, time, 0, 0, 0);
}


/**
	This function is used to update time to PMU(i.e. set time usage)

	@param		time (in) unsigned long for time
	@note

**/
void rtc_cal_msg_update_time(unsigned long time)
{
	pr_debug("%s : time=%lu", __func__, time);
	rtc_cal_msg(RTCCAL_ID_UPDATETIME, time, 0, 0, 0);
}



/**
	This function is used to send timer callback

	@param		void
	@note

**/
void rtc_cal_msg_timer_cb(void)
{
	pr_debug("%s", __func__);
	rtc_cal_msg(RTCCAL_ID_TIMERCB, 0, 0, 0, 0);
}



/**
	This function is used to send time correction for update PMU

	@param		unsigned long
	@note

**/

void rtc_cal_msg_time_correction(unsigned long arg)
{
	pr_debug("%s : %lu", __func__, arg);
	rtc_cal_msg(RTCCAL_ID_TIMECORR, 0, 0, 0, 0);
}


/**
	This function is used to get RTC time for initialization of calibration process

	@param		void
	@note

**/

void rtc_cal_msg_get_rtctime(void)
{
	pr_debug("%s", __func__);
	rtc_cal_msg(RTCCAL_ID_GETTIME, 0, 0, 0, 0);
}



/* RTC Calibration APIs */

int rtc_cal_get_read_rtctime(struct rtc_time *tm)
{
	int ret;
	unsigned int val;
	struct bcmpmu_rtc *rdata;

	pr_rtcdebug("%s", __func__);

	if (rtc_cal_bcmpmu == NULL)
		return -1;

	rdata = rtc_cal_bcmpmu;

	pRTC_TC->err_read1 = 0;
	pRTC_TC->err_read2 = 0;

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCYR,
					&val, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;
	tm->tm_year = (val & 0xFF) + 100;

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCMT,
					&val, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;
	if (val >= 1)
		tm->tm_mon = (val & 0x0F) - 1;

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCDT,
					&val, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;
	tm->tm_mday = (val & 0x1F);

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCHR,
					&val, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;
	tm->tm_hour = (val & 0x1F);

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCMN,
					&val, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;
	tm->tm_min = (val & 0x3F);

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCSC,
		&val, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;
	tm->tm_sec = (val & 0x3F);

	pRTC_TC->err_read1 = rtc_valid_tm(tm);
	ret = pRTC_TC->err_read1;

err:

	pRTC_TC->err_read2 = ret;
	if (ret)
		memcpy(&pRTC_TC->tm_read, &tm, sizeof(tm));

	pr_rtcdebug("%s: err=%d time=%d.%d.%d.%d.%d.%d",
		__func__, ret,
		tm->tm_year, tm->tm_mon, tm->tm_mday,
		tm->tm_hour, tm->tm_min, tm->tm_sec);

	return ret;
}


void bcm_rtc_cal_itimer_cb(unsigned long arg)
{
	pr_rtcdebug("%s : %lu", __func__, arg);

	rtc_cal_msg_timer_cb();

	rtc_jiffies = jiffies;

	mod_timer(&rtccal_tm_update, rtc_jiffies + RTCCAL_Int_Calc*HZ);
}

void bcm_rtc_cal_timerratio(unsigned long ratio, unsigned int temp)
{

	pr_rtcdebug("%s:ratio=%lu,temp=%u", __func__, ratio, temp);

	if (ratio == RTCSC_INVALID_RATIO)
		ratio = RTCCAL_Ratio;
	if (ratio > RTCCAL_Ratio_max)
		ratio = RTCCAL_Ratio_max;
	if (ratio < RTCCAL_Ratio_min)
		ratio = RTCCAL_Ratio_min;

	if (ratio < RTCCAL_Ratio) {
		pRTC_TC->time_ratio_curr = RTCCAL_Ratio - ratio;
		pRTC_TC->flag_ratio_curr = true;
		pRTC_TC->flag_err_curr = true;
	} else {
		pRTC_TC->time_ratio_curr = ratio - RTCCAL_Ratio;
		pRTC_TC->flag_ratio_curr = false;
		pRTC_TC->flag_err_curr = false;
	}
	pr_rtcdebug("%s:ratio=%lu,ratio_curr/time=%lu,flag=%u",
		__func__, ratio, pRTC_TC->time_ratio_curr,
		pRTC_TC->flag_ratio_curr);
}


static void rtc_cal_correctiontimerstart(unsigned long delay)
{

	pr_rtcdebug("%s:delay=%lu", __func__, delay);

	if (IsCorrectionTimerRun)
		pr_rtcdebug("%s:IsCorrectionTimerRun", __func__);

	del_timer_sync(&rtccal_tm_correct);
	memset(&rtccal_tm_correct, 0, sizeof(rtccal_tm_correct));
	init_timer(&rtccal_tm_correct);
	rtccal_tm_update.function = rtc_cal_msg_time_correction;

	rtc_jiffies = jiffies;

	mod_timer(&rtccal_tm_update, rtc_jiffies + delay);

	IsCorrectionTimerRun = TRUE;

}

static void rtc_cal_correctiontimerstop(void)
{
	pr_rtcdebug("%s", __func__);

	del_timer_sync(&rtccal_tm_correct);
	IsCorrectionTimerRun = FALSE;
}

static void rtc_cal_updatetimerstart(void)
{
	pr_rtcdebug("%s", __func__);

	if (IsTimerRun)
		pr_rtcdebug("%s:IsTimerRun", __func__);

	del_timer_sync(&rtccal_tm_update);
	memset(&rtccal_tm_update, 0, sizeof(rtccal_tm_update));
	init_timer(&rtccal_tm_update);
	rtccal_tm_update.function = bcm_rtc_cal_itimer_cb;

	rtc_jiffies = jiffies;

	mod_timer(&rtccal_tm_update, rtc_jiffies + RTCCAL_Int_Calc * HZ);

	IsTimerRun = TRUE;

}

static void rtc_cal_updatetimerstop(void)
{
	pr_rtcdebug("%s", __func__);

	del_timer_sync(&rtccal_tm_update);
	IsTimerRun = FALSE;
}

static u64 rtc_cal_tc_updatevals(unsigned long time)
{
	unsigned long time_left, time_total;
	u64 rtc_slip;
	u64 rtc_mul = 0, rtc_mul1 = 0;
	u64 rtc_m = 0, rtc_n = 0;
	rtc_m = (u64) RTCCAL_Slippage;
	rtc_n = (u64) RTCCAL_Ratio;

	pRTC_TC->time_hz_delt = time * HZ;
	time_total = pRTC_TC->time_hw_time * HZ + pRTC_TC->time_hz_acc
		+ pRTC_TC->time_hz_curr;

	time_left = pRTC_TC->time_hz_delt - pRTC_TC->time_hw_time * HZ;
	pr_rtcdebug("%s: hw=%lu, hz_acc=%lu, hz_curr=%lu",
		__func__, pRTC_TC->time_hw_time * HZ,
		pRTC_TC->time_hz_acc, pRTC_TC->time_hz_curr);
	pr_rtcdebug("%s: t=%lu, l=%lu, delt=%lu",
		__func__, time_total, time_left, pRTC_TC->time_hz_delt);

	pRTC_TC->time_hw_time = time;

	if (time_left > pRTC_TC->time_hz_acc) {
		time_left -= pRTC_TC->time_hz_acc;
		pRTC_TC->time_hz_acc = 0;
	} else
		pr_rtcdebug("%s:NO00", __func__);

	if (time_left > pRTC_TC->time_hz_curr) {
		pr_rtcdebug("%s:NO01 %lu", __func__,
			time_left-pRTC_TC->time_hz_curr);
		msleep((time_left-pRTC_TC->time_hz_curr)*(RTC_ONE_SEC/HZ));
	} else
		pRTC_TC->time_hz_curr -= time_left;

	pr_rtcdebug("%s: hz_curr=%lu, hz_acc=%lu, time_left=%lu",
		__func__, pRTC_TC->time_hz_curr,
		pRTC_TC->time_hz_acc, time_left);


	mul64to128(&rtc_mul1, &rtc_mul, rtc_m, rtc_n);
	rtc_slip = div64(rtc_mul, (unsigned long)RTC_MULF);

	pr_rtcdebug("%s: m=%llu, n=%llu, mul=%llu,mul1=%llu",
		__func__, rtc_m, rtc_n, rtc_mul, rtc_mul1);

	pr_rtcdebug("%s: slip=%llu, err_acc=%llu, slip_acc=%llu",
		__func__, rtc_slip, pRTC_TC->time_err_acc,
		pRTC_TC->slippage_acc);

	if ((rtc_slip < pRTC_TC->slippage_acc)
		&& (rtc_slip > pRTC_TC->time_err_acc)) {
		pRTC_TC->time_err_acc = 0;
		pRTC_TC->slippage_acc -= rtc_slip;
		pRTC_TC->flag_err_acc = pRTC_TC->flag_ratio_curr;
	} else {
		if (rtc_slip < pRTC_TC->slippage_acc)
			pRTC_TC->time_err_acc = pRTC_TC->slippage_acc
							- rtc_slip;
		else
			pRTC_TC->time_err_acc = 0;
		pRTC_TC->slippage_acc -= rtc_slip;
		pRTC_TC->flag_err_acc = pRTC_TC->flag_ratio_curr;
		pr_rtcdebug("%s:NO2", __func__);
	}

	pr_rtcdebug("%s: flag_acc=%d, time_acc=%llu",
		__func__, pRTC_TC->flag_err_acc, pRTC_TC->time_err_acc);

	return pRTC_TC->time_err_acc;

}


/*
 * API for calculating slippage amoung at the time of calling. return HZ val
 */
unsigned long rtc_cal_calc_slippage(void)
{
	u64 rtc_mul = 0, rtc_mul1 = 0, rtc_mul2 = 0, rtc_div = 0;
	u64 rtc_m = 0, rtc_n = 0;
	u64 dummy1 = 0, dummy2 = 0;
	u64 temp1 = 0, temp2 = 0;
	rtc_m = (u64) pRTC_TC->time_hz_curr;
	rtc_n = (u64) pRTC_TC->time_ratio_curr;

	pr_rtcdebug("%s:", __func__);

	mul64to128(&rtc_mul1, &rtc_mul, rtc_m, rtc_n);

	pRTC_TC->time_err_curr = rtc_mul;

	if (pRTC_TC->flag_err_acc == pRTC_TC->flag_ratio_curr) {
		add128(&temp2, &temp1, dummy1, pRTC_TC->time_err_acc,
			dummy2, pRTC_TC->time_err_curr);
		pRTC_TC->slippage_flag = pRTC_TC->flag_ratio_curr;
	} else if (pRTC_TC->flag_err_acc == true) {
		if (pRTC_TC->time_err_acc > pRTC_TC->time_err_curr) {
			sub128(&temp2, &temp1, dummy1, pRTC_TC->time_err_acc,
				dummy2, pRTC_TC->time_err_curr);
			pRTC_TC->slippage_flag = pRTC_TC->flag_err_acc;
		} else {
			sub128(&temp2, &temp1, dummy1, pRTC_TC->time_err_curr,
				dummy2, pRTC_TC->time_err_acc);
			pRTC_TC->slippage_flag = pRTC_TC->flag_err_curr;
		}
	} else if (pRTC_TC->flag_err_acc == false) {
		if (pRTC_TC->time_err_acc > pRTC_TC->time_err_curr) {
			sub128(&temp2, &temp1, dummy1, pRTC_TC->time_err_acc,
				dummy2, pRTC_TC->time_err_curr);
			pRTC_TC->slippage_flag = pRTC_TC->flag_err_acc;
		} else {
			sub128(&temp2, &temp1, dummy1, pRTC_TC->time_err_curr,
				dummy2, pRTC_TC->time_err_acc);
			pRTC_TC->slippage_flag = pRTC_TC->flag_err_curr;
		}
	} else {
		pr_rtcdebug("%s: condition check", __func__);
	}

	pRTC_TC->slippage_acc = temp1;

	mul64to128(&rtc_mul2, &rtc_mul1, temp1, (u64)RTC_MULF);
	rtc_div = div64(rtc_mul1, (unsigned long)RTCCAL_Ratio);

	pr_rtcdebug("%s: div=%llu, mul1=%llu", __func__, rtc_div, rtc_mul1);

	pRTC_TC->slippage_ms = (unsigned long)rtc_div;
	return rtc_div;
}


void rtc_cal_tc_savevals(void)
{
	u64 rtc_mul = 0, rtc_mul1 = 0;
	u64 rtc_m = 0, rtc_n = 0;
	u64 dummy1 = 0, dummy2 = 0;
	u64 temp1 = 0, temp2 = 0;
	rtc_m = (u64) pRTC_TC->time_hz_curr;
	rtc_n = (u64) pRTC_TC->time_ratio_curr;

	pr_rtcdebug("%s:", __func__);

	mul64to128(&rtc_mul1, &rtc_mul, rtc_m, rtc_n);
	pRTC_TC->time_err_curr = rtc_mul;

	if (pRTC_TC->flag_err_acc == pRTC_TC->flag_ratio_curr) {
		add128(&temp2, &temp1, dummy1, pRTC_TC->time_err_acc,
			dummy2, pRTC_TC->time_err_curr);
	} else if (pRTC_TC->flag_err_acc == true) {
		if (pRTC_TC->time_err_acc > pRTC_TC->time_err_curr) {
			sub128(&temp2, &temp1, dummy1, pRTC_TC->time_err_acc,
				dummy2, pRTC_TC->time_err_curr);
		} else {
			sub128(&temp2, &temp1, dummy1, pRTC_TC->time_err_curr,
				dummy2, pRTC_TC->time_err_acc);
			pRTC_TC->flag_err_acc = false;
		}
	} else if (pRTC_TC->flag_err_acc == false) {
		if (pRTC_TC->time_err_acc > pRTC_TC->time_err_curr) {
			sub128(&temp2, &temp1, dummy1, pRTC_TC->time_err_acc,
				dummy2, pRTC_TC->time_err_curr);
		} else {
			sub128(&temp2, &temp1, dummy1, pRTC_TC->time_err_curr,
				dummy2, pRTC_TC->time_err_acc);
			pRTC_TC->flag_err_acc = true;
		}
	} else {
		pr_rtcdebug("%s: condition check", __func__);
	}
	pRTC_TC->time_err_acc = temp1;
	pRTC_TC->slippage_acc = temp1;

	pRTC_TC->time_hz_acc += pRTC_TC->time_hz_curr;
	pRTC_TC->time_hz_curr = 0;
	pRTC_TC->slippage_flag = pRTC_TC->flag_err_acc;

}

void rtc_cal_tc_start(unsigned long time, u64 slippage, bool flag_acc)
{
	pr_rtcdebug("%s", __func__);

	rtc_jiffies = jiffies;

	pRTC_TC->time_hw_time = (u64)time;
	pRTC_TC->time_hz_acc = 0;
	pRTC_TC->time_hz_curr = 0;
	pRTC_TC->time_err_acc = slippage;
	pRTC_TC->flag_err_acc = flag_acc;
	pRTC_TC->time_err_curr = 0;
	pRTC_TC->time_hz_interval = 0;

	rtc_jiffies_prev = rtc_jiffies;

}


/*
 * API for updating time correction on RTC calibration internally.
 */
void rtc_cal_tc_update(void)
{
	bool rtc_update;
	unsigned long rtc_slippage;

	pr_rtcdebug("%s", __func__);

	rtc_update = false;

	rtc_jiffies = jiffies;

	if (rtc_jiffies_prev > rtc_jiffies)
		pRTC_TC->time_hz_interval = rtc_jiffies + 0xffffffff
							- rtc_jiffies_prev;
	else
		pRTC_TC->time_hz_interval = rtc_jiffies - rtc_jiffies_prev;

	rtccal_period += pRTC_TC->time_hz_interval;
	pRTC_TC->time_hz_curr += pRTC_TC->time_hz_interval;

	rtc_slippage = rtc_cal_calc_slippage();

	pr_rtcdebug("%s: slippage=%lu", __func__, rtc_slippage);

	if (rtc_slippage > RTCCAL_Slippage)
		rtc_update = true;

	if (rtc_correction == true)
		goto skip;

	if (rtc_update == TRUE) {
		unsigned long rtc_total_hz;
		unsigned long rtc_time_temp1, rtc_time_temp2;
		unsigned long rtc_time_delt;

		rtc_total_hz = pRTC_TC->time_hz_curr + pRTC_TC->time_hz_acc
			+ rtc_slippage * HZ / RTC_ONE_SEC ;
		rtc_time_temp1 = rtc_total_hz + HZ; /* HZ */
		rtc_time_temp2 = rtc_time_temp1 / HZ;/* sec */
		rtc_time_delt = rtc_time_temp1 - rtc_time_temp2 * HZ;

		pr_rtcdebug("%s:total=%lu,temp1=%lu,temp2=%lu,delt=%lu",
			__func__, rtc_total_hz, rtc_time_temp1,
			rtc_time_temp2, rtc_time_delt);

		rtccal_period = 0;
		rtc_correction = true;
		rtc_cal_correctiontimerstart(rtc_time_delt);
	}

	if ((RTCCAL_PeriodicUpdate) &&
		((rtccal_period/HZ) > RTCCAL_PeriodicUpdate)) {
		if (rtc_update == FALSE) {
			unsigned long rtc_total_hz;
			unsigned long rtc_time_temp1, rtc_time_temp2;
			unsigned long rtc_time_delt;

			rtc_total_hz = pRTC_TC->time_hz_curr
					+ pRTC_TC->time_hz_acc
					+ rtc_slippage * HZ / RTC_ONE_SEC ;
			rtc_time_temp1 = rtc_total_hz + HZ; /* HZ */
			rtc_time_temp2 = rtc_time_temp1 / HZ;/* sec */
			rtc_time_delt = rtc_time_temp1 - rtc_time_temp2 * HZ;

			pr_rtcdebug("%s:total=%lu,temp1=%lu,temp2=%lu,delt=%lu",
				__func__, rtc_total_hz, rtc_time_temp1,
				rtc_time_temp2, rtc_time_delt);

			rtc_cal_correctiontimerstart(rtc_time_delt);
			rtc_correction = true;
		}
		rtccal_period = 0;
	}

skip:
	rtc_jiffies_prev = rtc_jiffies;

}


/*
 * API for execute time correction into PMU device.
 */
void rtc_cal_tc_correction(void)
{
	unsigned long rtc_total_hz;
	unsigned long rtc_time_temp1, rtc_time_temp2;
	unsigned long rtc_time_delt;
	unsigned long rtc_slippage;

	pr_rtcdebug("%s", __func__);

	rtc_jiffies = jiffies;

	if (rtc_jiffies_prev > rtc_jiffies)
		pRTC_TC->time_hz_interval = rtc_jiffies + 0xffffffff
							- rtc_jiffies_prev;
	else
		pRTC_TC->time_hz_interval = rtc_jiffies - rtc_jiffies_prev;

	pRTC_TC->time_hz_curr += pRTC_TC->time_hz_interval;

	rtc_slippage = rtc_cal_calc_slippage();

	pr_rtcdebug("%s: slippage=%lu", __func__, rtc_slippage);

	rtc_total_hz = pRTC_TC->time_hz_curr + pRTC_TC->time_hz_acc
		+ rtc_slippage * HZ / RTC_ONE_SEC ;
	rtc_time_temp1 = rtc_total_hz + HZ; /* HZ */
	rtc_time_temp2 = rtc_time_temp1 / HZ;/* sec */
	rtc_time_delt = rtc_time_temp1 - rtc_time_temp2 * HZ;

	pRTC_TC->time_next = rtc_time_temp2 + pRTC_TC->time_hw_time;

	rtc_cal_msg_update_time(pRTC_TC->time_next);

	rtc_jiffies_prev = rtc_jiffies;

}


/*
 * API for handling ratio event. RPC on AP is calling this API
 * to forward ratio event
 */
void rtc_cal_tc_get_time(struct rtc_time *tm)
{
	unsigned long rtc_time_hz, temp_hz;
	struct rtctime_t time_rtc;
	unsigned long rtc_time;

	pr_rtcdebug("%s", __func__);

	if (rtccal_semaphore == NULL) {
		rtccal_semaphore = kzalloc(sizeof(struct semaphore),
			GFP_KERNEL);
		if (rtccal_semaphore == NULL) {
			pr_err("%s cannot create semaphore at %d\n",
				__func__, __LINE__);
		}
		sema_init((struct semaphore *)rtccal_semaphore, 1);
	}

	down((struct semaphore *)rtccal_semaphore);

	rtc_cal_tc_update();

	temp_hz = pRTC_TC->slippage_ms * HZ/RTC_ONE_SEC;
	if (pRTC_TC->slippage_flag)
		rtc_time_hz = pRTC_TC->time_hz_curr
					+ pRTC_TC->time_hz_acc - temp_hz;
	else
		rtc_time_hz = pRTC_TC->time_hz_curr
					+ pRTC_TC->time_hz_acc + temp_hz;
	rtc_time = pRTC_TC->time_hw_time;
	rtc_time += (rtc_time_hz / HZ);

	pr_rtcdebug(
		"%s:hw_time=%lu,hz_curr=%lu,hz_acc=%lu,slipp=%lu,flag=%d",
		__func__, pRTC_TC->time_hw_time, pRTC_TC->time_hz_curr,
		pRTC_TC->time_hz_acc, pRTC_TC->slippage_ms,
		pRTC_TC->slippage_flag);

	rtc_util_convert_sec2rtctime_t(&time_rtc, rtc_time);
	rtc_util_convert_rtctime_t2tm(tm, &time_rtc);

	up((struct semaphore *)rtccal_semaphore);

}


/*
 * API for handling ratio event. RPC on AP is calling this API
 * to forward ratio event
 */
int rtc_cal_tc_set_time(struct rtc_time *tm)
{
	int ret;
	struct bcmpmu_rtc *rdata;

	pr_rtcdebug("%s", __func__);

	rtc_correction = false;
	rdata = rtc_cal_bcmpmu;

	if (rdata == NULL)
		return -1;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCYR,
				tm->tm_year - 100, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCMT,
				tm->tm_mon + 1, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCDT,
				tm->tm_mday, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCHR,
				tm->tm_hour, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCMN,
				tm->tm_min, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCSC,
				tm->tm_sec, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

err:
	pRTC_TC->err_write = ret;
	if (ret)
		memcpy(&pRTC_TC->tm_write, &tm, sizeof(tm));

	return ret;

}


/*
 * API for handling ratio event. RPC on AP is calling this API
 * to forward ratio event
 */
static void rtc_cal_task(void)
{

	struct rtc_cal_msg_t	msg;
	struct rtctime_t time_rtc;
	unsigned long time_sec = 0;
	u64 rtc_slip_prev;
	bool rtc_slip_flag;
	struct rtc_time tm;
	unsigned int len;

	if (rtccal_semaphore == NULL) {
		rtccal_semaphore = kzalloc(sizeof(struct semaphore),
			GFP_KERNEL);
		if (rtccal_semaphore == NULL)
			pr_err("%s cannot create semaphore at %d\n",
				__func__, __LINE__);

		sema_init((struct semaphore *)rtccal_semaphore, 1);
	}

	while (TRUE) {
		memset(&msg, 0, sizeof(struct rtc_cal_msg_t));
		wait_event_interruptible(rtccal_q,
			!kfifo_is_empty(&rtccal_kfifo));

		len = kfifo_out_locked(&rtccal_kfifo, (unsigned char *)&msg,
			sizeof(struct rtc_cal_msg_t), &rtccal_flock);
		if (len != sizeof(struct rtc_cal_msg_t))
			pr_err("%s len=%u", __func__, len);

		pr_rtcdebug("rtc_cal_task : msg.id = %u,%u",
			(unsigned int)msg.id, len);

		down((struct semaphore *)rtccal_semaphore);

		switch (msg.id) {
		case RTCCAL_ID_SETTIME: /* set time access from upper layer */
			pr_rtcdebug("RTCCAL_ID_SETTIME : msg.val1=%lu",
				msg.val1);
			rtc_cal_correctiontimerstop();
			rtc_cal_updatetimerstop();
			rtc_cal_tc_start(msg.val1, RTC_RESET_SLIPPAGE, 0);
			rtc_cal_updatetimerstart();
			break;

		case RTCCAL_ID_CALEVENT: /* Cal event from CP */
			pr_rtcdebug("RTCCAL_ID_CALEVENT : val1=%lu,val2=%lu",
				msg.val1, msg.val2);
			rtc_cal_tc_update();
			rtc_cal_tc_savevals();
			bcm_rtc_cal_timerratio(msg.val1,
				(unsigned int)(msg.val2));
			rtc_cal_updatetimerstart();
			break;

		case RTCCAL_ID_UPDATETIME:
			pr_rtcdebug("RTCCAL_ID_UPDATETIME : time=%lu",
				msg.val1);
			rtc_cal_tc_update();
			rtc_slip_prev = rtc_cal_tc_updatevals(msg.val1);
			rtc_slip_flag = pRTC_TC->flag_err_acc;
			rtc_util_convert_sec2rtctime_t(&time_rtc, msg.val1);
			rtc_util_convert_rtctime_t2tm(&tm, &time_rtc);
			rtc_cal_tc_set_time(&tm);
			rtc_cal_tc_start(msg.val1, rtc_slip_prev,
				rtc_slip_flag);
			rtc_cal_updatetimerstart();
			break;

		case RTCCAL_ID_TIMERCB:
			pr_rtcdebug("RTCCAL_ID_TIMERCB : val1=%lu",
				msg.val1);
			rtc_cal_tc_update();
			break;

		case RTCCAL_ID_TIMECORR:
			pr_rtcdebug("RTCCAL_ID_TIMECORR");
			rtc_cal_tc_correction();
			rtc_cal_correctiontimerstop();
			break;

		case RTCCAL_ID_GETTIME: /* read and set initial time */
			pr_rtcdebug("RTCCAL_ID_GETTIME");
			rtc_cal_get_read_rtctime(&tm);
			rtc_util_convert_tm2rtctime_t(&time_rtc, &tm);
			time_sec = rtc_util_convert_rtctime_t2sec(&time_rtc);
			pr_rtcdebug("RTCCAL_ID_GETTIME : time_sec=%lu",
				time_sec);
			rtc_cal_tc_start(time_sec, RTC_RESET_SLIPPAGE, 0);
			rtc_cal_updatetimerstart();
			rtc_util_convert_rtctime_t2tm(&tm, &time_rtc);
			rtc_cal_tc_set_time(&tm);
			break;

		default:
			pr_err("No ID found!!");
			break;
			}
		rtc_cal_pr_vars();

		up((struct semaphore *)rtccal_semaphore);
		schedule();
	}
}


/**
	This function is used to init task queue

	@note
	@return		Boolean

**/
bool rtc_cal_init(void)
{
	bool status = TRUE;
	int ret;
	spin_lock_init(&rtccal_flock);
	init_waitqueue_head(&rtccal_q);

	ret = kfifo_alloc(&rtccal_kfifo,
		RTC_QUEUESIZE * sizeof(struct rtc_cal_msg_t), GFP_KERNEL);

	if (ret) {
		pr_err("%s ret=%d", __func__, ret);
		status = FALSE;
	}

	/* Load  parameter for setting RTC Calibration. */
	RTCCAL_Ratio = rtccal_data.nom_ratio;
	RTCCAL_Ratio_min = rtccal_data.min_ratio;
	RTCCAL_Ratio_max = rtccal_data.max_ratio;
	RTCCAL_Range = rtccal_data.range_ppm;
	RTCCAL_Threshold = rtccal_data.cal_thold;
	RTCCAL_Int_Measure = rtccal_data.cal_meas_int;
	RTCCAL_Int_Calc = rtccal_data.calc_interval;
	RTCCAL_Slippage = rtccal_data.slippage_err;
	RTCCAL_PeriodicUpdate = rtccal_data.update_interval;
	RTCCAL_Method = rtccal_data.method;

	pr_rtcdebug(
		"rtc_cal_init : RTCCAL_Ratio=%lu", RTCCAL_Ratio);
	pr_rtcdebug(
		"rtc_cal_init : RTCCAL_Ratio_min=%lu", RTCCAL_Ratio_min);
	pr_rtcdebug(
		"rtc_cal_init : RTCCAL_Ratio_max=%lu", RTCCAL_Ratio_max);
	pr_rtcdebug(
		"rtc_cal_init : RTCCAL_Range=%lu", RTCCAL_Range);
	pr_rtcdebug(
		"rtc_cal_init : RTCCAL_Threshold=%lu", RTCCAL_Threshold);
	pr_rtcdebug(
		"rtc_cal_init : RTCCAL_Int_Measure=%lu", RTCCAL_Int_Measure);
	pr_rtcdebug(
		"rtc_cal_init : RTCCAL_Int_Calc=%lu", RTCCAL_Int_Calc);
	pr_rtcdebug(
		"rtc_cal_init : RTCCAL_Slippage=%lu", RTCCAL_Slippage);
	pr_rtcdebug(
		"rtc_cal_init : RTCCAL_PeriodicUpdate=%lu",
		RTCCAL_PeriodicUpdate);

	memset(&sRTC_TC, 0, sizeof(struct rtc_cal_TC_t));
	pRTC_TC = &sRTC_TC;

	if (RTCCAL_PeriodicUpdate)
		RTCCAL_PeriodicUpdate *= RTC_PERIODICUPDATE;

	if (IsRTCInit == FALSE)
		IsRTCInit = TRUE;
	return status;
}


/*
 * API for handling ratio event. RPC on AP is calling this API
 * to forward ratio event
 */
void rtc_cal_run(void)
{
	if (IsRTCRun) {
		pr_info("rtc_cal_run : already started");
		return;
	}

	pr_rtcdebug("%s", __func__);

	rtccal_workqueue = create_workqueue("RTCCAL");
	INIT_WORK(&rtccal_work, (work_func_t)rtc_cal_task);
	queue_work(rtccal_workqueue, &rtccal_work);

	IsRTCRun = true;

	rtc_cal_msg_get_rtctime();

}

/*
 * API for handling ratio event. RPC on AP is calling this API
 * to forward ratio event
 */
void rtc_cal_shutdown(void)
{
	pr_rtcdebug("%s", __func__);

	rtc_cal_updatetimerstop();

	if (IsRTCRun) {
		IsRTCRun = false;
		cancel_work_sync(&rtccal_work);
		flush_workqueue(rtccal_workqueue);
		destroy_workqueue(rtccal_workqueue);
		rtccal_workqueue = NULL;

		kfifo_free(&rtccal_kfifo);

		pRTC_TC = NULL;
	}
}





/*
 * API for handling ratio event. RPC on AP is calling this API
 * to forward ratio event
 */
void bcm_rtc_cal_ratio(unsigned long val1, unsigned long val2)
{
	pr_rtcdebug("%s", __func__);

	if (rtc_cal_debug)
		return;

	rtc_cal_cp_ratio = val1;

	rtc_cal_msg_ratio_event(val1, (unsigned int) val2);
}



/*
 * API for setting time to RTC calibration component.
 * Broadcom RTC device driver needs to call this API
 * for RTC calibration feature.
 */
int bcm_rtc_cal_set_time(struct bcmpmu_rtc *rdata, struct rtc_time *tm)
{
	int ret;

	unsigned long rtc_sec;
	struct rtctime_t time_rtc;

	pr_rtcdebug("%s", __func__);

	rtc_util_convert_tm2rtctime_t(&time_rtc, tm);
	rtc_sec = rtc_util_convert_rtctime_t2sec(&time_rtc);
	rtc_cal_msg_set_time(rtc_sec);

	rtc_cal_bcmpmu = rdata;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCYR,
				tm->tm_year - 100, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCMT,
				tm->tm_mon + 1, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCDT,
				tm->tm_mday, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCHR,
				tm->tm_hour, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCMN,
				tm->tm_min, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCSC,
				tm->tm_sec, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

err:
	pRTC_TC->err_write = ret;
	if (ret)
		memcpy(&pRTC_TC->tm_write, &tm, sizeof(tm));

	return ret;

}


/*
 * API for reading time from RTC calibration component.
 * Broadcom RTC device driver needs to call this API
 * for RTC calibration feature.
 */
int bcm_rtc_cal_read_time(struct bcmpmu_rtc *rdata, struct rtc_time *tm)
{
	int ret = 0;

	pr_rtcdebug("%s", __func__);

	rtc_cal_bcmpmu = rdata;

	if (rdata == NULL || !rtc_cal_isrun()) {
		pr_rtcdebug("%s: not ready %d", __func__, rtc_cal_isrun());
		rtc_cal_get_read_rtctime(tm);
	} else
		rtc_cal_tc_get_time(tm);

	return ret;
}

#if defined(CONFIG_RTC_DRV_BCM59056)

/*
 * API for setting register value of rtc calibration in the PMU device.
 * HW dependent API.
 */
bool bcm_rtc_cal_reg_set(unsigned int *data)
{
	bool ret = false;

	return ret;
}

/*
 * API for reading register value of rtc calibraiton in the PMU device.
 * HW dependent API.
 */
bool bcm_rtc_cal_reg_read(void)
{
	bool ret = false;

	return ret;
}

/*
 * API for disabling/enabling rtc calibration feature in the PMU device.
 * HW dependent API.
 */
bool bcm_rtc_cal_enable(bool enable)
{
	bool ret = false;

	return ret;
}

#endif /* CONFIG_RTC_DRV_BCM59056 */


#ifdef CONFIG_DEBUG_FS


/*
 * debug FS for reading parameters
 */
static ssize_t rtc_cal_parameter_get(struct file *file, char __user *user_buf,
				size_t count, loff_t *ppos)
{
	char buf[256];
	unsigned long len = 0;

	len += snprintf(buf+len, sizeof(buf)-len,
			"device = %s\n", rtccal_data.device);
	len += snprintf(buf+len, sizeof(buf)-len,
			"nom_ratio = %lu\n", RTCCAL_Ratio);
	len += snprintf(buf+len, sizeof(buf)-len,
			"min_ratio = %lu\n", RTCCAL_Ratio_min);
	len += snprintf(buf+len, sizeof(buf)-len,
			"max_ratio = %lu\n", RTCCAL_Ratio_max);
	len += snprintf(buf+len, sizeof(buf)-len,
			"range_ppm = %lu\n", RTCCAL_Range);
	len += snprintf(buf+len, sizeof(buf)-len,
			"cal_thold = %lu\n", RTCCAL_Threshold);
	len += snprintf(buf+len, sizeof(buf)-len,
			"cal_meas_int = %lu\n", RTCCAL_Int_Measure);
	len += snprintf(buf+len, sizeof(buf)-len,
			"calc_interval = %lu\n", RTCCAL_Int_Calc);
	len += snprintf(buf+len, sizeof(buf)-len,
			"update_interval = %lu\n", RTCCAL_PeriodicUpdate);
	len += snprintf(buf+len, sizeof(buf)-len,
			"slippage_err = %lu\n", RTCCAL_Slippage);
	len += snprintf(buf+len, sizeof(buf)-len,
			"method = %lu\n", RTCCAL_Method);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static const struct file_operations rtc_cal_parameter_fops = {
	.read =         rtc_cal_parameter_get,
};

/*
 * debug FS for reading rtc time from rtc calibratino component.
 */
static ssize_t rtc_cal_info_get(struct file *file, char __user *user_buf,
				size_t count, loff_t *ppos)
{
	char buf[512];
	unsigned long len = 0;

	if (rtc_cal_bcmpmu == NULL) {
		len += snprintf(buf+len, sizeof(buf)-len,
				"\nDeviec not ready\n");

		goto err;
	}
	rtc_cal_tc_update();

	len += snprintf(buf+len, sizeof(buf)-len,
			"\nInternal variables\n");
	len += snprintf(buf+len, sizeof(buf)-len,
			"\ttime_next = %lu\n", pRTC_TC->time_next);
	len += snprintf(buf+len, sizeof(buf)-len,
			"\tslippage_acc = %llu\n", pRTC_TC->slippage_acc);
	len += snprintf(buf+len, sizeof(buf)-len,
			"\ttime_hw_time = %lu\n", pRTC_TC->time_hw_time);
	len += snprintf(buf+len, sizeof(buf)-len,
			"\ttime_hz_acc = %lu\n", pRTC_TC->time_hz_acc);
	len += snprintf(buf+len, sizeof(buf)-len,
			"\ttime_err_acc = %llu\n", pRTC_TC->time_err_acc);
	len += snprintf(buf+len, sizeof(buf)-len,
			"\tflag_err_acc = %d\n", pRTC_TC->flag_err_acc);
	len += snprintf(buf+len, sizeof(buf)-len,
			"\ttime_hz_curr = %lu\n", pRTC_TC->time_hz_curr);
	len += snprintf(buf+len, sizeof(buf)-len,
			"\ttime_hz_prev = %lu\n", pRTC_TC->time_hz_prev);
	len += snprintf(buf+len, sizeof(buf)-len,
			"\ttime_hz_interval = %lu\n",
			pRTC_TC->time_hz_interval);
	len += snprintf(buf+len, sizeof(buf)-len,
			"\ttime_err_curr = %llu\n", pRTC_TC->time_err_curr);
	len += snprintf(buf+len, sizeof(buf)-len,
			"\tflag_err_curr = %d\n", pRTC_TC->flag_err_curr);
	len += snprintf(buf+len, sizeof(buf)-len,
			"\ttime_ratio_curr = %lu\n", pRTC_TC->time_ratio_curr);
	len += snprintf(buf+len, sizeof(buf)-len,
			"\tflag_ratio_curr = %d\n", pRTC_TC->flag_ratio_curr);
	len += snprintf(buf+len, sizeof(buf)-len,
			"\ttime_hz_delt = %lu\n", pRTC_TC->time_hz_delt);
	len += snprintf(buf+len, sizeof(buf)-len,
			"\tslippage_ms = %lu\n", pRTC_TC->slippage_ms);
	len += snprintf(buf+len, sizeof(buf)-len,
			"\tslippage_flag = %d\n", pRTC_TC->slippage_flag);
	len += snprintf(buf+len, sizeof(buf)-len,
			"\terr_read1/2 = %d/%d\t", pRTC_TC->err_read1,
			pRTC_TC->err_read2);
	len += snprintf(buf+len, sizeof(buf)-len,
			"%d/", pRTC_TC->tm_read.tm_year);
	len += snprintf(buf+len, sizeof(buf)-len,
			"%d/", pRTC_TC->tm_read.tm_mon);
	len += snprintf(buf+len, sizeof(buf)-len,
			"%d/", pRTC_TC->tm_read.tm_mday);
	len += snprintf(buf+len, sizeof(buf)-len,
			"%d/", pRTC_TC->tm_read.tm_hour);
	len += snprintf(buf+len, sizeof(buf)-len,
			"%d/", pRTC_TC->tm_read.tm_min);
	len += snprintf(buf+len, sizeof(buf)-len,
			"%d\n", pRTC_TC->tm_read.tm_sec);
	len += snprintf(buf+len, sizeof(buf)-len,
			"\terr_write = %d\t", pRTC_TC->err_write);
	len += snprintf(buf+len, sizeof(buf)-len,
			"%d/", pRTC_TC->tm_write.tm_year);
	len += snprintf(buf+len, sizeof(buf)-len,
			"%d/", pRTC_TC->tm_write.tm_mon);
	len += snprintf(buf+len, sizeof(buf)-len,
			"%d/", pRTC_TC->tm_write.tm_mday);
	len += snprintf(buf+len, sizeof(buf)-len,
			"%d/", pRTC_TC->tm_write.tm_hour);
	len += snprintf(buf+len, sizeof(buf)-len,
			"%d/", pRTC_TC->tm_write.tm_min);
	len += snprintf(buf+len, sizeof(buf)-len,
			"%d\n\n", pRTC_TC->tm_write.tm_sec);

err:
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static const struct file_operations rtc_cal_info_fops = {
	.read =         rtc_cal_info_get,
};

static ssize_t rtc_cal_read_time_get(struct file *file, char __user *user_buf,
				size_t count, loff_t *ppos)
{
	char buf[256];
	unsigned long len = 0;
	struct rtc_time tm;

	if (rtc_cal_bcmpmu == NULL) {
		len += snprintf(buf+len, sizeof(buf)-len,
				"\nDeviec not ready\n");

		goto err;
	}

	bcm_rtc_cal_read_time(rtc_cal_bcmpmu, &tm);

	len += snprintf(buf+len, sizeof(buf)-len,
			"\nFrom RTC Cal\n");
	len += snprintf(buf+len, sizeof(buf)-len,
			"\tyear = %d, ", tm.tm_year);
	len += snprintf(buf+len, sizeof(buf)-len,
			"mon = %d, ", tm.tm_mon);
	len += snprintf(buf+len, sizeof(buf)-len,
			"mday = %d, ", tm.tm_mday);
	len += snprintf(buf+len, sizeof(buf)-len,
			"hour = %d, ", tm.tm_hour);
	len += snprintf(buf+len, sizeof(buf)-len,
			"min = %d, ", tm.tm_min);
	len += snprintf(buf+len, sizeof(buf)-len,
			"sec = %d\n", tm.tm_sec);

	len += snprintf(buf+len, sizeof(buf)-len,
			"\tcurrent ratio = %lu, ", pRTC_TC->time_ratio_curr);

	len += snprintf(buf+len, sizeof(buf)-len,
			"slippage = %lums\n", pRTC_TC->slippage_ms);

err:
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static const struct file_operations rtc_cal_read_time_fops = {
	.read =         rtc_cal_read_time_get,
};

/*
 * debug FS for reading current ratio value.
 */
static ssize_t rtc_cal_test_get(struct file *file, char __user *user_buf,
				size_t count, loff_t *ppos)
{
	unsigned long rtc_cal_ratio;
	char buf[256];
	unsigned long len = 0;

	if (rtc_cal_debug)
		rtc_cal_ratio = rtc_cal_test_ratio;
	else
		rtc_cal_ratio = rtc_cal_cp_ratio;

	rtc_cal_msg_ratio_event(rtc_cal_ratio, 0);

	len += snprintf(buf+len, sizeof(buf)-len,
			"debug = %d\n", rtc_cal_debug);

	len += snprintf(buf+len, sizeof(buf)-len,
			"ratio_event : ratio = %lu\n", rtc_cal_ratio);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static const struct file_operations rtc_cal_test_fops = {
	.read =         rtc_cal_test_get,
};

/*
 * debug FS for reading current ratio value.
 */
static ssize_t rtc_cal_test_get1(struct file *file, char __user *user_buf,
				size_t count, loff_t *ppos)
{
	unsigned long rtc_cal_ratio;
	char buf[256];
	unsigned long len = 0;

	rtc_cal_ratio = 8663200;

	rtc_cal_msg_ratio_event(rtc_cal_ratio, 0);

	len += snprintf(buf+len, sizeof(buf)-len,
			"ratio_event : ratio = %lu\n", rtc_cal_ratio);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static const struct file_operations rtc_cal_test_fops1 = {
	.read =         rtc_cal_test_get1,
};

/*
 * debug FS for reading current ratio value.
 */
static ssize_t rtc_cal_test_get2(struct file *file, char __user *user_buf,
				size_t count, loff_t *ppos)
{
	unsigned long rtc_cal_ratio;
	char buf[256];
	unsigned long len = 0;

	rtc_cal_ratio = 8670134;

	rtc_cal_msg_ratio_event(rtc_cal_ratio, 0);

	len += snprintf(buf+len, sizeof(buf)-len,
			"ratio_event : ratio = %lu\n", rtc_cal_ratio);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static const struct file_operations rtc_cal_test_fops2 = {
	.read =         rtc_cal_test_get2,
};

static int rtc_cal_setratio(void *data, u64 val)
{
	bcm_rtc_cal_ratio((u32)val, 0);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(rtc_cal_ratio, NULL,
	rtc_cal_setratio, "%llu\n");

static struct dentry *dent_rtc_cal_root_dir;

/*
 * debug FS initialization.
 */
int __init bcm_rtc_cal_debug_init(void)
{
	dent_rtc_cal_root_dir = debugfs_create_dir("rtc_cal", 0);

	if (!dent_rtc_cal_root_dir)
		return -ENOMEM;

	if (!debugfs_create_u32("debug", S_IRUSR|S_IWUSR,
		dent_rtc_cal_root_dir, &rtc_cal_debug))
		return -ENOMEM;

	if (!debugfs_create_file("parameters", S_IRUSR,
		dent_rtc_cal_root_dir, NULL, &rtc_cal_parameter_fops))
		return -ENOMEM;

	if (!debugfs_create_file("read_time", S_IRUSR,
		dent_rtc_cal_root_dir, NULL, &rtc_cal_read_time_fops))
		return -ENOMEM;

	if (!debugfs_create_file("info", S_IRUSR,
		dent_rtc_cal_root_dir, NULL, &rtc_cal_info_fops))
		return -ENOMEM;

	if (!debugfs_create_file("test", S_IRUSR,
		dent_rtc_cal_root_dir, NULL, &rtc_cal_test_fops))
		return -ENOMEM;

	if (!debugfs_create_file("test_min", S_IRUSR,
		dent_rtc_cal_root_dir, NULL, &rtc_cal_test_fops1))
		return -ENOMEM;

	if (!debugfs_create_file("test_max", S_IRUSR,
		dent_rtc_cal_root_dir, NULL, &rtc_cal_test_fops2))
		return -ENOMEM;

	if (!debugfs_create_u32("ratio", S_IRUSR|S_IWUSR,
		dent_rtc_cal_root_dir, &rtc_cal_test_ratio))
		return -ENOMEM;

	if (!debugfs_create_u32("cp_ratio", S_IRUSR,
		dent_rtc_cal_root_dir, &rtc_cal_cp_ratio))
		return -ENOMEM;

	if (!debugfs_create_file("ratio_event", S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH,
		dent_rtc_cal_root_dir, NULL, &rtc_cal_ratio))
			return -ENOMEM;

	return 0;

}


#endif


/*
 * API for initializing rtc calibration component.
 * Broadcom RTC device driver needs to call this API
 * when initialization device driver.
 */
void bcm_rtc_cal_init(struct bcmpmu_rtc *rdata)
{

	pr_rtcdebug("%s: HZ=%d", __func__, (int)HZ);

	rtc_cal_bcmpmu = rdata;

	/* start RPC */
	RTCSC_InitRpc();

	/* init parameters */
	rtc_cal_init();

	bcm_rtc_cal_debug_init();

	rtc_cal_run();

}

/*
 * API for shutting down rtc caliration component.
 * Broadcom RTC device driver needs to call this API
 * when shutdown device driver.
 */
void bcm_rtc_cal_shutdown(void)
{
	pr_rtcdebug("%s", __func__);

	rtc_cal_shutdown();
	rtc_cal_bcmpmu = NULL;
}



