#
# Copyright (C) 1999-2016, Broadcom Corporation
# 
#      Unless you and Broadcom execute a separate written software license
# agreement governing use of this software, this software is licensed to you
# under the terms of the GNU General Public License version 2 (the "GPL"),
# available at http://www.broadcom.com/licenses/GPLv2.php, with the
# following added to such license:
# 
#      As a special exception, the copyright holders of this software give you
# permission to link this software with independent modules, and to copy and
# distribute the resulting executable under terms of your choice, provided that
# you also meet, for each linked independent module, the terms and conditions of
# the license of that module.  An independent module is a module which is not
# derived from this software.  The special exception does not apply to any
# modifications of the software.
# 
#      Notwithstanding the above, under no circumstances may you combine this
# software in any way with any other Broadcom software provided under a license
# other than the GPL, without Broadcom's express prior written consent.
#
#
# <<Broadcom-WL-IPTag/Open:>>
#
# bcmdhd
#####################
# Basic feature
#####################

DHDCFLAGS += -Wall -Wno-date-time -Wstrict-prototypes -Dlinux -DLINUX -DBCMDRIVER      \
        -DBCMDONGLEHOST -DUNRELEASEDCHIP -DBCMDMA32 -DBCMFILEIMAGE      \
        -DDHDTHREAD -DDHD_BCMEVENTS -DSHOW_EVENTS -DBCMDBG -DWLP2P      \
        -DWIFI_ACT_FRAME -DARP_OFFLOAD_SUPPORT -DKEEP_ALIVE -DCSCAN     \
        -DPKT_FILTER_SUPPORT -DEMBEDDED_PLATFORM -DPNO_SUPPORT          \
        -DCONFIG_DTS -DCUSTOM_COUNTRY_CODE

#####################
# Bus Interface Type
#####################
ifneq ($(CONFIG_BCMDHD_PCIE),)
	BUS_IFACE_PCIE=y
else
	BUS_IFACE_SDIO=y
endif

#####################
# SDIO I/F
#####################
ifeq ($(BUS_IFACE_SDIO),y)
	DHDCFLAGS += -DBDC -DOOB_INTR_ONLY -DDHD_BCMEVENTS -DMMC_SDIO_ABORT
	DHDCFLAGS += -DBCMSDIO -DBCMLXSDMMC -DUSE_SDIOFIFO_IOVAR
	DHDCFLAGS += -U__ARM_ARCH_7A__
	# DPC priority
	DHDCFLAGS += -DCUSTOM_DPC_PRIO_SETTING=98
	# idle count
	DHDCFLAGS += -DDHD_USE_IDLECOUNT
	# SKB TAILPAD to avoid out of boundary memory access
	DHDCFLAGS += -DDHDENABLE_TAILPAD
	DHDCFLAGS += -DSUPPORT_P2P_GO_PS
	# Enable PROP_TXSTATUS
	DHDCFLAGS += -DPROP_TXSTATUS
endif

#####################
# PCIE I/F
#####################
ifeq ($(BUS_IFACE_PCIE),y)
	DHDCFLAGS += -DPCIE_FULL_DONGLE -DBCMPCIE
	# DPC priority
	DHDCFLAGS += -DCUSTOM_DPC_PRIO_SETTING=-1
	# Disable watchdog thread
	DHDCFLAGS += -DCUSTOM_DHD_WATCHDOG_MS=0
	# Use Coherent memory for ring
	DHDCFLAGS += -DDHD_USE_COHERENT_MEM_FOR_RING
	# Alloc Coherent memory from DMA coherent pool
	DHDCFLAGS += -DDHD_ALLOC_COHERENT_MEM_FROM_ATOMIC_POOL
	# Enable Link down recovery
	DHDCFLAGS += -DSUPPORT_LINKDOWN_RECOVERY
	# Enable Firmware Coredump
	DHDCFLAGS += -DDHD_FW_COREDUMP
	# Enable PKTID AUDIT
	DHDCFLAGS += -DDHD_PKTID_AUDIT_ENABLED
	# Enable Load Balancing support by default.
	# DHD_LB_RXP - Perform RX Packet processing in parallel
	# DHD_LB_STATS - To display the Load Blancing statistics
	DHDCFLAGS += -DDHD_LB -DDHD_LB_RXP -DDHD_LB_STATS
	# Enable wakelock for legacy scan
	DHDCFLAGS += -DDHD_USE_SCAN_WAKELOCK
