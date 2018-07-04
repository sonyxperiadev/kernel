/*
 * siw_touch_hal_abt.c - SiW touch hal driver for ABT
 *
 * Copyright (C) 2016 Silicon Works - http://www.siliconworks.co.kr
 * Copyright (C) 2018 Sony Mobile Communications Inc.
 * Author: Hyunho Kim <kimhh@siliconworks.co.kr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */

#include "siw_touch_cfg.h"

#if defined(__SIW_SUPPORT_ABT)	//See siw_touch_cfg.h

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kthread.h>
#include <linux/sched.h>

/* */
#include <linux/signal.h>
#include <linux/netdevice.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/inet.h>
#include <linux/socket.h>
#include <linux/net.h>
#include <net/sock.h>
#include <net/tcp.h>

#include "siw_touch.h"
#include "siw_touch_hal.h"
#include "siw_touch_bus.h"
#include "siw_touch_event.h"
#include "siw_touch_gpio.h"
#include "siw_touch_irq.h"
#include "siw_touch_sys.h"

/*
 * [ABT]
 * LG Touch Solution has ABT function to monitor real time operation of touch IC
 * and SiW touch driver supports network communication via LAN(or WiFi) for ABT.
 *
 * [Usage]
 * (In Windows)
 * 1. Run LG Touch Solution
 *    Setting > General > Connection Mode > WiFi [not WiFi(ABT)]
 *    Connect > {IP} : Select the IP of your PC
 * (In Target System)
 * 2. Turn on debug tool
 *    $ echo 1 > /sys/devices/virtual/input/{input name}/debug_tool
 *    {Input Name} : default is "siw_touch_input"(SIW_TOUCH_INPUT)
 *                   and can be changed via chip_idrv_name
 * 3. Setup network connection
 *    $ echo 2 {IP} > /sys/devices/virtual/input/{input name}/raw_report
 *    {IP} : the IP of your PC
 *
 */

#define DEFAULT_IP		"127.0.0.1"
#define NONE_IP			"0.0.0.0"

#define MAX_REPORT_SLOT					16
#define P_CONTOUR_POINT_MAX				8
#define DEF_RNDCPY_EVERY_NTH_FRAME		(4)
#define PACKET_SIZE						128

enum {
	DEFAULT_PORT = 8095,
	TS_TCP_PORT = 8097,
	SEND_PORT = 8090,
	OMK_BUF = 1000,
};

enum E_DATA_TYPE {
	DATA_TYPE_RAW = 0,
	DATA_TYPE_BASELINE,
	/* */
	DATA_TYPE_RN_ORG = 10,
	DATA_TYPE_SEG1 = 20,
	DATA_TYPE_SEG2,
	DATA_TYPE_MAX
};

enum DEBUG_TYPE {
	DEBUG_DATA = 0,
	DEBUG_MODE,
	DEBUG_DATA_RW_MODE,
	DEBUG_DATA_CAPTURE_MODE,
	DEBUG_DATA_CMD_MODE,
};

enum RW_TYPE {
	READ_TYPE = 55,
	WRITE_TYPE = 66,
};

enum DEBUG_REPORT_CMD {
	DEBUG_REPORT_POINT = 0x100,
	DEBUG_REPORT_OCD,
};

enum ABT_CONNECT_TOOL {
	ABT_CONN_NOTHING = 0,
	ABT_CONN_STUDIO,
	ABT_CONN_TOUCH,
	ABT_CONN_MAX
};

enum {
	ABT_RUNNING_OFF = 0,
	ABT_RUNNING_ON,
	ABT_RUNNING_EXIT,
};

enum {
	SIW_ABT_COMM_SEND_DATA_SZ = (4<<10),
};

/* UDP */
#pragma pack(push, 1)
struct siw_abt_send_data {
	u8 type;
	u8 mode;
	u8 flag;
	u8 touchCnt;
	u32 timestamp;
	u32 frame_num;
	u8 data[SIW_ABT_COMM_SEND_DATA_SZ];
};

struct siw_abt_dbg_report_hdr {
	u8 key_frame;
	u8 type;
	u16 data_size;
};

/* TCP */
enum {
	TCP_REG_READ = 0x80,
	TCP_REG_WRITE,
	TCP_FRAME_START,
	TCP_REPORT_START,
	TCP_SYNC_START,
	TCP_SYNCDEBUG_START,
	TCP_CAPTURE_STOP,
	TCP_CONNECT_CMD,
	TCP_DISCONNECT_CMD
};

enum {
	COMM_RES_SUCCESS	= 0,
	COMM_RES_WR_FAILED	= 0x8001,
};

struct siw_abt_packet_hdr {
	u8       cmd;
	u16      addr;
	u16      size;
};

/*
 * This shall be less than bus(i2c, spi) buffer size
 * See 'siw_touch_bus_alloc_buffer'
 */
enum {
	SIW_ABT_COMM_PACKET_FRAME_MAX = (32<<10),
};

struct siw_abt_comm_packet {
	struct siw_abt_packet_hdr hdr;
	union {
		u32		value;
		u8		frame[SIW_ABT_COMM_PACKET_FRAME_MAX];
	} data;
};

enum {
	ABT_COMM_NAME_SZ = 128,
	ABT_SEND_IP_SIZE = 16,
};

typedef int (*abt_sock_listener_t)(void *data, uint8_t *buf, uint32_t len);

struct siw_hal_abt_comm {
//	struct device *dev;

	char name[ABT_COMM_NAME_SZ];

	struct task_struct *thread;

	int family;
	int type;
	int protocol;

	/*ABT Studio socket */
	struct socket *sock;
	struct sockaddr_in addr;

	/* Touch Solution socket */
	struct socket *ts_sock;
	struct sockaddr_in ts_addr;

	struct socket *sock_send;
	struct sockaddr_in addr_send;

	struct socket *curr_sock;
	struct sockaddr_in *curr_addr;

	abt_sock_listener_t	sock_listener;

	struct siw_abt_send_data *data_send;

	struct siw_abt_comm_packet *recv_packet;
	struct siw_abt_comm_packet *send_packet;

	int send_connected;
	char send_ip[ABT_SEND_IP_SIZE+4];

	atomic_t running;
};

struct siw_hal_abt_log_file_head {
	/* 4 byte => 4B */
	/* 2B : resolution x */
	unsigned short resolution_x;
	/* 2B : resoultion y */
	unsigned short resolution_y;
	/* 4 byte => 8B */
	/* 1B : real node count x */
	unsigned char node_cnt_x;
	/* 1B : real node count y */
	unsigned char node_cnt_y;
	/* 1B : additional node count */
	unsigned char additional_node_cnt;
	unsigned char dummy1;
	/* 4 byte => 12B */
	/* 2B : RN MIN value */
	unsigned short rn_min;
	/* 2B : RN MAX value */
	unsigned short rn_max;
	/* 4 byte => 16B */
	/* 1B : RAW Size (1B or 2B) */
	unsigned char raw_data_size;
	/* 1B : RN Size (1B or 2B) */
	unsigned char rn_data_size;
	/* 1B : frame buf type data type : E_DATA_TYPE number */
	unsigned char frame_data_type;
	/* 1B : frame buf size data : unit size (1B or 2B) */
	unsigned char frame_data_size;
	/* 4 byte => 20B */
	/* 4B : x_node start/end x location(screen resolution) */
	unsigned short loc_x[2];
	/* 4 byte => 24B */
	/* 4B : y_node start/end y location(screen resolution) */
	unsigned short loc_y[2];
	/* 104 byte => 128B */
	/* total 128B */
	unsigned char dummy[104];
} __packed;

#pragma pack(pop)

struct siw_hal_abt_touch_info {
	u8 wakeUpType;
	u8 touchCnt:5;
	u8 buttonCnt:3;
	u16 palmBit;
} __packed;

struct siw_hal_abt_touch_data {
	u8 toolType:4;
	u8 event:4;
	s8 track_id;
	u16 x;
	u16 y;
	union {
		u8 pressure;
		u8 contourPcnt;		//contour_point count
	} byte;
	u8 angle;
	u16 width_major;
	u16 width_minor;
} __packed;

struct siw_hal_abt_tc_reg_cpy_to_report {
	u32 tc_reg_copy[5];
};

struct siw_hal_abt_on_chip_dbg {
	u32 rnd_addr;
	u32 rnd_piece_no;
};

struct siw_hal_abt_report_p {
	struct siw_hal_abt_touch_info touch_info;
	struct siw_hal_abt_touch_data touch_data[MAX_REPORT_SLOT];
	u16 contour_p[P_CONTOUR_POINT_MAX];		//contour_point
	struct siw_hal_abt_tc_reg_cpy_to_report tc_reg;
	struct siw_hal_abt_on_chip_dbg ocd;
	u8 dummy[16];
};

struct siw_hal_abt_data {
	struct device *dev;
	struct mutex abt_comm_lock;
	struct mutex abt_socket_lock;

	struct siw_hal_abt_comm abt_comm;
	int abt_socket_mutex_flag;
	int abt_socket_report_mode;

	u32 prev_rnd_piece_no;
	u32	dbg_offset;
	u32 dbg_offset_base;
	u32 ocd_pieces_cnt;
	int ocd_piece_size;

	u16 frame_num;
	int abt_report_mode;
	u8 abt_report_point;
	u8 abt_report_ocd;
	int abt_report_mode_onoff;
	int abt_conn_tool;

	int set_get_data_func;

	int client_connected;
	int client_connect_trying;

	u32 connect_error_count;
};

enum {
	ABT_SOCK_SEND_TIME_OUT	= 10,
	ABT_SOCK_RECV_TIME_OUT	= 10,
};

