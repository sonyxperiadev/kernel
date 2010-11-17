#ifndef	_BCM2835_IR_REMOTE_H_
#define	_BCM2835_IR_REMOTE_H_

typedef struct {
        unsigned int	type;
        unsigned int	code;
} bcm2835_ir_remote_event_t;


#define MAX_BCM2835_IR_REMOTE_IOCTL_CMD_SIZE	24

typedef enum {
        IOCTL_IR_REMOTE_GET_EV_ID  = 0x1,
} ir_remote_ioctl_id;


#define IOCTL_IR_REMOTE_GET_EV  _IOR('S', IOCTL_IR_REMOTE_GET_EV_ID, bcm2835_ir_remote_event_t)

#endif /* _BCM2835_IR_REMOTE_H_ */	
