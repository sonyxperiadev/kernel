/*
 * Copyright (c) 2014-2020 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef EPPING_INTERNAL_H
#define EPPING_INTERNAL_H
/**===========================================================================

   \file  epping_internal.h

   \brief Linux epping internal head file

   ==========================================================================*/

/*---------------------------------------------------------------------------
   Include files
   -------------------------------------------------------------------------*/

#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#if defined(WLAN_OPEN_SOURCE) && defined(CONFIG_HAS_WAKELOCK)
#include <linux/wakelock.h>
#endif
#include "htc_api.h"
#include "htc_packet.h"
#include "epping_test.h"
#include <qdf_atomic.h>
#include <sir_mac_prot_def.h>
#include <sir_debug.h>

#define EPPING_LOG_MASK (1<<EPPING_CMD_CAPTURE_RECV_CNT)
#define EPPING_STATS_LOG_COUNT 50000
#define EPPING_KTID_KILL_WAIT_TIME_MS 50

#define EPPING_FRAG_PER_MSDU   1
#ifndef EPPING_TXBUF
#define EPPING_TXBUF   (512/EPPING_FRAG_PER_MSDU)
#endif

/*---------------------------------------------------------------------------
   Preprocessor definitions and constants
   -------------------------------------------------------------------------*/
#define EPPING_MAX_ADAPTERS             1

#define EPPING_LOG(level, args ...) QDF_TRACE(QDF_MODULE_ID_HDD, level, ## args)
#define EPPING_HEX_DUMP(level, data, len) qdf_trace_hex_dump( \
						QDF_MODULE_ID_HDD, \
						level, \
						data, buf_len)

struct epping_cookie {
	HTC_PACKET HtcPkt;      /* HTC packet wrapper */
	struct epping_cookie *next;
};

typedef enum {
	EPPING_CTX_STATE_INITIAL = 0,
	EPPING_CTX_STATE_HIF_INIT,
	EPPING_CTX_STATE_STARTUP,
	EPPING_CTX_STATE_STARTED,
	EPPING_CTX_STATE_STOP
} epping_ctx_state_t;

#define EPPING_MAX_NUM_EPIDS 4
#define MAX_COOKIE_SLOTS_NUM 4
#define MAX_COOKIE_SLOT_SIZE 512
#define MAX_TX_PKT_DUP_NUM   4

#if defined(HIF_PCI) || defined(HIF_IPCI)
#define WLAN_EPPING_DELAY_TIMEOUT_US     10
#define EPPING_MAX_CE_NUMS               8
#define EPPING_MAX_WATER_MARK            8
typedef struct {
	struct task_struct *pid;
	void *arg;
	bool done;
	qdf_nbuf_t skb;
	HTC_ENDPOINT_ID eid;
	struct semaphore sem;
	bool inited;
	qdf_atomic_t atm;
} epping_poll_t;
#endif

typedef struct epping_context {
	int32_t con_mode;
	char *pwlan_module_name;
	uint32_t target_type;
	void *p_cds_context;    /* CDS context */
	struct device *parent_dev;      /* Pointer to the parent device */
	epping_ctx_state_t e_ctx_state;
	bool wow_nack;
	void *epping_adapter;
	HTC_HANDLE HTCHandle;
	HTC_ENDPOINT_ID EppingEndpoint[EPPING_MAX_NUM_EPIDS];
	unsigned int kperf_num_rx_recv[EPPING_MAX_NUM_EPIDS];
	unsigned int kperf_num_tx_acks[EPPING_MAX_NUM_EPIDS];
	unsigned int total_rx_recv;
	unsigned int total_tx_acks;
#if defined(HIF_PCI) || defined(HIF_IPCI)
	epping_poll_t epping_poll[EPPING_MAX_NUM_EPIDS];
#endif
	struct epping_cookie *cookie_list;
	int cookie_count;
	struct epping_cookie *s_cookie_mem[MAX_COOKIE_SLOTS_NUM];
	qdf_spinlock_t cookie_lock;
} epping_context_t;

typedef enum {
	EPPING_TX_TIMER_STOPPED,
	EPPING_TX_TIMER_RUNNING
} epping_tx_timer_state_t;

typedef struct epping_adapter_s {
	epping_context_t *pEpping_ctx;
	enum QDF_OPMODE device_mode;
	/** Handle to the network device */
	struct net_device *dev;
	struct qdf_mac_addr macAddressCurrent;
	uint8_t sessionId;
	/* for mboxping */
	qdf_spinlock_t data_lock;
	qdf_nbuf_queue_t nodrop_queue;
	qdf_timer_t epping_timer;
	epping_tx_timer_state_t epping_timer_state;
	bool registered;
	bool started;
	struct net_device_stats stats;
} epping_adapter_t;

/* epping_helper signatures */
int epping_cookie_init(epping_context_t *pEpping_ctx);
void epping_cookie_cleanup(epping_context_t *pEpping_ctx);
void epping_free_cookie(epping_context_t *pEpping_ctx,
			struct epping_cookie *cookie);
struct epping_cookie *epping_alloc_cookie(epping_context_t *pEpping_ctx);
void epping_get_dummy_mac_addr(tSirMacAddr macAddr);
void epping_hex_dump(void *data, int buf_len, const char *str);
void *epping_get_qdf_ctx(void);
void epping_log_packet(epping_adapter_t *adapter,
		       EPPING_HEADER *eppingHdr, int ret, const char *str);
void epping_log_stats(epping_adapter_t *adapter, const char *str);
void epping_set_kperf_flag(epping_adapter_t *adapter,
			   HTC_ENDPOINT_ID eid, uint8_t kperf_flag);

/* epping_tx signatures */
void epping_tx_timer_expire(epping_adapter_t *adapter);
void epping_tx_complete(void *ctx, HTC_PACKET *htc_pkt);
int epping_tx_send(qdf_nbuf_t skb, epping_adapter_t *adapter);

#ifdef HIF_SDIO
enum htc_send_full_action epping_tx_queue_full(void *Context,
						struct _HTC_PACKET *pPacket);
#endif
void epping_tx_dup_pkt(epping_adapter_t *adapter,
		       HTC_ENDPOINT_ID eid, qdf_nbuf_t skb);
/* epping_rx signatures */
void epping_rx(void *Context, HTC_PACKET *pPacket);

#ifdef HIF_SDIO
void epping_refill(void *ctx, HTC_ENDPOINT_ID Endpoint);
#endif

/* epping_txrx signatures */
epping_adapter_t *epping_add_adapter(epping_context_t *pEpping_ctx,
				     tSirMacAddr macAddr,
				     enum QDF_OPMODE device_mode,
				     bool rtnl_held);
void epping_destroy_adapter(epping_adapter_t *adapter);
int epping_connect_service(epping_context_t *pEpping_ctx);
#if defined(HIF_PCI) || defined(HIF_IPCI)
void epping_register_tx_copier(HTC_ENDPOINT_ID eid,
			       epping_context_t *pEpping_ctx);
void epping_unregister_tx_copier(HTC_ENDPOINT_ID eid,
				 epping_context_t *pEpping_ctx);
void epping_tx_copier_schedule(epping_context_t *pEpping_ctx,
			       HTC_ENDPOINT_ID eid, qdf_nbuf_t skb);
#endif /* HIF_PCI || HIF_IPCI */
#endif /* end #ifndef EPPING_INTERNAL_H */
