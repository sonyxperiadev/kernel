
EXTRA_CFLAGS := -fno-inline -Wno-multichar -O0

ifneq ($(CONFIG_BRCM_VIDEOCORE_ROOT),)

obj-y	+= $(CONFIG_BRCM_VIDEOCORE_ROOT)/services/gencmd/kernel/
obj-y	+= $(CONFIG_BRCM_VIDEOCORE_ROOT)/services/dispmanx/kernel/
obj-y	+= $(CONFIG_BRCM_VIDEOCORE_ROOT)/services/ilcs/kernel/
obj-y	+= $(CONFIG_BRCM_VIDEOCORE_ROOT)/services/hostreq/kernel/
obj-y	+= $(CONFIG_BRCM_VIDEOCORE_ROOT)/services/khronos/kernel/
obj-y	+= $(CONFIG_BRCM_VIDEOCORE_ROOT)/services/filesys/kernel/filesrv/
obj-y	+= $(CONFIG_BRCM_VIDEOCORE_ROOT)/services/filesys/kernel/filecli/
obj-y	+= $(CONFIG_BRCM_VIDEOCORE_ROOT)/services/vchiq/kernel/
obj-y	+= $(CONFIG_BRCM_VIDEOCORE_ROOT)/services/vmcs_fb/kernel_vceb/
obj-y	+= $(CONFIG_BRCM_VIDEOCORE_ROOT)/services/vc_ctl/kernel/
obj-y += $(CONFIG_BRCM_VIDEOCORE_ROOT)/services/vcos/kernel/

ifneq ($(CONFIG_BRCM_VIDEOCORE_VCEB),)
   ifeq ($(strip $(CONFIG_BRCM_VIDEOCORE_SANITIZED)),y)
      obj-y	+= $(CONFIG_BRCM_VIDEOCORE_ROOT)/vmcs/$(CONFIG_BRCM_VIDEOCORE_VERSION)/src_sanitized/interface/vceb/host/linux/
   else
      obj-y += $(CONFIG_BRCM_VIDEOCORE_ROOT)/vmcs/$(CONFIG_BRCM_VIDEOCORE_VERSION)/src/interface/vceb/host/linux/
   endif
endif

endif
