#ifndef __SMX3_TOUCH_LOG__
#define __SMX3_TOUCH_LOG__

#define TP_LOG_TAG "NVT-ts"

#define TP_LOG_LEVEL_ERROR		1
#define TP_LOG_LEVEL_WARRING	2
#define TP_LOG_LEVEL_INFO		4
#define TP_LOG_LEVEL_DEBUG		8
#define TP_LOG_LEVEL_TOUCH		16
#define TP_LOG_LEVEL_MAX	TP_LOG_LEVEL_TOUCH

extern char tp_probe_log[1024];
extern int tp_probe_log_index;

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
