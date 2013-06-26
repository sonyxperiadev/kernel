# bcmdhd_4330
#####################
# SDIO Basic feature
#####################

DHDCFLAGS = -Wall -Wstrict-prototypes -Dlinux -DLINUX -DBCMDRIVER             \
        -DBCMDONGLEHOST -DUNRELEASEDCHIP -DBCMDMA32 -DBCMFILEIMAGE            \
        -DDHDTHREAD -DBDC -DOOB_INTR_ONLY                                     \
        -DDHD_BCMEVENTS -DSHOW_EVENTS -DBCMDBG                                \
        -DMMC_SDIO_ABORT -DBCMSDIO -DBCMLXSDMMC -DBCMPLATFORM_BUS -DWLP2P     \
        -DWIFI_ACT_FRAME -DARP_OFFLOAD_SUPPORT          \
        -DKEEP_ALIVE -DCSCAN -DPKT_FILTER_SUPPORT                             \
        -DEMBEDDED_PLATFORM -DPNO_SUPPORT

#################
# Common feature
#################

DHDCFLAGS += -DCUSTOMER_HW4
DHDCFLAGS += -DBLOCK_IPV6_PACKET -DPASS_IPV4_SUSPEND
DHDCFLAGS += -DSUPPORT_DEEP_SLEEP
DHDCFLAGS += -DSIMPLE_MAC_PRINT

# Print out kernel panic point of file and line info when assertion happened
DHDCFLAGS += -DBCMASSERT_LOG

# For p2p connection issue
DHDCFLAGS += -DWL_CFG80211_GON_COLLISION
DHDCFLAGS += -DWL_SCB_TIMEOUT=10

# For Passing all multicast packets to host when not in suspend mode.
DHDCFLAGS += -DPASS_ALL_MCAST_PKTS

# Early suspend
DHDCFLAGS += -DDHD_USE_EARLYSUSPEND  

DHDCFLAGS += -DSUPPORT_PM2_ONLY

# For Scan result patch
DHDCFLAGS += -DESCAN_RESULT_PATCH
DHDCFLAGS += -DDUAL_ESCAN_RESULT_BUFFER

DHDCFLAGS += -DROAM_ENABLE -DROAM_CHANNEL_CACHE -DROAM_API 
DHDCFLAGS += -DENABLE_FW_ROAM_SUSPEND

# For Static Buffer
ifeq ($(CONFIG_BROADCOM_WIFI_RESERVED_MEM),y)
  DHDCFLAGS += -DCONFIG_DHD_USE_STATIC_BUF
  DHDCFLAGS += -DENHANCED_STATIC_BUF
  DHDCFLAGS += -DSTATIC_WL_PRIV_STRUCT
endif

# For CCX
ifeq ($(CONFIG_BRCM_CCX),y)
  DHDCFLAGS += -DBCMCCX
endif

DHDCFLAGS += -DWL_CFG80211

# SoftAP
DHDCFLAGS += -DSUPPORT_AUTO_CHANNEL -DSUPPORT_HIDDEN_AP
DHDCFLAGS += -DSUPPORT_SOFTAP_SINGL_DISASSOC 
DHDCFLAGS += -DUSE_STAMAC_4SOFTAP 

# DPC priority
DHDCFLAGS += -DCUSTOM_DPC_PRIO_SETTING=98

# WiFi turn off delay
DHDCFLAGS += -DWIFI_TURNOFF_DELAY=100

# DTIM listen interval in suspend mode(0 means follow AP's DTIM period)
DHDCFLAGS += -DCUSTOM_SUSPEND_BCN_LI_DTIM=3

# Priority mismatch fix with kernel stack
DHDCFLAGS += -DPKTPRIO_OVERRIDE

# Ioctl timeout 5000ms
DHDCFLAGS += -DIOCTL_RESP_TIMEOUT=5000

# WiFi Kernel thread type
DHDCFLAGS += -DUSE_KTHREAD_API

############
# JellyBean 
############
DHDCFLAGS += -DWL_ENABLE_P2P_IF
DHDCFLAGS += -DMULTIPLE_SUPPLICANT
DHDCFLAGS += -DWL_CFG80211_STA_EVENT

#################
# JellyBean Plus
#################
DHDCFLAGS += -DWL_SUPPORT_BACKPORTED_KPATCHES

