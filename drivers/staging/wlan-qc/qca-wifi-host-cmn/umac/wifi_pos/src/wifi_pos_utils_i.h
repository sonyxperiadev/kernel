/*
 * Copyright (c) 2012-2020 The Linux Foundation. All rights reserved.
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

/**
 * DOC: wifi_pos_utils_i.h
 * This file defines the prototypes for the utility helper functions
 * for the wifi_pos component.
 */

#ifdef WIFI_POS_CONVERGED
#ifndef _WIFI_POS_UTILS_H_
#define _WIFI_POS_UTILS_H_
/* Include files */
#include "qdf_types.h"
#include "qdf_status.h"
#include "ol_defines.h"
#include "qdf_trace.h"
#include "qdf_module.h"
#include "wifi_pos_utils_pub.h"
#include "wlan_cmn.h"

struct wlan_objmgr_psoc;
struct wifi_pos_req_msg;

#define wifi_pos_alert(params...) \
	QDF_TRACE_FATAL(QDF_MODULE_ID_WIFIPOS, params)
#define wifi_pos_err(params...) \
	QDF_TRACE_ERROR(QDF_MODULE_ID_WIFIPOS, params)
#define wifi_pos_warn(params...) \
	QDF_TRACE_WARN(QDF_MODULE_ID_WIFIPOS, params)
#define wifi_pos_notice(params...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_WIFIPOS, params)
#define wifi_pos_debug(params...) \
	QDF_TRACE_DEBUG(QDF_MODULE_ID_WIFIPOS, params)

#define wifipos_nofl_alert(params...) \
	QDF_TRACE_FATAL_NO_FL(QDF_MODULE_ID_WIFIPOS, params)
#define wifipos_nofl_err(params...) \
	QDF_TRACE_ERROR_NO_FL(QDF_MODULE_ID_WIFIPOS, params)
#define wifipos_nofl_warn(params...) \
	QDF_TRACE_WARN_NO_FL(QDF_MODULE_ID_WIFIPOS, params)
#define wifipos_nofl_notice(params...) \
	QDF_TRACE_INFO_NO_FL(QDF_MODULE_ID_WIFIPOS, params)
#define wifipos_nofl_debug(params...) \
	QDF_TRACE_DEBUG_NO_FL(QDF_MODULE_ID_WIFIPOS, params)

#define OEM_APP_SIGNATURE_LEN      16
#define OEM_APP_SIGNATURE_STR      "QUALCOMM-OEM-APP"

#ifndef OEM_DATA_RSP_SIZE
#define OEM_DATA_RSP_SIZE 1724
#endif

/**
 * struct app_reg_rsp_vdev_info - vdev info struct
 * @dev_mode: device mode
 * @vdev_id: vdev id
 *
 */
struct qdf_packed app_reg_rsp_vdev_info {
	uint8_t dev_mode;
	uint8_t vdev_id;
};

/**
 * struct wifi_app_reg_rsp - app registration response struct
 * @num_inf: number of interfaces active
 * @vdevs: array indicating all active vdev's information
 *
 */
struct qdf_packed wifi_app_reg_rsp {
	uint8_t num_inf;
	struct app_reg_rsp_vdev_info vdevs[1];
};

/**
 * struct oem_data_req - data request to be sent to firmware
 * @data_len: len of data
 * @data: buffer containing data
 *
 */
struct oem_data_req {
	uint32_t data_len;
	uint8_t *data;
};

/**
 * struct oem_data_rsp - response from firmware to data request sent earlier
 * @rsp_len_1: len of data_1
 * @data_1: first part of payload
 * @rsp_len_2: len of data_2
 * @data_2: second part of payload
 * @dma_len: len of DMAed data
 * @vaddr: virtual address of DMA data start
 *
 */
struct oem_data_rsp {
	uint32_t rsp_len_1;
	uint8_t *data_1;
	uint32_t rsp_len_2;
	uint8_t *data_2;
	uint32_t dma_len;
	void *vaddr;
};

/**
 * struct wifi_pos_err_rpt - Error report response for userspace.
 * @tag_len: tlv header of the message.
 * @info: Report info. Reserved for error report.
 * @dest_mac: Mac address of the sta in the request.
 * @reserved: Reserved in error report.
 */
struct qdf_packed wifi_pos_err_rpt {
	uint32_t tag_len;
	uint32_t info;
	uint8_t  dest_mac[QDF_MAC_ADDR_SIZE + 2];
	uint32_t reserved;
};

#define OEM_MSG_RSP_HEAD_TAG_ID 33
#define OEM_MEAS_RSP_HEAD_TAG_ID 41
/**
 * struct wifi_pos_err_msg_report - Error report message
 * @msg_tag_len: Message tlv header
 * @msg_subtype: Message subtype
 * @req_id: id corresponding to the request.
 * @fragment_info: Valid only for fragments.
 * @pdev_id: pdev_id of radion.
 * @time_left: time left in the measurment req.
 * @err_rpt: Error report data.
 */