endif



#################
# Common feature
#################

DHDCFLAGS += -DCUSTOMER_HW2 -DCUSTOMER_HW5
DHDCFLAGS += -DWL_CFG80211


# Debug
DHDCFLAGS += -DSIMPLE_MAC_PRINT
DHDCFLAGS += -DDEBUGFS_CFG80211
# Enable wakelock debug function
DHDCFLAGS += -DDHD_TRACE_WAKE_LOCK
# Print out kernel panic point of file and line info when assertion happened
DHDCFLAGS += -DBCMASSERT_LOG

# Print 8021X
DHDCFLAGS += -DDHD_8021X_DUMP

# VSDB
DHDCFLAGS += -DVSDB
DHDCFLAGS += -DPROP_TXSTATUS

# Wi-Fi Direct
DHDCFLAGS += -DWL_CFG80211_VSDB_PRIORITIZE_SCAN_REQUEST
# For p2p connection issue
DHDCFLAGS += -DWL_SCB_TIMEOUT=10
# For TDLS tear down inactive time 10 sec
DHDCFLAGS += -DCUSTOM_TDLS_IDLE_MODE_SETTING=10000
# for TDLS RSSI HIGH for establishing TDLS link
DHDCFLAGS += -DCUSTOM_TDLS_RSSI_THRESHOLD_HIGH=-80
# for TDLS RSSI HIGH for tearing down TDLS link
DHDCFLAGS += -DCUSTOM_TDLS_RSSI_THRESHOLD_LOW=-85

# Roaming
DHDCFLAGS += -DROAM_AP_ENV_DETECTION
DHDCFLAGS += -DROAM_ENABLE
DHDCFLAGS += -DENABLE_FW_ROAM_SUSPEND

# Deep sleep
DHDCFLAGS += -DSUPPORT_DEEP_SLEEP

# SoftAP
DHDCFLAGS += -DWL_SUPPORT_AUTO_CHANNEL -DSUPPORT_HIDDEN_AP
DHDCFLAGS += -DDISABLE_11H_SOFTAP
DHDCFLAGS += -DSUPPORT_HOSTAPD_BGN_MODE
DHDCFLAGS += -DDISABLE_WL_FRAMEBURST_SOFTAP

# HW5 specific features
DHDCFLAGS += -DSUPPORT_PM2_ONLY
DHDCFLAGS += -DSUPPORT_AMPDU_MPDU_CMD
DHDCFLAGS += -DBLOCK_IPV6_PACKET -DPASS_IPV4_SUSPEND

# Support MAC ACL setting
DHDCFLAGS += -DWL_CFG80211_ACL
# Connection statistics
DHDCFLAGS += -DCONNECTION_STATISTICS
# END HW5 specific features

# For special PNO Event keep wake lock for 10sec
DHDCFLAGS += -DCUSTOM_PNO_EVENT_LOCK_xTIME=10
# For Passing all multicast packets to host when not in suspend mode.
DHDCFLAGS += -DPASS_ALL_MCAST_PKTS


# Early suspend
DHDCFLAGS += -DDHD_USE_EARLYSUSPEND

# WiFi turn off delay
DHDCFLAGS += -DWIFI_TURNOFF_DELAY=100

# For Scan result patch
DHDCFLAGS += -DESCAN_RESULT_PATCH
DHDCFLAGS += -DESCAN_BUF_OVERFLOW_MGMT

# For Static Buffer
ifeq ($(CONFIG_BROADCOM_WIFI_RESERVED_MEM),y)
  DHDCFLAGS += -DCONFIG_DHD_USE_STATIC_BUF
  DHDCFLAGS += -DENHANCED_STATIC_BUF
  DHDCFLAGS += -DSTATIC_WL_PRIV_STRUCT
ifeq ($(BUS_IFACE_PCIE),y)
  # Use preallocated IOCTL and EVENT buffer
  DHDCFLAGS += -DDHD_USE_STATIC_CTRLBUF
