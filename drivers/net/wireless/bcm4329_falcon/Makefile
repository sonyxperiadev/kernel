# BCM4329

DHDCFLAGS = -DBCMWPA2 -DSDTEST -DDONGLEOVERLAYS \
	    -DLINUX -DBCMDRIVER -DBCMDONGLEHOST -DUNRELEASEDCHIP -DBCMDMA32 \
	    -DEMBEDDED_PLATFORM -DARP_OFFLOAD_SUPPORT -DPKT_FILTER_SUPPORT \
	    -DSOFTAP -DCSCAN -DBCMFILEIMAGE -DCONFIG_WIRELESS_EXT -DDHDTHREAD \
	    -DDHD_GPL -DDHD_SCHED -DBDC -DTOE -DDHD_BCMEVENTS \
	    -DSHOW_EVENTS -DBCMSDIO -DBCMLXSDMMC -DBCMPLATFORM_BUS \
	    -DSDIO_ISR_THREAD -DWIFI_ACT_FRAME -DDEFAULT_HW \
	    -Wall -Wstrict-prototypes -Werror \
	    -DWLPFN -DWLCNT -DWLBTAMP -DRWL_SOCKET -DRWL_WIFI -DRWL_DONGLE \
	    -Idrivers/net/wireless/bcm4329_falcon/dhd/sys \
	    -Idrivers/net/wireless/bcm4329_falcon/dongle \
	    -Idrivers/net/wireless/bcm4329_falcon/wl/exe \
	    -Idrivers/net/wireless/bcm4329_falcon/wl/sys \
	    -Idrivers/net/wireless/bcm4329_falcon/include

DHDO_BCMSDIO_SYS = bcmsdio/sys/bcmsdh.o bcmsdio/sys/bcmsdh_linux.o \
                   bcmsdio/sys/bcmsdh_sdmmc.o bcmsdio/sys/bcmsdh_sdmmc_linux.o 

DHDO_DHD_SYS = dhd/sys/dhd_bta.o dhd/sys/dhd_cdc.o \
               dhd/sys/dhd_common.o dhd/sys/dhd_custom_gpio.o \
               dhd/sys/dhd_linux.o dhd/sys/dhd_linux_sched.o \
               dhd/sys/dhd_sdio.o

DHDO_SHARED = shared/aiutils.o shared/bcmevent.o shared/bcmutils.o shared/bcmwifi.o \
              shared/hndpmu.o shared/linux_osl.o shared/sbutils.o shared/siutils.o

DHDO_WL_SYS = wl/sys/wl_iw.o

DHDOFILES  = $(DHDO_BCMSDIO_SYS)
DHDOFILES += $(DHDO_DHD_SYS)
DHDOFILES += $(DHDO_SHARED)
DHDOFILES += $(DHDO_WL_SYS)

obj-$(CONFIG_BCM4329_FALCON) += bcm4329_falcon.o
bcm4329_falcon-objs += $(DHDOFILES)
EXTRA_CFLAGS = $(DHDCFLAGS)
EXTRA_LDFLAGS += --strip-debug