struct qdf_packed wifi_pos_err_msg_report {
	uint32_t msg_tag_len;
	uint32_t msg_subtype;
	uint32_t req_id;
	uint32_t fragment_info;
	uint32_t pdev_id;
	uint32_t time_left;
	struct wifi_pos_err_rpt err_rpt;
};

/**
 * struct wifi_pos_dma_rings_cap - capabilities requested by firmware.
 * @pdev_id: pdev_id or mac_id of ring
 * @min_num_ptr: minimum depth of ring required
 * @min_buf_size: minimum size of each buffer
 * @min_buf_align: minimum allignment of buffer memory
 */
struct wifi_pos_dma_rings_cap {
	uint32_t pdev_id;
	uint32_t min_num_ptr;
	uint32_t min_buf_size;
	uint32_t min_buf_align;
};

/**
 * struct wifi_pos_dma_buf_info - buffer info struct containing phy to virtual
 * mapping.
 * @cookie: this identifies location of DMA buffer in pool array
 * @paddr: aligned physical address as exchanged with firmware
 * @vaddr: virtual address - unaligned. this helps in freeing later
 * @offset: offset of aligned address from unaligned
 */
struct wifi_pos_dma_buf_info {
	uint32_t cookie;
	void *paddr;
	void *vaddr;
	uint8_t offset;
};

/**
 * struct wifi_pos_dma_rings_cfg - DMA ring parameters to be programmed to FW.
 * @pdev_id: pdev_id of ring
 * @num_ptr: depth of ring
 * @base_paddr_unaligned: base physical addr unaligned
 * @base_vaddr_unaligned: base virtual addr unaligned
 * @base_paddr_aligned: base physical addr aligned
 * @base_vaddr_aligned: base virtual addr unaligned
 * @head_idx_addr: head index addr
 * @tail_idx_addr: tail index addr
 * @srng: hal srng
 */
struct wifi_pos_dma_rings_cfg {
	uint32_t pdev_id;
	uint32_t num_ptr;
	uint32_t ring_alloc_size;
	void *base_paddr_unaligned;
	void *base_vaddr_unaligned;
	void *base_paddr_aligned;
	void *base_vaddr_aligned;
	void *head_idx_addr;
	void *tail_idx_addr;
	void *srng;
};

typedef void (*wifi_pos_send_rsp_handler)(struct wlan_objmgr_psoc *, uint32_t,
					  enum wifi_pos_cmd_ids,
					  uint32_t, uint8_t *);

/**
 * struct wifi_pos_psoc_priv_obj - psoc obj data for wifi_pos
 * @app_pid: pid of app registered to host driver
 * @is_app_registered: indicates if app is registered
 * @fine_time_meas_cap: FTM cap for different roles, reflection of ini
 * @ftm_rr: configured value of FTM Ranging Request capability
 * @lci_capability: configured value of LCI capability
 * @rsvd: reserved
 * @oem_target_type
 * @oem_target_type: oem target type, populated from HDD
 * @oem_fw_version: firmware version, populated from HDD
 * @driver_version: driver version, populated from HDD
 * @allowed_dwell_time_min: allowed dwell time min, populated from HDD
 * @allowed_dwell_time_max: allowed dwell time max, populated from HDD
 * @current_dwell_time_min: current dwell time min, populated from HDD
 * @current_dwell_time_max: current dwell time max, populated from HDD
 * @hal_soc: hal_soc
 * @num_rings: DMA ring cap requested by firmware
 * @dma_cap: dma cap as read from service ready ext event
 * @dma_cfg: DMA ring cfg to be programmed to firmware
 * @dma_buf_pool: DMA buffer pools maintained at host: this will be 2-D array
 * where with num_rows = number of rings num_elements in each row = ring depth
 * @wifi_pos_lock: lock to access wifi pos priv object
 * @oem_6g_support_disable: oem target 6ghz support is disabled if set
 * @wifi_pos_req_handler: function pointer to handle TLV or non-TLV
 * @wifi_pos_send_rsp: function pointer to send msg to userspace APP
 * @wifi_pos_get_phy_mode: function pointer to get wlan phymode for given
 *                         channel, channel width
 * @wifi_pos_get_fw_phy_mode_for_freq: function pointer to get fw phymode
 *                                     for given freq and channel width
 * @wifi_pos_send_action: function pointer to send registered action frames
 *                        to userspace APP
 * @wifi_pos_get_pdev_id_by_dev_name: get pdev_id from device name
 * @rsp_version: rsp version
 *
 * wifi pos request messages
 * <----- fine_time_meas_cap (in bits) ----->
 *+----------+-----+-----+------+------+-------+-------+-----+-----+
 *|   8-31   |  7  |  6  |   5  |   4  |   3   |   2   |  1  |  0  |
 *+----------+-----+-----+------+------+-------+-------+-----+-----+
 *| reserved | SAP | SAP |P2P-GO|P2P-GO|P2P-CLI|P2P-CLI| STA | STA |
 *|          |resp |init |resp  |init  |resp   |init   |resp |init |
 *+----------+-----+-----+------+------+-------+-------+-----+-----+
 * resp - responder role; init- initiator role
 *
 */
