# bcmdhd
DHDCFLAGS = -Wall -Wstrict-prototypes -Dlinux -DBCMDRIVER                     \
        -DBCMDONGLEHOST -DUNRELEASEDCHIP -DBCMDMA32 -DWLBTAMP -DBCMFILEIMAGE  \
        -DDHDTHREAD -DDHD_GPL -DDHD_SCHED -DDHD_DEBUG -DBDC -DTOE    \
        -DDHD_BCMEVENTS -DSHOW_EVENTS -DDONGLEOVERLAYS -DBCMDBG               \
        -DCUSTOMER_HW_SAMSUNG -DOOB_INTR_ONLY                                 \
        -DMMC_SDIO_ABORT -DBCMSDIO -DBCMLXSDMMC -DBCMPLATFORM_BUS -DWLP2P     \
        -DNEW_COMPAT_WIRELESS -DWIFI_ACT_FRAME -DARP_OFFLOAD_SUPPORT          \
        -DKEEP_ALIVE -DCSCAN -DPKT_FILTER_SUPPORT                             \
        -DEMBEDDED_PLATFORM -DPNO_SUPPORT \
	 -DCUSTOMER_HW3
	 
# distinguish between the 43xx chip
ifeq ($(CONFIG_BCM4334),m)
DHDCFLAGS += -DBCM4334_CHIP -DHW_OOB -DBCM4334_CHECK_CHIP_REV
DHDCFLAGS += -DUSE_CID_CHECK -DCONFIG_CONTROL_PM
DHDCFLAGS += -DPROP_TXSTATUS
DHDCFLAGS += -DVSDB
endif
ifeq ($(CONFIG_BCM4334),y)
DHDCFLAGS += -DBCM4334_CHIP -DHW_OOB -DBCM4334_CHECK_CHIP_REV
DHDCFLAGS += -DUSE_CID_CHECK -DCONFIG_CONTROL_PM
DHDCFLAGS += -DPROP_TXSTATUS
DHDCFLAGS += -DVSDB
DHDCFLAGS += -DWL_CFG80211_VSDB_PRIORITIZE_SCAN_REQUEST
endif
ifeq ($(CONFIG_BCM4330),m)
DHDCFLAGS += -DBCM4330_CHIP -DUSE_CID_CHECK -DRDWR_MACADDR
DHDCFLAGS += -DMCAST_LIST_ACCUMULATION
endif
ifeq ($(CONFIG_BCM4330),y)
DHDCFLAGS += -DBCM4330_CHIP -DUSE_CID_CHECK -DRDWR_MACADDR
DHDCFLAGS += -DMCAST_LIST_ACCUMULATION
endif
ifeq ($(CONFIG_BCM43241),m)
DHDCFLAGS += -DBCM43241_CHIP -DHW_OOB
DHDCFLAGS += -DMCAST_LIST_ACCUMULATION
DHDCFLAGS += -fno-pic
endif
ifeq ($(CONFIG_BCM43241),y)
DHDCFLAGS += -DBCM43241_CHIP -HW_OOB
DHDCFLAGS += -DMCAST_LIST_ACCUMULATION
endif

# For p2p connection issue
DHDCFLAGS += -DWL_CFG80211_GON_COLLISION
DHDCFLAGS += -DWL_CFG80211_SYNC_GON_TIME
#end p2p connection

# For Passing all multicast packets to host when not in suspend mode.
DHDCFLAGS += -DPASS_ALL_MCAST_PKTS
DHDCFLAGS += -DUSE_INITIAL_2G_SCAN

# For Scan result patch
DHDCFLAGS += -DESCAN_RESULT_PATCH

# For Static Buffer
ifeq ($(CONFIG_BROADCOM_WIFI_RESERVED_MEM),y)
DHDCFLAGS += -DCONFIG_DHD_USE_STATIC_BUF
endif

# For CCX 
ifeq ($(CONFIG_BRCM_CCX),y)
DHDCFLAGS += -DBCMCCX
endif

# For Korean domestic projects
# DHDCFLAGS += -DROAM_CHANNEL_CACHE
# DHDCFLAGS += -DROAM_ENABLE
# DHDCFLAGS += -DROAM_API
# DHDCFLAGS += -DOKC_SUPPORT

ifeq ($(CONFIG_MACH_RHEA_RAY_EDN2X),y)
DHDCFLAGS += -UHW_OOB -UUSE_CID_CHECK
DHDCFLAGS += -DBCM4334_CHIP -DBCM4334_CHECK_CHIP_REV -DCONFIG_CONTROL_PM -DCONFIG_DHD_USE_STATIC_BUF
DHDCFLAGS += -DRDWR_MACADDR
DHDCFLAGS += -DCUSTOM_OOB_GPIO_NUM=85
endif