ifeq ($(CONFIG_BCMDHD_PREALLOC_PKTIDMAP),y)
  DHDCFLAGS += -DDHD_USE_STATIC_PKTIDMAP
endif
ifeq ($(CONFIG_BCMDHD_PREALLOC_MEMDUMP),y)
  DHDCFLAGS += -DDHD_USE_STATIC_MEMDUMP
endif
endif # BUS_IFACE_PCIE
endif # CONFIG_BROADCOM_WIFI_RESERVED_MEM

# DTIM listen interval in suspend mode(0 means follow AP's DTIM period)
DHDCFLAGS += -DCUSTOM_SUSPEND_BCN_LI_DTIM=3

# Ioctl timeout 5000ms
DHDCFLAGS += -DIOCTL_RESP_TIMEOUT=5000

# Ioctl Rx count timeout
DHDCFLAGS += -DMAX_CNTL_RX_TIMEOUT=3

# Priority mismatch fix with kernel stack
DHDCFLAGS += -DPKTPRIO_OVERRIDE

# Prevent rx thread monopolize
DHDCFLAGS += -DWAIT_DEQUEUE

# Config PM Control
DHDCFLAGS += -DCONFIG_CONTROL_PM

# Use Android wake lock mechanism
DHDCFLAGS += -DCONFIG_HAS_WAKELOCK

# Used short dwell time during initial scan
DHDCFLAGS += -DUSE_INITIAL_SHORT_DWELL_TIME

##############################
# Android Platform Definition
##############################

###############################
# Android M
###############################
DHDCFLAGS += -DWL_ENABLE_P2P_IF

# Default definitions for KitKat
DHDCFLAGS += -DWL_CFG80211_STA_EVENT
DHDCFLAGS += -DWL_IFACE_COMB_NUM_CHANNELS

ifneq ($(CONFIG_DHD_USE_SCHED_SCAN),)
DHDCFLAGS += -DWL_SCHED_SCAN
endif

# To support RTT
DHDCFLAGS += -DRTT_SUPPORT
# To support Link Statictics
DHDCFLAGS += -DLINKSTAT_SUPPORT
# To support GSCAN
DHDCFLAGS += -DGSCAN_SUPPORT
# To support Rssi Monitor
DHDCFLAGS += -DRSSI_MONITOR_SUPPORT

# To support WL_VENDOR_EXT_SUPPORT
DHDCFLAGS += -DWL_VENDOR_EXT_SUPPORT
# To support ANQPO - GSCAN must be supported
DHDCFLAGS += -DANQPO_SUPPORT

# Extra file list for Lollipop
ANDROID_OFILES := wl_cfgvendor.o dhd_rtt.o bcmxtlv.o bcm_app_utils.o




##########################
# driver type
# m: module type driver
# y: built-in type driver
##########################
DRIVER_TYPE ?= y

#########################
# Chip dependent feature
#########################

# Chipsets supported both SDIO and PCIE
ifneq ($(CONFIG_BCM4359),)
  DHDCFLAGS += -DBCM4359_CHIP -DSUPPORT_MULTIPLE_REVISION
  DHDCFLAGS += -DMIMO_ANT_SETTING

  DHDCFLAGS += -DENABLE_BCN_LI_BCN_WAKEUP
  DHDCFLAGS += -DDHD_DONOT_FORWARD_BCMEVENT_AS_NETWORK_PKT
#  DHDCFLAGS += -DCUSTOM_BCN_TIMEOUT=6
#  DHDCFLAGS += -DSOFTAP_UAPSD_OFF
# tput enhancement for common
  DHDCFLAGS += -DUSE_WL_TXBF
  DHDCFLAGS += -DUSE_WL_FRAMEBURST
  DHDCFLAGS += -DDISABLE_PM_BCNRX
  DHDCFLAGS += -DPROP_TXSTATUS_VSDB

# tput enhancement for PCIE
ifeq ($(BUS_IFACE_PCIE),y)
  DHDCFLAGS += -DCUSTOM_AMPDU_BA_WSIZE=64 -DCUSTOM_IBSS_AMPDU_BA_WSIZE=16
  DHDCFLAGS += -DCUSTOM_AMPDU_MPDU=48
  DHDCFLAGS += -DCUSTOM_AMPDU_RELEASE=32
  DHDCFLAGS += -DCUSTOM_AMSDU_AGGSF=2
  DHDCFLAGS += -DCUSTOM_TCPACK_SUPP_RATIO=15
  DHDCFLAGS += -DCUSTOM_TCPACK_DELAY_TIME=10
  DHDCFLAGS += -DDHDTCPACK_SUPPRESS