static u32 t_abt_dbg_mask = 0;
static u32 t_abt_sock_sndtimeo = ABT_SOCK_SEND_TIME_OUT;
static u32 t_abt_sock_rcvtimeo = ABT_SOCK_RECV_TIME_OUT;

/* usage
 * (1) echo <value> > /sys/module/{Siw Touch Module Name}/parameters/s_abt_dbg_mask
 * (2) insmod {Siw Touch Module Name}.ko s_abt_dbg_mask=<value>
 */
module_param_named(s_abt_dbg_mask, t_abt_dbg_mask, uint, S_IRUGO|S_IWUSR|S_IWGRP);

/* usage
 * (1) echo <value> > /sys/module/{Siw Touch Module Name}/parameters/s_abt_sock_sndtimeo
 * (2) insmod {Siw Touch Module Name}.ko s_abt_sock_sndtimeo=<value>
 */
module_param_named(s_abt_sock_sndtimeo, t_abt_sock_sndtimeo, uint, S_IRUGO|S_IWUSR|S_IWGRP);

/* usage
 * (1) echo <value> > /sys/module/{Siw Touch Module Name}/parameters/s_abt_sock_rcvtimeo
 * (2) insmod {Siw Touch Module Name}.ko s_abt_sock_rcvtimeo=<value>
 */
module_param_named(s_abt_sock_rcvtimeo, t_abt_sock_rcvtimeo, uint, S_IRUGO|S_IWUSR|S_IWGRP);

static u32 t_abt_port_default = DEFAULT_PORT;
static u32 t_abt_port_tcp = TS_TCP_PORT;
static u32 t_abt_port_send = SEND_PORT;
static u32 t_abt_buf_omk = OMK_BUF;

/* usage
 * (1) echo <value> > /sys/module/{Siw Touch Module Name}/parameters/s_abt_port_default
 * (2) insmod {Siw Touch Module Name}.ko s_abt_port_default=<value>
 */
module_param_named(s_abt_port_default, t_abt_port_default, uint, S_IRUGO|S_IWUSR|S_IWGRP);

/* usage
 * (1) echo <value> > /sys/module/{Siw Touch Module Name}/parameters/s_abt_port_tcp
 * (2) insmod {Siw Touch Module Name}.ko s_abt_port_tcp=<value>
 */
module_param_named(s_abt_port_tcp, t_abt_port_tcp, uint, S_IRUGO|S_IWUSR|S_IWGRP);

/* usage
 * (1) echo <value> > /sys/module/{Siw Touch Module Name}/parameters/s_abt_port_send
 * (2) insmod {Siw Touch Module Name}.ko s_abt_port_send=<value>
 */
module_param_named(s_abt_port_send, t_abt_port_send, uint, S_IRUGO|S_IWUSR|S_IWGRP);

/* usage
 * (1) echo <value> > /sys/module/{Siw Touch Module Name}/parameters/s_abt_buf_omk
 * (2) insmod {Siw Touch Module Name}.ko s_abt_buf_omk=<value>
 */
module_param_named(s_abt_buf_omk, t_abt_buf_omk, uint, S_IRUGO|S_IWUSR|S_IWGRP);


static const char *abt_conn_name_str[] = {
	[ABT_CONN_NOTHING]	= "CONN_NOTHING",
	[ABT_CONN_STUDIO]	= "CONN_STUDIO",
	[ABT_CONN_TOUCH]	= "CONN_TOUCH",
};

static inline const char *abt_conn_name(int idx)
{
	return (idx < ABT_CONN_MAX) ? abt_conn_name_str[idx] : "(invalid)";
}

#define SIW_ABT_TAG 	"abt"