ifeq ($(CONFIG_MACH_RHEA_STONE),y)
DHDCFLAGS += -UHW_OOB -UUSE_CID_CHECK
DHDCFLAGS += -DBCM4334_CHIP -DBCM4334_CHECK_CHIP_REV -DCONFIG_CONTROL_PM -DCONFIG_DHD_USE_STATIC_BUF
DHDCFLAGS += -DRDWR_MACADDR
DHDCFLAGS += -DCUSTOM_OOB_GPIO_NUM=74

endif

ifeq ($(CONFIG_MACH_RHEA_STONE_EDN2X),y)
DHDCFLAGS += -UHW_OOB -UUSE_CID_CHECK
DHDCFLAGS += -DBCM4334_CHIP -DBCM4334_CHECK_CHIP_REV -DCONFIG_CONTROL_PM -DCONFIG_DHD_USE_STATIC_BUF
DHDCFLAGS += -DRDWR_MACADDR
DHDCFLAGS += -DCUSTOM_OOB_GPIO_NUM=74
endif


# For Debug
EXTRA_CFLAGS += $(DHDCFLAGS) -DWL_CFG80211 -DRSSI_OFFSET=0
EXTRA_CFLAGS += -DDHD_DEBUG -DSRCBASE=\"$(src)/src\"

EXTRA_CFLAGS += -I$(src)/src/include/
EXTRA_CFLAGS += -I$(src)/src/dhd/sys/
EXTRA_CFLAGS += -I$(src)/src/dongle/
EXTRA_CFLAGS += -I$(src)/src/bcmsdio/sys/
EXTRA_CFLAGS += -I$(src)/src/wl/sys/
EXTRA_CFLAGS += -I$(src)/src/wl/bcmwifi/include /
EXTRA_CFLAGS += -I$(src)/src/wl/bcmwifi/src /
EXTRA_CFLAGS += -I$(src)/src/shared/

EXTRA_LDFLAGS += --strip-debug
KBUILD_CFLAGS += -I$(LINUXDIR)/include -I$(shell pwd)

#obj-m   += dhd.o

DHDOFILES = src/bcmsdio/sys/bcmsdh.o	src/bcmsdio/sys/bcmsdh_linux.o \
	 src/bcmsdio/sys/bcmsdh_sdmmc.o	src/bcmsdio/sys/bcmsdh_sdmmc_linux.o \
	 src/dhd/sys/dhd_bta.o		src/dhd/sys/dhd_cdc.o \
	 src/dhd/sys/dhd_common.o	src/dhd/sys/dhd_custom_gpio.o \
	 src/dhd/sys/dhd_custom_sec.o \
	 src/dhd/sys/dhd_linux.o	src/dhd/sys/dhd_linux_sched.o \
	 src/dhd/sys/dhd_cfg80211.o	src/dhd/sys/dhd_sdio.o \
	 src/shared/aiutils.o		src/shared/bcmevent.o \
	 src/shared/bcmutils.o		src/wl/bcmwifi/src/bcmwifi_channels.o \
	 src/shared/hndpmu.o		src/shared/linux_osl.o \
	 src/shared/sbutils.o		src/shared/siutils.o \
	 src/wl/sys/wl_android.o	src/wl/sys/wl_cfg80211.o \
	 src/wl/sys/wl_cfgp2p.o		src/wl/sys/wldev_common.o \
	 src/wl/sys/wl_linux_mon.o	src/wl/sys/wl_roam.o


obj-$(CONFIG_BCMDHD_4334) += bcmdhd.o
bcmdhd-objs += $(DHDOFILES)

all:
	@echo "$(MAKE) --no-print-directory -C $(KDIR) SUBDIRS=$(CURDIR) modules"
	@$(MAKE) --no-print-directory -C $(KDIR) \
		SUBDIRS=$(CURDIR) modules

clean:
	rm -rf *.o *.ko *.mod.c *~ .*.cmd \
	Module.symvers modules.order .tmp_versions modules.builtin \
	src/bcmsdio/sys/*.o \
	src/bcmsdio/sys/*.o.cmd \
	src/bcmsdio/sys/.*.o.cmd \
	src/dhd/sys/*.o \
	src/dhd/sys/*.o.cmd \
	src/dhd/sys/.*.o.cmd \
	src/shared/*.o \
	src/shared/*.o.cmd \
	src/shared/.*.o.cmd \
	src/wl/sys/*.o \
	src/wl/sys/*.o.cmd \
	src/wl/sys/.*.o.cmd \
	src/wl/bcmwifi/src/bcmwifi_channels.o \
	src/wl/bcmwifi/src/.*.o.cmd

install:
	@$(MAKE) --no-print-directory -C $(KDIR) \
		SUBDIRS=$(CURDIR) modules_install
