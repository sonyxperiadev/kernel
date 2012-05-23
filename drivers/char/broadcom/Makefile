
EXTRA_CFLAGS := -fno-inline -Wno-multichar -O2

obj-y += v3d/
obj-y += vce/
obj-y += fuse_log/
obj-$(CONFIG_BCM_MODEM) += modem/
obj-y += isp/
obj-y += unicam/
obj-$(CONFIG_GPS_IRQ)		 += brcm_gps_irq.o
obj-$(CONFIG_BCM_FRAME_PROFILER_SUPPORT)   += frame_profiler.o
obj-$(CONFIG_BCM_IDLE_PROFILER)		       += idle_profiler/
obj-$(CONFIG_BCM_BSC)                      += bsc.o
obj-$(CONFIG_BCM_HEADSET_SW)               += headset.o

ifdef CONFIG_RHEA_PANDA
obj-$(CONFIG_BCM_MDMGPIO)              += mdmgpio/

endif

obj-$(CONFIG_BCM_RTC_CAL)           += rtc_cal/