#define t_abt_info(_abt, fmt, args...)	\
		__t_dev_info(_abt->dev, SIW_ABT_TAG "[%d(%s)]: " fmt,	\
					_abt->abt_conn_tool, abt_conn_name(_abt->abt_conn_tool),	\
					##args)

#define t_abt_err(_abt, fmt, args...)	\
		__t_dev_err(_abt->dev, SIW_ABT_TAG "[%d(%s)] : " fmt,	\
					_abt->abt_conn_tool, abt_conn_name(_abt->abt_conn_tool),	\
					##args)

#define t_abt_warn(_abt, fmt, args...)	\
		__t_dev_warn(_abt->dev, SIW_ABT_TAG "[%d(%s)] : " fmt,	\
					_abt->abt_conn_tool, abt_conn_name(_abt->abt_conn_tool),	\
					##args)

#define t_abt_dbg(condition, _abt, fmt, args...)	\
		do {	\
			if (unlikely(t_abt_dbg_mask & (condition)))	\
				t_abt_info(_abt, fmt, ##args);	\
		} while (0)

#define t_abt_dbg_base(_dev, fmt, args...)	\
		t_abt_dbg(DBG_BASE, _dev, fmt, ##args)

#define t_abt_dbg_trace(_dev, fmt, args...)	\
		t_abt_dbg(DBG_TRACE, _dev, fmt, ##args)


#define siw_abt_sysfs_err_invalid_param(_abt)	\
		t_abt_err(_abt, "Invalid param\n");


/*
 * Do not modify these string arbitrarily,
 * which are related to simon operation.
 */
#define ABT_MON_SOCK_RSTR	"@ABTR"
#define ABT_MON_SOCK_SSTR	"@ABTS"

static void __used siwmon_submit_ops_abt_sock(
			struct siw_hal_abt_data *abt,
			void *buf,
			int len,
			int ret,
			int recv)
{
	size_t data[2] = { (size_t)buf, len };
	siwmon_submit_ops(abt->dev,
			(recv) ? ABT_MON_SOCK_RSTR : ABT_MON_SOCK_SSTR,
			(void *)data,
			2,
			ret);
}

#define siwmon_submit_ops_abt_sock_recv(_abt, _buf, _len, _ret)	\
		siwmon_submit_ops_abt_sock(_abt, _buf, _len, _ret, 1);

#define siwmon_submit_ops_abt_sock_send(_abt, _buf, _len, _ret)	\
		siwmon_submit_ops_abt_sock(_abt, _buf, _len, _ret, 0);


#define ABT_IP_FORMAT		"%d.%d.%d.%d"
#define ABT_IP_PRT_PARAM(_addr)	\
		((_addr)>>24)&0XFF, ((_addr)>>16)&0XFF, ((_addr)>>8)&0XFF, (_addr)&0XFF

static int __used abt_sock_msg_chk_size(struct siw_hal_abt_data *abt,
				int size, int boundary, char *name)
{
	if (size >= boundary) {
		return 0;
	}

	/*
 	 * EAGAIN is not real error
 	 * becase it happens when the server doesn't send any packet
	 */
	if (size == -EAGAIN) {
		return 0;
	}

	/*
 	 * 0 means normal disconnection
	 */
	if (size != 0)
		t_abt_err(abt, "%s failed, %d\n", name, size);

	return -1;
}

static int abt_sock_recvmsg(struct siw_hal_abt_data *abt,
					struct socket *sock,
					struct sockaddr_in *addr_in,
					unsigned char *buf,
					int len,
					int num)
{
	struct msghdr msg;
	struct kvec iov = {
		.iov_base = buf,
		.iov_len = len
	};
	int ret = 0;

	memset(&msg, 0, sizeof(struct msghdr));

	msg.msg_name = addr_in;
	msg.msg_namelen  = sizeof(struct sockaddr_in);

	ret = kernel_recvmsg(sock, &msg, &iov, num, len, 0);
	siwmon_submit_ops_abt_sock_recv(abt, buf, len, ret);
	(void)abt_sock_msg_chk_size(abt, ret, 0, "kernel_recvmsg");

	return ret;
}

static int abt_sock_sendmsg(struct siw_hal_abt_data *abt,
					struct socket *sock,
					struct sockaddr_in *addr_in,
					unsigned char *buf,
					int len,
					int num)
{
	struct msghdr msg;
	struct kvec iov = {
		.iov_base = buf,
		.iov_len = len
	};
	int ret;

	memset(&msg, 0, sizeof(struct msghdr));

	msg.msg_name = addr_in;
	msg.msg_namelen  = sizeof(struct sockaddr_in);

	ret = kernel_sendmsg(sock, &msg, &iov, 1, len);
	siwmon_submit_ops_abt_sock_send(abt, buf, len, ret);
	(void)abt_sock_msg_chk_size(abt, ret, 0, "kernel_sendmsg");

	return ret;
}

#define t_abt_log_sock_create(_abt, _name, _ret)	\
		{	\
			if (_ret < 0) {		\
				t_abt_err(_abt, "failed to create a %s socket, %d\n", _name, _ret);	\
			} else {	\
				t_abt_dbg_base(_abt, "%s socket created\n", _name);	\
			}	\
		} while (0)

static int siw_sock_create_kern(int family, int type, int protocol, struct socket **res)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 2, 0))
	return sock_create_kern(&init_net, family, type, protocol, res);
#else
	return sock_create_kern(family, type, protocol, res);
#endif
}

static int __used abt_sock_create_data(struct siw_hal_abt_data *abt,
				struct socket **res)
{
	int ret = siw_sock_create_kern(PF_INET, SOCK_DGRAM, IPPROTO_UDP, res);
	t_abt_log_sock_create(abt, "datagram", ret);
	return ret;
}

static int __used abt_sock_create_stream(struct siw_hal_abt_data *abt,
				struct socket **res)
{
	int ret = siw_sock_create_kern(PF_INET, SOCK_STREAM, IPPROTO_TCP, res);
	t_abt_log_sock_create(abt, "stream", ret);
	return ret;
}

static char *__abt_sock_connect_err_str(int ret)
{
	char *str;

	switch (ret) {
	case -EPFNOSUPPORT:
		str = "protocol family not supported";
		break;
	case -EAFNOSUPPORT:
		str = "address family not supported";
		break;
	case -EADDRINUSE:
		str = "address already in use";
		break;
	case -EADDRNOTAVAIL:
		str = "can't assign requested address";
		break;
	case -ENETDOWN:
		str = "network is down";
		break;
	case -ENETUNREACH:
		str = "network is unreachable";
		break;
	case -ENETRESET:
		str = "network dropped connection because of reset";
		break;
	case -ECONNABORTED:
		str = "sw caused connection abort";
		break;
	case -ECONNRESET:
		str = "connection reset by peer";
		break;
	default:
		str = " ";
		break;
	}

	return str;
}

static inline int __abt_sock_ret_is_err(int ret)
{
	return (ret < 0);
}

static int __used abt_sock_listen(struct siw_hal_abt_data *abt,
				struct socket *sock,
				struct sockaddr_in *addr_in,
				int backlog)
{
	int ret = kernel_listen(sock, backlog);
	if (__abt_sock_ret_is_err(ret)) {
		char *str = __abt_sock_connect_err_str(ret);

		t_abt_err(abt, "kernel_listen[" ABT_IP_FORMAT ":%d, %d](%d) failed, %d(%s)\n",
				ABT_IP_PRT_PARAM(__be32_to_cpu(addr_in->sin_addr.s_addr)),
				__be16_to_cpu(addr_in->sin_port),
				addr_in->sin_family,
				backlog,
				ret, str);
	} else {
		t_abt_dbg_base(abt, "kernel_listen[" ABT_IP_FORMAT ":%d, %d](%d) done\n",
				ABT_IP_PRT_PARAM(__be32_to_cpu(addr_in->sin_addr.s_addr)),
				__be16_to_cpu(addr_in->sin_port),
				addr_in->sin_family,
				backlog);
	}
	return ret;
}

static int __used abt_sock_bind(struct siw_hal_abt_data *abt,
				struct socket *sock,
				struct sockaddr_in *addr_in,
				int addrlen)
{
	int ret = kernel_bind(sock, (struct sockaddr *)addr_in, addrlen);
	if (__abt_sock_ret_is_err(ret)) {
		char *str = __abt_sock_connect_err_str(ret);

		t_abt_err(abt, "kernel_bind[" ABT_IP_FORMAT ":%d, %d] failed, %d(%s)\n",
				ABT_IP_PRT_PARAM(__be32_to_cpu(addr_in->sin_addr.s_addr)),
				__be16_to_cpu(addr_in->sin_port),
				addr_in->sin_family,
				ret, str);
	} else {
		t_abt_dbg_base(abt, "kernel_bind[" ABT_IP_FORMAT ":%d, %d] done\n",
				ABT_IP_PRT_PARAM(__be32_to_cpu(addr_in->sin_addr.s_addr)),
				__be16_to_cpu(addr_in->sin_port),
				addr_in->sin_family);
	}
	return ret;
}

static int __used abt_sock_connect(struct siw_hal_abt_data *abt,
				struct socket *sock,
				struct sockaddr_in *addr_in,
				int addrlen,
				int flags)
{
	int ret = kernel_connect(sock, (struct sockaddr *)addr_in, addrlen, flags);
	if (__abt_sock_ret_is_err(ret)) {
		char *str = __abt_sock_connect_err_str(ret);

		t_abt_err(abt, "kernel_connect[" ABT_IP_FORMAT ":%d, %d] failed, %d(%s)\n",
				ABT_IP_PRT_PARAM(__be32_to_cpu(addr_in->sin_addr.s_addr)),
				__be16_to_cpu(addr_in->sin_port),
				addr_in->sin_family,
				ret, str);
	} else {
		t_abt_dbg_base(abt, "kernel_connect[" ABT_IP_FORMAT ":%d, %d] done\n",
				ABT_IP_PRT_PARAM(__be32_to_cpu(addr_in->sin_addr.s_addr)),
				__be16_to_cpu(addr_in->sin_port),
				addr_in->sin_family);
	}
	return ret;
}

static inline int __abt_sock_ret_is_err_sd(int ret)
{
	if (ret == -ENOTCONN)
		return 0;

	return (ret < 0);
}

static int __used abt_sock_shutdown(struct siw_hal_abt_data *abt,
				struct socket *sock,
				struct sockaddr_in *addr_in)
{
	int ret = kernel_sock_shutdown(sock, SHUT_RDWR);
	if (__abt_sock_ret_is_err_sd(ret)) {
		t_abt_err(abt, "kernel_sock_shutdown[" ABT_IP_FORMAT ":%d, %d] failed, %d\n",
				ABT_IP_PRT_PARAM(__be32_to_cpu(addr_in->sin_addr.s_addr)),
				__be16_to_cpu(addr_in->sin_port),
				addr_in->sin_family,
				ret);
	} else {
		t_abt_dbg_base(abt, "kernel_sock_shutdown[" ABT_IP_FORMAT ":%d, %d] done\n",
				ABT_IP_PRT_PARAM(__be32_to_cpu(addr_in->sin_addr.s_addr)),
				__be16_to_cpu(addr_in->sin_port),
				addr_in->sin_family);
	}
	return ret;
}

static void __used abt_sock_set_timeout(struct siw_hal_abt_data *abt,
				struct socket *sock,
				long new_sndtimeo,
				long new_rcvtimeo,
				char *name)
{
	long old_sndtimeo, old_rcvtimeo;

	spin_lock_bh(&sock->sk->sk_lock.slock);

	if (new_sndtimeo) {
		old_sndtimeo = sock->sk->sk_sndtimeo/HZ;
		sock->sk->sk_sndtimeo = new_sndtimeo * HZ;
		t_abt_dbg_base(abt,
			"time_out(%s) changed : send %ld secs <- %ld secs\n",
			name, new_sndtimeo, old_sndtimeo);
	}

	if (new_rcvtimeo) {
		old_rcvtimeo = sock->sk->sk_rcvtimeo/HZ;
		sock->sk->sk_rcvtimeo = new_rcvtimeo * HZ;
		t_abt_dbg_base(abt,
			"time_out(%s) changed : recv %ld secs <- %ld secs\n",
			name, new_rcvtimeo, old_rcvtimeo);
	}
//	t_abt_info(abt, "rcvbuf : %d, sndbuf : %d\n", sock->sk->sk_rcvbuf, sock->sk->sk_sndbuf);

	spin_unlock_bh(&sock->sk->sk_lock.slock);
}

#if 0
static int __used __abt_sock_getsockopt(struct siw_hal_abt_data *abt,
				struct socket *sock, int level, int optname,
				char *optval, int *optlen)
{
	int ret = kernel_getsockopt(sock, level, optname, optval, optlen);
	if (ret < 0) {
		t_abt_err(abt, "kernel_getsockopt failed, %d\n", ret);
	} else {
		t_abt_dbg_base(abt, "kernel_getsockopt done\n");
	}
	return ret;
}

static int __used __abt_sock_setsockopt(struct siw_hal_abt_data *abt,
				struct socket *sock, int level, int optname,
				char *optval, int optlen)
{
	int ret = kernel_setsockopt(sock, level, optname, optval, optlen);
	if (ret < 0) {
		t_abt_err(abt, "kernel_setsockopt failed, %d\n", ret);
	} else {
		t_abt_dbg_base(abt, "kernel_setsockopt done\n");
	}
	return ret;
}
#endif

static int __used abt_conn_is_invalid(struct siw_hal_abt_data *abt)
{
	int inval = !!(!abt->abt_conn_tool || (abt->abt_conn_tool >= ABT_CONN_MAX));
	if (inval)
		t_abt_warn(abt, "Invalid Tool Select, %d\n", abt->abt_conn_tool);

	return inval;
}

static int __used abt_is_set_func(struct siw_hal_abt_data *abt)
{
	return abt->set_get_data_func;
}

static void __used abt_set_data_func(struct siw_hal_abt_data *abt, u8 mode)
{
	abt->set_get_data_func = !!(mode);
	t_abt_dbg_base(abt, "set_get_data_func = %d\n", mode);
}

static int abt_read_memory(struct siw_hal_abt_data *abt,
			u32 raddr, u32 waddr,
			u32 wdata, int wsize,
			u8 *rdata, int rsize)
{
	struct device *dev = abt->dev;
	u16 curr_read = 0;
	u32 rest_read = rsize;
	int ret = 0;

#if 0
	if (!raddr || !waddr) {
		t_abt_err(abt, "NULL addr : raddr 0x%X, waddr 0x%X\n",
				raddr, waddr);
		return -EINVAL;
	}
#endif

	curr_read = (rest_read > MAX_RW_SIZE) ? MAX_RW_SIZE : rest_read;

	while (curr_read > 0) {
		ret = siw_hal_reg_write(dev, waddr, (void *)&wdata, wsize);
		if (ret < 0) {
			return ret;
		}
		ret = siw_hal_reg_read(dev, raddr, (void *)rdata, curr_read);
		if (ret < 0) {
			return ret;
		}
		curr_read = ret;

		if (rest_read > curr_read) {
			rest_read -= curr_read;
			rdata += curr_read;
		} else {
			rest_read = 0;
		}
		wdata += curr_read>>2;

		curr_read = (rest_read > MAX_RW_SIZE) ? MAX_RW_SIZE : rest_read;
	}

	return rsize;
}

static int abt_force_set_report_mode(struct siw_hal_abt_data *abt,
				u32 mode)
{
	struct device *dev = abt->dev;
	struct siw_touch_chip *chip = to_touch_chip(dev);
//	struct siw_ts *ts = chip->ts;
	struct siw_hal_reg *reg = chip->reg;
	u32 rdata = 0;
	u32 wdata = mode;
	int ret = 0;

	/* send debug mode*/
	ret = siw_hal_write_value(dev,
				reg->cmd_raw_data_report_mode_write,
				wdata);
	if (ret < 0) {
		t_abt_err(abt, "debug report buffer write error\n");
		goto out;
	}
	abt->abt_report_mode = mode;
	t_abt_info(abt, "mode(%d) set done\n", mode);

	/* receive debug report buffer*/
	if (mode >= 0) {
		ret = siw_hal_read_value(dev,
					reg->cmd_raw_data_report_mode_read,
					&rdata);
		t_abt_info(abt, "rdata(0x%08X)\n", rdata);
		if (ret < 0 || rdata <= 0) {
			t_abt_err(abt, "debug report buffer pointer error\n");
			goto out;
		}
		t_abt_info(abt, "debug report buffer pointer, 0x%x\n", rdata);
	}


out:
	if (ret) {
		int t_ret = siw_hal_write_value(dev,
							reg->cmd_raw_data_report_mode_write,
							0);
		if (t_ret < 0)
			t_dev_warn(dev, "%s\n", __func__);
		abt->abt_report_mode = 0;
		abt->abt_report_mode_onoff = 0;
	}

	return ret;
}

static int abt_set_report_mode(struct siw_hal_abt_data *abt, u32 mode)
{
//	struct device *dev = abt->dev;
	int ret;

//	t_abt_info(abt, "[ABT] mode:%d\n", mode);

	if (abt->abt_report_mode == mode) {
	#if 0
		t_abt_warn(abt,
			"mode(%d) is already set\n",
			mode);
	#endif
		return 0;
	}

	if (mode < 0) {
		t_abt_err(abt,
			"mode(%d) is invalid\n", mode);
		return -EINVAL;
	}

	ret = abt_force_set_report_mode(abt, mode);

	return ret;
}

static int abt_ksocket_do_send(struct siw_hal_abt_data *abt,
				struct socket *sock,
				struct sockaddr_in *addr,
				unsigned char *buf, int len)
{
	int ret = 0;

	if (!sock) {
		t_abt_err(abt, "socket send err : NULL sock\n");
		return 0;
	}

	ret = abt_sock_sendmsg(abt, sock, addr, buf, len, 1);

	return ret;
}

static int abt_ksocket_send(struct siw_hal_abt_data *abt,
				struct socket *sock,
				struct sockaddr_in *addr,
				unsigned char *buf, int len)
{
	if (abt_conn_is_invalid(abt)) {
		return 0;
	}

	return abt_ksocket_do_send(abt, sock, addr, buf, len);
}

static void __used abt_ksocket_send_disconn(struct siw_hal_abt_data *abt,
			struct socket *sock, struct sockaddr_in *addr_in)
{
	struct siw_hal_abt_comm *abt_comm = &abt->abt_comm;
	struct siw_abt_comm_packet *send_pkt = abt_comm->send_packet;

	t_abt_dbg_base(abt, "send TCP_DISCONNECT_CMD(%d)\n", TCP_DISCONNECT_CMD);

	/* send disconnect command to server */
	send_pkt->hdr.cmd = TCP_DISCONNECT_CMD;
	send_pkt->hdr.size = 4;
	abt_ksocket_send(abt,
			sock,
			addr_in,
			(u8 *)send_pkt,
			sizeof(struct siw_abt_packet_hdr)
			+ send_pkt->hdr.size);
}

static int __used abt_ksocket_send_exit(struct siw_hal_abt_data *abt)
{
//	struct device *dev = abt->dev;
	struct socket *sock;
	struct sockaddr_in send_addr_in;
	uint8_t send_buf[1] = { 1 };
	int ret = 0;

	ret = abt_sock_create_data(abt, &sock);
	if (ret < 0) {
		goto out;
	}

	memset(&send_addr_in, 0, sizeof(struct sockaddr));
	send_addr_in.sin_family = PF_INET;
	send_addr_in.sin_addr.s_addr = in_aton(DEFAULT_IP);
	send_addr_in.sin_port = htons(t_abt_port_default);

	ret = abt_sock_connect(abt, sock,
					&send_addr_in,
					sizeof(struct sockaddr),
					!O_NONBLOCK);
	if (ret < 0) {
		goto out_sock;
	}

	t_abt_dbg_base(abt, "connect send socket (%s, %d)\n",
				DEFAULT_IP, t_abt_port_default);

	ret = abt_sock_sendmsg(abt, sock, &send_addr_in,
					send_buf, sizeof(send_buf), 1);

out_sock:
	sock_release(sock);

out:
	return ret;
}

static void __used abt_ksocket_free_send_socket(struct siw_hal_abt_data *abt)
{
	struct siw_hal_abt_comm *abt_comm = &abt->abt_comm;

	abt_comm->send_connected = 0;
	if (abt_comm->sock_send != NULL) {
		sock_release(abt_comm->sock_send);
		abt_comm->sock_send = NULL;
		t_abt_info(abt, "send socket released\n");
	}
}

static int32_t __used abt_ksocket_init_send_socket(struct siw_hal_abt_data *abt)
{
//	struct device *dev = abt->dev;
	struct siw_hal_abt_comm *abt_comm = &abt->abt_comm;
	struct socket *sock;
	struct sockaddr_in *addr_in = &abt_comm->addr_send;
	char *ip = (char *)abt_comm->send_ip;
	int ret = 0;

	if (abt_comm->sock_send != NULL) {
		return 0;
	}

	ret = abt_sock_create_data(abt, &sock);
	if (ret < 0) {
		goto out;
	}
	abt_comm->sock_send = sock;

	abt_sock_set_timeout(abt, sock,
			t_abt_sock_sndtimeo, t_abt_sock_rcvtimeo,
			"send_socket");

	memset(addr_in, 0, sizeof(struct sockaddr));
	addr_in->sin_family = PF_INET;
	addr_in->sin_addr.s_addr = in_aton(ip);
	addr_in->sin_port = htons(t_abt_port_send);

	ret = abt_sock_connect(abt, sock,
					 addr_in,
					 sizeof(struct sockaddr),
					 !O_NONBLOCK);
	if (ret < 0) {
		goto out;
	}

	abt_comm->send_connected = 1;
	t_abt_info(abt, "connect send socket (%s,%d)\n", ip, t_abt_port_send);

	return ret;

out:
	abt_ksocket_free_send_socket(abt);

	return ret;
}

static void abt_ksocket_exit_disconn(struct siw_hal_abt_data *abt)
{
	struct siw_hal_abt_comm *abt_comm = &abt->abt_comm;
	struct socket *sock;
	struct sockaddr_in *addr_in;
//	int ret = 0;

	mutex_lock(&abt->abt_comm_lock);

	if (atomic_read(&abt_comm->running) == ABT_RUNNING_OFF) {
		goto out;
	}

	atomic_set(&abt_comm->running, ABT_RUNNING_EXIT);

	sock = (abt->abt_conn_tool != ABT_CONN_TOUCH) ?
			abt_comm->sock : abt_comm->ts_sock;
	addr_in = (abt->abt_conn_tool != ABT_CONN_TOUCH) ?
			&abt_comm->addr : &abt_comm->ts_addr;

	if (!sock) {
		goto out;
	}

	switch (abt->abt_conn_tool) {
	case ABT_CONN_TOUCH:
		if (abt->client_connected) {
		#if 0
			abt_ksocket_send_disconn(abt, sock, addr_in);
		#else
			abt_sock_shutdown(abt, sock, addr_in);
		#endif
		} else if (abt->client_connect_trying) {
			abt_sock_shutdown(abt, sock, addr_in);
			touch_msleep(100);
		}
		break;

	case ABT_CONN_STUDIO:
		/* Reserved */
		break;

	default:
		break;
	}

out:
	mutex_unlock(&abt->abt_comm_lock);
}

static void abt_ksocket_exit_kill(struct siw_hal_abt_data *abt)
{
	struct siw_hal_abt_comm *abt_comm = &abt->abt_comm;
	int ret = 0;

	mutex_lock(&abt->abt_comm_lock);
	if (abt_comm->thread != NULL) {
		ret = kthread_stop(abt_comm->thread);
		if (ret < 0) {
			t_abt_err(abt,
				"unknown error %d while trying to " \
				"terminate kernel thread\n",
				ret);
		} else {
			u32 running = 0;
			do {
				running = atomic_read(&abt_comm->running);
				if (running == ABT_RUNNING_OFF) {
					break;
				}
				t_abt_dbg_base(abt,
					"waiting for killing thread. " \
					"running = %d\n",
					running);
				touch_msleep(10);
			} while (1);
			t_abt_dbg_base(abt, "succesfully killed kernel thread!\n");
		}
		abt_comm->thread = NULL;
	}

	if (abt_comm->sock != NULL) {
		sock_release(abt_comm->sock);
		abt_comm->sock = NULL;
		t_abt_dbg_base(abt, "sock released\n");
	}

	if (abt_comm->ts_sock != NULL) {
		sock_release(abt_comm->ts_sock);
		abt_comm->ts_sock = NULL;
		t_abt_dbg_base(abt, "ts_sock released\n");
	}

	abt_ksocket_free_send_socket(abt);

	abt_comm->curr_sock = NULL;
	abt_comm->curr_addr = NULL;

	abt->abt_socket_mutex_flag = 0;
	abt->abt_socket_report_mode = 0;

	abt->abt_conn_tool = ABT_CONN_NOTHING;
	mutex_unlock(&abt->abt_comm_lock);
}

static void abt_ksocket_exit(struct siw_hal_abt_data *abt)
{
	t_abt_dbg_base(abt, "start killing thread[%d]\n", abt->abt_conn_tool);

	abt_ksocket_exit_disconn(abt);

	t_abt_dbg_base(abt, "waiting for killing thread\n");

	abt_ksocket_exit_kill(abt);

	t_abt_dbg_base(abt, "socket exit\n");

	siw_touch_mon_resume(abt->dev);
}


static int32_t abt_ksocket_raw_data_send(
		struct siw_hal_abt_data *abt, uint8_t *buf, uint32_t len)
{
//	struct device *dev = abt->dev;
	struct siw_hal_abt_comm *abt_comm = &abt->abt_comm;
	int ret = 0;

	if (abt_comm->sock_send == NULL) {
		abt_ksocket_init_send_socket(abt);
	}

	if (!abt_comm->send_connected) {
		abt->connect_error_count++;
		if (abt->connect_error_count > 10) {
			t_abt_err(abt, "connection error - socket release\n");
			abt_force_set_report_mode(abt, 0);
			abt_ksocket_exit(abt);
		}
		return ret;
	}

	ret = abt_ksocket_do_send(abt,
			abt_comm->sock_send,
			&abt_comm->addr_send,
			buf, len);

	return ret;
}

static int abt_ksocket_recv_err(struct siw_hal_abt_data *abt,
				struct siw_abt_comm_packet *recv_pkt,
				uint32_t len)
{
	struct siw_hal_abt_comm *abt_comm = &abt->abt_comm;
	struct siw_abt_comm_packet *send_pkt = abt_comm->send_packet;
	const char *msg = NULL;
	int ret = 0;

	if (len < sizeof(struct siw_abt_packet_hdr)) {
		msg = "abnormal";
		ret = -EPROTONOSUPPORT;
	} else if (recv_pkt->hdr.size > sizeof(recv_pkt->data.frame)) {
		msg = "overflow";
		ret = -EOVERFLOW;
	} else if (len < (recv_pkt->hdr.size + sizeof(struct siw_abt_packet_hdr))) {
		msg = "slicing";
		ret = -EPROTONOSUPPORT;
	}

	if (ret < 0) {
		t_abt_err(abt, "%s: len %d, size %d, cmd %02Xh, addr %04Xh\n",
				msg,
				len,
				recv_pkt->hdr.size,
				recv_pkt->hdr.cmd, recv_pkt->hdr.addr);

		send_pkt->data.value = COMM_RES_WR_FAILED;
	}

	return ret;
}

static int abt_ksocket_recv_from_pctool(
				void *data, uint8_t *buf, uint32_t len)
{
	struct siw_hal_abt_data *abt = data;
	struct device *dev = abt->dev;
	struct siw_hal_abt_comm *abt_comm = &abt->abt_comm;
	struct siw_abt_comm_packet *send_pkt = abt_comm->send_packet;
	struct siw_abt_comm_packet *recv_pkt = NULL;
	int ret = 0;

	recv_pkt = (struct siw_abt_comm_packet *)buf;
	abt_comm->recv_packet = recv_pkt;

	send_pkt->hdr.cmd = recv_pkt->hdr.cmd;
	send_pkt->hdr.addr = recv_pkt->hdr.addr;
	send_pkt->hdr.size = sizeof(send_pkt->data.value);

	ret = abt_ksocket_recv_err(abt, recv_pkt, len);
	if (ret < 0) {
		goto send_resp;
	}

	t_abt_dbg_trace(abt, ">> cmd %02Xh, addr %04Xh, size %d\n",
		recv_pkt->hdr.cmd, recv_pkt->hdr.addr, recv_pkt->hdr.size);

	if (recv_pkt->hdr.cmd == TCP_REG_READ) {
		ret = siw_hal_reg_read(dev,
					recv_pkt->hdr.addr,
					(void *)send_pkt->data.frame,
					recv_pkt->data.value);
		if (ret < 0) {
			t_abt_err(abt,
				"TCP REG READ error : %d\n",
				ret);
		//	goto out;
		}

		send_pkt->hdr.size = recv_pkt->data.value;
		goto send_resp;
	}

	if (recv_pkt->hdr.cmd == TCP_REG_WRITE) {
		u8 *rcv_frame = recv_pkt->data.frame;

		if (recv_pkt->hdr.size > SIW_ABT_COMM_PACKET_FRAME_MAX) {
			t_abt_err(abt, "write size overflow : 0x%X > 0x%X\n",
				recv_pkt->hdr.size,
				SIW_ABT_COMM_PACKET_FRAME_MAX);
			return -EINVAL;
		}

		ret = siw_hal_reg_write(dev,
					recv_pkt->hdr.addr,
					(void *)rcv_frame,
					recv_pkt->hdr.size);
		if (ret < 0) {
			t_abt_err(abt,
				"TCP REG WRITE spi_read error : %d\n",
				ret);
			send_pkt->data.value = COMM_RES_WR_FAILED;
		} else {
			send_pkt->data.value = COMM_RES_SUCCESS;
		}

		goto send_resp;
	}

	send_pkt->data.value = COMM_RES_WR_FAILED;

send_resp:
	abt_ksocket_send(abt,
		abt_comm->ts_sock, &abt_comm->ts_addr,
		(u8 *)send_pkt,
		sizeof(struct siw_abt_packet_hdr) + send_pkt->hdr.size);

	return ret;
}

static int abt_ksocket_do_recv(struct siw_hal_abt_data *abt,
				unsigned char *buf, int len)
{
//	struct device *dev = abt->dev;
	struct siw_hal_abt_comm *abt_comm = &abt->abt_comm;
	struct socket *sock = NULL;
	struct sockaddr_in *addr = NULL;
	int size = 0;

	WARN_ON(abt_comm->sock_listener == NULL);

	sock = abt_comm->curr_sock;
	addr = abt_comm->curr_addr;
	if (!sock) {
		t_abt_err(abt, "socket recv err : NULL sock\n");
		return 0;
	}

	size = abt_sock_recvmsg(abt, sock, addr, buf, len, 1);
	if (size <= 0) {
		goto out;
	}

	if (abt_comm->sock_listener) {
		abt_comm->sock_listener((void *)abt, buf, size);
	}

out:
	return size;
}

static int abt_ksocket_recv(struct siw_hal_abt_data *abt,
				unsigned char *buf, int len)
{
	if (abt_conn_is_invalid(abt)) {
		return 0;
	}

	return abt_ksocket_do_recv(abt, buf, len);
}


/*
 * Ref. from 'include/uapi/asm-generic/errno-base.h'
 * #define	EAGAIN		11	// Try again
 * #define	EMLINK		31	// Too many links
 * #define	EPIPE		32	// Broken pipe
 *
 * Ref. from 'include/uapi/asm-generic/errno.h'
 * #define	ESTRPIPE		86	// Streams pipe error
 * #define	EUSERS			87	// Too many users
 * #define	ENOTSOCK		88	// Socket operation on non-socket
 * #define	EDESTADDRREQ	89	// Destination address required
 * #define	EMSGSIZE		90	// Message too long
 * #define	EPROTOTYPE		91	// Protocol wrong type for socket
 * #define	ENOPROTOOPT		92	// Protocol not available
 * #define	EPROTONOSUPPORT	93	// Protocol not supported
 * #define	ESOCKTNOSUPPORT	94	// Socket type not supported
 * #define	EOPNOTSUPP		95	// Operation not supported on transport endpoint
 * #define	EPFNOSUPPORT	96	// Protocol family not supported
 * #define	EAFNOSUPPORT	97	// Address family not supported by protocol
 * #define	EADDRINUSE		98	// Address already in use
 * #define	EADDRNOTAVAIL	99	// Cannot assign requested address
 * #define	ENETDOWN		100	// Network is down
 * #define	ENETUNREACH		101	// Network is unreachable
 * #define	ENETRESET		102	// Network dropped connection because of reset
 * #define	ECONNABORTED	103	// Software caused connection abort
 * #define	ECONNRESET		104	// Connection reset by peer
 * #define	ENOBUFS			105	// No buffer space available
 * #define	EISCONN			106	// Transport endpoint is already connected
 * #define	ENOTCONN		107	// Transport endpoint is not connected
 * #define	ESHUTDOWN		108	// Cannot send after transport endpoint shutdown
 * #define	ETIMEDOUT		110
 * #define	ECONNREFUSED	111	// Connection refused
 * #define	EHOSTDOWN		112	// Host is down
 * #define	EHOSTUNREACH	113	// No route to host
 * #define	EALREADY		114	// Operation already in progress
 * #define	EINPROGRESS		115	// Operation now in progress
 */

enum {
	ABT_SOCK_USE_NONE = 0,
	ABT_SOCK_USE_CONN = (1<<0),
	ABT_SOCK_USE_LISTEN = (1<<1),
	ABT_SOCK_USE_BIND = (1<<2),
};

enum {
	ABT_SOCK_PCTOOL_OPT = ABT_SOCK_USE_CONN,
	ABT_SOCK_STUDIO_OPT = ABT_SOCK_USE_BIND,
};

static int abt_ksocket_thread_init(struct siw_hal_abt_data *abt,
				int tool, int opt)
{
	struct siw_hal_abt_comm *abt_comm = &abt->abt_comm;
	struct sockaddr_in *addr_in = NULL;
	struct socket *sock = NULL;
	__be32 s_addr = 0;
	u32 s_port = 0;
	int ret = 0;

	t_abt_dbg_base(abt, "sock init[%d, %d] start\n", tool, opt);

	/* create a socket */
	switch (tool) {
	case ABT_CONN_STUDIO:
		/* Reserved */
		ret = -ESRCH;
		goto out;
		break;
	case ABT_CONN_TOUCH:
		ret = abt_sock_create_stream(abt, &sock);
		break;
	default:
		ret = -EINVAL;
		break;
	}
	if (ret < 0) {
		goto out;
	}

	switch (tool) {
	case ABT_CONN_STUDIO:
		/* Reserved */
		goto out;
		break;
	case ABT_CONN_TOUCH:
		addr_in = &abt_comm->ts_addr;
		s_addr = in_aton(abt_comm->send_ip);
		s_port = htons(t_abt_port_tcp);

		abt_comm->ts_sock = sock;
		abt_comm->curr_addr = &abt_comm->ts_addr;

		break;
	}
	abt_comm->curr_sock = sock;

	addr_in = abt_comm->curr_addr;

	memset(addr_in, 0, sizeof(struct sockaddr_in));
	addr_in->sin_family = PF_INET;
	addr_in->sin_addr.s_addr = s_addr;
	addr_in->sin_port = s_port;

	t_abt_dbg_base(abt, "sock init[%d, %d] addr config : " ABT_IP_FORMAT ":%d, %d\n",
				tool, opt,
				ABT_IP_PRT_PARAM(__be32_to_cpu(s_addr)),
				__be16_to_cpu(s_port),
				addr_in->sin_family);

	abt_sock_set_timeout(abt, sock,
			t_abt_sock_sndtimeo, t_abt_sock_rcvtimeo,
			"thread");

	if (opt & ABT_SOCK_USE_BIND) {
		ret = abt_sock_bind(abt, sock,
						addr_in,
						sizeof(struct sockaddr));
		if (ret < 0) {
			goto out;
		}
	}

	if (opt & ABT_SOCK_USE_LISTEN) {
		ret = abt_sock_listen(abt, sock,
					addr_in, 5);
		if (ret < 0) {
			goto out;
		}
	}

	if (opt & ABT_SOCK_USE_CONN) {
		abt->client_connect_trying = 1;
		ret = abt_sock_connect(abt, sock,
					addr_in,
					sizeof(struct sockaddr), !O_NONBLOCK);
		abt->client_connect_trying = 0;
		if (ret < 0) {
			goto out;
		}
	}

	t_abt_dbg_base(abt, "sock init[%d, %d] done : " ABT_IP_FORMAT ":%d, %d\n",
				tool, opt,
				ABT_IP_PRT_PARAM(__be32_to_cpu(s_addr)),
				__be16_to_cpu(s_port),
				addr_in->sin_family
				);

	return 0;

out:
	t_abt_err(abt, "sock init[%d, %d] failed, %d\n",
			tool, opt, ret);
	return ret;
}

static void abt_ksocket_thread_body(struct siw_hal_abt_data *abt,
				unsigned char *buf, int bufsize,
				int boundary, int delay)
{
	struct siw_hal_abt_comm *abt_comm = &abt->abt_comm;
	int size;
	int ret;

	while (1) {
		set_current_state(TASK_INTERRUPTIBLE);

		memset(buf, 0, bufsize);

		if (atomic_read(&abt_comm->running) != ABT_RUNNING_ON) {
			t_abt_dbg_base(abt, "running off\n");
			break;
		}

	//	mutex_lock(&abt->abt_comm_lock);
		size = abt_ksocket_recv(abt, buf, bufsize);
	//	mutex_unlock(&abt->abt_comm_lock);

		ret = abt_sock_msg_chk_size(abt, size, boundary,
					"abt_ksocket_recv");
		if (ret < 0) {
			break;
		}

		if (kthread_should_stop()) {
			t_abt_dbg_base(abt, "kthread_should_stop\n");
			break;
		}

		touch_msleep(delay);
	}
}

static void abt_ksocket_thread_exit(struct siw_hal_abt_data *abt,
				struct socket **res,
				struct sockaddr_in *addr)
{
	struct siw_hal_abt_comm *abt_comm = &abt->abt_comm;
	struct socket *sock = *res;
	int tool = abt->abt_conn_tool;

	mutex_lock(&abt->abt_comm_lock);

	if (*res != NULL) {
		if (atomic_read(&abt_comm->running) != ABT_RUNNING_EXIT) {
			abt_sock_shutdown(abt, sock, addr);
		}
		sock_release(sock);
		*res = NULL;
	}

	abt_comm->thread = NULL;
	atomic_set(&abt_comm->running, ABT_RUNNING_OFF);

	abt->abt_conn_tool = ABT_CONN_NOTHING;

	t_abt_dbg_base(abt, "thread terminated[%d]\n", tool);

	mutex_unlock(&abt->abt_comm_lock);
}

static int abt_ksocket_thread_for_pctool(void *data)
{
	struct siw_hal_abt_data *abt = data;
	struct siw_hal_abt_comm *abt_comm = &abt->abt_comm;
//	struct socket *sock = NULL;
	unsigned char *buf = NULL;
	int bufsize = 0;
	int tool = abt->abt_conn_tool;
	int ret = 0;

	/* kernel thread initialization */
	atomic_set(&abt_comm->running, ABT_RUNNING_ON);
//	abt_comm->dev = dev;

	ret = abt_ksocket_thread_init(abt, tool,
							ABT_SOCK_PCTOOL_OPT);
	if (ret) {
		atomic_set(&abt_comm->running, ABT_RUNNING_EXIT);
		goto out;
	}

	abt->abt_report_point = 1;
	abt->abt_report_ocd = 1;

	abt->client_connected = 1;
	t_abt_info(abt,
		"TCP connected with TS (ip %s, port %d)\n",
		abt_comm->send_ip, t_abt_port_tcp);

	buf = kzalloc(sizeof(struct siw_abt_comm_packet), GFP_KERNEL);
	bufsize = sizeof(struct siw_abt_comm_packet);

	abt_ksocket_thread_body(abt, buf, bufsize, 1, 0);

	kfree(buf);

out:
	__set_current_state(TASK_RUNNING);

	abt_ksocket_thread_exit(abt,
				&abt_comm->ts_sock,
				&abt_comm->ts_addr);

	abt->client_connected = 0;

	abt->abt_report_point = 0;
	abt->abt_report_ocd = 0;

	t_abt_info(abt, "%s terminated\n", abt_conn_name(tool));

	return ret;
}


#define abt_kthread_run(_abt, _func)	\
	({	\
		struct task_struct *_thread;	\
		t_abt_dbg_base(_abt, "Run kthread for %s\n", #_func);	\
		_thread = kthread_run(_func, _abt, "abt-%s", dev_name(_abt->dev));	\
		_thread;	\
	})

static int abt_ksocket_init(struct siw_hal_abt_data *abt,
			char *ip, int tool,
			abt_sock_listener_t listener)
{
//	struct device *dev = abt->dev;
	struct siw_hal_abt_comm *abt_comm = &abt->abt_comm;
	struct task_struct *thread = NULL;

	abt->abt_conn_tool = tool;

	if (abt_conn_is_invalid(abt)) {
		return -EINVAL;
	}

	t_abt_info(abt, "Tool Start\n");

	abt->abt_socket_report_mode = 1;

	memcpy(abt_comm->send_ip, ip, ABT_SEND_IP_SIZE);

	switch(abt->abt_conn_tool) {
	case ABT_CONN_STUDIO:
		/* Reserved */
		break;
	default:	/* ABT_CONN_TOUCH */
		thread = abt_kthread_run(abt,
					abt_ksocket_thread_for_pctool);
		break;
	}
	if (IS_ERR(thread)) {
		t_abt_err(abt, "unable to start kernel thread\n");
		return -ENOMEM;
	}

	abt_comm->thread = thread;
	abt_comm->sock_listener = listener;

	siw_touch_mon_pause(abt->dev);

	return 0;
}

enum {
	SHOW_ABT_TOOL_MODE_SIZE	= 4,
	SHOW_ABT_TOOL_IP_SIZE	= 16,
	SHOW_ABT_TOOL_RET_SIZE	= (SHOW_ABT_TOOL_MODE_SIZE + SHOW_ABT_TOOL_IP_SIZE),
};

/*
 * binary
 */
static ssize_t abt_show_tool_b(struct device *dev, char *buf)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct siw_ts *ts = chip->ts;
	struct siw_hal_abt_data *abt = (struct siw_hal_abt_data *)ts->abt;
	struct siw_hal_abt_comm *abt_comm = &abt->abt_comm;
	int size = 0;

	buf[0] = abt->abt_report_mode_onoff;
	size = SHOW_ABT_TOOL_MODE_SIZE;

	if (atomic_read(&abt_comm->running) == ABT_RUNNING_OFF) {
		memcpy((u8 *)&buf[1], NONE_IP, strlen(NONE_IP));
		size += strlen(NONE_IP);
		goto out;
	}

	memcpy((u8 *)&buf[1], (u8 *)abt_comm->send_ip,
		SHOW_ABT_TOOL_IP_SIZE);
	size += SHOW_ABT_TOOL_IP_SIZE;

out:
	t_abt_info(abt, "read raw report mode - mode:%d ip:%s\n",
		buf[0], (char *)&buf[1]);

	return (ssize_t)size;
}

/*
 * text (echo)
 */
static ssize_t abt_show_tool_t(struct device *dev, char *buf)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct siw_ts *ts = chip->ts;
	struct siw_hal_abt_data *abt = (struct siw_hal_abt_data *)ts->abt;
	struct siw_hal_abt_comm *abt_comm = &abt->abt_comm;
	int mode = abt->abt_report_mode_onoff;
	int size = 0;

	if (atomic_read(&abt_comm->running) == ABT_RUNNING_OFF) {
		size += siw_snprintf(buf, size,
						"mode:%d, ip:%s\n",
						mode, NONE_IP);
		goto out;
	}

	size += siw_snprintf(buf, size,
						"mode:%d, ip:%s\n",
						mode, abt_comm->send_ip);

out:
	t_abt_info(abt, "read raw report mode - mode:%d ip:%s\n",
		mode, abt_comm->send_ip);

	return (ssize_t)size;
}


#define STORE_ABT_TOOL_MODE_MAX		'0'

enum {
	STORE_ABT_MODE_STUDIO = 1,
	STORE_ABT_MODE_TOUCH,
	STORE_ABT_MODE_MAX,
};

static int abt_tool_do_start(struct siw_hal_abt_data *abt,
				char *ip, int tool,
				abt_sock_listener_t listener)
{
	int ret = 0;

	abt->abt_report_point = 0;
	abt->abt_report_ocd = 0;

	ret = abt_ksocket_init(abt, ip, tool, listener);
	if (ret) {
		t_abt_err(abt, "ksocket init[%d] failed, %d\n",
			abt->abt_conn_tool, ret);
		return ret;
	}
	abt_set_data_func(abt, 1);
	return 0;
}

static int abt_tool_start(struct siw_hal_abt_data *abt,
				char *ip, int tool,
				abt_sock_listener_t listener)
{
	struct siw_hal_abt_comm *abt_comm = &abt->abt_comm;

	if (abt_comm->thread != NULL) {
		if (!memcmp((u8 *)abt_comm->send_ip, (u8 *)ip, ABT_SEND_IP_SIZE)) {
			return 0;
		}

		t_abt_info(abt, "IP changed : ksocket exit, then restart\n");

		abt_ksocket_exit(abt);
	}

	return abt_tool_do_start(abt, ip, tool, listener);
}

static int abt_store_tool_touch(struct siw_hal_abt_data *abt, char *ip)
{
	int ret = 0;

	ret = abt_tool_start(abt, ip,
				ABT_CONN_TOUCH, abt_ksocket_recv_from_pctool);

	return ret;
}

static int abt_store_tool_exit_chk(struct siw_hal_abt_data *abt)
{
	struct siw_hal_abt_comm *abt_comm = &abt->abt_comm;

	if (atomic_read(&abt_comm->running) != ABT_RUNNING_OFF)
		return 1;

	if (abt_comm->thread != NULL)
		return 1;

	if (abt_comm->sock != NULL)
		return 1;

	if (abt_comm->ts_sock != NULL)
		return 1;

	if (abt_comm->sock_send != NULL)
		return 1;

	if (abt_comm->send_connected)
		return 1;

	if (abt_comm->curr_sock != NULL)
		return 1;

	return 0;
}

static int abt_store_tool_exit(struct siw_hal_abt_data *abt, char *ip)
{
//	struct device *dev = abt->dev;

	mutex_lock(&abt->abt_comm_lock);
	if (!abt_store_tool_exit_chk(abt)) {
		mutex_unlock(&abt->abt_comm_lock);
		return 0;
	}
	mutex_unlock(&abt->abt_comm_lock);

	abt_ksocket_exit(abt);
	abt_set_data_func(abt, 0);

	mutex_lock(&abt->abt_comm_lock);
	abt_set_report_mode(abt, 0);
	mutex_unlock(&abt->abt_comm_lock);
	return 0;
}

static ssize_t __abt_store_tool(struct device *dev,
				const char *buf, size_t count, int opt)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct siw_ts *ts = chip->ts;
	struct siw_hal_abt_data *abt = (struct siw_hal_abt_data *)ts->abt;
	int mode = 0;
	char __ip[ABT_SEND_IP_SIZE+1] = { '0', };
	char *ip = NULL;
	bool setFlag = false;
	int ret;

	if (atomic_read(&ts->state.debug_tool) != DEBUG_TOOL_ENABLE){
		t_abt_err(abt, "tool disabled\n");
		return count;
	}

	if (opt) {
		ip = __ip;
		if (sscanf(buf, "%d %s", &mode, ip) <= 0) {
			siw_abt_sysfs_err_invalid_param(abt);
			return count;
		}
	} else {
		mode = buf[0];
		if (mode >= '0') {
			mode -= '0';
		}
		ip = (char *)&buf[1];
	}

	if (mode && mode < STORE_ABT_MODE_MAX) {
		if (!ip[0] || (ip[0] == '0')) {
			t_abt_err(abt, "Invalid IP\n");
			return count;
		}
	}

	t_abt_info(abt,
		"set raw report mode - mode:%d, IP:%s\n",
		mode, ip);

	switch (mode) {
	case STORE_ABT_MODE_STUDIO:
		t_abt_info(abt, "Not supperted\n");
		break;

	case STORE_ABT_MODE_TOUCH:
		ret = abt_store_tool_touch(abt, ip);
		break;

	default:
		ret = abt_store_tool_exit(abt, ip);
		break;
	}

	if (setFlag) {
		mutex_lock(&abt->abt_comm_lock);
		abt_set_report_mode(abt, mode);
		mutex_unlock(&abt->abt_comm_lock);
	}

	if (!opt) {
		if (touch_test_abt_quirks(ts, ABT_QUIRK_RAW_RETURN_MODE_VAL)) {
			return (ssize_t)mode;
		}
	}

	return count;
}

/*
 * binary
 */
static ssize_t abt_store_tool_b(struct device *dev,
				const char *buf, size_t count)
{
	return __abt_store_tool(dev, buf, count, 0);
}

/*
 * text (echo)
 */
static ssize_t abt_store_tool_t(struct device *dev,
				const char *buf, size_t count)
{
	return __abt_store_tool(dev, buf, count, 1);
}

static void __used siw_hal_abt_report_mode(struct device *dev, u8 *all_data)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct siw_ts *ts = chip->ts;
	struct siw_hal_reg *reg = chip->reg;
	struct siw_hal_abt_data *abt = (struct siw_hal_abt_data *)ts->abt;
	struct siw_hal_abt_comm *abt_comm = &abt->abt_comm;
	struct siw_abt_send_data *packet_ptr = NULL;
	struct siw_abt_dbg_report_hdr *d_header = NULL;
	struct siw_hal_touch_info *t_info = NULL;
	struct timeval t_stamp;
	u32 rst_offset_val;
	u32 dbg_offset;
	u8 *d_data_ptr;
	int d_header_size;
	int i;
	int ret = 0;

	packet_ptr = abt_comm->data_send;
	d_header = (struct siw_abt_dbg_report_hdr *)(abt_comm->data_send->data);
	d_header_size = sizeof(struct siw_abt_dbg_report_hdr);
	t_info = (struct siw_hal_touch_info *)all_data;
	rst_offset_val = 1;
	dbg_offset = abt->dbg_offset_base;
	d_data_ptr = (u8 *)d_header + d_header_size;

	if (abt->abt_report_mode) {
		ret = abt_read_memory(abt,
				reg->data_i2cbase_addr,
				reg->serial_data_offset,
				dbg_offset,
				(int)sizeof(u32),
				(u8 *)d_header,
				d_header_size);
		if (ret < 0) {
			t_abt_err(abt,
					"Report reg addr read failed(%d, %d), %d\n",
					dbg_offset,
					(int)sizeof(u32),
					ret);
		}

		dbg_offset	+= (d_header_size>>2);
		if (d_header->type == abt->abt_report_mode) {
			for (i = 0; i < d_header->data_size>>MAX_RW_SIZE_POW; i++) {
				ret = abt_read_memory(abt,
						reg->data_i2cbase_addr,
						reg->serial_data_offset,
						dbg_offset,
						(int)sizeof(u32),
						d_data_ptr,
						sizeof(u8)<<MAX_RW_SIZE_POW);
				if (ret < 0) {
					t_abt_err(abt,
							"Report reg addr read failed(%d, %d), %d\n",
							dbg_offset,
							(int)sizeof(u32),
							ret);
				}

				d_data_ptr += (sizeof(u8)<<MAX_RW_SIZE_POW);
				dbg_offset += (sizeof(u8)<<MAX_RW_SIZE_POW)>>2;
			}

			if (d_header->data_size % MAX_RW_SIZE != 0) {
				ret = abt_read_memory(abt,
						reg->data_i2cbase_addr,
						reg->serial_data_offset,
						dbg_offset,
						(int)sizeof(u32),
						d_data_ptr,
						sizeof(u8)*(d_header->data_size%MAX_RW_SIZE));
				if (ret < 0) {
					t_abt_err(abt,
							"Report reg addr read failed(%d, %d), %d\n",
							dbg_offset,
							(int)sizeof(u32),
							ret);
				}

				d_data_ptr += (sizeof(u8)*(d_header->data_size
					% MAX_RW_SIZE));
				dbg_offset += (sizeof(u8)*(d_header->data_size
					% MAX_RW_SIZE)>>2);
			}
		} else {
			t_abt_err(abt, "debug data load error : type %d, size %d\n",
					d_header->type, d_header->data_size);
		}
		ret = siw_hal_reg_write(dev,
					reg->tc_interrupt_status,
					(void *)&rst_offset_val, sizeof(rst_offset_val));
	} else
		packet_ptr->touchCnt = 0;

	/* ABS0 */
	if (t_info->wakeup_type == 0) {
		if (abt->abt_report_ocd) {
			memcpy(d_data_ptr, &all_data[36<<2], sizeof(u8) * 112);
			d_data_ptr += (sizeof(u8) * 112);
		}

		if (t_info->data[0].track_id != 15) {
			if (abt->abt_report_point) {
				packet_ptr->touchCnt = t_info->touch_cnt;
				memcpy(d_data_ptr, &t_info->data[0],
					sizeof(struct siw_hal_touch_data) * t_info->touch_cnt);
				d_data_ptr += sizeof(struct siw_hal_touch_data)
							* t_info->touch_cnt;
			} else
				packet_ptr->touchCnt = 0;
		}
	}


	if (((u8 *)d_data_ptr) - ((u8 *)packet_ptr) > 0) {
		do_gettimeofday(&t_stamp);

		abt->frame_num++;
		packet_ptr->type = DEBUG_DATA;
		packet_ptr->mode = abt->abt_report_mode;
		packet_ptr->frame_num = abt->frame_num;
		packet_ptr->timestamp =
			t_stamp.tv_sec * 1000000 + t_stamp.tv_usec;

		packet_ptr->flag = 0;
		if (abt->abt_report_point)
			packet_ptr->flag |= 0x1;
		if (abt->abt_report_ocd)
			packet_ptr->flag |= (0x1)<<1;

		abt_ksocket_raw_data_send(abt, (u8 *)packet_ptr,
				(u8 *)d_data_ptr - (u8 *)packet_ptr);
	}
}