endif

# For Runtime PM feature
ifeq ($(BUS_IFACE_PCIE),y)
  DHDCFLAGS += -DBCMPCIE_OOB_HOST_WAKE
  DHDCFLAGS += -DDHD_PCIE_RUNTIMEPM -DMAX_IDLE_COUNT=11 -DCUSTOM_DHD_RUNTIME_MS=100
endif

ifeq ($(CONFIG_ARCH_QCOM),y)
#  DHDCFLAGS += -DSET_RPS_CPUS
  DHDCFLAGS := $(filter-out -DDHDTCPACK_SUPPRESS,$(DHDCFLAGS))
ifeq ($(BUS_IFACE_PCIE),y)
  DHDCFLAGS += -DBCMPCIE_DISABLE_ASYNC_SUSPEND
endif
endif

# New Features
  DHDCFLAGS += -DWL11U -DMFP -DQOS_MAP_SET
  DHDCFLAGS += -DBCMCCX
  DHDCFLAGS += -DWLTDLS
  DHDCFLAGS += -DWLFBT
  DHDCFLAGS += -DDHD_ENABLE_LPC
  DHDCFLAGS += -DSUPPORT_LTECX
#  DHDCFLAGS += -DSUPPORT_2G_VHT
  DHDCFLAGS += -DSUPPORT_WL_TXPOWER
  DHDCFLAGS += -DWL11ULB
  DHDCFLAGS += -DDISABLE_TXBFR
# DHDCFLAGS += -DWL_ABORT_SCAN

 # Support CUSTOM_EVENT_PM_WAKE
  DHDCFLAGS += -DCUSTOM_EVENT_PM_WAKE=30
 # virtual interface for RSDB
  DHDCFLAGS += -DWL_VIRTUAL_APSTA
 # Remove common feature for BCM4359
  DHDCFLAGS :=$(filter-out -DDISABLE_11H_SOFTAP,$(DHDCFLAGS))
ifeq ($(CONFIG_BCM4359),y)
  DHDCFLAGS += -DENABLE_INSMOD_NO_FW_LOAD
  DHDCFLAGS += -DUSE_LATE_INITCALL_SYNC
endif
ifeq ($(CONFIG_ARCH_MSM8996),y)
 # Enable Dongle Isolation
  DHDCFLAGS += -DDONGLE_ENABLE_ISOLATION
endif
endif

ifneq ($(CONFIG_BCM4356),)
  DHDCFLAGS += -DBCM4356_CHIP
  DHDCFLAGS += -DMIMO_ANT_SETTING
  DHDCFLAGS += -DENABLE_BCN_LI_BCN_WAKEUP
  DHDCFLAGS += -DDHD_DONOT_FORWARD_BCMEVENT_AS_NETWORK_PKT

# tput enhancement for common
  DHDCFLAGS += -DUSE_WL_TXBF
  DHDCFLAGS += -DUSE_WL_FRAMEBURST
  DHDCFLAGS += -DPROP_TXSTATUS_VSDB
  DHDCFLAGS += -DDISABLE_FRAMEBURST_VSDB
  DHDCFLAGS += -DDISABLE_PM_BCNRX

