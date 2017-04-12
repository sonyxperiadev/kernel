/*
* Copyright (c) 2014-2015 The Linux Foundation. All rights reserved.
*
* Previously licensed under the ISC license by Qualcomm Atheros, Inc.
*
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

/*
* This file was originally distributed by Qualcomm Atheros, Inc.
* under proprietary terms before Copyright ownership was assigned
* to the Linux Foundation.
*/

/******************************************************************************
 * wlan_logging_sock_svc.h
 *
 ******************************************************************************/

#ifndef WLAN_LOGGING_SOCK_SVC_H
#define WLAN_LOGGING_SOCK_SVC_H

#include <wlan_nlink_srv.h>
#include <vos_status.h>
#include <wlan_hdd_includes.h>
#include <vos_trace.h>
#include <wlan_nlink_common.h>


int wlan_logging_sock_init_svc(void);
int wlan_logging_sock_deinit_svc(void);
int wlan_logging_flush_pkt_queue(void);
int wlan_logging_sock_deactivate_svc(void);
int wlan_log_to_user(VOS_TRACE_LEVEL log_level, char *to_be_sent, int length);
int wlan_queue_logpkt_for_app(vos_pkt_t *pPacket, uint32 pkt_type);
void wlan_process_done_indication(uint8 type, uint32 reason_code);
int wlan_logging_sock_activate_svc(int log_fe_to_console, int num_buf,
                             int pkt_stats_enabled, int pkt_stats_buff);
void wlan_flush_host_logs_for_fatal(void);

void wlan_init_log_completion(void);
int wlan_set_log_completion(uint32 is_fatal,
                            uint32 indicator,
                            uint32 reason_code);
void wlan_get_log_and_reset_completion(uint32 *is_fatal,
                             uint32 *indicator,
                             uint32 *reason_code,
                             bool reset);
bool wlan_is_log_report_in_progress(void);
void wlan_reset_log_report_in_progress(void);

void wlan_deinit_log_completion(void);

void wlan_logging_set_log_level(void);

#define FW_MEM_DUMP_MAGIC 0x3C3A2D44

enum FW_MEM_DUMP_STATE{
       FW_MEM_DUMP_IDLE,
       FW_MEM_DUMP_READ_IN_PROGRESS,
       FW_MEM_DUMP_WRITE_IN_PROGRESS,
       FW_MEM_DUMP_WRITE_DONE,
};
int wlan_fwr_mem_dump_buffer_allocation(void);
bool wlan_fwr_mem_dump_test_and_set_write_allowed_bit(void);
bool wlan_fwr_mem_dump_test_and_set_read_allowed_bit(void);
void wlan_set_fwr_mem_dump_state(enum FW_MEM_DUMP_STATE fw_mem_dump_state);
void wlan_set_svc_fw_mem_dump_req_cb(void*,void*);
size_t wlan_fwr_mem_dump_fsread_handler(char __user *buf, size_t count, loff_t *pos,loff_t* bytes_left);
void wlan_indicate_mem_dump_complete(bool );
void wlan_store_fwr_mem_dump_size(uint32 dump_size);
void wlan_free_fwr_mem_dump_buffer(void);

bool wlan_is_logger_thread(int threadId);
int wlan_pkt_stats_to_user(void *perPktStat);
void wlan_disable_and_flush_pkt_stats(void);
 void wlan_fillTxStruct(void *pktStat);
 bool wlan_isPktStatsEnabled(void);
#ifdef FEATURE_WLAN_DIAG_SUPPORT
void wlan_report_log_completion(uint32 is_fatal,
      uint32 indicator,
      uint32 reason_code);
#else
static inline void wlan_report_log_completion(uint32 is_fatal,
        uint32 indicator,
        uint32 reason_code)
{
    return;
}

#endif


#endif /* WLAN_LOGGING_SOCK_SVC_H */