static int siw_hal_abt_irq_handler(struct device *dev)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct siw_ts *ts = chip->ts;
	struct siw_hal_reg *reg = chip->reg;
	struct siw_hal_abt_data *abt = (struct siw_hal_abt_data *)ts->abt;
	u8 all_data[264];
	int report_mode = abt_is_set_func(abt);
	int ret = 0;

	if (atomic_read(&chip->init) == IC_INIT_NEED) {
		t_dev_warn(dev, "Not Ready, Need IC init\n");
		return 0;
	}

#if defined(__SIW_SUPPORT_PM_QOS)
	pm_qos_update_request(&chip->pm_qos_req, 10);
#endif
	ret = siw_hal_reg_read(dev,
				reg->tc_ic_status,
			    (void *)all_data, sizeof(all_data));
#if defined(__SIW_SUPPORT_PM_QOS)
	pm_qos_update_request(&chip->pm_qos_req, PM_QOS_DEFAULT_VALUE);
#endif
	if (ret < 0) {
		goto out;
	}

	memcpy(&chip->info, all_data, sizeof(chip->info));

	ret = siw_ops_chk_status(ts);
	if (ret < 0) {
		goto out;
	}

	if (report_mode) {
		if (chip->info.wakeup_type == ABS_MODE)
			ret = siw_ops_irq_abs(ts);
		else
			ret = siw_ops_irq_lpwg(ts);

		siw_hal_abt_report_mode(dev, all_data);
		goto out;
	}

	if (chip->info.wakeup_type == ABS_MODE) {
		ret = siw_ops_irq_abs(ts);
		goto out;
	}

	ret = siw_ops_irq_lpwg(ts);

