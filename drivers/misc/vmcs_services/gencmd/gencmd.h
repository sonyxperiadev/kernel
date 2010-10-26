#ifndef _GENCMD_H
#define _GENCMD_H
#define GENCMD_DEBUG

#ifdef GENCMD_DEBUG
#define refmt(fmt) "[%s]: " fmt, __func__
#define gencmd_print(fmt, ...) \
	printk(KERN_ERR refmt(fmt), ##__VA_ARGS__)
#else
#define gencmd_print(fmt, ...)
#endif

int vc_gencmd(char *response, int maxlen, const char *format, ...);
#endif
