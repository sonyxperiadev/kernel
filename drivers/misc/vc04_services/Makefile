
#EXTRA_CFLAGS := -fno-inline -Wno-multichar -O0

ifeq ($(wildcard $(srctree)/vc4-dev/src/interface/vcos),)

#
# Allow the kernel to build when no videocore files are present
#

$(info vc4-dev/src/interface/vcos NOT found - Skipping videocore modules)

else

#
# If the videocore files have been copied over, then we build them.
#

#obj-y   += $(CONFIG_BRCM_VIDEOCORE_ROOT)/services/gpio/kernel/
#obj-y   += $(CONFIG_BRCM_VIDEOCORE_ROOT)/services/vc_ctl/kernel/

obj-y   += vcos/
obj-y 	+= vc_mem/
obj-y 	+= vchiq/
obj-y   += vmcs_fb/
obj-y 	+= vmcs_sm/
obj-y 	+= vc_dnfo/

obj-y	+= ../../../vc4-dev/src/interface/vceb/host/linux/

endif