#########################
# Chip dependent feature
#########################

ifneq ($(CONFIG_BCMDHD_4334),)
#  DHDCFLAGS += -DBCM4334_CHIP -DHW_OOB -DSUPPORT_MULTIPLE_REVISION
#  DHDCFLAGS += -DUSE_CID_CHECK -DCONFIG_CONTROL_PM
#  DHDCFLAGS += -DPROP_TXSTATUS -DPROP_TXSTATUS_VSDB
#  DHDCFLAGS += -DVSDB -DHT40_GO
#  DHDCFLAGS += -DWL_CFG80211_VSDB_PRIORITIZE_SCAN_REQUEST
#  DHDCFLAGS += -DDHD_USE_IDLECOUNT
#  DHDCFLAGS += -DSUPPORT_AMPDU_MPDU_CMD
#  DHDCFLAGS += -DUSE_DYNAMIC_F2_BLKSIZE -DDYNAMIC_F2_BLKSIZE_FOR_NONLEGACY=64
#  DHDCFLAGS += -DCUSTOM_GLOM_SETTING=5
#  DHDCFLAGS += -DROAM_AP_ENV_DETECTION
#  DHDCFLAGS += -DWES_SUPPORT
#  DHDCFLAGS :=$(filter-out -DWL_CFG80211_GON_COLLISION,$(DHDCFLAGS))
endif

ifneq ($(CONFIG_BCM4330),)
  DHDCFLAGS += -DBCM4330_CHIP -DSUPPORT_MULTIPLE_REVISION
  DHDCFLAGS += -DMCAST_LIST_ACCUMULATION
  DHDCFLAGS += -DCONFIG_CONTROL_PM
  DHDCFLAGS += -DCUSTOM_GLOM_SETTING=0
  DHDCFLAGS += -DPASS_ARP_PACKET
  DHDCFLAGS += -DWL_CFG80211_VSDB_PRIORITIZE_SCAN_REQUEST
endif

ifneq ($(CONFIG_BCM43241),)
  DHDCFLAGS += -DBCM43241_CHIP -DHW_OOB
  DHDCFLAGS += -DCONFIG_CONTROL_PM
  DHDCFLAGS += -DPROP_TXSTATUS
  DHDCFLAGS += -DVSDB -DHT40_GO
  DHDCFLAGS += -DWL_CFG80211_VSDB_PRIORITIZE_SCAN_REQUEST
  DHDCFLAGS += -DDHD_USE_IDLECOUNT
  DHDCFLAGS += -DSUPPORT_AMPDU_MPDU_CMD
  DHDCFLAGS += -DMIMO_ANT_SETTING -DAMPDU_HOSTREORDER
  DHDCFLAGS += -DCUSTOM_GLOM_SETTING=1 -DCUSTOM_SDIO_F2_BLKSIZE=128
  DHDCFLAGS += -DROAM_AP_ENV_DETECTION
  DHDCFLAGS += -DSDIO_CRC_ERROR_FIX
  DHDCFLAGS :=$(filter-out -DWL_CFG80211_GON_COLLISION,$(DHDCFLAGS))
  ifeq ($(CONFIG_BCM43241),m)
    DHDCFLAGS += -fno-pic
  endif
endif


#############################
# Platform dependent feature
#############################

ifeq ($(CONFIG_SPI_SC8810),y)
  DHDCFLAGS += -DREAD_MACADDR -DBCMSPI -DBCMSPI_ANDROID -DSPI_PIO_32BIT_RW -DSPI_PIO_RW_BIGENDIAN
  DHDCFLAGS += -UCUSTOM_DPC_PRIO_SETTING

  #Remove defines for SDMMC
  DHDCFLAGS :=$(filter-out -DOOB_INTR_ONLY,$(DHDCFLAGS))
  DHDCFLAGS :=$(filter-out -DBCMLXSDMMC,$(DHDCFLAGS))

  #Remove defines for JB
  DHDCFLAGS :=$(filter-out -DWL_ENABLE_P2P_IF,$(DHDCFLAGS))
  DHDCFLAGS :=$(filter-out -DMULTIPLE_SUPPLICANT,$(DHDCFLAGS))
  DHDCFLAGS :=$(filter-out -DWL_CFG80211_STA_EVENT,$(DHDCFLAGS))
endif

