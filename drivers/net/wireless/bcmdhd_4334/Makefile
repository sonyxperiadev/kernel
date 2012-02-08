# bcmdhd
DHDCFLAGS = -Wall -Wstrict-prototypes -Dlinux -DBCMDRIVER                     \
        -DBCMDONGLEHOST -DUNRELEASEDCHIP -DBCMDMA32 -DWLBTAMP -DBCMFILEIMAGE  \
        -DDHDTHREAD -DDHD_GPL -DDHD_SCHED -DDHD_DEBUG -DSDTEST -DBDC -DTOE    \
        -DDHD_BCMEVENTS -DSHOW_EVENTS -DDONGLEOVERLAYS -DBCMDBG               \
        -DCUSTOMER_HW_SAMSUNG -DOOB_INTR_ONLY                                 \
        -DMMC_SDIO_ABORT -DBCMSDIO -DBCMLXSDMMC -DBCMPLATFORM_BUS -DWLP2P     \
        -DNEW_COMPAT_WIRELESS -DWIFI_ACT_FRAME -DARP_OFFLOAD_SUPPORT          \
        -DKEEP_ALIVE -DCSCAN -DPKT_FILTER_SUPPORT                             \
        -DEMBEDDED_PLATFORM -DPNO_SUPPORT \
        -DCUSTOMER_HW3

DHDCFLAGS += \
	-Idrivers/net/wireless/bcmdhd_4334/src/dhd/sys \
	-Idrivers/net/wireless/bcmdhd_4334/src/bcmsdio/sys \
	-Idrivers/net/wireless/bcmdhd_4334/src/shared \
	-Idrivers/net/wireless/bcmdhd_4334/src/dongle \
	-Idrivers/net/wireless/bcmdhd_4334/src/wl/exe \
	-Idrivers/net/wireless/bcmdhd_4334/src/wl/sys \
	-Idrivers/net/wireless/bcmdhd_4334/src/include

# distinguish between the 43xx chip
ifeq ($(CONFIG_BCM4334),m)
DHDCFLAGS += -DHW_OOB -DBCM4334_CHECK_CHIP_REV
DHDCFLAGS += -DUSE_CID_CHECK -DCONFIG_CONTROL_PM
DHDCFLAGS += -DPROP_TXSTATUS
endif
ifeq ($(CONFIG_BCM4334),y)
DHDCFLAGS += -DHW_OOB -DBCM4334_CHECK_CHIP_REV
DHDCFLAGS += -DUSE_CID_CHECK -DCONFIG_CONTROL_PM
DHDCFLAGS += -DPROP_TXSTATUS
endif
ifeq ($(CONFIG_BCM4330),m)
DHDCFLAGS += -DUSE_CID_CHECK -DRDWR_MACADDR
endif
ifeq ($(CONFIG_BCM4330),y)
DHDCFLAGS += -DUSE_CID_CHECK -DRDWR_MACADDR
endif

# For Static Buffer
ifeq ($(CONFIG_BROADCOM_WIFI_RESERVED_MEM),y)
DHDCFLAGS += -DCONFIG_DHD_USE_STATIC_BUF
endif

ifeq ($(CONFIG_MACH_RHEA_RAY_EDN2X),y)
DHDCFLAGS += -UOOB_INTR_ONLY  -DSDIO_ISR_THREAD       
DHDCFLAGS += -UDHD_DEBUG
endif


# For Debug
EXTRA_CFLAGS += $(DHDCFLAGS) -DWL_CFG80211
EXTRA_CFLAGS += -DDHD_DEBUG -DSRCBASE=\"$(src)/src\"

EXTRA_CFLAGS += -I$(src)/src/include/
EXTRA_CFLAGS += -I$(src)/src/dhd/sys/
EXTRA_CFLAGS += -I$(src)/src/dongle/
EXTRA_CFLAGS += -I$(src)/src/bcmsdio/sys/
EXTRA_CFLAGS += -I$(src)/src/wl/sys/
EXTRA_CFLAGS += -I$(src)/src/shared/


DHDOFILES = src/bcmsdio/sys/bcmsdh.o	src/bcmsdio/sys/bcmsdh_linux.o \
	 src/bcmsdio/sys/bcmsdh_sdmmc.o	src/bcmsdio/sys/bcmsdh_sdmmc_linux.o \
	 src/dhd/sys/dhd_bta.o		src/dhd/sys/dhd_cdc.o \
	 src/dhd/sys/dhd_common.o	src/dhd/sys/dhd_custom_gpio.o \
	 src/dhd/sys/dhd_custom_sec.o \
	 src/dhd/sys/dhd_linux.o	src/dhd/sys/dhd_linux_sched.o \
	 src/dhd/sys/dhd_cfg80211.o	src/dhd/sys/dhd_sdio.o \
	 src/shared/aiutils.o		src/shared/bcmevent.o \
	 src/shared/bcmutils.o		src/shared/bcmwifi.o \
	 src/shared/hndpmu.o		src/shared/linux_osl.o \
	 src/shared/sbutils.o		src/shared/siutils.o \
	 src/wl/sys/wl_android.o	src/wl/sys/wl_cfg80211.o \
	 src/wl/sys/wl_cfgp2p.o		src/wl/sys/wldev_common.o \
	 src/wl/sys/wl_linux_mon.o






# Uncomment to enable firmware dongle debug messages.
# DHDCFLAGS += -DDHD_DEBUG -DSRCBASE=\"$(SRCBASE)\"

obj-$(CONFIG_BCMDHD_4334) += bcmdhd.o
bcmdhd-objs += $(DHDOFILES)
ifneq ($(CONFIG_WIRELESS_EXT),)
bcmdhd-objs += wl_iw.o
DHDCFLAGS += -DSOFTAP
endif

# Use CFG80211 for Android ICS only.
### Until if fix bmake, this is on so ICS gets WiFi.
###ifneq ($(CONFIG_ANDROID_CFG80211),)
ifneq ($(CONFIG_CFG80211),)
bcmdhd-objs += src/wl/sys/wl_cfg80211.o src/wl/sys/wl_cfgp2p.o  src/wl/sys/wl_linux_mon.o
DHDCFLAGS += -DWL_CFG80211 -DWLP2P
endif
###endif 

EXTRA_CFLAGS = $(DHDCFLAGS)

# Android needs CSCAN to update AP list.
ifeq ($(CONFIG_ANDROID),y)
DHDCFLAGS += -DCSCAN
endif

ifeq ($(CONFIG_BCMDHD_4334),m)
EXTRA_LDFLAGS += --strip-debug
endif




