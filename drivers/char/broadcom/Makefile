
EXTRA_CFLAGS := -fno-inline -Wno-multichar -O2

obj-y += v3d/
obj-y += vce/
obj-y += fuse_log/
obj-$(CONFIG_BCM_MODEM) += modem/
obj-y += isp/
obj-y += unicam/
obj-$(CONFIG_BCM_FRAME_PROFILER_SUPPORT)   += frame_profiler.o
obj-$(CONFIG_BCM_IDLE_PROFILER)		       += idle_profiler/
obj-$(CONFIG_GPIO_USER_MODE_SUPPORT)	    += user-gpio.o
obj-$(CONFIG_BCM_HALAUDIO_CORE)            += halaudio/
obj-$(CONFIG_BCM_HALAUDIO)                 += halaudio/ halaudio_drivers/
obj-$(CONFIG_BCM_AMXR)                     += amxr_drv.o
obj-$(CONFIG_BCM_AMXR_CORE)                += amxr_core/
obj-$(CONFIG_BCM_AAA)                      += aaa.o
obj-$(CONFIG_BCM_BSC)                      += bsc.o
obj-$(CONFIG_BCM_VCP)                      += vcp/
obj-$(CONFIG_BCM_HEADSET_SW)               += headset.o
obj-$(CONFIG_BCM_GIST)                     += gist.o
#obj-$(CONFIG_BCM_GIST)                    += gist_test.o
obj-$(CONFIG_BCM_HDMI_DET)                 += hdmi/
obj-$(CONFIG_BCM_WIFI_HDMI) 		          += wifi_hdmi/

obj-$(CONFIG_BCM_SIGMORPH)                 += sigmorph.o
#obj-$(CONFIG_BCM_SIGMORPH)                += sigmorph-test.o
obj-$(CONFIG_BCM_HDMI_DET)		+= hdmi/

bj-$(CONFIG_BCM_SIGMORPH)              += sigmorph.o
#obj-$(CONFIG_BCM_SIGMORPH)              += sigmorph-test.o

obj-$(CONFIG_BCM_CSX)                      += csx/
