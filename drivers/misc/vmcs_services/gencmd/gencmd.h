#ifndef _GENCMD_H
#define _GENCMD_H

//#define GENCMD_DEBUG

#ifdef GENCMD_DEBUG

#ifdef __KERNEL__
#define refmt(fmt) "[%s]: " fmt, __func__
#define gencmd_print(fmt, ...) \
	printk(KERN_ERR refmt(fmt), ##__VA_ARGS__)
#else
#define refmt(fmt) "[%s]: " fmt, __func__
#define gencmd_print(fmt, ...) \
    printf(refmt(fmt), ##__VA_ARGS__)
#endif

#else
#define gencmd_print(fmt, ...)
#endif

#define GENCMD_CMD_SIZE		512
#define GENCMD_RESP_SIZE	512

typedef enum
{
    GENCMD_IOCTL_SEND_RESP_ID = 0x81,

}GENCMD_COMMAND_ID_T;

#define GENCMD_IOCTL_CMD_SEND_RESP  _IOWR('G', GENCMD_IOCTL_SEND_RESP_ID, \
        GENCMD_IOCTL_SEND_RESP_T )

typedef struct {
    char *cmd;
    int cmd_len;
    char *resp;
    int resp_len;
} GENCMD_IOCTL_SEND_RESP_T;

int vc_gencmd(char *response, int maxlen, const char *format, ...);
#endif
