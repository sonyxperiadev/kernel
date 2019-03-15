#ifndef __SMX3_TOUCH_LOG__
#define __SMX3_TOUCH_LOG__

#define TP_LOG_TAG "NVT-ts"

#define TP_DYNAMIC_LOG

#define TP_LOG_LEVEL_ERROR		1
#define TP_LOG_LEVEL_WARRING	2
#define TP_LOG_LEVEL_INFO		4
#define TP_LOG_LEVEL_DEBUG		8
#define TP_LOG_LEVEL_TOUCH		16
#define TP_LOG_LEVEL_MAX	TP_LOG_LEVEL_TOUCH

extern char tp_probe_log[1024];
extern int tp_probe_log_index;

#ifdef TP_DYNAMIC_LOG
extern int tp_smx3_dynamic_log_level;

#define tp_dynamic_print(level, fmt, arg...) \
	do { \
		if (level <= tp_smx3_dynamic_log_level) \
			printk(fmt, ##arg); \
	} while (0)
#else
#define tp_dynamic_print(level, fmt, arg...) \
	printk(fmt, ##arg)
#endif /* TP_DYNAMIC_LOG */

#define TP_LOGE(fmt, arg...) \
	tp_dynamic_print(TP_LOG_LEVEL_ERROR, \
		"[%s][error] %s %d: "fmt"\n", \
		TP_LOG_TAG, __func__, __LINE__, ##arg)
#define TP_LOGW(fmt, arg...) \
	tp_dynamic_print(TP_LOG_LEVEL_WARRING, \
		"[%s][warring] %s %d: "fmt"\n", \
		TP_LOG_TAG, __func__, __LINE__, ##arg)
#define TP_LOGI(fmt, arg...) \
	tp_dynamic_print(TP_LOG_LEVEL_INFO, \
		"[%s][info] %s %d: "fmt"\n", \
		TP_LOG_TAG, __func__, __LINE__, ##arg)
#define TP_LOGD(fmt, arg...) \
	tp_dynamic_print(TP_LOG_LEVEL_DEBUG, \
		"[%s][debug] %s %d: "fmt"\n", \
		TP_LOG_TAG, __func__, __LINE__, ##arg)
#define TP_LOGT(fmt, arg...) \
	tp_dynamic_print(TP_LOG_LEVEL_TOUCH, \
		"[%s][touch] %s %d: "fmt"\n", \
		TP_LOG_TAG, __func__, __LINE__, ##arg)

/***** self-test log *****/
#define TP_TEST_LOG_TAG "fts-test"

#define tp_test_print(fmt, arg...) \
	printk(fmt, ##arg)

#define TP_TEST_LOGE(fmt, arg...) \
	tp_test_print("[%s][error] %s: "fmt, TP_TEST_LOG_TAG, __func__, ##arg)
#define TP_TEST_LOGW(fmt, arg...) \
	tp_test_print("[%s][warring] %s: "fmt, TP_TEST_LOG_TAG, __func__, ##arg)
#define TP_TEST_LOGI(fmt, arg...) \
	tp_test_print("[%s][info] %s: "fmt, TP_TEST_LOG_TAG, __func__, ##arg)
#define TP_TEST_LOGD(fmt, arg...) \
	tp_test_print("[%s][debug] %s: "fmt, TP_TEST_LOG_TAG, __func__, ##arg)

#endif /* __SMX3_TOUCH_LOG__ */
