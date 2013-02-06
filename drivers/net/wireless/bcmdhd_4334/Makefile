# bcmdhd_4334
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
DHDCFLAGS += -DSUPPORT_DEEP_SLEEP -DWL_CFG80211_STA_EVENT
DHDCFLAGS += -DSIMPLE_MAC_PRINT

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
DHDCFLAGS += -DDISABLE_FW_ROAM_SUSPEND

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


############
# JellyBean 
############
DHDCFLAGS += -DWL_ENABLE_P2P_IF
DHDCFLAGS += -DMULTIPLE_SUPPLICANT

#########################
# Chip dependent feature
#########################

ifneq ($(CONFIG_BCMDHD_4334),)
  DHDCFLAGS += -DBCM4334_CHIP -DHW_OOB -DSUPPORT_MULTIPLE_REVISION
  DHDCFLAGS += -DUSE_CID_CHECK -DCONFIG_CONTROL_PM
  DHDCFLAGS += -DPROP_TXSTATUS
  DHDCFLAGS += -DVSDB -DHT40_GO
  DHDCFLAGS += -DWL_CFG80211_VSDB_PRIORITIZE_SCAN_REQUEST
  DHDCFLAGS += -DDHD_USE_IDLECOUNT
  DHDCFLAGS += -DSUPPORT_AMPDU_MPDU_CMD
  DHDCFLAGS += -DVSDB_DYNAMIC_F2_BLKSIZE -DSDIO_F2_BLKSIZE=512 -DVSDB_F2_BLKSIZE=64
  DHDCFLAGS += -DCUSTOM_GLOM_SETTING=5 -DENABLE_BCN_LI_BCN_WAKEUP
#  DHDCFLAGS += -DWES_SUPPORT
endif

ifneq ($(CONFIG_BCM4330),)
#  DHDCFLAGS += -DBCM4330_CHIP
#  DHDCFLAGS += -DMCAST_LIST_ACCUMULATION
# DHDCFLAGS += -DCONFIG_CONTROL_PM
endif

ifneq ($(CONFIG_BCM43241),)
  DHDCFLAGS += -DBCM43241_CHIP -DHW_OOB
  DHDCFLAGS += -DMCAST_LIST_ACCUMULATION
  DHDCFLAGS += -DMIMO_ANT_SETTING -DCONFIG_CONTROL_PM
  DHDCFLAGS += -DAMPDU_HOSTREORDER -DDHD_USE_IDLECOUNT
  ifeq ($(CONFIG_BCM43241),m)
    DHDCFLAGS += -fno-pic
  endif
endif


#############################
# Platform dependent feature
#############################

ifeq ($(CONFIG_MACH_SAMSUNG_ESPRESSO),y)
  DHDCFLAGS += -DREAD_MACADDR -DHW_OOB
endif

ifeq ($(CONFIG_MACH_SAMSUNG_ESPRESSO_10),y)
  DHDCFLAGS += -DREAD_MACADDR -DHW_OOB
endif

ifeq ($(CONFIG_MACH_U1),y)
  DHDCFLAGS += -DUSE_CID_CHECK -DU1_MACADDR
  ifeq ($(CONFIG_TARGET_LOCALE_KOR),y)
    DHDCFLAGS += -DRDWR_MACADDR
  else
    DHDCFLAGS += -DWRITE_MACADDR
  endif
endif

# NAT keepalive packet filter for GAN-lite
ifeq ($(SEC_MODEL_NAME),U1)
         ifeq ($(X_BUILD_LOCALE),EUR_ORG)
              DHDCFLAGS += -DGAN_LITE_NAT_KEEPALIVE_FILTER
         endif
endif

ifeq ($(CONFIG_MACH_SAMSUNG_T1),y)
  DHDCFLAGS += -DUSE_CID_CHECK -DWRITE_MACADDR
endif


ifeq ($(CONFIG_TARGET_LOCALE_KOR),y)
  DHDCFLAGS += -DOKC_SUPPORT
  ifeq ($(CONFIG_BCM4334),y)
    DHDCFLAGS += -DRDWR_KORICS_MACADDR
  else
    DHDCFLAGS += -DRDWR_MACADDR
  endif
endif

ifeq ($(CONFIG_MACH_U1_KOR_KT), y)
  DHDCFLAGS += -DVLAN_MODE_OFF
  DHDCFLAGS += -DKEEP_ALIVE_PACKET_PERIOD_30_SEC
  DHDCFLAGS += -DFULL_ROAMING_SCAN_PERIOD_60_SEC
endif

#For INITIAL 2G scan features
ifneq ($(CONFIG_TARGET_LOCALE_KOR),y)
  DHDCFLAGS += -DUSE_INITIAL_2G_SCAN
endif

# For SLP feature
ifeq ($(CONFIG_SLP),y)
  DHDCFLAGS += -DPLATFORM_SLP
endif

# GGSM_WIFI_5GHz_CHANNELS feature is define for only GGSM model
ifeq ($(GGSM_WIFI_5GHz_CHANNELS),true)
  DHDCFLAGS += -DCUSTOMER_SET_COUNTRY
endif

ifneq ($(findstring GlobalConfig, $(wildcard $(srctree)/include/sec_feature/*)),)
  DHDCFLAGS += -DUSE_SECFEATURE
endif

ifeq ($(CONFIG_TARGET_LOCALE_CHN),y)
  DHDCFLAGS += -DBCMWAPI_WPI -DBCMWAPI_WAI
endif

ifeq ($(CONFIG_ARCH_HAWAII),y)
DHDCFLAGS += -UHW_OOB
DHDCFLAGS += -DRDWR_MACADDR
DHDCFLAGS += -DBCMHOST_XTAL_PU_TIME_MOD
# DHDCFLAGS += -DENABLE_INSMOD_NO_FW_LOAD
DHDCFLAGS += -DENABLE_BCN_LI_DTIM
DHDCFLAGS += -USUPPORT_MULTIPLE_REVISION
DHDCFLAGS += -DSET_CUSTOM_NVRAM
endif

# Local defines to allow multiple module builds
ifneq ($(CONFIG_BCMDHD_FW_PATH_4334),)
DHDCFLAGS += -DCONFIG_BCMDHD_FW_PATH=CONFIG_BCMDHD_FW_PATH_4334
endif

ifneq ($(CONFIG_BCMDHD_NVRAM_PATH_4334),)
DHDCFLAGS += -DCONFIG_BCMDHD_NVRAM_PATH=CONFIG_BCMDHD_FW_PATH_4334
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

obj-$(CONFIG_BCMDHD_4334) += bcmdhd_4334.o

bcmdhd_4334-objs += $(DHDOFILES)

all:
	@echo "$(MAKE) --no-print-directory -C $(KDIR) SUBDIRS=$(CURDIR) modules"
	@$(MAKE) --no-print-directory -C $(KDIR) SUBDIRS=$(CURDIR) modules

clean:
	rm -rf *.o *.ko *.mod.c *~ .*.cmd *.o.cmd .*.o.cmd \
	Module.symvers modules.order .tmp_versions modules.builtin

install:
	@$(MAKE) --no-print-directory -C $(KDIR) \
		SUBDIRS=$(CURDIR) modules_install