# tput enhancement for SDIO
ifeq ($(BUS_IFACE_SDIO),y)
  DHDCFLAGS += -DHW_OOB
  DHDCFLAGS += -DCUSTOM_GLOM_SETTING=8 -DCUSTOM_RXCHAIN=1
  DHDCFLAGS += -DUSE_DYNAMIC_F2_BLKSIZE -DDYNAMIC_F2_BLKSIZE_FOR_NONLEGACY=128
  DHDCFLAGS += -DBCMSDIOH_TXGLOM -DCUSTOM_TXGLOM=1 -DBCMSDIOH_TXGLOM_HIGHSPEED
  DHDCFLAGS += -DRXFRAME_THREAD
  DHDCFLAGS += -DCUSTOM_AMPDU_BA_WSIZE=64 -DCUSTOM_IBSS_AMPDU_BA_WSIZE=16
  DHDCFLAGS += -DCUSTOM_DPC_CPUCORE=0
  DHDCFLAGS += -DCUSTOM_DEF_TXGLOM_SIZE=40 -DDHD_TXBOUND=40
  DHDCFLAGS += -DENABLE_ADAPTIVE_SCHED -DCUSTOM_CPUFREQ_THRESH=1000000
  DHDCFLAGS += -DCUSTOM_MAX_TXGLOM_SIZE=40
  DHDCFLAGS += -DMAX_HDR_READ=128
  DHDCFLAGS += -DDHD_FIRSTREAD=128
  DHDCFLAGS += -DCUSTOM_AMPDU_MPDU=16
  DHDCFLAGS += -DDHDTCPACK_SUPPRESS
endif

# tput enhancement for PCIE
ifeq ($(BUS_IFACE_PCIE),y)
  DHDCFLAGS += -DCUSTOM_AMPDU_BA_WSIZE=64 -DCUSTOM_IBSS_AMPDU_BA_WSIZE=16
  DHDCFLAGS += -DCUSTOM_AMPDU_MPDU=32
  DHDCFLAGS += -DCUSTOM_AMPDU_RELEASE=16
# DHDCFLAGS += -DCUSTOM_AMSDU_AGGSF=4
# DHDCFLAGS += -DSET_PCIEIRQ_CPU0
endif


ifeq ($(BUS_IFACE_PCIE),y)
  DHDCFLAGS += -DBCMPCIE_OOB_HOST_WAKE
  DHDCFLAGS += -DCUSTOM_DHD_WATCHDOG_MS=0
  DHDCFLAGS += -DDHD_PCIE_RUNTIMEPM -DMAX_IDLE_COUNT=11 -DCUSTOM_DHD_RUNTIME_MS=100
endif

ifeq ($(CONFIG_ARCH_QCOM),y)
  #DHDCFLAGS += -DSET_RPS_CPUS
ifeq ($(BUS_IFACE_PCIE),y)
  DHDCFLAGS += -DBCMPCIE_DISABLE_ASYNC_SUSPEND
endif
endif

# New Features
  DHDCFLAGS += -DWL11U -DMFP
  DHDCFLAGS += -DBCMCCX
  DHDCFLAGS += -DWLTDLS
  DHDCFLAGS += -DWLFBT
  DHDCFLAGS += -DDHD_ENABLE_LPC
  DHDCFLAGS += -DSUPPORT_LTECX
# DHDCFLAGS += -DSUPPORT_2G_VHT
  DHDCFLAGS += -DSUPPORT_WL_TXPOWER
# DHDCFLAGS += -DDISABLE_IF_COUNTERS
  DHDCFLAGS += -DDISABLE_TXBFR

#BCM4356
ifeq ($(CONFIG_BCM4356),y)
  DHDCFLAGS += -DENABLE_INSMOD_NO_FW_LOAD
  DHDCFLAGS += -DUSE_LATE_INITCALL_SYNC
endif
ifeq ($(BUS_IFACE_SDIO),y)
  DHDCFLAGS += -DCUSTOM_PSPRETEND_THR=30
endif
endif

ifneq ($(CONFIG_BCM4354),)
  DHDCFLAGS += -DBCM4354_CHIP
  DHDCFLAGS += -DMIMO_ANT_SETTING
  DHDCFLAGS += -DUSE_CID_CHECK
  DHDCFLAGS += -DENABLE_BCN_LI_BCN_WAKEUP

# tput enhancement for common
  DHDCFLAGS += -DUSE_WL_TXBF
  DHDCFLAGS += -DUSE_WL_FRAMEBURST
  DHDCFLAGS += -DPROP_TXSTATUS_VSDB