out:
	return ret;
}

void siw_hal_switch_to_abt_irq_handler(struct siw_ts *ts)
{
	siw_ops_set_irq_handler(ts, (void *)siw_hal_abt_irq_handler);
	t_dev_info(ts->dev, "set new irq handler for ABT\n");
}

#if defined(__SIW_ATTR_PERMISSION_ALL)
#define __TOUCH_ABT_PERM	(S_IRUGO | S_IWUGO)
#else
#define __TOUCH_ABT_PERM	(S_IRUGO | S_IWUSR | S_IWGRP)
#endif

#define SIW_TOUCH_HAL_ABT_ATTR(_name, _show, _store)	\
		__TOUCH_ATTR(_name, __TOUCH_ABT_PERM, _show, _store)

#define _SIW_TOUCH_HAL_ABT_T(_name)	\
		touch_attr_##_name

static SIW_TOUCH_HAL_ABT_ATTR(raw_report, abt_show_tool_b, abt_store_tool_b);
static SIW_TOUCH_HAL_ABT_ATTR(raw_report_t, abt_show_tool_t, abt_store_tool_t);

static struct attribute *siw_hal_abt_attribute_list[] = {
	&_SIW_TOUCH_HAL_ABT_T(raw_report).attr,
	&_SIW_TOUCH_HAL_ABT_T(raw_report_t).attr,
	NULL,
};