struct wifi_pos_psoc_priv_obj {
	uint32_t app_pid;
	bool is_app_registered;
	uint32_t fine_time_meas_cap;
	uint32_t ftm_rr:1;
	uint32_t lci_capability:1;
	uint32_t rsvd:30;

	uint32_t oem_target_type;
	uint32_t oem_fw_version;
	struct wifi_pos_driver_version driver_version;
	uint16_t allowed_dwell_time_min;
	uint16_t allowed_dwell_time_max;
	uint16_t current_dwell_time_min;
	uint16_t current_dwell_time_max;

	void *hal_soc;
	uint8_t num_rings;
	struct wifi_pos_dma_rings_cap *dma_cap;
	struct wifi_pos_dma_rings_cfg *dma_cfg;
	struct wifi_pos_dma_buf_info **dma_buf_pool;

	qdf_spinlock_t wifi_pos_lock;
	bool oem_6g_support_disable;
	QDF_STATUS (*wifi_pos_req_handler)(struct wlan_objmgr_psoc *psoc,
				    struct wifi_pos_req_msg *req);
	wifi_pos_send_rsp_handler wifi_pos_send_rsp;
	void (*wifi_pos_get_phy_mode)(qdf_freq_t freq, uint32_t chan_width,
				      enum wlan_phymode *phy_mode);
	void (*wifi_pos_get_fw_phy_mode_for_freq)(uint32_t, uint32_t,
						  uint32_t *);
	void (*wifi_pos_send_action)(struct wlan_objmgr_psoc *psoc,
				     uint32_t oem_subtype, uint8_t *buf,
				     uint32_t len);
	QDF_STATUS (*wifi_pos_get_pdev_id_by_dev_name)(
			char *dev_name, uint8_t *pdev_id,
			struct wlan_objmgr_psoc **psoc);
	uint32_t rsp_version;
};

/**
 * wifi_pos_get_psoc_priv_obj: API to get wifi_psoc private object
 * @psoc: pointer to psoc object
 *
 * Return: psoc private object on success, NULL otherwise
 */
struct wifi_pos_psoc_priv_obj *wifi_pos_get_psoc_priv_obj(
		struct wlan_objmgr_psoc *psoc);

/**
 * wifi_pos_lock_init: API to init lock used protect use of psoc global pointer
 * variable
 *
 * Return: none.
 */
void wifi_pos_lock_init(void);

/**
 * wifi_pos_lock_deinit: API to deinit lock used protect use of psoc global
 * pointer variable
 *
 * Return: none.
 */
void wifi_pos_lock_deinit(void);

/**
 * wifi_pos_set_psoc: API to set global PSOC object
 * @psoc: pointer to psoc object
 *
 * Since request from userspace is not associated with any vdev/pdev/psoc, this
 * API is used to set global psoc object.
 *
 * Return: none.
 */
void wifi_pos_set_psoc(struct wlan_objmgr_psoc *psoc);

/**
 * wifi_pos_get_psoc: API to get global PSOC object
 *
 * Since request from userspace is not associated with any vdev/pdev/psoc, this
 * API is used to get global psoc object.
 * Return: global psoc object.
 */
struct wlan_objmgr_psoc *wifi_pos_get_psoc(void);

/**
 * wifi_pos_get_psoc: API to clear global PSOC object
 *
 * Return: none.
 */
void wifi_pos_clear_psoc(void);

/**
 * wifi_pos_populate_caps: API to get OEM caps
 * @psoc: psoc object
 * @caps: capabilities buffer to populate
 *
 * Return: status of operation.
 */
QDF_STATUS wifi_pos_populate_caps(struct wlan_objmgr_psoc *psoc,
				  struct wifi_pos_driver_caps *caps);

/**
 * wifi_pos_get_app_pid: returns oem app pid.
 * @psoc: pointer to psoc object
 *
 * Return: oem app pid
 */
uint32_t wifi_pos_get_app_pid(struct wlan_objmgr_psoc *psoc);

/**
 * wifi_pos_is_app_registered: indicates if oem app is registered.
 * @psoc: pointer to psoc object
 *
 * Return: true if app is registered, false otherwise
 */
bool wifi_pos_is_app_registered(struct wlan_objmgr_psoc *psoc);

#endif /* _WIFI_POS_UTILS_H_ */
#endif /* WIFI_POS_CONVERGED */
