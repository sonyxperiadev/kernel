#ifndef _TV_INTF_H
#define _TV_INTF_H

#define TV_INTF_DEBUG

#ifdef TV_INTF_DEBUG

#ifdef __KERNEL__
#define refmt(fmt) "[%s]: " fmt, __func__
#define tv_intf_print(fmt, ...) \
	printk(KERN_ERR refmt(fmt), ##__VA_ARGS__)
#else
#define refmt(fmt) "[%s]: " fmt, __func__
#define tv_intf_print(fmt, ...) \
    printf(refmt(fmt), ##__VA_ARGS__)
#endif

#else
#define tv_intf_print(fmt, ...)
#endif

#define TV_INTF_CMD_SIZE		512
#define TV_INTF_RESP_SIZE	512

typedef enum
{
    TV_INTF_IOCTL_SEND_RESP_ID = 0x81,

}TV_INTF_COMMAND_ID_T;

#define TV_INTF_IOCTL_CMD_SEND_RESP  _IOWR('G', TV_INTF_IOCTL_SEND_RESP_ID, \
        TV_INTF_IOCTL_SEND_RESP_T )

typedef struct {
    char *cmd;
    int cmd_len;
    char *resp;
    int resp_len;
} TV_INTF_IOCTL_SEND_RESP_T;

int bcm2835_tv_intf(char *response, int maxlen, const char *format, ...);
#endif