static const struct attribute_group __used siw_hal_abt_attribute_group = {
	.attrs = siw_hal_abt_attribute_list,
};

static struct siw_hal_abt_data *siw_hal_abt_alloc(struct device *dev)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct siw_ts *ts = chip->ts;
	struct siw_hal_abt_data *abt = NULL;
	struct siw_abt_send_data *data_send = NULL;
	struct siw_abt_comm_packet *send_packet = NULL;

	abt = touch_kzalloc(dev, sizeof(*abt), GFP_KERNEL);
	if (!abt) {
		t_dev_err(dev,
				"failed to allocate memory for abt\n");
		goto out;
	}

	data_send = touch_kzalloc(dev, sizeof(struct siw_abt_send_data), GFP_KERNEL);
	if (!data_send) {
		t_dev_err(dev,
				"failed to allocate data_send\n");
		goto out_data_send;
	}
	abt->abt_comm.data_send = data_send;

	send_packet = touch_kzalloc(dev, sizeof(struct siw_abt_comm_packet), GFP_KERNEL);
	if (!send_packet) {
		t_dev_err(dev,
				"failed to allocate send_packet\n");
		goto out_send_packet;
	}
	abt->abt_comm.send_packet = send_packet;

	t_dev_dbg_base(dev, "create abt (0x%zX)\n", (size_t)sizeof(*abt));

	abt->dev = ts->dev;

	mutex_init(&abt->abt_comm_lock);
	mutex_init(&abt->abt_socket_lock);
	abt->abt_socket_mutex_flag = 1;

	abt->prev_rnd_piece_no = DEF_RNDCPY_EVERY_NTH_FRAME;

	abt->abt_conn_tool = ABT_CONN_NOTHING;

	abt->set_get_data_func = 0;

	switch (touch_chip_type(ts)) {
	case CHIP_LG4894:
		abt->dbg_offset_base = 0x2A98;
		break;
	case CHIP_LG4895:
		abt->dbg_offset_base = 0x2E10;
		break;
	default:
		abt->dbg_offset_base = 0x3800;
		break;
	}
	abt->dbg_offset_base >>= 2;
	abt->dbg_offset = abt->dbg_offset_base;

	ts->abt = abt;

	return abt;