# tput enhancement for SDIO
ifeq ($(BUS_IFACE_SDIO),y)
  DHDCFLAGS += -DHW_OOB
  DHDCFLAGS += -DCUSTOM_GLOM_SETTING=8 -DCUSTOM_RXCHAIN=1
  DHDCFLAGS += -DUSE_DYNAMIC_F2_BLKSIZE -DDYNAMIC_F2_BLKSIZE_FOR_NONLEGACY=128
  DHDCFLAGS += -DBCMSDIOH_TXGLOM -DCUSTOM_TXGLOM=1 -DBCMSDIOH_TXGLOM_HIGHSPEED
  DHDCFLAGS += -DRXFRAME_THREAD
  DHDCFLAGS += -DCUSTOM_AMPDU_BA_WSIZE=64 -DCUSTOM_IBSS_AMPDU_BA_WSIZE=16
  DHDCFLAGS += -DCUSTOM_DPC_CPUCORE=0
  DHDCFLAGS += -DCUSTOM_DEF_TXGLOM_SIZE=40 -DDHD_TXBOUND=40
  DHDCFLAGS += -DENABLE_ADAPTIVE_SCHED -DCUSTOM_CPUFREQ_THRESH=1000000
  DHDCFLAGS += -DCUSTOM_MAX_TXGLOM_SIZE=40
  DHDCFLAGS += -DMAX_HDR_READ=128
  DHDCFLAGS += -DDHD_FIRSTREAD=128
  DHDCFLAGS += -DCUSTOM_AMPDU_MPDU=16
  DHDCFLAGS += -DDHDTCPACK_SUPPRESS
endif

# tput enhancement for PCIE
ifeq ($(BUS_IFACE_PCIE),y)
  DHDCFLAGS += -DCUSTOM_AMPDU_BA_WSIZE=64 -DCUSTOM_IBSS_AMPDU_BA_WSIZE=16
  DHDCFLAGS += -DCUSTOM_AMPDU_MPDU=32
  DHDCFLAGS += -DCUSTOM_AMPDU_RELEASE=16
endif

ifeq ($(BUS_IFACE_PCIE),y)
  DHDCFLAGS += -DBCMPCIE_OOB_HOST_WAKE
endif



# New Features
  DHDCFLAGS += -DWL11U
  DHDCFLAGS += -DBCMCCX
  DHDCFLAGS += -DWLTDLS
  DHDCFLAGS += -DWLFBT
  DHDCFLAGS += -DDHD_ENABLE_LPC
# DHDCFLAGS += -DWLAIBSS
  DHDCFLAGS += -DSUPPORT_LTECX
#  DHDCFLAGS += -DSUPPORT_2G_VHT
  DHDCFLAGS += -DSUPPORT_WL_TXPOWER
ifeq ($(CONFIG_BCM4354),y)
  DHDCFLAGS += -DENABLE_INSMOD_NO_FW_LOAD
  DHDCFLAGS += -DUSE_LATE_INITCALL_SYNC
endif

ifeq ($(BUS_IFACE_SDIO),y)
  DHDCFLAGS += -DCUSTOM_PSPRETEND_THR=30
endif
endif

# Chipsets supported SDIO only
ifeq ($(BUS_IFACE_SDIO),y)
ifneq ($(CONFIG_BCM4339),)
  DHDCFLAGS += -DBCM4339_CHIP -DHW_OOB
  DHDCFLAGS += -DUSE_CID_CHECK
  DHDCFLAGS += -DENABLE_BCN_LI_BCN_WAKEUP
  DHDCFLAGS += -DUSE_SDIOFIFO_IOVAR

  # tput enhancement
  DHDCFLAGS += -DCUSTOM_GLOM_SETTING=8 -DCUSTOM_RXCHAIN=1
  DHDCFLAGS += -DUSE_DYNAMIC_F2_BLKSIZE -DDYNAMIC_F2_BLKSIZE_FOR_NONLEGACY=128
  DHDCFLAGS += -DBCMSDIOH_TXGLOM -DCUSTOM_TXGLOM=1 -DBCMSDIOH_TXGLOM_HIGHSPEED
  DHDCFLAGS += -DDHDTCPACK_SUPPRESS
  DHDCFLAGS += -DUSE_WL_TXBF
  DHDCFLAGS += -DUSE_WL_FRAMEBURST
  DHDCFLAGS += -DRXFRAME_THREAD
  DHDCFLAGS += -DCUSTOM_AMPDU_BA_WSIZE=64 -DCUSTOM_IBSS_AMPDU_BA_WSIZE=16
  DHDCFLAGS += -DCUSTOM_DPC_CPUCORE=0
  DHDCFLAGS += -DPROP_TXSTATUS_VSDB
