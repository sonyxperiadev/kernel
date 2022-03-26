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
 * DOC: declares vendor commands interfacing with linux kernel
 */


#ifndef _WLAN_QCA_VENDOR_H_
#define _WLAN_QCA_VENDOR_H_

/* Vendor id to be used in vendor specific command and events
 * to user space.
 * NOTE: The authoritative place for definition of QCA_NL80211_VENDOR_ID,
 * vendor subcmd definitions prefixed with QCA_NL80211_VENDOR_SUBCMD, and
 * qca_wlan_vendor_attr is open source file src/common/qca-vendor.h in
 * git://w1.fi/srv/git/hostap.git; the values here are just a copy of that
 */

#define QCA_NL80211_VENDOR_ID           0x001374

#ifndef BIT
#define BIT(x) (1U << (x))
#endif

/**
 * enum qca_nl80211_vendor_subcmds: NL 80211 vendor sub command
 *
 * @QCA_NL80211_VENDOR_SUBCMD_UNSPEC: Unspecified
 * @QCA_NL80211_VENDOR_SUBCMD_TEST: Test
 *	Sub commands 2 to 8 are not used
 * @QCA_NL80211_VENDOR_SUBCMD_ROAMING: Roaming
 * @QCA_NL80211_VENDOR_SUBCMD_AVOID_FREQUENCY: Avoid frequency.
 * @QCA_NL80211_VENDOR_SUBCMD_DFS_CAPABILITY: DFS capability
 * @QCA_NL80211_VENDOR_SUBCMD_NAN: NAN command/event which is used to pass
 *	NAN Request/Response and NAN Indication messages. These messages are
 *	interpreted between the framework and the firmware component. While
 *	sending the command from userspace to the driver, payload is not
 *	encapsulated inside any attribute. Attribute QCA_WLAN_VENDOR_ATTR_NAN
 *	is used when receiving vendor events in userspace from the driver.
 * @QCA_NL80211_VENDOR_SUBCMD_STATS_EXT: Ext stats
 * @QCA_NL80211_VENDOR_SUBCMD_LL_STATS_SET: Link layer stats set
 * @QCA_NL80211_VENDOR_SUBCMD_LL_STATS_GET: Link layer stats get
 * @QCA_NL80211_VENDOR_SUBCMD_LL_STATS_CLR: Link layer stats clear
 * @QCA_NL80211_VENDOR_SUBCMD_LL_STATS_RADIO_RESULTS: Link layer stats radio
 *	results
 * @QCA_NL80211_VENDOR_SUBCMD_LL_STATS_IFACE_RESULTS: Link layer stats interface
 *	results
 * @QCA_NL80211_VENDOR_SUBCMD_LL_STATS_PEERS_RESULTS: Link layer stats peer
 *	results
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_START: Ext scan start
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_STOP: Ext scan stop
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_VALID_CHANNELS: Ext scan get valid
 *	channels
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_CAPABILITIES: Ext scan get capability
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_CACHED_RESULTS: Ext scan get cached
 *	results
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SCAN_RESULTS_AVAILABLE: Ext scan results
 *	available. Used when report_threshold is reached in scan cache.
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_FULL_SCAN_RESULT: Ext scan full scan
 *	result. Used to report scan results when each probe rsp. is received,
 *	if report_events enabled in wifi_scan_cmd_params.
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SCAN_EVENT: Ext scan event from target.
 *	Indicates progress of scanning state-machine.
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_AP_FOUND: Ext scan hotlist
 *	ap found
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_BSSID_HOTLIST: Ext scan set hotlist
 *	bssid
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_BSSID_HOTLIST: Ext scan reset
 *	hotlist bssid
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SIGNIFICANT_CHANGE: Ext scan significant
 *	change
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_SIGNIFICANT_CHANGE: Ext scan set
 *	significant change
 *	ap found
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_SIGNIFICANT_CHANGE: Ext scan reset
 *	significant change
 * @QCA_NL80211_VENDOR_SUBCMD_TDLS_ENABLE: Ext tdls enable
 * @QCA_NL80211_VENDOR_SUBCMD_TDLS_DISABLE: Ext tdls disable
 * @QCA_NL80211_VENDOR_SUBCMD_TDLS_GET_STATUS: Ext tdls get status
 * @QCA_NL80211_VENDOR_SUBCMD_TDLS_STATE: Ext tdls state
 * @QCA_NL80211_VENDOR_SUBCMD_GET_SUPPORTED_FEATURES: Get supported features
 * @QCA_NL80211_VENDOR_SUBCMD_SCANNING_MAC_OUI: Set scanning_mac_oui
 * @QCA_NL80211_VENDOR_SUBCMD_NO_DFS_FLAG: No DFS flag
 * @QCA_NL80211_VENDOR_SUBCMD_GET_CONCURRENCY_MATRIX: Get Concurrency Matrix
 * @QCA_NL80211_VENDOR_SUBCMD_KEY_MGMT_SET_KEY: Get the key mgmt offload keys
 * @QCA_NL80211_VENDOR_SUBCMD_KEY_MGMT_ROAM_AUTH: After roaming, send the
 * roaming and auth information.
 * @QCA_NL80211_VENDOR_SUBCMD_OCB_SET_SCHED: Set OCB schedule
 *
 * @QCA_NL80211_VENDOR_SUBCMD_DO_ACS: ACS command/event which is used to
 *	invoke the ACS function in device and pass selected channels to
 *	hostapd. Uses enum qca_wlan_vendor_attr_acs_offload attributes.
 *
 * @QCA_NL80211_VENDOR_SUBCMD_GET_FEATURES: Get the supported features by the
 * driver.
 * @QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_STARTED: Indicate that driver
 *	started CAC on DFS channel
 * @QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_FINISHED: Indicate that driver
 * 	completed the CAC check on DFS channel
 * @QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_ABORTED: Indicate that the CAC
 * 	check was aborted by the driver
 * @QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_NOP_FINISHED: Indicate that the
 * 	driver completed NOP
 * @QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_RADAR_DETECTED: Indicate that the
 * 	driver detected radar signal on the current operating channel
 * @QCA_NL80211_VENDOR_SUBCMD_GET_WIFI_INFO: get wlan driver information
 * @QCA_NL80211_VENDOR_SUBCMD_WIFI_LOGGER_START: start wifi logger
 * @QCA_NL80211_VENDOR_SUBCMD_WIFI_LOGGER_MEMORY_DUMP: memory dump request
 * @QCA_NL80211_VENDOR_SUBCMD_GET_LOGGER_FEATURE_SET: get logger feature set
 * @QCA_NL80211_VENDOR_SUBCMD_ROAM: roam
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_SSID_HOTLIST: extscan set ssid hotlist
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_SSID_HOTLIST:
 *	extscan reset ssid hotlist
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_SSID_FOUND: hotlist ssid found
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_SSID_LOST: hotlist ssid lost
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_SET_LIST: set pno list
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_SET_PASSPOINT_LIST: set passpoint list
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_RESET_PASSPOINT_LIST:
 *	reset passpoint list
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_NETWORK_FOUND: pno network found
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_PASSPOINT_NETWORK_FOUND:
 *	passpoint network found
 * @QCA_NL80211_VENDOR_SUBCMD_SET_WIFI_CONFIGURATION: set wifi config
 * @QCA_NL80211_VENDOR_SUBCMD_GET_WIFI_CONFIGURATION: get wifi config
 * @QCA_NL80211_VENDOR_SUBCMD_GET_LOGGER_FEATURE_SET: get logging features
 * @QCA_NL80211_VENDOR_SUBCMD_LINK_PROPERTIES: get link properties
 * @QCA_NL80211_VENDOR_SUBCMD_GW_PARAM_CONFIG: set gateway parameters
 * @QCA_NL80211_VENDOR_SUBCMD_GET_PREFERRED_FREQ_LIST: get preferred channel
	list
 * @QCA_NL80211_VENDOR_SUBCMD_SET_PROBABLE_OPER_CHANNEL: channel hint
 * @QCA_NL80211_VENDOR_SUBCMD_SETBAND: Command to configure the band
 *	to the host driver. This command sets the band through either
 *	the attribute QCA_WLAN_VENDOR_ATTR_SETBAND_VALUE or
 *	QCA_WLAN_VENDOR_ATTR_SETBAND_MASK. QCA_WLAN_VENDOR_ATTR_SETBAND_VALUE
 *	refers enum qca_set_band as unsigned integer values and
 *	QCA_WLAN_VENDOR_ATTR_SETBAND_MASK refers it as 32 bit unsigned BitMask
 *	values. Also, the acceptable values for
 *	QCA_WLAN_VENDOR_ATTR_SETBAND_VALUE are only till QCA_SETBAND_2G. Further
 *	values/bitmask's are valid for QCA_WLAN_VENDOR_ATTR_SETBAND_MASK. The
 *	attribute QCA_WLAN_VENDOR_ATTR_SETBAND_VALUE is deprecated and the
 *	recommendation is to use the QCA_WLAN_VENDOR_ATTR_SETBAND_MASK. If the
 *	implementations configure using both the attributes, the configurations
 *	through QCA_WLAN_VENDOR_ATTR_SETBAND_MASK shall always take the
 *	precedence.
 * @QCA_NL80211_VENDOR_SUBCMD_TRIGGER_SCAN: venodr scan command
 * @QCA_NL80211_VENDOR_SUBCMD_SCAN_DONE: vendor scan complete
 * @QCA_NL80211_VENDOR_SUBCMD_ABORT_SCAN: vendor abort scan
 * @QCA_NL80211_VENDOR_SUBCMD_OTA_TEST: enable OTA test
 * @QCA_NL80211_VENDOR_SUBCMD_SET_TXPOWER_SCALE: set tx power by percentage
 * @QCA_NL80211_VENDOR_SUBCMD_SET_TXPOWER_SCALE_DECR_DB: reduce tx power by DB
 * @QCA_NL80211_VENDOR_SUBCMD_SET_SAP_CONFIG: SAP configuration
 * @QCA_NL80211_VENDOR_SUBCMD_TSF: TSF operations command
 * @QCA_NL80211_VENDOR_SUBCMD_WISA: WISA mode configuration
 * @QCA_NL80211_VENDOR_SUBCMD_P2P_LISTEN_OFFLOAD_START: Command used to
 *	start the P2P Listen Offload function in device and pass the listen
 *	channel, period, interval, count, number of device types, device
 *	types and vendor information elements to device driver and firmware.
 * @QCA_NL80211_VENDOR_SUBCMD_P2P_LISTEN_OFFLOAD_STOP: Command/event used to
 *	indicate stop request/response of the P2P Listen Offload function in
 *	device. As an event, it indicates either the feature stopped after it
 *	was already running or feature has actually failed to start.
 * @QCA_NL80211_VENDOR_SUBCMD_GET_STATION: send BSS Information
 * @QCA_NL80211_VENDOR_SUBCMD_SAP_CONDITIONAL_CHAN_SWITCH: After SAP starts
 *     beaconing, this sub command provides the driver, the frequencies on the
 *     5 GHz to check for any radar activity. Driver selects one channel from
 *     this priority list provided through
 *     @QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_FREQ_LIST and starts
 *     to check for radar activity on it. If no radar activity is detected
 *     during the channel availability check period, driver internally switches
 *     to the selected frequency of operation. If the frequency is zero, driver
 *     internally selects a channel. The status of this conditional switch is
 *     indicated through an event using the same sub command through
 *     @QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_STATUS. Attributes are
 *     listed in qca_wlan_vendor_attr_sap_conditional_chan_switch
 * @QCA_NL80211_VENDOR_SUBCMD_LL_STATS_EXT: Command/event used to config
 *      indication period and threshold for MAC layer counters.
 * @QCA_NL80211_VENDOR_SUBCMD_CONFIGURE_TDLS: Configure the TDLS behavior
 *	in the host driver. The different TDLS configurations are defined
 *	by the attributes in enum qca_wlan_vendor_attr_tdls_configuration.
 * @QCA_NL80211_VENDOR_SUBCMD_GET_HE_CAPABILITIES: Get HE related capabilities
 * @QCA_NL80211_VENDOR_SUBCMD_SET_SAR_LIMITS:Set the Specific Absorption Rate
 *	(SAR) power limits. A critical regulation for FCC compliance, OEMs
 *	require methods to set SAR limits on TX power of WLAN/WWAN.
 *	enum qca_vendor_attr_sar_limits attributes are used with this command.
 * @QCA_NL80211_VENDOR_SUBCMD_EXTERNAL_ACS: Vendor command used to get/set
 *      configuration of vendor ACS.
 * @QCA_NL80211_VENDOR_SUBCMD_CHIP_PWRSAVE_FAILURE: Vendor event carrying the
 *      requisite information leading to a power save failure. The information
 *      carried as part of this event is represented by the
 *      enum qca_attr_chip_power_save_failure attributes.
 * @QCA_NL80211_VENDOR_SUBCMD_NUD_STATS_SET: Start/Stop the NUD statistics
 *      collection. Uses attributes defined in enum qca_attr_nud_stats_set.
 * @QCA_NL80211_VENDOR_SUBCMD_NUD_STATS_GET: Get the NUD statistics. These
 *      statistics are represented by the enum qca_attr_nud_stats_get
 *      attributes.
 * @QCA_NL80211_VENDOR_SUBCMD_FETCH_BSS_TRANSITION_STATUS: Sub-command to fetch
 *      the BSS transition status, whether accept or reject, for a list of
 *      candidate BSSIDs provided by the userspace. This uses the vendor
 *      attributes QCA_WLAN_VENDOR_ATTR_BTM_MBO_TRANSITION_REASON and
 *      QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO. The userspace shall specify
 *      the attributes QCA_WLAN_VENDOR_ATTR_BTM_MBO_TRANSITION_REASON and an
 *      array of QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO_BSSID nested in
 *      QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO in the request. In the response
 *      the driver shall specify array of
 *      QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO_BSSID and
 *      QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO_STATUS pairs nested in
 *      QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO.
 * @QCA_NL80211_VENDOR_SUBCMD_SET_TRACE_LEVEL: Set the trace level for a
 *      specific QCA module. The trace levels are represented by
 *      enum qca_attr_trace_level attributes.
 * @QCA_NL80211_VENDOR_SUBCMD_BRP_SET_ANT_LIMIT: Set the Beam Refinement
 *      Protocol antenna limit in different modes. See enum
 *      qca_wlan_vendor_attr_brp_ant_limit_mode.
 * @QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_START: Start spectral scan. The scan
 *      parameters are specified by enum qca_wlan_vendor_attr_spectral_scan.
 *      This returns a cookie (%QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_COOKIE)
 *      identifying the operation in success case. In failure cases an
 *      error code (%QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_ERROR_CODE)
 *      describing the reason for the failure is returned.
 * @QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_STOP: Stop spectral scan. This uses
 *      a cookie (%QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_COOKIE) from
 * @QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_START to identify the scan to
 *      be stopped.
 * @QCA_NL80211_VENDOR_SUBCMD_ACTIVE_TOS: Set the active Type Of Service on the
 *     specific interface. This can be used to modify some of the low level
 *     scan parameters (off channel dwell time, home channel time) in the
 *     driver/firmware. These parameters are maintained within the host
 *     driver.
 *     This command is valid only when the interface is in the connected
 *     state.
 *     These scan parameters shall be reset by the driver/firmware once
 *     disconnected. The attributes used with this command are defined in
 *     enum qca_wlan_vendor_attr_active_tos.
 * @QCA_NL80211_VENDOR_SUBCMD_HANG: Event indicating to the user space that the
 *      driver has detected an internal failure. This event carries the
 *      information indicating the reason that triggered this detection. The
 *      attributes for this command are defined in
 *      enum qca_wlan_vendor_attr_hang.
 * @QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_CONFIG: Get the current values
 *     of spectral parameters used. The spectral scan parameters are specified
 *     by enum qca_wlan_vendor_attr_spectral_scan.
 * @QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_DIAG_STATS: Get the debug stats
 *     for spectral scan functionality. The debug stats are specified by
 *     enum qca_wlan_vendor_attr_spectral_diag_stats.
 * @QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_CAP_INFO: Get spectral
 *     scan system capabilities. The capabilities are specified
 *     by enum qca_wlan_vendor_attr_spectral_cap.
 * @QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_STATUS: Get the current
 *     status of spectral scan. The status values are specified
 *     by enum qca_wlan_vendor_attr_spectral_scan_status.
 * @QCA_NL80211_VENDOR_SUBCMD_PEER_FLUSH_PENDING: Sub-command to flush
 *     peer pending packets. Specify the peer MAC address in
 *     QCA_WLAN_VENDOR_ATTR_PEER_ADDR and the access category of the packets
 *     in QCA_WLAN_VENDOR_ATTR_AC. The attributes are listed
 *     in enum qca_wlan_vendor_attr_flush_pending.
 * @QCA_NL80211_VENDOR_SUBCMD_GET_RROP_INFO: Get vendor specific Representative
 *     RF Operating Parameter (RROP) information. The attributes for this
 *     information are defined in enum qca_wlan_vendor_attr_rrop_info. This is
 *     intended for use by external Auto Channel Selection applications.
 * @QCA_NL80211_VENDOR_SUBCMD_GET_SAR_LIMITS: Get the Specific Absorption Rate
 *	(SAR) power limits. This is a companion to the command
 *	@QCA_NL80211_VENDOR_SUBCMD_SET_SAR_LIMITS and is used to retrieve the
 *	settings currently in use. The attributes returned by this command are
 *	defined by enum qca_vendor_attr_sar_limits.
 * @QCA_NL80211_VENDOR_SUBCMD_WLAN_MAC_INFO: Provides the current behaviour of
 *      the WLAN hardware MAC's associated with each WLAN netdev interface.
 *      This works both as a query (user space asks the current mode) or event
 *      interface (driver advertizing the current mode to the user space).
 *      Driver does not trigger this event for temporary hardware mode changes.
 *      Mode changes w.r.t Wi-Fi connection updation ( VIZ creation / deletion,
 *      channel change etc ) are updated with this event. Attributes for this
 *      interface are defined in enum qca_wlan_vendor_attr_mac.
 * @QCA_NL80211_VENDOR_SUBCMD_SET_QDEPTH_THRESH: Set MSDU queue depth threshold
 *	per peer per TID. Attributes for this command are define in
 *	enum qca_wlan_set_qdepth_thresh_attr
 * @QCA_NL80211_VENDOR_SUBCMD_WIFI_TEST_CONFIGURATION: Sub command to set WiFi
 *	test configuration. Attributes for this command are defined in
 *	enum qca_wlan_vendor_attr_wifi_test_config.
 * @QCA_NL80211_VENDOR_SUBCMD_NAN_EXT: An extendable version of NAN vendor
 *	command. The earlier command for NAN, QCA_NL80211_VENDOR_SUBCMD_NAN,
 *	carried a payload which was a binary blob of data. The command was not
 *	extendable to send more information. The newer version carries the
 *	legacy blob encapsulated within an attribute and can be extended with
 *	additional vendor attributes that can enhance the NAN command
 *	interface.
 * @QCA_NL80211_VENDOR_SUBCMD_PEER_CFR_CAPTURE_CFG: This command is used to
 *	configure parameters per peer to capture Channel Frequency Response
 *	(CFR) and enable Periodic CFR capture. The attributes for this command
 *	are defined in enum qca_wlan_vendor_peer_cfr_capture_attr.
 * @QCA_NL80211_VENDOR_SUBCMD_GET_FW_STATE: Sub command to get firmware state.
 *	The returned firmware state is specified in the attribute
 *	QCA_WLAN_VENDOR_ATTR_FW_STATE.
 * @QCA_NL80211_VENDOR_SUBCMD_PEER_STATS_CACHE_FLUSH: This vendor subcommand
 *	is used by host driver to flush per-peer cached statistics to user space
 *	application. This interface is used as an event from host driver to
 *	user space application. Attributes for this event are specified in
 *	enum qca_wlan_vendor_attr_peer_stats_cache_params.
 *	QCA_WLAN_VENDOR_ATTR_PEER_STATS_CACHE_DATA attribute is expected to be
 *	sent as event from host driver.
 * @QCA_NL80211_VENDOR_SUBCMD_MPTA_HELPER_CONFIG: This sub command is used to
 *	improve the success rate of Zigbee joining network.
 *	Due to PTA master limitation, zigbee joining network success rate is
 *	low while wlan is working. Wlan host driver need to configure some
 *	parameters including Zigbee state and specific WLAN periods to enhance
 *	PTA master. All this parameters are delivered by the NetLink attributes
 *	defined in "enum qca_mpta_helper_vendor_attr".
 * @QCA_NL80211_VENDOR_SUBCMD_BEACON_REPORTING: This sub command is used to
 *	implement Beacon frame reporting feature.
 *
 *	Userspace can request the driver/firmware to periodically report
 *	received Beacon frames whose BSSID is same as the current connected
 *	BSS's MAC address.
 *
 *	In case the STA seamlessly (without sending disconnect indication to
 *	userspace) roams to a different BSS, Beacon frame reporting will be
 *	automatically enabled for the Beacon frames whose BSSID is same as the
 *	MAC address of the new BSS. Beacon reporting will be stopped when the
 *	STA is disconnected (when the disconnect indication is sent to
 *	userspace) and need to be explicitly enabled by userspace for next
 *	connection.
 *
 *	When a Beacon frame matching configured conditions is received, and if
 *	userspace has requested to send asynchronous beacon reports, the
 *	driver/firmware will encapsulate the details of the Beacon frame in an
 *	event and send it to userspace along with updating the BSS information
 *	in cfg80211 scan cache, otherwise driver will only update the cfg80211
 *	scan cache with the information from the received Beacon frame but
 *	will not send any active report to userspace.
 *
 *	The userspace can request the driver/firmware to stop reporting Beacon
 *	frames. If the driver/firmware is not able to receive Beacon frames
 *	due to other Wi-Fi operations such as off-channel activities, etc.,
 *	the driver/firmware will send a pause event to userspace and stop
 *	reporting Beacon frames. Whether the beacon reporting will be
 *	automatically resumed or not by the driver/firmware later will be
 *	reported to userspace using the
 *	QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_AUTO_RESUMES flag. The beacon
 *	reporting shall be resumed for all the cases except either when
 *	userspace sets QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_DO_NOT_RESUME flag
 *	in the command which triggered the current beacon reporting or during
 *	any disconnection case as indicated by setting
 *	QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_PAUSE_REASON to
 *	QCA_WLAN_VENDOR_BEACON_REPORTING_PAUSE_REASON_DISCONNECTED by the
 *	driver.
 *
 *	After QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_OP_PAUSE event is received
 *	by userspace with QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_AUTO_RESUMES
 *	flag not set, the next first
 *	QCA_WLAN_VENDOR_BEACON_REPORTING_OP_BEACON_INFO event from the driver
 *	shall be considered as un-pause event.
 *
 *	All the attributes used with this command are defined in
 *	enum qca_wlan_vendor_attr_beacon_reporting_params.
 * @QCA_NL80211_VENDOR_SUBCMD_INTEROP_ISSUES_AP: In practice, some aps have
 *	interop issues with the DUT. This sub command is used to transfer the
 *	ap info between driver and user space. This works both as a command
 *	or event. As a command, it configs the stored list of aps from user
 *	space to firmware; as an event, it indicates the ap info detected by
 *	firmware to user space for persistent storage. The attributes defined
 *	in enum qca_vendor_attr_interop_issues_ap are used to deliver the
 *	parameters.
 * @QCA_NL80211_VENDOR_SUBCMD_OEM_DATA: This command is used to send OEM data
 *	binary blobs from application/service to firmware. The attributes
 *	defined in enum qca_wlan_vendor_attr_oem_data_params are used to
 *	deliver the parameters.
 * @QCA_NL80211_VENDOR_SUBCMD_AVOID_FREQUENCY_EXT: This command/event is used
 *	to send/receive avoid frequency data using
 *	enum qca_wlan_vendor_attr_avoid_frequency_ext.
 *	This new command is alternative to existing command
 *	QCA_NL80211_VENDOR_SUBCMD_AVOID_FREQUENCY since existing command/event
 *	is using stream of bytes instead of structured data using vendor
 *	attributes.
 * @QCA_NL80211_VENDOR_SUBCMD_ADD_STA_NODE: This vendor subcommand is used to
 *	add the STA node details in driver/firmware. Attributes for this event
 *	are specified in enum qca_wlan_vendor_attr_add_sta_node_params.
 * @QCA_NL80211_VENDOR_SUBCMD_BTC_CHAIN_MODE: This command is used to set BT
 *	coex chain mode from application/service.
 *	The attributes defined in enum qca_vendor_attr_btc_chain_mode are used
 *	to deliver the parameters.
 * @QCA_NL80211_VENDOR_SUBCMD_GET_STA_INFO: This vendor subcommand is used to
 *	get information of a station from driver to userspace. This command can
 *	be used in both STA and AP modes. For STA mode, it provides information
 *	of the current association when in connected state or the last
 *	association when in disconnected state. For AP mode, only information
 *	of the currently connected stations is available. This command uses
 *	attributes defined in enum qca_wlan_vendor_attr_get_sta_info.
 * @QCA_NL80211_VENDOR_SUBCMD_REQUEST_SAR_LIMITS_EVENT: This acts as an event.
 *	Host drivers can request the user space entity to set the SAR power
 *	limits with this event. Accordingly, the user space entity is expected
 *	to set the SAR power limits. Host drivers can retry this event to the
 *	user space for the SAR power limits configuration from user space. If
 *	the driver does not get the SAR power limits from user space for all
 *	the retried attempts, it can configure a default SAR power limit.
 * @QCA_NL80211_VENDOR_SUBCMD_UPDATE_STA_INFO: This acts as a vendor event and
 *	is used to update the information about the station from the driver to
 *	userspace. Uses attributes from enum
 *	qca_wlan_vendor_attr_update_sta_info.
 *
 * @QCA_NL80211_VENDOR_SUBCMD_DRIVER_DISCONNECT_REASON: This acts as an event.
 *	The host driver initiates the disconnection for scenarios such as beacon
 *	miss, NUD failure, peer kick out, etc. The disconnection indication
 *	through cfg80211_disconnected() expects the reason codes from enum
 *	ieee80211_reasoncode which does not signify these various reasons why
 *	the driver has triggered the disconnection. This event will be used to
 *	send the driver specific reason codes by the host driver to userspace.
 *	Host drivers should trigger this event and pass the respective reason
 *	code immediately prior to triggering cfg80211_disconnected(). The
 *	attributes used with this event are defined in enum
 *	qca_wlan_vendor_attr_driver_disconnect_reason.
 *
 * @QCA_NL80211_VENDOR_SUBCMD_CONFIG_TSPEC: This vendor subcommand is used to
 *	add/delete TSPEC for each AC. One command is for one specific AC only.
 *	This command can only be used in STA mode and the STA must be
 *	associated with an AP when the command is issued. Uses attributes
 *	defined in enum qca_wlan_vendor_attr_config_tspec.
 *
 * @QCA_NL80211_VENDOR_SUBCMD_CONFIG_TWT: Vendor subcommand to configure TWT.
 *	Uses attributes defined in enum qca_wlan_vendor_attr_config_twt.
 *
 * @QCA_NL80211_VENDOR_SUBCMD_GETBAND: Command to get the configured band from
 *	the host driver. The band configurations obtained are referred through
 *	QCA_WLAN_VENDOR_ATTR_SETBAND_MASK.
 *
 * @QCA_NL80211_VENDOR_SUBCMD_MEDIUM_ASSESS: Vendor subcommand/event for medium
 *	assessment.
 *	Uses attributes defined in enum qca_wlan_vendor_attr_medium_assess.
 *
 * @QCA_NL80211_VENDOR_SUBCMD_UPDATE_SSID: This acts as a vendor event and is
 *	used to update SSID information in hostapd when it is updated in the
 *	driver. Uses the attribute NL80211_ATTR_SSID.
 *
 * @QCA_NL80211_VENDOR_SUBCMD_WIFI_FW_STATS: This vendor subcommand is used by
 *	the driver to send opaque data from the firmware to userspace. The
 *	driver sends an event to userspace whenever such data is received from
 *	the firmware.
 *
 *	QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_DATA is used as the attribute to
 *	send this opaque data for this event.
 *
 *	The format of the opaque data is specific to the particular firmware
 *	version and there is no guarantee of the format remaining same.
 *
 * @QCA_NL80211_VENDOR_SUBCMD_MBSSID_TX_VDEV_STATUS: This acts as an event.
 *	The host driver selects Tx VDEV, and notifies user. The attributes
 *	used with this event are defined in enum
 *	qca_wlan_vendor_attr_mbssid_tx_vdev_status.
 */

enum qca_nl80211_vendor_subcmds {
	QCA_NL80211_VENDOR_SUBCMD_UNSPEC = 0,
	QCA_NL80211_VENDOR_SUBCMD_TEST = 1,
	QCA_NL80211_VENDOR_SUBCMD_ROAMING = 9,
	QCA_NL80211_VENDOR_SUBCMD_AVOID_FREQUENCY = 10,
	QCA_NL80211_VENDOR_SUBCMD_DFS_CAPABILITY = 11,
	QCA_NL80211_VENDOR_SUBCMD_NAN = 12,
	QCA_NL80211_VENDOR_SUBCMD_STATS_EXT = 13,

	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_SET = 14,
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_GET = 15,
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_CLR = 16,
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_RADIO_RESULTS = 17,
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_IFACE_RESULTS = 18,
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_PEERS_RESULTS = 19,

	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_START = 20,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_STOP = 21,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_VALID_CHANNELS = 22,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_CAPABILITIES = 23,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_CACHED_RESULTS = 24,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SCAN_RESULTS_AVAILABLE = 25,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_FULL_SCAN_RESULT = 26,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SCAN_EVENT = 27,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_AP_FOUND = 28,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_BSSID_HOTLIST = 29,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_BSSID_HOTLIST = 30,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SIGNIFICANT_CHANGE = 31,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_SIGNIFICANT_CHANGE = 32,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_SIGNIFICANT_CHANGE = 33,

	QCA_NL80211_VENDOR_SUBCMD_TDLS_ENABLE = 34,
	QCA_NL80211_VENDOR_SUBCMD_TDLS_DISABLE = 35,
	QCA_NL80211_VENDOR_SUBCMD_TDLS_GET_STATUS = 36,
	QCA_NL80211_VENDOR_SUBCMD_TDLS_STATE = 37,

	QCA_NL80211_VENDOR_SUBCMD_GET_SUPPORTED_FEATURES = 38,

	QCA_NL80211_VENDOR_SUBCMD_SCANNING_MAC_OUI = 39,
	QCA_NL80211_VENDOR_SUBCMD_NO_DFS_FLAG = 40,

	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_AP_LOST = 41,

	/* Get Concurrency Matrix */
	QCA_NL80211_VENDOR_SUBCMD_GET_CONCURRENCY_MATRIX = 42,

	QCA_NL80211_VENDOR_SUBCMD_KEY_MGMT_SET_KEY = 50,
	QCA_NL80211_VENDOR_SUBCMD_KEY_MGMT_ROAM_AUTH = 51,
	QCA_NL80211_VENDOR_SUBCMD_APFIND = 52,

	/* Deprecated */
	QCA_NL80211_VENDOR_SUBCMD_OCB_SET_SCHED = 53,

	QCA_NL80211_VENDOR_SUBCMD_DO_ACS = 54,

	QCA_NL80211_VENDOR_SUBCMD_GET_FEATURES = 55,

	/* Off loaded DFS events */
	QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_STARTED = 56,
	QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_FINISHED = 57,
	QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_ABORTED = 58,
	QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_NOP_FINISHED = 59,
	QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_RADAR_DETECTED = 60,

	QCA_NL80211_VENDOR_SUBCMD_GET_WIFI_INFO = 61,
	QCA_NL80211_VENDOR_SUBCMD_WIFI_LOGGER_START = 62,
	QCA_NL80211_VENDOR_SUBCMD_WIFI_LOGGER_MEMORY_DUMP = 63,
	QCA_NL80211_VENDOR_SUBCMD_ROAM = 64,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_SSID_HOTLIST = 65,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_SSID_HOTLIST = 66,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_SSID_FOUND = 67,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_SSID_LOST = 68,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_SET_LIST = 69,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_SET_PASSPOINT_LIST = 70,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_RESET_PASSPOINT_LIST = 71,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_NETWORK_FOUND = 72,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_PASSPOINT_NETWORK_FOUND = 73,

	/* Wi-Fi Configuration subcommands */
	QCA_NL80211_VENDOR_SUBCMD_SET_WIFI_CONFIGURATION = 74,
	QCA_NL80211_VENDOR_SUBCMD_GET_WIFI_CONFIGURATION = 75,
	QCA_NL80211_VENDOR_SUBCMD_GET_LOGGER_FEATURE_SET = 76,
	QCA_NL80211_VENDOR_SUBCMD_GET_RING_DATA = 77,

	QCA_NL80211_VENDOR_SUBCMD_TDLS_GET_CAPABILITIES = 78,
	QCA_NL80211_VENDOR_SUBCMD_OFFLOADED_PACKETS = 79,
	QCA_NL80211_VENDOR_SUBCMD_MONITOR_RSSI = 80,
	QCA_NL80211_VENDOR_SUBCMD_NDP = 81,

	/* NS Offload enable/disable cmd */
	QCA_NL80211_VENDOR_SUBCMD_ND_OFFLOAD = 82,

	QCA_NL80211_VENDOR_SUBCMD_PACKET_FILTER = 83,
	QCA_NL80211_VENDOR_SUBCMD_GET_BUS_SIZE = 84,

	QCA_NL80211_VENDOR_SUBCMD_GET_WAKE_REASON_STATS = 85,

	QCA_NL80211_VENDOR_SUBCMD_DATA_OFFLOAD = 91,
	/* OCB commands */
	QCA_NL80211_VENDOR_SUBCMD_OCB_SET_CONFIG = 92,
	QCA_NL80211_VENDOR_SUBCMD_OCB_SET_UTC_TIME = 93,
	QCA_NL80211_VENDOR_SUBCMD_OCB_START_TIMING_ADVERT = 94,
	QCA_NL80211_VENDOR_SUBCMD_OCB_STOP_TIMING_ADVERT = 95,
	QCA_NL80211_VENDOR_SUBCMD_OCB_GET_TSF_TIMER = 96,
	QCA_NL80211_VENDOR_SUBCMD_DCC_GET_STATS = 97,
	QCA_NL80211_VENDOR_SUBCMD_DCC_CLEAR_STATS = 98,
	QCA_NL80211_VENDOR_SUBCMD_DCC_UPDATE_NDL = 99,
	QCA_NL80211_VENDOR_SUBCMD_DCC_STATS_EVENT = 100,

	/* subcommand to get link properties */
	QCA_NL80211_VENDOR_SUBCMD_LINK_PROPERTIES = 101,
	/* LFR Subnet Detection */
	QCA_NL80211_VENDOR_SUBCMD_GW_PARAM_CONFIG = 102,

	/* DBS subcommands */
	QCA_NL80211_VENDOR_SUBCMD_GET_PREFERRED_FREQ_LIST = 103,
	QCA_NL80211_VENDOR_SUBCMD_SET_PROBABLE_OPER_CHANNEL = 104,

	/* Vendor setband command */
	QCA_NL80211_VENDOR_SUBCMD_SETBAND = 105,

	/* Vendor scan commands */
	QCA_NL80211_VENDOR_SUBCMD_TRIGGER_SCAN = 106,
	QCA_NL80211_VENDOR_SUBCMD_SCAN_DONE = 107,

	/* OTA test subcommand */
	QCA_NL80211_VENDOR_SUBCMD_OTA_TEST = 108,
	/* Tx power scaling subcommands */
	QCA_NL80211_VENDOR_SUBCMD_SET_TXPOWER_SCALE = 109,
	/* Tx power scaling in db subcommands */
	QCA_NL80211_VENDOR_SUBCMD_SET_TXPOWER_SCALE_DECR_DB = 115,
	QCA_NL80211_VENDOR_SUBCMD_ACS_POLICY = 116,
	QCA_NL80211_VENDOR_SUBCMD_STA_CONNECT_ROAM_POLICY = 117,
	QCA_NL80211_VENDOR_SUBCMD_SET_SAP_CONFIG  = 118,
	QCA_NL80211_VENDOR_SUBCMD_TSF = 119,
	QCA_NL80211_VENDOR_SUBCMD_WISA = 120,
	QCA_NL80211_VENDOR_SUBCMD_GET_STATION = 121,
	QCA_NL80211_VENDOR_SUBCMD_P2P_LISTEN_OFFLOAD_START = 122,
	QCA_NL80211_VENDOR_SUBCMD_P2P_LISTEN_OFFLOAD_STOP = 123,
	QCA_NL80211_VENDOR_SUBCMD_SAP_CONDITIONAL_CHAN_SWITCH = 124,
	QCA_NL80211_VENDOR_SUBCMD_GPIO_CONFIG_COMMAND = 125,

	QCA_NL80211_VENDOR_SUBCMD_GET_HW_CAPABILITY = 126,
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_EXT = 127,
	/* FTM/indoor location subcommands */
	QCA_NL80211_VENDOR_SUBCMD_LOC_GET_CAPA = 128,
	QCA_NL80211_VENDOR_SUBCMD_FTM_START_SESSION = 129,
	QCA_NL80211_VENDOR_SUBCMD_FTM_ABORT_SESSION = 130,
	QCA_NL80211_VENDOR_SUBCMD_FTM_MEAS_RESULT = 131,
	QCA_NL80211_VENDOR_SUBCMD_FTM_SESSION_DONE = 132,
	QCA_NL80211_VENDOR_SUBCMD_FTM_CFG_RESPONDER = 133,
	QCA_NL80211_VENDOR_SUBCMD_AOA_MEAS = 134,
	QCA_NL80211_VENDOR_SUBCMD_AOA_ABORT_MEAS = 135,
	QCA_NL80211_VENDOR_SUBCMD_AOA_MEAS_RESULT = 136,

	/* Encrypt/Decrypt command */
	QCA_NL80211_VENDOR_SUBCMD_ENCRYPTION_TEST = 137,

	QCA_NL80211_VENDOR_SUBCMD_GET_CHAIN_RSSI = 138,
	/* DMG low level RF sector operations */
	QCA_NL80211_VENDOR_SUBCMD_DMG_RF_GET_SECTOR_CFG = 139,
	QCA_NL80211_VENDOR_SUBCMD_DMG_RF_SET_SECTOR_CFG = 140,
	QCA_NL80211_VENDOR_SUBCMD_DMG_RF_GET_SELECTED_SECTOR = 141,
	QCA_NL80211_VENDOR_SUBCMD_DMG_RF_SET_SELECTED_SECTOR = 142,

	/* Configure the TDLS mode from user space */
	QCA_NL80211_VENDOR_SUBCMD_CONFIGURE_TDLS = 143,

	QCA_NL80211_VENDOR_SUBCMD_GET_HE_CAPABILITIES = 144,

	/* Vendor abort scan command */
	QCA_NL80211_VENDOR_SUBCMD_ABORT_SCAN = 145,

	/* Set Specific Absorption Rate(SAR) Power Limits */
	QCA_NL80211_VENDOR_SUBCMD_SET_SAR_LIMITS = 146,

	/* External Auto channel configuration setting */
	QCA_NL80211_VENDOR_SUBCMD_EXTERNAL_ACS = 147,

	QCA_NL80211_VENDOR_SUBCMD_CHIP_PWRSAVE_FAILURE = 148,
	QCA_NL80211_VENDOR_SUBCMD_NUD_STATS_SET = 149,
	QCA_NL80211_VENDOR_SUBCMD_NUD_STATS_GET = 150,
	QCA_NL80211_VENDOR_SUBCMD_FETCH_BSS_TRANSITION_STATUS = 151,

	/* Set the trace level for QDF */
	QCA_NL80211_VENDOR_SUBCMD_SET_TRACE_LEVEL = 152,

	QCA_NL80211_VENDOR_SUBCMD_BRP_SET_ANT_LIMIT = 153,

	QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_START = 154,
	QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_STOP = 155,
	QCA_NL80211_VENDOR_SUBCMD_ACTIVE_TOS = 156,
	QCA_NL80211_VENDOR_SUBCMD_HANG = 157,
	QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_CONFIG = 158,
	QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_DIAG_STATS = 159,
	QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_CAP_INFO = 160,
	QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_STATUS = 161,
	QCA_NL80211_VENDOR_SUBCMD_PEER_FLUSH_PENDING = 162,
	QCA_NL80211_VENDOR_SUBCMD_GET_RROP_INFO = 163,
	QCA_NL80211_VENDOR_SUBCMD_GET_SAR_LIMITS = 164,
	QCA_NL80211_VENDOR_SUBCMD_WLAN_MAC_INFO = 165,
	QCA_NL80211_VENDOR_SUBCMD_SET_QDEPTH_THRESH = 166,
	QCA_NL80211_VENDOR_SUBCMD_THERMAL_CMD = 167,
	/* Wi-Fi test configuration subcommand */
	QCA_NL80211_VENDOR_SUBCMD_WIFI_TEST_CONFIGURATION = 169,
	QCA_NL80211_VENDOR_SUBCMD_NAN_EXT = 171,
	QCA_NL80211_VENDOR_SUBCMD_PEER_CFR_CAPTURE_CFG = 173,
	QCA_NL80211_VENDOR_SUBCMD_THROUGHPUT_CHANGE_EVENT = 174,
	QCA_NL80211_VENDOR_SUBCMD_COEX_CONFIG = 175,
	QCA_NL80211_VENDOR_SUBCMD_GET_FW_STATE = 177,
	QCA_NL80211_VENDOR_SUBCMD_PEER_STATS_CACHE_FLUSH = 178,
	QCA_NL80211_VENDOR_SUBCMD_MPTA_HELPER_CONFIG = 179,
	QCA_NL80211_VENDOR_SUBCMD_BEACON_REPORTING = 180,
	QCA_NL80211_VENDOR_SUBCMD_INTEROP_ISSUES_AP = 181,
	QCA_NL80211_VENDOR_SUBCMD_OEM_DATA = 182,
	QCA_NL80211_VENDOR_SUBCMD_AVOID_FREQUENCY_EXT = 183,
	QCA_NL80211_VENDOR_SUBCMD_ADD_STA_NODE = 184,
	QCA_NL80211_VENDOR_SUBCMD_BTC_CHAIN_MODE = 185,
	QCA_NL80211_VENDOR_SUBCMD_GET_STA_INFO = 186,
	QCA_NL80211_VENDOR_SUBCMD_GET_SAR_LIMITS_EVENT = 187,
	QCA_NL80211_VENDOR_SUBCMD_UPDATE_STA_INFO = 188,
	QCA_NL80211_VENDOR_SUBCMD_DRIVER_DISCONNECT_REASON = 189,
	QCA_NL80211_VENDOR_SUBCMD_CONFIG_TSPEC = 190,
	QCA_NL80211_VENDOR_SUBCMD_CONFIG_TWT = 191,
	QCA_NL80211_VENDOR_SUBCMD_GETBAND = 192,
	QCA_NL80211_VENDOR_SUBCMD_MEDIUM_ASSESS = 193,
	QCA_NL80211_VENDOR_SUBCMD_UPDATE_SSID = 194,
	QCA_NL80211_VENDOR_SUBCMD_WIFI_FW_STATS = 195,
	QCA_NL80211_VENDOR_SUBCMD_MBSSID_TX_VDEV_STATUS = 196,
};

enum qca_wlan_vendor_tos {
	QCA_WLAN_VENDOR_TOS_BK = 0,
	QCA_WLAN_VENDOR_TOS_BE = 1,
	QCA_WLAN_VENDOR_TOS_VI = 2,
	QCA_WLAN_VENDOR_TOS_VO = 3,
};

/**
 * enum qca_wlan_vendor_attr_active_tos - Used by the vendor command
 * QCA_NL80211_VENDOR_SUBCMD_ACTIVE_TOS.
 */
enum qca_wlan_vendor_attr_active_tos {
	QCA_WLAN_VENDOR_ATTR_ACTIVE_TOS_INVALID = 0,
	/* Type Of Service - Represented by qca_wlan_vendor_tos */
	QCA_WLAN_VENDOR_ATTR_ACTIVE_TOS = 1,
	/* Flag attribute representing the start (attribute included) or stop
	 * (attribute not included) of the respective TOS.
	 */
	QCA_WLAN_VENDOR_ATTR_ACTIVE_TOS_START = 2,
	QCA_WLAN_VENDOR_ATTR_ACTIVE_TOS_MAX = 3,
};

enum qca_wlan_vendor_hang_reason {
	/* Unspecified reason */
	QCA_WLAN_HANG_REASON_UNSPECIFIED = 0,
	/* No Map for the MAC entry for the received frame */
	QCA_WLAN_HANG_RX_HASH_NO_ENTRY_FOUND = 1,
	/* peer deletion timeout happened */
	QCA_WLAN_HANG_PEER_DELETION_TIMEDOUT = 2,
	/* peer unmap timeout */
	QCA_WLAN_HANG_PEER_UNMAP_TIMEDOUT = 3,
	/* Scan request timed out */
	QCA_WLAN_HANG_SCAN_REQ_EXPIRED = 4,
	/* Consecutive Scan attempt failures */
	QCA_WLAN_HANG_SCAN_ATTEMPT_FAILURES = 5,
	/* Unable to get the message buffer */
	QCA_WLAN_HANG_GET_MSG_BUFF_FAILURE = 6,
	/* Current command processing is timedout */
	QCA_WLAN_HANG_ACTIVE_LIST_TIMEOUT = 7,
	/* Timeout for an ACK from FW for suspend request */
	QCA_WLAN_HANG_SUSPEND_TIMEOUT = 8,
	/* Timeout for an ACK from FW for resume request */
	QCA_WLAN_HANG_RESUME_TIMEOUT = 9,
	/* Transmission timeout for consecutive data frames */
	QCA_WLAN_HANG_TRANSMISSIONS_TIMEOUT = 10,
	/* Timeout for the TX completion status of data frame */
	QCA_WLAN_HANG_TX_COMPLETE_TIMEOUT = 11,
	/* DXE failure for tx/Rx, DXE resource unavailability */
	QCA_WLAN_HANG_DXE_FAILURE = 12,
	/* WMI pending commands exceed the maximum count */
	QCA_WLAN_HANG_WMI_EXCEED_MAX_PENDING_CMDS = 13,
	/* Timeout for peer STA connection accept command's response from the
	 * FW in AP mode. This command is triggered when a STA (peer) connects
	 * to AP (DUT).
	 */
	QCA_WLAN_HANG_AP_STA_CONNECT_REQ_TIMEOUT = 14,
	/* Timeout for the AP connection accept command's response from the FW
	 * in STA mode. This command is triggered when the STA (DUT) connects
	 * to an AP (peer).
	 */
	QCA_WLAN_HANG_STA_AP_CONNECT_REQ_TIMEOUT = 15,
	/* Timeout waiting for the response to the MAC HW mode change command
	 * sent to FW as a part of MAC mode switch among DBS (Dual Band
	 * Simultaneous), SCC (Single Channel Concurrency), and MCC (Multi
	 * Channel Concurrency) mode.
	 */
	QCA_WLAN_HANG_MAC_HW_MODE_CHANGE_TIMEOUT = 16,
	/* Timeout waiting for the response from FW to configure the MAC HW's
	 * mode. This operation is to configure the single/two MACs in either
	 * SCC/MCC/DBS mode.
	 */
	QCA_WLAN_HANG_MAC_HW_MODE_CONFIG_TIMEOUT = 17,
	/* Timeout waiting for response of VDEV start command from the FW */
	QCA_WLAN_HANG_VDEV_START_RESPONSE_TIMED_OUT = 18,
	/* Timeout waiting for response of VDEV restart command from the FW */
	QCA_WLAN_HANG_VDEV_RESTART_RESPONSE_TIMED_OUT = 19,
	/* Timeout waiting for response of VDEV stop command from the FW */
	QCA_WLAN_HANG_VDEV_STOP_RESPONSE_TIMED_OUT = 20,
	/* Timeout waiting for response of VDEV delete command from the FW */
	QCA_WLAN_HANG_VDEV_DELETE_RESPONSE_TIMED_OUT = 21,
	/* Timeout waiting for response of peer all delete request command to
	 * the FW on a specific VDEV.
	 */
	QCA_WLAN_HANG_VDEV_PEER_DELETE_ALL_RESPONSE_TIMED_OUT = 22,
	/* WMI sequence mismatch between WMI command and Tx completion */
	QCA_WLAN_HANG_WMI_BUF_SEQUENCE_MISMATCH = 23,
	/* Write to Device HAL register failed */
	QCA_WLAN_HANG_REG_WRITE_FAILURE = 24,
	/* No credit left to send the wow_wakeup_from_sleep to firmware */
	QCA_WLAN_HANG_SUSPEND_NO_CREDIT = 25,
};

/**
 * enum qca_wlan_vendor_attr_hang - Used by the vendor command
 * QCA_NL80211_VENDOR_SUBCMD_HANG.
 */
enum qca_wlan_vendor_attr_hang {
	QCA_WLAN_VENDOR_ATTR_HANG_INVALID = 0,
	/*
	 * Reason for the Hang - Represented by enum
	 * qca_wlan_vendor_hang_reason.
	 */
	QCA_WLAN_VENDOR_ATTR_HANG_REASON = 1,
	/* The binary blob data associated with the hang reason specified by
	 * QCA_WLAN_VENDOR_ATTR_HANG_REASON. This binary data is expected to
	 * contain the required dump to analyze the reason for the hang.
	 * NLA_BINARY attribute, the max size is 1024 bytes.
	 */
	QCA_WLAN_VENDOR_ATTR_HANG_REASON_DATA = 2,
	QCA_WLAN_VENDOR_ATTR_HANG_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_HANG_MAX =
		QCA_WLAN_VENDOR_ATTR_HANG_AFTER_LAST - 1,
};

/**
 * enum qca_vendor_attr_set_trace_level - Config params for QDF set trace level
 * @QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_INVALID: Invalid trace level
 * @QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_PARAM : Trace level parameters
 * @QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_MODULE_ID : Module of which trace
    level needs to be updated.
 * @QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_TRACE_MASK : verbose mask, which need
 * to be set.
 * @QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_AFTER_LAST : after last.
 * @QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_MAX : Max attribute.
 */
enum qca_vendor_attr_set_trace_level {
	QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_INVALID = 0,
	/*
	* Array of QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_PARAM
	* attributes.
	*/
	QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_PARAM = 1,
	QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_MODULE_ID = 2,
	QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_TRACE_MASK = 3,
	QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_MAX =
		QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_get_station - Sub commands used by
 * QCA_NL80211_VENDOR_SUBCMD_GET_STATION to get the corresponding
 * station information. The information obtained through these
 * commands signify the current info in connected state and
 * latest cached information during the connected state , if queried
 * when in disconnected state.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INVALID: Invalid attribute
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO: bss info
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_ASSOC_FAIL_REASON: assoc fail reason
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_REMOTE: remote station info
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_AFTER_LAST: After last
 */
enum qca_wlan_vendor_attr_get_station {
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_ASSOC_FAIL_REASON,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_REMOTE,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_GET_STATION_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_MAX =
		QCA_WLAN_VENDOR_ATTR_GET_STATION_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_update_sta_info - Defines attributes
 * used by QCA_NL80211_VENDOR_SUBCMD_UPDATE_STA_INFO vendor command.
 *
 * @QCA_WLAN_VENDOR_ATTR_UPDATE_STA_INFO_CONNECT_CHANNELS: Type is NLA_UNSPEC.
 * Used in STA mode. This attribute represents the list of channel center
 * frequencies in MHz (u32) the station has learnt during the last connection
 * or roaming attempt. This information shall not signify the channels for
 * an explicit scan request from the user space. Host drivers can update this
 * information to the user space in both connected and disconnected state.
 * In the disconnected state this information shall signify the channels
 * scanned in the last connection/roam attempt that lead to the disconnection.
 */
enum qca_wlan_vendor_attr_update_sta_info {
	QCA_WLAN_VENDOR_ATTR_UPDATE_STA_INFO_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_UPDATE_STA_INFO_CONNECT_CHANNELS = 1,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_UPDATE_STA_INFO_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_UPDATE_STA_INFO_MAX =
		QCA_WLAN_VENDOR_ATTR_UPDATE_STA_INFO_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_802_11_mode - dot11 mode
 * @QCA_WLAN_802_11_MODE_11B: mode B
 * @QCA_WLAN_802_11_MODE_11G: mode G
 * @QCA_WLAN_802_11_MODE_11N: mode N
 * @QCA_WLAN_802_11_MODE_11A: mode A
 * @QCA_WLAN_802_11_MODE_11AC: mode AC
 * @QCA_WLAN_802_11_MODE_11AX: mode AX
 * @QCA_WLAN_802_11_MODE_INVALID: Invalid dot11 mode
 */
enum qca_wlan_802_11_mode {
	QCA_WLAN_802_11_MODE_11B,
	QCA_WLAN_802_11_MODE_11G,
	QCA_WLAN_802_11_MODE_11N,
	QCA_WLAN_802_11_MODE_11A,
	QCA_WLAN_802_11_MODE_11AC,
	QCA_WLAN_802_11_MODE_11AX,
	QCA_WLAN_802_11_MODE_INVALID,
};

/**
 * enum qca_wlan_auth_type - Authentication key management type
 * @QCA_WLAN_AUTH_TYPE_INVALID: Invalid key management type
 * @QCA_WLAN_AUTH_TYPE_OPEN: Open key
 * @QCA_WLAN_AUTH_TYPE_SHARED: shared key
 * @QCA_WLAN_AUTH_TYPE_WPA: wpa key
 * @QCA_WLAN_AUTH_TYPE_WPA_PSK: wpa psk key
 * @QCA_WLAN_AUTH_TYPE_WPA_NONE: wpa none key
 * @QCA_WLAN_AUTH_TYPE_RSN: rsn key
 * @QCA_WLAN_AUTH_TYPE_RSN_PSK: rsn psk key
 * @QCA_WLAN_AUTH_TYPE_FT: ft key
 * @QCA_WLAN_AUTH_TYPE_FT_PSK: ft psk key
 * @QCA_WLAN_AUTH_TYPE_SHA256: shared 256 key
 * @QCA_WLAN_AUTH_TYPE_SHA256_PSK: shared 256 psk
 * @QCA_WLAN_AUTH_TYPE_WAI: wai key
 * @QCA_WLAN_AUTH_TYPE_WAI_PSK wai psk key
 * @QCA_WLAN_AUTH_TYPE_CCKM_WPA: cckm wpa key
 * @QCA_WLAN_AUTH_TYPE_CCKM_RSN: cckm rsn key
 * @QCA_WLAN_AUTH_TYPE_AUTOSWITCH: Autoswitch key
 * @QCA_WLAN_AUTH_TYPE_FT_SAE: FT sae akm
 * @QCA_WLAN_AUTH_TYPE_FT_SUITEB_EAP_SHA384: FT suite B SHA384
 * @QCA_WLAN_AUTH_TYPE_SAE: SAE key
 * @QCA_WLAN_AUTH_TYPE_FILS_SHA256: FILS SHA 256 key
 * @QCA_WLAN_AUTH_TYPE_FILS_SHA384: FILS SAH 384 key
 * @QCA_WLAN_AUTH_TYPE_FT_FILS_SHA256: FT FILS SHA 256 key
 * @QCA_WLAN_AUTH_TYPE_FT_FILS_SHA384: FT FILS SHA 384 key
 * @QCA_WLAN_AUTH_TYPE_DPP_RSN: DPP RSN key
 * @QCA_WLAN_AUTH_TYPE_OWE: owe key
 * @QCA_WLAN_AUTH_TYPE_SUITEB_EAP_SHA256 : EAP SHA 256 key
 * @QCA_WLAN_AUTH_TYPE_SUITEB_EAP_SHA384 : EAP SHA 384 key
 */
enum qca_wlan_auth_type {
	QCA_WLAN_AUTH_TYPE_INVALID,
	QCA_WLAN_AUTH_TYPE_OPEN,
	QCA_WLAN_AUTH_TYPE_SHARED,
	QCA_WLAN_AUTH_TYPE_WPA,
	QCA_WLAN_AUTH_TYPE_WPA_PSK,
	QCA_WLAN_AUTH_TYPE_WPA_NONE,
	QCA_WLAN_AUTH_TYPE_RSN,
	QCA_WLAN_AUTH_TYPE_RSN_PSK,
	QCA_WLAN_AUTH_TYPE_FT,
	QCA_WLAN_AUTH_TYPE_FT_PSK,
	QCA_WLAN_AUTH_TYPE_SHA256,
	QCA_WLAN_AUTH_TYPE_SHA256_PSK,
	QCA_WLAN_AUTH_TYPE_WAI,
	QCA_WLAN_AUTH_TYPE_WAI_PSK,
	QCA_WLAN_AUTH_TYPE_CCKM_WPA,
	QCA_WLAN_AUTH_TYPE_CCKM_RSN,
	QCA_WLAN_AUTH_TYPE_AUTOSWITCH,
	QCA_WLAN_AUTH_TYPE_FT_SAE,
	QCA_WLAN_AUTH_TYPE_FT_SUITEB_EAP_SHA384,
	QCA_WLAN_AUTH_TYPE_SAE,
	QCA_WLAN_AUTH_TYPE_FILS_SHA256,
	QCA_WLAN_AUTH_TYPE_FILS_SHA384,
	QCA_WLAN_AUTH_TYPE_FT_FILS_SHA256,
	QCA_WLAN_AUTH_TYPE_FT_FILS_SHA384,
	QCA_WLAN_AUTH_TYPE_DPP_RSN,
	QCA_WLAN_AUTH_TYPE_OWE,
	QCA_WLAN_AUTH_TYPE_SUITEB_EAP_SHA256,
	QCA_WLAN_AUTH_TYPE_SUITEB_EAP_SHA384,
};

/**
 * enum qca_wlan_vendor_attr_get_station_info - Station Info queried
 * through QCA_NL80211_VENDOR_SUBCMD_GET_STATION.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_INVALID: Invalid Attribute
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_LINK_STANDARD_NL80211_ATTR:
 *  Get the standard NL attributes Nested with this attribute.
 *  Ex : Query BW , BITRATE32 , NSS , Signal , Noise of the Link -
 *  NL80211_ATTR_SSID / NL80211_ATTR_SURVEY_INFO (Connected Channel) /
 *  NL80211_ATTR_STA_INFO
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_AP_STANDARD_NL80211_ATTR:
 *  Get the standard NL attributes Nested with this attribute.
 *  Ex : Query HT/VHT Capability advertized by the AP.
 *  NL80211_ATTR_VHT_CAPABILITY / NL80211_ATTR_HT_CAPABILITY
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_ROAM_COUNT:
 *  Number of successful Roam attempts before a
 *  disconnect, Unsigned 32 bit value
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_AKM:
 *  Authentication Key Management Type used for the connected session.
 *  Signified by enum qca_wlan_auth_type
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_802_11_MODE: 802.11 Mode of the
 *  connected Session, signified by enum qca_wlan_802_11_mode
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_AP_HS20_INDICATION:
 *  HS20 Indication Element
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_ASSOC_FAIL_REASON:
 *  Status Code Corresponding to the Association Failure.
 *  Unsigned 32 bit value.
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_MAX_PHY_RATE:
 *  Max phy rate of remote station
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_TX_PACKETS:
 *  TX packets to remote station
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_TX_BYTES:
 *  TX bytes to remote station
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_RX_PACKETS:
 *  RX packets from remote station
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_RX_BYTES:
 *  RX bytes from remote station
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_LAST_TX_RATE:
 *  Last TX rate with remote station
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_LAST_RX_RATE:
 *  Last RX rate with remote station
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_WMM:
 *  Remote station enable/disable WMM
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_SUPPORTED_MODE:
 *  Remote station connection mode
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_AMPDU:
 *  Remote station AMPDU enable/disable
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_TX_STBC:
 *  Remote station TX Space-time block coding enable/disable
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_RX_STBC:
 *  Remote station RX Space-time block coding enable/disable
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_CH_WIDTH:
 *  Remote station channel width
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_SGI_ENABLE:
 *  Remote station short GI enable/disable
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_PAD: Attribute type for padding
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_BEACON_IES: Binary attribute
 *  containing the raw information elements from Beacon frames. Represents
 *  the Beacon frames of the current BSS in the connected state. When queried
 *  in the disconnected state, these IEs correspond to the last connected BSSID.
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_DRIVER_DISCONNECT_REASON: u32, Driver
 *  disconnect reason for the last disconnection if the disconnection is
 *  triggered from the host driver. The values are referred from
 *  enum qca_disconnect_reason_codes. If the disconnect is from
 *  peer/userspace this value is QCA_DISCONNECT_REASON_UNSPECIFIED.
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_ASSOC_REQ_IES: Binary attribute
 *  Applicable in AP mode only. It contains the raw information elements
 *  from assoc request frame of the given peer station. User queries with the
 *  mac address of peer station when it disconnects. Host driver sends
 *  assoc request frame of the given station. Host driver doesn't provide
 *  the IEs when the peer station is still in connected state.
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_HE_OPERATION: Attribute type for
 * sending HE operation info.
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_AFTER_LAST: After last
 */
enum qca_wlan_vendor_attr_get_station_info {
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_LINK_STANDARD_NL80211_ATTR,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_AP_STANDARD_NL80211_ATTR,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_ROAM_COUNT,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_AKM,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_802_11_MODE,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_AP_HS20_INDICATION,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_HT_OPERATION,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_VHT_OPERATION,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_ASSOC_FAIL_REASON,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_MAX_PHY_RATE,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_TX_PACKETS,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_TX_BYTES,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_RX_PACKETS,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_RX_BYTES,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_LAST_TX_RATE,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_LAST_RX_RATE,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_WMM,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_SUPPORTED_MODE,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_AMPDU,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_TX_STBC,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_RX_STBC,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_CH_WIDTH,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_SGI_ENABLE,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_PAD,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_RX_RETRY_COUNT,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_RX_BC_MC_COUNT,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_TX_FAILURE,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_AVG_RSSI_PER_CHAIN,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_TX_RETRY_SUCCEED,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_RX_LAST_PKT_RSSI,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_TX_RETRY,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_TX_RETRY_EXHAUST,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_TX_TOTAL_FW,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_TX_RETRY_FW,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_TX_RETRY_EXHAUST_FW,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_BEACON_IES,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_DRIVER_DISCONNECT_REASON,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_ASSOC_REQ_IES,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_HE_OPERATION,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_MAX =
		QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_tdls_enable - TDLS enable attribute
 *
 * @QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_INVALID: Invalid initial value
 * @QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MAC_ADDR: An array of 6 x Unsigned 8-bit
 *	value
 * @QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_CHANNEL: Signed 32-bit value, but lets
 *	keep as unsigned for now
 * @QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_GLOBAL_OPERATING_CLASS: operating class
 * @QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MAX_LATENCY_MS: Enable max latency in ms
 * @QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MIN_BANDWIDTH_KBPS: Enable min bandwidth
 *	in KBPS
 * @QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_AFTER_LAST: After last
 * @QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MAX: Max value
 */
enum qca_wlan_vendor_attr_tdls_enable {
	QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MAC_ADDR,
	QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_CHANNEL,
	QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_GLOBAL_OPERATING_CLASS,
	QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MAX_LATENCY_MS,
	QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MIN_BANDWIDTH_KBPS,
	QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MAX =
		QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_tdls_disable: tdls disable attribute
 *
 * @QCA_WLAN_VENDOR_ATTR_TDLS_DISABLE_INVALID: Invalid initial value
 * @QCA_WLAN_VENDOR_ATTR_TDLS_DISABLE_MAC_ADDR: An array of 6 x Unsigned
 *	8-bit value
 * @QCA_WLAN_VENDOR_ATTR_TDLS_DISABLE_AFTER_LAST: After last
 * @QCA_WLAN_VENDOR_ATTR_TDLS_DISABLE_MAX: Max value
 */
enum qca_wlan_vendor_attr_tdls_disable {
	QCA_WLAN_VENDOR_ATTR_TDLS_DISABLE_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_TDLS_DISABLE_MAC_ADDR,
	QCA_WLAN_VENDOR_ATTR_TDLS_DISABLE_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_TDLS_DISABLE_MAX =
		QCA_WLAN_VENDOR_ATTR_TDLS_DISABLE_AFTER_LAST - 1,
};

/**
 * qca_chip_power_save_failure_reason: Power save failure reason
 * @QCA_CHIP_POWER_SAVE_FAILURE_REASON_PROTOCOL: Indicates power save failure
 * due to protocol/module.
 * @QCA_CHIP_POWER_SAVE_FAILURE_REASON_HARDWARE: power save failure
 * due to hardware
 */
enum qca_chip_power_save_failure_reason {
	QCA_CHIP_POWER_SAVE_FAILURE_REASON_PROTOCOL = 0,
	QCA_CHIP_POWER_SAVE_FAILURE_REASON_HARDWARE = 1,
};

/**
 * qca_attr_chip_power_save_failure: attributes to vendor subcmd
 * @QCA_NL80211_VENDOR_SUBCMD_CHIP_PWRSAVE_FAILURE. This carry the requisite
 * information leading to the power save failure.
 * @QCA_ATTR_CHIP_POWER_SAVE_FAILURE_INVALID : invalid
 * @QCA_ATTR_CHIP_POWER_SAVE_FAILURE_REASON : power save failure reason
 * represented by enum qca_chip_power_save_failure_reason
 * @QCA_ATTR_CHIP_POWER_SAVE_FAILURE_LAST : Last
 * @QCA_ATTR_CHIP_POWER_SAVE_FAILURE_MAX : Max value
 */
enum qca_attr_chip_power_save_failure {
	QCA_ATTR_CHIP_POWER_SAVE_FAILURE_INVALID = 0,

	QCA_ATTR_CHIP_POWER_SAVE_FAILURE_REASON = 1,

	/* keep last */
	QCA_ATTR_CHIP_POWER_SAVE_FAILURE_LAST,
	QCA_ATTR_CHIP_POWER_SAVE_FAILURE_MAX =
		QCA_ATTR_CHIP_POWER_SAVE_FAILURE_LAST - 1,
};


/**
 * enum qca_wlan_vendor_attr_tdls_get_status - tdls get status attribute
 *
 * @QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_INVALID: Invalid initial value
 * @QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_MAC_ADDR: An array of 6 x Unsigned
 *	8-bit value
 * @QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_STATE: get status state,
 *	unsigned 32-bit value
 * @QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_REASON: get status reason
 * @QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_CHANNEL: get status channel,
 *	unsigned 32-bit value
 * @QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_GLOBAL_OPERATING_CLASS: get operating
 *	class, unsigned 32-bit value
 * @QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_AFTER_LAST: After last
 * @QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_MAX: Max value
 */
enum qca_wlan_vendor_attr_tdls_get_status {
	QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_MAC_ADDR,
	QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_STATE,
	QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_REASON,
	QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_CHANNEL,
	QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_GLOBAL_OPERATING_CLASS,
	QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_MAX =
		QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_tdls_state - tdls state attribute
 *
 * @QCA_WLAN_VENDOR_ATTR_TDLS_STATE_INVALID: Initial invalid value
 * @QCA_WLAN_VENDOR_ATTR_TDLS_STATE_MAC_ADDR: An array of 6 x Unsigned
 *	8-bit value
 * @QCA_WLAN_VENDOR_ATTR_TDLS_NEW_STATE: TDLS new state,
 *	unsigned 32-bit value
 * @QCA_WLAN_VENDOR_ATTR_TDLS_STATE_REASON: TDLS state reason
 * @QCA_WLAN_VENDOR_ATTR_TDLS_STATE_CHANNEL: TDLS state channel,
 *	unsigned 32-bit value
 * @QCA_WLAN_VENDOR_ATTR_TDLS_STATE_GLOBAL_OPERATING_CLASS: TDLS state
 *	operating class, unsigned 32-bit value
 * @QCA_WLAN_VENDOR_ATTR_TDLS_STATE_AFTER_LAST: After last
 * @QCA_WLAN_VENDOR_ATTR_TDLS_STATE_MAX: Max value
 */
enum qca_wlan_vendor_attr_tdls_state {
	QCA_WLAN_VENDOR_ATTR_TDLS_STATE_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_TDLS_STATE_MAC_ADDR,
	QCA_WLAN_VENDOR_ATTR_TDLS_NEW_STATE,
	QCA_WLAN_VENDOR_ATTR_TDLS_STATE_REASON,
	QCA_WLAN_VENDOR_ATTR_TDLS_STATE_CHANNEL,
	QCA_WLAN_VENDOR_ATTR_TDLS_STATE_GLOBAL_OPERATING_CLASS,
	QCA_WLAN_VENDOR_ATTR_TDLS_STATE_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_TDLS_STATE_MAX =
		QCA_WLAN_VENDOR_ATTR_TDLS_STATE_AFTER_LAST - 1,
};

/* enum's to provide TDLS capabilities */
enum qca_wlan_vendor_attr_get_tdls_capabilities {
	QCA_WLAN_VENDOR_ATTR_TDLS_GET_CAPS_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_TDLS_GET_CAPS_MAX_CONC_SESSIONS = 1,
	QCA_WLAN_VENDOR_ATTR_TDLS_GET_CAPS_FEATURES_SUPPORTED = 2,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_TDLS_GET_CAPS_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_TDLS_GET_CAPS_MAX =
		QCA_WLAN_VENDOR_ATTR_TDLS_GET_CAPS_AFTER_LAST - 1,
};

enum qca_wlan_vendor_attr {
	QCA_WLAN_VENDOR_ATTR_INVALID = 0,
	/* used by QCA_NL80211_VENDOR_SUBCMD_DFS_CAPABILITY */
	QCA_WLAN_VENDOR_ATTR_DFS     = 1,
	/* used by QCA_NL80211_VENDOR_SUBCMD_NAN */
	QCA_WLAN_VENDOR_ATTR_NAN     = 2,
	/* used by QCA_NL80211_VENDOR_SUBCMD_STATS_EXT */
	QCA_WLAN_VENDOR_ATTR_STATS_EXT     = 3,
	/* used by QCA_NL80211_VENDOR_SUBCMD_STATS_EXT */
	QCA_WLAN_VENDOR_ATTR_IFINDEX     = 4,
	/*
	 * used by QCA_NL80211_VENDOR_SUBCMD_ROAMING, u32 with values defined
	 * by enum qca_roaming_policy.
	 */
	QCA_WLAN_VENDOR_ATTR_ROAMING_POLICY = 5,
	QCA_WLAN_VENDOR_ATTR_MAC_ADDR = 6,
	/* used by QCA_NL80211_VENDOR_SUBCMD_GET_FEATURES */
	QCA_WLAN_VENDOR_ATTR_FEATURE_FLAGS = 7,
	QCA_WLAN_VENDOR_ATTR_TEST = 8,
	/*
	 * used by QCA_NL80211_VENDOR_SUBCMD_GET_FEATURES
	 * Unsigned 32-bit value.
	 */
	QCA_WLAN_VENDOR_ATTR_CONCURRENCY_CAPA = 9,
	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_MAX_CONCURRENT_CHANNELS_2_4_BAND = 10,
	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_MAX_CONCURRENT_CHANNELS_5_0_BAND = 11,
	/* Unsigned 32-bit value from enum qca_set_band. Also, the acceptable
	 * value for this attribute are only till QCA_SETBAND_2G. This attribute
	 * is deprecated. Recommendation is to use
	 * QCA_WLAN_VENDOR_ATTR_SETBAND_MASK instead. If the band is configured
	 * using both the attributes, the ones configured through
	 * QCA_WLAN_VENDOR_ATTR_SETBAND_MASK take the precedence.
	 */
	QCA_WLAN_VENDOR_ATTR_SETBAND_VALUE = 12,
	/* Dummy (NOP) attribute for 64 bit padding */
	QCA_WLAN_VENDOR_ATTR_PAD = 13,
	/*
	 * Unique FTM session cookie (Unsigned 64 bit). Specified in
	 * QCA_NL80211_VENDOR_SUBCMD_FTM_START_SESSION. Reported in
	 * the session in QCA_NL80211_VENDOR_SUBCMD_FTM_MEAS_RESULT and
	 * QCA_NL80211_VENDOR_SUBCMD_FTM_SESSION_DONE.
	 */
	QCA_WLAN_VENDOR_ATTR_FTM_SESSION_COOKIE = 14,
	/*
	 * Indoor location capabilities, returned by
	 * QCA_NL80211_VENDOR_SUBCMD_LOC_GET_CAPA.
	 * see enum qca_wlan_vendor_attr_loc_capa.
	 */
	QCA_WLAN_VENDOR_ATTR_LOC_CAPA = 15,
	/*
	 * Array of nested attributes containing information about each peer
	 * in FTM measurement session. See enum qca_wlan_vendor_attr_peer_info
	 * for supported attributes for each peer.
	 */
	QCA_WLAN_VENDOR_ATTR_FTM_MEAS_PEERS = 16,
	/*
	 * Array of nested attributes containing measurement results for
	 * one or more peers, reported by the
	 * QCA_NL80211_VENDOR_SUBCMD_FTM_MEAS_RESULT event.
	 * See enum qca_wlan_vendor_attr_peer_result for list of supported
	 * attributes.
	 */
	QCA_WLAN_VENDOR_ATTR_FTM_MEAS_PEER_RESULTS = 17,
	/* Flag attribute for enabling or disabling responder functionality. */
	QCA_WLAN_VENDOR_ATTR_FTM_RESPONDER_ENABLE = 18,
	/*
	 * Used in the QCA_NL80211_VENDOR_SUBCMD_FTM_CFG_RESPONDER
	 * command to specify the LCI report that will be sent by
	 * the responder during a measurement exchange. The format is
	 * defined in IEEE P802.11-REVmc/D7.0, 9.4.2.22.10.
	 */
	QCA_WLAN_VENDOR_ATTR_FTM_LCI = 19,
	/*
	 * Used in the QCA_NL80211_VENDOR_SUBCMD_FTM_CFG_RESPONDER
	 * command to specify the location civic report that will
	 * be sent by the responder during a measurement exchange.
	 * The format is defined in IEEE P802.11-REVmc/D7.0, 9.4.2.22.13.
	 */
	QCA_WLAN_VENDOR_ATTR_FTM_LCR = 20,
	/*
	 * Session/measurement completion status code,
	 * reported in QCA_NL80211_VENDOR_SUBCMD_FTM_SESSION_DONE and
	 * QCA_NL80211_VENDOR_SUBCMD_AOA_MEAS_RESULT
	 * see enum qca_vendor_attr_loc_session_status.
	 */
	QCA_WLAN_VENDOR_ATTR_LOC_SESSION_STATUS = 21,
	/*
	 * Initial dialog token used by responder (0 if not specified),
	 * unsigned 8 bit value.
	 */
	QCA_WLAN_VENDOR_ATTR_FTM_INITIAL_TOKEN = 22,
	/*
	 * AOA measurement type. Requested in QCA_NL80211_VENDOR_SUBCMD_AOA_MEAS
	 * and optionally in QCA_NL80211_VENDOR_SUBCMD_FTM_START_SESSION if
	 * AOA measurements are needed as part of an FTM session.
	 * Reported by QCA_NL80211_VENDOR_SUBCMD_AOA_MEAS_RESULT. See
	 * enum qca_wlan_vendor_attr_aoa_type.
	 */
	QCA_WLAN_VENDOR_ATTR_AOA_TYPE = 23,
	/*
	 * A bit mask (unsigned 32 bit value) of antenna arrays used
	 * by indoor location measurements. Refers to the antenna
	 * arrays described by QCA_VENDOR_ATTR_LOC_CAPA_ANTENNA_ARRAYS.
	 */
	QCA_WLAN_VENDOR_ATTR_LOC_ANTENNA_ARRAY_MASK = 24,
	/*
	 * AOA measurement data. Its contents depends on the AOA measurement
	 * type and antenna array mask:
	 * QCA_WLAN_VENDOR_ATTR_AOA_TYPE_TOP_CIR_PHASE: array of U16 values,
	 * phase of the strongest CIR path for each antenna in the measured
	 * array(s).
	 * QCA_WLAN_VENDOR_ATTR_AOA_TYPE_TOP_CIR_PHASE_AMP: array of 2 U16
	 * values, phase and amplitude of the strongest CIR path for each
	 * antenna in the measured array(s).
	 */
	QCA_WLAN_VENDOR_ATTR_AOA_MEAS_RESULT = 25,
	/*
	 *  Used in QCA_NL80211_VENDOR_SUBCMD_GET_CHAIN_RSSI command
	 * to specify the chain number (unsigned 32 bit value) to inquire
	 * the corresponding antenna RSSI value */
	QCA_WLAN_VENDOR_ATTR_CHAIN_INDEX = 26,
	/*
	 *  Used in QCA_NL80211_VENDOR_SUBCMD_GET_CHAIN_RSSI command
	 * to report the specific antenna RSSI value (unsigned 32 bit value) */
	QCA_WLAN_VENDOR_ATTR_CHAIN_RSSI = 27,
	/* Frequency in MHz, various uses. Unsigned 32 bit value */
	QCA_WLAN_VENDOR_ATTR_FREQ = 28,
	/*
	 *  TSF timer value, unsigned 64 bit value.
	 * May be returned by various commands.
	 */
	QCA_WLAN_VENDOR_ATTR_TSF = 29,
	/*
	 * DMG RF sector index, unsigned 16 bit number. Valid values are
	 * 0..127 for sector indices or 65535 as special value used to
	 * unlock sector selection in
	 * QCA_NL80211_VENDOR_SUBCMD_DMG_RF_SET_SELECTED_SECTOR.
	 */
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_INDEX = 30,
	/*
	 * DMG RF sector type, unsigned 8 bit value. One of the values
	 * in enum qca_wlan_vendor_attr_dmg_rf_sector_type.
	 */
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_TYPE = 31,
	/*
	 * Bitmask of DMG RF modules for which information is requested. Each
	 * bit corresponds to an RF module with the same index as the bit
	 * number. Unsigned 32 bit number but only low 8 bits can be set since
	 * all DMG chips currently have up to 8 RF modules.
	 */
	QCA_WLAN_VENDOR_ATTR_DMG_RF_MODULE_MASK = 32,
	/*
	 * Array of nested attributes where each entry is DMG RF sector
	 * configuration for a single RF module.
	 * Attributes for each entry are taken from enum
	 * qca_wlan_vendor_attr_dmg_rf_sector_cfg.
	 * Specified in QCA_NL80211_VENDOR_SUBCMD_DMG_RF_SET_SECTOR_CFG
	 * and returned by QCA_NL80211_VENDOR_SUBCMD_DMG_RF_GET_SECTOR_CFG.
	 */
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG = 33,
	/*
	 * Used in QCA_NL80211_VENDOR_SUBCMD_STATS_EXT command
	 * to report frame aggregation statistics to userspace.
	 */
	QCA_WLAN_VENDOR_ATTR_RX_AGGREGATION_STATS_HOLES_NUM = 34,
	QCA_WLAN_VENDOR_ATTR_RX_AGGREGATION_STATS_HOLES_INFO = 35,
	/*
	 * Unsigned 8-bit value representing MBO transition reason code as
	 * provided by the AP used by subcommand
	 * QCA_NL80211_VENDOR_SUBCMD_FETCH_BSS_TRANSITION_STATUS. This is
	 * specified by the userspace in the request to the driver.
	 */
	QCA_WLAN_VENDOR_ATTR_BTM_MBO_TRANSITION_REASON = 36,
	/*
	 * Array of nested attributes, BSSID and status code, used by subcommand
	 * QCA_NL80211_VENDOR_SUBCMD_FETCH_BSS_TRANSITION_STATUS, where each
	 * entry is taken from enum qca_wlan_vendor_attr_btm_candidate_info.
	 * The userspace space specifies the list/array of candidate BSSIDs in
	 * the order of preference in the request. The driver specifies the
	 * status code, for each BSSID in the list, in the response. The
	 * acceptable candidates are listed in the order preferred by the
	 * driver.
	 */
	QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO = 37,
	/*
	 * Used in QCA_NL80211_VENDOR_SUBCMD_BRP_SET_ANT_LIMIT command
	 * See enum qca_wlan_vendor_attr_brp_ant_limit_mode.
	 */
	QCA_WLAN_VENDOR_ATTR_BRP_ANT_LIMIT_MODE = 38,
	/*
	 * Used in QCA_NL80211_VENDOR_SUBCMD_BRP_SET_ANT_LIMIT command
	 * to define the number of antennas to use for BRP.
	 * different purpose in each ANT_LIMIT_MODE:
	 * DISABLE - ignored
	 * EFFECTIVE - upper limit to number of antennas to be used
	 * FORCE - exact number of antennas to be used
	 * unsigned 8 bit value
	 */
	QCA_WLAN_VENDOR_ATTR_BRP_ANT_NUM_LIMIT = 39,
	/*
	 * Used in QCA_NL80211_VENDOR_SUBCMD_GET_CHAIN_RSSI command
	 * to report the corresponding antenna index to the chain RSSI value
	 */
	QCA_WLAN_VENDOR_ATTR_ANTENNA_INFO = 40,
	/*
	 * Used in QCA_NL80211_VENDOR_SUBCMD_GET_CHAIN_RSSI command
	 * to report the specific antenna EVM value (unsigned 32 bit value).
	 * With a determinate group of antennas, the driver specifies the
	 * EVM value for each antenna ID, and application extract them
	 * in user space.
	 */
	QCA_WLAN_VENDOR_ATTR_CHAIN_EVM = 41,
	/*
	 * Used in QCA_NL80211_VENDOR_SUBCMD_GET_FW_STATE command to report
	 * wlan firmware current state. FW state is an unsigned 8 bit value,
	 * one of the values in enum qca_wlan_vendor_attr_fw_state.
	 */
	QCA_WLAN_VENDOR_ATTR_FW_STATE = 42,

	/* Unsigned 32-bitmask value from enum qca_set_band. Substitutes the
	 * attribute QCA_WLAN_VENDOR_ATTR_SETBAND_VALUE for which only the
	 * integer values of enum qca_set_band till QCA_SETBAND_2G are valid.
	 * This attribute shall consider the bitmask combinations to define
	 * the respective Band combinations and always takes precedence over
	 * QCA_WLAN_VENDOR_ATTR_SETBAND_VALUE.
	 */
	QCA_WLAN_VENDOR_ATTR_SETBAND_MASK = 43,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_MAX = QCA_WLAN_VENDOR_ATTR_AFTER_LAST - 1
};

enum qca_wlan_vendor_attr_extscan_config_params {
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_INVALID = 0,

	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_REQUEST_ID = 1,

	/*
	 * Attributes for data used by
	 * QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_VALID_CHANNELS sub command.
	 */

	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_GET_VALID_CHANNELS_CONFIG_PARAM_WIFI_BAND
	= 2,
	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_GET_VALID_CHANNELS_CONFIG_PARAM_MAX_CHANNELS
	= 3,

	/*
	 * Attributes for input params used by
	 * QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_START sub command.
	 */

	/* Unsigned 32-bit value; channel frequency */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_CHANNEL_SPEC_CHANNEL = 4,
	/* Unsigned 32-bit value; dwell time in ms. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_CHANNEL_SPEC_DWELL_TIME = 5,
	/* Unsigned 8-bit value; 0: active; 1: passive; N/A for DFS */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_CHANNEL_SPEC_PASSIVE = 6,
	/* Unsigned 8-bit value; channel class */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_CHANNEL_SPEC_CLASS = 7,

	/* Unsigned 8-bit value; bucket index, 0 based */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_INDEX = 8,
	/* Unsigned 8-bit value; band. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_BAND = 9,
	/* Unsigned 32-bit value; desired period, in ms. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_PERIOD = 10,
	/* Unsigned 8-bit value; report events semantics. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_REPORT_EVENTS = 11,
	/*
	 * Unsigned 32-bit value. Followed by a nested array of
	 * EXTSCAN_CHANNEL_SPEC_* attributes.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_NUM_CHANNEL_SPECS = 12,

	/*
	 * Array of QCA_WLAN_VENDOR_ATTR_EXTSCAN_CHANNEL_SPEC_* attributes.
	 * Array size: QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_NUM_CHANNEL_SPECS
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_CHANNEL_SPEC = 13,

	/* Unsigned 32-bit value; base timer period in ms. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SCAN_CMD_PARAMS_BASE_PERIOD = 14,
	/*
	 * Unsigned 32-bit value; number of APs to store in each scan in the
	 * BSSID/RSSI history buffer (keep the highest RSSI APs).
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SCAN_CMD_PARAMS_MAX_AP_PER_SCAN = 15,
	/*
	 * Unsigned 8-bit value; in %, when scan buffer is this much full, wake
	 * up AP.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SCAN_CMD_PARAMS_REPORT_THRESHOLD_PERCENT
	= 16,

	/*
	 * Unsigned 8-bit value; number of scan bucket specs; followed by a
	 * nested array of_EXTSCAN_BUCKET_SPEC_* attributes and values. The size
	 * of the array is determined by NUM_BUCKETS.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SCAN_CMD_PARAMS_NUM_BUCKETS = 17,

	/*
	 * Array of QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_* attributes.
	 * Array size: QCA_WLAN_VENDOR_ATTR_EXTSCAN_SCAN_CMD_PARAMS_NUM_BUCKETS
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC = 18,

	/* Unsigned 8-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_GET_CACHED_SCAN_RESULTS_CONFIG_PARAM_FLUSH
	= 19,
	/* Unsigned 32-bit value; maximum number of results to be returned. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_GET_CACHED_SCAN_RESULTS_CONFIG_PARAM_MAX
	= 20,

	/* An array of 6 x unsigned 8-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_AP_THRESHOLD_PARAM_BSSID = 21,
	/* Signed 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_AP_THRESHOLD_PARAM_RSSI_LOW = 22,
	/* Signed 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_AP_THRESHOLD_PARAM_RSSI_HIGH = 23,
	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_AP_THRESHOLD_PARAM_CHANNEL = 24,

	/*
	 * Number of hotlist APs as unsigned 32-bit value, followed by a nested
	 * array of AP_THRESHOLD_PARAM attributes and values. The size of the
	 * array is determined by NUM_AP.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_BSSID_HOTLIST_PARAMS_NUM_AP = 25,

	/*
	 * Array of QCA_WLAN_VENDOR_ATTR_EXTSCAN_AP_THRESHOLD_PARAM_* attributes.
	 * Array size: QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_NUM_CHANNEL_SPECS
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_AP_THRESHOLD_PARAM = 26,

	/* Unsigned 32-bit value; number of samples for averaging RSSI. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SIGNIFICANT_CHANGE_PARAMS_RSSI_SAMPLE_SIZE
	= 27,
	/* Unsigned 32-bit value; number of samples to confirm AP loss. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SIGNIFICANT_CHANGE_PARAMS_LOST_AP_SAMPLE_SIZE
	= 28,
	/* Unsigned 32-bit value; number of APs breaching threshold. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SIGNIFICANT_CHANGE_PARAMS_MIN_BREACHING = 29,
	/*
	 * Unsigned 32-bit value; number of APs. Followed by an array of
	 * AP_THRESHOLD_PARAM attributes. Size of the array is NUM_AP.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SIGNIFICANT_CHANGE_PARAMS_NUM_AP = 30,
	/* Unsigned 32-bit value; number of samples to confirm AP loss. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_BSSID_HOTLIST_PARAMS_LOST_AP_SAMPLE_SIZE
	= 31,
	/*
	 * Unsigned 32-bit value. If max_period is non zero or different than
	 * period, then this bucket is an exponential backoff bucket.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_MAX_PERIOD = 32,
	/* Unsigned 32-bit value. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_BASE = 33,
	/*
	 * Unsigned 32-bit value. For exponential back off bucket, number of
	 * scans to perform for a given period.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_STEP_COUNT = 34,
	/*
	 * Unsigned 8-bit value; in number of scans, wake up AP after these
	 * many scans.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SCAN_CMD_PARAMS_REPORT_THRESHOLD_NUM_SCANS
	= 35,

	/*
	 * Attributes for data used by
	 * QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_SSID_HOTLIST sub command.
	 */

	/* Unsigned 3-2bit value; number of samples to confirm SSID loss. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SSID_HOTLIST_PARAMS_LOST_SSID_SAMPLE_SIZE
	= 36,
	/*
	 * Number of hotlist SSIDs as unsigned 32-bit value, followed by a
	 * nested array of SSID_THRESHOLD_PARAM_* attributes and values. The
	 * size of the array is determined by NUM_SSID.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SSID_HOTLIST_PARAMS_NUM_SSID = 37,
	/*
	 * Array of QCA_WLAN_VENDOR_ATTR_EXTSCAN_SSID_THRESHOLD_PARAM_*
	 * attributes.
	 * Array size: QCA_WLAN_VENDOR_ATTR_EXTSCAN_SSID_HOTLIST_PARAMS_NUM_SSID
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SSID_THRESHOLD_PARAM = 38,

	/* An array of 33 x unsigned 8-bit value; NULL terminated SSID */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SSID_THRESHOLD_PARAM_SSID = 39,
	/* Unsigned 8-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SSID_THRESHOLD_PARAM_BAND = 40,
	/* Signed 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SSID_THRESHOLD_PARAM_RSSI_LOW = 41,
	/* Signed 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SSID_THRESHOLD_PARAM_RSSI_HIGH = 42,
	/* Unsigned 32-bit value; a bitmask with additional extscan config flag.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_CONFIGURATION_FLAGS = 43,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_MAX =
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_AFTER_LAST - 1,
};

enum qca_wlan_vendor_attr_extscan_results {
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_INVALID = 0,

	/*
	 * Unsigned 32-bit value; must match the request Id supplied by
	 * Wi-Fi HAL in the corresponding subcmd NL msg.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_REQUEST_ID = 1,

	/*
	 * Unsigned 32-bit value; used to indicate the status response from
	 * firmware/driver for the vendor sub-command.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_STATUS = 2,

	/*
	 * EXTSCAN Valid Channels attributes */
	/* Unsigned 32bit value; followed by a nested array of CHANNELS. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_NUM_CHANNELS = 3,
	/*
	 * An array of NUM_CHANNELS x unsigned 32-bit value integers
	 * representing channel numbers.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CHANNELS = 4,

	/* EXTSCAN Capabilities attributes */

	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_SCAN_CACHE_SIZE = 5,
	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_SCAN_BUCKETS = 6,
	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_AP_CACHE_PER_SCAN
	= 7,
	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_RSSI_SAMPLE_SIZE
	= 8,
	/* Signed 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_SCAN_REPORTING_THRESHOLD
	= 9,
	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_HOTLIST_BSSIDS = 10,
	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_SIGNIFICANT_WIFI_CHANGE_APS
	= 11,
	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_BSSID_HISTORY_ENTRIES
	= 12,

	/*
	 * EXTSCAN Attributes used with
	 * QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SCAN_RESULTS_AVAILABLE sub-command.
	 */

	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_NUM_RESULTS_AVAILABLE = 13,

	/*
	 * EXTSCAN attributes used with
	 * QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_FULL_SCAN_RESULT sub-command.
	 */

	/*
	 * An array of NUM_RESULTS_AVAILABLE x
	 * QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_*
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_LIST = 14,

	/* Unsigned 64-bit value; age of sample at the time of retrieval */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_TIME_STAMP = 15,
	/* 33 x unsigned 8-bit value; NULL terminated SSID */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_SSID = 16,
	/* An array of 6 x unsigned 8-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_BSSID = 17,
	/* Unsigned 32-bit value; channel frequency in MHz */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_CHANNEL = 18,
	/* Signed 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_RSSI = 19,
	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_RTT = 20,
	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_RTT_SD = 21,
	/* Unsigned 16-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_BEACON_PERIOD = 22,
	/* Unsigned 16-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_CAPABILITY = 23,
	/* Unsigned 32-bit value; size of the IE DATA blob */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_IE_LENGTH = 24,
	/*
	 * An array of IE_LENGTH x unsigned 8-bit value; blob of all the
	 * information elements found in the beacon; this data should be a
	 * packed list of wifi_information_element objects, one after the
	 * other.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_IE_DATA = 25,

	/*
	 * Unsigned 8-bit value; set by driver to indicate more scan results are
	 * available.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_MORE_DATA = 26,

	/*
	 * EXTSCAN attributes for
	 * QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SCAN_EVENT sub-command.
	 */
	/* Unsigned 8-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_EVENT_TYPE = 27,
	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_EVENT_STATUS = 28,

	/*
	 * EXTSCAN attributes for
	 * QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_AP_FOUND sub-command.
	 */
	/*
	 * Use attr QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_NUM_RESULTS_AVAILABLE
	 * to indicate number of results.
	 * Also, use QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_LIST to indicate the
	 * list of results.
	 */

	/*
	 * EXTSCAN attributes for
	 * QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SIGNIFICANT_CHANGE sub-command.
	 */
	/* An array of 6 x unsigned 8-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SIGNIFICANT_CHANGE_RESULT_BSSID = 29,
	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SIGNIFICANT_CHANGE_RESULT_CHANNEL
	= 30,
	/* Unsigned 32-bit value. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SIGNIFICANT_CHANGE_RESULT_NUM_RSSI
	= 31,
	/*
	 * A nested array of signed 32-bit RSSI values. Size of the array is
	 * determined by (NUM_RSSI of SIGNIFICANT_CHANGE_RESULT_NUM_RSSI.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SIGNIFICANT_CHANGE_RESULT_RSSI_LIST
	= 32,

	/*
	 * EXTSCAN attributes used with
	 * QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_CACHED_RESULTS sub-command.
	 */
	/*
	 * Use attr QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_NUM_RESULTS_AVAILABLE
	 * to indicate number of extscan cached results returned.
	 * Also, use QCA_WLAN_VENDOR_ATTR_EXTSCAN_CACHED_RESULTS_LIST to indicate
	 *  the list of extscan cached results.
	 */

	/*
	 * An array of NUM_RESULTS_AVAILABLE x
	 * QCA_NL80211_VENDOR_ATTR_EXTSCAN_CACHED_RESULTS_*
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_CACHED_RESULTS_LIST = 33,
	/* Unsigned 32-bit value; a unique identifier for the scan unit. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_CACHED_RESULTS_SCAN_ID = 34,
	/*
	 * Unsigned 32-bit value; a bitmask w/additional information about scan.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_CACHED_RESULTS_FLAGS = 35,
	/*
	 * Use attr QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_NUM_RESULTS_AVAILABLE
	 * to indicate number of wifi scan results/bssids retrieved by the scan.
	 * Also, use QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_LIST to indicate the
	 * list of wifi scan results returned for each cached result block.
	 */

	/*
	 * EXTSCAN attributes for
	 * QCA_NL80211_VENDOR_SUBCMD_PNO_NETWORK_FOUND sub-command.
	 */
	/*
	 * Use QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_NUM_RESULTS_AVAILABLE for
	 * number of results.
	 * Use QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_LIST to indicate the nested
	 * list of wifi scan results returned for each
	 * wifi_passpoint_match_result block.
	 * Array size: QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_NUM_RESULTS_AVAILABLE.
	 */

	/*
	 * EXTSCAN attributes for
	 * QCA_NL80211_VENDOR_SUBCMD_PNO_PASSPOINT_NETWORK_FOUND sub-command.
	 */
	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_PNO_RESULTS_PASSPOINT_NETWORK_FOUND_NUM_MATCHES
	= 36,
	/*
	 * A nested array of
	 * QCA_WLAN_VENDOR_ATTR_EXTSCAN_PNO_RESULTS_PASSPOINT_MATCH_*
	 * attributes. Array size =
	 * *_ATTR_EXTSCAN_PNO_RESULTS_PASSPOINT_NETWORK_FOUND_NUM_MATCHES.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_PNO_RESULTS_PASSPOINT_MATCH_RESULT_LIST = 37,

	/* Unsigned 32-bit value; network block id for the matched network */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_PNO_RESULTS_PASSPOINT_MATCH_ID = 38,
	/*
	 * Use QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_LIST to indicate the nested
	 * list of wifi scan results returned for each
	 * wifi_passpoint_match_result block.
	 */
	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_PNO_RESULTS_PASSPOINT_MATCH_ANQP_LEN = 39,
	/*
	 * An array size of PASSPOINT_MATCH_ANQP_LEN of unsigned 8-bit values;
	 * ANQP data in the information_element format.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_PNO_RESULTS_PASSPOINT_MATCH_ANQP = 40,

	/* Unsigned 32-bit value; a EXTSCAN Capabilities attribute. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_HOTLIST_SSIDS = 41,
	/* Unsigned 32-bit value; a EXTSCAN Capabilities attribute. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_NUM_EPNO_NETS = 42,
	/* Unsigned 32-bit value; a EXTSCAN Capabilities attribute. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_NUM_EPNO_NETS_BY_SSID
	= 43,
	/* Unsigned 32-bit value; a EXTSCAN Capabilities attribute. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_NUM_WHITELISTED_SSID
	= 44,

	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_BUCKETS_SCANNED = 45,
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_MAX_NUM_BLACKLISTED_BSSID = 46,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_MAX =
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_AFTER_LAST - 1,
};

/**
 * enum qca_vendor_interop_issues_ap_type - interop issues type
 * This enum defines the valid set of values of interop issues type. These
 * values are used by attribute %QCA_WLAN_VENDOR_ATTR_INTEROP_ISSUES_AP_TYPE.
 *
 * @QCA_VENDOR_INTEROP_ISSUES_AP_ON_STA_PS: the ap has power save interop issue
 * when the STA's Qpower feature is enabled.
 */
enum qca_vendor_interop_issues_ap_type {
	QCA_VENDOR_INTEROP_ISSUES_AP_INVALID = 0,
	QCA_VENDOR_INTEROP_ISSUES_AP_ON_STA_PS = 1,
};

/**
 * enum qca_vendor_attr_interop_issues_ap - attribute for ap with interop issues
 * values are used by %QCA_NL80211_VENDOR_SUBCMD_INTEROP_ISSUES_AP.
 *
 * @QCA_WLAN_VENDOR_ATTR_INTEROP_ISSUES_AP_INVALID: invalid value
 * @QCA_WLAN_VENDOR_ATTR_INTEROP_ISSUES_AP_TYPE: interop issues type
 * 32-bit unsigned value, The type defined in enum
 * qca_vendor_interop_issues_ap_type are used.
 * @QCA_WLAN_VENDOR_ATTR_INTEROP_ISSUES_AP_LIST: aps' bssid container
 * array of nested QCA_WLAN_VENDOR_ATTR_INTEROP_ISSUES_AP_BSSID attributes,
 * it is present and mandatory for the command but is not used for
 * the event since only a single BSSID is reported in an event.
 * @QCA_WLAN_VENDOR_ATTR_INTEROP_ISSUES_AP_BSSID: ap's bssid
 * 6-byte MAC address. It is used within the nested
 * QCA_WLAN_VENDOR_ATTR_INTEROP_ISSUES_AP_LIST attribute in command case
 * and without such encapsulation in the event case.
 * @QCA_WLAN_VENDOR_ATTR_INTEROP_ISSUES_AP_AFTER_LAST: last value
 * @QCA_WLAN_VENDOR_ATTR_INTEROP_ISSUES_AP_MAX: max value
 */
enum qca_vendor_attr_interop_issues_ap {
	QCA_WLAN_VENDOR_ATTR_INTEROP_ISSUES_AP_INVALID,
	QCA_WLAN_VENDOR_ATTR_INTEROP_ISSUES_AP_TYPE,
	QCA_WLAN_VENDOR_ATTR_INTEROP_ISSUES_AP_LIST,
	QCA_WLAN_VENDOR_ATTR_INTEROP_ISSUES_AP_BSSID,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_INTEROP_ISSUES_AP_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_INTEROP_ISSUES_AP_MAX =
		QCA_WLAN_VENDOR_ATTR_INTEROP_ISSUES_AP_AFTER_LAST - 1
};

#ifdef WLAN_FEATURE_LINK_LAYER_STATS

/**
 * enum qca_wlan_vendor_attr_ll_stats_set - vendor attribute set stats
 *
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_SET_INVALID: Invalid initial value
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_SET_CONFIG_MPDU_SIZE_THRESHOLD: Size threshold
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_SET_CONFIG_AGGRESSIVE_STATS_GATHERING:
 *	Aggresive stats gathering
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_SET_AFTER_LAST: After last
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_SET_MAX: Max value
 */
enum qca_wlan_vendor_attr_ll_stats_set {
	QCA_WLAN_VENDOR_ATTR_LL_STATS_SET_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_SET_CONFIG_MPDU_SIZE_THRESHOLD = 1,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_SET_CONFIG_AGGRESSIVE_STATS_GATHERING,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_LL_STATS_SET_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_SET_MAX =
		QCA_WLAN_VENDOR_ATTR_LL_STATS_SET_AFTER_LAST - 1
};

/**
 * enum qca_wlan_vendor_attr_ll_stats_get - vendor attribute get stats
 *
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_GET_INVALID: Invalid initial value
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_GET_CONFIG_REQ_ID: Unsigned 32bit value
 *	provided by the caller issuing the GET stats command. When reporting
 *	the stats results, the driver uses the same value to indicate which
 *	GET request the results correspond to.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_GET_CONFIG_REQ_MASK: Get config request mask
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_CONFIG_RSP_MASK: Config response mask
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_CONFIG_STOP_RSP: Config stop response
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_GET_AFTER_LAST: After last
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_GET_MAX: Max value
 */
enum qca_wlan_vendor_attr_ll_stats_get {
	QCA_WLAN_VENDOR_ATTR_LL_STATS_GET_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_GET_CONFIG_REQ_ID,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_GET_CONFIG_REQ_MASK,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_GET_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_GET_MAX =
		QCA_WLAN_VENDOR_ATTR_LL_STATS_GET_AFTER_LAST - 1
};

/**
 * enum qca_wlan_vendor_attr_ll_stats_clr - vendor attribute clear stats
 *
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_INVALID: Invalid initial value
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_CONFIG_REQ_MASK: Config request mask
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_CONFIG_STOP_REQ: Config stop mask
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_CONFIG_RSP_MASK: Config response mask
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_CONFIG_STOP_RSP: Config stop response
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_AFTER_LAST: After last
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_MAX: Max value
 */
enum qca_wlan_vendor_attr_ll_stats_clr {
	QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_CONFIG_REQ_MASK,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_CONFIG_STOP_REQ,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_CONFIG_RSP_MASK,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_CONFIG_STOP_RSP,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_MAX =
		QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_AFTER_LAST - 1
};

/**
 * enum qca_wlan_vendor_attr_ll_stats_results_type - ll stats result type
 *
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE_INVALID: Initial invalid value
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE_RADIO: Link layer stats type radio
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE_IFACE: Link layer stats type interface
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE_PEER: Link layer stats type peer
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE_AFTER_LAST: Last value
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE_MAX: Max value
 */
enum qca_wlan_vendor_attr_ll_stats_results_type {
	QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE_INVALID = 0,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE_RADIO = 1,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE_IFACE,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE_PEER,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE_MAX =
		QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE_AFTER_LAST - 1
};

/**
 * enum qca_wlan_vendor_attr_ll_stats_results - vendor attribute stats results
 *
 * Attributes of type QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_* are nested
 * within the interface stats.
 *
 * Attributes of type QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_* could be nested
 * within the interface stats.
 *
 * Attributes of type QCA_WLAN_VENDOR_ATTR_LL_STATS_PEER_INFO_* are nested
 * within the interface stats.
 *
 * Attributes of type QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_* could be nested
 * within the peer info stats.
 *
 * Attributes of type QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_INFO_* could be
 * nested within the channel stats.
 *
 * Attributes of type QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_ could be nested
 * within the radio stats.
 *
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_INVALID: Invalid initial value
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RESULTS_REQ_ID: Unsigned 32bit value
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_BEACON_RX: Unsigned 32bit value
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_MGMT_RX: Unsigned 32bit value
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_MGMT_ACTION_RX: Unsigned 32bit value
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_MGMT_ACTION_TX: Unsigned 32bit value
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_RSSI_MGMT: Unsigned 32bit value
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_RSSI_DATA: Unsigned 32bit value
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_RSSI_ACK: Unsigned 32bit value
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_MODE: Interface mode, e.g., STA,
 *	SOFTAP, IBSS, etc. Type = enum wifi_interface_mode
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_MAC_ADDR: Interface MAC address.
 *	An array of 6 Unsigned int8_t
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_STATE:
 *	Type = enum wifi_connection_state,  e.g., DISCONNECTED, AUTHENTICATING,
 *	etc. Valid for STA, CLI only
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_ROAMING:
 *	Type = enum wifi_roam_state. Roaming state, e.g., IDLE or ACTIVE
 *	(is that valid for STA only?)
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_CAPABILITIES: Unsigned 32bit value.
 *	WIFI_CAPABILITY_XXX
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_SSID: NULL terminated SSID. An
 *	array of 33 Unsigned 8bit values
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_BSSID: BSSID. An array of 6
 *	Unsigned 8bit values
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_AP_COUNTRY_STR: Country string
 *	advertised by AP. An array of 3 Unsigned 8bit values
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_COUNTRY_STR: Country string for
 *	this association. An array of 3 Unsigned 8bit values
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_AC: Type = enum wifi_traffic_ac e.g.
 *	V0, VI, BE and BK
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_TX_MPDU: Unsigned int 32 value
 *	corresponding to respective AC
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_RX_MPDU: Unsigned int 32 value
 *	corresponding to respective AC
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_TX_MCAST: Unsigned int 32 value
 *	corresponding to respective AC
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_RX_MCAST: Unsigned int 32 value
 *	corresponding to respective AC
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_RX_AMPDU: Unsigned int 32 value
 *	corresponding to respective AC
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_TX_AMPDU: Unsigned int 32 value
 *	corresponding to respective AC
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_MPDU_LOST: Unsigned int 32 value
 *	corresponding to respective AC
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_RETRIES: Unsigned int 32 value
 *	corresponding to respective AC
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_RETRIES_SHORT: Unsigned int 32 value
 *	corresponding to respective AC
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_CONTENTION_TIME_MIN: Unsigned int 32
 *	value corresponding to respective AC
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_CONTENTION_TIME_MAX: Unsigned int 32
 *	value corresponding to respective AC
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_CONTENTION_TIME_AVG: Unsigned int 32
 *	value corresponding to respective AC
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_CONTENTION_NUM_SAMPLES: Unsigned int 32
 *	value corresponding to respective AC
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_NUM_PEERS: Unsigned int 32
 *	value corresponding to respective AC
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_PEER_INFO_TYPE: Type = enum wifi_peer_type
 *	Peer type, e.g., STA, AP, P2P GO etc
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_PEER_INFO_MAC_ADDRESS: MAC addr corresponding
 *	to respective peer. An array of 6 Unsigned 8bit values
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_PEER_INFO_CAPABILITIES: Unsigned int 32bit
 *	value representing capabilities corresponding to respective peer.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_PEER_INFO_NUM_RATES: Unsigned 32bit value.
 *	Number of rates
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_PREAMBLE: Unsigned int 8bit value:
 *	0: OFDM, 1:CCK, 2:HT 3:VHT 4..7 reserved
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_NSS: Unsigned int 8bit value:
 *	0:1x1, 1:2x2, 3:3x3, 4:4x4
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_BW: Unsigned int 8bit value:
 *	0:20MHz, 1:40Mhz, 2:80Mhz, 3:160Mhz
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_MCS_INDEX: Unsigned int 8bit value:
 *	OFDM/CCK rate code would be as per IEEE Std in the units of 0.5mbps
 *	HT/VHT it would be mcs index
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_BIT_RATE: Unsigned 32bit value.
 *	Bit rate in units of 100Kbps
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_TX_MPDU: Unsigned int 32bit value.
 *	Number of successfully transmitted data pkts i.e., with ACK received
 *	corresponding to the respective rate
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_RX_MPDU: Unsigned int 32bit value.
 *	Number of received data pkts corresponding to the respective rate
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_MPDU_LOST: Unsigned int 32bit value.
 *	Number of data pkts losses, i.e., no ACK received corresponding to
 *	the respective rate
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_RETRIES: Unsigned int 32bit value.
 *	Total number of data pkt retries for the respective rate
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_RETRIES_SHORT: Unsigned int 32bit value.
 *	Total number of short data pkt retries for the respective rate
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_RETRIES_LONG: Unsigned int 32bit value.
 *	Total number of long data pkt retries for the respective rate
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ID: Radio id
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME: Unsigned 32bit value.
 *	Total number of msecs the radio is awake accruing over time
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_TX_TIME: Unsigned 32bit value.
 *	Total number of msecs the radio is transmitting accruing over time
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_RX_TIME: Unsigned 32bit value.
 *	Total number of msecs the radio is in active receive accruing over time
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME_SCAN: Unsigned 32bit value.
 *	Total number of msecs the radio is awake due to all scan accruing
 *	over time
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME_NBD: Unsigned 32bit value.
 *	Total number of msecs the radio is awake due to NAN accruing over time.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME_GSCAN: Unsigned 32bit value.
 *	Total number of msecs the radio is awake due to GSCAN accruing over time
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME_ROAM_SCAN: Unsigned 32bit value.
 *	Total number of msecs the radio is awake due to roam scan accruing over
 *	time.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME_PNO_SCAN: Unsigned 32bit value.
 *	Total number of msecs the radio is awake due to PNO scan accruing over
 *	time.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME_HS20: Unsigned 32bit value.
 *	Total number of msecs the radio is awake due to HS2.0 scans and GAS
 *	exchange accruing over time.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_NUM_CHANNELS: Unsigned 32bit value.
 *	Number of channels
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_INFO_WIDTH:
 *	Type = enum wifi_channel_width. Channel width, e.g., 20, 40, 80, etc.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_INFO_CENTER_FREQ:
 *	Unsigned 32bit value. Primary 20MHz channel.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_INFO_CENTER_FREQ0:
 *	Unsigned 32bit value. Center frequency (MHz) first segment.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_INFO_CENTER_FREQ1:
 *	Unsigned 32bit value. Center frequency (MHz) second segment.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_ON_TIME: Unsigned int 32bit value
 *	representing total number of msecs the radio is awake on that channel
 *	accruing over time, corresponding to the respective channel.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_CCA_BUSY_TIME:  Unsigned int 32bit
 *	value representing total number of msecs the CCA register is busy
 *	accruing  over time corresponding to the respective channel.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_NUM_RADIOS: Number of radios
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CH_INFO: Channel info
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_PEER_INFO: Peer info
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_PEER_INFO_RATE_INFO: Peer rate info
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_INFO: WMM info
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RESULTS_MORE_DATA: Unsigned 8bit value.
 *	Used by the driver; if set to 1, it indicates that more stats, e.g.,
 *	peers or radio, are to follow in the next
 *	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_*_RESULTS event. Otherwise, it
 *	is set to 0.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_AVERAGE_TSF_OFFSET: tsf offset
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_LEAKY_AP_DETECTED: leaky ap detected
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_LEAKY_AP_AVG_NUM_FRAMES_LEAKED:
 *	average number of frames leaked
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_LEAKY_AP_GUARD_TIME: guard time
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE: Link Layer stats type
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_NUM_TX_LEVELS: LL Radio Number of
 *      Tx Levels
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_TX_TIME_PER_LEVEL:Number of msecs the
 *      radio spent in transmitting for each power level
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_RTS_SUCC_CNT: RTS successful count
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_RTS_FAIL_CNT: RTS fail count
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_PPDU_SUCC_CNT: PPDU successful count
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_PPDU_FAIL_CNT: PPDU fail count
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_TX_TIME:  Unsigned int 32bit
 *      value representing total number of msecs the radio is transmitting on
 *      this channel.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_RX_TIME:  Unsigned int 32bit
 *      value representing total number of msecs the radio is receiving all
 *      802.11 frames intended for this device on this channel.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_AFTER_LAST: After last
 * @QCA_WLAN_VENDOR_ATTR_FEATURE_SET_MAX: Max value
 */

enum qca_wlan_vendor_attr_ll_stats_results {
	QCA_WLAN_VENDOR_ATTR_LL_STATS_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RESULTS_REQ_ID = 1,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_BEACON_RX = 2,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_MGMT_RX = 3,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_MGMT_ACTION_RX = 4,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_MGMT_ACTION_TX = 5,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_RSSI_MGMT = 6,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_RSSI_DATA = 7,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_RSSI_ACK = 8,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_MODE = 9,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_MAC_ADDR = 10,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_STATE = 11,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_ROAMING = 12,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_CAPABILITIES = 13,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_SSID = 14,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_BSSID = 15,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_AP_COUNTRY_STR = 16,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_COUNTRY_STR = 17,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_AC = 18,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_TX_MPDU = 19,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_RX_MPDU = 20,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_TX_MCAST = 21,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_RX_MCAST = 22,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_RX_AMPDU = 23,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_TX_AMPDU = 24,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_MPDU_LOST = 25,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_RETRIES = 26,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_RETRIES_SHORT = 27,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_RETRIES_LONG = 28,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_CONTENTION_TIME_MIN = 29,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_CONTENTION_TIME_MAX = 30,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_CONTENTION_TIME_AVG = 31,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_CONTENTION_NUM_SAMPLES = 32,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_NUM_PEERS = 33,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_PEER_INFO_TYPE = 34,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_PEER_INFO_MAC_ADDRESS = 35,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_PEER_INFO_CAPABILITIES = 36,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_PEER_INFO_NUM_RATES = 37,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_PREAMBLE = 38,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_NSS = 39,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_BW = 40,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_MCS_INDEX = 41,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_BIT_RATE = 42,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_TX_MPDU = 43,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_RX_MPDU = 44,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_MPDU_LOST = 45,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_RETRIES = 46,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_RETRIES_SHORT = 47,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_RETRIES_LONG = 48,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ID = 49,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME = 50,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_TX_TIME = 51,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_RX_TIME = 52,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME_SCAN = 53,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME_NBD = 54,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME_GSCAN = 55,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME_ROAM_SCAN = 56,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME_PNO_SCAN = 57,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME_HS20 = 58,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_NUM_CHANNELS = 59,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_INFO_WIDTH = 60,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_INFO_CENTER_FREQ = 61,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_INFO_CENTER_FREQ0 = 62,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_INFO_CENTER_FREQ1 = 63,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_ON_TIME = 64,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_CCA_BUSY_TIME = 65,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_NUM_RADIOS = 66,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_CH_INFO = 67,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_PEER_INFO = 68,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_PEER_INFO_RATE_INFO = 69,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_INFO = 70,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_RESULTS_MORE_DATA = 71,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_AVERAGE_TSF_OFFSET = 72,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_LEAKY_AP_DETECTED = 73,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_LEAKY_AP_AVG_NUM_FRAMES_LEAKED = 74,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_LEAKY_AP_GUARD_TIME = 75,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE = 76,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_NUM_TX_LEVELS = 77,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_TX_TIME_PER_LEVEL = 78,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_RTS_SUCC_CNT = 79,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_RTS_FAIL_CNT = 80,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_PPDU_SUCC_CNT = 81,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_PPDU_FAIL_CNT = 82,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_TX_TIME = 84,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_RX_TIME = 85,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_LL_STATS_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_MAX =
		QCA_WLAN_VENDOR_ATTR_LL_STATS_AFTER_LAST - 1
};

enum qca_wlan_vendor_attr_ll_stats_type {
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_TYPE_INVALID = 0,
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_TYPE_RADIO = 1,
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_TYPE_IFACE = 2,
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_TYPE_PEERS = 3,

	/* keep last */
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_TYPE_AFTER_LAST,
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_TYPE_MAX =
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_TYPE_AFTER_LAST - 1,
};

#endif /* WLAN_FEATURE_LINK_LAYER_STATS */

/**
 * enum qca_wlan_vendor_attr_get_supported_features - get supported feature
 *
 * @QCA_WLAN_VENDOR_ATTR_FEATURE_SET_INVALID: Invalid initial value
 * @QCA_WLAN_VENDOR_ATTR_FEATURE_SET: Unsigned 32bit value
 * @QCA_WLAN_VENDOR_ATTR_FEATURE_SET_AFTER_LAST: After last
 * @QCA_WLAN_VENDOR_ATTR_FEATURE_SET_MAX: Max value
 */
enum qca_wlan_vendor_attr_get_supported_features {
	QCA_WLAN_VENDOR_ATTR_FEATURE_SET_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_FEATURE_SET = 1,
	QCA_WLAN_VENDOR_ATTR_FEATURE_SET_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_FEATURE_SET_MAX =
		QCA_WLAN_VENDOR_ATTR_FEATURE_SET_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_set_scanning_mac_oui - set scanning mac oui
 *
 * @QCA_WLAN_VENDOR_ATTR_SET_SCANNING_MAC_OUI_INVALID: Invalid initial value
 * @QCA_WLAN_VENDOR_ATTR_SET_SCANNING_MAC_OUI: An array of 3 x Unsigned 8-bit
 *	value
 * @QCA_WLAN_VENDOR_ATTR_SET_SCANNING_MAC_OUI_AFTER_LAST: After last
 * @QCA_WLAN_VENDOR_ATTR_SET_SCANNING_MAC_OUI_MAX: Max value
 */
enum qca_wlan_vendor_attr_set_scanning_mac_oui {
	QCA_WLAN_VENDOR_ATTR_SET_SCANNING_MAC_OUI_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_SET_SCANNING_MAC_OUI = 1,
	QCA_WLAN_VENDOR_ATTR_SET_SCANNING_MAC_OUI_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_SET_SCANNING_MAC_OUI_MAX =
		QCA_WLAN_VENDOR_ATTR_SET_SCANNING_MAC_OUI_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_scan - Specifies vendor scan attributes
 *
 * @QCA_WLAN_VENDOR_ATTR_SCAN_IE: IEs that should be included as part of scan
 * @QCA_WLAN_VENDOR_ATTR_SCAN_FREQUENCIES: Nested unsigned 32-bit attributes
 *	with frequencies to be scanned (in MHz)
 * @QCA_WLAN_VENDOR_ATTR_SCAN_SSIDS: Nested attribute with SSIDs to be scanned
 * @QCA_WLAN_VENDOR_ATTR_SCAN_SUPP_RATES: Nested array attribute of supported
 *	rates to be included
 * @QCA_WLAN_VENDOR_ATTR_SCAN_TX_NO_CCK_RATE: flag used to send probe requests
 *	at non CCK rate in 2GHz band
 * @QCA_WLAN_VENDOR_ATTR_SCAN_FLAGS: Unsigned 32-bit scan flags
 * @QCA_WLAN_VENDOR_ATTR_SCAN_COOKIE: Unsigned 64-bit cookie provided by the
 *	driver for the specific scan request
 * @QCA_WLAN_VENDOR_ATTR_SCAN_STATUS: Unsigned 8-bit status of the scan
 *	request decoded as in enum scan_status
 * @QCA_WLAN_VENDOR_ATTR_SCAN_MAC: 6-byte MAC address to use when randomisation
 *	scan flag is set
 * @QCA_WLAN_VENDOR_ATTR_SCAN_MAC_MASK: 6-byte MAC address mask to be used with
 *	randomisation
 * @QCA_WLAN_VENDOR_ATTR_SCAN_BSSID: 6-byte MAC address representing the
 *	specific BSSID to scan for.
 * @QCA_WLAN_VENDOR_ATTR_SCAN_DWELL_TIME: Unsigned 64-bit dwell time in
 *	microseconds. This is a common value which applies across all
 *	frequencies specified by QCA_WLAN_VENDOR_ATTR_SCAN_FREQUENCIES.
 */
enum qca_wlan_vendor_attr_scan {
	QCA_WLAN_VENDOR_ATTR_SCAN_INVALID_PARAM = 0,
	QCA_WLAN_VENDOR_ATTR_SCAN_IE = 1,
	QCA_WLAN_VENDOR_ATTR_SCAN_FREQUENCIES = 2,
	QCA_WLAN_VENDOR_ATTR_SCAN_SSIDS = 3,
	QCA_WLAN_VENDOR_ATTR_SCAN_SUPP_RATES = 4,
	QCA_WLAN_VENDOR_ATTR_SCAN_TX_NO_CCK_RATE = 5,
	QCA_WLAN_VENDOR_ATTR_SCAN_FLAGS = 6,
	QCA_WLAN_VENDOR_ATTR_SCAN_COOKIE = 7,
	QCA_WLAN_VENDOR_ATTR_SCAN_STATUS = 8,
	QCA_WLAN_VENDOR_ATTR_SCAN_MAC = 9,
	QCA_WLAN_VENDOR_ATTR_SCAN_MAC_MASK = 10,
	QCA_WLAN_VENDOR_ATTR_SCAN_BSSID = 11,
	QCA_WLAN_VENDOR_ATTR_SCAN_DWELL_TIME = 12,
	QCA_WLAN_VENDOR_ATTR_SCAN_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_SCAN_MAX =
	QCA_WLAN_VENDOR_ATTR_SCAN_AFTER_LAST - 1
};

/**
 * enum scan_status - Specifies the valid values the vendor scan attribute
 *	QCA_WLAN_VENDOR_ATTR_SCAN_STATUS can take
 * @VENDOR_SCAN_STATUS_NEW_RESULTS: implies the vendor scan is successful with
 *	new scan results
 * @VENDOR_SCAN_STATUS_ABORTED: implies the vendor scan was aborted in-between
 */
enum scan_status {
	VENDOR_SCAN_STATUS_NEW_RESULTS,
	VENDOR_SCAN_STATUS_ABORTED,
	VENDOR_SCAN_STATUS_MAX,
};

/**
 * enum qca_wlan_vendor_attr_get_concurrency_matrix - get concurrency matrix
 *
 * NL attributes for data used by
 * QCA_NL80211_VENDOR_SUBCMD_GET_CONCURRENCY_MATRIX sub command.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_CONCURRENCY_MATRIX_INVALID: Invalid initial value
 * @QCA_WLAN_VENDOR_ATTR_GET_CONCURRENCY_MATRIX_CONFIG_PARAM_SET_SIZE_MAX:
 *	Unsigned 32-bit value
 * @QCA_WLAN_VENDOR_ATTR_GET_CONCURRENCY_MATRIX_RESULTS_SET_SIZE:
 *	Unsigned 32-bit value
 * @QCA_WLAN_VENDOR_ATTR_GET_CONCURRENCY_MATRIX_RESULTS_SET: Set results. An
 *	array of SET_SIZE x Unsigned 32bit values representing concurrency
 *	combinations
 * @QCA_WLAN_VENDOR_ATTR_GET_CONCURRENCY_MATRIX_AFTER_LAST: After last
 * @QCA_WLAN_VENDOR_ATTR_GET_CONCURRENCY_MATRIX_MAX: Max value
 */
enum qca_wlan_vendor_attr_get_concurrency_matrix {
	QCA_WLAN_VENDOR_ATTR_GET_CONCURRENCY_MATRIX_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_GET_CONCURRENCY_MATRIX_CONFIG_PARAM_SET_SIZE_MAX
									= 1,
	QCA_WLAN_VENDOR_ATTR_GET_CONCURRENCY_MATRIX_RESULTS_SET_SIZE = 2,
	QCA_WLAN_VENDOR_ATTR_GET_CONCURRENCY_MATRIX_RESULTS_SET = 3,
	QCA_WLAN_VENDOR_ATTR_GET_CONCURRENCY_MATRIX_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_GET_CONCURRENCY_MATRIX_MAX =
		QCA_WLAN_VENDOR_ATTR_GET_CONCURRENCY_MATRIX_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_set_no_dfs_flag - vendor attribute set no dfs flag
 *
 * @QCA_WLAN_VENDOR_ATTR_SET_NO_DFS_FLAG_INVALID: Invalid initial value
 * @QCA_WLAN_VENDOR_ATTR_SET_NO_DFS_FLAG: Unsigned 32-bit value
 * @QCA_WLAN_VENDOR_ATTR_SET_NO_DFS_FLAG_AFTER_LAST: After last
 * @QCA_WLAN_VENDOR_ATTR_SET_NO_DFS_FLAG_MAX: Max value
 */
enum qca_wlan_vendor_attr_set_no_dfs_flag {
	QCA_WLAN_VENDOR_ATTR_SET_NO_DFS_FLAG_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_SET_NO_DFS_FLAG = 1,
	QCA_WLAN_VENDOR_ATTR_SET_NO_DFS_FLAG_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_SET_NO_DFS_FLAG_MAX =
		QCA_WLAN_VENDOR_ATTR_SET_NO_DFS_FLAG_AFTER_LAST - 1,
};

/**
 * enum qca_vendor_attr_wisa_cmd
 * @QCA_WLAN_VENDOR_ATTR_WISA_INVALID: Invalid attr
 * @QCA_WLAN_VENDOR_ATTR_WISA_MODE: WISA mode value attr (u32)
 * @QCA_WLAN_VENDOR_ATTR_WISA_AFTER_LAST: After last
 * @QCA_WLAN_VENDOR_ATTR_WISA_MAX: Max Value
 * WISA setup vendor commands
 */
enum qca_vendor_attr_wisa_cmd {
	QCA_WLAN_VENDOR_ATTR_WISA_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_WISA_MODE,
	QCA_WLAN_VENDOR_ATTR_WISA_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_WISA_MAX =
		QCA_WLAN_VENDOR_ATTR_WISA_AFTER_LAST - 1
};

enum qca_roaming_policy {
	QCA_ROAMING_NOT_ALLOWED,
	QCA_ROAMING_ALLOWED_WITHIN_ESS,
};

/**
 * enum qca_roam_reason - Represents the reason codes for roaming. Used by
 * QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_REASON.
 *
 * @QCA_ROAM_REASON_UNKNOWN: Any reason that do not classify under the below
 * reasons.
 *
 * @QCA_ROAM_REASON_PER: Roam triggered when packet error rates(PER) breached
 * the configured threshold.
 *
 * @QCA_ROAM_REASON_BEACON_MISS: Roam triggered due to the continuous configured
 * beacon misses from the then connected AP.
 *
 * @QCA_ROAM_REASON_POOR_RSSI: Roam triggered due to the poor RSSI reported
 * by the connected AP.
 *
 * @QCA_ROAM_REASON_BETTER_RSSI: Roam triggered for finding a BSSID with a
 * better RSSI than the connected BSSID. Here the RSSI of the current BSSID is
 * not poor.
 *
 * @QCA_ROAM_REASON_CONGESTION: Roam triggered considering the connected channel
 * or environment being very noisy / congested.
 *
 * @QCA_ROAM_REASON_EXPLICIT_REQUEST: Roam triggered due to an explicit request
 * from the user (user space).
 *
 * @QCA_ROAM_REASON_BTM: Roam triggered due to BTM request frame received from
 * connected AP.
 *
 * @QCA_ROAM_REASON_BSS_LOAD: Roam triggered due to the channel utilization
 * breaching out the configured threshold.
 *
 */
enum qca_roam_reason {
	QCA_ROAM_REASON_UNKNOWN,
	QCA_ROAM_REASON_PER,
	QCA_ROAM_REASON_BEACON_MISS,
	QCA_ROAM_REASON_POOR_RSSI,
	QCA_ROAM_REASON_BETTER_RSSI,
	QCA_ROAM_REASON_CONGESTION,
	QCA_ROAM_REASON_USER_TRIGGER,
	QCA_ROAM_REASON_BTM,
	QCA_ROAM_REASON_BSS_LOAD,
};

/**
 * enum qca_wlan_vendor_attr_roam_auth - vendor event for roaming
 * @QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_BSSID: BSSID of the roamed AP
 * @QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_REQ_IE: Request IE
 * @QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_RESP_IE: Response IE
 * @QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_AUTHORIZED: Authorization Status
 * @QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_KEY_REPLAY_CTR: Replay Counter
 * @QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_PTK_KCK: KCK of the PTK
 * @QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_PTK_KEK: KEK of the PTK
 * @QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_SUBNET_STATUS: subnet change status
 * @QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_STATUS:
 * Indicates the status of re-association requested by user space for
 * the BSSID specified by QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_BSSID.
 * Type u16.
 * Represents the status code from AP. Use
 * %WLAN_STATUS_UNSPECIFIED_FAILURE if the device cannot give you the
 * real status code for failures.
 * @QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_RETAIN_CONNECTION:
 * This attribute indicates that the old association was maintained when
 * a re-association is requested by user space and that re-association
 * attempt fails (i.e., cannot connect to the requested BSS, but can
 * remain associated with the BSS with which the association was in
 * place when being requested to roam). Used along with
 * WLAN_VENDOR_ATTR_ROAM_AUTH_STATUS to indicate the current
 * re-association status. Type flag.
 * This attribute is applicable only for re-association failure cases.
 * @QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_PMK: AUTH PMK
 * @QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_PMKID: AUTH PMKID
 * @QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_FILS_ERP_NEXT_SEQ_NUM: FILS erp next
 * seq number
 * @QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_REASON: A 16-bit unsigned value
 * representing the reasons for the roaming. Defined by enum
 * qca_roam_reason.
 */
enum qca_wlan_vendor_attr_roam_auth {
	QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_BSSID,
	QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_REQ_IE,
	QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_RESP_IE,
	QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_AUTHORIZED,
	QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_KEY_REPLAY_CTR,
	QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_PTK_KCK,
	QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_PTK_KEK,
	QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_SUBNET_STATUS,
	QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_STATUS,
	QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_RETAIN_CONNECTION,
	QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_PMK,
	QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_PMKID,
	QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_FILS_ERP_NEXT_SEQ_NUM,
	QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_REASON,
	QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_MAX =
		QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_AFTER_LAST - 1
};

/**
 * enum qca_wlan_vendor_attr_wifi_config - wifi config
 *
 * @QCA_WLAN_VENDOR_ATTR_WIFI_CONFIG_INVALID: Invalid initial value
 * @QCA_WLAN_VENDOR_ATTR_WIFI_CONFIG_DYNAMIC_DTIM: dynamic DTIM
 * @QCA_WLAN_VENDOR_ATTR_WIFI_CONFIG_STATS_AVG_FACTOR: avg factor
 * @QCA_WLAN_VENDOR_ATTR_WIFI_CONFIG_GUARD_TIME: guard time
 * @QCA_WLAN_VENDOR_ATTR_WIFI_CONFIG_AFTER_LAST: after last
 * @QCA_WLAN_VENDOR_ATTR_WIFI_CONFIG_MAX: max value
 */
enum qca_wlan_vendor_attr_wifi_config {
	QCA_WLAN_VENDOR_ATTR_WIFI_CONFIG_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_WIFI_CONFIG_DYNAMIC_DTIM = 1,
	QCA_WLAN_VENDOR_ATTR_WIFI_CONFIG_STATS_AVG_FACTOR = 2,
	QCA_WLAN_VENDOR_ATTR_WIFI_CONFIG_GUARD_TIME = 3,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_WIFI_CONFIG_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_WIFI_CONFIG_MAX =
		QCA_WLAN_VENDOR_ATTR_WIFI_CONFIG_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_epno_type - the type of request to the EPNO command
 * @QCA_WLAN_EPNO: epno type request
 * @QCA_WLAN_PNO: pno type request
 */
enum qca_wlan_epno_type {
	QCA_WLAN_EPNO,
	QCA_WLAN_PNO
};

/**
 * enum qca_wlan_vendor_attr_pno_config_params - pno config params
 *
 * @QCA_WLAN_VENDOR_ATTR_PNO_INVALID - Invalid initial value
 *
 *	NL attributes for data used by
 *	QCA_NL80211_VENDOR_SUBCMD_PNO_SET_PASSPOINT_LIST sub command.
 * @QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_LIST_PARAM_NUM:
 *	Unsigned 32-bit value; pno passpoint number of networks
 * @QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_LIST_PARAM_NETWORK_ARRAY:
 *	Array of nested QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_NETWORK_PARAM_*
 *	attributes. Array size =
 *	QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_LIST_PARAM_NUM.
 * @QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_NETWORK_PARAM_ID:
 *	Unsigned 32-bit value; network id
 * @QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_NETWORK_PARAM_REALM:
 *	An array of 256 x Unsigned 8-bit value; NULL terminated UTF8 encoded
 *	realm, 0 if unspecified.
 * @QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_NETWORK_PARAM_ROAM_CNSRTM_ID:
 *	An array of 16 x Unsigned 32-bit value; roaming consortium ids
 *	to match, 0 if unspecified.
 * @QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_NETWORK_PARAM_ROAM_PLMN:
 *	An array of 6 x Unsigned 8-bit value; mcc/mnc combination, 0s if
 *	unspecified.
 *
 *	NL attributes for data used by
 *	QCA_NL80211_VENDOR_SUBCMD_PNO_SET_LIST sub command.
 * @QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_NUM_NETWORKS:
 *	Unsigned 32-bit value; set pno number of networks
 * @QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORKS_LIST:
 *	Array of nested
 *	QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORK_*
 *	attributes. Array size =
 *		QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_NUM_NETWORKS
 * @QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORK_SSID:
 *	An array of 33 x Unsigned 8-bit value; NULL terminated SSID
 * @QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORK_RSSI_THRESHOLD:
 *	Signed 8-bit value; threshold for considering this SSID as found,
 *	required granularity for this threshold is 4dBm to 8dBm
 * @QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORK_FLAGS:
 *	Unsigned 8-bit value; WIFI_PNO_FLAG_XXX
 * @QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORK_AUTH_BIT:
 *	Unsigned 8-bit value; auth bit field for matching WPA IE
 * @QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_TYPE
 *	Unsigned 8-bit to indicate ePNO type; values from qca_wlan_epno_type
 *@QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_CHANNEL_LIST
 *	Nested attribute to send the channel list
 *@QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_SCAN_INTERVAL
 *	Unsigned 32-bit value; indicates the Interval between PNO scan
 *	cycles in msec
 *@QCA_WLAN_VENDOR_ATTR_EPNO_MIN5GHZ_RSSI
 *	Signed 32-bit value; minimum 5GHz RSSI for a BSSID to be considered
 *@QCA_WLAN_VENDOR_ATTR_EPNO_MIN24GHZ_RSSI
 *	Signed 32-bit value; minimum 2.4GHz RSSI for a BSSID to be considered
 *	This attribute is obsolete now.
 *@QCA_WLAN_VENDOR_ATTR_EPNO_INITIAL_SCORE_MAX
 *	Signed 32-bit value; the maximum score that a network
 *	can have before bonuses
 *@QCA_WLAN_VENDOR_ATTR_EPNO_CURRENT_CONNECTION_BONUS
 *	Signed 32-bit value; only report when there is a network's
 *	score this much higher han the current connection
 *@QCA_WLAN_VENDOR_ATTR_EPNO_SAME_NETWORK_BONUS
 *	Signed 32-bit value; score bonus for all networks with
 *	the same network flag
 *@QCA_WLAN_VENDOR_ATTR_EPNO_SECURE_BONUS
 *	Signed 32-bit value; score bonus for networks that are not open
 *@QCA_WLAN_VENDOR_ATTR_EPNO_BAND5GHZ_BONUS
 *	Signed 32-bit value; 5GHz RSSI score bonus applied to all
 *	5GHz networks
 *@QCA_WLAN_VENDOR_ATTR_PNO_CONFIG_REQUEST_ID
 *	Unsigned 32-bit value, representing the PNO Request ID
 * @QCA_WLAN_VENDOR_ATTR_PNO_AFTER_LAST: After last
 * @QCA_WLAN_VENDOR_ATTR_PNO_MAX: max
 */
enum qca_wlan_vendor_attr_pno_config_params {
	QCA_WLAN_VENDOR_ATTR_PNO_INVALID = 0,

	QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_LIST_PARAM_NUM = 1,
	QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_LIST_PARAM_NETWORK_ARRAY = 2,
	QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_NETWORK_PARAM_ID = 3,
	QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_NETWORK_PARAM_REALM = 4,
	QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_NETWORK_PARAM_ROAM_CNSRTM_ID = 5,
	QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_NETWORK_PARAM_ROAM_PLMN = 6,

	QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_NUM_NETWORKS = 7,
	QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORKS_LIST = 8,
	QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORK_SSID = 9,
	QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORK_RSSI_THRESHOLD = 10,
	QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORK_FLAGS = 11,
	QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORK_AUTH_BIT = 12,
	QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_TYPE = 13,
	QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_CHANNEL_LIST = 14,
	QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_SCAN_INTERVAL = 15,
	QCA_WLAN_VENDOR_ATTR_EPNO_MIN5GHZ_RSSI = 16,
	QCA_WLAN_VENDOR_ATTR_EPNO_MIN24GHZ_RSSI = 17,
	QCA_WLAN_VENDOR_ATTR_EPNO_INITIAL_SCORE_MAX = 18,
	QCA_WLAN_VENDOR_ATTR_EPNO_CURRENT_CONNECTION_BONUS = 19,
	QCA_WLAN_VENDOR_ATTR_EPNO_SAME_NETWORK_BONUS = 20,
	QCA_WLAN_VENDOR_ATTR_EPNO_SECURE_BONUS = 21,
	QCA_WLAN_VENDOR_ATTR_EPNO_BAND5GHZ_BONUS = 22,

	QCA_WLAN_VENDOR_ATTR_PNO_CONFIG_REQUEST_ID = 23,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_PNO_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_PNO_MAX =
		QCA_WLAN_VENDOR_ATTR_PNO_AFTER_LAST - 1,
};

/**
 * enum qca_scan_freq_list_type: Frequency list types
 *
 * @QCA_PREFERRED_SCAN_FREQ_LIST: The driver shall use the scan frequency list
 *	specified with attribute QCA_ATTR_ROAM_CONTROL_SCAN_FREQ_LIST as
 *	a preferred frequency list for roaming.
 *
 * @QCA_SPECIFIC_SCAN_FREQ_LIST: The driver shall use the frequency list
 *	specified with attribute QCA_ATTR_ROAM_CONTROL_SCAN_FREQ_LIST as
 *	a specific frequency list for roaming.
 */
enum qca_scan_freq_list_type {
	QCA_PREFERRED_SCAN_FREQ_LIST = 1,
	QCA_SPECIFIC_SCAN_FREQ_LIST = 2,
};

/**
 * enum qca_roam_scan_scheme: scan scheme
 *
 * @QCA_ROAM_SCAN_SCHEME_NO_SCAN: No Frequencies specified to scan.
 *	Indicates the driver to not scan on a Roam Trigger scenario, but
 *	Disconnect.e.g., On a BTM request from the AP driver/firmware shall
 *	disconnect from the current connected AP by notifying a failure
 *	code in the BTM response.
 *
 * @QCA_ROAM_SCAN_SCHEME_PARTIAL_SCAN: Indicates the driver/firmware to
 *	trigger partial frequency scans. These frequencies are the ones learnt
 *	or maintained by the driver based on the probability of finding the
 *	BSSID's in the ESS for which the roaming is triggered.
 *
 * @QCA_ROAM_SCAN_SCHEME_FULL_SCAN: Indicates the driver/firmware to
 *	trigger the scan on all the valid frequencies to find the better
 *	candidates to roam.
 */
enum qca_roam_scan_scheme {
	QCA_ROAM_SCAN_SCHEME_NO_SCAN = 0,
	QCA_ROAM_SCAN_SCHEME_PARTIAL_SCAN = 1,
	QCA_ROAM_SCAN_SCHEME_FULL_SCAN = 2,
};

/**
 * enum qca_vendor_attr_scan_freq_list_scheme: Frequency list scheme
 *
 * @QCA_ATTR_ROAM_CONTROL_SCAN_FREQ_LIST: An array of unsigned 32-bit values.
 *	List of frequencies in MHz to be considered for a roam scan.
 *
 * @QCA_ATTR_ROAM_CONTROL_SCAN_FREQ_LIST_TYPE: Unsigned 32-bit value.
 *	Type of frequency list scheme being configured/gotten as defined by the
 *	enum qca_scan_freq_list_type.
 */
enum qca_vendor_attr_scan_freq_list_scheme {
	QCA_ATTR_ROAM_CONTROL_SCAN_FREQ_LIST = 1,
	QCA_ATTR_ROAM_CONTROL_SCAN_FREQ_LIST_TYPE = 2,

	/* keep last */
	QCA_ATTR_ROAM_CONTROL_SCAN_FREQ_LIST_SCHEME_AFTER_LAST,
	QCA_ATTR_ROAM_CONTROL_SCAN_FREQ_LIST_SCHEME_MAX =
	QCA_ATTR_ROAM_CONTROL_SCAN_FREQ_LIST_SCHEME_AFTER_LAST - 1,
};

/*
 * enum qca_vendor_roam_triggers: Bitmap of roaming triggers
 *
 * @QCA_ROAM_TRIGGER_REASON_PER: Set if the roam has to be triggered based on
 *	a bad packet error rates (PER).
 * @QCA_ROAM_TRIGGER_REASON_BEACON_MISS: Set if the roam has to be triggered
 *	based on beacon misses from the connected AP.
 * @QCA_ROAM_TRIGGER_REASON_POOR_RSSI: Set if the roam has to be triggered
 *	due to poor RSSI of the connected AP.
 * @QCA_ROAM_TRIGGER_REASON_BETTER_RSSI: Set if the roam has to be triggered
 *	upon finding a BSSID with a better RSSI than the connected BSSID.
 *	Here the RSSI of the current BSSID need not be poor.
 * @QCA_ROAM_TRIGGER_REASON_PERIODIC: Set if the roam has to be triggered
 *	by triggering a periodic scan to find a better AP to roam.
 * @QCA_ROAM_TRIGGER_REASON_DENSE: Set if the roam has to be triggered
 *	when the connected channel environment is too noisy/congested.
 * @QCA_ROAM_TRIGGER_REASON_BTM: Set if the roam has to be triggered
 *	when BTM Request frame is received from the connected AP.
 * @QCA_ROAM_TRIGGER_REASON_BSS_LOAD: Set if the roam has to be triggered
 *	when the channel utilization is goes above the configured threshold.
 * @QCA_ROAM_TRIGGER_REASON_USER_TRIGGER: Set if the roam has to be triggered
 *	based on the request from the user (space).
 * @QCA_ROAM_TRIGGER_REASON_DEAUTH: Set if the roam has to be triggered when
 *	device receives Deauthentication/Disassociation frame from connected
 *	AP.
 * @QCA_ROAM_TRIGGER_REASON_IDLE: Set if the roam has to be triggered when the
 *	DUT is in idle state (no tx/rx) and suspend mode, if the current RSSI
 *	is determined to be a poor one.
 * @QCA_ROAM_TRIGGER_REASON_TX_FAILURES: Set if the roam has to be triggered
 *	based on continuous TX Data Frame failures to the connected AP.
 * @QCA_ROAM_TRIGGER_REASON_EXTERNAL_SCAN: Set if the roam has to be triggered
 *	based on the scan results obtained from an external scan (not
 *	triggered to aim roaming)
 *
 * Set the corresponding roam trigger reason bit to consider it for roam
 * trigger.
 * Userspace can set multiple bits and send to the driver. The driver shall
 * consider all of them to trigger/initiate a roam scan.
 */
enum qca_vendor_roam_triggers {
	QCA_ROAM_TRIGGER_REASON_PER		= 1 << 0,
	QCA_ROAM_TRIGGER_REASON_BEACON_MISS	= 1 << 1,
	QCA_ROAM_TRIGGER_REASON_POOR_RSSI	= 1 << 2,
	QCA_ROAM_TRIGGER_REASON_BETTER_RSSI	= 1 << 3,
	QCA_ROAM_TRIGGER_REASON_PERIODIC	= 1 << 4,
	QCA_ROAM_TRIGGER_REASON_DENSE		= 1 << 5,
	QCA_ROAM_TRIGGER_REASON_BTM		= 1 << 6,
	QCA_ROAM_TRIGGER_REASON_BSS_LOAD	= 1 << 7,
	QCA_ROAM_TRIGGER_REASON_USER_TRIGGER	= 1 << 8,
	QCA_ROAM_TRIGGER_REASON_DEAUTH          = 1 << 9,
	QCA_ROAM_TRIGGER_REASON_IDLE		= 1 << 10,
	QCA_ROAM_TRIGGER_REASON_TX_FAILURES	= 1 << 11,
	QCA_ROAM_TRIGGER_REASON_EXTERNAL_SCAN	= 1 << 12,
};

/**
 * enum qca_vendor_attr_roam_candidate_selection_criteria:
 *
 * Each attribute carries a weightage in percentage (%).
 *
 * @QCA_ATTR_ROAM_CAND_SEL_CRITERIA_SCORE_RSSI: Unsigned 8-bit value.
 *	Represents the weightage to be given for the RSSI selection
 *	criteria among other parameters.
 *
 * @QCA_ATTR_ROAM_CAND_SEL_CRITERIA_RATE: Unsigned 8-bit value.
 *	Represents the weightage to be given for the rate selection
 *	criteria among other parameters.
 *
 * @QCA_ATTR_ROAM_CAND_SEL_CRITERIA_SCORE_BW: Unsigned 8-bit value.
 *	Represents the weightage to be given for the band width selection
 *	criteria among other parameters.
 *
 * @QCA_ATTR_ROAM_CAND_SEL_CRITERIA_SCORE_BAND: Unsigned 8-bit value.
 *	Represents the weightage to be given for the band selection
 *	criteria among other parameters.
 *
 * @QCA_ATTR_ROAM_CAND_SEL_CRITERIA_SCORE_NSS: Unsigned 8-bit value.
 *	Represents the weightage to be given for the NSS selection
 *	criteria among other parameters.
 *
 * @QCA_ATTR_ROAM_CAND_SEL_CRITERIA_SCORE_CHAN_CONGESTION: Unsigned 8-bit value.
 *	Represents the weightage to be given for the channel congestion
 *	selection criteria among other parameters.
 *
 * @QCA_ATTR_ROAM_CAND_SEL_CRITERIA_SCORE_BEAMFORMING: Unsigned 8-bit value.
 *	Represents the weightage to be given for the beamforming selection
 *	criteria among other parameters.
 *
 * @QCA_ATTR_ROAM_CAND_SEL_CRITERIA_SCORE_OCE_WAN: Unsigned 8-bit value.
 *	Represents the weightage to be given for the OCE selection
 *	criteria among other parameters.
 */
enum qca_vendor_attr_roam_candidate_selection_criteria {
	QCA_ATTR_ROAM_CAND_SEL_CRITERIA_SCORE_RSSI = 1,
	QCA_ATTR_ROAM_CAND_SEL_CRITERIA_RATE = 2,
	QCA_ATTR_ROAM_CAND_SEL_CRITERIA_SCORE_BW = 3,
	QCA_ATTR_ROAM_CAND_SEL_CRITERIA_SCORE_BAND = 4,
	QCA_ATTR_ROAM_CAND_SEL_CRITERIA_SCORE_NSS = 5,
	QCA_ATTR_ROAM_CAND_SEL_CRITERIA_SCORE_CHAN_CONGESTION = 6,
	QCA_ATTR_ROAM_CAND_SEL_CRITERIA_SCORE_BEAMFORMING = 7,
	QCA_ATTR_ROAM_CAND_SEL_CRITERIA_SCORE_OCE_WAN = 8,

	/* keep last */
	QCA_ATTR_ROAM_CAND_SEL_CRITERIA_RATE_AFTER_LAST,
	QCA_ATTR_ROAM_CAND_SEL_CRITERIA_RATE_MAX =
	QCA_ATTR_ROAM_CAND_SEL_CRITERIA_RATE_AFTER_LAST - 1,
};

/**
 * enum qca_vendor_attr_roam_control - Attributes to carry roam configuration
 *	The following attributes are used to set/get/clear the respective
 *	configurations to/from the driver.
 *	For the get, the attribute for the configuration to be queried shall
 *	carry any of its acceptable values to the driver. In return, the driver
 *	shall send the configured values within the same attribute to the user
 *	space.
 *
 * @QCA_ATTR_ROAM_CONTROL_ENABLE: Unsigned 8-bit value.
 *	Signifies to enable/disable roam control in driver.
 *	1-enable, 0-disable
 *	Enable: Mandates the driver to do the further roams using the
 *	configuration parameters set through
 *	QCA_WLAN_VENDOR_ROAMING_SUBCMD_CONTROL_SET.
 *	Disable: Disables the driver/firmware roaming triggered through
 *	QCA_WLAN_VENDOR_ROAMING_SUBCMD_CONTROL_SET. Further roaming is
 *	expected to continue with the default configurations.
 *
 * @QCA_ATTR_ROAM_CONTROL_STATUS: Unsigned 8-bit value.
 *	This is used along with QCA_WLAN_VENDOR_ROAMING_SUBCMD_CONTROL_GET.
 *	Roam control status is obtained through this attribute.
 *
 * @QCA_ATTR_ROAM_CONTROL_CLEAR_ALL: Flag attribute to indicate the
 *	complete config set through QCA_WLAN_VENDOR_ROAMING_SUBCMD_CONTROL_SET
 *	is to be cleared in the driver.
 *	This is used along with QCA_WLAN_VENDOR_ROAMING_SUBCMD_CONTROL_CLEAR
 *	and shall be ignored if used with other sub commands.
 *	If this attribute is specified along with subcmd
 *	QCA_WLAN_VENDOR_ROAMING_SUBCMD_CONTROL_CLEAR, the driver shall ignore
 *	all other attributes, if there are any.
 *	If this attribute is not specified when the subcmd
 *	QCA_WLAN_VENDOR_ROAMING_SUBCMD_CONTROL_CLEAR is sent, the driver shall
 *	clear the data corresponding to the attributes specified.
 *
 * @QCA_ATTR_ROAM_CONTROL_FREQ_LIST_SCHEME: Nested attribute to carry the
 *	list of frequencies and its type, represented by
 *	enum qca_vendor_attr_scan_freq_list_scheme.
 *	Frequency list and its type are mandatory for this attribute to set
 *	the frequencies.
 *	Frequency type is mandatory for this attribute to get the frequencies
 *	and the frequency list is obtained through
 *	QCA_ATTR_ROAM_CONTROL_SCAN_FREQ_LIST.
 *	Frequency list type is mandatory for this attribute to clear the
 *	frequencies.
 *
 * @QCA_ATTR_ROAM_CONTROL_SCAN_PERIOD: Unsigned 32-bit value.
 *	Carries the value of scan period in seconds to set.
 *	The value of scan period is obtained with the same attribute for get.
 *	Clears the scan period in the driver when specified with clear command.
 *	Scan period is the idle time in seconds between each subsequent
 *	channel scans.
 *
 * @QCA_ATTR_ROAM_CONTROL_FULL_SCAN_PERIOD: Unsigned 32-bit value.
 *	Carries the value of full scan period in seconds to set.
 *	The value of full scan period is obtained with the same attribute for
 *	get.
 *	Clears the full scan period in the driver when specified with clear
 *	command. Full scan period is the idle period in seconds between two
 *	successive full channel roam scans.
 *
 * @QCA_ATTR_ROAM_CONTROL_TRIGGERS: Unsigned 32-bit value.
 *	Carries a bitmap of the roam triggers specified in
 *	enum qca_vendor_roam_triggers.
 *	The driver shall enable roaming by enabling corresponding roam triggers
 *	based on the trigger bits sent with this attribute.
 *	If this attribute is not configured, the driver shall proceed with
 *	default behavior.
 *	The bitmap configured is obtained with the same attribute for get.
 *	Clears the bitmap configured in driver when specified with clear
 *	command.
 *
 * @QCA_ATTR_ROAM_CONTROL_SELECTION_CRITERIA: Nested attribute signifying the
 *	weightage in percentage (%) to be given for each selection criteria.
 *	Different roam candidate selection criteria are represented by
 *	enum qca_vendor_attr_roam_candidate_selection_criteria.
 *	The driver shall select the roam candidate based on corresponding
 *	candidate selection scores sent.
 *
 *	An empty nested attribute is used to indicate that no specific
 *	preference score/criteria is configured (i.e., to disable this mechanism
 *	in the set case and to show that the mechanism is disabled in the get
 *	case).
 *
 *	Userspace can send multiple attributes out of this enum to the driver.
 *	Since this attribute represents the weight/percentage of preference for
 *	the respective selection criteria, it is preferred to configure 100%
 *	total weightage. The value in each attribute or cumulative weight of the
 *	values in all the nested attributes should not exceed 100%. The driver
 *	shall reject such configuration.
 *
 *	If the weights configured through this attribute are less than 100%,
 *	the driver shall honor the weights (x%) passed for the corresponding
 *	selection criteria and choose/distribute rest of the weight (100-x)%
 *	for the other selection criteria, based on its internal logic.
 *
 *	The selection criteria configured is obtained with the same
 *	attribute for get.
 *
 *	Clears the selection criteria configured in the driver when specified
 *	with clear command.
 *
 * @QCA_ATTR_ROAM_CONTROL_SCAN_SCHEME: Unsigned 32-bit value.
 *	Represents value of scan frequency scheme from
 *      enum qca_roam_scan_scheme.
 *
 * @QCA_ATTR_ROAM_CONTROL_CONNECTED_RSSI_THRESHOLD: Signed 32-bit value in dBm,
 *	signifying the RSSI threshold of the current connected AP, indicating
 *	the driver to trigger roam only when the current connected AP's RSSI
 *	is lesser than this threshold.
 *
 * @QCA_ATTR_ROAM_CONTROL_CANDIDATE_RSSI_THRESHOLD: Signed 32-bit value in dBm,
 *	signifying the RSSI threshold of the candidate AP, indicating
 *	the driver to trigger roam only to the candidate AP with RSSI
 *	better than this threshold.
 *
 * @QCA_ATTR_ROAM_CONTROL_USER_REASON: Unsigned 32-bit value. Represents the
 *	user triggered reason code to be sent to the AP in response to AP's
 *	request to trigger the roam if the roaming cannot be triggered.
 *	Applies to all the scenarios of AP assisted roaming (e.g., BTM).
 *
 * @QCA_ATTR_ROAM_CONTROL_SCAN_SCHEME_TRIGGERS: Unsigned 32-bit value.
 *	Carries a bitmap of the roam triggers specified in
 *	enum qca_vendor_roam_triggers.
 *	Represents the roam triggers for which the specific scan scheme from
 *	enum qca_roam_scan_scheme has to be applied.
 *	It's an optional attribute. If this attribute is not configured, but
 *	QCA_ATTR_ROAM_CONTROL_SCAN_SCHEME is specified, then the scan scheme
 *	specified through QCA_ATTR_ROAM_CONTROL_SCAN_SCHEME is applicable for
 *	all the roams.
 *	If both QCA_ATTR_ROAM_CONTROL_SCAN_SCHEME and
 *	QCA_ATTR_ROAM_CONTROL_SCAN_SCHEME_TRIGGERS are not specified, the
 *	driver shall proceed with the default behavior.
 */
enum qca_vendor_attr_roam_control {
	QCA_ATTR_ROAM_CONTROL_ENABLE = 1,
	QCA_ATTR_ROAM_CONTROL_STATUS = 2,
	QCA_ATTR_ROAM_CONTROL_CLEAR_ALL = 3,
	QCA_ATTR_ROAM_CONTROL_FREQ_LIST_SCHEME = 4,
	QCA_ATTR_ROAM_CONTROL_SCAN_PERIOD = 5,
	QCA_ATTR_ROAM_CONTROL_FULL_SCAN_PERIOD = 6,
	QCA_ATTR_ROAM_CONTROL_TRIGGERS = 7,
	QCA_ATTR_ROAM_CONTROL_SELECTION_CRITERIA = 8,
	QCA_ATTR_ROAM_CONTROL_SCAN_SCHEME = 9,
	QCA_ATTR_ROAM_CONTROL_CONNECTED_RSSI_THRESHOLD = 10,
	QCA_ATTR_ROAM_CONTROL_CANDIDATE_RSSI_THRESHOLD = 11,
	QCA_ATTR_ROAM_CONTROL_USER_REASON = 12,
	QCA_ATTR_ROAM_CONTROL_SCAN_SCHEME_TRIGGERS = 13,

	/* keep last */
	QCA_ATTR_ROAM_CONTROL_AFTER_LAST,
	QCA_ATTR_ROAM_CONTROL_MAX =
	QCA_ATTR_ROAM_CONTROL_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_roaming_config_params: Attributes for data used by
 * QCA_NL80211_VENDOR_SUBCMD_ROAM sub command.
 *
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_SUBCMD: Unsigned 32-bit value.
 *	Represents the different roam sub commands referred by
 *	enum qca_wlan_vendor_roaming_subcmd.
 *
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_REQ_ID: Unsigned 32-bit value.
 *	Represents the Request ID for the specific set of commands.
 *	This also helps to map specific set of commands to the respective
 *	ID / client. e.g., helps to identify the user entity configuring the
 *	Blacklist BSSID and accordingly clear the respective ones with the
 *	matching ID.
 *
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_WHITE_LIST_SSID_NUM_NETWORKS: Unsigned
 *	32-bit value.Represents the number of whitelist SSIDs configured.
 *
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_WHITE_LIST_SSID_LIST: Nested attribute
 *	to carry the list of Whitelist SSIDs.
 *
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_WHITE_LIST_SSID: SSID (binary attribute,
 *	0..32 octets). Represents the white list SSID. Whitelist SSIDs
 *	represent the list of SSIDs to which the firmware/driver can consider
 *	to roam to.
 *
 * The following PARAM_A_BAND_XX attributes are applied to 5GHz BSSIDs when
 * comparing with a 2.4GHz BSSID. They are not applied when comparing two
 * 5GHz BSSIDs.The following attributes are set through the Roaming SUBCMD -
 * QCA_WLAN_VENDOR_ROAMING_SUBCMD_SET_EXTSCAN_ROAM_PARAMS.
 *
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_A_BAND_BOOST_THRESHOLD: Signed 32-bit
 *	value, RSSI threshold above which 5GHz RSSI is favored.
 *
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_A_BAND_PENALTY_THRESHOLD: Signed 32-bit
 *	value, RSSI threshold below which 5GHz RSSI is penalized.
 *
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_A_BAND_BOOST_FACTOR: Unsigned 32-bit
 *	value, factor by which 5GHz RSSI is boosted.
 *	boost=(RSSI_measured-5GHz_boost_threshold)*5GHz_boost_factor
 *
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_A_BAND_PENALTY_FACTOR: Unsigned 32-bit
 *	value, factor by which 5GHz RSSI is penalized.
 *	penalty=(5GHz_penalty_threshold-RSSI_measured)*5GHz_penalty_factor
 *
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_A_BAND_MAX_BOOST: Unsigned 32-bit
 *	value, maximum boost that can be applied to a 5GHz RSSI.
 *
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_LAZY_ROAM_HISTERESYS: Unsigned 32-bit
 *	value, boost applied to current BSSID to ensure the currently
 *	associated BSSID is favored so as to prevent ping-pong situations.
 *
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_ALERT_ROAM_RSSI_TRIGGER: Signed 32-bit
 *	value, RSSI below which "Alert" roam is enabled.
 *	"Alert" mode roaming - firmware is "urgently" hunting for another BSSID
 *	because the RSSI is low, or because many successive beacons have been
 *	lost or other bad link conditions.
 *
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_LAZY_ROAM_ENABLE: Unsigned 32-bit
 *	value. 1-Enable, 0-Disable. Represents "Lazy" mode, where
 *	firmware is hunting for a better BSSID or white listed SSID even though
 *	the RSSI of the link is good. The parameters enabling the roaming are
 *	configured through the PARAM_A_BAND_XX attrbutes.
 *
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_BSSID_PREFS: Nested attribute,
 *	represents the BSSIDs preferred over others while evaluating them
 *	for the roaming.
 *
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_LAZY_ROAM_NUM_BSSID: Unsigned
 *	32-bit value. Represents the number of preferred BSSIDs set.
 *
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_LAZY_ROAM_BSSID: 6-byte MAC
 *	address representing the BSSID to be preferred.
 *
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_LAZY_ROAM_RSSI_MODIFIER: Signed
 *	32-bit value, representing the modifier to be applied to the RSSI of
 *	the BSSID for the purpose of comparing it with other roam candidate.
 *
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_BSSID_PARAMS: Nested attribute,
 *	represents the BSSIDs to get blacklisted for roaming.
 *
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_BSSID_PARAMS_NUM_BSSID: Unsigned
 *	32-bit value, represents the number of blacklisted BSSIDs.
 *
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_BSSID_PARAMS_BSSID: 6-byte MAC
 *	address representing the Blacklisted BSSID.
 *
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_BSSID_PARAMS_HINT: Flag attribute,
 *	indicates this BSSID blacklist as a hint to the driver. The driver can
 *	select this BSSID in the worst case (when no other BSSIDs are better).
 *
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_CONTROL: Nested attribute to
 *	set/get/clear the roam control config as
 *	defined @enum qca_vendor_attr_roam_control.
 *
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_AFTER_LAST: After last
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_MAX: Max
 */
enum qca_wlan_vendor_attr_roaming_config_params {
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_INVALID = 0,

	QCA_WLAN_VENDOR_ATTR_ROAMING_SUBCMD = 1,
	QCA_WLAN_VENDOR_ATTR_ROAMING_REQ_ID = 2,

	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_WHITE_LIST_SSID_NUM_NETWORKS = 3,
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_WHITE_LIST_SSID_LIST = 4,
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_WHITE_LIST_SSID = 5,

	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_A_BAND_BOOST_THRESHOLD = 6,
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_A_BAND_PENALTY_THRESHOLD = 7,
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_A_BAND_BOOST_FACTOR = 8,
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_A_BAND_PENALTY_FACTOR = 9,
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_A_BAND_MAX_BOOST = 10,
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_LAZY_ROAM_HISTERESYS = 11,
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_ALERT_ROAM_RSSI_TRIGGER = 12,

	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_LAZY_ROAM_ENABLE = 13,

	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_BSSID_PREFS = 14,
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_LAZY_ROAM_NUM_BSSID = 15,
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_LAZY_ROAM_BSSID = 16,
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_LAZY_ROAM_RSSI_MODIFIER = 17,

	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_BSSID_PARAMS = 18,
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_BSSID_PARAMS_NUM_BSSID = 19,
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_BSSID_PARAMS_BSSID = 20,
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_BSSID_PARAMS_HINT = 21,

	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_CONTROL = 22,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_MAX =
		QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_roaming_subcmd: Referred by
 * QCA_WLAN_VENDOR_ATTR_ROAMING_SUBCMD.
 *
 * @QCA_WLAN_VENDOR_ROAMING_SUBCMD_SSID_WHITE_LIST: Sub command to
 *	configure the white list SSIDs. These are configured through
 *	the following attributes.
 *	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_WHITE_LIST_SSID_NUM_NETWORKS,
 *	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_WHITE_LIST_SSID_LIST,
 *	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_WHITE_LIST_SSID
 *
 * @QCA_WLAN_VENDOR_ROAMING_SUBCMD_SET_EXTSCAN_ROAM_PARAMS: Sub command to
 *	configure the Roam params. These parameters are evaluated on the extscan
 *	results. Refers the attributes PARAM_A_BAND_XX above to configure the
 *	params.
 *
 * @QCA_WLAN_VENDOR_ROAMING_SUBCMD_SET_LAZY_ROAM: Sets the Lazy roam. Uses
 *	the attribute QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_LAZY_ROAM_ENABLE
 *	to enable/disable Lazy roam.
 *
 * @QCA_WLAN_VENDOR_ROAMING_SUBCMD_SET_BSSID_PREFS: Sets the BSSID
 *	preference. Contains the attribute
 *	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_BSSID_PREFS to set the BSSID
 *	preference.
 *
 * @QCA_WLAN_VENDOR_ROAMING_SUBCMD_SET_BSSID_PARAMS: set bssid params
 *
 * @QCA_WLAN_VENDOR_ROAMING_SUBCMD_SET_BLACKLIST_BSSID: Sets the Blacklist
 *	BSSIDs. Refers QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_BSSID_PARAMS to
 *	set the same.
 *
 * @QCA_WLAN_VENDOR_ROAMING_SUBCMD_CONTROL_SET: Command to set the
 *	roam control config to the driver with the attribute
 *	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_CONTROL.
 *
 * @QCA_WLAN_VENDOR_ROAMING_SUBCMD_CONTROL_GET: Command to obtain the
 *	roam control config from driver with the attribute
 *	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_CONTROL.
 *	For the get, the attribute for the configuration to be queried shall
 *	carry any of its acceptable value to the driver. In return, the driver
 *	shall send the configured values within the same attribute to the user
 *	space.
 *
 * @QCA_WLAN_VENDOR_ROAMING_SUBCMD_CONTROL_CLEAR: Command to clear the
 *	roam control config in the driver with the attribute
 *	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_CONTROL.
 *	The driver shall continue with its default roaming behavior when data
 *	corresponding to an attribute is cleared.
 */
enum qca_wlan_vendor_roaming_subcmd {
	QCA_WLAN_VENDOR_ROAMING_SUBCMD_SSID_WHITE_LIST = 1,
	QCA_WLAN_VENDOR_ROAMING_SUBCMD_SET_EXTSCAN_ROAM_PARAMS = 2,
	QCA_WLAN_VENDOR_ROAMING_SUBCMD_SET_LAZY_ROAM = 3,
	QCA_WLAN_VENDOR_ROAMING_SUBCMD_SET_BSSID_PREFS = 4,
	QCA_WLAN_VENDOR_ROAMING_SUBCMD_SET_BSSID_PARAMS = 5,
	QCA_WLAN_VENDOR_ROAMING_SUBCMD_SET_BLACKLIST_BSSID = 6,
	QCA_WLAN_VENDOR_ROAMING_SUBCMD_CONTROL_SET = 7,
	QCA_WLAN_VENDOR_ROAMING_SUBCMD_CONTROL_GET = 8,
	QCA_WLAN_VENDOR_ROAMING_SUBCMD_CONTROL_CLEAR = 9,
};

/**
 * enum qca_wlan_vendor_attr_get_wifi_info - wifi driver information
 *
 * @QCA_WLAN_VENDOR_ATTR_WIFI_INFO_GET_INVALID: Invalid initial value
 * @QCA_WLAN_VENDOR_ATTR_WIFI_INFO_DRIVER_VERSION: get host driver version
 * @QCA_WLAN_VENDOR_ATTR_WIFI_INFO_FIRMWARE_VERSION: ger firmware version
 * @QCA_WLAN_VENDOR_ATTR_WIFI_INFO_RADIO_INDEX - get radio index
 * @QCA_WLAN_VENDOR_ATTR_WIFI_INFO_GET_AFTER_LAST: after last
 * @QCA_WLAN_VENDOR_ATTR_WIFI_INFO_GET_MAX: subcmd max
 */
enum qca_wlan_vendor_attr_get_wifi_info {
	QCA_WLAN_VENDOR_ATTR_WIFI_INFO_GET_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_WIFI_INFO_DRIVER_VERSION     = 1,
	QCA_WLAN_VENDOR_ATTR_WIFI_INFO_FIRMWARE_VERSION   = 2,
	QCA_WLAN_VENDOR_ATTR_WIFI_INFO_RADIO_INDEX        = 3,

	/* KEEP LAST */
	QCA_WLAN_VENDOR_ATTR_WIFI_INFO_GET_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_WIFI_INFO_GET_MAX  =
		QCA_WLAN_VENDOR_ATTR_WIFI_INFO_GET_AFTER_LAST - 1,
};

enum qca_wlan_vendor_attr_logger_results {
	QCA_WLAN_VENDOR_ATTR_LOGGER_RESULTS_INVALID = 0,

	/*
	 * Unsigned 32-bit value; must match the request Id supplied by
	 * Wi-Fi HAL in the corresponding subcmd NL msg.
	 */
	QCA_WLAN_VENDOR_ATTR_LOGGER_RESULTS_REQUEST_ID = 1,

	/*
	 * Unsigned 32-bit value; used to indicate the size of memory
	 * dump to be allocated.
	 */
	QCA_WLAN_VENDOR_ATTR_LOGGER_RESULTS_MEMDUMP_SIZE = 2,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_LOGGER_RESULTS_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_LOGGER_RESULTS_MAX =
	QCA_WLAN_VENDOR_ATTR_LOGGER_RESULTS_AFTER_LAST - 1,
};

/**
 * qca_wlan_vendor_channel_prop_flags: This represent the flags for a channel.
 * This is used by QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FLAGS.
 */
enum qca_wlan_vendor_channel_prop_flags {
	/* Bits 0, 1, 2, and 3 are reserved */

	/* Turbo channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_TURBO         = 1 << 4,
	/* CCK channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_CCK           = 1 << 5,
	/* OFDM channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_OFDM          = 1 << 6,
	/* 2.4 GHz spectrum channel. */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_2GHZ          = 1 << 7,
	/* 5 GHz spectrum channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_5GHZ          = 1 << 8,
	/* Only passive scan allowed */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_PASSIVE       = 1 << 9,
	/* Dynamic CCK-OFDM channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_DYN           = 1 << 10,
	/* GFSK channel (FHSS PHY) */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_GFSK          = 1 << 11,
	/* Radar found on channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_RADAR         = 1 << 12,
	/* 11a static turbo channel only */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_STURBO        = 1 << 13,
	/* Half rate channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HALF          = 1 << 14,
	/* Quarter rate channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_QUARTER       = 1 << 15,
	/* HT 20 channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HT20          = 1 << 16,
	/* HT 40 with extension channel above */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HT40PLUS      = 1 << 17,
	/* HT 40 with extension channel below */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HT40MINUS     = 1 << 18,
	/* HT 40 intolerant */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HT40INTOL     = 1 << 19,
	/* VHT 20 channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_VHT20         = 1 << 20,
	/* VHT 40 with extension channel above */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_VHT40PLUS     = 1 << 21,
	/* VHT 40 with extension channel below */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_VHT40MINUS    = 1 << 22,
	/* VHT 80 channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_VHT80         = 1 << 23,
	/* HT 40 intolerant mark bit for ACS use */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HT40INTOLMARK = 1 << 24,
	/* Channel temporarily blocked due to noise */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_BLOCKED       = 1 << 25,
	/* VHT 160 channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_VHT160        = 1 << 26,
	/* VHT 80+80 channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_VHT80_80      = 1 << 27,
	/* HE 20 channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HE20          = 1 << 28,
	/* HE 40 with extension channel above */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HE40PLUS      = 1 << 29,
	/* HE 40 with extension channel below */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HE40MINUS     = 1 << 30,
	/* HE 40 intolerant */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HE40INTOL     = 1 << 31,
};

/**
 * qca_wlan_vendor_channel_prop_flags_2: This represents the flags for a
 * channel, and is a continuation of qca_wlan_vendor_channel_prop_flags. This is
 * used by QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FLAGS_2.
 */
enum qca_wlan_vendor_channel_prop_flags_2 {
	/* HE 40 intolerant mark bit for ACS use */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HE40INTOLMARK = 1 << 0,
	/* HE 80 channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HE80          = 1 << 1,
	/* HE 160 channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HE160         = 1 << 2,
	/* HE 80+80 channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HE80_80       = 1 << 3,
};

/**
 * qca_wlan_vendor_channel_prop_flags_ext: This represent the extended flags for
 * each channel. This is used by
 * QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FLAG_EXT.
 */
enum qca_wlan_vendor_channel_prop_flags_ext {
	/* Radar found on channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_EXT_RADAR_FOUND     = 1 << 0,
	/* DFS required on channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_EXT_DFS             = 1 << 1,
	/* DFS required on channel for 2nd band of 80+80 */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_EXT_DFS_CFREQ2      = 1 << 2,
	/* If channel has been checked for DFS */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_EXT_DFS_CLEAR       = 1 << 3,
	/* Excluded in 802.11d */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_EXT_11D_EXCLUDED    = 1 << 4,
	/* Channel Switch Announcement received on this channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_EXT_CSA_RECEIVED    = 1 << 5,
	/* Ad-hoc is not allowed */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_EXT_DISALLOW_ADHOC  = 1 << 6,
	/* Station only channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_EXT_DISALLOW_HOSTAP = 1 << 7,
	/* DFS radar history for slave device (STA mode) */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_EXT_HISTORY_RADAR   = 1 << 8,
	/* DFS CAC valid for slave device (STA mode) */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_EXT_CAC_VALID       = 1 << 9,
};

/**
 * qca_wlan_vendor_attr_nud_stats_set: Attributes to vendor subcmd
 * QCA_NL80211_VENDOR_SUBCMD_NUD_STATS_SET. This carries the requisite
 * information to start/stop the NUD statistics collection.
 */
enum qca_attr_nud_stats_set {
	QCA_ATTR_NUD_STATS_SET_INVALID = 0,

	/*
	 * Flag to start/stop the NUD statistics collection.
	 * Start - If included, Stop - If not included
	 */
	QCA_ATTR_NUD_STATS_SET_START = 1,
	/* IPv4 address of the default gateway (in network byte order) */
	QCA_ATTR_NUD_STATS_GW_IPV4 = 2,
	/*
	 * Represents the data packet type to be monitored.
	 * Host driver tracks the stats corresponding to each data frame
	 * represented by these flags.
	 * These data packets are represented by
	 * enum qca_wlan_vendor_nud_stats_set_data_pkt_info.
	 */
	QCA_ATTR_NUD_STATS_SET_DATA_PKT_INFO = 3,
	/* keep last */
	QCA_ATTR_NUD_STATS_SET_LAST,
	QCA_ATTR_NUD_STATS_SET_MAX =
		QCA_ATTR_NUD_STATS_SET_LAST - 1,
};

/**
 * enum qca_attr_connectivity_check_stats_set - attribute to vendor subcmd
 *	QCA_NL80211_VENDOR_SUBCMD_NUD_STATS_SET. This carry the requisite
 *	information to start / stop the NUD stats collection.
 * @QCA_ATTR_CONNECTIVITY_CHECK_STATS_STATS_PKT_INFO_TYPE: set pkt info stats
 *	Bitmap to Flag to Start / Stop the NUD stats collection
 *	Start - If included , Stop - If not included
 * @QCA_ATTR_CONNECTIVITY_CHECK_STATS_DNS_DOMAIN_NAME: set gatway ipv4 address
 *	IPv4 address of Default Gateway (in network byte order)
 *	QCA_NL80211_VENDOR_SUBCMD_NUD_STATS_SET. This carry the requisite
 *	information to start / stop the NUD stats collection.
 * @QCA_ATTR_CONNECTIVITY_CHECK_STATS_SRC_PORT: set nud debug stats
 *	Flag to Start / Stop the NUD stats collection
 *	Start - If included , Stop - If not included
 * @QCA_ATTR_CONNECTIVITY_CHECK_STATS_DEST_PORT: set gatway ipv4 address
 *	IPv4 address of Default Gateway (in network byte order)
 *	QCA_NL80211_VENDOR_SUBCMD_NUD_STATS_SET. This carry the requisite
 *	information to start / stop the NUD stats collection.
 * @QCA_ATTR_CONNECTIVITY_CHECK_STATS_DEST_IPV4: set nud debug stats
 *	Flag to Start / Stop the NUD stats collection
 *	Start - If included , Stop - If not included
 * @QCA_ATTR_CONNECTIVITY_CHECK_STATS_DEST_IPV6: set gatway ipv4 address
 *	IPv4 address of Default Gateway (in network byte order)
 */
enum qca_attr_connectivity_check_stats_set {
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_SET_INVALID = 0,
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_STATS_PKT_INFO_TYPE = 1,
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_DNS_DOMAIN_NAME = 2,
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_SRC_PORT = 3,
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_DEST_PORT = 4,
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_DEST_IPV4 = 5,
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_DEST_IPV6 = 6,
	/* keep last */
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_SET_LAST,
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_SET_MAX =
		QCA_ATTR_CONNECTIVITY_CHECK_STATS_SET_LAST - 1,
};

/**
 * qca_wlan_vendor_nud_stats_data_pkt_flags: Flag representing the various
 * data types for which the stats have to get collected.
 */
enum qca_wlan_vendor_connectivity_check_pkt_flags {
	QCA_WLAN_VENDOR_CONNECTIVITY_CHECK_SET_ARP = 1 << 0,
	QCA_WLAN_VENDOR_CONNECTIVITY_CHECK_SET_DNS = 1 << 1,
	QCA_WLAN_VENDOR_CONNECTIVITY_CHECK_SET_TCP_HANDSHAKE = 1 << 2,
	QCA_WLAN_VENDOR_CONNECTIVITY_CHECK_SET_ICMPV4 = 1 << 3,
	QCA_WLAN_VENDOR_CONNECTIVITY_CHECK_SET_ICMPV6 = 1 << 4,
	/* Used by QCA_ATTR_NUD_STATS_PKT_TYPE only in nud stats get
	 * to represent the stats of respective data type.
	 */
	QCA_WLAN_VENDOR_CONNECTIVITY_CHECK_SET_TCP_SYN = 1 << 5,
	QCA_WLAN_VENDOR_CONNECTIVITY_CHECK_SET_TCP_SYN_ACK = 1 << 6,
	QCA_WLAN_VENDOR_CONNECTIVITY_CHECK_SET_TCP_ACK = 1 << 7,
};

enum qca_attr_connectivity_check_stats {
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_INVALID = 0,
	/* Data packet type for which the stats are collected.
	 * Represented by enum qca_wlan_vendor_nud_stats_data_pkt_flags
	 */
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_PKT_TYPE = 1,
	/* ID corresponding to the DNS frame for which the respective DNS stats
	 * are monitored (u32).
	 */
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_PKT_DNS_DOMAIN_NAME = 2,
	/* source / destination port on which the respective proto stats are
	 * collected (u32).
	 */
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_PKT_SRC_PORT = 3,
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_PKT_DEST_PORT = 4,
	/* IPv4/IPv6 address for which the destined data packets are
	 * monitored. (in network byte order)
	 */
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_PKT_DEST_IPV4 = 5,
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_PKT_DEST_IPV6 = 6,
	/* Data packet Request count received from netdev */
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_PKT_REQ_COUNT_FROM_NETDEV = 7,
	/* Data packet Request count sent to lower MAC from upper MAC */
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_PKT_REQ_COUNT_TO_LOWER_MAC = 8,
	/* Data packet Request count received by lower MAC from upper MAC */
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_PKT_REQ_RX_COUNT_BY_LOWER_MAC = 9,
	/* Data packet Request count successfully transmitted by the device */
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_PKT_REQ_COUNT_TX_SUCCESS = 10,
	/* Data packet Response count received by lower MAC */
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_PKT_RSP_RX_COUNT_BY_LOWER_MAC = 11,
	/* Data packet Response count received by upper MAC */
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_PKT_RSP_RX_COUNT_BY_UPPER_MAC = 12,
	/* Data packet Response count delivered to netdev */
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_PKT_RSP_COUNT_TO_NETDEV = 13,
	/* Data Packet Response count that are dropped out of order */
	QCA_ATTR_CONNECTIVITY_CHECK_STATS_PKT_RSP_COUNT_OUT_OF_ORDER_DROP = 14,

	/* keep last */
	QCA_ATTR_CONNECTIVITY_CHECK_DATA_STATS_LAST,
	QCA_ATTR_CONNECTIVITY_CHECK_DATA_STATS_MAX =
		QCA_ATTR_CONNECTIVITY_CHECK_DATA_STATS_LAST - 1,
};

/**
 * qca_attr_nud_stats_get: Attributes to vendor subcmd
 * QCA_NL80211_VENDOR_SUBCMD_NUD_STATS_GET. This carries the requisite
 * NUD statistics collected when queried.
 */
enum qca_attr_nud_stats_get {
	QCA_ATTR_NUD_STATS_GET_INVALID = 0,
	/* ARP Request count from netdev */
	QCA_ATTR_NUD_STATS_ARP_REQ_COUNT_FROM_NETDEV = 1,
	/* ARP Request count sent to lower MAC from upper MAC */
	QCA_ATTR_NUD_STATS_ARP_REQ_COUNT_TO_LOWER_MAC = 2,
	/* ARP Request count received by lower MAC from upper MAC */
	QCA_ATTR_NUD_STATS_ARP_REQ_RX_COUNT_BY_LOWER_MAC = 3,
	/* ARP Request count successfully transmitted by the device */
	QCA_ATTR_NUD_STATS_ARP_REQ_COUNT_TX_SUCCESS = 4,
	/* ARP Response count received by lower MAC */
	QCA_ATTR_NUD_STATS_ARP_RSP_RX_COUNT_BY_LOWER_MAC = 5,
	/* ARP Response count received by upper MAC */
	QCA_ATTR_NUD_STATS_ARP_RSP_RX_COUNT_BY_UPPER_MAC = 6,
	/* ARP Response count delivered to netdev */
	QCA_ATTR_NUD_STATS_ARP_RSP_COUNT_TO_NETDEV = 7,
	/* ARP Response count delivered to netdev */
	QCA_ATTR_NUD_STATS_ARP_RSP_COUNT_OUT_OF_ORDER_DROP = 8,
	/*
	 * Flag indicating if the station's link to the AP is active.
	 * Active Link - If included, Inactive link - If not included
	 */
	QCA_ATTR_NUD_STATS_AP_LINK_ACTIVE = 9,
	/*
	 * Flag indicating if there is any duplicate address detected (DAD).
	 * Yes - If detected, No - If not detected.
	 */
	QCA_ATTR_NUD_STATS_IS_DAD = 10,
	/*
	 * List of Data types for which the stats are requested.
	 * This list does not carry ARP stats as they are done by the
	 * above attributes. Represented by enum qca_attr_nud_data_stats.
	 */
	QCA_ATTR_NUD_STATS_DATA_PKT_STATS = 11,
	/* keep last */
	QCA_ATTR_NUD_STATS_GET_LAST,
	QCA_ATTR_NUD_STATS_GET_MAX =
		QCA_ATTR_NUD_STATS_GET_LAST - 1,
};

enum qca_wlan_btm_candidate_status {
	QCA_STATUS_ACCEPT = 0,
	QCA_STATUS_REJECT_EXCESSIVE_FRAME_LOSS_EXPECTED = 1,
	QCA_STATUS_REJECT_EXCESSIVE_DELAY_EXPECTED = 2,
	QCA_STATUS_REJECT_INSUFFICIENT_QOS_CAPACITY = 3,
	QCA_STATUS_REJECT_LOW_RSSI = 4,
	QCA_STATUS_REJECT_HIGH_INTERFERENCE = 5,
	QCA_STATUS_REJECT_UNKNOWN = 6,
};

enum qca_wlan_vendor_attr_btm_candidate_info {
	QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO_INVALID = 0,

	/* 6-byte MAC address representing the BSSID of transition candidate */
	QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO_BSSID = 1,
	/*
	 * Unsigned 32-bit value from enum qca_wlan_btm_candidate_status
	 * returned by the driver. It says whether the BSSID provided in
	 * QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO_BSSID is acceptable by
	 * the driver, if not it specifies the reason for rejection.
	 * Note that the user-space can overwrite the transition reject reason
	 * codes provided by driver based on more information.
	 */
	QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO_STATUS = 2,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO_MAX =
	QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO_AFTER_LAST - 1,
};

enum qca_attr_trace_level {
	QCA_ATTR_TRACE_LEVEL_INVALID = 0,
	/*
	 * Nested array of the following attributes:
	 * QCA_ATTR_TRACE_LEVEL_MODULE,
	 * QCA_ATTR_TRACE_LEVEL_MASK.
	 */
	QCA_ATTR_TRACE_LEVEL_PARAM = 1,
	/*
	 * Specific QCA host driver module. Please refer to the QCA host
	 * driver implementation to get the specific module ID.
	 */
	QCA_ATTR_TRACE_LEVEL_MODULE = 2,
	/* Different trace level masks represented in the QCA host driver. */
	QCA_ATTR_TRACE_LEVEL_MASK = 3,

	/* keep last */
	QCA_ATTR_TRACE_LEVEL_AFTER_LAST,
	QCA_ATTR_TRACE_LEVEL_MAX =
		QCA_ATTR_TRACE_LEVEL_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_get_logger_features - value for logger
 *                                                 supported features
 * @QCA_WLAN_VENDOR_ATTR_LOGGER_INVALID - Invalid
 * @QCA_WLAN_VENDOR_ATTR_LOGGER_SUPPORTED - Indicate the supported features
 * @QCA_WLAN_VENDOR_ATTR_LOGGER_AFTER_LAST - To keep track of the last enum
 * @QCA_WLAN_VENDOR_ATTR_LOGGER_MAX - max value possible for this type
 *
 * enum values are used for NL attributes for data used by
 * QCA_NL80211_VENDOR_SUBCMD_GET_LOGGER_FEATURE_SET sub command.
 */
enum qca_wlan_vendor_attr_get_logger_features {
	QCA_WLAN_VENDOR_ATTR_LOGGER_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_LOGGER_SUPPORTED = 1,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_LOGGER_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_LOGGER_MAX =
		QCA_WLAN_VENDOR_ATTR_LOGGER_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_link_properties - link properties
 *
 * @QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_INVALID: Invalid initial value
 * @QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_NSS: Unsigned 8-bit value to
 *	specify the number of spatial streams negotiated
 * @QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_RATE_FLAGS: Unsigned 8-bit value
 *	to specify negotiated rate flags i.e. ht, vht and channel width
 * @QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_FREQ: Unsigned 32bit value to
 *	specify the operating frequency
 * @QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_MAC_ADDR: MAC Address of the peer
 * (STA / AP ) for the connected link.
 * @QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_STA_FLAGS: Attribute containing a
 * &struct nl80211_sta_flag_update for the respective connected link. MAC
 * address of the peer represented by
 * QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_MAC_ADDR.
 * @QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_AFTER_LAST: after last
 * @QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_MAX: max value
 */
enum qca_wlan_vendor_attr_link_properties {
	QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_NSS = 1,
	QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_RATE_FLAGS = 2,
	QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_FREQ = 3,
	QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_MAC_ADDR  = 4,
	QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_STA_FLAGS  = 5,

	/* KEEP LAST */
	QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_MAX =
		QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_nd_offload - vendor NS offload support
 *
 * @QCA_WLAN_VENDOR_ATTR_ND_OFFLOAD_INVALID - Invalid
 * @QCA_WLAN_VENDOR_ATTR_ND_OFFLOAD_FLAG - Flag to set NS offload
 * @QCA_WLAN_VENDOR_ATTR_ND_OFFLOAD_AFTER_LAST - To keep track of the last enum
 * @QCA_WLAN_VENDOR_ATTR_ND_OFFLOAD_MAX - max value possible for this type
 *
 * enum values are used for NL attributes for data used by
 * QCA_NL80211_VENDOR_SUBCMD_ND_OFFLOAD sub command.
 */
enum qca_wlan_vendor_attr_nd_offload {
	QCA_WLAN_VENDOR_ATTR_ND_OFFLOAD_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_ND_OFFLOAD_FLAG,

	/* Keep last */
	QCA_WLAN_VENDOR_ATTR_ND_OFFLOAD_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_ND_OFFLOAD_MAX =
		QCA_WLAN_VENDOR_ATTR_ND_OFFLOAD_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_features - Vendor device/driver feature flags
 *
 * @QCA_WLAN_VENDOR_FEATURE_KEY_MGMT_OFFLOAD: Device supports key
 *	management offload, a mechanism where the station's firmware
 *	does the exchange with the AP to establish the temporal keys
 *	after roaming, rather than having the user space wpa_supplicant do it.
 * @QCA_WLAN_VENDOR_FEATURE_SUPPORT_HW_MODE_ANY: Device supports automatic
 *	band selection based on channel selection results.
 * @QCA_WLAN_VENDOR_FEATURE_OFFCHANNEL_SIMULTANEOUS: Device supports
 *	simultaneous off-channel operations.
 * @QCA_WLAN_VENDOR_FEATURE_P2P_LISTEN_OFFLOAD: Device supports P2P
 *	Listen offload; a mechanism where the station's firmware takes care of
 *	responding to incoming Probe Request frames received from other P2P
 *	Devices whilst in Listen state, rather than having the user space
 *	wpa_supplicant do it. Information from received P2P requests are
 *	forwarded from firmware to host whenever the host processor wakes up.
 * @QCA_WLAN_VENDOR_FEATURE_OCE_STA: Device supports all OCE non-AP STA
 *	specific features.
 * @QCA_WLAN_VENDOR_FEATURE_OCE_AP: Device supports all OCE AP specific
 *	features.
 * @QCA_WLAN_VENDOR_FEATURE_OCE_STA_CFON: Device supports OCE STA-CFON
 *	specific features only. If a Device sets this bit but not the
 *	%QCA_WLAN_VENDOR_FEATURE_OCE_AP, the userspace shall assume that
 *	this Device may not support all OCE AP functionalities but can support
 *	only OCE STA-CFON functionalities.
 * @QCA_WLAN_VENDOR_FEATURE_SELF_MANAGED_REGULATORY: Device supports self
 *	managed regulatory.
 * @QCA_WLAN_VENDOR_FEATURE_TWT: Device supports TWT (Target Wake Time).
 * @QCA_WLAN_VENDOR_FEATURE_11AX: Device supports 802.11ax (HE)
 * @QCA_WLAN_VENDOR_FEATURE_6GHZ_SUPPORT: Device supports 6 GHz band operation
 * @QCA_WLAN_VENDOR_FEATURE_THERMAL_CONFIG: Device is capable of receiving
 *	and applying thermal configuration through
 *	%QCA_WLAN_VENDOR_ATTR_THERMAL_LEVEL and
 *	%QCA_WLAN_VENDOR_ATTR_THERMAL_COMPLETION_WINDOW attributes from
 *	userspace.
 * @QCA_WLAN_VENDOR_FEATURE_ADAPTIVE_11R: Device supports Adaptive 11r.
 *	With Adaptive 11r feature, access points advertise the vendor
 *	specific IEs and MDE but do not include FT AKM in the RSNE.
 *	The Adaptive 11r supported stations are expected to identify
 *	such vendor specific IEs and connect to the AP in FT mode though
 *	the profile is configured in non-FT mode.
 *	The driver-based SME cases also need to have this support for
 *	Adaptive 11r to handle the connection and roaming scenarios.
 *	This flag indicates the support for the same to the user space.
 * @NUM_QCA_WLAN_VENDOR_FEATURES: Number of assigned feature bits
 */
enum qca_wlan_vendor_features {
	QCA_WLAN_VENDOR_FEATURE_KEY_MGMT_OFFLOAD = 0,
	QCA_WLAN_VENDOR_FEATURE_SUPPORT_HW_MODE_ANY = 1,
	QCA_WLAN_VENDOR_FEATURE_OFFCHANNEL_SIMULTANEOUS = 2,
	QCA_WLAN_VENDOR_FEATURE_P2P_LISTEN_OFFLOAD = 3,
	QCA_WLAN_VENDOR_FEATURE_OCE_STA = 4,
	QCA_WLAN_VENDOR_FEATURE_OCE_AP = 5,
	QCA_WLAN_VENDOR_FEATURE_OCE_STA_CFON = 6,
	QCA_WLAN_VENDOR_FEATURE_SELF_MANAGED_REGULATORY = 7,
	QCA_WLAN_VENDOR_FEATURE_TWT = 8,
	QCA_WLAN_VENDOR_FEATURE_11AX = 9,
	QCA_WLAN_VENDOR_FEATURE_6GHZ_SUPPORT = 10,
	QCA_WLAN_VENDOR_FEATURE_THERMAL_CONFIG = 11,
	QCA_WLAN_VENDOR_FEATURE_ADAPTIVE_11R = 12,

	NUM_QCA_WLAN_VENDOR_FEATURES /* keep last */
};

/**
 * enum qca_wlan_vendor_attr_sap_conditional_chan_switch - Parameters for SAP
 *     conditional channel switch
 * @QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_INVALID: Invalid initial
 *     value
 * @QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_FREQ_LIST: Priority based
 * frequency list (an array of u32 values in host byte order)
 * @QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_STATUS: Status of the
 *     conditional switch (u32)- 0: Success, Non-zero: Failure
 * @QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_AFTER_LAST: After last
 * @QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_MAX: Subcommand max
 */
enum qca_wlan_vendor_attr_sap_conditional_chan_switch {
	QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_FREQ_LIST = 1,
	QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_STATUS = 2,

	/* Keep Last */
	QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_MAX =
	QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_AFTER_LAST - 1,
};

/**
 * enum wifi_logger_supported_features - values for supported logger features
 * @WIFI_LOGGER_MEMORY_DUMP_SUPPORTED: Memory dump of FW
 * @WIFI_LOGGER_PER_PACKET_TX_RX_STATUS_SUPPORTED: Per packet statistics
 * @WIFI_LOGGER_CONNECT_EVENT_SUPPORTED: Logging of Connectivity events
 * @WIFI_LOGGER_POWER_EVENT_SUPPORTED: Power of driver
 * @WIFI_LOGGER_WAKE_LOCK_SUPPORTE: Wakelock of driver
 * @WIFI_LOGGER_WATCHDOG_TIMER_SUPPORTED: monitor FW health
 * @WIFI_LOGGER_DRIVER_DUMP_SUPPORTED: support driver dump
 * @WIFI_LOGGER_PACKET_FATE_SUPPORTED: tracks connection packets fate
 */
enum wifi_logger_supported_features {
	WIFI_LOGGER_MEMORY_DUMP_SUPPORTED = (1 << (0)),
	WIFI_LOGGER_PER_PACKET_TX_RX_STATUS_SUPPORTED = (1 << (1)),
	WIFI_LOGGER_CONNECT_EVENT_SUPPORTED = (1 << (2)),
	WIFI_LOGGER_POWER_EVENT_SUPPORTED = (1 << (3)),
	WIFI_LOGGER_WAKE_LOCK_SUPPORTED = (1 << (4)),
	WIFI_LOGGER_VERBOSE_SUPPORTED = (1 << (5)),
	WIFI_LOGGER_WATCHDOG_TIMER_SUPPORTED = (1 << (6)),
	WIFI_LOGGER_DRIVER_DUMP_SUPPORTED = (1 << (7)),
	WIFI_LOGGER_PACKET_FATE_SUPPORTED = (1 << (8))
};

/**
 * enum qca_wlan_vendor_attr_acs_offload - Defines attributes to be used with
 * vendor command/event QCA_NL80211_VENDOR_SUBCMD_DO_ACS.
 *
 * @QCA_WLAN_VENDOR_ATTR_ACS_PRIMARY_CHANNEL: Required (u8).
 * Used with event to notify the primary channel number selected in ACS
 * operation.
 * Note: If both the driver and user-space application supports the 6 GHz band,
 * QCA_WLAN_VENDOR_ATTR_ACS_PRIMARY_CHANNEL is deprecated; use
 * QCA_WLAN_VENDOR_ATTR_ACS_PRIMARY_FREQUENCY instead.
 * To maintain backward compatibility, QCA_WLAN_VENDOR_ATTR_ACS_PRIMARY_CHANNEL
 * is still used if either of the driver or user space application doesn't
 * support the 6 GHz band.
 *
 * @QCA_WLAN_VENDOR_ATTR_ACS_SECONDARY_CHANNEL: Required (u8).
 * Used with event to notify the secondary channel number selected in ACS
 * operation.
 * Note: If both the driver and user-space application supports the 6 GHz band,
 * QCA_WLAN_VENDOR_ATTR_ACS_SECONDARY_CHANNEL is deprecated; use
 * QCA_WLAN_VENDOR_ATTR_ACS_SECONDARY_FREQUENCY instead.
 * To maintain backward compatibility,
 * QCA_WLAN_VENDOR_ATTR_ACS_SECONDARY_CHANNEL is still used if either of
 * the driver or user space application doesn't support 6 GHz band.
 *
 * @QCA_WLAN_VENDOR_ATTR_ACS_HW_MODE: Required (u8).
 * (a) Used with command to configure hw_mode from
 * enum qca_wlan_vendor_acs_hw_mode for ACS operation.
 * (b) Also used with event to notify the hw_mode of selected primary channel
 * in ACS operation.
 *
 * @QCA_WLAN_VENDOR_ATTR_ACS_HT_ENABLED: Flag attribute.
 * Used with command to configure ACS operation for HT mode.
 * Disable (flag attribute not present) - HT disabled and
 * Enable (flag attribute present) - HT enabled.
 *
 * @QCA_WLAN_VENDOR_ATTR_ACS_HT40_ENABLED: Flag attribute.
 * Used with command to configure ACS operation for HT40 mode.
 * Disable (flag attribute not present) - HT40 disabled and
 * Enable (flag attribute present) - HT40 enabled.
 *
 * @QCA_WLAN_VENDOR_ATTR_ACS_VHT_ENABLED: Flag attribute.
 * Used with command to configure ACS operation for VHT mode.
 * Disable (flag attribute not present) - VHT disabled and
 * Enable (flag attribute present) - VHT enabled.
 *
 * @QCA_WLAN_VENDOR_ATTR_ACS_CHWIDTH: Optional (u16) with command and
 * mandatory with event.
 * If specified in command path, ACS operation is configured with the given
 * channel width (in MHz).
 * In event path, specifies the channel width of the primary channel selected.
 *
 * @QCA_WLAN_VENDOR_ATTR_ACS_CH_LIST: Required and type is NLA_UNSPEC.
 * Used with command to configure channel list using an array of
 * channel numbers (u8).
 * Note: If both the driver and user-space application supports the 6 GHz band,
 * the driver mandates use of QCA_WLAN_VENDOR_ATTR_ACS_FREQ_LIST whereas
 * QCA_WLAN_VENDOR_ATTR_ACS_CH_LIST is optional.
 *
 * @QCA_WLAN_VENDOR_ATTR_ACS_VHT_SEG0_CENTER_CHANNEL: Required (u8).
 * Used with event to notify the VHT segment 0 center channel number selected in
 * ACS operation.
 * Note: If both the driver and user-space application supports the 6 GHz band,
 * QCA_WLAN_VENDOR_ATTR_ACS_VHT_SEG0_CENTER_CHANNEL is deprecated; use
 * QCA_WLAN_VENDOR_ATTR_ACS_VHT_SEG0_CENTER_FREQUENCY instead.
 * To maintain backward compatibility,
 * QCA_WLAN_VENDOR_ATTR_ACS_VHT_SEG0_CENTER_CHANNEL is still used if either of
 * the driver or user space application doesn't support the 6 GHz band.
 *
 * @QCA_WLAN_VENDOR_ATTR_ACS_VHT_SEG1_CENTER_CHANNEL: Required (u8).
 * Used with event to notify the VHT segment 1 center channel number selected in
 * ACS operation.
 * Note: If both the driver and user-space application supports the 6 GHz band,
 * QCA_WLAN_VENDOR_ATTR_ACS_VHT_SEG1_CENTER_CHANNEL is deprecated; use
 * QCA_WLAN_VENDOR_ATTR_ACS_VHT_SEG1_CENTER_FREQUENCY instead.
 * To maintain backward compatibility,
 * QCA_WLAN_VENDOR_ATTR_ACS_VHT_SEG1_CENTER_CHANNEL is still used if either of
 * the driver or user space application doesn't support the 6 GHz band.
 *
 * @QCA_WLAN_VENDOR_ATTR_ACS_FREQ_LIST: Required and type is NLA_UNSPEC.
 * Used with command to configure the channel list using an array of channel
 * center frequencies in MHz (u32).
 * Note: If both the driver and user-space application supports the 6 GHz band,
 * the driver first parses the frequency list and if it fails to get a frequency
 * list, parses the channel list specified using
 * QCA_WLAN_VENDOR_ATTR_ACS_CH_LIST (considers only 2 GHz and 5 GHz channels in
 * QCA_WLAN_VENDOR_ATTR_ACS_CH_LIST).
 *
 * @QCA_WLAN_VENDOR_ATTR_ACS_PRIMARY_FREQUENCY: Required (u32).
 * Used with event to notify the primary channel center frequency (MHz) selected
 * in ACS operation.
 * Note: If the driver supports the 6 GHz band, the event sent from the driver
 * includes this attribute along with QCA_WLAN_VENDOR_ATTR_ACS_PRIMARY_CHANNEL.
 *
 * @QCA_WLAN_VENDOR_ATTR_ACS_SECONDARY_FREQUENCY: Required (u32).
 * Used with event to notify the secondary channel center frequency (MHz)
 * selected in ACS operation.
 * Note: If the driver supports the 6 GHz band, the event sent from the driver
 * includes this attribute along with
 * QCA_WLAN_VENDOR_ATTR_ACS_SECONDARY_CHANNEL.
 *
 * @QCA_WLAN_VENDOR_ATTR_ACS_VHT_SEG0_CENTER_FREQUENCY: Required (u32).
 * Used with event to notify the VHT segment 0 center channel frequency (MHz)
 * selected in ACS operation.
 * Note: If the driver supports the 6 GHz band, the event sent from the driver
 * includes this attribute along with
 * QCA_WLAN_VENDOR_ATTR_ACS_VHT_SEG0_CENTER_CHANNEL.
 *
 * @QCA_WLAN_VENDOR_ATTR_ACS_VHT_SEG1_CENTER_FREQUENCY: Required (u32).
 * Used with event to notify the VHT segment 1 center channel frequency (MHz)
 * selected in ACS operation.
 * Note: If the driver supports the 6 GHz band, the event sent from the driver
 * includes this attribute along with
 * QCA_WLAN_VENDOR_ATTR_ACS_VHT_SEG1_CENTER_CHANNEL.
 *
 * @QCA_WLAN_VENDOR_ATTR_ACS_EDMG_ENABLED: Flag attribute.
 * Used with command to notify the driver of EDMG request for ACS
 * operation.
 *
 * @QCA_WLAN_VENDOR_ATTR_ACS_EDMG_CHANNEL: Optional (u8).
 * Used with event to notify the EDMG channel number selected in ACS
 * operation.
 * EDMG primary channel is indicated by QCA_WLAN_VENDOR_ATTR_ACS_PRIMARY_CHANNEL
 */
enum qca_wlan_vendor_attr_acs_offload {
	QCA_WLAN_VENDOR_ATTR_ACS_CHANNEL_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_ACS_PRIMARY_CHANNEL = 1,
	QCA_WLAN_VENDOR_ATTR_ACS_SECONDARY_CHANNEL = 2,
	QCA_WLAN_VENDOR_ATTR_ACS_HW_MODE = 3,
	QCA_WLAN_VENDOR_ATTR_ACS_HT_ENABLED = 4,
	QCA_WLAN_VENDOR_ATTR_ACS_HT40_ENABLED = 5,
	QCA_WLAN_VENDOR_ATTR_ACS_VHT_ENABLED = 6,
	QCA_WLAN_VENDOR_ATTR_ACS_CHWIDTH = 7,
	QCA_WLAN_VENDOR_ATTR_ACS_CH_LIST = 8,
	QCA_WLAN_VENDOR_ATTR_ACS_VHT_SEG0_CENTER_CHANNEL = 9,
	QCA_WLAN_VENDOR_ATTR_ACS_VHT_SEG1_CENTER_CHANNEL = 10,
	QCA_WLAN_VENDOR_ATTR_ACS_FREQ_LIST = 11,
	QCA_WLAN_VENDOR_ATTR_ACS_PRIMARY_FREQUENCY = 12,
	QCA_WLAN_VENDOR_ATTR_ACS_SECONDARY_FREQUENCY = 13,
	QCA_WLAN_VENDOR_ATTR_ACS_VHT_SEG0_CENTER_FREQUENCY = 14,
	QCA_WLAN_VENDOR_ATTR_ACS_VHT_SEG1_CENTER_FREQUENCY = 15,
	QCA_WLAN_VENDOR_ATTR_ACS_EDMG_ENABLED = 16,
	QCA_WLAN_VENDOR_ATTR_ACS_EDMG_CHANNEL = 17,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_ACS_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_ACS_MAX =
	QCA_WLAN_VENDOR_ATTR_ACS_AFTER_LAST - 1
};

/**
 * enum qca_wlan_vendor_acs_hw_mode - Defines HW mode to be used with the
 * vendor command/event QCA_NL80211_VENDOR_SUBCMD_DO_ACS.
 *
 * @QCA_ACS_MODE_IEEE80211B: 802.11b mode
 * @QCA_ACS_MODE_IEEE80211G: 802.11g mode
 * @QCA_ACS_MODE_IEEE80211A: 802.11a mode
 * @QCA_ACS_MODE_IEEE80211AD: 802.11ad mode
 * @QCA_ACS_MODE_IEEE80211ANY: all modes
 * @QCA_ACS_MODE_IEEE80211AX: 802.11ax mode
 */
enum qca_wlan_vendor_acs_hw_mode {
	QCA_ACS_MODE_IEEE80211B,
	QCA_ACS_MODE_IEEE80211G,
	QCA_ACS_MODE_IEEE80211A,
	QCA_ACS_MODE_IEEE80211AD,
	QCA_ACS_MODE_IEEE80211ANY,
	QCA_ACS_MODE_IEEE80211AX,
};

/**
 * enum qca_access_policy - access control policy
 *
 * Access control policy is applied on the configured IE
 * (QCA_WLAN_VENDOR_ATTR_CONFIG_ACCESS_POLICY_IE).
 * To be set with QCA_WLAN_VENDOR_ATTR_CONFIG_ACCESS_POLICY.
 *
 * @QCA_ACCESS_POLICY_ACCEPT_UNLESS_LISTED: Deny Wi-Fi Connections which match
 *»       with the specific configuration (IE) set, i.e. allow all the
 *»       connections which do not match the configuration.
 * @QCA_ACCESS_POLICY_DENY_UNLESS_LISTED: Accept Wi-Fi Connections which match
 *»       with the specific configuration (IE) set, i.e. deny all the
 *»       connections which do not match the configuration.
 */
enum qca_access_policy {
	QCA_ACCESS_POLICY_ACCEPT_UNLESS_LISTED,
	QCA_ACCESS_POLICY_DENY_UNLESS_LISTED,
};

/**
 * enum qca_ignore_assoc_disallowed - Ignore assoc disallowed values
 *
 * The valid values for the ignore assoc disallowed
 *
 * @QCA_IGNORE_ASSOC_DISALLOWED_DISABLE: Disable ignore assoc disallowed
 * @QCA_IGNORE_ASSOC_DISALLOWED_ENABLE: Enable ignore assoc disallowed
 *
 */
enum qca_ignore_assoc_disallowed {
	QCA_IGNORE_ASSOC_DISALLOWED_DISABLE,
	QCA_IGNORE_ASSOC_DISALLOWED_ENABLE
};

/* Attributes for data used by
 * QCA_NL80211_VENDOR_SUBCMD_SET_WIFI_CONFIGURATION and
 * QCA_NL80211_VENDOR_SUBCMD_GET_WIFI_CONFIGURATION subcommands.
 */
#define QCA_WLAN_VENDOR_ATTR_DISCONNECT_IES\
		QCA_WLAN_VENDOR_ATTR_CONFIG_DISCONNECT_IES
#define QCA_WLAN_VENDOR_ATTR_BEACON_REPORT_FAIL\
		QCA_WLAN_VENDOR_ATTR_CONFIG_BEACON_REPORT_FAIL
#define QCA_WLAN_VENDOR_ATTR_ROAM_REASON\
		QCA_WLAN_VENDOR_ATTR_CONFIG_ROAM_REASON
enum qca_wlan_vendor_attr_config {
	QCA_WLAN_VENDOR_ATTR_CONFIG_INVALID = 0,
	/*
	 * Unsigned 32-bit value to set the DTIM period.
	 * Whether the wifi chipset wakes at every dtim beacon or a multiple of
	 * the DTIM period. If DTIM is set to 3, the STA shall wake up every 3
	 * DTIM beacons.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_MODULATED_DTIM = 1,
	/*
	 * Unsigned 32-bit value to set the wifi_iface stats averaging factor
	 * used to calculate statistics like average the TSF offset or average
	 * number of frame leaked.
	 * For instance, upon Beacon frame reception:
	 * current_avg = ((beacon_TSF - TBTT) * factor + previous_avg * (0x10000 - factor) ) / 0x10000
	 * For instance, when evaluating leaky APs:
	 * current_avg = ((num frame received within guard time) * factor + previous_avg * (0x10000 - factor)) / 0x10000
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_STATS_AVG_FACTOR = 2,
	/*
	 * Unsigned 32-bit value to configure guard time, i.e., when
	 * implementing IEEE power management based on frame control PM bit, how
	 * long the driver waits before shutting down the radio and after
	 * receiving an ACK frame for a Data frame with PM bit set.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_GUARD_TIME = 3,
	/* Unsigned 32-bit value to change the FTM capability dynamically */
	QCA_WLAN_VENDOR_ATTR_CONFIG_FINE_TIME_MEASUREMENT = 4,
	/* Unsigned 16-bit value to configure maximum TX rate dynamically */
	QCA_WLAN_VENDOR_ATTR_CONF_TX_RATE = 5,
	/*
	 * Unsigned 32-bit value to configure the number of continuous
	 * Beacon Miss which shall be used by the firmware to penalize
	 * the RSSI.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_PENALIZE_AFTER_NCONS_BEACON_MISS = 6,
	/*
	 * Unsigned 8-bit value to configure the channel avoidance indication
	 * behavior. Firmware to send only one indication and ignore duplicate
	 * indications when set to avoid multiple Apps wakeups.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_CHANNEL_AVOIDANCE_IND = 7,
	/*
	 * 8-bit unsigned value to configure the maximum TX MPDU for
	 * aggregation.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_TX_MPDU_AGGREGATION = 8,
	/*
	 * 8-bit unsigned value to configure the maximum RX MPDU for
	 * aggregation.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_RX_MPDU_AGGREGATION = 9,
	/*
	 * 8-bit unsigned value to configure the Non aggregrate/11g sw
	 * retry threshold (0 disable, 31 max).
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_NON_AGG_RETRY = 10,
	/*
	 * 8-bit unsigned value to configure the aggregrate sw
	 * retry threshold (0 disable, 31 max).
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_AGG_RETRY = 11,
	/*
	 * 8-bit unsigned value to configure the MGMT frame
	 * retry threshold (0 disable, 31 max).
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_MGMT_RETRY = 12,
	/*
	 * 8-bit unsigned value to configure the CTRL frame
	 * retry threshold (0 disable, 31 max).
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_CTRL_RETRY = 13,
	/*
	 * 8-bit unsigned value to configure the propagation delay for
	 * 2G/5G band (0~63, units in us)
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_PROPAGATION_DELAY = 14,
	/*
	 * Unsigned 32-bit value to configure the number of unicast TX fail
	 * packet count. The peer is disconnected once this threshold is
	 * reached.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_TX_FAIL_COUNT = 15,
	/*
	 * Attribute used to set scan default IEs to the driver.
	 *
	 * These IEs can be used by scan operations that will be initiated by
	 * the driver/firmware.
	 *
	 * For further scan requests coming to the driver, these IEs should be
	 * merged with the IEs received along with scan request coming to the
	 * driver. If a particular IE is present in the scan default IEs but not
	 * present in the scan request, then that IE should be added to the IEs
	 * sent in the Probe Request frames for that scan request.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_SCAN_DEFAULT_IES = 16,
	/* Unsigned 32-bit attribute for generic commands */
	QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_COMMAND = 17,
	/* Unsigned 32-bit value attribute for generic commands */
	QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_VALUE = 18,
	/* Unsigned 32-bit data attribute for generic command response */
	QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_DATA = 19,
	/*
	 * Unsigned 32-bit length attribute for
	 * QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_DATA
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_LENGTH = 20,
	/*
	 * Unsigned 32-bit flags attribute for
	 * QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_DATA
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_FLAGS = 21,
	/*
	 * Unsigned 32-bit, defining the access policy.
	 * See enum qca_access_policy. Used with
	 * QCA_WLAN_VENDOR_ATTR_CONFIG_ACCESS_POLICY_IE_LIST.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ACCESS_POLICY = 22,
	/*
	 * Sets the list of full set of IEs for which a specific access policy
	 * has to be applied. Used along with
	 * QCA_WLAN_VENDOR_ATTR_CONFIG_ACCESS_POLICY to control the access.
	 * Zero length payload can be used to clear this access constraint.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ACCESS_POLICY_IE_LIST = 23,
	/*
	 * Unsigned 32-bit, specifies the interface index (netdev) for which the
	 * corresponding configurations are applied. If the interface index is
	 * not specified, the configurations are attributed to the respective
	 * wiphy.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_IFINDEX = 24,
	/*
	 * 8-bit unsigned value to trigger QPower:
	 * 1-Enable, 0-Disable
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_QPOWER = 25,
	/*
	 * 8-bit unsigned value to configure the driver and below layers to
	 * ignore the assoc disallowed set by APs while connecting
	 * 1-Ignore, 0-Don't ignore
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_IGNORE_ASSOC_DISALLOWED = 26,
	/*
	 * 32-bit unsigned value to trigger antenna diversity features:
	 * 1-Enable, 0-Disable
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_ENA = 27,
	/* 32-bit unsigned value to configure specific chain antenna */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_CHAIN = 28,
	/*
	 * 32-bit unsigned value to trigger cycle selftest
	 * 1-Enable, 0-Disable
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_SELFTEST = 29,
	/*
	 * 32-bit unsigned to configure the cycle time of selftest
	 * the unit is micro-second
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_SELFTEST_INTVL = 30,
	/* 32-bit unsigned value to set reorder timeout for AC_VO */
	QCA_WLAN_VENDOR_ATTR_CONFIG_RX_REORDER_TIMEOUT_VOICE = 31,
	/* 32-bit unsigned value to set reorder timeout for AC_VI */
	QCA_WLAN_VENDOR_ATTR_CONFIG_RX_REORDER_TIMEOUT_VIDEO = 32,
	/* 32-bit unsigned value to set reorder timeout for AC_BE */
	QCA_WLAN_VENDOR_ATTR_CONFIG_RX_REORDER_TIMEOUT_BESTEFFORT = 33,
	/* 32-bit unsigned value to set reorder timeout for AC_BK */
	QCA_WLAN_VENDOR_ATTR_CONFIG_RX_REORDER_TIMEOUT_BACKGROUND = 34,
	/* 6-byte MAC address to point out the specific peer */
	QCA_WLAN_VENDOR_ATTR_CONFIG_RX_BLOCKSIZE_PEER_MAC = 35,
	/* 32-bit unsigned value to set window size for specific peer */
	QCA_WLAN_VENDOR_ATTR_CONFIG_RX_BLOCKSIZE_WINLIMIT = 36,
	/* 8-bit unsigned value to set the beacon miss threshold in 2.4 GHz */
	QCA_WLAN_VENDOR_ATTR_CONFIG_BEACON_MISS_THRESHOLD_24 = 37,
	/* 8-bit unsigned value to set the beacon miss threshold in 5 GHz */
	QCA_WLAN_VENDOR_ATTR_CONFIG_BEACON_MISS_THRESHOLD_5 = 38,
	/*
	 * 32-bit unsigned value to configure 5 or 10 MHz channel width for
	 * station device while in disconnect state. The attribute use the
	 * value of enum nl80211_chan_width: NL80211_CHAN_WIDTH_5 means 5 MHz,
	 * NL80211_CHAN_WIDTH_10 means 10 MHz. If set, the device work in 5 or
	 * 10 MHz channel width, the station will not connect to a BSS using 20
	 * MHz or higher bandwidth. Set to NL80211_CHAN_WIDTH_20_NOHT to
	 * clear this constraint.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_SUB20_CHAN_WIDTH = 39,
	/*
	 * 32-bit unsigned value to configure the propagation absolute delay
	 * for 2G/5G band (units in us)
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_PROPAGATION_ABS_DELAY = 40,
	/* 32-bit unsigned value to set probe period */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_PROBE_PERIOD = 41,
	/* 32-bit unsigned value to set stay period */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_STAY_PERIOD = 42,
	/* 32-bit unsigned value to set snr diff */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_SNR_DIFF = 43,
	/* 32-bit unsigned value to set probe dwell time */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_PROBE_DWELL_TIME = 44,
	/* 32-bit unsigned value to set mgmt snr weight */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_MGMT_SNR_WEIGHT = 45,
	/* 32-bit unsigned value to set data snr weight */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_DATA_SNR_WEIGHT = 46,
	/* 32-bit unsigned value to set ack snr weight */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_ACK_SNR_WEIGHT = 47,
	/*
	 * 32-bit unsigned value to configure the listen interval.
	 * This is in units of beacon intervals. This configuration alters
	 * the negotiated listen interval with the AP during the connection.
	 * It is highly recommended to configure a value less than or equal to
	 * the one negotiated during the association. Configuring any greater
	 * value can have adverse effects (frame loss, AP disassociating STA,
	 * etc.).
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_LISTEN_INTERVAL = 48,
	/*
	 * 8 bit unsigned value that is set on an AP/GO virtual interface to
	 * disable operations that would cause the AP/GO to leave its operating
	 * channel.
	 *
	 * This will restrict the scans to the AP/GO operating channel and the
	 * channels of the other band, if DBS is supported.A STA/CLI interface
	 * brought up after this setting is enabled, will be restricted to
	 * connecting to devices only on the AP/GO interface's operating channel
	 * or on the other band in DBS case. P2P supported channel list is
	 * modified, to only include AP interface's operating-channel and the
	 * channels of the other band if DBS is supported.
	 *
	 * These restrictions are only applicable as long as the AP/GO interface
	 * is alive. If the AP/GO interface is brought down then this
	 * setting/restriction is forgotten.
	 *
	 * If this variable is set on an AP/GO interface while a multi-channel
	 * concurrent session is active, it has no effect on the operation of
	 * the current interfaces, other than restricting the scan to the AP/GO
	 * operating channel and the other band channels if DBS is supported.
	 * However, if the STA is brought down and restarted then the new STA
	 * connection will either be formed on the AP/GO channel or on the
	 * other band in a DBS case. This is because of the scan being
	 * restricted on these channels as mentioned above.
	 *
	 * 1-Disable offchannel operations, 0-Enable offchannel operations.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_RESTRICT_OFFCHANNEL = 49,

	/*
	 * 8 bit unsigned value to enable/disable LRO (Large Receive Offload)
	 * on an interface.
	 * 1 - Enable , 0 - Disable.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_LRO = 50,

	/*
	 * 8 bit unsigned value to globally enable/disable scan
	 * 1 - Enable, 0 - Disable.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_SCAN_ENABLE = 51,

	/* 8-bit unsigned value to set the total beacon miss count */
	QCA_WLAN_VENDOR_ATTR_CONFIG_TOTAL_BEACON_MISS_COUNT = 52,

	/*
	 * Unsigned 32-bit value to configure the number of continuous
	 * Beacon Miss which shall be used by the firmware to penalize
	 * the RSSI for BTC.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_PENALIZE_AFTER_NCONS_BEACON_MISS_BTC = 53,

	/*
	 * 8-bit unsigned value to configure the driver and below layers to
	 * enable/disable all fils features.
	 * 0-enable, 1-disable
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_DISABLE_FILS = 54,

	/* 16-bit unsigned value to configure the level of WLAN latency
	 * module. See enum qca_wlan_vendor_attr_config_latency_level.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL = 55,

	/*
	 * 8-bit unsigned value indicating the driver to use the RSNE as-is from
	 * the connect interface. Exclusively used for the scenarios where the
	 * device is used as a test bed device with special functionality and
	 * not recommended for production. This helps driver to not validate the
	 * RSNE passed from user space and thus allow arbitrary IE data to be
	 * used for testing purposes.
	 * 1-enable, 0-disable.
	 * Applications set/reset this configuration. If not reset, this
	 * parameter remains in use until the driver is unloaded.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_RSN_IE = 56,

	/*
	 * 8-bit unsigned value to trigger green Tx power saving.
	 * 1-Enable, 0-Disable
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_GTX = 57,

	/*
	 * Attribute to configure disconnect IEs to the driver.
	 * This carries an array of unsigned 8-bit characters.
	 *
	 * If this is configured, driver shall fill the IEs in disassoc/deauth
	 * frame.
	 * These IEs are expected to be considered only for the next
	 * immediate disconnection (disassoc/deauth frame) originated by
	 * the DUT, irrespective of the entity (user space/driver/firmware)
	 * triggering the disconnection.
	 * The host drivers are not expected to use the IEs set through
	 * this interface for further disconnections after the first immediate
	 * disconnection initiated post the configuration.
	 * If the IEs are also updated through cfg80211 interface (after the
	 * enhancement to cfg80211_disconnect), host driver is expected to
	 * take the union of IEs from both of these interfaces and send in
	 * further disassoc/deauth frames.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_DISCONNECT_IES = 58,

	/* 8-bit unsigned value for ELNA bypass.
	 * 1-Enable, 0-Disable
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ELNA_BYPASS = 59,

	/* 8-bit unsigned value. This attribute enables/disables the host driver
	 * to send the Beacon Report response with failure reason for the
	 * scenarios where STA cannot honor the Beacon report request from AP.
	 * 1-Enable, 0-Disable.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_BEACON_REPORT_FAIL = 60,

	/* 8-bit unsigned value. This attribute enables/disables the host driver
	 * to send roam reason information in the reassociation request to the
	 * AP. 1-Enable, 0-Disable.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ROAM_REASON = 61,

	/* 32-bit unsigned value to configure different PHY modes to the
	 * driver/firmware. The possible values are defined in
	 * enum qca_wlan_vendor_phy_mode. The configuration will be reset to
	 * default value, i.e., QCA_WLAN_VENDOR_PHY_MODE_AUTO upon restarting
	 * the driver.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_PHY_MODE = 62,

	/* 8-bit unsigned value to configure the maximum supported channel width
	 * for STA mode. If this value is configured when STA is in connected
	 * state, it should not exceed the negotiated channel width. If it is
	 * configured when STA is in disconnected state, the configured value
	 * will take effect for the next immediate connection.
	 * Possible values are:
	 *   NL80211_CHAN_WIDTH_20
	 *   NL80211_CHAN_WIDTH_40
	 *   NL80211_CHAN_WIDTH_80
	 *   NL80211_CHAN_WIDTH_80P80
	 *   NL80211_CHAN_WIDTH_160
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_CHANNEL_WIDTH = 63,

	/* 8-bit unsigned value to enable/disable dynamic bandwidth adjustment.
	 * This attribute is only applicable for STA mode. When dynamic
	 * bandwidth adjustment is disabled, STA will use static channel width
	 * the value of which is negotiated during connection.
	 * 1-enable (default), 0-disable
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_DYNAMIC_BW = 64,

	/* 8-bit unsigned value to configure the maximum number of subframes of
	 * TX MSDU for aggregation. Possible values are 0-31. When set to 0,
	 * it is decided by hardware.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_TX_MSDU_AGGREGATION = 65,

	/* 8-bit unsigned value to configure the maximum number of subframes of
	 * RX MSDU for aggregation. Possible values are 0-31. When set to 0,
	 * it is decided by hardware.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_RX_MSDU_AGGREGATION = 66,

	/* 8-bit unsigned value. This attribute is used to dynamically
	 * enable/disable the LDPC capability of the device. When configured in
	 * the disconnected state, the updated configuration will be considered
	 * for the immediately following connection attempt. If this
	 * configuration is modified while the device is in the connected state,
	 * the LDPC TX will be updated with this configuration immediately,
	 * while the LDPC RX configuration update will take place starting from
	 * the subsequent association attempt.
	 * 1-Enable, 0-Disable.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_LDPC = 67,

	/* 8-bit unsigned value. This attribute is used to dynamically
	 * enable/disable the TX STBC capability of the device. When configured
	 * in the disconnected state, the updated configuration will be
	 * considered for the immediately following connection attempt. If the
	 * connection is formed with TX STBC enabled and if this configuration
	 * is disabled during that association, the TX will be impacted
	 * immediately. Further connection attempts will disable TX STBC.
	 * However, enabling the TX STBC for a connected session with disabled
	 * capability is not allowed and will fail.
	 * 1-Enable, 0-Disable.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_TX_STBC = 68,

	/* 8-bit unsigned value. This attribute is used to dynamically
	 * enable/disable the RX STBC capability of the device. When configured
	 * in the disconnected state, the updated configuration will be
	 * considered for the immediately following connection attempt. If the
	 * configuration is modified in the connected state, there will be no
	 * impact for the current association, but further connection attempts
	 * will use the updated configuration.
	 * 1-Enable, 0-Disable.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_RX_STBC = 69,

	/* 8-bit unsigned value. This attribute is used to dynamically configure
	 * the number of spatial streams. When configured in the disconnected
	 * state, the updated configuration will be considered for the
	 * immediately following connection attempt. If the NSS is updated after
	 * the connection, the updated NSS value is notified to the peer using
	 * the Operating Mode Notification/Spatial Multiplexing Power Save
	 * frame. The updated NSS value after the connection shall not be
	 * greater than the one negotiated during the connection. Any such
	 * higher value configuration shall be returned with a failure.
	 * Only symmetric NSS configuration (such as 2X2 or 1X1) can be done
	 * using this attribute. QCA_WLAN_VENDOR_ATTR_CONFIG_TX_NSS and
	 * QCA_WLAN_VENDOR_ATTR_CONFIG_RX_NSS attributes shall be used to
	 * configure the asymmetric NSS configuration (such as 1X2).
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_NSS = 70,

	/*
	 * 8-bit unsigned value to trigger Optimized Power Management:
	 * 1-Enable, 0-Disable
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_OPTIMIZED_POWER_MANAGEMENT = 71,

	/* 8-bit unsigned value. This attribute takes the QOS/access category
	 * value represented by the enum qca_wlan_ac_type and expects the driver
	 * to upgrade the UDP frames to this QOS. The value of QCA_WLAN_AC_ALL
	 * is invalid for this attribute. This will override the DSCP value
	 * configured in the frame with the intention to only upgrade the QOS.
	 * That said, it is not intended to downgrade the QOS for the frames.
	 * Set the value to 0 ( corresponding to BE ) if the QOS upgrade needs
	 * to disable.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_UDP_QOS_UPGRADE = 72,

	/* 8-bit unsigned value. This attribute is used to dynamically configure
	 * the number of chains to be used for transmitting data. This
	 * configuration is allowed only when in connected state and will be
	 * effective until disconnected. The driver rejects this configuration
	 * if the number of spatial streams being used in the current connection
	 * cannot be supported by this configuration.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_NUM_TX_CHAINS = 73,

	/* 8-bit unsigned value. This attribute is used to dynamically configure
	 * the number of chains to be used for receiving data. This
	 * configuration is allowed only when in connected state and will be
	 * effective until disconnected. The driver rejects this configuration
	 * if the number of spatial streams being used in the current connection
	 * cannot be supported by this configuration.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_NUM_RX_CHAINS = 74,

	/* 8-bit unsigned value. This attribute is used to dynamically configure
	 * the number of spatial streams used for transmitting the data. When
	 * configured in the disconnected state, the configured value will
	 * be considered for the following connection attempt.
	 * If the NSS is updated after the connection, the updated NSS value
	 * is notified to the peer using the Operating Mode Notification/Spatial
	 * Multiplexing Power Save frame.
	 * The TX NSS value configured after the connection shall not be greater
	 * than the value negotiated during the connection. Any such higher
	 * value configuration shall be treated as invalid configuration by
	 * the driver. This attribute shall be configured along with
	 * QCA_WLAN_VENDOR_ATTR_CONFIG_RX_NSS attribute to define the symmetric
	 * configuration (such as 2X2 or 1X1) or the asymmetric
	 * configuration (such as 1X2).
	 * If QCA_WLAN_VENDOR_ATTR_CONFIG_NSS attribute is also provided along
	 * with this QCA_WLAN_VENDOR_ATTR_CONFIG_TX_NSS attribute the driver
	 * will update the TX NSS based on QCA_WLAN_VENDOR_ATTR_CONFIG_TX_NSS.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_TX_NSS = 77,

	/* 8-bit unsigned value. This attribute is used to dynamically configure
	 * the number of spatial streams used for receiving the data. When
	 * configured in the disconnected state, the configured value will
	 * be considered for the following connection attempt.
	 * If the NSS is updated after the connection, the updated NSS value
	 * is notified to the peer using the Operating Mode Notification/Spatial
	 * Multiplexing Power Save frame.
	 * The RX NSS value configured after the connection shall not be greater
	 * than the value negotiated during the connection. Any such higher
	 * value configuration shall be treated as invalid configuration by
	 * the driver. This attribute shall be configured along with
	 * QCA_WLAN_VENDOR_ATTR_CONFIG_TX_NSS attribute to define the symmetric
	 * configuration (such as 2X2 or 1X1) or the asymmetric
	 * configuration (such as 1X2).
	 * If QCA_WLAN_VENDOR_ATTR_CONFIG_NSS attribute is also provided along
	 * with this QCA_WLAN_VENDOR_ATTR_CONFIG_RX_NSS attribute the driver
	 * will update the RX NSS based on QCA_WLAN_VENDOR_ATTR_CONFIG_RX_NSS.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_RX_NSS = 78,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_CONFIG_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_CONFIG_MAX =
	QCA_WLAN_VENDOR_ATTR_CONFIG_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_wifi_logger_start - Enum for wifi logger starting
 * @QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_START_INVALID: Invalid attribute
 * @QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_RING_ID: Ring ID
 * @QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_VERBOSE_LEVEL: Verbose level
 * @QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_FLAGS: Flag
 * @QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_START_AFTER_LAST: Last value
 * @QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_START_MAX: Max value
 */
enum qca_wlan_vendor_attr_wifi_logger_start {
	QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_START_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_RING_ID = 1,
	QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_VERBOSE_LEVEL = 2,
	QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_FLAGS = 3,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_START_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_START_MAX =
		QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_START_AFTER_LAST - 1,
};

/*
 * enum qca_wlan_vendor_attr_wifi_logger_get_ring_data - Get ring data
 * @QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_GET_RING_DATA_INVALID: Invalid attribute
 * @QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_GET_RING_DATA_ID: Ring ID
 * @QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_GET_RING_DATA_AFTER_LAST: Last value
 * @QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_GET_RING_DATA_MAX: Max value
 */
enum qca_wlan_vendor_attr_wifi_logger_get_ring_data {
	QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_GET_RING_DATA_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_GET_RING_DATA_ID = 1,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_GET_RING_DATA_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_GET_RING_DATA_MAX =
		QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_GET_RING_DATA_AFTER_LAST - 1,
};

#ifdef WLAN_FEATURE_OFFLOAD_PACKETS
/**
 * enum wlan_offloaded_packets_control - control commands
 * @WLAN_START_OFFLOADED_PACKETS: start offloaded packets
 * @WLAN_STOP_OFFLOADED_PACKETS: stop offloaded packets
 *
 */
enum wlan_offloaded_packets_control {
	WLAN_START_OFFLOADED_PACKETS = 1,
	WLAN_STOP_OFFLOADED_PACKETS  = 2
};

/**
 * enum qca_wlan_vendor_attr_data_offload_ind - Vendor Data Offload Indication
 *
 * @QCA_WLAN_VENDOR_ATTR_DATA_OFFLOAD_IND_SESSION: Session corresponding to
 *      the offloaded data.
 * @QCA_WLAN_VENDOR_ATTR_DATA_OFFLOAD_IND_PROTOCOL: Protocol of the offloaded
 *      data.
 * @QCA_WLAN_VENDOR_ATTR_DATA_OFFLOAD_IND_EVENT: Event type for the data offload
 *      indication.
 */
enum qca_wlan_vendor_attr_data_offload_ind {
	QCA_WLAN_VENDOR_ATTR_DATA_OFFLOAD_IND_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_DATA_OFFLOAD_IND_SESSION,
	QCA_WLAN_VENDOR_ATTR_DATA_OFFLOAD_IND_PROTOCOL,
	QCA_WLAN_VENDOR_ATTR_DATA_OFFLOAD_IND_EVENT,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_DATA_OFFLOAD_IND_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_DATA_OFFLOAD_IND_MAX =
	QCA_WLAN_VENDOR_ATTR_DATA_OFFLOAD_IND_AFTER_LAST - 1
};

/**
 * enum qca_wlan_vendor_attr_offloaded_packets - Used by the vendor command
 * QCA_NL80211_VENDOR_SUBCMD_OFFLOADED_PACKETS.
 */
enum qca_wlan_vendor_attr_offloaded_packets {
	QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_INVALID = 0,
	/* Takes valid value from the enum
	 * qca_wlan_offloaded_packets_sending_control
	 * Unsigned 32-bit value
	 */
	QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_SENDING_CONTROL,
	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_REQUEST_ID,
	/* array of u8 len: Max packet size */
	QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_IP_PACKET_DATA,
	/* 6-byte MAC address used to represent source MAC address */
	QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_SRC_MAC_ADDR,
	/* 6-byte MAC address used to represent destination MAC address */
	QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_DST_MAC_ADDR,
	/* Unsigned 32-bit value, in milli seconds */
	QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_PERIOD,
	/* This optional unsigned 16-bit attribute is used for specifying
	 * ethernet protocol type. If not specified ethertype defaults to IPv4.
	 */
	QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_ETHER_PROTO_TYPE,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_MAX =
		QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_AFTER_LAST - 1,
};

#endif

/**
 * enum qca_wlan_rssi_monitoring_control - rssi control commands
 * @QCA_WLAN_RSSI_MONITORING_CONTROL_INVALID: invalid
 * @QCA_WLAN_RSSI_MONITORING_START: rssi monitoring start
 * @QCA_WLAN_RSSI_MONITORING_STOP: rssi monitoring stop
 */
enum qca_wlan_rssi_monitoring_control {
	QCA_WLAN_RSSI_MONITORING_CONTROL_INVALID = 0,
	QCA_WLAN_RSSI_MONITORING_START,
	QCA_WLAN_RSSI_MONITORING_STOP,
};

/**
 * enum qca_wlan_vendor_attr_rssi_monitoring - rssi monitoring
 * @QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_INVALID: Invalid
 * @QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_CONTROL: control
 * @QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_MAX_RSSI: max rssi
 * @QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_MIN_RSSI: min rssi
 * @QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_CUR_BSSID: current bssid
 * @QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_CUR_RSSI: current rssi
 * @QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_AFTER_LAST: after last
 * @QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_MAX: max
 */
enum qca_wlan_vendor_attr_rssi_monitoring {
	QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_INVALID = 0,

	QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_CONTROL,
	QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_REQUEST_ID,

	QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_MAX_RSSI,
	QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_MIN_RSSI,

	/* attributes to be used/received in callback */
	QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_CUR_BSSID,
	QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_CUR_RSSI,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_MAX =
		QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_ndp_params - Used by the vendor command
 * QCA_NL80211_VENDOR_SUBCMD_NDP.
 * @QCA_WLAN_VENDOR_ATTR_NDP_PARAM_INVALID
 * @QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD: sub commands values in qca_wlan_ndp_sub_cmd
 * @QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID:
 * @QCA_WLAN_VENDOR_ATTR_NDP_SERVICE_INSTANCE_ID: indicats a service info
 * @QCA_WLAN_VENDOR_ATTR_NDP_CHANNEL: channel frequency in MHz
 * @QCA_WLAN_VENDOR_ATTR_NDP_PEER_DISCOVERY_MAC_ADDR: Interface Discovery MAC
 * address
 * @QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR: Interface name on which NDP is being
 * created
 * @QCA_WLAN_VENDOR_ATTR_NDP_CONFIG_SECURITY: CONFIG_SECURITY is deprecated, use
 * NCS_SK_TYPE/PMK/SCID instead
 * @QCA_WLAN_VENDOR_ATTR_NDP_CONFIG_QOS: value for QoS
 * @QCA_WLAN_VENDOR_ATTR_NDP_APP_INFO: app info
 * @QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID: NDP instance Id
 * @QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID_ARRAY: Array of instance Ids
 * @QCA_WLAN_VENDOR_ATTR_NDP_RESPONSE_CODE: initiator/responder NDP response
 * code: accept/reject
 * @QCA_WLAN_VENDOR_ATTR_NDP_NDI_MAC_ADDR: NDI MAC address
 * @QCA_WLAN_VENDOR_ATTR_NDP_DRV_RESPONSE_STATUS_TYPE: errors types returned by
 * driver
 * @QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_VALUE: value error values returned by
 * driver
 * @QCA_WLAN_VENDOR_ATTR_NDP_CHANNEL_CONFIG: Channel setup configuration
 * @QCA_WLAN_VENDOR_ATTR_NDP_CSID: Cipher Suite Shared Key Type
 * @QCA_WLAN_VENDOR_ATTR_NDP_PMK: PMK_INFO
 * @QCA_WLAN_VENDOR_ATTR_NDP_SCID: Security Context Identifier that contains the
 * PMKID
 * @QCA_WLAN_VENDOR_ATTR_NDP_PASSPHRASE: passphrase
 * @QCA_WLAN_VENDOR_ATTR_NDP_SERVICE_NAME: service name
 * @QCA_WLAN_VENDOR_ATTR_NDP_SCHEDULE_UPDATE_REASON: bitmap indicating schedule
 * update:
 *     BIT_0: NSS Update
 *     BIT_1: Channel list update
 * @QCA_WLAN_VENDOR_ATTR_NDP_NSS: nss
 * @QCA_WLAN_VENDOR_ATTR_NDP_NUM_CHANNELS: NUMBER NDP CHANNEL
 * @QCA_WLAN_VENDOR_ATTR_NDP_CHANNEL_WIDTH: CHANNEL BANDWIDTH:
 *     0:20 MHz,
 *     1:40 MHz,
 *     2:80 MHz,
 *     3:160 MHz
 * @QCA_WLAN_VENDOR_ATTR_NDP_CHANNEL_INFO: Array of channel/band width
 * @QCA_WLAN_VENDOR_ATTR_NDP_PARAMS_AFTER_LAST: id after last valid attribute
 * @QCA_WLAN_VENDOR_ATTR_NDP_PARAMS_MAX: max value of this enum type
 * @QCA_WLAN_VENDOR_ATTR_NDP_IPV6_ADDR: IPv6 address used by NDP, 16 bytes array
 * @QCA_WLAN_VENDOR_ATTR_NDP_TRANSPORT_PORT: Unsigned 16-bit value indicating
 * transport port used by NDP.
 * QCA_WLAN_VENDOR_ATTR_NDP_TRANSPORT_PROTOCOL: Unsigned 8-bit value indicating
 * protocol used by NDP and assigned by the Internet Assigned Numbers Authority
 * as per: www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml
*/
enum qca_wlan_vendor_attr_ndp_params {
	QCA_WLAN_VENDOR_ATTR_NDP_PARAM_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD = 1,
	QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID = 2,
	QCA_WLAN_VENDOR_ATTR_NDP_SERVICE_INSTANCE_ID = 3,
	QCA_WLAN_VENDOR_ATTR_NDP_CHANNEL = 4,
	QCA_WLAN_VENDOR_ATTR_NDP_PEER_DISCOVERY_MAC_ADDR = 5,
	QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR = 6,
	QCA_WLAN_VENDOR_ATTR_NDP_CONFIG_SECURITY = 7,
	QCA_WLAN_VENDOR_ATTR_NDP_CONFIG_QOS = 8,
	QCA_WLAN_VENDOR_ATTR_NDP_APP_INFO = 9,
	QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID = 10,
	QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID_ARRAY = 11,
	QCA_WLAN_VENDOR_ATTR_NDP_RESPONSE_CODE = 12,
	QCA_WLAN_VENDOR_ATTR_NDP_NDI_MAC_ADDR = 13,
	QCA_WLAN_VENDOR_ATTR_NDP_DRV_RESPONSE_STATUS_TYPE = 14,
	QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_VALUE = 15,
	QCA_WLAN_VENDOR_ATTR_NDP_CHANNEL_CONFIG = 16,
	QCA_WLAN_VENDOR_ATTR_NDP_CSID = 17,
	QCA_WLAN_VENDOR_ATTR_NDP_PMK = 18,
	QCA_WLAN_VENDOR_ATTR_NDP_SCID = 19,
	QCA_WLAN_VENDOR_ATTR_NDP_PASSPHRASE = 20,
	QCA_WLAN_VENDOR_ATTR_NDP_SERVICE_NAME = 21,
	QCA_WLAN_VENDOR_ATTR_NDP_SCHEDULE_UPDATE_REASON = 22,
	QCA_WLAN_VENDOR_ATTR_NDP_NSS = 23,
	QCA_WLAN_VENDOR_ATTR_NDP_NUM_CHANNELS = 24,
	QCA_WLAN_VENDOR_ATTR_NDP_CHANNEL_WIDTH = 25,
	QCA_WLAN_VENDOR_ATTR_NDP_CHANNEL_INFO = 26,
	QCA_WLAN_VENDOR_ATTR_NDP_IPV6_ADDR = 27,
	QCA_WLAN_VENDOR_ATTR_NDP_TRANSPORT_PORT = 28,
	QCA_WLAN_VENDOR_ATTR_NDP_TRANSPORT_PROTOCOL = 29,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_NDP_PARAMS_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_NDP_PARAMS_MAX =
		QCA_WLAN_VENDOR_ATTR_NDP_PARAMS_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_ndp_sub_cmd - NDP sub comands types for
 * QCA_NL80211_VENDOR_SUBCMD_NDP.
 * @QCA_WLAN_VENDOR_ATTR_NDP_INVALID: invalid value
 * @QCA_WLAN_VENDOR_ATTR_NDP_INTERFACE_CREATE: create a ndi
 * @QCA_WLAN_VENDOR_ATTR_NDP_INTERFACE_DELETE: delete a ndi
 * @QCA_WLAN_VENDOR_ATTR_NDP_INITIATOR_REQUEST: initiate a ndp session
 * @QCA_WLAN_VENDOR_ATTR_NDP_INITIATOR_RESPONSE: response for above
 * @QCA_WLAN_VENDOR_ATTR_NDP_RESPONDER_REQUEST: respond to ndp session
 * @QCA_WLAN_VENDOR_ATTR_NDP_RESPONDER_RESPONSE: response for above
 * @QCA_WLAN_VENDOR_ATTR_NDP_END_REQUEST: initiate a ndp end
 * @QCA_WLAN_VENDOR_ATTR_NDP_END_RESPONSE: response for above
 * @QCA_WLAN_VENDOR_ATTR_NDP_REQUEST_IND: notify the peer about the end request
 * @QCA_WLAN_VENDOR_ATTR_NDP_CONFIRM_IND: confirm the ndp session is complete
 * @QCA_WLAN_VENDOR_ATTR_NDP_END_IND: indicate the peer about the end request
 * being received
 * @QCA_WLAN_VENDOR_ATTR_NDP_SCHEDULE_UPDATE_IND: indicate the peer of schedule
 * update
 */
enum qca_wlan_ndp_sub_cmd {
	QCA_WLAN_VENDOR_ATTR_NDP_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_NDP_INTERFACE_CREATE = 1,
	QCA_WLAN_VENDOR_ATTR_NDP_INTERFACE_DELETE = 2,
	QCA_WLAN_VENDOR_ATTR_NDP_INITIATOR_REQUEST = 3,
	QCA_WLAN_VENDOR_ATTR_NDP_INITIATOR_RESPONSE = 4,
	QCA_WLAN_VENDOR_ATTR_NDP_RESPONDER_REQUEST = 5,
	QCA_WLAN_VENDOR_ATTR_NDP_RESPONDER_RESPONSE = 6,
	QCA_WLAN_VENDOR_ATTR_NDP_END_REQUEST = 7,
	QCA_WLAN_VENDOR_ATTR_NDP_END_RESPONSE = 8,
	QCA_WLAN_VENDOR_ATTR_NDP_REQUEST_IND = 9,
	QCA_WLAN_VENDOR_ATTR_NDP_CONFIRM_IND = 10,
	QCA_WLAN_VENDOR_ATTR_NDP_END_IND = 11,
	QCA_WLAN_VENDOR_ATTR_NDP_SCHEDULE_UPDATE_IND = 12
};

/**
 * qca_wlan_vendor_external_acs_event_chan_info_attr: Represents per channel
 * information. These attributes are sent as part of
 * QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_CHAN_INFO. Each set of the following
 * attributes correspond to a single channel.
 * @QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FLAGS: A bitmask (u16)
 * with flags specified in qca_wlan_vendor_channel_prop_flags_ext.
 * @QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FLAG_EXT: A bitmask (u16)
 * with flags specified in qca_wlan_vendor_channel_prop_flags_ext.
 * @QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FREQ: frequency
 * @QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_MAX_REG_POWER: maximum
 * regulatory transmission power
 * @QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_MAX_POWER: maximum
 * transmission power
 * @QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_MIN_POWER: minimum
 * transmission power
 * @QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_REG_CLASS_ID: regulatory
 * class id
 * @QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_ANTENNA_GAIN: maximum
 * antenna gain in dbm
 * @QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_VHT_SEG_0: vht segment 0
 * @QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_VHT_SEG_1: vht segment 1
 * @QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FREQ_VHT_SEG_0: vht
 * segment 0 in center freq in MHz.
 * @QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FREQ_VHT_SEG_1: vht
 * segment 1 in center freq in MHz.
 *
 */
enum qca_wlan_vendor_external_acs_event_chan_info_attr {
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_INVALID = 0,

	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FLAGS = 1,
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FLAG_EXT = 2,
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FREQ = 3,
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_MAX_REG_POWER = 4,
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_MAX_POWER = 5,
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_MIN_POWER = 6,
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_REG_CLASS_ID = 7,
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_ANTENNA_GAIN = 8,
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_VHT_SEG_0 = 9,
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_VHT_SEG_1 = 10,
	/*
	 * A bitmask (u32) with flags specified in
	 * enum qca_wlan_vendor_channel_prop_flags_2.
	 */
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FLAGS_2 = 11,

	/*
	 * VHT segment 0 in MHz (u32) and the attribute is mandatory.
	 * Note: Event QCA_NL80211_VENDOR_SUBCMD_EXTERNAL_ACS includes
	 * QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FREQ_VHT_SEG_0
	 * along with
	 * QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_VHT_SEG_0.
	 *
	 * If both the driver and user-space application supports the 6 GHz
	 * band, QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_VHT_SEG_0
	 * is deprecated and
	 * QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FREQ_VHT_SEG_0
	 * should be used.
	 *
	 * To maintain backward compatibility,
	 * QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FREQ_VHT_SEG_0
	 * is still used if either of the driver or user space application
	 * doesn't support the 6 GHz band.
	 */
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FREQ_VHT_SEG_0 = 12,

	/*
	 * VHT segment 1 in MHz (u32) and the attribute is mandatory.
	 * Note: Event QCA_NL80211_VENDOR_SUBCMD_EXTERNAL_ACS includes
	 * QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FREQ_VHT_SEG_1
	 * along with
	 * QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_VHT_SEG_1.
	 *
	 * If both the driver and user-space application supports the 6 GHz
	 * band, QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_VHT_SEG_1
	 * is deprecated and
	 * QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FREQ_VHT_SEG_1
	 * should be considered.
	 *
	 * To maintain backward compatibility,
	 * QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FREQ_VHT_SEG_1
	 * is still used if either of the driver or user space application
	 * doesn't support the 6 GHz band.
	 */
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FREQ_VHT_SEG_1 = 13,

	/* keep last */
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_LAST,
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_MAX =
		QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_start_acs_config: attribute to vendor sub-command
 * QCA_NL80211_VENDOR_SUBCMD_START_ACS. This will be triggered by host
 * driver.
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_REASON: This reason refers to
 * qca_wlan_vendor_acs_select_reason. This helps acs module to understand why
 * ACS need to be started
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_IS_SPECTRAL_SUPPORTED: Does
 * driver supports spectral scanning or not
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_IS_OFFLOAD_ENABLED: Is 11ac is
 * offloaded to firmware.
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_ADD_CHAN_STATS_SUPPORT: Does driver
 * provides additional channel capability as part of scan operation.
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_AP_UP:Flag attribute to indicate
 * interface status is UP
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_SAP_MODE: Operating mode of
 * interface. It takes one of nl80211_iftype values.
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_CHAN_WIDTH: This is the upper bound
 * of chan width. ACS logic should try to get a channel with specified width
 * if not found then look for lower values.
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_BAND: nl80211_bands
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_PHY_MODE: PHY/HW mode such as
 * a/b/g/n/ac.
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_FREQ_LIST: Supported frequency list
 * among which ACS should choose best frequency.
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_PCL:Preferred Chan List by the
 * driver which will have <channel(u8), weight(u8)> format as array of
 * nested values.
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_CHAN_INFO: Array of nested attribute
 * for each channel. It takes attr as defined in
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_POLICY:External ACS policy such as
 * PCL mandatory, PCL preferred, etc.It uses values defined in enum
 * qca_wlan_vendor_attr_external_acs_policy.
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_RROPAVAIL_INFO: Reference RF
 * Operating Parameter (RROP) availability information (u16). It uses values
 * defined in enum qca_wlan_vendor_attr_rropavail_info.
 */
enum qca_wlan_vendor_attr_external_acs_event {
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_REASON = 1,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_IS_SPECTRAL_SUPPORTED = 2,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_IS_OFFLOAD_ENABLED = 3,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_ADD_CHAN_STATS_SUPPORT = 4,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_AP_UP = 5,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_SAP_MODE = 6,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_CHAN_WIDTH = 7,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_BAND = 8,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_PHY_MODE = 9,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_FREQ_LIST = 10,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_PCL = 11,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_CHAN_INFO = 12,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_POLICY = 13,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_RROPAVAIL_INFO = 14,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_LAST,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_MAX =
		QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_LAST - 1,
};

enum qca_iface_type {
	QCA_IFACE_TYPE_STA,
	QCA_IFACE_TYPE_AP,
	QCA_IFACE_TYPE_P2P_CLIENT,
	QCA_IFACE_TYPE_P2P_GO,
	QCA_IFACE_TYPE_IBSS,
	QCA_IFACE_TYPE_TDLS,
};

/**
 * enum qca_wlan_vendor_attr_pcl_config: attribute to vendor sub-command
 * QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_PCL and
 * QCA_NL80211_VENDOR_SUBCMD_GET_PREFERRED_FREQ_LIST.
 * @QCA_WLAN_VENDOR_ATTR_PCL_CONFIG_INVALID: invalid value
 * @QCA_WLAN_VENDOR_ATTR_PCL_CONFIG_CHANNEL: pcl channel number
 * @QCA_WLAN_VENDOR_ATTR_PCL_CONFIG_WEIGHT: pcl channel weight
 * @QCA_WLAN_VENDOR_ATTR_PCL_CONFIG_FREQ: pcl channel frequency
 * @QCA_WLAN_VENDOR_ATTR_PCL_CONFIG_FLAG: pcl channel flag bitmask
 */
enum qca_wlan_vendor_attr_pcl_config {
	QCA_WLAN_VENDOR_ATTR_PCL_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_PCL_CHANNEL = 1,
	QCA_WLAN_VENDOR_ATTR_PCL_WEIGHT = 2,
	QCA_WLAN_VENDOR_ATTR_PCL_FREQ = 3,
	QCA_WLAN_VENDOR_ATTR_PCL_FLAG = 4,
};

enum qca_set_band {
	QCA_SETBAND_AUTO = 0,
	QCA_SETBAND_5G = BIT(0),
	QCA_SETBAND_2G = BIT(1),
	QCA_SETBAND_6G = BIT(2),
};

/**
 * enum set_reset_packet_filter - set packet filter control commands
 * @QCA_WLAN_SET_PACKET_FILTER: Set Packet Filter
 * @QCA_WLAN_GET_PACKET_FILTER: Get Packet filter
 * @QCA_WLAN_WRITE_PACKET_FILTER: Write packet filter program/data
 * @QCA_WLAN_READ_PACKET_FILTER: Read packet filter program/data
 * @QCA_WLAN_ENABLE_PACKET_FILTER: Enable APF interpreter
 * @QCA_WLAN_DISABLE_PACKET_FILTER: Disable APF interpreter
 */
enum set_reset_packet_filter {
	QCA_WLAN_SET_PACKET_FILTER = 1,
	QCA_WLAN_GET_PACKET_FILTER = 2,
	QCA_WLAN_WRITE_PACKET_FILTER = 3,
	QCA_WLAN_READ_PACKET_FILTER = 4,
	QCA_WLAN_ENABLE_PACKET_FILTER = 5,
	QCA_WLAN_DISABLE_PACKET_FILTER = 6,
};

/**
 * enum qca_wlan_vendor_attr_packet_filter - APF control commands
 * @QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_INVALID: Invalid
 * @QCA_WLAN_VENDOR_ATTR_SET_RESET_PACKET_FILTER: Filter ID
 * @QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_VERSION: Filter Version
 * @QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_SIZE: Total Length
 * @QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_CURRENT_OFFSET: Current offset
 * @QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_PROGRAM: length of APF instructions
 * @QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_PROG_LENGTH: length of the program
 *	section in packet filter buffer
 */
enum qca_wlan_vendor_attr_packet_filter {
	QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_SET_RESET_PACKET_FILTER,
	QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_VERSION,
	QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_ID,
	QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_SIZE,
	QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_CURRENT_OFFSET,
	QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_PROGRAM,
	QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_PROG_LENGTH,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_MAX =
	QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_wake_stats - Wake lock stats used by vendor
 * command QCA_NL80211_VENDOR_SUBCMD_GET_WAKE_REASON_STATS.
 */
enum qca_wlan_vendor_attr_wake_stats {
	QCA_WLAN_VENDOR_ATTR_WAKE_STATS_INVALID = 0,
	/* Unsigned 32-bit value indicating the total count of wake event */
	QCA_WLAN_VENDOR_ATTR_WAKE_STATS_TOTAL_CMD_EVENT_WAKE,
	/* Array of individual wake count, each index representing wake reason
	 */
	QCA_WLAN_VENDOR_ATTR_WAKE_STATS_CMD_EVENT_WAKE_CNT_PTR,
	/* Unsigned 32-bit value representing wake count array */
	QCA_WLAN_VENDOR_ATTR_WAKE_STATS_CMD_EVENT_WAKE_CNT_SZ,
	/* Unsigned 32-bit total wake count value of driver/fw */
	QCA_WLAN_VENDOR_ATTR_WAKE_STATS_TOTAL_DRIVER_FW_LOCAL_WAKE,
	/* Array of wake stats of driver/fw */
	QCA_WLAN_VENDOR_ATTR_WAKE_STATS_DRIVER_FW_LOCAL_WAKE_CNT_PTR,
	/* Unsigned 32-bit total wake count value of driver/fw */
	QCA_WLAN_VENDOR_ATTR_WAKE_STATS_DRIVER_FW_LOCAL_WAKE_CNT_SZ,
	/* Unsigned 32-bit total wake count value of packets received */
	QCA_WLAN_VENDOR_ATTR_WAKE_STATS_TOTAL_RX_DATA_WAKE,
	/* Unsigned 32-bit wake count value unicast packets received */
	QCA_WLAN_VENDOR_ATTR_WAKE_STATS_RX_UNICAST_CNT,
	/* Unsigned 32-bit wake count value multicast packets received */
	QCA_WLAN_VENDOR_ATTR_WAKE_STATS_RX_MULTICAST_CNT,
	/* Unsigned 32-bit wake count value broadcast packets received */
	QCA_WLAN_VENDOR_ATTR_WAKE_STATS_RX_BROADCAST_CNT,
	/* Unsigned 32-bit wake count value of ICMP packets */
	QCA_WLAN_VENDOR_ATTR_WAKE_STATS_ICMP_PKT,
	/* Unsigned 32-bit wake count value of ICMP6 packets */
	QCA_WLAN_VENDOR_ATTR_WAKE_STATS_ICMP6_PKT,
	/* Unsigned 32-bit value ICMP6 router advertisement */
	QCA_WLAN_VENDOR_ATTR_WAKE_STATS_ICMP6_RA,
	/* Unsigned 32-bit value ICMP6 neighbor advertisement */
	QCA_WLAN_VENDOR_ATTR_WAKE_STATS_ICMP6_NA,
	/* Unsigned 32-bit value ICMP6 neighbor solicitation */
	QCA_WLAN_VENDOR_ATTR_WAKE_STATS_ICMP6_NS,
	/* Unsigned 32-bit wake count value of receive side ICMP4 multicast */
	QCA_WLAN_VENDOR_ATTR_WAKE_STATS_ICMP4_RX_MULTICAST_CNT,
	/* Unsigned 32-bit wake count value of receive side ICMP6 multicast */
	QCA_WLAN_VENDOR_ATTR_WAKE_STATS_ICMP6_RX_MULTICAST_CNT,
	/* Unsigned 32-bit wake count value of receive side multicast */
	QCA_WLAN_VENDOR_ATTR_WAKE_STATS_OTHER_RX_MULTICAST_CNT,
	/* Unsigned 32-bit wake count value of a given RSSI breach */
	QCA_WLAN_VENDOR_ATTR_WAKE_STATS_RSSI_BREACH_CNT,
	/* Unsigned 32-bit wake count value of low RSSI */
	QCA_WLAN_VENDOR_ATTR_WAKE_STATS_LOW_RSSI_CNT,
	/* Unsigned 32-bit value GSCAN count */
	QCA_WLAN_VENDOR_ATTR_WAKE_STATS_GSCAN_CNT,
	/* Unsigned 32-bit value PNO complete count */
	QCA_WLAN_VENDOR_ATTR_WAKE_STATS_PNO_COMPLETE_CNT,
	/* Unsigned 32-bit value PNO match count */
	QCA_WLAN_VENDOR_ATTR_WAKE_STATS_PNO_MATCH_CNT,
	/* keep last */
	QCA_WLAN_VENDOR_GET_WAKE_STATS_AFTER_LAST,
	QCA_WLAN_VENDOR_GET_WAKE_STATS_MAX =
		QCA_WLAN_VENDOR_GET_WAKE_STATS_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_thermal_level - Defines various thermal levels
 * configured by userspace to the driver/firmware. The values will be
 * encapsulated in QCA_WLAN_VENDOR_ATTR_THERMAL_LEVEL attribute.
 * The driver/firmware takes necessary actions requested by userspace
 * such as throttling wifi tx etc. in order to mitigate high temperature.
 *
 * @QCA_WLAN_VENDOR_THERMAL_LEVEL_NONE: Stop/clear all throttling actions.
 * @QCA_WLAN_VENDOR_THERMAL_LEVEL_LIGHT: Throttle tx lightly.
 * @QCA_WLAN_VENDOR_THERMAL_LEVEL_MODERATE: Throttle tx moderately.
 * @QCA_WLAN_VENDOR_THERMAL_LEVEL_SEVERE: Throttle tx severely.
 * @QCA_WLAN_VENDOR_THERMAL_LEVEL_CRITICAL: Critical thermal level reached.
 * @QCA_WLAN_VENDOR_THERMAL_LEVEL_EMERGENCY: Emergency thermal level reached.
 */
enum qca_wlan_vendor_thermal_level {
	QCA_WLAN_VENDOR_THERMAL_LEVEL_NONE = 0,
	QCA_WLAN_VENDOR_THERMAL_LEVEL_LIGHT = 1,
	QCA_WLAN_VENDOR_THERMAL_LEVEL_MODERATE = 2,
	QCA_WLAN_VENDOR_THERMAL_LEVEL_SEVERE = 3,
	QCA_WLAN_VENDOR_THERMAL_LEVEL_CRITICAL = 4,
	QCA_WLAN_VENDOR_THERMAL_LEVEL_EMERGENCY = 5,
};

/**
 * enum qca_wlan_vendor_attr_thermal_cmd - Vendor subcmd attributes to set
 * cmd value. Used for NL attributes for data used by
 * QCA_NL80211_VENDOR_SUBCMD_THERMAL_CMD sub command.
 */
enum qca_wlan_vendor_attr_thermal_cmd {
	QCA_WLAN_VENDOR_ATTR_THERMAL_CMD_INVALID = 0,
	/*
	 * The value of command, driver will implement different operations
	 * according to this value. It uses values defined in
	 * enum qca_wlan_vendor_attr_thermal_cmd_type.
	 * u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_THERMAL_CMD_VALUE = 1,
	/*
	 * Userspace uses this attribute to configure thermal level to
	 * driver/firmware. Used in request, u32 attribute, possible values
	 * are defined in enum qca_wlan_vendor_thermal_level.
	 */
	QCA_WLAN_VENDOR_ATTR_THERMAL_LEVEL = 2,
	/*
	 * Userspace uses this attribute to configure the time in which the
	 * driver/firmware should complete applying settings it received from
	 * userspace with QCA_WLAN_VENDOR_ATTR_THERMAL_CMD_TYPE_SET_LEVEL
	 * command type. Used in request, u32 attribute, value is in milli
	 * seconds. A value of zero indicates to apply the settings
	 * immediately. The driver/firmware can delay applying the configured
	 * thermal settings within the time specified in this attribute if
	 * there is any critical ongoing operation.
	 */
	QCA_WLAN_VENDOR_ATTR_THERMAL_COMPLETION_WINDOW = 3,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_THERMAL_CMD_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_THERMAL_CMD_MAX =
		QCA_WLAN_VENDOR_ATTR_THERMAL_CMD_AFTER_LAST - 1
};

/**
 * qca_wlan_vendor_attr_thermal_cmd_type: Attribute values for
 * QCA_WLAN_VENDOR_ATTR_THERMAL_CMD_VALUE to the vendor subcmd
 * QCA_NL80211_VENDOR_SUBCMD_THERMAL_CMD. This represents the
 * thermal command types sent to driver.
 * @QCA_WLAN_VENDOR_ATTR_THERMAL_CMD_TYPE_GET_PARAMS: Request to
 * get thermal shutdown configuration parameters for display. Parameters
 * responded from driver are defined in
 * enum qca_wlan_vendor_attr_get_thermal_params_rsp.
 * @QCA_WLAN_VENDOR_ATTR_THERMAL_CMD_TYPE_GET_TEMPERATURE: Request to
 * get temperature. Host should respond with a temperature data. It is defined
 * in enum qca_wlan_vendor_attr_thermal_get_temperature.
 * @QCA_WLAN_VENDOR_ATTR_THERMAL_CMD_TYPE_SUSPEND: Request to execute thermal
 * suspend action.
 * @QCA_WLAN_VENDOR_ATTR_THERMAL_CMD_TYPE_RESUME: Request to execute thermal
 * resume action.
 * @QCA_WLAN_VENDOR_ATTR_THERMAL_CMD_TYPE_SET_LEVEL: Configure thermal level to
 * the driver/firmware.
 */
enum qca_wlan_vendor_attr_thermal_cmd_type {
	QCA_WLAN_VENDOR_ATTR_THERMAL_CMD_TYPE_GET_PARAMS,
	QCA_WLAN_VENDOR_ATTR_THERMAL_CMD_TYPE_GET_TEMPERATURE,
	QCA_WLAN_VENDOR_ATTR_THERMAL_CMD_TYPE_SUSPEND,
	QCA_WLAN_VENDOR_ATTR_THERMAL_CMD_TYPE_RESUME,
	QCA_WLAN_VENDOR_ATTR_THERMAL_CMD_TYPE_SET_LEVEL,
};

/**
 * enum qca_vendor_element_id - QCA Vendor Specific element types
 *
 * These values are used to identify QCA Vendor Specific elements. The
 * payload of the element starts with the three octet OUI (OUI_QCA) and
 * is followed by a single octet type which is defined by this enum.
 *
 * @QCA_VENDOR_ELEM_P2P_PREF_CHAN_LIST: P2P preferred channel list.
 *      This element can be used to specify preference order for supported
 *      channels. The channels in this list are in preference order (the first
 *      one has the highest preference) and are described as a pair of
 *      (global) Operating Class and Channel Number (each one octet) fields.
 *
 *      This extends the standard P2P functionality by providing option to have
 *      more than one preferred operating channel. When this element is present,
 *      it replaces the preference indicated in the Operating Channel attribute.
 *      For supporting other implementations, the Operating Channel attribute is
 *      expected to be used with the highest preference channel. Similarly, all
 *      the channels included in this Preferred channel list element are
 *      expected to be included in the Channel List attribute.
 *
 *      This vendor element may be included in GO Negotiation Request, P2P
 *      Invitation Request, and Provision Discovery Request frames.
 *
 * @QCA_VENDOR_ELEM_HE_CAPAB: HE Capabilities element.
 *      This element can be used for pre-standard publication testing of HE
 *      before P802.11ax draft assigns the element ID. The payload of this
 *      vendor specific element is defined by the latest P802.11ax draft.
 *      Please note that the draft is still work in progress and this element
 *      payload is subject to change.
 *
 * @QCA_VENDOR_ELEM_HE_OPER: HE Operation element.
 *      This element can be used for pre-standard publication testing of HE
 *      before P802.11ax draft assigns the element ID. The payload of this
 *      vendor specific element is defined by the latest P802.11ax draft.
 *      Please note that the draft is still work in progress and this element
 *      payload is subject to change.
 *
 * @QCA_VENDOR_ELEM_RAPS: RAPS element (OFDMA-based Random Access Parameter Set
 *      element).
 *      This element can be used for pre-standard publication testing of HE
 *      before P802.11ax draft assigns the element ID extension. The payload of
 *      this vendor specific element is defined by the latest P802.11ax draft
 *      (not including the Element ID Extension field). Please note that the
 *      draft is still work in progress and this element payload is subject to
 *      change.
 *
 * @QCA_VENDOR_ELEM_MU_EDCA_PARAMS: MU EDCA Parameter Set element.
 *      This element can be used for pre-standard publication testing of HE
 *      before P802.11ax draft assigns the element ID extension. The payload of
 *      this vendor specific element is defined by the latest P802.11ax draft
 *      (not including the Element ID Extension field). Please note that the
 *      draft is still work in progress and this element payload is subject to
 *      change.
 *
 * @QCA_VENDOR_ELEM_BSS_COLOR_CHANGE: BSS Color Change Announcement element.
 *      This element can be used for pre-standard publication testing of HE
 *      before P802.11ax draft assigns the element ID extension. The payload of
 *      this vendor specific element is defined by the latest P802.11ax draft
 *      (not including the Element ID Extension field). Please note that the
 *      draft is still work in progress and this element payload is subject to
 *      change.
 */
enum qca_vendor_element_id {
	QCA_VENDOR_ELEM_P2P_PREF_CHAN_LIST = 0,
	QCA_VENDOR_ELEM_HE_CAPAB = 1,
	QCA_VENDOR_ELEM_HE_OPER = 2,
	QCA_VENDOR_ELEM_RAPS = 3,
	QCA_VENDOR_ELEM_MU_EDCA_PARAMS = 4,
	QCA_VENDOR_ELEM_BSS_COLOR_CHANGE = 5,
};

/**
 * enum qca_vendor_attr_get_tsf: Vendor attributes for TSF capture
 * @QCA_WLAN_VENDOR_ATTR_TSF_INVALID: Invalid attribute value
 * @QCA_WLAN_VENDOR_ATTR_TSF_CMD: enum qca_tsf_operation (u32)
 * @QCA_WLAN_VENDOR_ATTR_TSF_TIMER_VALUE: Unsigned 64 bit TSF timer value
 * @QCA_WLAN_VENDOR_ATTR_TSF_SOC_TIMER_VALUE: Unsigned 64 bit Synchronized
 *	SOC timer value at TSF capture
 * @QCA_WLAN_VENDOR_ATTR_TSF_AFTER_LAST: after last
 * @QCA_WLAN_VENDOR_ATTR_TSF_MAX: Max value
 */
enum qca_vendor_attr_tsf_cmd {
	QCA_WLAN_VENDOR_ATTR_TSF_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_TSF_CMD,
	QCA_WLAN_VENDOR_ATTR_TSF_TIMER_VALUE,
	QCA_WLAN_VENDOR_ATTR_TSF_SOC_TIMER_VALUE,
	QCA_WLAN_VENDOR_ATTR_TSF_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_TSF_MAX =
		QCA_WLAN_VENDOR_ATTR_TSF_AFTER_LAST - 1
};

/**
 * enum qca_tsf_operation: TSF driver commands
 * @QCA_TSF_CAPTURE: Initiate TSF Capture
 * @QCA_TSF_GET: Get TSF capture value
 * @QCA_TSF_SYNC_GET: Initiate TSF capture and return with captured value
 */
enum qca_tsf_cmd {
	QCA_TSF_CAPTURE,
	QCA_TSF_GET,
	QCA_TSF_SYNC_GET,
};

/**
 * enum qca_vendor_attr_get_preferred_freq_list - get preferred channel list
 * @QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_INVALID: invalid value
 * @QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_IFACE_TYPE: interface type
 * @QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST: preferred frequency list
 * @QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_WEIGHED_PCL: pcl with weight
 * @QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_AFTER_LAST: after last
 * @QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_MAX: max
 */
enum qca_vendor_attr_get_preferred_freq_list {
	QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_INVALID,
	/* A 32-unsigned value; the interface type/mode for which the preferred
	 * frequency list is requested (see enum qca_iface_type for possible
	 * values); used in both south- and north-bound.
	 */
	QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_IFACE_TYPE,
	/* An array of 32-unsigned values; values are frequency (MHz); used
	 * in north-bound only.
	 */
	QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST,
	/* An array of nested values as per enum
	 *  qca_wlan_vendor_attr_pcl_config attribute.
	 *  Each element contains frequency (MHz), weight, and flag
	 *  bit mask indicating how the frequency should be used in P2P
	 *  negotiation.
	 */
	QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_WEIGHED_PCL,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_MAX =
		QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_AFTER_LAST - 1
};

/**
 * enum qca_vendor_attr_probable_oper_channel - channel hint
 * @QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_INVALID: invalid value
 * @QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_IFACE_TYPE: interface type
 * @QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_FREQ: frequency hint value
 * @QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_AFTER_LAST: last
 * @QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_MAX: max
 */
enum qca_vendor_attr_probable_oper_channel {
	QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_INVALID,
	/* 32-bit unsigned value; indicates the connection/iface type likely to
	 * come on this channel (see enum qca_iface_type).
	 */
	QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_IFACE_TYPE,
	/* 32-bit unsigned value; the frequency (MHz) of the probable channel */
	QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_FREQ,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_MAX =
		QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_AFTER_LAST - 1
};

/**
 * enum qca_wlan_vendor_attr_gw_param_config - gateway param config
 * @QCA_WLAN_VENDOR_ATTR_GW_PARAM_CONFIG_INVALID: Invalid
 * @QCA_WLAN_VENDOR_ATTR_GW_PARAM_CONFIG_GW_MAC_ADDR: gateway mac addr
 * @QCA_WLAN_VENDOR_ATTR_GW_PARAM_CONFIG_IPV4_ADDR: ipv4 addr
 * @QCA_WLAN_VENDOR_ATTR_GW_PARAM_CONFIG_IPV6_ADDR: ipv6 addr
 */
enum qca_wlan_vendor_attr_gw_param_config {
	QCA_WLAN_VENDOR_ATTR_GW_PARAM_CONFIG_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_GW_PARAM_CONFIG_GW_MAC_ADDR,
	QCA_WLAN_VENDOR_ATTR_GW_PARAM_CONFIG_IPV4_ADDR,
	QCA_WLAN_VENDOR_ATTR_GW_PARAM_CONFIG_IPV6_ADDR,
	QCA_WLAN_VENDOR_ATTR_GW_PARAM_CONFIG_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_GW_PARAM_CONFIG_MAX =
		QCA_WLAN_VENDOR_ATTR_GW_PARAM_CONFIG_AFTER_LAST - 1,
};

/**
 * enum drv_dbs_capability - DBS capability
 * @DRV_DBS_CAPABILITY_DISABLED: DBS disabled
 * @DRV_DBS_CAPABILITY_1X1: 1x1
 * @DRV_DBS_CAPABILITY_2X2: 2x2
 */
enum drv_dbs_capability {
	DRV_DBS_CAPABILITY_DISABLED, /* not supported or disabled */
	DRV_DBS_CAPABILITY_1X1,
	DRV_DBS_CAPABILITY_2X2,
};

/**
 * enum qca_vendor_attr_txpower_decr_db - Attributes for TX power decrease
 *
 * These attributes are used with QCA_NL80211_VENDOR_SUBCMD_SET_TXPOWER_DECR_DB.
 */
enum qca_vendor_attr_txpower_decr_db {
	QCA_WLAN_VENDOR_ATTR_TXPOWER_DECR_DB_INVALID,
	/*
	 * 8-bit unsigned value to indicate the reduction of TX power in dB for
	 * a virtual interface.
	 */
	QCA_WLAN_VENDOR_ATTR_TXPOWER_DECR_DB,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_TXPOWER_DECR_DB_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_TXPOWER_DECR_DB_MAX =
	QCA_WLAN_VENDOR_ATTR_TXPOWER_DECR_DB_AFTER_LAST - 1
};

/**
 * enum qca_vendor_attr_ota_test - Enable OTA test
 * @QCA_WLAN_VENDOR_ATTR_OTA_TEST_INVALID: invalid value
 * @QCA_WLAN_VENDOR_ATTR_OTA_TEST_ENABLE: enable OTA test
 * @QCA_WLAN_VENDOR_ATTR_OTA_TEST_AFTER_LAST: after last
 * @QCA_WLAN_VENDOR_ATTR_OTA_TEST_MAX: max
 */
enum qca_vendor_attr_ota_test {
	QCA_WLAN_VENDOR_ATTR_OTA_TEST_INVALID,
	/* 8-bit unsigned value to indicate if OTA test is enabled */
	QCA_WLAN_VENDOR_ATTR_OTA_TEST_ENABLE,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_OTA_TEST_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_OTA_TEST_MAX =
		QCA_WLAN_VENDOR_ATTR_OTA_TEST_AFTER_LAST - 1
};

/** enum qca_vendor_attr_txpower_scale - vendor sub commands index
 * @QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_INVALID: invalid value
 * @QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE: scaling value
 * @QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_AFTER_LAST: last value
 * @QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_MAX: max value
 */
enum qca_vendor_attr_txpower_scale {
	QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_INVALID,
	/* 8-bit unsigned value to indicate the scaling of tx power */
	QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_MAX =
		QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_AFTER_LAST - 1
};

/**
 * enum qca_vendor_attr_txpower_scale_decr_db - vendor sub commands index
 * @QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_DECR_DB_INVALID: invalid value
 * @QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_DECR_DB: scaling value
 * @QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_DECR_DB_AFTER_LAST: last value
 * @QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_DECR_DB_MAX: max value
 */
enum qca_vendor_attr_txpower_scale_decr_db {
	QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_DECR_DB_INVALID,
	/* 8-bit unsigned value to indicate the scaling of tx power */
	QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_DECR_DB,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_DECR_DB_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_DECR_DB_MAX =
		QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_DECR_DB_AFTER_LAST - 1
};

/**
 * enum dfs_mode - state of DFS mode
 * @DFS_MODE_NONE: DFS mode attribute is none
 * @DFS_MODE_ENABLE:  DFS mode is enabled
 * @DFS_MODE_DISABLE: DFS mode is disabled
 * @DFS_MODE_DEPRIORITIZE: Deprioritize DFS channels in scanning
 */
enum dfs_mode {
	DFS_MODE_NONE,
	DFS_MODE_ENABLE,
	DFS_MODE_DISABLE,
	DFS_MODE_DEPRIORITIZE
};

/**
 * enum qca_wlan_vendor_attr_acs_config - Defines Configuration attributes
 * used by the vendor command QCA_NL80211_VENDOR_SUBCMD_ACS_POLICY.
 *
 * @QCA_WLAN_VENDOR_ATTR_ACS_DFS_MODE: Required (u8)
 * DFS mode for ACS operation from enum qca_acs_dfs_mode.
 *
 * @QCA_WLAN_VENDOR_ATTR_ACS_CHANNEL_HINT: Required (u8)
 * channel number hint for ACS operation, if valid channel is specified then
 * ACS operation gives priority to this channel.
 * Note: If both the driver and user space application supports the 6 GHz band,
 * this attribute is deprecated and QCA_WLAN_VENDOR_ATTR_ACS_FREQUENCY_HINT
 * should be used.
 * To maintain backward compatibility, QCA_WLAN_VENDOR_ATTR_ACS_CHANNEL_HINT
 * is still used if either of the driver or user space application doesn't
 * support the 6 GHz band.
 *
 * @QCA_WLAN_VENDOR_ATTR_ACS_FREQUENCY_HINT: Required (u32).
 * Channel center frequency (MHz) hint for ACS operation, if a valid center
 * frequency is specified, ACS operation gives priority to this channel.
 */
enum qca_wlan_vendor_attr_acs_config {
	QCA_WLAN_VENDOR_ATTR_ACS_MODE_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_ACS_DFS_MODE = 1,
	QCA_WLAN_VENDOR_ATTR_ACS_CHANNEL_HINT = 2,
	QCA_WLAN_VENDOR_ATTR_ACS_FREQUENCY_HINT = 3,

	QCA_WLAN_VENDOR_ATTR_ACS_DFS_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_ACS_DFS_MAX =
		QCA_WLAN_VENDOR_ATTR_ACS_DFS_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_get_hw_capability - Wi-Fi hardware capability
 */
enum qca_wlan_vendor_attr_get_hw_capability {
	QCA_WLAN_VENDOR_ATTR_HW_CAPABILITY_INVALID,
	/*
	 * Antenna isolation
	 * An attribute used in the response.
	 * The content of this attribute is encoded in a byte array. Each byte
	 * value is an antenna isolation value. The array length is the number
	 * of antennas.
	 */
	QCA_WLAN_VENDOR_ATTR_ANTENNA_ISOLATION,
	/*
	 * Request HW capability
	 * An attribute used in the request.
	 * The content of this attribute is a u32 array for one or more of
	 * hardware capabilities (attribute IDs) that are being requested. Each
	 * u32 value has a value from this
	 * enum qca_wlan_vendor_attr_get_hw_capability
	 * identifying which capabilities are requested.
	 */
	QCA_WLAN_VENDOR_ATTR_GET_HW_CAPABILITY,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_HW_CAPABILITY_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_HW_CAPABILITY_MAX =
	QCA_WLAN_VENDOR_ATTR_HW_CAPABILITY_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_sta_connect_roam_policy_config -
 *                        config params for sta roam policy
 * @QCA_WLAN_VENDOR_ATTR_STA_CONNECT_ROAM_POLICY_INVALID: Invalid
 * @QCA_WLAN_VENDOR_ATTR_STA_DFS_MODE: If sta should skip Dfs channels
 * @QCA_WLAN_VENDOR_ATTR_STA_SKIP_UNSAFE_CHANNEL:
 * If sta should skip unsafe channels or not in scanning
 * @QCA_WLAN_VENDOR_ATTR_STA_CONNECT_ROAM_POLICY_LAST:
 * @QCA_WLAN_VENDOR_ATTR_STA_CONNECT_ROAM_POLICY_MAX: max attribute
 */
enum qca_wlan_vendor_attr_sta_connect_roam_policy_config {
	QCA_WLAN_VENDOR_ATTR_STA_CONNECT_ROAM_POLICY_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_STA_DFS_MODE,
	QCA_WLAN_VENDOR_ATTR_STA_SKIP_UNSAFE_CHANNEL,

	QCA_WLAN_VENDOR_ATTR_STA_CONNECT_ROAM_POLICY_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_STA_CONNECT_ROAM_POLICY_MAX =
	QCA_WLAN_VENDOR_ATTR_STA_CONNECT_ROAM_POLICY_AFTER_LAST - 1,
};

/* Attributes for FTM commands and events */

/**
 * enum qca_wlan_vendor_attr_loc_capa - Indoor location capabilities
 *
 * @QCA_WLAN_VENDOR_ATTR_LOC_CAPA_FLAGS: Various flags. See
 *      enum qca_wlan_vendor_attr_loc_capa_flags.
 * @QCA_WLAN_VENDOR_ATTR_FTM_CAPA_MAX_NUM_SESSIONS: Maximum number
 *      of measurement sessions that can run concurrently.
 *      Default is one session (no session concurrency).
 * @QCA_WLAN_VENDOR_ATTR_FTM_CAPA_MAX_NUM_PEERS: The total number of unique
 *      peers that are supported in running sessions. For example,
 *      if the value is 8 and maximum number of sessions is 2, you can
 *      have one session with 8 unique peers, or 2 sessions with 4 unique
 *      peers each, and so on.
 * @QCA_WLAN_VENDOR_ATTR_FTM_CAPA_MAX_NUM_BURSTS_EXP: Maximum number
 *      of bursts per peer, as an exponent (2^value). Default is 0,
 *      meaning no multi-burst support.
 * @QCA_WLAN_VENDOR_ATTR_FTM_CAPA_MAX_MEAS_PER_BURST: Maximum number
 *      of measurement exchanges allowed in a single burst.
 * @QCA_WLAN_VENDOR_ATTR_AOA_CAPA_SUPPORTED_TYPES: Supported AOA measurement
 *      types. A bit mask (unsigned 32 bit value), each bit corresponds
 *      to an AOA type as defined by enum qca_vendor_attr_aoa_type.
 */
enum qca_wlan_vendor_attr_loc_capa {
	QCA_WLAN_VENDOR_ATTR_LOC_CAPA_INVALID,
	QCA_WLAN_VENDOR_ATTR_LOC_CAPA_FLAGS,
	QCA_WLAN_VENDOR_ATTR_FTM_CAPA_MAX_NUM_SESSIONS,
	QCA_WLAN_VENDOR_ATTR_FTM_CAPA_MAX_NUM_PEERS,
	QCA_WLAN_VENDOR_ATTR_FTM_CAPA_MAX_NUM_BURSTS_EXP,
	QCA_WLAN_VENDOR_ATTR_FTM_CAPA_MAX_MEAS_PER_BURST,
	QCA_WLAN_VENDOR_ATTR_AOA_CAPA_SUPPORTED_TYPES,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_LOC_CAPA_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_LOC_CAPA_MAX =
	QCA_WLAN_VENDOR_ATTR_LOC_CAPA_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_loc_capa_flags: Indoor location capability flags
 *
 * @QCA_WLAN_VENDOR_ATTR_LOC_CAPA_FLAG_FTM_RESPONDER: Set if driver
 *      can be configured as an FTM responder (for example, an AP that
 *      services FTM requests). QCA_NL80211_VENDOR_SUBCMD_FTM_CFG_RESPONDER
 *      will be supported if set.
 * @QCA_WLAN_VENDOR_ATTR_LOC_CAPA_FLAG_FTM_INITIATOR: Set if driver
 *      can run FTM sessions. QCA_NL80211_VENDOR_SUBCMD_FTM_START_SESSION
 *      will be supported if set.
* @QCA_WLAN_VENDOR_ATTR_LOC_CAPA_FLAG_ASAP: Set if FTM responder
 *      supports immediate (ASAP) response.
 * @QCA_WLAN_VENDOR_ATTR_LOC_CAPA_FLAG_AOA: Set if driver supports standalone
 *      AOA measurement using QCA_NL80211_VENDOR_SUBCMD_AOA_MEAS.
 * @QCA_WLAN_VENDOR_ATTR_LOC_CAPA_FLAG_AOA_IN_FTM: Set if driver supports
 *      requesting AOA measurements as part of an FTM session.
 */
enum qca_wlan_vendor_attr_loc_capa_flags {
	QCA_WLAN_VENDOR_ATTR_LOC_CAPA_FLAG_FTM_RESPONDER = 1 << 0,
	QCA_WLAN_VENDOR_ATTR_LOC_CAPA_FLAG_FTM_INITIATOR = 1 << 1,
	QCA_WLAN_VENDOR_ATTR_LOC_CAPA_FLAG_ASAP = 1 << 2,
	QCA_WLAN_VENDOR_ATTR_LOC_CAPA_FLAG_AOA = 1 << 3,
	QCA_WLAN_VENDOR_ATTR_LOC_CAPA_FLAG_AOA_IN_FTM = 1 << 4,
};

/**
 * enum qca_wlan_vendor_attr_sap_config - Parameters for AP configuration
 *
 * @QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_CHANNEL: Optional (u8)
 * Channel number on which Access Point should restart.
 * Note: If both the driver and user space application supports the 6 GHz band,
 * this attribute is deprecated and QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_FREQUENCY
 * should be used.
 * To maintain backward compatibility, QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_CHANNEL
 * is still used if either of the driver or user space application doesn't
 * support the 6 GHz band.
 *
 * @QCA_WLAN_VENDOR_ATTR_SAP_MANDATORY_FREQUENCY_LIST: Required
 * Nested attribute to hold list of center frequencies on which AP is
 * expected to operate. This is irrespective of ACS configuration.
 * This list is a priority based one and is looked for before the AP is
 * created to ensure the best concurrency sessions (avoid MCC and use DBS/SCC)
 * co-exist in the system.
 *
 * @QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_FREQUENCY: Optional (u32)
 * Channel center frequency (MHz) on which the access point should restart.
 */
enum qca_wlan_vendor_attr_sap_config {
	QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_CHANNEL = 1,
	QCA_WLAN_VENDOR_ATTR_SAP_MANDATORY_FREQUENCY_LIST = 2,
	QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_FREQUENCY = 3,

	/* Keep last */
	QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_MAX =
	QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_ftm_peer_info: Information about
 * a single peer in a measurement session.
 *
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_MAC_ADDR: The MAC address of the peer.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_MEAS_FLAGS: Various flags related
 *      to measurement. See enum qca_wlan_vendor_attr_ftm_peer_meas_flags.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_MEAS_PARAMS: Nested attribute of
 *      FTM measurement parameters, as specified by IEEE P802.11-REVmc/D7.0
 *      9.4.2.167. See enum qca_wlan_vendor_attr_ftm_meas_param for
 *      list of supported attributes.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_SECURE_TOKEN_ID: Initial token ID for
 *      secure measurement.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_AOA_BURST_PERIOD: Request AOA
 *      measurement every <value> bursts. If 0 or not specified,
 *      AOA measurements will be disabled for this peer.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_FREQ: Frequency in MHz where
 *      the measurement frames are exchanged. Optional; if not
 *      specified, try to locate the peer in the kernel scan
 *      results cache and use frequency from there.
 */
enum qca_wlan_vendor_attr_ftm_peer_info {
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_INVALID,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_MAC_ADDR,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_MEAS_FLAGS,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_MEAS_PARAMS,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_SECURE_TOKEN_ID,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_AOA_BURST_PERIOD,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_FREQ,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_MAX =
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_ftm_peer_meas_flags: Measurement request flags,
 *      per-peer
 *
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_MEAS_FLAG_ASAP: If set, request
 *      immediate (ASAP) response from peer.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_MEAS_FLAG_LCI: If set, request
 *      LCI report from peer. The LCI report includes the absolute
 *      location of the peer in "official" coordinates (similar to GPS).
 *      See IEEE P802.11-REVmc/D7.0, 11.24.6.7 for more information.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_MEAS_FLAG_LCR: If set, request
 *      Location civic report from peer. The LCR includes the location
 *      of the peer in free-form format. See IEEE P802.11-REVmc/D7.0,
 *      11.24.6.7 for more information.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_MEAS_FLAG_SECURE: If set,
 *      request a secure measurement.
 *      QCA_WLAN_VENDOR_ATTR_FTM_PEER_SECURE_TOKEN_ID must also be provided.
 */
enum qca_wlan_vendor_attr_ftm_peer_meas_flags {
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_MEAS_FLAG_ASAP    = 1 << 0,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_MEAS_FLAG_LCI     = 1 << 1,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_MEAS_FLAG_LCR     = 1 << 2,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_MEAS_FLAG_SECURE  = 1 << 3,
};

/**
 * enum qca_wlan_vendor_attr_ftm_meas_param: Measurement parameters
 *
 * @QCA_WLAN_VENDOR_ATTR_FTM_PARAM_MEAS_PER_BURST: Number of measurements
 *      to perform in a single burst.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PARAM_NUM_BURSTS_EXP: Number of bursts to
 *      perform, specified as an exponent (2^value).
 * @QCA_WLAN_VENDOR_ATTR_FTM_PARAM_BURST_DURATION: Duration of burst
 *      instance, as specified in IEEE P802.11-REVmc/D7.0, 9.4.2.167.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PARAM_BURST_PERIOD: Time between bursts,
 *      as specified in IEEE P802.11-REVmc/D7.0, 9.4.2.167. Must
 *      be larger than QCA_WLAN_VENDOR_ATTR_FTM_PARAM_BURST_DURATION.
 */
enum qca_wlan_vendor_attr_ftm_meas_param {
	QCA_WLAN_VENDOR_ATTR_FTM_PARAM_INVALID,
	QCA_WLAN_VENDOR_ATTR_FTM_PARAM_MEAS_PER_BURST,
	QCA_WLAN_VENDOR_ATTR_FTM_PARAM_NUM_BURSTS_EXP,
	QCA_WLAN_VENDOR_ATTR_FTM_PARAM_BURST_DURATION,
	QCA_WLAN_VENDOR_ATTR_FTM_PARAM_BURST_PERIOD,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_FTM_PARAM_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_FTM_PARAM_MAX =
	QCA_WLAN_VENDOR_ATTR_FTM_PARAM_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_ftm_peer_result: Per-peer results
 *
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_MAC_ADDR: MAC address of the reported
 *       peer.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_STATUS: Status of measurement
 *      request for this peer.
 *      See enum qca_wlan_vendor_attr_ftm_peer_result_status.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_FLAGS: Various flags related
 *      to measurement results for this peer.
 *      See enum qca_wlan_vendor_attr_ftm_peer_result_flags.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_VALUE_SECONDS: Specified when
 *      request failed and peer requested not to send an additional request
 *      for this number of seconds.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_LCI: LCI report when received
 *      from peer. In the format specified by IEEE P802.11-REVmc/D7.0,
 *      9.4.2.22.10.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_LCR: Location civic report when
 *      received from peer. In the format specified by IEEE P802.11-REVmc/D7.0,
 *      9.4.2.22.13.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_MEAS_PARAMS: Reported when peer
 *      overridden some measurement request parameters. See
 *      enum qca_wlan_vendor_attr_ftm_meas_param.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_AOA_MEAS: AOA measurement
 *      for this peer. Same contents as @QCA_WLAN_VENDOR_ATTR_AOA_MEAS_RESULT.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_MEAS: Array of measurement
 *      results. Each entry is a nested attribute defined
 *      by enum qca_wlan_vendor_attr_ftm_meas.
 */
enum qca_wlan_vendor_attr_ftm_peer_result {
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_INVALID,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_MAC_ADDR,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_STATUS,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_FLAGS,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_VALUE_SECONDS,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_LCI,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_LCR,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_MEAS_PARAMS,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_AOA_MEAS,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_MEAS,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_MAX =
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_ftm_peer_result_status
 *
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_STATUS_OK: Request sent ok and results
 *      will be provided. Peer may have overridden some measurement parameters,
 *      in which case overridden parameters will be report by
 *      QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_MEAS_PARAM attribute.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_STATUS_INCAPABLE: Peer is incapable
 *      of performing the measurement request. No more results will be sent
 *      for this peer in this session.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_STATUS_FAILED: Peer reported request
 *      failed, and requested not to send an additional request for number
 *      of seconds specified by QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_VALUE_SECONDS
 *      attribute.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_STATUS_INVALID: Request validation
 *      failed. Request was not sent over the air.
 */
enum qca_wlan_vendor_attr_ftm_peer_result_status {
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_STATUS_OK,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_STATUS_INCAPABLE,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_STATUS_FAILED,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_STATUS_INVALID,
};

/**
 * enum qca_wlan_vendor_attr_ftm_peer_result_flags: Various flags
 *  for measurement result, per-peer
 *
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_FLAG_DONE: If set,
 *      measurement completed for this peer. No more results will be reported
 *      for this peer in this session.
 */
enum qca_wlan_vendor_attr_ftm_peer_result_flags {
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_FLAG_DONE = 1 << 0,
};

/**
 * enum qca_vendor_attr_loc_session_status: Session completion status code
 *
 * @QCA_WLAN_VENDOR_ATTR_LOC_SESSION_STATUS_OK: Session completed
 *      successfully.
 * @QCA_WLAN_VENDOR_ATTR_LOC_SESSION_STATUS_ABORTED: Session aborted
 *      by request.
 * @QCA_WLAN_VENDOR_ATTR_LOC_SESSION_STATUS_INVALID: Session request
 *      was invalid and was not started.
 * @QCA_WLAN_VENDOR_ATTR_LOC_SESSION_STATUS_FAILED: Session had an error
 *      and did not complete normally (for example out of resources).
 */
enum qca_vendor_attr_loc_session_status {
	QCA_WLAN_VENDOR_ATTR_LOC_SESSION_STATUS_OK,
	QCA_WLAN_VENDOR_ATTR_LOC_SESSION_STATUS_ABORTED,
	QCA_WLAN_VENDOR_ATTR_LOC_SESSION_STATUS_INVALID,
	QCA_WLAN_VENDOR_ATTR_LOC_SESSION_STATUS_FAILED,
};

/**
 * enum qca_wlan_vendor_attr_ftm_meas: Single measurement data
 *
 * @QCA_WLAN_VENDOR_ATTR_FTM_MEAS_T1: Time of departure (TOD) of FTM packet as
 *      recorded by responder, in picoseconds.
 *      See IEEE P802.11-REVmc/D7.0, 11.24.6.4 for more information.
 * @QCA_WLAN_VENDOR_ATTR_FTM_MEAS_T2: Time of arrival (TOA) of FTM packet at
 *      initiator, in picoseconds.
 *      See IEEE P802.11-REVmc/D7.0, 11.24.6.4 for more information.
 * @QCA_WLAN_VENDOR_ATTR_FTM_MEAS_T3: TOD of ACK packet as recorded by
 *      initiator, in picoseconds.
 *      See IEEE P802.11-REVmc/D7.0, 11.24.6.4 for more information.
 * @QCA_WLAN_VENDOR_ATTR_FTM_MEAS_T4: TOA of ACK packet at
 *      responder, in picoseconds.
 *      See IEEE P802.11-REVmc/D7.0, 11.24.6.4 for more information.
 * @QCA_WLAN_VENDOR_ATTR_FTM_MEAS_RSSI: RSSI (signal level) as recorded
 *      during this measurement exchange. Optional and will be provided if
 *      the hardware can measure it.
 * @QCA_WLAN_VENDOR_ATTR_FTM_MEAS_TOD_ERR: TOD error reported by
 *      responder. Not always provided.
 *      See IEEE P802.11-REVmc/D7.0, 9.6.8.33 for more information.
 * @QCA_WLAN_VENDOR_ATTR_FTM_MEAS_TOA_ERR: TOA error reported by
 *      responder. Not always provided.
 *      See IEEE P802.11-REVmc/D7.0, 9.6.8.33 for more information.
 * @QCA_WLAN_VENDOR_ATTR_FTM_MEAS_INITIATOR_TOD_ERR: TOD error measured by
 *      initiator. Not always provided.
 *      See IEEE P802.11-REVmc/D7.0, 9.6.8.33 for more information.
 * @QCA_WLAN_VENDOR_ATTR_FTM_MEAS_INITIATOR_TOA_ERR: TOA error measured by
 *      initiator. Not always provided.
 *      See IEEE P802.11-REVmc/D7.0, 9.6.8.33 for more information.
 * @QCA_WLAN_VENDOR_ATTR_FTM_MEAS_PAD: Dummy attribute for padding.
 */
enum qca_wlan_vendor_attr_ftm_meas {
	QCA_WLAN_VENDOR_ATTR_FTM_MEAS_INVALID,
	QCA_WLAN_VENDOR_ATTR_FTM_MEAS_T1,
	QCA_WLAN_VENDOR_ATTR_FTM_MEAS_T2,
	QCA_WLAN_VENDOR_ATTR_FTM_MEAS_T3,
	QCA_WLAN_VENDOR_ATTR_FTM_MEAS_T4,
	QCA_WLAN_VENDOR_ATTR_FTM_MEAS_RSSI,
	QCA_WLAN_VENDOR_ATTR_FTM_MEAS_TOD_ERR,
	QCA_WLAN_VENDOR_ATTR_FTM_MEAS_TOA_ERR,
	QCA_WLAN_VENDOR_ATTR_FTM_MEAS_INITIATOR_TOD_ERR,
	QCA_WLAN_VENDOR_ATTR_FTM_MEAS_INITIATOR_TOA_ERR,
	QCA_WLAN_VENDOR_ATTR_FTM_MEAS_PAD,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_FTM_MEAS_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_FTM_MEAS_MAX =
	QCA_WLAN_VENDOR_ATTR_FTM_MEAS_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_p2p_listen_offload - vendor sub commands index
 * @QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_INVALID: invalid value
 * @QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_CHANNEL:
 *     A 32-bit unsigned value; the P2P listen frequency (MHz); must be one
 *     of the social channels.
 * @QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_PERIOD: listen offload period
 *     A 32-bit unsigned value; the P2P listen offload period (ms).
 * @QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_INTERVAL:
 *     A 32-bit unsigned value; the P2P listen interval duration (ms).
 * @QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_COUNT:
 *     A 32-bit unsigned value; number of interval times the Firmware needs
 *     to run the offloaded P2P listen operation before it stops.
 * @QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_DEVICE_TYPES: device types
 *     An array of unsigned 8-bit characters; vendor information elements.
 * @QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_VENDOR_IE: vendor IEs
 * @QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_CTRL_FLAG: control flag for FW
 *     A 32-bit unsigned value; a control flag to indicate whether listen
 *     results need to be flushed to wpa_supplicant.
 * @QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_STOP_REASON: offload stop reason
 *     A 8-bit unsigned value; reason code for P2P listen offload stop
 *     event.
 * @QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_AFTER_LAST: last value
 * @QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_MAX: max value
 */
enum qca_wlan_vendor_attr_p2p_listen_offload {
	QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_CHANNEL,
	QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_PERIOD,
	QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_INTERVAL,
	QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_COUNT,
	QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_DEVICE_TYPES,
	QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_VENDOR_IE,
	QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_CTRL_FLAG,
	QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_STOP_REASON,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_MAX =
	QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_AFTER_LAST - 1
};

/**
 * enum qca_wlan_vendor_drv_info - WLAN driver info
 * @QCA_WLAN_VENDOR_ATTR_DRV_INFO_INVALID: Invalid
 * @QCA_WLAN_VENDOR_ATTR_DRV_INFO_BUS_SIZE: Maximum Message size info
 * between Firmware & Host.
 */
enum qca_wlan_vendor_drv_info {
	QCA_WLAN_VENDOR_ATTR_DRV_INFO_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_DRV_INFO_BUS_SIZE,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_DRV_INFO_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_DRV_INFO_MAX =
		QCA_WLAN_VENDOR_ATTR_DRV_INFO_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_aoa_type - AOA measurement type
 *
 * @QCA_WLAN_VENDOR_ATTR_AOA_TYPE_TOP_CIR_PHASE: Phase of the strongest
 *      CIR (channel impulse response) path for each antenna.
 * @QCA_WLAN_VENDOR_ATTR_AOA_TYPE_TOP_CIR_PHASE_AMP: Phase and amplitude
 *      of the strongest CIR path for each antenna.
 */
enum qca_wlan_vendor_attr_aoa_type {
	QCA_WLAN_VENDOR_ATTR_AOA_TYPE_TOP_CIR_PHASE,
	QCA_WLAN_VENDOR_ATTR_AOA_TYPE_TOP_CIR_PHASE_AMP,
	QCA_WLAN_VENDOR_ATTR_AOA_TYPE_MAX
};

/**
 * enum qca_wlan_vendor_attr_encryption_test - Attributes to
 * validate encryption engine
 *
 * @QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_NEEDS_DECRYPTION: Flag attribute.
 *    This will be included if the request is for decryption; if not included,
 *    the request is treated as a request for encryption by default.
 * @QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_CIPHER: Unsigned 32-bit value
 *    indicating the key cipher suite. Takes same values as
 *    NL80211_ATTR_KEY_CIPHER.
 * @QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_KEYID: Unsigned 8-bit value
 *    Key Id to be used for encryption
 * @QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_TK: Array of 8-bit values.
 *    Key (TK) to be used for encryption/decryption
 * @QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_PN: Array of 8-bit values.
 *    Packet number to be specified for encryption/decryption
 *    6 bytes for TKIP/CCMP/GCMP.
 * @QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_DATA: Array of 8-bit values
 *    representing the 802.11 packet (header + payload + FCS) that
 *    needs to be encrypted/decrypted.
 *    Encrypted/decrypted response from the driver will also be sent
 *    to userspace with the same attribute.
 */
enum qca_wlan_vendor_attr_encryption_test {
	QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_NEEDS_DECRYPTION,
	QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_CIPHER,
	QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_KEYID,
	QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_TK,
	QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_PN,
	QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_DATA,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_MAX =
		QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_AFTER_LAST - 1
};

/**
 * enum qca_wlan_vendor_attr_dmg_rf_sector_type - Type of
 * sector for DMG RF sector operations.
 *
 * @QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_TYPE_RX: RX sector
 * @QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_TYPE_TX: TX sector
 */
enum qca_wlan_vendor_attr_dmg_rf_sector_type {
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_TYPE_RX,
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_TYPE_TX,
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_TYPE_MAX
};

/**
 * enum qca_wlan_vendor_attr_fw_state - State of firmware
 *
 * @QCA_WLAN_VENDOR_ATTR_FW_STATE_ERROR: FW is in bad state
 * @QCA_WLAN_VENDOR_ATTR_FW_STATE_ACTIVE: FW is active
 */
enum qca_wlan_vendor_attr_fw_state {
	QCA_WLAN_VENDOR_ATTR_FW_STATE_ERROR,
	QCA_WLAN_VENDOR_ATTR_FW_STATE_ACTIVE,
	QCA_WLAN_VENDOR_ATTR_FW_STATE_MAX
};

/**
 * BRP antenna limit mode
 *
 * @QCA_WLAN_VENDOR_ATTR_BRP_ANT_LIMIT_MODE_DISABLE: Disable BRP force
 *      antenna limit, BRP will be performed as usual.
 * @QCA_WLAN_VENDOR_ATTR_BRP_ANT_LIMIT_MODE_EFFECTIVE: Define maximal
 *      antennas limit. the hardware may use less antennas than the
 *      maximum limit.
 * @QCA_WLAN_VENDOR_ATTR_BRP_ANT_LIMIT_MODE_FORCE: The hardware will
 *      use exactly the specified number of antennas for BRP.
 */
enum qca_wlan_vendor_attr_brp_ant_limit_mode {
	QCA_WLAN_VENDOR_ATTR_BRP_ANT_LIMIT_MODE_DISABLE,
	QCA_WLAN_VENDOR_ATTR_BRP_ANT_LIMIT_MODE_EFFECTIVE,
	QCA_WLAN_VENDOR_ATTR_BRP_ANT_LIMIT_MODE_FORCE,
	QCA_WLAN_VENDOR_ATTR_BRP_ANT_LIMIT_MODE_MAX
};

/**
 * enum qca_wlan_vendor_attr_dmg_rf_sector_cfg - Attributes for
 * DMG RF sector configuration for a single RF module.
 * The values are defined in a compact way which closely matches
 * the way it is stored in HW registers.
 * The configuration provides values for 32 antennas and 8 distribution
 * amplifiers, and together describes the characteristics of the RF
 * sector - such as a beam in some direction with some gain.
 *
 * @QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_MODULE_INDEX: Index
 *      of RF module for this configuration.
 * @QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_ETYPE0: Bit 0 of edge
 *      amplifier gain index. Unsigned 32 bit number containing
 *      bits for all 32 antennas.
 * @QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_ETYPE1: Bit 1 of edge
 *      amplifier gain index. Unsigned 32 bit number containing
 *      bits for all 32 antennas.
 * @QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_ETYPE2: Bit 2 of edge
 *      amplifier gain index. Unsigned 32 bit number containing
 *      bits for all 32 antennas.
 * @QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_PSH_HI: Phase values
 *      for first 16 antennas, 2 bits per antenna.
 * @QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_PSH_LO: Phase values
 *      for last 16 antennas, 2 bits per antenna.
 * @QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_DTYPE_X16: Contains
 *      DTYPE values (3 bits) for each distribution amplifier, followed
 *      by X16 switch bits for each distribution amplifier. There are
 *      total of 8 distribution amplifiers.
 */
enum qca_wlan_vendor_attr_dmg_rf_sector_cfg {
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_MODULE_INDEX = 1,
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_ETYPE0 = 2,
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_ETYPE1 = 3,
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_ETYPE2 = 4,
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_PSH_HI = 5,
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_PSH_LO = 6,
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_DTYPE_X16 = 7,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_MAX =
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_AFTER_LAST - 1
};

/**
 * enum qca_wlan_vendor_attr_tdls_configuration - Attributes for
 * @QCA_NL80211_VENDOR_SUBCMD_CONFIGURE_TDLS configuration to the host driver.
 *
 * @QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TRIGGER_MODE: Configure the TDLS trigger
 *	mode in the host driver. enum qca_wlan_vendor_tdls_trigger_mode
 *	represents the different TDLS trigger modes.
 * @QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TX_STATS_PERIOD: Duration (u32) within
 *	which QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TX_THRESHOLD number
 *	of packets shall meet the criteria for implicit TDLS setup.
 * @QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TX_THRESHOLD: Number (u32) of Tx/Rx
 *	packets within a duration.
 *	QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TX_STATS_PERIOD to initiate
 *	a TDLS setup.
 * @QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_DISCOVERY_PERIOD: Time (u32) to inititate
 *	a TDLS Discovery to the Peer.
 * @QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_MAX_DISCOVERY_ATTEMPT: Max number (u32) of
 *	discovery attempts to know the TDLS capability of the peer. A peer is
 *	marked as TDLS not capable if there is no response for all the attempts.
 * @QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_IDLE_TIMEOUT: Represents a duration (u32)
 *	within which QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_IDLE_PACKET_THRESHOLD
 *	number of TX / RX frames meet the criteria for TDLS teardown.
 * @QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_IDLE_PACKET_THRESHOLD: Minimum number
 *	(u32) of Tx/Rx packets within a duration
 *	CA_WLAN_VENDOR_ATTR_TDLS_CONFIG_IDLE_TIMEOUT to tear down a TDLS link
 * @QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_SETUP_RSSI_THRESHOLD: Threshold
 *	corresponding to the RSSI of the peer below which a TDLS
 *	setup is triggered.
 * @QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TEARDOWN_RSSI_THRESHOLD: Threshold
 *	corresponding to the RSSI of the peer above which
 *	a TDLS teardown is triggered.
 */
enum qca_wlan_vendor_attr_tdls_configuration {
	QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TRIGGER_MODE = 1,

	/* Attributes configuring the TDLS Implicit Trigger */
	QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TX_STATS_PERIOD = 2,
	QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TX_THRESHOLD = 3,
	QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_DISCOVERY_PERIOD = 4,
	QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_MAX_DISCOVERY_ATTEMPT = 5,
	QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_IDLE_TIMEOUT = 6,
	QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_IDLE_PACKET_THRESHOLD = 7,
	QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_SETUP_RSSI_THRESHOLD = 8,
	QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TEARDOWN_RSSI_THRESHOLD = 9,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_MAX =
		QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_AFTER_LAST - 1
};

/**
 * enum qca_wlan_vendor_tdls_trigger_mode: Represents the TDLS trigger mode in
 *	the driver.
 *
 * The following are the different values for
 * QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TRIGGER_MODE.
 *
 * @QCA_WLAN_VENDOR_TDLS_TRIGGER_MODE_EXPLICIT: The trigger to
 *	initiate/teardown the TDLS connection to a respective peer comes
 *	 from the user space. wpa_supplicant provides the commands
 *	TDLS_SETUP, TDLS_TEARDOWN, TDLS_DISCOVER to do this.
 * @QCA_WLAN_VENDOR_TDLS_TRIGGER_MODE_IMPLICIT: Host driver triggers this TDLS
 *	setup/teardown to the eligible peer once the configured criteria
 *	(such as TX/RX threshold, RSSI) is met. The attributes
 *	in QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_IMPLICIT_PARAMS correspond to
 *	the different configuration criteria for the TDLS trigger from the
 *	host driver.
 * @QCA_WLAN_VENDOR_TDLS_TRIGGER_MODE_EXTERNAL: Enables the driver to trigger
 *	the TDLS setup / teardown through the implicit mode, only to the
 *	configured MAC addresses(wpa_supplicant, with tdls_external_control = 1,
 *	configures the MAC address through TDLS_SETUP/TDLS_TEARDOWN commands).
 *	External mode works on top of the implicit mode, thus the host Driver
 *	is expected to be configured in TDLS Implicit mode too to operate in
 *	External mode. Configuring External mode alone without Implicit
 *	mode is invalid.
 *
 * All the above implementations work as expected only when the host driver
 * advertises the capability WPA_DRIVER_FLAGS_TDLS_EXTERNAL_SETUP -
 * representing that the TDLS message exchange is not internal to the host
 * driver, but depends on wpa_supplicant to do the message exchange.
 */
enum qca_wlan_vendor_tdls_trigger_mode {
	QCA_WLAN_VENDOR_TDLS_TRIGGER_MODE_EXPLICIT = 1 << 0,
	QCA_WLAN_VENDOR_TDLS_TRIGGER_MODE_IMPLICIT = 1 << 1,
	QCA_WLAN_VENDOR_TDLS_TRIGGER_MODE_EXTERNAL = 1 << 2,
};

/**
 * enum qca_vendor_attr_sar_limits_selections - Source of SAR power limits
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_BDF0: Select SAR profile #0
 *	that is hard-coded in the Board Data File (BDF).
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_BDF1: Select SAR profile #1
 *	that is hard-coded in the Board Data File (BDF).
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_BDF2: Select SAR profile #2
 *	that is hard-coded in the Board Data File (BDF).
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_BDF3: Select SAR profile #3
 *	that is hard-coded in the Board Data File (BDF).
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_BDF4: Select SAR profile #4
 *	that is hard-coded in the Board Data File (BDF).
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_NONE: Do not select any
 *	source of SAR power limits, thereby disabling the SAR power
 *	limit feature.
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_USER: Select the SAR power
 *	limits configured by %QCA_NL80211_VENDOR_SUBCMD_SET_SAR.
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_V2_0: Select the SAR power
 *	limits version 2.0 configured by %QCA_NL80211_VENDOR_SUBCMD_SET_SAR.
 *
 * This enumerates the valid set of values that may be supplied for
 * attribute %QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT in an instance of
 * the %QCA_NL80211_VENDOR_SUBCMD_SET_SAR_LIMITS vendor command or in
 * the response to an instance of the
 * %QCA_NL80211_VENDOR_SUBCMD_GET_SAR_LIMITS vendor command.
 */
enum qca_vendor_attr_sar_limits_selections {
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_BDF0 = 0,
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_BDF1 = 1,
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_BDF2 = 2,
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_BDF3 = 3,
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_BDF4 = 4,
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_NONE = 5,
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_USER = 6,
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_V2_0 = 7,
};

/**
 * enum qca_vendor_attr_sar_limits_spec_modulations -
 *	SAR limits specification modulation
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_MODULATION_CCK -
 *	CCK modulation
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_MODULATION_OFDM -
 *	OFDM modulation
 *
 * This enumerates the valid set of values that may be supplied for
 * attribute %QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_MODULATION in an
 * instance of attribute %QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC in an
 * instance of the %QCA_NL80211_VENDOR_SUBCMD_SET_SAR_LIMITS vendor
 * command or in the response to an instance of the
 * %QCA_NL80211_VENDOR_SUBCMD_GET_SAR_LIMITS vendor command.
 */
enum qca_vendor_attr_sar_limits_spec_modulations {
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_MODULATION_CCK = 0,
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_MODULATION_OFDM = 1,
};

/**
 * enum qca_vendor_attr_sar_limits - Attributes for SAR power limits
 *
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SAR_ENABLE: Optional (u32) value to
 *	select which SAR power limit table should be used.  Valid
 *	values are enumerated in enum
 *	%qca_vendor_attr_sar_limits_selections.  The existing SAR
 *	power limit selection is unchanged if this attribute is not
 *	present.
 *
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_NUM_SPECS: Optional (u32) value
 *	which specifies the number of SAR power limit specifications
 *	which will follow.
 *
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC: Nested array of SAR power
 *	limit specifications. The number of specifications is
 *	specified by @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_NUM_SPECS. Each
 *	specification contains a set of
 *	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_* attributes. A
 *	specification is uniquely identified by the attributes
 *	%QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_BAND,
 *	%QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_CHAIN, and
 *	%QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_MODULATION and always
 *	contains as a payload the attribute
 *	%QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_POWER_LIMIT.
 *	%QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_POWER_LIMIT_INDEX.
 *	Either %QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_POWER_LIMIT or
 *	%QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_POWER_LIMIT_INDEX is
 *	needed based upon the value of
 *	%QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SAR_ENABLE.
 *
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_BAND: Optional (u32) value to
 *	indicate for which band this specification applies. Valid
 *	values are enumerated in enum %nl80211_band (although not all
 *	bands may be supported by a given device). If the attribute is
 *	not supplied then the specification will be applied to all
 *	supported bands.
 *
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_CHAIN: Optional (u32) value
 *	to indicate for which antenna chain this specification
 *	applies, i.e. 1 for chain 1, 2 for chain 2, etc. If the
 *	attribute is not supplied then the specification will be
 *	applied to all chains.
 *
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_MODULATION: Optional (u32)
 *	value to indicate for which modulation scheme this
 *	specification applies. Valid values are enumerated in enum
 *	%qca_vendor_attr_sar_limits_spec_modulations. If the attribute
 *	is not supplied then the specification will be applied to all
 *	modulation schemes.
 *
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_POWER_LIMIT: Required (u32)
 *	value to specify the actual power limit value in units of 0.5
 *	dBm (i.e., a value of 11 represents 5.5 dBm).
 *	This is required, when %QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT is
 *	%QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_USER.
 *
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_POWER_LIMIT_INDEX: Required (u32)
 *	value to indicate SAR V2 indices (0 - 11) to select SAR V2 profiles.
 *	This is required, when %QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT is
 *	%QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_V2_0.
 *
 * These attributes are used with %QCA_NL80211_VENDOR_SUBCMD_SET_SAR_LIMITS
 * and %QCA_NL80211_VENDOR_SUBCMD_GET_SAR_LIMITS.
 */
enum qca_vendor_attr_sar_limits {
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SAR_ENABLE = 1,
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_NUM_SPECS = 2,
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC = 3,
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_BAND = 4,
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_CHAIN = 5,
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_MODULATION = 6,
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_POWER_LIMIT = 7,
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_POWER_LIMIT_INDEX = 8,

	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_MAX =
		QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_AFTER_LAST - 1
};

/**
 * enum qca_wlan_vendor_attr_ll_stats_ext - Attributes for MAC layer monitoring
 *    offload which is an extension for LL_STATS.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CFG_PERIOD: Monitoring period. Unit in ms.
 *    If MAC counters do not exceed the threshold, FW will report monitored
 *    link layer counters periodically as this setting. The first report is
 *    always triggered by this timer.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CFG_THRESHOLD: It is a percentage (1-99).
 *    For each MAC layer counter, FW holds two copies. One is the current value.
 *    The other is the last report. Once a current counter's increment is larger
 *    than the threshold, FW will indicate that counter to host even if the
 *    monitoring timer does not expire.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_PS_CHG: Peer STA power state change
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TID: TID of MSDU
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_NUM_MSDU: Count of MSDU with the same
 *    failure code.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_STATUS: TX failure code
 *    1: TX packet discarded
 *    2: No ACK
 *    3: Postpone
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_MAC_ADDRESS: peer MAC address
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_PS_STATE: Peer STA current state
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_GLOBAL: Global threshold.
 *    Threshold for all monitored parameters. If per counter dedicated threshold
 *    is not enabled, this threshold will take effect.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_EVENT_MODE: Indicate what triggers this
 *    event, PERORID_TIMEOUT == 1, THRESH_EXCEED == 0.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_IFACE_ID: interface ID
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_ID: peer ID
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_BITMAP: bitmap for TX counters
 *    Bit0: TX counter unit in MSDU
 *    Bit1: TX counter unit in MPDU
 *    Bit2: TX counter unit in PPDU
 *    Bit3: TX counter unit in byte
 *    Bit4: Dropped MSDUs
 *    Bit5: Dropped Bytes
 *    Bit6: MPDU retry counter
 *    Bit7: MPDU failure counter
 *    Bit8: PPDU failure counter
 *    Bit9: MPDU aggregation counter
 *    Bit10: MCS counter for ACKed MPDUs
 *    Bit11: MCS counter for Failed MPDUs
 *    Bit12: TX Delay counter
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_BITMAP: bitmap for RX counters
 *    Bit0: MAC RX counter unit in MPDU
 *    Bit1: MAC RX counter unit in byte
 *    Bit2: PHY RX counter unit in PPDU
 *    Bit3: PHY RX counter unit in byte
 *    Bit4: Disorder counter
 *    Bit5: Retry counter
 *    Bit6: Duplication counter
 *    Bit7: Discard counter
 *    Bit8: MPDU aggregation size counter
 *    Bit9: MCS counter
 *    Bit10: Peer STA power state change (wake to sleep) counter
 *    Bit11: Peer STA power save counter, total time in PS mode
 *    Bit12: Probe request counter
 *    Bit13: Other management frames counter
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_CCA_BSS_BITMAP: bitmap for CCA
 *    Bit0: Idle time
 *    Bit1: TX time
 *    Bit2: time RX in current bss
 *    Bit3: Out of current bss time
 *    Bit4: Wireless medium busy time
 *    Bit5: RX in bad condition time
 *    Bit6: TX in bad condition time
 *    Bit7: time wlan card not available
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_SIGNAL_BITMAP: bitmap for signal
 *    Bit0: Per channel SNR counter
 *    Bit1: Per channel noise floor counter
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_NUM: number of peers
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_CHANNEL_NUM: number of channels
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_AC_RX_NUM: number of RX stats
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_CCA_BSS: per channel BSS CCA stats
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER: container for per PEER stats
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_MSDU: Number of total TX MSDUs
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_MPDU: Number of total TX MPDUs
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_PPDU: Number of total TX PPDUs
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_BYTES: bytes of TX data
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_DROP: Number of dropped TX packets
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_DROP_BYTES: Bytes dropped
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_RETRY: waiting time without an ACK
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_NO_ACK: number of MPDU not-ACKed
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_NO_BACK: number of PPDU not-ACKed
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_AGGR_NUM:
 *    aggregation stats buffer length
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_SUCC_MCS_NUM: length of mcs stats
 *    buffer for ACKed MPDUs.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_FAIL_MCS_NUM: length of mcs stats
 *    buffer for failed MPDUs.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_DELAY_ARRAY_SIZE:
 *    length of delay stats array.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_AGGR: TX aggregation stats
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_SUCC_MCS: MCS stats for ACKed MPDUs
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_FAIL_MCS: MCS stats for failed MPDUs
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_DELAY: tx delay stats
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU: MPDUs received
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_BYTES: bytes received
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_PPDU: PPDU received
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_PPDU_BYTES: PPDU bytes received
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_LOST: packets lost
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_RETRY: number of RX packets
 *    flagged as retransmissions
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_DUP: number of RX packets
 *    flagged as duplicated
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_DISCARD: number of RX
 *    packets discarded
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_AGGR_NUM: length of RX aggregation
 *    stats buffer.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MCS_NUM: length of RX mcs
 *    stats buffer.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MCS: RX mcs stats buffer
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_AGGR: aggregation stats buffer
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_PS_TIMES: times STAs go to sleep
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_PS_DURATION: STAs' total sleep time
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_PROBE_REQ: number of probe
 *    requests received
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MGMT: number of other mgmt
 *    frames received
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_IDLE_TIME: Percentage of idle time
 *    there is no TX, nor RX, nor interference.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_TIME: percentage of time
 *    transmitting packets.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_TIME: percentage of time
 *    for receiving.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_BUSY: percentage of time
 *    interference detected.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_BAD: percentage of time
 *    receiving packets with errors.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_BAD: percentage of time
 *    TX no-ACK.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_NO_AVAIL: percentage of time
 *    the chip is unable to work in normal conditions.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_IN_BSS_TIME: percentage of time
 *    receiving packets in current BSS.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_OUT_BSS_TIME: percentage of time
 *    receiving packets not in current BSS.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_ANT_NUM: number of antennas
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_SIGNAL:
 *    This is a container for per antenna signal stats.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_ANT_SNR: per antenna SNR value
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_ANT_NF: per antenna NF value
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_IFACE_RSSI_BEACON: RSSI of beacon
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_IFACE_SNR_BEACON: SNR of beacon
 */
enum qca_wlan_vendor_attr_ll_stats_ext {
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_INVALID = 0,

	/* Attributes for configurations */
	QCA_WLAN_VENDOR_ATTR_LL_STATS_CFG_PERIOD,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_CFG_THRESHOLD,

	/* Peer STA power state change */
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_PS_CHG,

	/* TX failure event */
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TID,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_NUM_MSDU,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_STATUS,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_PS_STATE,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_MAC_ADDRESS,

	/* MAC counters */
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_GLOBAL,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_EVENT_MODE,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_IFACE_ID,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_ID,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_BITMAP,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_BITMAP,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_CCA_BSS_BITMAP,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_SIGNAL_BITMAP,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_NUM,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_CHANNEL_NUM,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_CCA_BSS,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER,

	/* Sub-attributes for PEER_AC_TX */
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_MSDU,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_MPDU,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_PPDU,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_BYTES,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_DROP,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_DROP_BYTES,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_RETRY,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_NO_ACK,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_NO_BACK,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_AGGR_NUM,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_SUCC_MCS_NUM,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_FAIL_MCS_NUM,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_AGGR,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_SUCC_MCS,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_FAIL_MCS,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_DELAY_ARRAY_SIZE,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_DELAY,

	/* Sub-attributes for PEER_AC_RX */
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_BYTES,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_PPDU,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_PPDU_BYTES,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_LOST,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_RETRY,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_DUP,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_DISCARD,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_AGGR_NUM,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MCS_NUM,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MCS,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_AGGR,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_PS_TIMES,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_PS_DURATION,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_PROBE_REQ,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MGMT,

	/* Sub-attributes for CCA_BSS */
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_IDLE_TIME,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_TIME,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_TIME,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_BUSY,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_BAD,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_BAD,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_NO_AVAIL,

	/* sub-attribute for BSS_RX_TIME */
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_IN_BSS_TIME,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_OUT_BSS_TIME,

	/* Sub-attributes for PEER_SIGNAL */
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_ANT_NUM,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_SIGNAL,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_ANT_SNR,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_ANT_NF,

	/* Sub-attributes for IFACE_BSS */
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_IFACE_RSSI_BEACON,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_IFACE_SNR_BEACON,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_LAST,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_MAX =
		QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_LAST - 1
};

/**
 * enum qca_wlan_vendor_attr_external_acs_channels: Attributes to vendor subcmd
 * QCA_NL80211_VENDOR_SUBCMD_EXTERNAL_ACS. This carries a list of channels
 * in priority order as decided after ACS operation in userspace.
 *
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_REASON: Required (u8).
 * One of reason code from enum qca_wlan_vendor_acs_select_reason.
 *
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_LIST: Required
 * Array of nested values for each channel with following attributes:
 *     QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_PRIMARY,
 *     QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_SECONDARY,
 *     QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_CENTER_SEG0,
 *     QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_CENTER_SEG1,
 *     QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_WIDTH
 * Note: If both the driver and user-space application supports the 6 GHz band,
 * QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_LIST is deprecated and use
 * QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_FREQUENCY_LIST.
 * To maintain backward compatibility,
 * QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_LIST
 * is still used if either of the driver or user space application doesn't
 * support the 6 GHz band.
 *
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_PRIMARY: Required (u8).
 * Primary channel number
 * Note: If both the driver and user-space application supports the 6 GHz band,
 * QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_PRIMARY is deprecated and use
 * QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_FREQUENCY_PRIMARY.
 * To maintain backward compatibility,
 * QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_PRIMARY
 * is still used if either of the driver or user space application doesn't
 * support the 6 GHz band.
 *
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_SECONDARY: Required (u8).
 * Secondary channel number, required only for 160 and 80+80 MHz bandwidths.
 * Note: If both the driver and user-space application supports the 6 GHz band,
 * QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_SECONDARY is deprecated and use
 * QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_FREQUENCY_SECONDARY.
 * To maintain backward compatibility,
 * QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_SECONDARY
 * is still used if either of the driver or user space application
 * doesn't support the 6 GHz band.
 *
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_CENTER_SEG0: Required (u8).
 * VHT seg0 channel number
 * Note: If both the driver and user-space application supports the 6 GHz band,
 * QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_CENTER_SEG0 is deprecated and use
 * QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_FREQUENCY_CENTER_SEG0.
 * To maintain backward compatibility,
 * QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_CENTER_SEG0
 * is still used if either of the driver or user space application
 * doesn't support the 6 GHz band.
 *
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_CENTER_SEG1: Required (u8).
 * VHT seg1 channel number
 * Note: If both the driver and user-space application supports the 6 GHz band,
 * QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_CENTER_SEG1 is deprecated and use
 * QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_FREQUENCY_CENTER_SEG1.
 * To maintain backward compatibility,
 * QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_CENTER_SEG1
 * is still used if either of the driver or user space application
 * doesn't support the 6 GHz band.
 *
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_WIDTH: Required (u8).
 * Takes one of enum nl80211_chan_width values.
 *
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_FREQUENCY_LIST: Required
 * Array of nested values for each channel with following attributes:
 *	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_FREQUENCY_PRIMARY in MHz (u32),
 *	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_FREQUENCY_SECONDARY in MHz (u32),
 *	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_FREQUENCY_CENTER_SEG0 in MHz (u32),
 *	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_FREQUENCY_CENTER_SEG1 in MHz (u32),
 *	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_WIDTH
 * Note: If user-space application has no support of the 6 GHz band, this
 * attribute is optional.
 *
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_FREQUENCY_PRIMARY: Required (u32)
 * Primary channel frequency in MHz
 * Note: If user-space application has no support of the 6 GHz band, this
 * attribute is optional.
 *
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_FREQUENCY_SECONDARY: Required (u32)
 * Secondary channel frequency in MHz used for HT 40 MHz channels.
 * Note: If user-space application has no support of the 6 GHz band, this
 * attribute is optional.
 *
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_FREQUENCY_CENTER_SEG0: Required (u32)
 * VHT seg0 channel frequency in MHz
 * Note: If user-space application has no support of the 6GHz band, this
 * attribute is optional.
 *
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_FREQUENCY_CENTER_SEG1: Required (u32)
 * VHT seg1 channel frequency in MHz
 * Note: If user-space application has no support of the 6 GHz band, this
 * attribute is optional.
 */
enum qca_wlan_vendor_attr_external_acs_channels {
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_INVALID = 0,

	/* One of reason code (u8) from enum qca_wlan_vendor_acs_select_reason
	 */
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_REASON = 1,

	/* Array of nested values for each channel with following attributes:
	 * QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_BAND,
	 * QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_PRIMARY,
	 * QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_SECONDARY,
	 * QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_CENTER_SEG0,
	 * QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_CENTER_SEG1,
	 * QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_WIDTH
	 */
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_LIST = 2,
	/* This (u8) will hold values of one of enum nl80211_bands */
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_BAND = 3,
	/* Primary channel (u8) */
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_PRIMARY = 4,
	/* Secondary channel (u8) used for HT 40 MHz channels */
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_SECONDARY = 5,
	/* VHT seg0 channel (u8) */
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_CENTER_SEG0 = 6,
	/* VHT seg1 channel (u8) */
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_CENTER_SEG1 = 7,
	/* Channel width (u8). Takes one of enum nl80211_chan_width values. */
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_WIDTH = 8,

	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_FREQUENCY_LIST = 9,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_FREQUENCY_PRIMARY = 10,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_FREQUENCY_SECONDARY = 11,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_FREQUENCY_CENTER_SEG0 = 12,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_FREQUENCY_CENTER_SEG1 = 13,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_LAST,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_MAX =
		QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_LAST - 1
};

/**
 * qca_wlan_vendor_acs_select_reason: This represents the different reasons why
 * the ACS has to be triggered. These values are used by
 * QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_REASON and
 * QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_REASON
 */
enum qca_wlan_vendor_acs_select_reason {
	/* Represents the reason that the ACS triggered during the AP start */
	QCA_WLAN_VENDOR_ACS_SELECT_REASON_INIT,
	/* Represents the reason that DFS found with the current channel */
	QCA_WLAN_VENDOR_ACS_SELECT_REASON_DFS,
	/* Represents the reason that LTE co-exist in the current band. */
	QCA_WLAN_VENDOR_ACS_SELECT_REASON_LTE_COEX,
	/* Represents the reason that generic, uncategorized interference has
	 * been found in the current channel.
	 */
	QCA_WLAN_VENDOR_ACS_SELECT_REASON_GENERIC_INTERFERENCE,
	/* Represents the reason that excessive 802.11 interference has been
	 * found in the current channel.
	 */
	QCA_WLAN_VENDOR_ACS_SELECT_REASON_80211_INTERFERENCE,
	/* Represents the reason that generic Continuous Wave (CW) interference
	 * has been found in the current channel.
	 */
	QCA_WLAN_VENDOR_ACS_SELECT_REASON_CW_INTERFERENCE,
	/* Represents the reason that Microwave Oven (MWO) interference has been
	 * found in the current channel.
	 */
	QCA_WLAN_VENDOR_ACS_SELECT_REASON_MWO_INTERFERENCE,
	/* Represents the reason that generic Frequency-Hopping Spread Spectrum
	 * (FHSS) interference has been found in the current channel. This may
	 * include 802.11 waveforms.
	 */
	QCA_WLAN_VENDOR_ACS_SELECT_REASON_FHSS_INTERFERENCE,
	/* Represents the reason that non-802.11 generic Frequency-Hopping
	 * Spread Spectrum (FHSS) interference has been found in the current
	 * channel.
	 */
	QCA_WLAN_VENDOR_ACS_SELECT_REASON_NON_80211_FHSS_INTERFERENCE,
	/* Represents the reason that generic Wideband (WB) interference has
	 * been found in the current channel. This may include 802.11 waveforms.
	 */
	QCA_WLAN_VENDOR_ACS_SELECT_REASON_WB_INTERFERENCE,
	/* Represents the reason that non-802.11 generic Wideband (WB)
	 * interference has been found in the current channel.
	 */
	QCA_WLAN_VENDOR_ACS_SELECT_REASON_NON_80211_WB_INTERFERENCE,
	/* Represents the reason that Jammer interference has been found in the
	 * current channel.
	 */
	QCA_WLAN_VENDOR_ACS_SELECT_REASON_JAMMER_INTERFERENCE,
};

/**
 * enum qca_wlan_gpio_attr - Parameters for GPIO configuration
 *
 * @QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_COMMAND: Required (u32)
 * value to specify the gpio command, please refer to enum qca_gpio_cmd_type
 * to get the available value that this item can use.
 *
 * @QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_PINNUM: Required (u32)
 * value to specify the gpio number.
 * This is required, when %QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_COMMAND is
 * %QCA_WLAN_VENDOR_GPIO_CONFIG or %.QCA_WLAN_VENDOR_GPIO_OUTPUT.
 *
 * @QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_VALUE: Required (u32)
 * value to specify the gpio output level, please refer to enum qca_gpio_value
 * to get the available value that this item can use.
 * This is required, when %QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_COMMAND is
 * %QCA_WLAN_VENDOR_GPIO_OUTPUT.
 *
 * @QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_PULL_TYPE: Required (u32)
 * value to specify the gpio pull type, please refer to enum qca_gpio_pull_type
 * to get the available value that this item can use.
 * This is required, when %QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_COMMAND is
 * %QCA_WLAN_VENDOR_GPIO_CONFIG.
 *
 * @QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_INTR_MODE: Required (u32)
 * value to specify the gpio interrupt mode, please refer to enum
 * qca_gpio_interrupt_mode to get the available value that this item can use.
 * This is required, when %QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_COMMAND is
 * %QCA_WLAN_VENDOR_GPIO_CONFIG.
 *
 * @QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_DIR: Required (u32)
 * value to specify the gpio direction, please refer to enum qca_gpio_direction
 * to get the available value that this item can use.
 * This is required, when %QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_COMMAND is
 * %QCA_WLAN_VENDOR_GPIO_CONFIG.
 */
enum qca_wlan_gpio_attr {
	QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_INVALID = 0,
	/* Unsigned 32-bit attribute for GPIO command */
	QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_COMMAND = 1,
	/* Unsigned 32-bit attribute for GPIO PIN number to configure */
	QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_PINNUM = 2,
	/* Unsigned 32-bit attribute for GPIO value to configure */
	QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_VALUE = 3,
	/* Unsigned 32-bit attribute for GPIO pull type */
	QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_PULL_TYPE = 4,
	/* Unsigned 32-bit attribute for GPIO interrupt mode */
	QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_INTR_MODE = 5,
	/* Unsigned 32-bit attribute for GPIO direction to configure */
	QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_DIR = 6,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_LAST,
	QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_MAX =
		QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_LAST - 1,
};

/**
 * enum gpio_cmd_type - GPIO configuration command type
 * @QCA_WLAN_VENDOR_GPIO_CONFIG: set gpio configuration info
 * @QCA_WLAN_VENDOR_GPIO_OUTPUT: set gpio output level
 */
enum qca_gpio_cmd_type {
	QCA_WLAN_VENDOR_GPIO_CONFIG = 0,
	QCA_WLAN_VENDOR_GPIO_OUTPUT = 1,
};

/**
 * enum qca_gpio_pull_type - GPIO pull type
 * @QCA_WLAN_GPIO_PULL_NONE: set gpio pull type to none
 * @QCA_WLAN_GPIO_PULL_UP: set gpio pull up
 * @QCA_WLAN_GPIO_PULL_DOWN: set gpio pull down
 */
enum qca_gpio_pull_type {
	QCA_WLAN_GPIO_PULL_NONE = 0,
	QCA_WLAN_GPIO_PULL_UP = 1,
	QCA_WLAN_GPIO_PULL_DOWN = 2,
	QCA_WLAN_GPIO_PULL_MAX,
};

/**
 * enum qca_gpio_direction - GPIO direction
 * @QCA_WLAN_GPIO_INPUT: set gpio as input mode
 * @QCA_WLAN_GPIO_OUTPUT: set gpio as output mode
 * @QCA_WLAN_GPIO_VALUE_MAX: invalid value
 */
enum qca_gpio_direction {
	QCA_WLAN_GPIO_INPUT = 0,
	QCA_WLAN_GPIO_OUTPUT = 1,
	QCA_WLAN_GPIO_DIR_MAX,
};

/**
 * enum qca_gpio_value - GPIO Value
 * @QCA_WLAN_GPIO_LEVEL_LOW: set gpio output level to low
 * @QCA_WLAN_GPIO_LEVEL_HIGH: set gpio output level to high
 * @QCA_WLAN_GPIO_LEVEL_MAX: invalid value
 */
enum qca_gpio_value {
	QCA_WLAN_GPIO_LEVEL_LOW = 0,
	QCA_WLAN_GPIO_LEVEL_HIGH = 1,
	QCA_WLAN_GPIO_LEVEL_MAX,
};

/**
 * enum gpio_interrupt_mode - GPIO interrupt mode
 * @QCA_WLAN_GPIO_INTMODE_DISABLE: disable interrupt trigger
 * @QCA_WLAN_GPIO_INTMODE_RISING_EDGE: interrupt with gpio rising edge trigger
 * @QCA_WLAN_GPIO_INTMODE_FALLING_EDGE: interrupt with gpio falling edge trigger
 * @QCA_WLAN_GPIO_INTMODE_BOTH_EDGE: interrupt with gpio both edge trigger
 * @QCA_WLAN_GPIO_INTMODE_LEVEL_LOW: interrupt with gpio level low trigger
 * @QCA_WLAN_GPIO_INTMODE_LEVEL_HIGH: interrupt with gpio level high trigger
 * @QCA_WLAN_GPIO_INTMODE_MAX: invalid value
 */
enum qca_gpio_interrupt_mode {
	QCA_WLAN_GPIO_INTMODE_DISABLE = 0,
	QCA_WLAN_GPIO_INTMODE_RISING_EDGE = 1,
	QCA_WLAN_GPIO_INTMODE_FALLING_EDGE = 2,
	QCA_WLAN_GPIO_INTMODE_BOTH_EDGE = 3,
	QCA_WLAN_GPIO_INTMODE_LEVEL_LOW = 4,
	QCA_WLAN_GPIO_INTMODE_LEVEL_HIGH = 5,
	QCA_WLAN_GPIO_INTMODE_MAX,
};

/**
 * qca_wlan_set_qdepth_thresh_attr - Parameters for setting
 * MSDUQ depth threshold per peer per tid in the target
 *
 * Associated Vendor Command:
 * QCA_NL80211_VENDOR_SUBCMD_SET_QDEPTH_THRESH
 */
enum qca_wlan_set_qdepth_thresh_attr {
	QCA_WLAN_VENDOR_ATTR_QDEPTH_THRESH_INVALID = 0,
	/* 6-byte MAC address */
	QCA_WLAN_VENDOR_ATTR_QDEPTH_THRESH_MAC_ADDR,
	/* Unsigned 32-bit attribute for holding the TID */
	QCA_WLAN_VENDOR_ATTR_QDEPTH_THRESH_TID,
	/* Unsigned 32-bit attribute for holding the update mask
	 * bit 0 - Update high priority msdu qdepth threshold
	 * bit 1 - Update low priority msdu qdepth threshold
	 * bit 2 - Update UDP msdu qdepth threshold
	 * bit 3 - Update Non UDP msdu qdepth threshold
	 * rest of bits are reserved
	 */
	QCA_WLAN_VENDOR_ATTR_QDEPTH_THRESH_UPDATE_MASK,
	/* Unsigned 32-bit attribute for holding the threshold value */
	QCA_WLAN_VENDOR_ATTR_QDEPTH_THRESH_VALUE,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_QDEPTH_THRESH_LAST,
	QCA_WLAN_VENDOR_ATTR_QDEPTH_THRESH_MAX =
		QCA_WLAN_VENDOR_ATTR_QDEPTH_THRESH_LAST - 1,
};

/**
 * qca_wlan_vendor_attr_external_acs_policy: Attribute values for
 * QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_POLICY to the vendor subcmd
 * QCA_NL80211_VENDOR_SUBCMD_EXTERNAL_ACS. This represents the
 * external ACS policies to select the channels w.r.t. the PCL weights.
 * (QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_PCL represents the channels and
 * their PCL weights.)
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_POLICY_PCL_MANDATORY: Mandatory to
 * select a channel with non-zero PCL weight.
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_POLICY_PCL_PREFERRED: Prefer a
 * channel with non-zero PCL weight.
 *
 */
enum qca_wlan_vendor_attr_external_acs_policy {
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_POLICY_PCL_PREFERRED,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_POLICY_PCL_MANDATORY,
};

/**
 * enum qca_wlan_vendor_attr_spectral_scan - Spectral scan config parameters
 */
enum qca_wlan_vendor_attr_spectral_scan {
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_INVALID = 0,
	/*
	 * Number of times the chip enters spectral scan mode before
	 * deactivating spectral scans. When set to 0, chip will enter spectral
	 * scan mode continuously. u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_SCAN_COUNT = 1,
	/*
	 * Spectral scan period. Period increment resolution is 256*Tclk,
	 * where Tclk = 1/44 MHz (Gmode), 1/40 MHz (Amode). u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_SCAN_PERIOD = 2,
	/* Spectral scan priority. u32 attribute. */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_PRIORITY = 3,
	/* Number of FFT data points to compute. u32 attribute. */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_FFT_SIZE = 4,
	/*
	 * Enable targeted gain change before starting the spectral scan FFT.
	 * u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_GC_ENA = 5,
	/* Restart a queued spectral scan. u32 attribute. */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_RESTART_ENA = 6,
	/*
	 * Noise floor reference number for the calculation of bin power.
	 * u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_NOISE_FLOOR_REF = 7,
	/*
	 * Disallow spectral scan triggers after TX/RX packets by setting
	 * this delay value to roughly SIFS time period or greater.
	 * u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_INIT_DELAY = 8,
	/*
	 * Number of strong bins (inclusive) per sub-channel, below
	 * which a signal is declared a narrow band tone. u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_NB_TONE_THR = 9,
	/*
	 * Specify the threshold over which a bin is declared strong (for
	 * scan bandwidth analysis). u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_STR_BIN_THR = 10,
	/* Spectral scan report mode. u32 attribute. */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_WB_RPT_MODE = 11,
	/*
	 * RSSI report mode, if the ADC RSSI is below
	 * QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_RSSI_THR,
	 * then FFTs will not trigger, but timestamps and summaries get
	 * reported. u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_RSSI_RPT_MODE = 12,
	/*
	 * ADC RSSI must be greater than or equal to this threshold (signed dB)
	 * to ensure spectral scan reporting with normal error code.
	 * u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_RSSI_THR = 13,
	/*
	 * Format of frequency bin magnitude for spectral scan triggered FFTs:
	 * 0: linear magnitude, 1: log magnitude (20*log10(lin_mag)).
	 * u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_PWR_FORMAT = 14,
	/*
	 * Format of FFT report to software for spectral scan triggered FFTs.
	 * 0: No FFT report (only spectral scan summary report)
	 * 1: 2-dword summary of metrics for each completed FFT + spectral scan
	 * report
	 * 2: 2-dword summary of metrics for each completed FFT + 1x-oversampled
	 * bins (in-band) per FFT + spectral scan summary report
	 * 3: 2-dword summary of metrics for each completed FFT + 2x-oversampled
	 * bins (all) per FFT + spectral scan summary report
	 * u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_RPT_MODE = 15,
	/*
	 * Number of LSBs to shift out in order to scale the FFT bins.
	 * u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_BIN_SCALE = 16,
	/*
	 * Set to 1 (with spectral_scan_pwr_format=1), to report bin magnitudes
	 * in dBm power. u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_DBM_ADJ = 17,
	/*
	 * Per chain enable mask to select input ADC for search FFT.
	 * u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_CHN_MASK = 18,
	/*
	 * An unsigned 64-bit integer provided by host driver to identify the
	 * spectral scan request. This attribute is included in the scan
	 * response message for @QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_START
	 * and used as an attribute in
	 * @QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_STOP to identify the
	 * specific scan to be stopped.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_COOKIE = 19,
	/* Skip interval for FFT reports. u32 attribute */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_FFT_PERIOD = 20,
	/* Set to report only one set of FFT results.
	 * u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_SHORT_REPORT = 21,
	/* Debug level for spectral module in driver.
	 * 0 : Verbosity level 0
	 * 1 : Verbosity level 1
	 * 2 : Verbosity level 2
	 * 3 : Matched filterID display
	 * 4 : One time dump of FFT report
	 * u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_DEBUG_LEVEL = 22,
	/* Type of spectral scan request. u32 attribute.
	 * It uses values defined in enum
	 * qca_wlan_vendor_attr_spectral_scan_request_type.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_REQUEST_TYPE = 23,
	/* This specifies the frequency span over which spectral
	 * scan would be carried out. Its value depends on the
	 * value of QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_MODE and
	 * the relation is as follows.
	 * QCA_WLAN_VENDOR_SPECTRAL_SCAN_MODE_NORMAL
	 *    Not applicable. Spectral scan would happen in the
	 *    operating span.
	 * QCA_WLAN_VENDOR_SPECTRAL_SCAN_MODE_AGILE
	 *    Center frequency (in MHz) of the span of interest or
	 *    for convenience, center frequency (in MHz) of any channel
	 *    in the span of interest. For 80+80 MHz agile spectral scan
	 *    request it represents center frequency (in MHz) of the primary
	 *    80 MHz span or for convenience, center frequency (in MHz) of any
	 *    channel in the primary 80 MHz span. If agile spectral scan is
	 *    initiated without setting a valid frequency it returns the
	 *    error code
	 *    (QCA_WLAN_VENDOR_SPECTRAL_SCAN_ERR_PARAM_NOT_INITIALIZED).
	 * u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_FREQUENCY = 24,
	/* Spectral scan mode. u32 attribute.
	 * It uses values defined in enum qca_wlan_vendor_spectral_scan_mode.
	 * If this attribute is not present, it is assumed to be
	 * normal mode (QCA_WLAN_VENDOR_SPECTRAL_SCAN_MODE_NORMAL).
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_MODE = 25,
	/* Spectral scan error code. u32 attribute.
	 * It uses values defined in enum
	 * qca_wlan_vendor_spectral_scan_error_code.
	 * This attribute is included only in failure scenarios.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_ERROR_CODE = 26,
	/* 8-bit unsigned value to enable/disable debug of the
	 * Spectral DMA ring.
	 * 1-enable, 0-disable
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_DMA_RING_DEBUG = 27,
	/* 8-bit unsigned value to enable/disable debug of the
	 * Spectral DMA buffers.
	 * 1-enable, 0-disable
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_DMA_BUFFER_DEBUG = 28,
	/* This specifies the frequency span over which spectral scan would be
	 * carried out. Its value depends on the value of
	 * QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_MODE and the relation is as
	 * follows.
	 * QCA_WLAN_VENDOR_SPECTRAL_SCAN_MODE_NORMAL
	 *    Not applicable. Spectral scan would happen in the operating span.
	 * QCA_WLAN_VENDOR_SPECTRAL_SCAN_MODE_AGILE
	 *    This attribute is applicable only for agile spectral scan
	 *    requests in 80+80 MHz mode. It represents center frequency (in
	 *    MHz) of the secondary 80 MHz span or for convenience, center
	 *    frequency (in MHz) of any channel in the secondary 80 MHz span.
	 * u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_FREQUENCY_2 = 29,

	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_MAX =
		QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_spectral_diag_stats - Used by the vendor command
 * QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_DIAG_STATS.
 */
enum qca_wlan_vendor_attr_spectral_diag_stats {
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_DIAG_INVALID = 0,
	/* Number of spectral TLV signature mismatches.
	 * u64 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_DIAG_SIG_MISMATCH = 1,
	/* Number of spectral phyerror events with insufficient length when
	 * parsing for secondary 80 search FFT report. u64 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_DIAG_SEC80_SFFT_INSUFFLEN = 2,
	/* Number of spectral phyerror events without secondary 80
	 * search FFT report. u64 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_DIAG_NOSEC80_SFFT = 3,
	/* Number of spectral phyerror events with vht operation segment 1 id
	 * mismatches in search fft report. u64 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_DIAG_VHTSEG1ID_MISMATCH = 4,
	/* Number of spectral phyerror events with vht operation segment 2 id
	 * mismatches in search fft report. u64 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_DIAG_VHTSEG2ID_MISMATCH = 5,

	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_DIAG_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_DIAG_MAX =
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_DIAG_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_spectral_cap - Used by the vendor command
 * QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_CAP_INFO.
 */
enum qca_wlan_vendor_attr_spectral_cap {
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_INVALID = 0,
	/* Flag attribute to indicate phydiag capability */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_PHYDIAG = 1,
	/* Flag attribute to indicate radar detection capability */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_RADAR = 2,
	/* Flag attribute to indicate spectral capability */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_SPECTRAL = 3,
	/* Flag attribute to indicate advanced spectral capability */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_ADVANCED_SPECTRAL = 4,
	/* Spectral hardware generation. u32 attribute.
	 * It uses values defined in enum
	 * qca_wlan_vendor_spectral_scan_cap_hw_gen.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_HW_GEN = 5,
	/* Spectral bin scaling formula ID. u16 attribute.
	 * It uses values defined in enum
	 * qca_wlan_vendor_spectral_scan_cap_formula_id.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_FORMULA_ID = 6,
	/* Spectral bin scaling param - low level offset.
	 * s16 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_LOW_LEVEL_OFFSET = 7,
	/* Spectral bin scaling param - high level offset.
	 * s16 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_HIGH_LEVEL_OFFSET = 8,
	/* Spectral bin scaling param - RSSI threshold.
	 * s16 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_RSSI_THR = 9,
	/* Spectral bin scaling param - default AGC max gain.
	 * u8 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_DEFAULT_AGC_MAX_GAIN = 10,
	/* Flag attribute to indicate agile spectral scan capability
	 * for 20/40/80 MHz modes.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_AGILE_SPECTRAL = 11,
	/* Flag attribute to indicate agile spectral scan capability
	 * for 160 MHz mode.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_AGILE_SPECTRAL_160 = 12,
	/* Flag attribute to indicate agile spectral scan capability
	 * for 80+80 MHz mode.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_AGILE_SPECTRAL_80_80 = 13,
	/* Number of spectral detectors used for scan in 20 MHz.
	 * u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_NUM_DETECTORS_20_MHZ = 14,
	/* Number of spectral detectors used for scan in 40 MHz.
	 * u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_NUM_DETECTORS_40_MHZ = 15,
	/* Number of spectral detectors used for scan in 80 MHz.
	 * u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_NUM_DETECTORS_80_MHZ = 16,
	/* Number of spectral detectors used for scan in 160 MHz.
	 * u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_NUM_DETECTORS_160_MHZ = 17,
	/* Number of spectral detectors used for scan in 80+80 MHz.
	 * u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_NUM_DETECTORS_80P80_MHZ = 18,

	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_MAX =
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_spectral_scan_status - used by the vendor command
 * QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_STATUS.
 */
enum qca_wlan_vendor_attr_spectral_scan_status {
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_STATUS_INVALID = 0,
	/* Flag attribute to indicate whether spectral scan is enabled */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_STATUS_IS_ENABLED = 1,
	/* Flag attribute to indicate whether spectral scan is in progress*/
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_STATUS_IS_ACTIVE = 2,
	/* Spectral scan mode. u32 attribute.
	 * It uses values defined in enum qca_wlan_vendor_spectral_scan_mode.
	 * If this attribute is not present, normal mode
	 * (QCA_WLAN_VENDOR_SPECTRAL_SCAN_MODE_NORMAL is assumed to be
	 * requested.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_STATUS_MODE = 3,

	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_STATUS_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_STATUS_MAX =
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_STATUS_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_config_latency_level - Level for
 * wlan latency module.
 *
 * There will be various of Wi-Fi functionality like scan/roaming/adaptive
 * power saving which would causing data exchange out of service, this
 * would be a big impact on latency. For latency sensitive applications over
 * Wi-Fi are intolerant to such operations and thus would configure them
 * to meet their respective needs. It is well understood by such applications
 * that altering the default behavior would degrade the Wi-Fi functionality
 * w.r.t the above pointed WLAN operations.
 *
 * @QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL_NORMAL:
 *      Default WLAN operation level which throughput orientated.
 * @QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL_MODERATE:
 *      Use moderate level to improve latency by limit scan duration.
 * @QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL_LOW:
 *      Use low latency level to benifit application like concurrent
 *      downloading or video streaming via constraint scan/adaptive PS.
 * @QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL_ULTRALOW:
 *      Use ultra low latency level to benefit for gaming/voice
 *      application via constraint scan/roaming/adaptive PS.
 */
enum qca_wlan_vendor_attr_config_latency_level {
	QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL_NORMAL = 1,
	QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL_MODERATE = 2,
	QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL_LOW = 3,
	QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL_ULTRALOW = 4,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL_MAX =
	QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL_AFTER_LAST - 1,
};

/**
 * qca_wlan_vendor_attr_spectral_scan_request_type: Attribute values for
 * QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_REQUEST_TYPE to the vendor subcmd
 * QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_START. This represents the
 * spectral scan request types.
 * @QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_REQUEST_TYPE_SCAN_AND_CONFIG: Request to
 * set the spectral parameters and start scan.
 * @QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_REQUEST_TYPE_SCAN: Request to
 * only set the spectral parameters.
 * @QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_REQUEST_TYPE_CONFIG: Request to
 * only start the spectral scan.
 */
enum qca_wlan_vendor_attr_spectral_scan_request_type {
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_REQUEST_TYPE_SCAN_AND_CONFIG,
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_REQUEST_TYPE_SCAN,
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_REQUEST_TYPE_CONFIG,
};

/**
 * qca_wlan_vendor_spectral_scan_mode: Attribute values for
 * QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_MODE in the vendor subcmd
 * QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_START and
 * QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_STATUS_MODE in the vendor subcmd
 * QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_STATUS. This represents the
 * spectral scan modes.
 * @QCA_WLAN_VENDOR_SPECTRAL_SCAN_MODE_NORMAL: Normal spectral scan:
 * spectral scan in the current operating span.
 * @QCA_WLAN_VENDOR_SPECTRAL_SCAN_MODE_AGILE: Agile spectral scan:
 * spectral scan in the configured agile span.
 */
enum qca_wlan_vendor_spectral_scan_mode {
	QCA_WLAN_VENDOR_SPECTRAL_SCAN_MODE_NORMAL = 0,
	QCA_WLAN_VENDOR_SPECTRAL_SCAN_MODE_AGILE = 1,
};

/**
 * qca_wlan_vendor_spectral_scan_error_code: Attribute values for
 * QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_ERROR_CODE in the vendor subcmd
 * QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_START.
 * @QCA_WLAN_VENDOR_SPECTRAL_SCAN_ERR_PARAM_UNSUPPORTED: Changing the value
 * of a parameter is not supported.
 * @QCA_WLAN_VENDOR_SPECTRAL_SCAN_ERR_MODE_UNSUPPORTED: Requested spectral scan
 * mode is not supported.
 * @QCA_WLAN_VENDOR_SPECTRAL_SCAN_ERR_PARAM_INVALID_VALUE: A parameter
 * has invalid value.
 * @QCA_WLAN_VENDOR_SPECTRAL_SCAN_ERR_PARAM_NOT_INITIALIZED: A parameter
 * is not initialized.
 */
enum qca_wlan_vendor_spectral_scan_error_code {
	QCA_WLAN_VENDOR_SPECTRAL_SCAN_ERR_PARAM_UNSUPPORTED = 0,
	QCA_WLAN_VENDOR_SPECTRAL_SCAN_ERR_MODE_UNSUPPORTED = 1,
	QCA_WLAN_VENDOR_SPECTRAL_SCAN_ERR_PARAM_INVALID_VALUE = 2,
	QCA_WLAN_VENDOR_SPECTRAL_SCAN_ERR_PARAM_NOT_INITIALIZED = 3,
};

/**
 * qca_wlan_vendor_spectral_scan_cap_hw_gen: Attribute values for
 * QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_HW_GEN to the vendor subcmd
 * QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_CAP_INFO. This represents the
 * spectral hardware generation.
 * @QCA_WLAN_VENDOR_SPECTRAL_SCAN_CAP_HW_GEN_1: generation 1
 * @QCA_WLAN_VENDOR_SPECTRAL_SCAN_CAP_HW_GEN_2: generation 2
 * @QCA_WLAN_VENDOR_SPECTRAL_SCAN_CAP_HW_GEN_3: generation 3
 */
enum qca_wlan_vendor_spectral_scan_cap_hw_gen {
	QCA_WLAN_VENDOR_SPECTRAL_SCAN_CAP_HW_GEN_1 = 0,
	QCA_WLAN_VENDOR_SPECTRAL_SCAN_CAP_HW_GEN_2 = 1,
	QCA_WLAN_VENDOR_SPECTRAL_SCAN_CAP_HW_GEN_3 = 2,
};

/**
 * qca_wlan_vendor_spectral_scan_cap_formula_id: Attribute values for
 * QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_FORMULA_ID in the vendor subcmd
 * QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_CAP_INFO. This represents the
 * Spectral bin scaling formula ID.
 * @QCA_WLAN_VENDOR_SPECTRAL_SCAN_CAP_NO_SCALING: No scaling
 * @QCA_WLAN_VENDOR_SPECTRAL_SCAN_CAP_AGC_GAIN_RSSI_CORR_BASED: AGC gain
 * and RSSI threshold based formula.
 */
enum qca_wlan_vendor_spectral_scan_cap_formula_id {
	QCA_WLAN_VENDOR_SPECTRAL_SCAN_CAP_NO_SCALING = 0,
	QCA_WLAN_VENDOR_SPECTRAL_SCAN_CAP_AGC_GAIN_RSSI_CORR_BASED = 1,
};

/**
 * enum qca_wlan_vendor_attr_rropavail_info - Specifies whether Representative
 * RF Operating Parameter (RROP) information is available, and if so, at which
 * point in the application-driver interaction sequence it can be retrieved by
 * the application from the driver. This point may vary by architecture and
 * other factors. This is a u16 value.
 */
enum qca_wlan_vendor_attr_rropavail_info {
	/* RROP information is unavailable. */
	QCA_WLAN_VENDOR_ATTR_RROPAVAIL_INFO_UNAVAILABLE,
	/* RROP information is available and the application can retrieve the
	 * information after receiving an QCA_NL80211_VENDOR_SUBCMD_EXTERNAL_ACS
	 * event from the driver.
	 */
	QCA_WLAN_VENDOR_ATTR_RROPAVAIL_INFO_EXTERNAL_ACS_START,
	/* RROP information is available only after a vendor specific scan
	 * (requested using QCA_NL80211_VENDOR_SUBCMD_TRIGGER_SCAN) has
	 * successfully completed. The application can retrieve the information
	 * after receiving the QCA_NL80211_VENDOR_SUBCMD_SCAN_DONE event from
	 * the driver.
	 */
	QCA_WLAN_VENDOR_ATTR_RROPAVAIL_INFO_VSCAN_END,
};

/**
 * enum qca_wlan_vendor_attr_rrop_info - Specifies vendor specific
 * Representative RF Operating Parameter (RROP) information. It is sent for the
 * vendor command QCA_NL80211_VENDOR_SUBCMD_GET_RROP_INFO. This information is
 * intended for use by external Auto Channel Selection applications. It provides
 * guidance values for some RF parameters that are used by the system during
 * operation. These values could vary by channel, band, radio, and so on.
 */
enum qca_wlan_vendor_attr_rrop_info {
	QCA_WLAN_VENDOR_ATTR_RROP_INFO_INVALID = 0,

	/* Representative Tx Power List (RTPL) which has an array of nested
	 * values as per attributes in enum qca_wlan_vendor_attr_rtplinst.
	 */
	QCA_WLAN_VENDOR_ATTR_RROP_INFO_RTPL = 1,

	QCA_WLAN_VENDOR_ATTR_RROP_INFO_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_RROP_INFO_MAX =
	QCA_WLAN_VENDOR_ATTR_RROP_INFO_AFTER_LAST - 1
};

/**
 * enum qca_wlan_vendor_attr_rtplinst - Specifies attributes for individual list
 * entry instances in the Representative Tx Power List (RTPL). It provides
 * simplified power values intended for helping external Auto channel Selection
 * applications compare potential Tx power performance between channels, other
 * operating conditions remaining identical. These values are not necessarily
 * the actual Tx power values that will be used by the system. They are also not
 * necessarily the max or average values that will be used. Instead, they are
 * relative, summarized keys for algorithmic use computed by the driver or
 * underlying firmware considering a number of vendor specific factors.
 */
enum qca_wlan_vendor_attr_rtplinst {
	QCA_WLAN_VENDOR_ATTR_RTPLINST_INVALID = 0,

	/* Primary channel number (u8).
	 * Note: If both the driver and user space application support the
	 * 6 GHz band, this attribute is deprecated and
	 * QCA_WLAN_VENDOR_ATTR_RTPLINST_PRIMARY_FREQUENCY should be used. To
	 * maintain backward compatibility,
	 * QCA_WLAN_VENDOR_ATTR_RTPLINST_PRIMARY is still used if either the
	 * driver or user space application or both do not support the 6 GHz
	 * band.
	 */
	QCA_WLAN_VENDOR_ATTR_RTPLINST_PRIMARY = 1,
	/* Representative Tx power in dBm (s32) with emphasis on throughput. */
	QCA_WLAN_VENDOR_ATTR_RTPLINST_TXPOWER_THROUGHPUT = 2,
	/* Representative Tx power in dBm (s32) with emphasis on range. */
	QCA_WLAN_VENDOR_ATTR_RTPLINST_TXPOWER_RANGE = 3,
	/* Primary channel center frequency (u32) in MHz */
	QCA_WLAN_VENDOR_ATTR_RTPLINST_PRIMARY_FREQUENCY = 4,

	QCA_WLAN_VENDOR_ATTR_RTPLINST_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_RTPLINST_MAX =
		QCA_WLAN_VENDOR_ATTR_RTPLINST_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_mac - Used by the vendor command
 * QCA_NL80211_VENDOR_SUBCMD_WLAN_MAC_INFO.
 *
 * @QCA_WLAN_VENDOR_ATTR_MAC_INFO: MAC mode info list which has an
 *  array of nested values as per attributes in
 *  enum qca_wlan_vendor_attr_mac_mode_info.
 */
enum qca_wlan_vendor_attr_mac {
	QCA_WLAN_VENDOR_ATTR_MAC_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_MAC_INFO = 1,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_MAC_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_MAC_MAX =
		QCA_WLAN_VENDOR_ATTR_MAC_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_mac_iface_info - Information of the connected
 * WiFi netdev interface on a respective MAC. Used by the attribute
 * QCA_WLAN_VENDOR_ATTR_MAC_IFACE_INFO.
 *
 * @QCA_WLAN_VENDOR_ATTR_MAC_IFACE_INFO_ID: Wi-Fi Netdev's interface id(u32).
 * @QCA_WLAN_VENDOR_ATTR_MAC_IFACE_INFO_FREQ: Associated frequency in MHz of
 *  the connected Wi-Fi interface(u32).
 */
enum qca_wlan_vendor_attr_mac_iface_info {
	QCA_WLAN_VENDOR_ATTR_MAC_IFACE_INFO_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_MAC_IFACE_INFO_IFINDEX = 1,
	QCA_WLAN_VENDOR_ATTR_MAC_IFACE_INFO_FREQ = 2,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_MAC_IFACE_INFO_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_MAC_IFACE_INFO_MAX =
		QCA_WLAN_VENDOR_ATTR_MAC_IFACE_INFO_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_mac_info - Points to MAC the information.
 *  Used by the attribute QCA_WLAN_VENDOR_ATTR_MAC_INFO of the
 *  vendor command QCA_NL80211_VENDOR_SUBCMD_WLAN_MAC_INFO.
 *
 * @QCA_WLAN_VENDOR_ATTR_MAC_INFO_MAC_ID: Hardware MAC ID associated for the
 *  MAC (u32)
 * @QCA_WLAN_VENDOR_ATTR_MAC_INFO_BAND: Band supported by the respective MAC
 *  at a given point. This is a u32 bitmask of BIT(NL80211_BAND_*) as described
 *  in enum nl80211_band.
 * @QCA_WLAN_VENDOR_ATTR_MAC_IFACE_INFO: Refers to list of WLAN net dev
 * interfaces associated with this MAC. Represented by enum
 * qca_wlan_vendor_attr_mac_iface_info.
 */
enum qca_wlan_vendor_attr_mac_info {
	QCA_WLAN_VENDOR_ATTR_MAC_INFO_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_MAC_INFO_MAC_ID = 1,
	QCA_WLAN_VENDOR_ATTR_MAC_INFO_BAND = 2,
	QCA_WLAN_VENDOR_ATTR_MAC_IFACE_INFO = 3,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_MAC_INFO_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_MAC_INFO_MAX =
		QCA_WLAN_VENDOR_ATTR_MAC_INFO_AFTER_LAST - 1,

};

/**
 * enum he_fragmentation_val - HE fragmentation support values
 * Indicates level of dynamic fragmentation that is supported by
 * a STA as a recipient.
 * HE fragmentation values are defined as per 11ax spec and are used in
 * HE capability IE to advertise the support. These values are validated
 * in the driver to check the device capability and advertised in the HE
 * capability element.
 *
 * @HE_FRAG_DISABLE: no support for dynamic fragmentation
 * @HE_FRAG_LEVEL1: support for dynamic fragments that are
 *	contained within an MPDU or S-MPDU, no support for dynamic fragments
 *	within an A-MPDU that is not an S-MPDU.
 * @HE_FRAG_LEVEL2: support for dynamic fragments that are
 *	contained within an MPDU or S-MPDU and support for up to one dynamic
 *	fragment for each MSDU, each A-MSDU if supported by the recipient, and
 *	each MMPDU within an A-MPDU or multi-TID A-MPDU that is not an
 *	MPDU or S-MPDU.
 * @HE_FRAG_LEVEL3: support for dynamic fragments that are
 *	contained within an MPDU or S-MPDU and support for multiple dynamic
 *	fragments for each MSDU and for each A-MSDU if supported by the
 *	recipient within an A-MPDU or multi-TID AMPDU and up to one dynamic
 *	fragment for each MMPDU in a multi-TID A-MPDU that is not an S-MPDU.
 */
enum he_fragmentation_val {
	HE_FRAG_DISABLE,
	HE_FRAG_LEVEL1,
	HE_FRAG_LEVEL2,
	HE_FRAG_LEVEL3,
};

/**
 * enum he_mcs_config - HE MCS support configuration
 *
 * Configures the HE Tx/Rx MCS map in HE capability IE for given bandwidth.
 * These values are used in driver to configure the HE MCS map to advertise
 * Tx/Rx MCS map in HE capability and these values are applied for all the
 * streams supported by the device. To configure MCS for different bandwidths,
 * vendor command needs to be sent using this attribute with appropriate value.
 * For example, to configure HE_80_MCS_0_7, send vendor command using HE MCS
 * attribute with QCA_WLAN_VENDOR_ATTR_HE_80_MCS0_7. And to configure HE MCS
 * for HE_160_MCS0_11 send this command using HE MCS config attribute with
 * value QCA_WLAN_VENDOR_ATTR_HE_160_MCS0_11;
 *
 * @HE_80_MCS0_7: support for HE 80/40/20MHz MCS 0 to 7
 * @HE_80_MCS0_9: support for HE 80/40/20MHz MCS 0 to 9
 * @HE_80_MCS0_11: support for HE 80/40/20MHz MCS 0 to 11
 * @HE_160_MCS0_7: support for HE 160MHz MCS 0 to 7
 * @HE_160_MCS0_9: support for HE 160MHz MCS 0 to 9
 * @HE_160_MCS0_11: support for HE 160MHz MCS 0 to 11
 * @HE_80p80_MCS0_7: support for HE 80p80MHz MCS 0 to 7
 * @HE_80p80_MCS0_9: support for HE 80p80MHz MCS 0 to 9
 * @HE_80p80_MCS0_11: support for HE 80p80MHz MCS 0 to 11
 */
enum he_mcs_config {
	HE_80_MCS0_7 = 0,
	HE_80_MCS0_9 = 1,
	HE_80_MCS0_11 = 2,
	HE_160_MCS0_7 = 4,
	HE_160_MCS0_9 = 5,
	HE_160_MCS0_11 = 6,
	HE_80p80_MCS0_7 = 8,
	HE_80p80_MCS0_9 = 9,
	HE_80p80_MCS0_11 = 10,
};

/**
 * enum qca_wlan_ba_session_config - BA session configuration
 *
 * Indicates the configuration values for BA session configuration attribute.
 *
 * @QCA_WLAN_ADD_BA: Establish a new BA session with given configuration.
 * @QCA_WLAN_DELETE_BA: Delete the existing BA session for given TID.
 */
enum qca_wlan_ba_session_config {
	QCA_WLAN_ADD_BA = 1,
	QCA_WLAN_DELETE_BA = 2,
};

/**
 * enum qca_wlan_ac_type - access category type
 *
 * Indicates the access category type value.
 *
 * @QCA_WLAN_AC_BE: BE access category
 * @QCA_WLAN_AC_BK: BK access category
 * @QCA_WLAN_AC_VI: VI access category
 * @QCA_WLAN_AC_VO: VO access category
 * @QCA_WLAN_AC_ALL: All ACs
 */
enum qca_wlan_ac_type {
	QCA_WLAN_AC_BE = 0,
	QCA_WLAN_AC_BK = 1,
	QCA_WLAN_AC_VI = 2,
	QCA_WLAN_AC_VO = 3,
	QCA_WLAN_AC_ALL = 4,
};

/**
 * enum qca_wlan_he_ltf_cfg - HE LTF configuration
 *
 * Indicates the HE LTF configuration value.
 *
 * @QCA_WLAN_HE_LTF_AUTO: HE-LTF is automatically set to
 * the mandatory HE-LTF based on the GI setting
 * @QCA_WLAN_HE_LTF_1X: 1X HE LTF is 3.2us LTF
 * @QCA_WLAN_HE_LTF_2X: 2X HE LTF is 6.4us LTF
 * @QCA_WLAN_HE_LTF_4X: 4X HE LTF is 12.8us LTF
 */
enum qca_wlan_he_ltf_cfg {
	QCA_WLAN_HE_LTF_AUTO = 0,
	QCA_WLAN_HE_LTF_1X = 1,
	QCA_WLAN_HE_LTF_2X = 2,
	QCA_WLAN_HE_LTF_4X = 3,
};

/**
 * enum qca_wlan_he_mac_padding_dur - HE trigger frame MAC padding duration
 *
 * Indicates the HE trigger frame MAC padding duration value.
 *
 * @QCA_WLAN_HE_NO_ADDITIONAL_PROCESS_TIME: no additional time required to
 * process the trigger frame.
 * @QCA_WLAN_HE_8US_OF_PROCESS_TIME: indicates the 8us of processing time for
 * trigger frame.
 * @QCA_WLAN_HE_16US_OF_PROCESS_TIME: indicates the 16us of processing time for
 * trigger frame.
 */
enum qca_wlan_he_mac_padding_dur {
	QCA_WLAN_HE_NO_ADDITIONAL_PROCESS_TIME = 0,
	QCA_WLAN_HE_8US_OF_PROCESS_TIME = 1,
	QCA_WLAN_HE_16US_OF_PROCESS_TIME = 2,
};

/**
 * enum qca_wlan_he_om_ctrl_ch_bw - HE OM control field BW configuration
 *
 * Indicates the HE Operating mode control channel width setting value.
 *
 * @QCA_WLAN_HE_OM_CTRL_BW_20M: Primary 20 MHz
 * @QCA_WLAN_HE_OM_CTRL_BW_40M: Primary 40 MHz
 * @QCA_WLAN_HE_OM_CTRL_BW_80M: Primary 80 MHz
 * @QCA_WLAN_HE_OM_CTRL_BW_160M: 160 MHz and 80+80 MHz
 */
enum qca_wlan_he_om_ctrl_ch_bw {
	QCA_WLAN_HE_OM_CTRL_BW_20M = 0,
	QCA_WLAN_HE_OM_CTRL_BW_40M = 1,
	QCA_WLAN_HE_OM_CTRL_BW_80M = 2,
	QCA_WLAN_HE_OM_CTRL_BW_160M = 3,
};

/**
 * enum qca_wlan_vendor_attr_he_omi_tx: Represents attributes for
 * HE operating mode control transmit request. These attributes are
 * sent as part of QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_OMI_TX and
 * QCA_NL80211_VENDOR_SUBCMD_WIFI_TEST_CONFIGURATION.
 *
 * @QCA_WLAN_VENDOR_ATTR_HE_OMI_RX_NSS: Mandatory 8-bit unsigned value
 * indicates the maximum number of spatial streams, NSS, that the STA
 * supports in reception for PPDU bandwidths less than or equal to 80 MHz
 * and is set to NSS - 1.
 *
 * @QCA_WLAN_VENDOR_ATTR_HE_OMI_CH_BW: Mandatory 8-bit unsigned value
 * indicates the operating channel width supported by the STA for both
 * reception and transmission. Uses enum qca_wlan_he_om_ctrl_ch_bw values.
 *
 * @QCA_WLAN_VENDOR_ATTR_HE_OMI_ULMU_DISABLE: Mandatory 8-bit unsigned value
 * indicates the all trigger based UL MU operations by the STA.
 * 0 - UL MU operations are enabled by the STA.
 * 1 - All triggered UL MU transmissions are suspended by the STA.
 *
 * @QCA_WLAN_VENDOR_ATTR_HE_OMI_TX_NSTS: Mandatory 8-bit unsigned value
 * indicates the maximum number of space-time streams, NSTS, that
 * the STA supports in transmission and is set to NSTS - 1.
 *
 * @QCA_WLAN_VENDOR_ATTR_HE_OMI_ULMU_DATA_DISABLE: 8-bit unsigned value
 * combined with the UL MU Disable subfield and the recipient's setting
 * of the OM Control UL MU Data Disable RX Support subfield in the HE MAC
 * capabilities to determine which HE TB PPDUs are possible by the
 * STA to transmit.
 * 0 - UL MU data operations are enabled by the STA.
 * 1 - Determine which HE TB PPDU types are allowed by the STA if UL MU disable
 * bit is not set, else UL MU Tx is suspended.
 *
 */
enum qca_wlan_vendor_attr_he_omi_tx {
	QCA_WLAN_VENDOR_ATTR_HE_OMI_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_HE_OMI_RX_NSS = 1,
	QCA_WLAN_VENDOR_ATTR_HE_OMI_CH_BW = 2,
	QCA_WLAN_VENDOR_ATTR_HE_OMI_ULMU_DISABLE = 3,
	QCA_WLAN_VENDOR_ATTR_HE_OMI_TX_NSTS = 4,
	QCA_WLAN_VENDOR_ATTR_HE_OMI_ULMU_DATA_DISABLE = 5,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_HE_OMI_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_HE_OMI_MAX =
	QCA_WLAN_VENDOR_ATTR_HE_OMI_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_phy_mode - Different PHY modes
 * These values are used with %QCA_WLAN_VENDOR_ATTR_CONFIG_PHY_MODE.
 *
 * @QCA_WLAN_VENDOR_PHY_MODE_AUTO: autoselect
 * @QCA_WLAN_VENDOR_PHY_MODE_2G_AUTO: 2.4 GHz 802.11b/g/n/ax autoselect
 * @QCA_WLAN_VENDOR_PHY_MODE_5G_AUTO: 5 GHz 802.11a/n/ac/ax autoselect
 * @QCA_WLAN_VENDOR_PHY_MODE_11A: 5 GHz, OFDM
 * @QCA_WLAN_VENDOR_PHY_MODE_11B: 2.4 GHz, CCK
 * @QCA_WLAN_VENDOR_PHY_MODE_11G: 2.4 GHz, OFDM
 * @QCA_WLAN_VENDOR_PHY_MODE_11AGN: Support 802.11n in both 2.4 GHz and 5 GHz
 * @QCA_WLAN_VENDOR_PHY_MODE_11NG_HT20: 2.4 GHz, HT20
 * @QCA_WLAN_VENDOR_PHY_MODE_11NG_HT40PLUS: 2.4 GHz, HT40 (ext ch +1)
 * @QCA_WLAN_VENDOR_PHY_MODE_11NG_HT40MINUS: 2.4 GHz, HT40 (ext ch -1)
 * @QCA_WLAN_VENDOR_PHY_MODE_11NG_HT40: 2.4 GHz, Auto HT40
 * @QCA_WLAN_VENDOR_PHY_MODE_11NA_HT20: 5 GHz, HT20
 * @QCA_WLAN_VENDOR_PHY_MODE_11NA_HT40PLUS: 5 GHz, HT40 (ext ch +1)
 * @QCA_WLAN_VENDOR_PHY_MODE_11NA_HT40MINUS: 5 GHz, HT40 (ext ch -1)
 * @QCA_WLAN_VENDOR_PHY_MODE_11NA_HT40: 5 GHz, Auto HT40
 * @QCA_WLAN_VENDOR_PHY_MODE_11AC_VHT20: 5 GHz, VHT20
 * @QCA_WLAN_VENDOR_PHY_MODE_11AC_VHT40PLUS: 5 GHz, VHT40 (Ext ch +1)
 * @QCA_WLAN_VENDOR_PHY_MODE_11AC_VHT40MINUS: 5 GHz VHT40 (Ext ch -1)
 * @QCA_WLAN_VENDOR_PHY_MODE_11AC_VHT40: 5 GHz, VHT40
 * @QCA_WLAN_VENDOR_PHY_MODE_11AC_VHT80: 5 GHz, VHT80
 * @QCA_WLAN_VENDOR_PHY_MODE_11AC_VHT80P80: 5 GHz, VHT80+80
 * @QCA_WLAN_VENDOR_PHY_MODE_11AC_VHT160: 5 GHz, VHT160
 * @QCA_WLAN_VENDOR_PHY_MODE_11AX_HE20: HE20
 * @QCA_WLAN_VENDOR_PHY_MODE_11AX_HE40: HE40
 * @QCA_WLAN_VENDOR_PHY_MODE_11AX_HE40PLUS: HE40 (ext ch +1)
 * @QCA_WLAN_VENDOR_PHY_MODE_11AX_HE40MINUS: HE40 (ext ch -1)
 * @QCA_WLAN_VENDOR_PHY_MODE_11AX_HE80: HE80
 * @QCA_WLAN_VENDOR_PHY_MODE_11AX_HE80P80: HE 80P80
 * @QCA_WLAN_VENDOR_PHY_MODE_11AX_HE160: HE160
 */
enum qca_wlan_vendor_phy_mode {
	QCA_WLAN_VENDOR_PHY_MODE_AUTO = 0,
	QCA_WLAN_VENDOR_PHY_MODE_2G_AUTO = 1,
	QCA_WLAN_VENDOR_PHY_MODE_5G_AUTO = 2,
	QCA_WLAN_VENDOR_PHY_MODE_11A = 3,
	QCA_WLAN_VENDOR_PHY_MODE_11B = 4,
	QCA_WLAN_VENDOR_PHY_MODE_11G = 5,
	QCA_WLAN_VENDOR_PHY_MODE_11AGN = 6,
	QCA_WLAN_VENDOR_PHY_MODE_11NG_HT20 = 7,
	QCA_WLAN_VENDOR_PHY_MODE_11NG_HT40PLUS = 8,
	QCA_WLAN_VENDOR_PHY_MODE_11NG_HT40MINUS = 9,
	QCA_WLAN_VENDOR_PHY_MODE_11NG_HT40 = 10,
	QCA_WLAN_VENDOR_PHY_MODE_11NA_HT20 = 11,
	QCA_WLAN_VENDOR_PHY_MODE_11NA_HT40PLUS = 12,
	QCA_WLAN_VENDOR_PHY_MODE_11NA_HT40MINUS = 13,
	QCA_WLAN_VENDOR_PHY_MODE_11NA_HT40 = 14,
	QCA_WLAN_VENDOR_PHY_MODE_11AC_VHT20 = 15,
	QCA_WLAN_VENDOR_PHY_MODE_11AC_VHT40PLUS = 16,
	QCA_WLAN_VENDOR_PHY_MODE_11AC_VHT40MINUS = 17,
	QCA_WLAN_VENDOR_PHY_MODE_11AC_VHT40 = 18,
	QCA_WLAN_VENDOR_PHY_MODE_11AC_VHT80 = 19,
	QCA_WLAN_VENDOR_PHY_MODE_11AC_VHT80P80 = 20,
	QCA_WLAN_VENDOR_PHY_MODE_11AC_VHT160 = 21,
	QCA_WLAN_VENDOR_PHY_MODE_11AX_HE20 = 22,
	QCA_WLAN_VENDOR_PHY_MODE_11AX_HE40 = 23,
	QCA_WLAN_VENDOR_PHY_MODE_11AX_HE40PLUS = 24,
	QCA_WLAN_VENDOR_PHY_MODE_11AX_HE40MINUS = 25,
	QCA_WLAN_VENDOR_PHY_MODE_11AX_HE80 = 26,
	QCA_WLAN_VENDOR_PHY_MODE_11AX_HE80P80 = 27,
	QCA_WLAN_VENDOR_PHY_MODE_11AX_HE160 = 28,
};

/* Attributes for data used by
 * QCA_NL80211_VENDOR_SUBCMD_WIFI_TEST_CONFIGURATION
 */
enum qca_wlan_vendor_attr_wifi_test_config {
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_INVALID = 0,
	/* 8-bit unsigned value to configure the driver to enable/disable
	 * WMM feature. This attribute is used to configure testbed device.
	 * 1-enable, 0-disable
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_WMM_ENABLE = 1,

	/* 8-bit unsigned value to configure the driver to accept/reject
	 * the addba request from peer. This attribute is used to configure
	 * the testbed device.
	 * 1-accept addba, 0-reject addba
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ACCEPT_ADDBA_REQ = 2,

	/* 8-bit unsigned value to configure the driver to send or not to
	 * send the addba request to peer.
	 * This attribute is used to configure the testbed device.
	 * 1-send addba, 0-do not send addba
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_SEND_ADDBA_REQ = 3,

	/* 8-bit unsigned value to indicate the HE fragmentation support.
	 * Uses enum he_fragmentation_val values.
	 * This attribute is used to configure the testbed device to
	 * allow the advertised hardware capabilities to be downgraded
	 * for testing purposes.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_FRAGMENTATION = 4,

	/* 8-bit unsigned value to indicate the HE MCS support.
	 * Uses enum he_mcs_config values.
	 * This attribute is used to configure the testbed device to
	 * allow the advertised hardware capabilities to be downgraded
	 * for testing purposes.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_MCS = 5,

	/* 8-bit unsigned value to configure the driver to allow or not to
	 * allow the connection with WEP/TKIP in HT/VHT/HE modes.
	 * This attribute is used to configure the testbed device.
	 * 1-allow wep/tkip in HT/VHT/HE, 0-do not allow wep/tkip
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_WEP_TKIP_IN_HE = 6,

	/* 8-bit unsigned value to configure the driver to add a
	 * new BA session or delete the existing BA session for
	 * given TID. ADDBA command uses the buffer size and tid
	 * configuration if user specifies the values else default
	 * value for buffer size is used for all tids if the tid
	 * also not specified. For DEL_BA command TID value is
	 * required to process the command.
	 * Uses enum qca_wlan_ba_session_config values.
	 * This attribute is used to configure the testbed device.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ADD_DEL_BA_SESSION = 7,

	/* 16-bit unsigned value to configure the buffer size in addba
	 * request and response frames.
	 * This attribute is used to configure the testbed device.
	 * The range of the value is 0 to 256.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ADDBA_BUFF_SIZE = 8,

	/* 8-bit unsigned value to configure the buffer size in addba
	 * request and response frames.
	 * This attribute is used to configure the testbed device.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_BA_TID = 9,

	/* 8-bit unsigned value to configure the no ack policy.
	 * To configure no ack policy, access category value
	 * is required to process the command.
	 * This attribute is used to configure the testbed device.
	 * 1 - enable no ack, 0 - disable no ack
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ENABLE_NO_ACK = 10,

	/* 8-bit unsigned value to configure the AC for no ack policy
	 * This attribute is used to configure the testbed device.
	 * uses the enum qca_wlan_ac_type values
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_NO_ACK_AC = 11,

	/* 8-bit unsigned value to configure the HE LTF
	 * This attribute is used to configure the testbed device.
	 * Uses the enum qca_wlan_he_ltf_cfg values.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_LTF = 12,

	/* 8-bit unsigned value to configure the tx beamformee.
	 * This attribute is used to configure the testbed device.
	 * 1 - enable, 0 - disable.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ENABLE_TX_BEAMFORMEE = 13,

	/* 8-bit unsigned value to configure the tx beamformee number of
	 * space-time streams.
	 * This attribute is used to configure the testbed device.
	 * The range of the value is 0 to 8
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_TX_BEAMFORMEE_NSTS = 14,

	/* 8-bit unsigned value to configure the MU EDCA params for given AC
	 * This attribute is used to configure the testbed device.
	 * Uses the enum qca_wlan_ac_type values.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_MU_EDCA_AC = 15,

	/* 8-bit unsigned value to configure the MU EDCA AIFSN for given AC
	 * To configure MU EDCA AIFSN value, MU EDCA access category value
	 * is required to process the command.
	 * This attribute is used to configure the testbed device.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_MU_EDCA_AIFSN = 16,

	/* 8-bit unsigned value to configure the MU EDCA ECW min value for
	 * given AC.
	 * To configure MU EDCA ECW min value, MU EDCA access category value
	 * is required to process the command.
	 * This attribute is used to configure the testbed device.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_MU_EDCA_ECWMIN = 17,

	/* 8-bit unsigned value to configure the MU EDCA ECW max value for
	 * given AC.
	 * To configure MU EDCA ECW max value, MU EDCA access category value
	 * is required to process the command.
	 * This attribute is used to configure the testbed device.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_MU_EDCA_ECWMAX = 18,

	/* 8-bit unsigned value to configure the MU EDCA timer for given AC
	 * To configure MU EDCA timer value, MU EDCA access category value
	 * is required to process the command.
	 * This attribute is used to configure the testbed device.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_MU_EDCA_TIMER = 19,

	/* 8-bit unsigned value to configure the HE trigger frame MAC padding
	 * duration.
	 * This attribute is used to configure the testbed device.
	 * Uses the enum qca_wlan_he_mac_padding_dur values.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_MAC_PADDING_DUR = 20,

	/* 8-bit unsigned value to override the MU EDCA params to defaults
	 * regardless of the AP beacon MU EDCA params. If it is enabled use
	 * the default values else use the MU EDCA params from AP beacon.
	 * This attribute is used to configure the testbed device.
	 * 1 - enable override, 0 - disable.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_OVERRIDE_MU_EDCA = 21,

	/* 8-bit unsigned value to configure the support for receiving
	 * an MPDU that contains an operating mode control subfield.
	 * This attribute is used to configure the testbed device.
	 * 1-enable, 0-disable.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_OM_CTRL_SUPP = 22,

	/* Nested attribute values required to setup the TWT session.
	 * enum qca_wlan_vendor_attr_twt_setup provides the necessary
	 * information to set up the session. It contains broadcast flags,
	 * set_up flags, trigger value, flow type, flow ID, wake interval
	 * exponent, protection, target wake time, wake duration, wake interval
	 * mantissa. These nested attributes are used to setup a host triggered
	 * TWT session.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_TWT_SETUP = 23,

	/* This nested attribute is used to terminate the current TWT session.
	 * It does not currently carry any attributes.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_TWT_TERMINATE = 24,

	/* This nested attribute is used to suspend the current TWT session.
	 * It does not currently carry any attributes.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_TWT_SUSPEND = 25,

	/* Nested attribute values to indicate the request for resume.
	 * This attribute is used to resume the TWT session.
	 * enum qca_wlan_vendor_attr_twt_resume provides the necessary
	 * parameters required to resume the TWT session.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_TWT_RESUME = 26,

	/* 8-bit unsigned value to set the HE operating mode control
	 * (OM CTRL) Channel Width subfield.
	 * The Channel Width subfield indicates the operating channel width
	 * supported by the STA for both reception and transmission.
	 * Uses the enum qca_wlan_he_om_ctrl_ch_bw values.
	 * This setting is cleared with the
	 * QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_CLEAR_HE_OM_CTRL_CONFIG
	 * flag attribute to reset defaults.
	 * This attribute is used to configure the testbed device.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_OM_CTRL_BW = 27,

	/* 8-bit unsigned value to configure the number of spatial
	 * streams in HE operating mode control field.
	 * This setting is cleared with the
	 * QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_CLEAR_HE_OM_CTRL_CONFIG
	 * flag attribute to reset defaults.
	 * This attribute is used to configure the testbed device.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_OM_CTRL_NSS = 28,

	/* Flag attribute to configure the UL MU disable bit in
	 * HE operating mode control field.
	 * This setting is cleared with the
	 * QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_CLEAR_HE_OM_CTRL_CONFIG
	 * flag attribute to reset defaults.
	 * This attribute is used to configure the testbed device.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_OM_CTRL_UL_MU_DISABLE = 29,

	/* Flag attribute to clear the previously set HE operating mode
	 * control field configuration.
	 * This attribute is used to configure the testbed device to reset
	 * defaults to clear any previously set HE operating mode control
	 * field configuration.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_CLEAR_HE_OM_CTRL_CONFIG = 30,

	/* 8-bit unsigned value to configure HE single user PPDU
	 * transmission. By default this setting is disabled and it
	 * is disabled in the reset defaults of the device configuration.
	 * This attribute is used to configure the testbed device.
	 * 1-enable, 0-disable
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_TX_SUPPDU = 31,

	/* 8-bit unsigned value to configure action frame transmission
	 * in HE trigger based PPDU transmission.
	 * By default this setting is disabled and it is disabled in
	 * the reset defaults of the device configuration.
	 * This attribute is used to configure the testbed device.
	 * 1-enable, 0-disable
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_ACTION_TX_TB_PPDU = 32,

	/* Nested attribute to indicate HE operating mode control field
	 * transmission. It contains operating mode control field Nss,
	 * channel bandwidth, Tx Nsts and UL MU disable attributes.
	 * These nested attributes are used to send HE operating mode control
	 * with configured values.
	 * Uses the enum qca_wlan_vendor_attr_he_omi_tx attributes.
	 * This attribute is used to configure the testbed device.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_OMI_TX = 33,

	/* 8-bit unsigned value to configure +HTC_HE support to indicate the
	 * support for the reception of a frame that carries an HE variant
	 * HT Control field.
	 * This attribute is used to configure the testbed device.
	 * 1-enable, 0-disable
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_HTC_HE_SUPP = 34,

	/* 8-bit unsigned value to configure VHT support in 2.4G band.
	 * This attribute is used to configure the testbed device.
	 * 1-enable, 0-disable
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ENABLE_2G_VHT = 35,

	/* 8-bit unsigned value to configure HE testbed defaults.
	 * This attribute is used to configure the testbed device.
	 * 1-set the device HE capabilities to testbed defaults.
	 * 0-reset the device HE capabilities to supported config.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_SET_HE_TESTBED_DEFAULTS = 36,

	/* 8-bit unsigned value to configure twt request support.
	 * This attribute is used to configure the testbed device.
	 * 1-enable, 0-disable.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_TWT_REQ_SUPPORT = 37,

	/* 8-bit unsigned value to configure protection for Management
	 * frames when PMF is enabled for the association.
	 * This attribute is used to configure the testbed device.
	 * 0-use the correct key, 1-use an incorrect key, 2-disable protection.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_PMF_PROTECTION = 38,

	/* Flag attribute to inject Disassociation frame to the connected AP.
	 * This attribute is used to configure the testbed device.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_DISASSOC_TX = 39,

	/* 8-bit unsigned value to configure an override for the RSNXE Used
	 * subfield in the MIC control field of the FTE in FT Reassociation
	 * Request frame.
	 * 0 - Default behavior, 1 - override with 1, 2 - override with 0.
	 * This attribute is used to configure the testbed device.
	 * This attribute can be configured when STA is in disconnected or
	 * associated state and the configuration is valid until the next
	 * disconnection.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_FT_REASSOCREQ_RSNXE_USED = 40,

	/* 8-bit unsigned value to configure the driver to ignore CSA (Channel
	 * Switch Announcement) when STA is in connected state.
	 * 0 - Default behavior, 1 - Ignore CSA.
	 * This attribute is used to configure the testbed device.
	 * This attribute can be configured when STA is in disconnected or
	 * associated state and the configuration is valid until the next
	 * disconnection.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_IGNORE_CSA = 41,

	/* Nested attribute values required to configure OCI (Operating Channel
	 * Information). Attributes defined in enum
	 * qca_wlan_vendor_attr_oci_override are nested within this attribute.
	 * This attribute is used to configure the testbed device.
	 * This attribute can be configured when STA is in disconnected or
	 * associated state and the configuration is valid until the next
	 * disconnection.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_OCI_OVERRIDE = 42,

	/* 8-bit unsigned value to configure the driver/firmware to ignore SA
	 * Query timeout. If this configuration is enabled STA shall not send
	 * Deauthentication frmae when SA Query times out (mainly, after a
	 * channel switch when OCV is enabled).
	 * 0 - Default behavior, 1 - Ignore SA Query timeout.
	 * This attribute is used to configure the testbed device.
	 * This attribute can be configured only when STA is in associated state
	 * and the configuration is valid until the disconnection.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_IGNORE_SA_QUERY_TIMEOUT = 43,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_MAX =
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_twt_operation - Operation of the config TWT request
 * Values for %QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_OPERATION.
 *
 * @QCA_WLAN_TWT_SET: Setup a TWT session. Required parameters are configured
 * through QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_PARAMS. Refers the enum
 * qca_wlan_vendor_attr_twt_setup.
 *
 * @QCA_WLAN_TWT_GET: Get the configured TWT parameters. Required parameters are
 * obtained through QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_PARAMS. Refers the enum
 * qca_wlan_vendor_attr_twt_setup.
 *
 * @QCA_WLAN_TWT_TERMINATE: Terminate the TWT session. Required parameters are
 * obtained through QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_PARAMS. Refers the enum
 * qca_wlan_vendor_attr_twt_setup. Valid only after the TWT session is setup.
 *
 * @QCA_WLAN_TWT_SUSPEND: Suspend the TWT session. Required parameters are
 * obtained through QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_PARAMS. Refers the enum
 * qca_wlan_vendor_attr_twt_setup. Valid only after the TWT session is setup.
 *
 * @QCA_WLAN_TWT_RESUME: Resume the TWT session. Required parameters are
 * configured through QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_PARAMS. Refers the enum
 * qca_wlan_vendor_attr_twt_resume. Valid only after the TWT session is setup.
 */
enum qca_wlan_twt_operation {
	QCA_WLAN_TWT_SET = 0,
	QCA_WLAN_TWT_GET = 1,
	QCA_WLAN_TWT_TERMINATE = 2,
	QCA_WLAN_TWT_SUSPEND = 3,
	QCA_WLAN_TWT_RESUME = 4,
};

/* enum qca_wlan_vendor_attr_config_twt: Defines attributes used by
 * %QCA_NL80211_VENDOR_SUBCMD_CONFIG_TWT
 *
 * @QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_OPERATION:
 * u8 attribute. Specify the TWT operation of this request. Possible values
 * are defined in enum qca_wlan_twt_operation. The parameters for the
 * respective operation is specified through
 * QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_PARAMS.
 *
 * @QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_PARAMS: Nested attribute representing the
 * parameters configured for TWT. These parameters are represented by
 * enum qca_wlan_vendor_attr_twt_setup or enum qca_wlan_vendor_attr_twt_resume
 * based on the operation.
 */
enum qca_wlan_vendor_attr_config_twt {
	QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_OPERATION = 1,
	QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_PARAMS = 2,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_MAX =
	QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_twt_setup: Represents attributes for
 * TWT (Target Wake Time) setup request. These attributes are sent as part of
 * %QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_TWT_SETUP and
 * %QCA_NL80211_VENDOR_SUBCMD_WIFI_TEST_CONFIGURATION. Also used by
 * attributes through %QCA_NL80211_VENDOR_SUBCMD_CONFIG_TWT.
 *
 * @QCA_WLAN_VENDOR_ATTR_TWT_SETUP_BCAST: Flag attribute.
 * Disable (flag attribute not present) - Individual TWT
 * Enable (flag attribute present) - Broadcast TWT.
 * Individual means the session is between the STA and the AP.
 * This session is established using a separate negotiation between
 * STA and AP.
 * Broadcast means the session is across multiple STAs and an AP. The
 * configuration parameters are announced in Beacon frames by the AP.
 * This is used in
 * 1. TWT SET Request and Response
 * 2. TWT GET Response
 *
 * @QCA_WLAN_VENDOR_ATTR_TWT_SETUP_REQ_TYPE: Required (u8).
 * Unsigned 8-bit qca_wlan_vendor_twt_setup_req_type to
 * specify the TWT request type. This is used in TWT SET operation.
 *
 * @QCA_WLAN_VENDOR_ATTR_TWT_SETUP_TRIGGER: Flag attribute
 * Enable (flag attribute present) - TWT with trigger support.
 * Disable (flag attribute not present) - TWT without trigger support.
 * Trigger means the AP will send the trigger frame to allow STA to send data.
 * Without trigger, the STA will wait for the MU EDCA timer before
 * transmitting the data.
 * This is used in
 * 1. TWT SET Request and Response
 * 2. TWT GET Response
 *
 * @QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_TYPE: Required (u8)
 * 0 - Announced TWT - In this mode, STA may skip few service periods to
 * save more power. If STA wants to wake up, it will send a PS-POLL/QoS
 * NULL frame to AP.
 * 1 - Unannounced TWT - The STA will wakeup during every SP.
 * This is a required parameter for
 * 1. TWT SET Request and Response
 * 2. TWT GET Response
 *
 * @QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_ID: Optional (u8)
 * Flow ID is the unique identifier for each TWT session.
 * If not provided then dialog ID will be set to zero.
 * This is an optional parameter for
 * 1. TWT SET Request and Response
 * 2. TWT GET Request and Response
 * 3. TWT TERMINATE Request and Response
 * 4. TWT SUSPEND Request and Response
 *
 * @QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_INTVL_EXP: Required (u8)
 * This attribute (exp) is used along with the mantissa to derive the
 * wake interval using the following formula:
 * pow(2,exp) = wake_intvl_us/wake_intvl_mantis
 * Wake interval is the interval between 2 successive SP.
 * This is a required parameter for
 * 1. TWT SET Request and Response
 * 2. TWT GET Response
 *
 * @QCA_WLAN_VENDOR_ATTR_TWT_SETUP_PROTECTION: Flag attribute
 * Enable (flag attribute present) - Protection required.
 * Disable (flag attribute not present) - Protection not required.
 * If protection is enabled, then the AP will use protection
 * mechanism using RTS/CTS to self to reserve the airtime.
 * This is used in
 * 1. TWT SET Request and Response
 * 2. TWT GET Response
 *
 * @QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_TIME: Optional (u32)
 * This attribute is used as the SP offset which is the offset from
 * TSF after which the wake happens. The units are in microseconds. If
 * this attribute is not provided, then the value will be set to zero.
 * This is an optional parameter for
 * 1. TWT SET Request and Response
 * 2. TWT GET Response
 *
 * @QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_DURATION: Required (u32)
 * This is the duration of the service period. This is specified as
 * multiples of 256 microseconds. Valid values are 0x1 to 0xFF.
 * This is a required parameter for
 * 1. TWT SET Request and Response
 * 2. TWT GET Response
 *
 * @QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_INTVL_MANTISSA: Required (u32)
 * This attribute is used to configure wake interval mantissa.
 * The units are in TU.
 * This is a required parameter for
 * 1. TWT SET Request and Response
 * 2. TWT GET Response
 *
 * @QCA_WLAN_VENDOR_ATTR_TWT_SETUP_STATUS: Required (u8)
 * This field is applicable for TWT response only.
 * This contains status values in enum qca_wlan_vendor_twt_status
 * and is passed to the userspace. This is used in TWT SET operation.
 * This is a required parameter for
 * 1. TWT SET Response
 * 2. TWT TERMINATE Response
 * 3. TWT SUSPEND Response
 * 4. TWT RESUME Response
 *
 * @QCA_WLAN_VENDOR_ATTR_TWT_SETUP_RESP_TYPE: Required (u8)
 * This field is applicable for TWT response only.
 * This field contains response type from the TWT responder and is
 * passed to the userspace. The values for this field are defined in
 * enum qca_wlan_vendor_twt_setup_resp_type. This is used in TWT SET
 * response.
 *
 * @QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_TIME_TSF: Required (u64)
 * This field is applicable for TWT response only.
 * This field contains absolute TSF value of the wake time received
 * from the TWT responder and is passed to the userspace.
 * This is a required parameter for
 * 1. TWT SET Response
 * 2. TWT GET Response
 *
 * @QCA_WLAN_VENDOR_ATTR_TWT_SETUP_TWT_INFO_ENABLED: Flag attribute.
 * Enable (flag attribute present) - Indicates that the TWT responder
 * supports reception of TWT information frame from the TWT requestor.
 * Disable (flag attribute not present) - Indicates that the responder
 * doesn't support reception of TWT information frame from requestor.
 * This is used in
 * 1. TWT SET Response
 * 2. TWT GET Response
 *
 * @QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAC_ADDR: 6-byte MAC address
 * Represents the MAC address of the peer for which the TWT session
 * is being configured. This is used in AP mode to represent the respective
 * client. In AP mode, this is an optional parameter for response and is
 * a required parameter for
 * 1. TWT SET Request
 * 2. TWT GET Request
 * 3. TWT TERMINATE Request
 * 4. TWT SUSPEND Request
 * In STA mode, this is an optional parameter in request and response for
 * the above four TWT operations.
 */
enum qca_wlan_vendor_attr_twt_setup {
	QCA_WLAN_VENDOR_ATTR_TWT_SETUP_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_TWT_SETUP_BCAST = 1,
	QCA_WLAN_VENDOR_ATTR_TWT_SETUP_REQ_TYPE = 2,
	QCA_WLAN_VENDOR_ATTR_TWT_SETUP_TRIGGER = 3,
	QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_TYPE = 4,
	QCA_WLAN_VENDOR_ATTR_TWT_SETUP_FLOW_ID = 5,
	QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_INTVL_EXP = 6,
	QCA_WLAN_VENDOR_ATTR_TWT_SETUP_PROTECTION = 7,
	QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_TIME = 8,
	QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_DURATION = 9,
	QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_INTVL_MANTISSA = 10,

	/* TWT Response only attributes */
	QCA_WLAN_VENDOR_ATTR_TWT_SETUP_STATUS = 11,
	QCA_WLAN_VENDOR_ATTR_TWT_SETUP_RESP_TYPE = 12,
	QCA_WLAN_VENDOR_ATTR_TWT_SETUP_WAKE_TIME_TSF = 13,
	QCA_WLAN_VENDOR_ATTR_TWT_SETUP_TWT_INFO_ENABLED = 14,

	QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAC_ADDR = 15,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_TWT_SETUP_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_TWT_SETUP_MAX =
	QCA_WLAN_VENDOR_ATTR_TWT_SETUP_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_twt_status - Represents the status of the requested
 * TWT operation
 *
 * @QCA_WLAN_VENDOR_TWT_STATUS_OK: TWT request successfully completed
 * @QCA_WLAN_VENDOR_TWT_STATUS_TWT_NOT_ENABLED: TWT not enabled
 * @QCA_WLAN_VENDOR_TWT_STATUS_USED_DIALOG_ID: TWT dialog ID is already used
 * @QCA_WLAN_VENDOR_TWT_STATUS_SESSION_BUSY: TWT session is busy
 * @QCA_WLAN_VENDOR_TWT_STATUS_SESSION_NOT_EXIST: TWT session does not exist
 * @QCA_WLAN_VENDOR_TWT_STATUS_NOT_SUSPENDED: TWT session not in suspend state
 * @QCA_WLAN_VENDOR_TWT_STATUS_INVALID_PARAM: Invalid parameters
 * @QCA_WLAN_VENDOR_TWT_STATUS_NOT_READY: FW not ready
 * @QCA_WLAN_VENDOR_TWT_STATUS_NO_RESOURCE: FW resource exhausted
 * @QCA_WLAN_VENDOR_TWT_STATUS_NO_ACK: Peer AP/STA did not ACK the
 * request/response frame
 * @QCA_WLAN_VENDOR_TWT_STATUS_NO_RESPONSE: Peer AP did not send the response
 * frame
 * @QCA_WLAN_VENDOR_TWT_STATUS_DENIED: AP did not accept the request
 * @QCA_WLAN_VENDOR_TWT_STATUS_UNKNOWN_ERROR: Adding TWT dialog failed due to an
 * unknown reason
 * @QCA_WLAN_VENDOR_TWT_STATUS_ALREADY_SUSPENDED: TWT session already in
 * suspend state
 */
enum qca_wlan_vendor_twt_status {
	QCA_WLAN_VENDOR_TWT_STATUS_OK = 0,
	QCA_WLAN_VENDOR_TWT_STATUS_TWT_NOT_ENABLED = 1,
	QCA_WLAN_VENDOR_TWT_STATUS_USED_DIALOG_ID = 2,
	QCA_WLAN_VENDOR_TWT_STATUS_SESSION_BUSY = 3,
	QCA_WLAN_VENDOR_TWT_STATUS_SESSION_NOT_EXIST = 4,
	QCA_WLAN_VENDOR_TWT_STATUS_NOT_SUSPENDED = 5,
	QCA_WLAN_VENDOR_TWT_STATUS_INVALID_PARAM = 6,
	QCA_WLAN_VENDOR_TWT_STATUS_NOT_READY = 7,
	QCA_WLAN_VENDOR_TWT_STATUS_NO_RESOURCE = 8,
	QCA_WLAN_VENDOR_TWT_STATUS_NO_ACK = 9,
	QCA_WLAN_VENDOR_TWT_STATUS_NO_RESPONSE = 10,
	QCA_WLAN_VENDOR_TWT_STATUS_DENIED = 11,
	QCA_WLAN_VENDOR_TWT_STATUS_UNKNOWN_ERROR = 12,
	QCA_WLAN_VENDOR_TWT_STATUS_ALREADY_SUSPENDED = 13,
};

/**
 * enum qca_wlan_vendor_attr_twt_resume - Represents attributes for
 * TWT (Target Wake Time) resume request. These attributes are sent as part of
 * %QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_TWT_RESUME and
 * %QCA_NL80211_VENDOR_SUBCMD_WIFI_TEST_CONFIGURATION. Also used by
 * attributes through %QCA_NL80211_VENDOR_SUBCMD_CONFIG_TWT.
 *
 * @QCA_WLAN_VENDOR_ATTR_TWT_RESUME_NEXT_TWT: Optional (u8)
 * @QCA_WLAN_VENDOR_ATTR_TWT_RESUME_NEXT2_TWT: Optional (u32)
 * These attributes are used as the SP offset which is the offset from TSF after
 * which the wake happens. The units are in microseconds. Please note that
 * _NEXT_TWT is limited to u8 whereas _NEXT2_TWT takes the u32 data.
 * _NEXT2_TWT takes the precedence over _NEXT_TWT and thus the recommendation
 * is to use _NEXT2_TWT. If neither of these attributes is provided, the value
 * will be set to zero.
 *
 * @QCA_WLAN_VENDOR_ATTR_TWT_RESUME_NEXT_TWT_SIZE: Required (u32)
 * This attribute represents the next TWT subfield size.
 * Value 0 represents 0 bits, 1 represents 32 bits, 2 for 48 bits,
 * and 4 for 64 bits.
 *
 * @QCA_WLAN_VENDOR_ATTR_TWT_RESUME_FLOW_ID: Required (u8).
 * Flow ID is the unique identifier for each TWT session. This attribute
 * represents the respective TWT session to resume.
 *
 * @QCA_WLAN_VENDOR_ATTR_TWT_RESUME_MAC_ADDR: 6-byte MAC address
 * Represents the MAC address of the peer to which TWT Resume is
 * being sent. This is used in AP mode to represent the respective
 * client and is a required parameter. In STA mode, this is an optional
 * parameter
 */
enum qca_wlan_vendor_attr_twt_resume {
	QCA_WLAN_VENDOR_ATTR_TWT_RESUME_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_TWT_RESUME_NEXT_TWT = 1,
	QCA_WLAN_VENDOR_ATTR_TWT_RESUME_NEXT_TWT_SIZE = 2,
	QCA_WLAN_VENDOR_ATTR_TWT_RESUME_FLOW_ID = 3,
	QCA_WLAN_VENDOR_ATTR_TWT_RESUME_NEXT2_TWT = 4,
	QCA_WLAN_VENDOR_ATTR_TWT_RESUME_MAC_ADDR = 5,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_TWT_RESUME_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_TWT_RESUME_MAX =
	QCA_WLAN_VENDOR_ATTR_TWT_RESUME_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_twt_setup_req_type - Required (u8)
 * Represents the setup type being requested for TWT.
 * @QCA_WLAN_VENDOR_TWT_SETUP_REQUEST: STA is not specifying all the TWT
 * parameters but relying on AP to fill the parameters during the negotiation.
 * @QCA_WLAN_VENDOR_TWT_SETUP_SUGGEST: STA will provide all the suggested
 * values which the AP may accept or AP may provide alternative parameters
 * which the STA may accept.
 * @QCA_WLAN_VENDOR_TWT_SETUP_DEMAND: STA is not willing to accept any
 * alternate parameters than the requested ones.
 */
enum qca_wlan_vendor_twt_setup_req_type {
	QCA_WLAN_VENDOR_TWT_SETUP_REQUEST = 1,
	QCA_WLAN_VENDOR_TWT_SETUP_SUGGEST = 2,
	QCA_WLAN_VENDOR_TWT_SETUP_DEMAND = 3,
};

/**
 * enum qca_wlan_vendor_twt_setup_resp_type - Represents the response type by
 * the TWT responder
 *
 * @QCA_WLAN_VENDOR_TWT_RESP_ALTERNATE: TWT responder suggests TWT
 * parameters that are different from TWT requesting STA suggested
 * or demanded TWT parameters
 * @QCA_WLAN_VENDOR_TWT_RESP_DICTATE: TWT responder demands TWT
 * parameters that are different from TWT requesting STA TWT suggested
 * or demanded parameters
 * @QCA_WLAN_VENDOR_TWT_RESP_REJECT: TWT responder rejects TWT
 * setup
 * @QCA_WLAN_VENDOR_TWT_RESP_ACCEPT: TWT responder accepts the TWT
 * setup.
 */
enum qca_wlan_vendor_twt_setup_resp_type {
	QCA_WLAN_VENDOR_TWT_RESP_ALTERNATE = 1,
	QCA_WLAN_VENDOR_TWT_RESP_DICTATE = 2,
	QCA_WLAN_VENDOR_TWT_RESP_REJECT = 3,
	QCA_WLAN_VENDOR_TWT_RESP_ACCEPT = 4,
};

/**
 * enum qca_wlan_throughput_level - Current throughput level
 *
 * Indicates the current level of throughput calculated by driver. The driver
 * may choose different thresholds to decide whether the throughput level is
 * low or medium or high based on variety of parameters like physical link
 * capacity of current connection, number of pakcets being dispatched per
 * second etc. The throughput level events might not be consistent with the
 * actual current throughput value being observed.
 *
 * @QCA_WLAN_THROUGHPUT_LEVEL_LOW: Low level of throughput
 * @QCA_WLAN_THROUGHPUT_LEVEL_MEDIUM: Medium level of throughput
 * @QCA_WLAN_THROUGHPUT_LEVEL_HIGH: High level of throughput
 */
enum qca_wlan_throughput_level {
	QCA_WLAN_THROUGHPUT_LEVEL_LOW = 0,
	QCA_WLAN_THROUGHPUT_LEVEL_MEDIUM = 1,
	QCA_WLAN_THROUGHPUT_LEVEL_HIGH = 2,
};

/**
 * enum qca_wlan_vendor_attr_throughput_change - Vendor subcmd attributes to
 * report throughput changes from driver to user space. enum values are used
 * for NL attributes sent with
 * %QCA_NL80211_VENDOR_SUBCMD_THROUGHPUT_CHANGE_EVENT sub command.
 */
enum qca_wlan_vendor_attr_throughput_change {
	QCA_WLAN_VENDOR_ATTR_THROUGHPUT_CHANGE_INVALID = 0,
	/*
	 * Indicates the direction of throughput in which the change is being
	 * reported. u8 attribute. Value is 0 for TX and 1 for RX.
	 */
	QCA_WLAN_VENDOR_ATTR_THROUGHPUT_CHANGE_DIRECTION = 1,

	/*
	 * Indicates the newly observed throughput level.
	 * qca_wlan_throughput_level describes the possible range of values.
	 * u8 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_THROUGHPUT_CHANGE_THROUGHPUT_LEVEL = 2,

	/*
	 * Indicates the driver's guidance on the new value to be set to
	 * kernel's tcp parameter tcp_limit_output_bytes. u32 attribute. Driver
	 * may optionally include this attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_THROUGHPUT_CHANGE_TCP_LIMIT_OUTPUT_BYTES = 3,

	/*
	 * Indicates the driver's guidance on the new value to be set to
	 * kernel's tcp parameter tcp_adv_win_scale. s8 attribute. Possible
	 * values are from -31 to 31. Driver may optionally include this
	 * attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_THROUGHPUT_CHANGE_TCP_ADV_WIN_SCALE = 4,

	/*
	 * Indicates the driver's guidance on the new value to be set to
	 * kernel's tcp parameter tcp_delack_seg. u32 attribute. Driver may
	 * optionally include this attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_THROUGHPUT_CHANGE_TCP_DELACK_SEG = 5,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_THROUGHPUT_CHANGE_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_THROUGHPUT_CHANGE_MAX =
	QCA_WLAN_VENDOR_ATTR_THROUGHPUT_CHANGE_AFTER_LAST - 1,
};

/* enum qca_wlan_nan_subcmd_type - Type of NAN command used by attribute
 * QCA_WLAN_VENDOR_ATTR_NAN_SUBCMD_TYPE as a part of vendor command
 * QCA_NL80211_VENDOR_SUBCMD_NAN_EXT.
 */
enum qca_wlan_nan_ext_subcmd_type {
	/* Subcmd of type NAN Enable Request */
	QCA_WLAN_NAN_EXT_SUBCMD_TYPE_ENABLE_REQ = 1,
	/* Subcmd of type NAN Disable Request */
	QCA_WLAN_NAN_EXT_SUBCMD_TYPE_DISABLE_REQ = 2,
};

/**
 * enum qca_wlan_vendor_attr_nan_params - Used by the vendor command
 * QCA_NL80211_VENDOR_SUBCMD_NAN_EXT.
 */
enum qca_wlan_vendor_attr_nan_params {
	QCA_WLAN_VENDOR_ATTR_NAN_INVALID = 0,
	/* Carries NAN command for firmware component. Every vendor command
	 * QCA_NL80211_VENDOR_SUBCMD_NAN_EXT must contain this attribute with a
	 * payload containing the NAN command. NLA_BINARY attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_NAN_CMD_DATA = 1,
	/* Indicates the type of NAN command sent with
	 * QCA_NL80211_VENDOR_SUBCMD_NAN_EXT. enum qca_wlan_nan_ext_subcmd_type
	 * describes the possible range of values. This attribute is mandatory
	 * if the command being issued is either
	 * QCA_WLAN_NAN_EXT_SUBCMD_TYPE_ENABLE_REQ or
	 * QCA_WLAN_NAN_EXT_SUBCMD_TYPE_DISABLE_REQ. NLA_U32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_NAN_SUBCMD_TYPE = 2,
	/* Frequency (in MHz) of primary NAN discovery social channel in 2.4 GHz
	 * band. This attribute is mandatory when command type is
	 * QCA_WLAN_NAN_EXT_SUBCMD_TYPE_ENABLE_REQ. NLA_U32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_NAN_DISC_24GHZ_BAND_FREQ = 3,
	/* Frequency (in MHz) of secondary NAN discovery social channel in 5 GHz
	 * band. This attribute is optional and should be included when command
	 * type is QCA_WLAN_NAN_EXT_SUBCMD_TYPE_ENABLE_REQ and NAN discovery
	 * has to be started on 5GHz along with 2.4GHz. NLA_U32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_NAN_DISC_5GHZ_BAND_FREQ = 4,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_NAN_PARAMS_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_NAN_PARAMS_MAX =
		QCA_WLAN_VENDOR_ATTR_NAN_PARAMS_AFTER_LAST - 1
};

/**
 * enum qca_wlan_vendor_cfr_method - QCA vendor CFR methods used by
 * attribute QCA_WLAN_VENDOR_ATTR_PEER_CFR_METHOD as part of vendor
 * command QCA_NL80211_VENDOR_SUBCMD_PEER_CFR_CAPTURE_CFG.
 * @QCA_WLAN_VENDOR_CFR_METHOD_QOS_NULL: CFR method using QOS Null frame.
 * @QCA_WLAN_VENDOR_CFR_QOS_NULL_WITH_PHASE: CFR method using QOS Null frame
 * with phase
 * @QCA_WLAN_VENDOR_CFR_PROBE_RESPONSE: CFR method using probe response frame
 */
enum qca_wlan_vendor_cfr_method {
	QCA_WLAN_VENDOR_CFR_METHOD_QOS_NULL = 0,
	QCA_WLAN_VENDOR_CFR_QOS_NULL_WITH_PHASE = 1,
	QCA_WLAN_VENDOR_CFR_PROBE_RESPONSE = 2,
};

/**
 * enum qca_wlan_vendor_cfr_capture_type - QCA vendor CFR capture type used by
 * attribute QCA_WLAN_VENDOR_ATTR_PEER_CFR_CAPTURE_TYPE.
 * @QCA_WLAN_VENDOR_CFR_DIRECT_FTM: Filter directed FTM ACK frames.
 * @QCA_WLAN_VENDOR_CFR_ALL_FTM_ACK: Filter all FTM ACK frames.
 * @QCA_WLAN_VENDOR_CFR_DIRECT_NDPA_NDP: Filter NDPA NDP directed frames.
 * @QCA_WLAN_VENDOR_CFR_TA_RA: Filter frames based on TA/RA/Subtype which
 * is provided by one or more of below attributes:
 *	%QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_TA
 *	%QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_RA
 *	%QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_TA_MASK
 *	%QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_RA_MASK
 *	%QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_MGMT_FILTER
 *	%QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_CTRL_FILTER
 *	%QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_DATA_FILTER
 * @QCA_WLAN_CFR_ALL_PACKET: Filter all packets.
 * @QCA_WLAN_VENDOR_CFR_NDPA_NDP_ALL: Filter all NDPA NDP frames.
 */
enum qca_wlan_vendor_cfr_capture_type {
	QCA_WLAN_VENDOR_CFR_DIRECT_FTM = 0,
	QCA_WLAN_VENDOR_CFR_ALL_FTM_ACK = 1,
	QCA_WLAN_VENDOR_CFR_DIRECT_NDPA_NDP = 2,
	QCA_WLAN_VENDOR_CFR_TA_RA = 3,
	QCA_WLAN_VENDOR_CFR_ALL_PACKET = 4,
	QCA_WLAN_VENDOR_CFR_NDPA_NDP_ALL = 5,
};

/**
 * enum qca_wlan_vendor_peer_cfr_capture_attr - Used by the vendor command
 * QCA_NL80211_VENDOR_SUBCMD_PEER_CFR_CAPTURE_CFG to configure peer
 * Channel Frequency Response capture parameters and enable periodic CFR
 * capture.
 *
 * @QCA_WLAN_VENDOR_ATTR_CFR_PEER_MAC_ADDR: Optional (6-byte MAC address)
 * MAC address of peer. This is for CFR version 1 only.
 *
 * @QCA_WLAN_VENDOR_ATTR_PEER_CFR_ENABLE: Required (flag)
 * Enable peer CFR Capture. This attribute is mandatory to
 * enable peer CFR capture. If this attribute is not present,
 * peer CFR capture is disabled.
 *
 * @QCA_WLAN_VENDOR_ATTR_PEER_CFR_BANDWIDTH: Optional (u8)
 * BW of measurement, attribute uses the values in enum nl80211_chan_width
 * Supported values: 20, 40, 80, 80+80, 160.
 * Note that all targets may not support all bandwidths.
 * This attribute is mandatory for version 1 if attribute
 * QCA_WLAN_VENDOR_ATTR_PEER_CFR_ENABLE is used.
 *
 * @QCA_WLAN_VENDOR_ATTR_PEER_CFR_PERIODICITY: Optional (u32)
 * Periodicity of CFR measurement in msec.
 * Periodicity should be a multiple of Base timer.
 * Current Base timer value supported is 10 msecs (default).
 * 0 for one shot capture.
 * This attribute is mandatory for version 1 if attribute
 * QCA_WLAN_VENDOR_ATTR_PEER_CFR_ENABLE is used.
 *
 * @QCA_WLAN_VENDOR_ATTR_PEER_CFR_METHOD: Optional (u8)
 * Method used to capture Channel Frequency Response.
 * Attribute uses the values defined in enum qca_wlan_vendor_cfr_method.
 * This attribute is mandatory for version 1 if attribute
 * QCA_WLAN_VENDOR_ATTR_PEER_CFR_ENABLE is used.
 *
 * @QCA_WLAN_VENDOR_ATTR_PERIODIC_CFR_CAPTURE_ENABLE: Optional (flag)
 * Enable periodic CFR capture.
 * This attribute is mandatory for version 1 to enable Periodic CFR capture.
 * If this attribute is not present, periodic CFR capture is disabled.
 *
 * @QCA_WLAN_VENDOR_ATTR_CFR_VERSION: Optional (u8)
 * Value is 1 or 2 since there are two versions of CFR capture. Two versions
 * can't be enabled at same time. This attribute is mandatory if target
 * support both versions and use one of them.
 *
 * @QCA_WLAN_VENDOR_ATTR_CFR_ENABLE_GROUP_BITMAP: Optional (u32)
 * This attribute is mandatory for version 2 if
 * QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_ENTRY is used.
 * Bits 15:0 Bit fields indicating which group to be enabled.
 * Bits 31:16 Reserved for future use.
 *
 * @QCA_WLAN_VENDOR_ATTR_PEER_CFR_DURATION: Optional (u32)
 * CFR capture duration in microsecond. This attribute is mandatory for
 * version 2 if attribute QCA_WLAN_VENDOR_ATTR_PEER_CFR_INTERVAL is used.
 *
 * @QCA_WLAN_VENDOR_ATTR_PEER_CFR_INTERVAL: Optional (u32)
 * CFR capture interval in microsecond. This attribute is mandatory for
 * version 2 if attribute QCA_WLAN_VENDOR_ATTR_PEER_CFR_DURATION is used.
 *
 * @QCA_WLAN_VENDOR_ATTR_PEER_CFR_CAPTURE_TYPE: Optional (u32)
 * CFR capture type is defined in enum qca_wlan_vendor_cfr_capture_type.
 * This attribute is mandatory for version 2.
 *
 * @QCA_WLAN_VENDOR_ATTR_PEER_CFR_UL_MU_MASK: Optional (u64)
 * Bit fields indicating which user in the current UL MU
 * transmissions are enabled for CFR capture. Bits 36 to 0 indicating
 * user indexes for 37 users in a UL MU transmission. If bit 0 is set,
 * then the CFR capture will happen for user index 0 in the current
 * UL MU transmission. If bits 0,2 are set, then CFR capture for UL MU
 * TX corresponds to user indices 0 and 2.  Bits 63:37 Reserved for future use.
 * This is for CFR version 2 only.
 *
 * @QCA_WLAN_VENDOR_ATTR_PEER_CFR_FREEZE_TLV_DELAY_COUNT: Optional (u32)
 * Indicates the number of consecutive Rx packets to be skipped
 * before CFR capture is enabled again.
 * This is for CFR version 2 only.
 *
 * @QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_TABLE: Nested attribute containing
 * one or more %QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_ENTRY attributes.
 *
 * @QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_ENTRY: Nested attribute containing
 * the following GROUP attributes:
 *	%QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_NUMBER,
 *	%QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_TA,
 *	%QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_RA,
 *	%QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_TA_MASK,
 *	%QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_RA_MASK,
 *	%QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_NSS,
 *	%QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_BW,
 *	%QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_MGMT_FILTER,
 *	%QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_CTRL_FILTER,
 *	%QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_DATA_FILTER
 *
 * @QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_NUMBER: Optional (u32)
 * Target support multiple groups for some configurations. Group number could be
 * any value between 0 and 15. This is for CFR version 2 only.
 *
 * @QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_TA: Optional (6-byte MAC address)
 * Transmitter address which is used to filter packets, this MAC address takes
 * effect with QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_TA_MASK.
 * This is for CFR version 2 only.
 *
 * @QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_RA: Optional (6-byte MAC address)
 * Receiver address which is used to filter packets, this MAC address takes
 * effect with QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_RA_MASK.
 * This is for CFR version 2 only.
 *
 * @QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_TA_MASK: Optional (6-byte MAC address)
 * Mask of transmitter address which is used to filter packets.
 * This is for CFR version 2 only.
 *
 * @QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_RA_MASK: Optional (6-byte MAC address)
 * Mask of receiver address which is used to filter packets.
 * This is for CFR version 2 only.
 *
 * @QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_NSS: Optional (u32)
 * Indicates packets with a specific NSS will be filtered for CFR capture.
 * This is for CFR version 2 only. This is a bitmask. Bits 7:0, CFR capture will
 * be done for packets matching the NSS specified within this bitmask.
 * Bits 31:8 Reserved for future use. Bits 7:0 map to NSS:
 *     bit 0 : NSS 1
 *     bit 1 : NSS 2
 *     ...
 *     bit 7 : NSS 8
 *
 * @QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_BW: Optional (u32)
 * Indicates packets with a specific BW will be filtered for CFR capture.
 * This is for CFR version 2 only. This is a bitmask. Bits 4:0, CFR capture
 * will be done for packets matching the bandwidths specified within this
 * bitmask. Bits 31:5 Reserved for future use. Bits 4:0 map to bandwidth
 * numerated in enum nl80211_band (although not all bands may be supported
 * by a given device).
 *
 * @QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_MGMT_FILTER: Optional (u32)
 * Management packets matching the subtype filter categories will be
 * filtered in by MAC for CFR capture. This is a bitmask, in which each bit
 * represents the corresponding mgmt subtype value as per
 * IEEE 802.11(2016) 9.2.4.1.3 Type and Subtype subfields.
 * For example, beacon frame control type is 8, its value is 1<<8 = 0x100.
 * This is for CFR version 2 only
 *
 * @QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_CTRL_FILTER: Optional (u32)
 * Control packets matching the subtype filter categories will be
 * filtered in by MAC for CFR capture. This is a bitmask, in which each bit
 * represents the corresponding control subtype value as per
 * IEEE 802.11(2016) 9.2.4.1.3 Type and Subtype subfields.
 * This is for CFR version 2 only.
 *
 * @QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_DATA_FILTER: Optional (u32)
 * Data packets matching the subtype filter categories will be
 * filtered in by MAC for CFR capture. This is a bitmask, in which each bit
 * represents the corresponding data subtype value as per
 * IEEE 802.11(2016) 9.2.4.1.3 Type and Subtype subfields.
 * This is for CFR version 2 only.
 *
 */
enum qca_wlan_vendor_peer_cfr_capture_attr {
	QCA_WLAN_VENDOR_ATTR_PEER_CFR_CAPTURE_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_CFR_PEER_MAC_ADDR = 1,
	QCA_WLAN_VENDOR_ATTR_PEER_CFR_ENABLE = 2,
	QCA_WLAN_VENDOR_ATTR_PEER_CFR_BANDWIDTH = 3,
	QCA_WLAN_VENDOR_ATTR_PEER_CFR_PERIODICITY = 4,
	QCA_WLAN_VENDOR_ATTR_PEER_CFR_METHOD = 5,
	QCA_WLAN_VENDOR_ATTR_PERIODIC_CFR_CAPTURE_ENABLE = 6,
	QCA_WLAN_VENDOR_ATTR_PEER_CFR_VERSION = 7,
	QCA_WLAN_VENDOR_ATTR_PEER_CFR_ENABLE_GROUP_BITMAP = 8,
	QCA_WLAN_VENDOR_ATTR_PEER_CFR_DURATION = 9,
	QCA_WLAN_VENDOR_ATTR_PEER_CFR_INTERVAL = 10,
	QCA_WLAN_VENDOR_ATTR_PEER_CFR_CAPTURE_TYPE = 11,
	QCA_WLAN_VENDOR_ATTR_PEER_CFR_UL_MU_MASK = 12,
	QCA_WLAN_VENDOR_ATTR_PEER_CFR_FREEZE_TLV_DELAY_COUNT = 13,
	QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_TABLE = 14,
	QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_ENTRY = 15,
	QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_NUMBER = 16,
	QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_TA = 17,
	QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_RA = 18,
	QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_TA_MASK = 19,
	QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_RA_MASK = 20,
	QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_NSS = 21,
	QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_BW = 22,
	QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_MGMT_FILTER = 23,
	QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_CTRL_FILTER = 24,
	QCA_WLAN_VENDOR_ATTR_PEER_CFR_GROUP_DATA_FILTER = 25,

	/* Keep last */
	QCA_WLAN_VENDOR_ATTR_PEER_CFR_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_PEER_CFR_MAX =
	QCA_WLAN_VENDOR_ATTR_PEER_CFR_AFTER_LAST - 1,
};

/**
 * enum qca_coex_config_profiles - This enum defines different types of
 * traffic streams that can be prioritized one over the other during coex
 * scenarios.
 * The types defined in this enum are categorized in the below manner.
 * 0 - 31 values corresponds to WLAN
 * 32 - 63 values corresponds to BT
 * 64 - 95 values corresponds to Zigbee
 * @QCA_WIFI_STA_DISCOVERY: Prioritize discovery frames for WLAN STA
 * @QCA_WIFI_STA_CONNECTION: Prioritize connection frames for WLAN STA
 * @QCA_WIFI_STA_CLASS_3_MGMT: Prioritize class 3 mgmt frames for WLAN STA
 * @QCA_WIFI_STA_DATA : Prioritize data frames for WLAN STA
 * @QCA_WIFI_STA_ALL: Priritize all frames for WLAN STA
 * @QCA_WIFI_SAP_DISCOVERY: Prioritize discovery frames for WLAN SAP
 * @QCA_WIFI_SAP_CONNECTION: Prioritize connection frames for WLAN SAP
 * @QCA_WIFI_SAP_CLASS_3_MGMT: Prioritize class 3 mgmt frames for WLAN SAP
 * @QCA_WIFI_SAP_DATA: Prioritize data frames for WLAN SAP
 * @QCA_WIFI_SAP_ALL: Prioritize all frames for WLAN SAP
 * @QCA_BT_A2DP: Prioritize BT A2DP
 * @QCA_BT_BLE: Prioritize BT BLE
 * @QCA_BT_SCO: Prioritize BT SCO
 * @QCA_ZB_LOW: Prioritize Zigbee Low
 * @QCA_ZB_HIGH: Prioritize Zigbee High
 */
enum qca_coex_config_profiles {
	/* 0 - 31 corresponds to WLAN */
	QCA_WIFI_STA_DISCOVERY = 0,
	QCA_WIFI_STA_CONNECTION = 1,
	QCA_WIFI_STA_CLASS_3_MGMT = 2,
	QCA_WIFI_STA_DATA = 3,
	QCA_WIFI_STA_ALL = 4,
	QCA_WIFI_SAP_DISCOVERY = 5,
	QCA_WIFI_SAP_CONNECTION = 6,
	QCA_WIFI_SAP_CLASS_3_MGMT = 7,
	QCA_WIFI_SAP_DATA = 8,
	QCA_WIFI_SAP_ALL = 9,
	QCA_WIFI_CASE_MAX = 31,
	/* 32 - 63 corresponds to BT */
	QCA_BT_A2DP = 32,
	QCA_BT_BLE = 33,
	QCA_BT_SCO = 34,
	QCA_BT_CASE_MAX = 63,
	/* 64 - 95 corresponds to Zigbee */
	QCA_ZB_LOW = 64,
	QCA_ZB_HIGH = 65,
	QCA_ZB_CASE_MAX = 95,
	/* 0xff is default value if the u8 profile value is not set. */
	QCA_PROFILE_DEFAULT_VALUE = 255
};

/**
 * enum qca_vendor_attr_coex_config_types - Coex configurations types.
 * This enum defines the valid set of values of coex configuration types. These
 * values may used by attribute
 * %QCA_VENDOR_ATTR_COEX_CONFIG_THREE_WAY_CONFIG_TYPE.
 *
 * @QCA_WLAN_VENDOR_ATTR_COEX_CONFIG_THREE_WAY_COEX_RESET: Reset all the
 *	weights to default values.
 * @QCA_WLAN_VENDOR_ATTR_COEX_CONFIG_THREE_WAY_COEX_START: Start to config
 *	weights with configurability value.
 */
enum qca_vendor_attr_coex_config_types {
	QCA_WLAN_VENDOR_ATTR_COEX_CONFIG_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_COEX_CONFIG_THREE_WAY_COEX_RESET = 1,
	QCA_WLAN_VENDOR_ATTR_COEX_CONFIG_THREE_WAY_COEX_START = 2,
	QCA_WLAN_VENDOR_ATTR_COEX_CONFIG_TYPE_MAX
};

/**
 * enum qca_vendor_attr_coex_config_three_way - Specifies vendor coex config
 * attributes
 * Attributes for data used by
 * QCA_NL80211_VENDOR_SUBCMD_COEX_CONFIG
 *
 * QCA_VENDOR_ATTR_COEX_CONFIG_THREE_WAY_CONFIG_TYPE: u32 attribute.
 * Indicate config type.
 * the config types are 32-bit values from qca_vendor_attr_coex_config_types
 *
 * @QCA_VENDOR_ATTR_COEX_CONFIG_THREE_WAY_PRIORITY_1: u32 attribute.
 *	Indicate the Priority 1 profiles.
 *	the profiles are 8-bit values from enum qca_coex_config_profiles
 *	In same priority level, maximum to 4 profiles can be set here.
 * @QCA_VENDOR_ATTR_COEX_CONFIG_THREE_WAY_PRIORITY_2: u32 attribute.
 *	Indicate the Priority 2 profiles.
 *	the profiles are 8-bit values from enum qca_coex_config_profiles
 *	In same priority level, maximum to 4 profiles can be set here.
 * @QCA_VENDOR_ATTR_COEX_CONFIG_THREE_WAY_PRIORITY_3: u32 attribute.
 *	Indicate the Priority 3 profiles.
 *	the profiles are 8-bit values from enum qca_coex_config_profiles
 *	In same priority level, maximum to 4 profiles can be set here.
 * @QCA_VENDOR_ATTR_COEX_CONFIG_THREE_WAY_PRIORITY_4: u32 attribute.
 *	Indicate the Priority 4 profiles.
 *	the profiles are 8-bit values from enum qca_coex_config_profiles
 *	In same priority level, maximum to 4 profiles can be set here.
 * NOTE:
 * limitations for QCA_VENDOR_ATTR_COEX_CONFIG_THREE_WAY_PRIORITY_x priority
 * arrangement:
 *	1: In the same u32 attribute(priority x), the profiles enum values own
 *	same priority level.
 *	2: 0xff is default value if the u8 profile value is not set.
 *	3: max to 4 rules/profiles in same priority level.
 *	4: max to 4 priority level (priority 1 - priority 4)
 *	5: one priority level only supports one scenario from WLAN/BT/ZB,
 *	hybrid rules not support.
 *	6: if WMI_COEX_CONFIG_THREE_WAY_COEX_RESET called, priority x will
 *	remain blank to reset all parameters.
 * For example:
 *
 *	If the attributes as follow:
 *	priority 1:
 *	------------------------------------
 *	|  0xff  |    0   |   1   |    2   |
 *	------------------------------------
 *	priority 2:
 *	-------------------------------------
 *	|  0xff  |  0xff  |  0xff  |   32   |
 *	-------------------------------------
 *	priority 3:
 *	-------------------------------------
 *	|  0xff  |  0xff  |  0xff  |   65   |
 *	-------------------------------------
 *	then it means:
 *	1: WIFI_STA_DISCOVERY, WIFI_STA_CLASS_3_MGMT and WIFI_STA_CONNECTION
 *		owns same priority level.
 *	2: WIFI_STA_DISCOVERY, WIFI_STA_CLASS_3_MGMT and WIFI_STA_CONNECTION
 *		has priority over BT_A2DP and ZB_HIGH.
 *	3: BT_A2DP has priority over ZB_HIGH.
 */

enum qca_vendor_attr_coex_config_three_way {
	QCA_VENDOR_ATTR_COEX_CONFIG_THREE_WAY_INVALID = 0,
	QCA_VENDOR_ATTR_COEX_CONFIG_THREE_WAY_CONFIG_TYPE = 1,
	QCA_VENDOR_ATTR_COEX_CONFIG_THREE_WAY_PRIORITY_1 = 2,
	QCA_VENDOR_ATTR_COEX_CONFIG_THREE_WAY_PRIORITY_2 = 3,
	QCA_VENDOR_ATTR_COEX_CONFIG_THREE_WAY_PRIORITY_3 = 4,
	QCA_VENDOR_ATTR_COEX_CONFIG_THREE_WAY_PRIORITY_4 = 5,

	/* Keep last */
	QCA_VENDOR_ATTR_COEX_CONFIG_THREE_WAY_AFTER_LAST,
	QCA_VENDOR_ATTR_COEX_CONFIG_THREE_WAY_MAX =
	QCA_VENDOR_ATTR_COEX_CONFIG_THREE_WAY_AFTER_LAST - 1,
};

/**
 * enum qca_vendor_attr_peer_stats_cache_type - Represents peer stats cache type
 * This enum defines the valid set of values of peer stats cache types. These
 * values are used by attribute
 * %QCA_WLAN_VENDOR_ATTR_PEER_STATS_CACHE_TYPE_INVALID.
 *
 * @QCA_WLAN_VENDOR_ATTR_PEER_TX_RATE_STATS: Represents peer tx rate statistics.
 * @QCA_WLAN_VENDOR_ATTR_PEER_RX_RATE_STATS: Represents peer rx rate statistics.
 * @QCA_WLAN_VENDOR_ATTR_PEER_TX_SOJOURN_STATS: Represents peer tx sojourn
 * statistics
 */
enum qca_vendor_attr_peer_stats_cache_type {
	QCA_WLAN_VENDOR_ATTR_PEER_STATS_CACHE_TYPE_INVALID = 0,

	QCA_WLAN_VENDOR_ATTR_PEER_TX_RATE_STATS,
	QCA_WLAN_VENDOR_ATTR_PEER_RX_RATE_STATS,
	QCA_WLAN_VENDOR_ATTR_PEER_TX_SOJOURN_STATS,
};

/**
 * enum qca_wlan_vendor_attr_peer_stats_cache_params - This enum defines
 * attributes required for QCA_NL80211_VENDOR_SUBCMD_PEER_STATS_CACHE_FLUSH
 * Attributes are required to flush peer rate statistics from driver to
 * user application.
 *
 * @QCA_WLAN_VENDOR_ATTR_PEER_STATS_CACHE_TYPE: Unsigned 32-bit attribute
 * Indicate peer stats cache type.
 * The statistics types are 32-bit values from
 * qca_vendor_attr_peer_stats_cache_type
 * @QCA_WLAN_VENDOR_ATTR_PEER_STATS_CACHE_PEER_MAC: Unsigned 8-bit array
 * of size 6, representing peer mac address.
 * @QCA_WLAN_VENDOR_ATTR_PEER_STATS_CACHE_DATA: Opaque data attribute
 * containing buffer of statistics to send event to application layer entity.
 * @QCA_WLAN_VENDOR_ATTR_PEER_STATS_CACHE_PEER_COOKIE: Unsigned 64-bit attribute
 * representing cookie for peer unique session.
 */
enum qca_wlan_vendor_attr_peer_stats_cache_params {
	QCA_WLAN_VENDOR_ATTR_PEER_STATS_INVALID = 0,

	QCA_WLAN_VENDOR_ATTR_PEER_STATS_CACHE_TYPE = 1,
	QCA_WLAN_VENDOR_ATTR_PEER_STATS_CACHE_PEER_MAC = 2,
	QCA_WLAN_VENDOR_ATTR_PEER_STATS_CACHE_DATA = 3,
	QCA_WLAN_VENDOR_ATTR_PEER_STATS_CACHE_PEER_COOKIE = 4,

	/* Keep last */
	QCA_WLAN_VENDOR_ATTR_PEER_STATS_CACHE_LAST,
	QCA_WLAN_VENDOR_ATTR_PEER_STATS_CACHE_MAX =
		QCA_WLAN_VENDOR_ATTR_PEER_STATS_CACHE_LAST - 1
};

/**
 * enum qca_mpta_helper_attr_zigbee_state - current states of zegbee.
 * this enum defines all the possible state of zigbee, which can be
 * delivered by NetLink attribute QCA_MPTA_HELPER_VENDOR_ATTR_ZIGBEE_STATE.
 *
 * @ZIGBEE_IDLE: zigbee in idle state
 * @ZIGBEE_FORM_NETWORK: zibee forming network
 * @ZIGBEE_WAIT_JOIN: zigbee waiting for joining network
 * @ZIGBEE_JOIN: zigbee joining network
 * @ZIGBEE_NETWORK_UP: zigbee network is up
 * @ZIGBEE_HMI: zigbee in HMI mode
 */
enum qca_mpta_helper_attr_zigbee_state {
	ZIGBEE_IDLE = 0,
	ZIGBEE_FORM_NETWORK = 1,
	ZIGBEE_WAIT_JOIN = 2,
	ZIGBEE_JOIN = 3,
	ZIGBEE_NETWORK_UP = 4,
	ZIGBEE_HMI = 5,
};

/**
 * enum qca_mpta_helper_vendor_attr - used for NL attributes sent by
 * vendor sub-command QCA_NL80211_VENDOR_SUBCMD_MPTA_HELPER_CONFIG.
 */
enum qca_mpta_helper_vendor_attr {
	QCA_MPTA_HELPER_VENDOR_ATTR_INVALID = 0,
	/* Optional attribute used to update zigbee state.
	 * enum qca_mpta_helper_attr_zigbee_state.
	 * NLA_U32 attribute.
	 */
	QCA_MPTA_HELPER_VENDOR_ATTR_ZIGBEE_STATE = 1,
	/* Optional attribute used to configure wlan duration for Shape-OCS
	 * during interrupt.
	 * Set in pair with QCA_MPTA_HELPER_VENDOR_ATTR_INT_NON_WLAN_DURATION.
	 * Value range 0 ~ 300 (ms).
	 * NLA_U32 attribute.
	 */
	QCA_MPTA_HELPER_VENDOR_ATTR_INT_WLAN_DURATION = 2,
	/* Optional attribute used to configure non wlan duration for Shape-OCS
	 * during interrupt.
	 * Set in pair with QCA_MPTA_HELPER_VENDOR_ATTR_INT_WLAN_DURATION.
	 * Value range 0 ~ 300 (ms).
	 * NLA_U32 attribute.
	 */
	QCA_MPTA_HELPER_VENDOR_ATTR_INT_NON_WLAN_DURATION = 3,
	/* Optional attribute used to configure wlan duration for Shape-OCS
	 * monitor period.
	 * Set in pair with QCA_MPTA_HELPER_VENDOR_ATTR_MON_NON_WLAN_DURATION.
	 * Value range 0 ~ 300 (ms)
	 * NLA_U32 attribute
	 */
	QCA_MPTA_HELPER_VENDOR_ATTR_MON_WLAN_DURATION = 4,
	/* Optional attribute used to configure non wlan duration for Shape-OCS
	 * monitor period.
	 * Set in pair with QCA_MPTA_HELPER_VENDOR_ATTR_MON_WLAN_DURATION.
	 * Value range 0 ~ 300 (ms)
	 * NLA_U32 attribute
	 */
	QCA_MPTA_HELPER_VENDOR_ATTR_MON_NON_WLAN_DURATION = 5,
	/* Optional attribute used to configure ocs interrupt duration.
	 * Set in pair with QCA_MPTA_HELPER_VENDOR_ATTR_MON_OCS_DURATION.
	 * Value range 1000 ~ 20000 (ms)
	 * NLA_U32 attribute
	 */
	QCA_MPTA_HELPER_VENDOR_ATTR_INT_OCS_DURATION = 6,
	/* Optional attribute used to configure ocs monitor duration.
	 * Set in pair with QCA_MPTA_HELPER_VENDOR_ATTR_INT_OCS_DURATION.
	 * Value range 1000 ~ 20000 (ms)
	 * NLA_U32 attribute
	 */
	QCA_MPTA_HELPER_VENDOR_ATTR_MON_OCS_DURATION = 7,
	/* Optional attribute used to notify wlan FW current zigbee channel.
	 * Value range 11 ~ 26
	 * NLA_U32 attribute
	 */
	QCA_MPTA_HELPER_VENDOR_ATTR_ZIGBEE_CHAN = 8,
	/* Optional attribute used to configure wlan mute duration.
	 * Value range 0 ~ 400 (ms)
	 * NLA_U32 attribute
	 */
	QCA_MPTA_HELPER_VENDOR_ATTR_WLAN_MUTE_DURATION = 9,

	/* keep last */
	QCA_MPTA_HELPER_VENDOR_ATTR_AFTER_LAST,
	QCA_MPTA_HELPER_VENDOR_ATTR_MAX =
		QCA_MPTA_HELPER_VENDOR_ATTR_AFTER_LAST - 1
};

/**
 * enum qca_wlan_vendor_beacon_reporting_op_types - Defines different types of
 * operations for which %QCA_NL80211_VENDOR_SUBCMD_BEACON_REPORTING can be
 * used. Will be used by %QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_OP_TYPE.
 *
 * @QCA_WLAN_VENDOR_BEACON_REPORTING_OP_START: Sent by userspace to the driver
 * to request the driver to start reporting Beacon frames.
 * @QCA_WLAN_VENDOR_BEACON_REPORTING_OP_STOP: Sent by userspace to the driver
 * to request the driver to stop reporting Beacon frames.
 * @QCA_WLAN_VENDOR_BEACON_REPORTING_OP_BEACON_INFO: Sent by the driver to
 * userspace to report received Beacon frames.
 * @QCA_WLAN_VENDOR_BEACON_REPORTING_OP_PAUSE: Sent by the driver to userspace
 * to indicate that the driver is going to pause reporting Beacon frames.
 */
enum qca_wlan_vendor_beacon_reporting_op_types {
	QCA_WLAN_VENDOR_BEACON_REPORTING_OP_START = 0,
	QCA_WLAN_VENDOR_BEACON_REPORTING_OP_STOP = 1,
	QCA_WLAN_VENDOR_BEACON_REPORTING_OP_BEACON_INFO = 2,
	QCA_WLAN_VENDOR_BEACON_REPORTING_OP_PAUSE = 3,
};

/**
 * enum qca_wlan_vendor_beacon_reporting_pause_reasons - Defines different
 * types of reasons for which the driver is pausing reporting Beacon frames.
 * Will be used by %QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_PAUSE_REASON.
 *
 * @QCA_WLAN_VENDOR_BEACON_REPORTING_PAUSE_REASON_UNSPECIFIED: For unspecified
 * reasons.
 * @QCA_WLAN_VENDOR_BEACON_REPORTING_PAUSE_REASON_SCAN_STARTED: When the
 * driver/firmware is starting a scan.
 * @QCA_WLAN_VENDOR_BEACON_REPORTING_PAUSE_REASON_DISCONNECTED: When the
 * driver/firmware disconnects from the ESS and indicates the disconnection to
 * userspace (non-seamless roaming case). This reason code will be used by the
 * driver/firmware to indicate stopping of beacon report events. Userspace
 * will need to start beacon reporting again (if desired) by sending vendor
 * command QCA_NL80211_VENDOR_SUBCMD_BEACON_REPORTING with
 * QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_OP_TYPE set to
 * QCA_WLAN_VENDOR_BEACON_REPORTING_OP_START after the next connection is
 * completed.
 */
enum qca_wlan_vendor_beacon_reporting_pause_reasons {
	QCA_WLAN_VENDOR_BEACON_REPORTING_PAUSE_REASON_UNSPECIFIED = 0,
	QCA_WLAN_VENDOR_BEACON_REPORTING_PAUSE_REASON_SCAN_STARTED = 1,
	QCA_WLAN_VENDOR_BEACON_REPORTING_PAUSE_REASON_DISCONNECTED = 2,
};

/*
 * enum qca_wlan_vendor_attr_beacon_reporting_params - List of attributes used
 * in vendor sub-command QCA_NL80211_VENDOR_SUBCMD_BEACON_REPORTING.
 */
enum qca_wlan_vendor_attr_beacon_reporting_params {
	QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_INVALID = 0,
	/* Specifies the type of operation that the vendor command/event is
	 * intended for. Possible values for this attribute are defined in
	 * enum qca_wlan_vendor_beacon_reporting_op_types. u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_OP_TYPE = 1,
	/* Optionally set by userspace to request the driver to report Beacon
	 * frames using asynchronous vendor events when the
	 * QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_OP_TYPE is set to
	 * QCA_WLAN_VENDOR_BEACON_REPORTING_OP_START. NLA_FLAG attribute.
	 * If this flag is not set, the driver will only update Beacon frames
	 * in cfg80211 scan cache but not send any vendor events.
	 */
	QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_ACTIVE_REPORTING = 2,
	/* Optionally used by userspace to request the driver/firmware to
	 * report Beacon frames periodically when the
	 * QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_OP_TYPE is set to
	 * QCA_WLAN_VENDOR_BEACON_REPORTING_OP_START.
	 * u32 attribute, indicates the period of Beacon frames to be reported
	 * and in the units of beacon interval.
	 * If this attribute is missing in the command, then the default value
	 * of 1 will be assumed by driver, i.e., to report every Beacon frame.
	 * Zero is an invalid value.
	 * If a valid value is received for this attribute, the driver will
	 * update the cfg80211 scan cache periodically as per the value
	 * received in this attribute in addition to updating the cfg80211 scan
	 * cache when there is significant change in Beacon frame IEs.
	 */
	QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_PERIOD = 3,
	/* Used by the driver to encapsulate the SSID when the
	 * QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_OP_TYPE is set to
	 * QCA_WLAN_VENDOR_BEACON_REPORTING_OP_BEACON_INFO.
	 * u8 array with a maximum size of 32.
	 *
	 * When generating beacon report from non-MBSSID Beacon frame, the SSID
	 * will be taken from the SSID element of the received Beacon frame.
	 *
	 * When generating beacon report from Multiple BSSID Beacon frame and
	 * if the BSSID of the current connected BSS matches the BSSID of the
	 * transmitting BSS, the SSID will be taken from the SSID element of
	 * the received Beacon frame.
	 *
	 * When generating beacon report from Multiple BSSID Beacon frame and
	 * if the BSSID of the current connected BSS matches the BSSID of one
	 * of the* nontransmitting BSSs, the SSID will be taken from the SSID
	 * field included in the nontransmitted BSS profile whose derived BSSI
	 * is same as the BSSID of the current connected BSS. When there is no
	 * nontransmitted BSS profile whose derived BSSID is same as the BSSID
	 * of current connected* BSS, this attribute will not be present.
	 */
	QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_SSID = 4,
	/* Used by the driver to encapsulate the BSSID of the AP to which STA
	 * is currently connected to when the
	 * QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_OP_TYPE is set to
	 * QCA_WLAN_VENDOR_BEACON_REPORTING_OP_BEACON_INFO. u8 array with a
	 * fixed size of 6 bytes.
	 *
	 * When generating beacon report from a Multiple BSSID beacon and the
	 * current connected BSSID matches one of the nontransmitted BSSIDs in
	 * a Multiple BSSID set, this BSSID will be that particular
	 * nontransmitted BSSID and not the transmitted BSSID (i.e., the
	 * transmitting address of the Beacon frame).
	 */
	QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_BSSID = 5,
	/* Used by the driver to encapsulate the frequency in MHz on which
	 * the Beacon frame was received when the
	 * QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_OP_TYPE is
	 * set to QCA_WLAN_VENDOR_BEACON_REPORTING_OP_BEACON_INFO.
	 * u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_FREQ = 6,
	/* Used by the driver to encapsulate the Beacon interval
	 * when the QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_OP_TYPE is set to
	 * QCA_WLAN_VENDOR_BEACON_REPORTING_OP_BEACON_INFO.
	 * u16 attribute. The value will be copied from the Beacon frame and
	 * the units are TUs.
	 */
	QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_BI = 7,
	/* Used by the driver to encapsulate the Timestamp field from the
	 * Beacon frame when the QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_OP_TYPE
	 * is set to QCA_WLAN_VENDOR_BEACON_REPORTING_OP_BEACON_INFO.
	 * u64 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_TSF = 8,
	/* Used by the driver to encapsulate the CLOCK_BOOTTIME when this
	 * Beacon frame is received in the driver when the
	 * QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_OP_TYPE is set to
	 * QCA_WLAN_VENDOR_BEACON_REPORTING_OP_BEACON_INFO. u64 attribute, in
	 * the units of nanoseconds. This value is expected to have accuracy of
	 * about 10 ms.
	 */
	QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_BOOTTIME_WHEN_RECEIVED = 9,
	/* Used by the driver to encapsulate the IEs of the Beacon frame from
	 * which this event is generated when the
	 * QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_OP_TYPE is set to
	 * QCA_WLAN_VENDOR_BEACON_REPORTING_OP_BEACON_INFO. u8 array.
	 */
	QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_IES = 10,
	/* Used by the driver to specify the reason for the driver/firmware to
	 * pause sending beacons to userspace when the
	 * QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_OP_TYPE is set to
	 * QCA_WLAN_VENDOR_BEACON_REPORTING_OP_PAUSE. Possible values are
	 * defined in enum qca_wlan_vendor_beacon_reporting_pause_reasons, u32
	 * attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_PAUSE_REASON = 11,
	/* Used by the driver to specify whether the driver will automatically
	 * resume reporting beacon events to userspace later (for example after
	 * the ongoing off-channel activity is completed etc.) when the
	 * QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_OP_TYPE is set to
	 * QCA_WLAN_VENDOR_BEACON_REPORTING_OP_PAUSE. NLA_FLAG attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_AUTO_RESUMES = 12,
	/* Optionally set by userspace to request the driver not to resume
	 * beacon reporting after a pause is completed, when the
	 * QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_OP_TYPE is set to
	 * QCA_WLAN_VENDOR_BEACON_REPORTING_OP_START. NLA_FLAG attribute.
	 * If this flag is set, the driver will not resume beacon reporting
	 * after any pause in beacon reporting is completed. Userspace has to
	 * send QCA_WLAN_VENDOR_BEACON_REPORTING_OP_START command again in order
	 * to initiate beacon reporting again. If this flag is set in the recent
	 * QCA_WLAN_VENDOR_BEACON_REPORTING_OP_START command, then in the
	 * subsequent QCA_WLAN_VENDOR_BEACON_REPORTING_OP_PAUSE event (if any)
	 * the QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_AUTO_RESUMES shall not be
	 * set by the driver. Setting this flag until and unless there is a
	 * specific need is not recommended as there is a chance of some beacons
	 * received after pause command and next start command being not
	 * reported.
	 */
	QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_DO_NOT_RESUME = 13,

	/* Keep last */
	QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_LAST,
	QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_MAX =
		QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_LAST - 1
};

/**
 * enum qca_vendor_oem_device_type - Represents the target device in firmware.
 * It is used by QCA_WLAN_VENDOR_ATTR_OEM_DEVICE_INFO.
 *
 * @QCA_VENDOR_OEM_DEVICE_VIRTUAL: The command is intended for
 * a virtual device.
 *
 * @QCA_VENDOR_OEM_DEVICE_PHYSICAL: The command is intended for
 * a physical device.
 */
enum qca_vendor_oem_device_type {
	QCA_VENDOR_OEM_DEVICE_VIRTUAL = 0,
	QCA_VENDOR_OEM_DEVICE_PHYSICAL = 1,
};

/*
 * enum qca_wlan_vendor_attr_oem_data_params - Used by the vendor command
 * QCA_NL80211_VENDOR_SUBCMD_OEM_DATA.
 *
 * @QCA_WLAN_VENDOR_ATTR_OEM_DATA_CMD_DATA: This NLA_BINARY attribute is
 * used to set/query the data to/from the firmware. On query, the same
 * attribute is used to carry the respective data in the reply sent by the
 * driver to userspace. The request to set/query the data and the format of the
 * respective data from the firmware are embedded in the attribute. The
 * maximum size of the attribute payload is 1024 bytes.
 *
 * @QCA_WLAN_VENDOR_ATTR_OEM_DEVICE_INFO: The binary blob will be routed
 * based on this field. This optional attribute is included to specify whether
 * the device type is a virtual device or a physical device for the command.
 * This attribute can be omitted for a virtual device (default) command.
 * This u8 attribute is used to carry information for the device type using
 * values defined by enum qca_vendor_oem_device_type.
 *
 * Userspace has to set the QCA_WLAN_VENDOR_ATTR_OEM_DATA_RESPONSE_EXPECTED
 * attribute when the data is queried from the firmware.
 *
 * @QCA_WLAN_VENDOR_ATTR_OEM_DATA_RESPONSE_EXPECTED: This NLA_FLAG attribute
 * is set when the userspace queries data from the firmware. This attribute
 * should not be set when userspace sets the OEM data to the firmware.
 */
enum qca_wlan_vendor_attr_oem_data_params {
	QCA_WLAN_VENDOR_ATTR_OEM_DATA_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_OEM_DATA_CMD_DATA = 1,
	QCA_WLAN_VENDOR_ATTR_OEM_DEVICE_INFO = 2,
	QCA_WLAN_VENDOR_ATTR_OEM_DATA_RESPONSE_EXPECTED = 3,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_OEM_DATA_PARAMS_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_OEM_DATA_PARAMS_MAX =
		QCA_WLAN_VENDOR_ATTR_OEM_DATA_PARAMS_AFTER_LAST - 1
};

/**
 * enum qca_wlan_vendor_attr_avoid_frequency_ext - Defines attributes to be
 * used with vendor command/event QCA_NL80211_VENDOR_SUBCMD_AVOID_FREQUENCY_EXT.
 *
 * @QCA_WLAN_VENDOR_ATTR_AVOID_FREQUENCY_RANGE: Required
 * Nested attribute containing multiple ranges with following attributes:
 *	QCA_WLAN_VENDOR_ATTR_AVOID_FREQUENCY_START and
 *	QCA_WLAN_VENDOR_ATTR_AVOID_FREQUENCY_END.
 *
 * @QCA_WLAN_VENDOR_ATTR_AVOID_FREQUENCY_START: Required (u32)
 * Starting center frequency in MHz.
 *
 * @QCA_WLAN_VENDOR_ATTR_AVOID_FREQUENCY_END: Required (u32)
 * Ending center frequency in MHz.
 */
enum qca_wlan_vendor_attr_avoid_frequency_ext {
	QCA_WLAN_VENDOR_ATTR_AVOID_FREQUENCY_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_AVOID_FREQUENCY_RANGE = 1,
	QCA_WLAN_VENDOR_ATTR_AVOID_FREQUENCY_START = 2,
	QCA_WLAN_VENDOR_ATTR_AVOID_FREQUENCY_END = 3,

	QCA_WLAN_VENDOR_ATTR_AVOID_FREQUENCY_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_AVOID_FREQUENCY_MAX =
		QCA_WLAN_VENDOR_ATTR_AVOID_FREQUENCY_AFTER_LAST - 1
};

/*
 * enum qca_wlan_vendor_attr_add_sta_node_params - Used by the vendor command
 * QCA_NL80211_VENDOR_SUBCMD_ADD_STA_NODE.
 */
enum qca_wlan_vendor_attr_add_sta_node_params {
	QCA_WLAN_VENDOR_ATTR_ADD_STA_NODE_INVALID = 0,
	/* 6 byte MAC address of STA */
	QCA_WLAN_VENDOR_ATTR_ADD_STA_NODE_MAC_ADDR = 1,
	/* Authentication algorithm used by the station of size u16;
	 * defined in enum nl80211_auth_type.
	 */
	QCA_WLAN_VENDOR_ATTR_ADD_STA_NODE_AUTH_ALGO = 2,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_ADD_STA_NODE_PARAM_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_ADD_STA_NODE_PARAM_MAX =
		QCA_WLAN_VENDOR_ATTR_ADD_STA_NODE_PARAM_AFTER_LAST - 1
};

/**
 * enum qca_btc_chain_mode - Specifies BT coex chain mode.
 * This enum defines the valid set of values of BT coex chain mode.
 * These values are used by attribute %QCA_VENDOR_ATTR_BTC_CHAIN_MODE of
 * %QCA_NL80211_VENDOR_SUBCMD_BTC_CHAIN_MODE.
 *
 * @QCA_BTC_CHAIN_SHARED: chains of BT and WLAN 2.4G are shared.
 * @QCA_BTC_CHAIN_SEPARATED: chains of BT and WLAN 2.4G are separated.
 */
enum qca_btc_chain_mode {
	QCA_BTC_CHAIN_SHARED = 0,
	QCA_BTC_CHAIN_SEPARATED = 1,
};

/**
 * enum qca_vendor_attr_btc_chain_mode - Specifies attributes for BT coex
 * chain mode.
 * Attributes for data used by QCA_NL80211_VENDOR_SUBCMD_BTC_CHAIN_MODE.
 *
 * @QCA_VENDOR_ATTR_COEX_BTC_CHAIN_MODE: u32 attribute.
 * Indicates the BT coex chain mode, are 32-bit values from
 * enum qca_btc_chain_mode. This attribute is mandatory.
 *
 * @QCA_VENDOR_ATTR_COEX_BTC_CHAIN_MODE_RESTART: flag attribute.
 * If set, vdev should be restarted when BT coex chain mode is updated.
 * This attribute is optional.
 */
enum qca_vendor_attr_btc_chain_mode {
	QCA_VENDOR_ATTR_BTC_CHAIN_MODE_INVALID = 0,
	QCA_VENDOR_ATTR_BTC_CHAIN_MODE = 1,
	QCA_VENDOR_ATTR_BTC_CHAIN_MODE_RESTART = 2,

	/* Keep last */
	QCA_VENDOR_ATTR_BTC_CHAIN_MODE_LAST,
	QCA_VENDOR_ATTR_BTC_CHAIN_MODE_MAX =
	QCA_VENDOR_ATTR_BTC_CHAIN_MODE_LAST - 1,
};

/**
 * enum qca_vendor_wlan_sta_flags - Station feature flags
 * Bits will be set to 1 if the corresponding features are enabled.
 * @QCA_VENDOR_WLAN_STA_FLAG_AMPDU: AMPDU is enabled for the station
 * @QCA_VENDOR_WLAN_STA_FLAG_TX_STBC: TX Space-time block coding is enabled
    for the station
 * @QCA_VENDOR_WLAN_STA_FLAG_RX_STBC: RX Space-time block coding is enabled
    for the station
 */
enum qca_vendor_wlan_sta_flags {
	QCA_VENDOR_WLAN_STA_FLAG_AMPDU = BIT(0),
	QCA_VENDOR_WLAN_STA_FLAG_TX_STBC = BIT(1),
	QCA_VENDOR_WLAN_STA_FLAG_RX_STBC = BIT(2),
};

/**
 * enum qca_vendor_wlan_sta_guard_interval - Station guard interval
 * @QCA_VENDOR_WLAN_STA_GI_800_NS: Legacy normal guard interval
 * @QCA_VENDOR_WLAN_STA_GI_400_NS: Legacy short guard interval
 * @QCA_VENDOR_WLAN_STA_GI_1600_NS: Guard interval used by HE
 * @QCA_VENDOR_WLAN_STA_GI_3200_NS: Guard interval used by HE
 */
enum qca_vendor_wlan_sta_guard_interval {
	QCA_VENDOR_WLAN_STA_GI_800_NS = 0,
	QCA_VENDOR_WLAN_STA_GI_400_NS = 1,
	QCA_VENDOR_WLAN_STA_GI_1600_NS = 2,
	QCA_VENDOR_WLAN_STA_GI_3200_NS = 3,
};

/**
 * enum qca_wlan_vendor_attr_get_sta_info - Defines attributes
 * used by QCA_NL80211_VENDOR_SUBCMD_GET_STA_INFO vendor command.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_MAC:
 * Required attribute in request for AP mode only, 6-byte MAC address,
 * corresponding to the station's MAC address for which information is
 * requested. For STA mode this is not required as the info always correspond
 * to the self STA and the current/last association.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_FLAGS:
 * Optionally used in response, u32 attribute, contains a bitmap of different
 * fields defined in enum qca_vendor_wlan_sta_flags, used in AP mode only.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_GUARD_INTERVAL:
 * Optionally used in response, u32 attribute, possible values are defined in
 * enum qca_vendor_wlan_sta_guard_interval, used in AP mode only.
 * Guard interval used by the station.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_RX_RETRY_COUNT:
 * Optionally used in response, u32 attribute, used in AP mode only.
 * Value indicates the number of data frames received from station with retry
 * bit set to 1 in FC.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_RX_BC_MC_COUNT:
 * Optionally used in response, u32 attribute, used in AP mode only.
 * Counter for number of data frames with broadcast or multicast address in
 * the destination address received from the station.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_TX_RETRY_SUCCEED:
 * Optionally used in response, u32 attribute, used in both STA and AP modes.
 * Value indicates the number of data frames successfully transmitted only
 * after retrying the packets and for which the TX status has been updated
 * back to host from target.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_TX_RETRY_EXHAUSTED:
 * Optionally used in response, u32 attribute, used in both STA and AP mode.
 * Value indicates the number of data frames not transmitted successfully even
 * after retrying the packets for the number of times equal to the total number
 * of retries allowed for that packet and for which the TX status has been
 * updated back to host from target.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_TARGET_TX_TOTAL:
 * Optionally used in response, u32 attribute, used in AP mode only.
 * Counter in the target for the number of data frames successfully transmitted
 * to the station.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_TARGET_TX_RETRY:
 * Optionally used in response, u32 attribute, used in AP mode only.
 * Value indicates the number of data frames successfully transmitted only
 * after retrying the packets.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_TARGET_TX_RETRY_EXHAUSTED:
 * Optionally used in response, u32 attribute, used in both STA & AP mode.
 * Value indicates the number of data frames not transmitted successfully even
 * after retrying the packets for the number of times equal to the total number
 * of retries allowed for that packet.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_TX_PROBE_REQ_BMISS_COUNT: u32, used in
 * the STA mode only. Represent the number of probe requests sent by the STA
 * while attempting to roam on missing certain number of beacons from the
 * connected AP. If queried in the disconnected state, this represents the
 * count for the last connected state.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_RX_PROBE_RESP_BMISS_COUNT: u32, used in
 * the STA mode. Represent the number of probe responses received by the station
 * while attempting to roam on missing certain number of beacons from the
 * connected AP. When queried in the disconnected state, this represents the
 * count when in last connected state.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_TARGET_TX_ALL_COUNT: u32, used in the
 * STA mode only. Represents the total number of frames sent out by STA
 * including Data, ACK, RTS, CTS, Control Management. This data is maintained
 * only for the connect session. Represents the count of last connected session,
 * when queried in the disconnected state.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_TX_RTS_COUNT: u32, used in the STA mode.
 * Total number of RTS sent out by the STA. This data is maintained per connect
 * session. Represents the count of last connected session, when queried in the
 * disconnected state.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_TX_RTS_RETRY_FAIL_COUNT: u32, used in the
 * STA mode.Represent the number of RTS transmission failure that reach retry
 * limit. This data is maintained per connect session. Represents the count of
 * last connected session, when queried in the disconnected state.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_TX_DATA_NON_AGGREGATED_COUNT: u32, used in
 * the STA mode. Represent the total number of non aggregated frames transmitted
 * by the STA. This data is maintained per connect session. Represents the count
 * of last connected session, when queried in the disconnected state.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_TX_DATA_AGGREGATED_COUNT: u32, used in the
 * STA mode. Represent the total number of aggregated frames transmitted by the
 * STA. This data is maintained per connect session. Represents the count of
 * last connected session, when queried in the disconnected state.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_RX_FRAMES_GOOD_PLCP_COUNT: u32, used in
 * the STA mode. Represents the number of received frames with a good PLCP. This
 * data is maintained per connect session. Represents the count of last
 * connected session, when queried in the disconnected state.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_RX_FRAMES_INVALID_DELIMITER_COUNT: u32,
 * used in the STA mode. Represents the number of occasions that no valid
 * delimiter is detected by A-MPDU parser. This data is maintained per connect
 * session. Represents the count of last connected session, when queried in the
 * disconnected state.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_RX_FRAMES_CRC_FAIL_COUNT: u32, used in the
 * STA mode. Represents the number of frames for which CRC check failed in the
 * MAC. This data is maintained per connect session. Represents the count of
 * last connected session, when queried in the disconnected state.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_RX_ACKS_GOOD_FCS_COUNT: u32, used in the
 * STA mode. Represents the number of unicast ACKs received with good FCS. This
 * data is maintained per connect session. Represents the count of last
 * connected session, when queried in the disconnected state.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_RX_BLOCKACK_COUNT: u32, used in the STA
 * mode. Represents the number of received Block Acks. This data is maintained
 * per connect session. Represents the count of last connected session, when
 * queried in the disconnected state.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_RX_BEACON_COUNT: u32, used in the STA
 * mode. Represents the number of beacons received from the connected BSS. This
 * data is maintained per connect session. Represents the count of last
 * connected session, when queried in the disconnected state.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_RX_OTHER_BEACON_COUNT: u32, used in the
 * STA mode. Represents the number of beacons received by the other BSS when in
 * connected state (through the probes done by the STA). This data is maintained
 * per connect session. Represents the count of last connected session, when
 * queried in the disconnected state.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_RX_UCAST_DATA_GOOD_FCS_COUNT: u64, used in
 * the STA mode. Represents the number of received DATA frames with good FCS and
 * matching Receiver Address when in connected state. This data is maintained
 * per connect session. Represents the count of last connected session, when
 * queried in the disconnected state.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_RX_DATA_BC_MC_DROP_COUNT: u32, used in the
 * STA mode. Represents the number of RX Data multicast frames dropped by the HW
 * when in the connected state. This data is maintained per connect session.
 * Represents the count of last connected session, when queried in the
 * disconnected state.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_TARGET_POWER_24G_1MBPS: u32, used in the
 * STA mode. This represents the target power in dBm for the transmissions done
 * to the AP in 2.4 GHz at 1 Mbps (DSSS) rate. This data is maintained per
 * connect session. Represents the count of last connected session, when
 * queried in the disconnected state.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_TARGET_POWER_24G_6MBPS: u32, used in the
 * STA mode. This represents the Target power in dBm for transmissions done to
 * the AP in 2.4 GHz at 6 Mbps (OFDM) rate. This data is maintained per connect
 * session. Represents the count of last connected session, when queried in the
 * disconnected state.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_TARGET_POWER_24G_MCS0: u32, used in the
 * STA mode. This represents the Target power in dBm for transmissions done to
 * the AP in 2.4 GHz at MCS0 rate. This data is maintained per connect session.
 * Represents the count of last connected session, when queried in the
 * disconnected state.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_TARGET_POWER_5G_6MBPS: u32, used in the
 * STA mode. This represents the Target power in dBm for transmissions done to
 * the AP in 5 GHz at 6 Mbps (OFDM) rate. This data is maintained per connect
 * session. Represents the count of last connected session, when queried in
 * the disconnected state.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_TARGET_POWER_5G_MCS0: u32, used in the
 * STA mode. This represents the Target power in dBm for for transmissions done
 * to the AP in 5 GHz at MCS0 rate. This data is maintained per connect session.
 * Represents the count of last connected session, when queried in the
 * disconnected state.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_RX_HW_BUFFERS_OVERFLOW_COUNT: u32, used
 * in the STA mode. This represents the Nested attribute representing the
 * overflow counts of each receive buffer allocated to the hardware during the
 * STA's connection. The number of hw buffers might vary for each WLAN
 * solution and hence this attribute represents the nested array of all such
 * HW buffer count. This data is maintained per connect session. Represents
 * the count of last connected session, when queried in the disconnected state.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_MAX_TX_POWER: u32, Max TX power (dBm)
 * allowed as per the regulatory requirements for the current or last connected
 * session. Used in the STA mode.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_LATEST_TX_POWER: u32, Latest TX power
 * (dBm) used by the station in its latest unicast frame while communicating
 * to the AP in the connected state. When queried in the disconnected state,
 * this represents the TX power used by the STA with last AP communication
 * when in connected state.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_ANI_LEVEL: u32, Adaptive noise immunity
 * level used to adjust the RX sensitivity. Represents the current ANI level
 * when queried in the connected state. When queried in the disconnected
 * state, this corresponds to the latest ANI level at the instance of
 * disconnection.
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_BIP_MIC_ERROR_COUNT: u32, used in STA mode
 * only. This represents the number of group addressed robust management frames
 * received from this station with an invalid MIC or a missing MME when PMF is
 * enabled.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_BIP_REPLAY_COUNT: u32, used in STA mode
 * only. This represents the number of group addressed robust management frames
 * received from this station with the packet number less than or equal to the
 * last received packet number when PMF is enabled.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_BEACON_MIC_ERROR_COUNT: u32, used in STA
 * mode only. This represents the number of Beacon frames received from this
 * station with an invalid MIC or a missing MME when beacon protection is
 * enabled.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_BEACON_REPLAY_COUNT: u32, used in STA mode
 * only. This represents number of Beacon frames received from this station with
 * the packet number less than or equal to the last received packet number when
 * beacon protection is enabled.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_CONNECT_FAIL_REASON_CODE: u32, used in
 * STA mode only. Driver uses this attribute to populate the connection failure
 * reason codes and the values are defined in
 * enum qca_sta_connect_fail_reason_codes. Userspace applications can send
 * QCA_NL80211_VENDOR_SUBCMD_GET_STA_INFO vendor command after receiving
 * connection failure from driver. The driver shall not include this attribute
 * in response to QCA_NL80211_VENDOR_SUBCMD_GET_STA_INFO command if there is no
 * connection failure observed in the last attempted connection.
 */
enum qca_wlan_vendor_attr_get_sta_info {
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_MAC = 1,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_FLAGS = 2,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_GUARD_INTERVAL = 3,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_RX_RETRY_COUNT = 4,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_RX_BC_MC_COUNT = 5,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_TX_RETRY_SUCCEED = 6,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_TX_RETRY_EXHAUSTED = 7,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_TARGET_TX_TOTAL = 8,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_TARGET_TX_RETRY = 9,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_TARGET_TX_RETRY_EXHAUSTED = 10,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_TX_PROBE_REQ_BMISS_COUNT = 11,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_RX_PROBE_RESP_BMISS_COUNT = 12,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_TARGET_TX_ALL_COUNT = 13,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_TX_RTS_COUNT = 14,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_TX_RTS_RETRY_FAIL_COUNT = 15,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_TX_DATA_NON_AGGREGATED_COUNT = 16,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_TX_DATA_AGGREGATED_COUNT = 17,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_RX_FRAMES_GOOD_PLCP_COUNT = 18,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_RX_FRAMES_INVALID_DELIMITER_COUNT = 19,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_RX_FRAMES_CRC_FAIL_COUNT = 20,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_RX_ACKS_GOOD_FCS_COUNT = 21,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_RX_BLOCKACK_COUNT = 22,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_RX_BEACON_COUNT = 23,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_RX_OTHER_BEACON_COUNT = 24,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_RX_UCAST_DATA_GOOD_FCS_COUNT = 25,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_RX_DATA_BC_MC_DROP_COUNT = 26,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_TARGET_POWER_24G_1MBPS = 27,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_TARGET_POWER_24G_6MBPS = 28,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_TARGET_POWER_24G_MCS0 = 29,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_TARGET_POWER_5G_6MBPS = 30,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_TARGET_POWER_5G_MCS0 = 31,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_RX_HW_BUFFERS_OVERFLOW_COUNT = 32,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_MAX_TX_POWER = 33,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_LATEST_TX_POWER = 34,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_ANI_LEVEL = 35,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_BIP_MIC_ERROR_COUNT = 39,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_BIP_REPLAY_COUNT = 40,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_BEACON_MIC_ERROR_COUNT = 41,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_BEACON_REPLAY_COUNT = 42,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_CONNECT_FAIL_REASON_CODE = 43,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_MAX =
		QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_AFTER_LAST - 1,
};

/**
 * enum qca_disconnect_reason_codes - Specifies driver disconnect reason codes.
 * Used when the driver triggers the STA to disconnect from the AP.
 *
 * @QCA_DISCONNECT_REASON_UNSPECIFIED: The host driver triggered the
 * disconnection with the AP due to unspecified reasons.
 *
 * @QCA_DISCONNECT_REASON_INTERNAL_ROAM_FAILURE: The host driver triggered the
 * disconnection with the AP due to a roaming failure. This roaming is triggered
 * internally (host driver/firmware).
 *
 * @QCA_DISCONNECT_REASON_EXTERNAL_ROAM_FAILURE: The driver disconnected from
 * the AP when the user/external triggered roaming fails.
 *
 * @QCA_DISCONNECT_REASON_GATEWAY_REACHABILITY_FAILURE: This reason code is used
 * by the host driver whenever gateway reachability failure is detected and the
 * driver disconnects with AP.
 *
 * @QCA_DISCONNECT_REASON_UNSUPPORTED_CHANNEL_CSA: The driver disconnected from
 * the AP on a channel switch announcement from it with an unsupported channel.
 *
 * @QCA_DISCONNECT_REASON_OPER_CHANNEL_DISABLED_INDOOR: On a concurrent AP start
 * with indoor channels disabled and if the STA is connected on one of these
 * disabled channels, the host driver disconnected the STA with this reason
 * code.
 *
 * @QCA_DISCONNECT_REASON_OPER_CHANNEL_USER_DISABLED: Disconnection due to an
 * explicit request from the user to disable the current operating channel.
 *
 * @QCA_DISCONNECT_REASON_DEVICE_RECOVERY: STA disconnected from the AP due to
 * the internal host driver/firmware recovery.
 *
 * @QCA_DISCONNECT_REASON_KEY_TIMEOUT: The driver triggered the disconnection on
 * a timeout for the key installations from the user space.
 *
 * @QCA_DISCONNECT_REASON_OPER_CHANNEL_BAND_CHANGE: The dDriver disconnected the
 * STA on a band change request from the user space to a different band from the
 * current operation channel/band.
 *
 * @QCA_DISCONNECT_REASON_IFACE_DOWN: The STA disconnected from the AP on an
 * interface down trigger from the user space.
 *
 * @QCA_DISCONNECT_REASON_PEER_XRETRY_FAIL: The host driver disconnected the
 * STA on getting continuous transmission failures for multiple Data frames.
 *
 * @QCA_DISCONNECT_REASON_PEER_INACTIVITY: The STA does a keep alive
 * notification to the AP by transmitting NULL/G-ARP frames. This disconnection
 * represents inactivity from AP on such transmissions.

 * @QCA_DISCONNECT_REASON_SA_QUERY_TIMEOUT: This reason code is used on
 * disconnection when SA Query times out (AP does not respond to SA Query).
 *
 * @QCA_DISCONNECT_REASON_BEACON_MISS_FAILURE: The host driver disconnected the
 * STA on missing the beacons continuously from the AP.
 *
 * @QCA_DISCONNECT_REASON_CHANNEL_SWITCH_FAILURE: Disconnection due to STA not
 * able to move to the channel mentioned by the AP in CSA.
 *
 * @QCA_DISCONNECT_REASON_USER_TRIGGERED: User triggered disconnection.
 */
enum qca_disconnect_reason_codes {
	QCA_DISCONNECT_REASON_UNSPECIFIED = 0,
	QCA_DISCONNECT_REASON_INTERNAL_ROAM_FAILURE = 1,
	QCA_DISCONNECT_REASON_EXTERNAL_ROAM_FAILURE = 2,
	QCA_DISCONNECT_REASON_GATEWAY_REACHABILITY_FAILURE = 3,
	QCA_DISCONNECT_REASON_UNSUPPORTED_CHANNEL_CSA = 4,
	QCA_DISCONNECT_REASON_OPER_CHANNEL_DISABLED_INDOOR = 5,
	QCA_DISCONNECT_REASON_OPER_CHANNEL_USER_DISABLED = 6,
	QCA_DISCONNECT_REASON_DEVICE_RECOVERY = 7,
	QCA_DISCONNECT_REASON_KEY_TIMEOUT = 8,
	QCA_DISCONNECT_REASON_OPER_CHANNEL_BAND_CHANGE = 9,
	QCA_DISCONNECT_REASON_IFACE_DOWN = 10,
	QCA_DISCONNECT_REASON_PEER_XRETRY_FAIL = 11,
	QCA_DISCONNECT_REASON_PEER_INACTIVITY = 12,
	QCA_DISCONNECT_REASON_SA_QUERY_TIMEOUT = 13,
	QCA_DISCONNECT_REASON_BEACON_MISS_FAILURE = 14,
	QCA_DISCONNECT_REASON_CHANNEL_SWITCH_FAILURE = 15,
	QCA_DISCONNECT_REASON_USER_TRIGGERED = 16,
};

/**
 * enum qca_wlan_vendor_attr_driver_disconnect_reason - Defines attributes
 * used by %QCA_NL80211_VENDOR_SUBCMD_DRIVER_DISCONNECT_REASON vendor command.
 *
 * @QCA_WLAN_VENDOR_ATTR_DRIVER_DISCONNECT_REASCON_CODE: u32 attribute.
 * This attribute represents the driver specific reason codes (local
 * driver/firmware initiated reasons for disconnection) defined
 * in enum qca_disconnect_reason_codes.
 */
enum qca_wlan_vendor_attr_driver_disconnect_reason {
	QCA_WLAN_VENDOR_ATTR_DRIVER_DISCONNECT_REASON_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_DRIVER_DISCONNECT_REASCON_CODE = 1,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_DRIVER_DISCONNECT_REASON_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_DRIVER_DISCONNECT_REASON_MAX =
	QCA_WLAN_VENDOR_ATTR_DRIVER_DISCONNECT_REASON_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_tspec_operation - Operation of the config TSPEC request
 *
 * Values for %QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_OPERATION.
 */
enum qca_wlan_tspec_operation {
	QCA_WLAN_TSPEC_ADD = 0,
	QCA_WLAN_TSPEC_DEL = 1,
	QCA_WLAN_TSPEC_GET = 2,
};

/**
 * enum qca_wlan_tspec_direction - Direction in TSPEC
 * As what is defined in IEEE Std 802.11-2016, Table 9-139.
 *
 * Values for %QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_DIRECTION.
 */
enum qca_wlan_tspec_direction {
	QCA_WLAN_TSPEC_DIRECTION_UPLINK = 0,
	QCA_WLAN_TSPEC_DIRECTION_DOWNLINK = 1,
	QCA_WLAN_TSPEC_DIRECTION_DIRECT = 2,
	QCA_WLAN_TSPEC_DIRECTION_BOTH = 3,
};

/**
 * enum qca_wlan_tspec_ack_policy - MAC acknowledgment policy in TSPEC
 * As what is defined in IEEE Std 802.11-2016, Table 9-141.
 *
 * Values for %QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_ACK_POLICY.
 */
enum qca_wlan_tspec_ack_policy {
	QCA_WLAN_TSPEC_NORMAL_ACK = 0,
	QCA_WLAN_TSPEC_NO_ACK = 1,
	/* Reserved */
	QCA_WLAN_TSPEC_BLOCK_ACK = 3,
};

/**
 * enum qca_wlan_vendor_attr_config_tspec - Defines attributes
 * used by QCA_NL80211_VENDOR_SUBCMD_CONFIG_TSPEC vendor command.
 *
 * @QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_OPERATION:
 * u8 attribute. Specify the TSPEC operation of this request. Possible values
 * are defined in enum qca_wlan_tspec_operation.
 * Mandatory attribute for all kinds of config TSPEC requests.
 *
 * @QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_TSID:
 * u8 attribute. TS ID. Possible values are 0-7.
 * Applicable for operation: QCA_WLAN_TSPEC_ADD, QCA_WLAN_TSPEC_DEL,
 * QCA_WLAN_TSPEC_GET. A mandatory attribute.
 *
 * @QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_DIRECTION:
 * u8 attribute. Direction of data carried by the TS. Possible values are
 * defined in enum qca_wlan_tspec_direction.
 * Applicable for operation: QCA_WLAN_TSPEC_ADD. A mandatory attribute.
 *
 * @QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_APSD:
 * Flag attribute. Indicate whether APSD is enabled for the traffic associated
 * with the TS. set - enabled, not set - disabled.
 * Applicable for operation: QCA_WLAN_TSPEC_ADD. A mandatory attribute.
 *
 * @QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_USER_PRIORITY:
 * u8 attribute. User priority to be used for the transport of MSDUs/ -MSDUs
 * belonging to this TS. Possible values are 0-7.
 * Applicable for operation: QCA_WLAN_TSPEC_ADD. An optional attribute.
 *
 * @QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_ACK_POLICY:
 * u8 attribute. Indicate whether MAC acknowledgements are required for
 * MPDUs/A-MSDUs belonging to this TS and the form of those acknowledgements.
 * Possible values are defined in enum qca_wlan_tspec_ack_policy.
 * Applicable for operation: QCA_WLAN_TSPEC_ADD. A mandatory attribute.
 *
 * @QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_NOMINAL_MSDU_SIZE:
 * u16 attribute. Specify the nominal size in bytes of MSDUs/A-MSDUs
 * belonging to this TS.
 * Applicable for operation: QCA_WLAN_TSPEC_ADD. A mandatory attribute.
 *
 * @QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_MAXIMUM_MSDU_SIZE:
 * u16 attribute. Specify the maximum size in bytes of MSDUs/A-MSDUs
 * belonging to this TS.
 * Applicable for operation: QCA_WLAN_TSPEC_ADD. A mandatory attribute.
 *
 * @QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_MIN_SERVICE_INTERVAL:
 * u32 attribute. Specify the minimum interval in microseconds between the
 * start of two successive SPs.
 * Applicable for operation: QCA_WLAN_TSPEC_ADD. A mandatory attribute.
 *
 * @QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_MAX_SERVICE_INTERVAL:
 * u32 attribute. Specify the maximum interval in microseconds between the
 * start of two successive SPs.
 * Applicable for operation: QCA_WLAN_TSPEC_ADD. A mandatory attribute.
 *
 * @QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_INACTIVITY_INTERVAL:
 * u32 attribute. Specify the minimum interval in microseconds that can elapse
 * without arrival or transfer of an MPDU belonging to the TS before this TS
 * is deleted by the MAC entity at the HC.
 * Applicable for operation: QCA_WLAN_TSPEC_ADD. A mandatory attribute.
 *
 * @QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_SUSPENSION_INTERVAL:
 * u32 attribute. Specify the minimum interval in microseconds that can elapse
 * without arrival or transfer of an MSDU belonging to the TS before the
 * generation of successive QoS(+)CF-Poll is stopped for this TS. A value of
 * 0xFFFFFFFF disables the suspension interval. The value of the suspension
 * interval is always less than or equal to the inactivity interval.
 * Applicable for operation: QCA_WLAN_TSPEC_ADD. A mandatory attribute.
 *
 * @QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_MINIMUM_DATA_RATE:
 * u32 attribute. Indicate the lowest data rate in bps specified at the MAC
 * SAP for transport of MSDUs or A-MSDUs belonging to this TS within the
 * bounds of this TSPEC.
 * Applicable for operation: QCA_WLAN_TSPEC_ADD. An optional attribute.
 *
 * @QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_MEAN_DATA_RATE:
 * u32 attribute. Indicate the average data rate in bps specified at the MAC
 * SAP for transport of MSDUs or A-MSDUs belonging to this TS within the
 * bounds of this TSPEC.
 * Applicable for operation: QCA_WLAN_TSPEC_ADD. An optional attribute.
 *
 * @QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_PEAK_DATA_RATE:
 * u32 attribute. Indicate the maximum allowable data rate in bps specified at
 * the MAC SAP for transport of MSDUs or A-MSDUs belonging to this TS within
 * the bounds of this TSPEC.
 * Applicable for operation: QCA_WLAN_TSPEC_ADD. An optional attribute.
 *
 * @QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_BURST_SIZE:
 * u32 attribute. Specify the maximum burst size in bytes of the MSDUs/A-MSDUs
 * belonging to this TS that arrive at the MAC SAP at the peak data rate. A
 * value of 0 indicates that there are no bursts.
 * Applicable for operation: QCA_WLAN_TSPEC_ADD. An optional attribute.
 *
 * @QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_MINIMUM_PHY_RATE:
 * u32 attribute. Indicate the minimum PHY rate in bps for transport of
 * MSDUs/A-MSDUs belonging to this TS within the bounds of this TSPEC.
 * Applicable for operation: QCA_WLAN_TSPEC_ADD. An optional attribute.
 *
 * @QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_SURPLUS_BANDWIDTH_ALLOWANCE:
 * u16 attribute. Specify the excess allocation of time (and bandwidth) over
 * and above the stated application rates required to transport an MSDU/A-MSDU
 * belonging to the TS in this TSPEC.
 * Applicable for operation: QCA_WLAN_TSPEC_ADD. A mandatory attribute.
 */
enum qca_wlan_vendor_attr_config_tspec {
	QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_OPERATION = 1,
	QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_TSID = 2,
	QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_DIRECTION = 3,
	QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_APSD = 4,
	QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_USER_PRIORITY = 5,
	QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_ACK_POLICY = 6,
	QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_NOMINAL_MSDU_SIZE = 7,
	QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_MAXIMUM_MSDU_SIZE = 8,
	QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_MIN_SERVICE_INTERVAL = 9,
	QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_MAX_SERVICE_INTERVAL = 10,
	QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_INACTIVITY_INTERVAL = 11,
	QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_SUSPENSION_INTERVAL = 12,
	QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_MINIMUM_DATA_RATE = 13,
	QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_MEAN_DATA_RATE = 14,
	QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_PEAK_DATA_RATE = 15,
	QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_BURST_SIZE = 16,
	QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_MINIMUM_PHY_RATE = 17,
	QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_SURPLUS_BANDWIDTH_ALLOWANCE = 18,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_MAX =
	QCA_WLAN_VENDOR_ATTR_CONFIG_TSPEC_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_medium_assess_type - Type of medium assess request
 *
 * Values for %QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_TYPE.
 */
enum qca_wlan_medium_assess_type {
	QCA_WLAN_MEDIUM_ASSESS_CCA = 0,
	QCA_WLAN_MEDIUM_ASSESS_CONGESTION_REPORT = 1,
};

/**
 * enum qca_wlan_vendor_attr_medium_assess - Attributes used by
 * %QCA_NL80211_VENDOR_SUBCMD_MEDIUM_ASSESS vendor command.
 *
 * @QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_TYPE:
 * u8 attribute. Mandatory in all kinds of medium assess requests/responses.
 * Specify the type of medium assess request and indicate its type in response.
 * Possible values are defined in enum qca_wlan_medium_assess_type.
 *
 * @QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_PERIOD:
 * u32 attribute. Mandatory in CCA request.
 * Specify the assessment period in terms of seconds. Assessment result will be
 * sent as the response to the CCA request after the assessment period.
 *
 * @QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_TOTAL_CYCLE_COUNT:
 * u32 attribute. Mandatory in response to CCA request.
 * Total timer tick count of the assessment cycle.
 *
 * @QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_IDLE_COUNT:
 * u32 attribute. Mandatory in response to CCA request.
 * Timer tick count of idle time in the assessment cycle.
 *
 * @QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_IBSS_RX_COUNT:
 * u32 attribute. Mandatory in response to CCA request.
 * Timer tick count of Intra BSS traffic RX time in the assessment cycle.
 *
 * @QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_OBSS_RX_COUNT:
 * u32 attribute. Mandatory in response to CCA request.
 * Timer tick count of Overlapping BSS traffic RX time in the assessment cycle.
 *
 * @QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_MAX_IBSS_RSSI:
 * s32 attribute. Mandatory in response to CCA request.
 * Maximum RSSI of Intra BSS traffic in the assessment cycle.
 *
 * @QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_MIN_IBSS_RSSI:
 * s32 attribute. Mandatory in response to CCA request.
 * Minimum RSSI of Intra BSS traffic in the assessment cycle.
 *
 * @QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_CONGESTION_REPORT_ENABLE:
 * u8 attribute. Mandatory in congestion report request.
 * 1-enable 0-disable.
 *
 * @QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_CONGESTION_REPORT_THRESHOLD:
 * u8 attribute. Mandatory in congestion report enable request and will be
 * ignored if present in congestion report disable request. Possible values are
 * 0-100. A vendor event QCA_NL80211_VENDOR_SUBCMD_MEDIUM_ASSESS with the type
 * QCA_WLAN_MEDIUM_ASSESS_CONGESTION_REPORT will be sent to userspace if
 * congestion percentage reaches the configured threshold.
 *
 * @QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_CONGESTION_REPORT_INTERVAL:
 * u8 attribute. Optional in congestion report enable request and will be
 * ignored if present in congestion report disable request.
 * Specify the interval of congestion report event in terms of seconds. Possible
 * values are 1-255. Default value 1 will be used if this attribute is omitted
 * or using invalid values.
 *
 * @QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_CONGESTION_PERCENTAGE:
 * u8 attribute. Mandatory in congestion report event.
 * Indicate the actual congestion percentage. Possible values are 0-100.
 */
enum qca_wlan_vendor_attr_medium_assess {
	QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_TYPE = 1,

	QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_PERIOD = 2,
	QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_TOTAL_CYCLE_COUNT = 3,
	QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_IDLE_COUNT = 4,
	QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_IBSS_RX_COUNT = 5,
	QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_OBSS_RX_COUNT = 6,
	QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_MAX_IBSS_RSSI = 7,
	QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_MIN_IBSS_RSSI = 8,

	QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_CONGESTION_REPORT_ENABLE = 9,
	QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_CONGESTION_REPORT_THRESHOLD = 10,
	QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_CONGESTION_REPORT_INTERVAL = 11,
	QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_CONGESTION_PERCENTAGE = 12,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_MAX =
	QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_mbssid_tx_vdev_status - Defines attributes
 * used by QCA_NL80211_VENDOR_SUBCMD_MBSSID_TX_VDEV_STATUS vendor command.
 *
 * @QCA_WLAN_VENDOR_ATTR_MBSSID_TX_VDEV_STATUS_VAL:
 * u8 attribute. Notify the TX VDEV status. Possible values 0, 1
 * belonging to MBSSID/EMA_AP configuration. 0 means Non-Tx VDEV,
 * 1 means Tx VDEV. Mandatory attribute for all MBSSID VDEV status events.
 */
enum qca_wlan_vendor_attr_mbssid_tx_vdev_status {
	QCA_WLAN_VENDOR_ATTR_MBSSID_TX_VDEV_STATUS_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_MBSSID_TX_VDEV_STATUS_VAL = 1,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_MBSSID_TX_VDEV_STATUS_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_MBSSID_TX_VDEV_STATUS_MAX =
	QCA_WLAN_VENDOR_ATTR_MBSSID_TX_VDEV_STATUS_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_oci_override_frame_type - OCI override frame type
 * @QCA_WLAN_VENDOR_OCI_OVERRIDE_FRAME_SA_QUERY_REQ: SA Query Request frame
 * @QCA_WLAN_VENDOR_OCI_OVERRIDE_FRAME_SA_QUERY_RESP: SA Query Response frame
 * @QCA_WLAN_VENDOR_OCI_OVERRIDE_FRAME_FT_REASSOC_REQ: FT Reassociation Request
 * frame
 * @QCA_WLAN_VENDOR_OCI_OVERRIDE_FRAME_FILS_REASSOC_REQ: FILS Reassociation
 * Request frame.
 */
enum qca_wlan_vendor_oci_override_frame_type {
	QCA_WLAN_VENDOR_OCI_OVERRIDE_FRAME_SA_QUERY_REQ = 1,
	QCA_WLAN_VENDOR_OCI_OVERRIDE_FRAME_SA_QUERY_RESP = 2,
	QCA_WLAN_VENDOR_OCI_OVERRIDE_FRAME_FT_REASSOC_REQ = 3,
	QCA_WLAN_VENDOR_OCI_OVERRIDE_FRAME_FILS_REASSOC_REQ = 4,
};

/**
 * enum qca_sta_connect_fail_reason_codes - Defines values carried
 * by QCA_WLAN_VENDOR_ATTR_GET_STA_INFO_CONNECT_FAIL_REASON_CODE vendor
 * attribute.
 * @QCA_STA_CONNECT_FAIL_REASON_NO_BSS_FOUND: No probe response frame received
 * for unicast probe request.
 * @QCA_STA_CONNECT_FAIL_REASON_AUTH_TX_FAIL: STA failed to send auth request.
 * @QCA_STA_CONNECT_FAIL_REASON_AUTH_NO_ACK_RECEIVED: AP didn't send ACK for
 * ath request.
 * @QCA_STA_CONNECT_FAIL_REASON_AUTH_NO_RESP_RECEIVED: Auth response is not
 * received from AP.
 * @QCA_STA_CONNECT_FAIL_REASON_ASSOC_REQ_TX_FAIL: STA failed to send assoc
 * request.
 * @QCA_STA_CONNECT_FAIL_REASON_ASSOC_NO_ACK_RECEIVED: AP didn't send ACK for
 * assoc request.
 * @QCA_STA_CONNECT_FAIL_REASON_ASSOC_NO_RESP_RECEIVED: Assoc response is not
 * received from AP.
 */
enum qca_sta_connect_fail_reason_codes {
	QCA_STA_CONNECT_FAIL_REASON_NO_BSS_FOUND = 1,
	QCA_STA_CONNECT_FAIL_REASON_AUTH_TX_FAIL = 2,
	QCA_STA_CONNECT_FAIL_REASON_AUTH_NO_ACK_RECEIVED = 3,
	QCA_STA_CONNECT_FAIL_REASON_AUTH_NO_RESP_RECEIVED = 4,
	QCA_STA_CONNECT_FAIL_REASON_ASSOC_REQ_TX_FAIL = 5,
	QCA_STA_CONNECT_FAIL_REASON_ASSOC_NO_ACK_RECEIVED = 6,
	QCA_STA_CONNECT_FAIL_REASON_ASSOC_NO_RESP_RECEIVED = 7,
};

/**
 * enum qca_wlan_vendor_attr_oci_override: Represents attributes for
 * OCI override request. These attributes are used inside nested attribute
 * %QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_OCI_OVERRIDE in QCA vendor command
 * %QCA_NL80211_VENDOR_SUBCMD_WIFI_TEST_CONFIGURATION.
 *
 * @QCA_WLAN_VENDOR_ATTR_OCI_OVERRIDE_FRAME_TYPE: Required attribute, u8.
 * Values from enum qca_wlan_vendor_oci_override_frame_type used in this
 * attribute to specify the frame type in which the OCI is to be overridden.
 *
 * @QCA_WLAN_VENDOR_ATTR_OCI_OVERRIDE_FREQUENCY: Required (u32)
 * OCI frequency (in MHz) to override in the specified frame type.
 */
enum qca_wlan_vendor_attr_oci_override {
	QCA_WLAN_VENDOR_ATTR_OCI_OVERRIDE_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_OCI_OVERRIDE_FRAME_TYPE = 1,
	QCA_WLAN_VENDOR_ATTR_OCI_OVERRIDE_FREQUENCY = 2,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_OCI_OVERRIDE_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_OCI_OVERRIDE_MAX =
	QCA_WLAN_VENDOR_ATTR_OCI_OVERRIDE_AFTER_LAST - 1,
};

#endif