out_send_packet:
	touch_kfree(dev, send_packet);

out_data_send:
	touch_kfree(dev, abt);

out:
	return NULL;
}

static void siw_hal_abt_free(struct device *dev)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct siw_ts *ts = chip->ts;
	struct siw_hal_abt_data *abt = (struct siw_hal_abt_data *)ts->abt;

	if (abt) {
		t_dev_dbg_base(dev, "free abt\n");

		abt_store_tool_exit(abt, NULL);

		mutex_destroy(&abt->abt_comm_lock);
		mutex_destroy(&abt->abt_socket_lock);
		ts->abt = NULL;

		touch_kfree(dev, abt->abt_comm.send_packet);
		touch_kfree(dev, abt->abt_comm.data_send);
		touch_kfree(dev, abt);
	}
}

int siw_hal_abt_init(struct device *dev)
{
	/* Reserved */

	return 0;
}

static int siw_hal_abt_create_sysfs(struct device *dev)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct siw_ts *ts = chip->ts;
	struct device *idev = &ts->input->dev;
	struct kobject *kobj = &ts->kobj;
	struct siw_hal_abt_data *abt;
	int ret = 0;

	if (kobj->parent != idev->kobj.parent) {
		t_dev_err(dev, "Invalid kobject\n");
		return -EINVAL;
	}

	abt = siw_hal_abt_alloc(dev);
	if (!abt) {
		ret = -ENOMEM;
		goto out;
	}

	ret = sysfs_create_group(kobj, &siw_hal_abt_attribute_group);
	if (ret < 0) {
		t_dev_err(dev, "%s abt sysfs register failed, %d\n",
				touch_chip_name(ts), ret);
		goto out_sysfs;
	}

	t_dev_dbg_base(dev, "%s abt sysfs registered\n",
			touch_chip_name(ts));

	return 0;