#For INITIAL 2G scan features
ifneq ($(CONFIG_TARGET_LOCALE_KOR),y)
  DHDCFLAGS += -DUSE_INITIAL_2G_SCAN
endif

# For SLP feature
ifeq ($(CONFIG_SLP),y)
  DHDCFLAGS += -DPLATFORM_SLP
  DHDCFLAGS += -UWL_ENABLE_P2P_IF
  DHDCFLAGS += -UMULTIPLE_SUPPLICANT
  DHDCFLAGS += -UWL_CFG80211_STA_EVENT
endif

# GGSM_WIFI_5GHz_CHANNELS feature is define for only GGSM model
ifeq ($(GGSM_WIFI_5GHz_CHANNELS),true)
  DHDCFLAGS += -DCUSTOMER_SET_COUNTRY
endif

# Remove the MACH specific flag for compilation.
# Since this code base is used for Hawaii and Java kind of variants.
DHDCFLAGS += -UHW_OOB
DHDCFLAGS += -DRDWR_MACADDR
DHDCFLAGS += -DBCMHOST_XTAL_PU_TIME_MOD
# DHDCFLAGS += -DENABLE_INSMOD_NO_FW_LOAD
DHDCFLAGS += -DENABLE_BCN_LI_DTIM
DHDCFLAGS += -USUPPORT_MULTIPLE_REVISION
DHDCFLAGS += -DSET_CUSTOM_NVRAM

# Local defines to allow multiple module builds
ifneq ($(CONFIG_BCMDHD_FW_PATH_4330),)
DHDCFLAGS += -DCONFIG_BCMDHD_FW_PATH=CONFIG_BCMDHD_FW_PATH_4330
endif

ifneq ($(CONFIG_BCMDHD_NVRAM_PATH_4330),)
DHDCFLAGS += -DCONFIG_BCMDHD_NVRAM_PATH=CONFIG_BCMDHD_FW_PATH_4330
endif


##############################################################
# dhd_sec_feature.h
DHDCFLAGS += -include "dhd_sec_feature.h"
##############################################################

ifeq ($(CONFIG_TARGET_LOCALE_CHN),y)
  DHDCFLAGS += -DBCMWAPI_WPI -DBCMWAPI_WAI
endif

#########
# Others
#########

EXTRA_LDFLAGS += --strip-debug

EXTRA_CFLAGS += $(DHDCFLAGS) -DDHD_DEBUG
EXTRA_CFLAGS += -DSRCBASE=\"$(src)\"
EXTRA_CFLAGS += -I$(src)/include/ -I$(src)/
KBUILD_CFLAGS += -I$(LINUXDIR)/include -I$(shell pwd)

DHDOFILES := bcmsdh.o bcmsdh_linux.o bcmsdh_sdmmc.o bcmsdh_sdmmc_linux.o \
	dhd_cdc.o dhd_common.o dhd_custom_gpio.o dhd_custom_sec.o \
	dhd_linux.o dhd_linux_sched.o dhd_cfg80211.o dhd_sdio.o aiutils.o bcmevent.o \
	bcmutils.o bcmwifi_channels.o hndpmu.o linux_osl.o sbutils.o siutils.o \
	wl_android.o wl_cfg80211.o wl_cfgp2p.o wldev_common.o wl_linux_mon.o wl_roam.o

# For SPI projects
ifeq ($(CONFIG_SPI_SC8810),y)
DHDOFILES += bcmsdspi_linux.o bcmspibrcm.o
DHDOFILES :=$(filter-out bcmsdh_sdmmc.o,$(DHDOFILES))
DHDOFILES :=$(filter-out bcmsdh_sdmmc_linux.o,$(DHDOFILES))
endif

obj-$(CONFIG_BCM4330) += bcmdhd_4330.o

bcmdhd_4330-objs += $(DHDOFILES)

all:
	@echo "$(MAKE) --no-print-directory -C $(KDIR) SUBDIRS=$(CURDIR) modules"
	@$(MAKE) --no-print-directory -C $(KDIR) SUBDIRS=$(CURDIR) modules

clean:
	rm -rf *.o *.ko *.mod.c *~ .*.cmd *.o.cmd .*.o.cmd \
	Module.symvers modules.order .tmp_versions modules.builtin

install:
	@$(MAKE) --no-print-directory -C $(KDIR) \
		SUBDIRS=$(CURDIR) modules_install
