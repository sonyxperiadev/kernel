# BCMDHD

DHDCFLAGS = -Dlinux -DLINUX -DBCMDRIVER -DBCMDONGLEHOST -DDHDTHREAD
DHDCFLAGS += -DUNRELEASEDCHIP -DBCMDMA32 -DDHD_SCHED -DWLP2P
DHDCFLAGS += -DDHD_DEBUG -DBDC -DTOE -DDHD_BCMEVENTS -DSHOW_EVENTS
DHDCFLAGS += -DDHD_SDALIGN=64 -DMAX_HDR_READ=64 -DDHD_FIRSTREAD=64
DHDCFLAGS += -DBCMSDIO -DDHD_GPL -DBCMLXSDMMC -DBCMPLATFORM_BUS
DHDCFLAGS += -Wall -Wstrict-prototypes -Werror
#DHDCFLAGS += -DCUSTOMER_HW3 -DCUSTOM_OOB_GPIO_NUM=85
DHDCFLAGS += -DCUSTOMER_HW3 

# Selective F/W loading for 43362
DHDCFLAGS += -DDHD_SELECTIVE_FW_43362

# WAPI support (enable with nl80211 patch)
#DHDCFLAGS += -DBCMWAPI_WPI -DBCMWAPI_WAI

# In-Band Interrupt
#DHDCFLAGS += -DSDIO_ISR_THREAD -DMMC_SDIO_ABORT

# HW-OOB Interrupt
#DHDCFLAGS += -DOOB_INTR_ONLY -DHW_OOB -DMMC_SDIO_ABORT

# SW-OOB Interrupt
#DHDCFLAGS += -DOOB_INTR_ONLY -DMMC_SDIO_ABORT

ifeq ($(CONFIG_BCMDHD_43362_LINUX),m)
DHDCFLAGS += -DOOB_INTR_ONLY -DMMC_SDIO_ABORT -DCUSTOM_OOB_GPIO_NUM=85
endif

DHDCFLAGS += \
	-Idrivers/net/wireless/bcmdhd_lnx/dhd/sys \
	-Idrivers/net/wireless/bcmdhd_lnx/bcmsdio/sys \
	-Idrivers/net/wireless/bcmdhd_lnx/shared \
	-Idrivers/net/wireless/bcmdhd_lnx/dongle \
	-Idrivers/net/wireless/bcmdhd_lnx/wl/exe \
	-Idrivers/net/wireless/bcmdhd_lnx/wl/sys \
	-Idrivers/net/wireless/bcmdhd_lnx/include

#------------------------------------------------------------------------

DHDO_BCMSDIO_SYS = \
	bcmsdio/sys/bcmsdh.o bcmsdio/sys/bcmsdh_linux.o \
	bcmsdio/sys/bcmsdh_sdmmc.o bcmsdio/sys/bcmsdh_sdmmc_linux.o

DHDO_DHD_SYS = \
	dhd/sys/dhd_cdc.o dhd/sys/dhd_common.o \
	dhd/sys/dhd_custom_gpio.o dhd/sys/dhd_linux.o \
	dhd/sys/dhd_linux_mon.o dhd/sys/dhd_linux_sched.o \
	dhd/sys/dhd_sdio.o

DHDO_SHARED = \
	shared/aiutils.o shared/bcmevent.o \
	shared/bcmutils.o shared/bcmwifi.o \
	shared/hndpmu.o shared/linux_osl.o \
	shared/sbutils.o shared/siutils.o

DHDO_WL_SYS = \
	wl/sys/wl_cfg80211.o wl/sys/wl_cfgp2p.o \
	wl/sys/wldev_common.o

DHDOFILES = $(DHDO_BCMSDIO_SYS)
DHDOFILES += $(DHDO_DHD_SYS)
DHDOFILES += $(DHDO_SHARED)
DHDOFILES += $(DHDO_WL_SYS)

obj-$(CONFIG_BCMDHD_43362_LINUX) += bcmdhd.o
bcmdhd-objs += $(DHDOFILES)
EXTRA_CFLAGS = $(DHDCFLAGS)
EXTRA_LDFLAGS += --strip-debug