out_sysfs:
	siw_hal_abt_free(dev);

out:
	return ret;
}

static void siw_hal_abt_remove_sysfs(struct device *dev)
{
	struct siw_touch_chip *chip = to_touch_chip(dev);
	struct siw_ts *ts = chip->ts;
	struct device *idev = &ts->input->dev;
	struct kobject *kobj = &ts->kobj;

	if (kobj->parent != idev->kobj.parent) {
		t_dev_err(dev, "Invalid kobject\n");
		return;
	}

	if (ts->abt == NULL) {
		return;
	}

	sysfs_remove_group(kobj, &siw_hal_abt_attribute_group);

	siw_hal_abt_free(dev);

	t_dev_dbg_base(dev, "abt sysfs unregistered\n");
}

int siw_hal_abt_sysfs(struct device *dev, int on_off)
{
	if (on_off == DRIVER_INIT) {
		return siw_hal_abt_create_sysfs(dev);
	}

	siw_hal_abt_remove_sysfs(dev);
	return 0;

}

__siw_setup_u32("siw_abt_port_default=", abt_setup_port_default, t_abt_port_default);
__siw_setup_u32("siw_abt_port_tcp=", abt_setup_port_tcp, t_abt_port_tcp);
__siw_setup_u32("siw_abt_port_send=", abt_setup_port_send, t_abt_port_send);

#endif	/* __SIW_SUPPORT_ABT */