ifeq ($(CONFIG_ARCH_QCOM),y)
  DHDCFLAGS += -DCUSTOM_DEF_TXGLOM_SIZE=32 -DDHD_TXBOUND=32
  DHDCFLAGS += -DENABLE_ADAPTIVE_SCHED -DCUSTOM_CPUFREQ_THRESH=1000000
endif
  DHDCFLAGS += -DCUSTOM_MAX_TXGLOM_SIZE=32
  DHDCFLAGS += -DMAX_CNTL_TX_TIMEOUT=3

  # New Features
  DHDCFLAGS += -DWL11U
  DHDCFLAGS += -DBCMCCX
  DHDCFLAGS += -DWLTDLS
  DHDCFLAGS += -DWLFBT
  DHDCFLAGS += -DDHD_ENABLE_LPC
  DHDCFLAGS += -DSUPPORT_LTECX
# DHDCFLAGS += -DSUPPORT_2G_VHT
  DHDCFLAGS += -DSUPPORT_WL_TXPOWER
ifeq ($(CONFIG_BCM4339),y)
  DHDCFLAGS += -DENABLE_INSMOD_NO_FW_LOAD
  DHDCFLAGS += -DUSE_LATE_INITCALL_SYNC
endif
  DHDCFLAGS += -DCUSTOM_PSPRETEND_THR=30
  DHDCFLAGS += -DDISABLE_WL_FRAMEBURST_SOFTAP
endif
endif
#############################
# Platform dependent feature
#############################
#########
# Others
#########


EXTRA_LDFLAGS += --strip-debug
EXTRA_CFLAGS += $(DHDCFLAGS) -DDHD_DEBUG
EXTRA_CFLAGS += -DSRCBASE=\"$(src)\"
EXTRA_CFLAGS += -I$(src)/include/ -I$(src)/
KBUILD_CFLAGS += -I$(LINUXDIR)/include -I$(shell pwd)

DHDOFILES := dhd_pno.o dhd_common.o dhd_ip.o dhd_custom_gpio.o \
	dhd_linux.o dhd_linux_sched.o dhd_cfg80211.o dhd_linux_wq.o aiutils.o bcmevent.o \
	bcmutils.o bcmwifi_channels.o hndpmu.o linux_osl.o sbutils.o siutils.o \
	wl_android.o wl_cfg80211.o wl_cfgp2p.o wl_cfg_btcoex.o wldev_common.o \
	wl_linux_mon.o wl_roam.o dhd_linux_platdev.o dhd_linux_wq.o wl_cfg_btcoex.o \
	dhd_wlfc.o hnd_pktq.o hnd_pktpool.o dhd_debug.o dhd_debug_linux.o


ifeq ($(BUS_IFACE_SDIO),y)
DHDOFILES += bcmsdh.o bcmsdh_linux.o bcmsdh_sdmmc.o bcmsdh_sdmmc_linux.o
DHDOFILES += dhd_cdc.o dhd_sdio.o
endif


ifeq ($(BUS_IFACE_PCIE),y)
DHDOFILES += dhd_pcie.o dhd_pcie_linux.o pcie_core.o dhd_flowring.o dhd_msgbuf.o
endif


DHDOFILES += $(ANDROID_OFILES)

# NAN feature
ifeq ($(CONFIG_WL_NAN),y)
DHDOFILES += wl_cfgnan.o bcmxtlv.o
endif
bcmdhd-objs := $(DHDOFILES)
obj-$(DRIVER_TYPE)   += bcmdhd.o

all:
	@echo "$(MAKE) --no-print-directory -C $(KDIR) SUBDIRS=$(CURDIR) modules"
	@$(MAKE) --no-print-directory -C $(KDIR) SUBDIRS=$(CURDIR) modules

clean:
	rm -rf *.o *.ko *.mod.c *~ .*.cmd *.o.cmd .*.o.cmd \
	Module.symvers modules.order .tmp_versions modules.builtin

install:
	@$(MAKE) --no-print-directory -C $(KDIR) \
		SUBDIRS=$(CURDIR) modules_install
