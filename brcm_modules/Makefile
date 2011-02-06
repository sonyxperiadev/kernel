CUR_MODULES_DIR := ../$(BRCM_MODULES_DIR)

obj-$(CONFIG_HANTRO) += $(CUR_MODULES_DIR)/drivers/video/hantro/
obj-$(CONFIG_GE) += $(CUR_MODULES_DIR)/drivers/video/ge/

obj-$(CONFIG_BRCM_HAL_CAM)  += $(CUR_MODULES_DIR)/drivers/char/brcm/hal_cam/
obj-$(CONFIG_BRCM_HAL_CAM_WITH_STILL_YUV) += $(CUR_MODULES_DIR)/drivers/char/brcm/hal_cam/

obj-$(CONFIG_BRCM_FUSE_IPC_2153SDB) += $(CUR_MODULES_DIR)/drivers/char/brcm/fuse_ipc/
obj-$(CONFIG_BRCM_FUSE_IPC_2157SDB) += $(CUR_MODULES_DIR)/drivers/char/brcm/fuse_ipc/
obj-$(CONFIG_BRCM_FUSE_RPC_2157SDB) += $(CUR_MODULES_DIR)/drivers/char/brcm/fuse_rpc/
obj-$(CONFIG_BRCM_FUSE_RIL_2157SDB) += $(CUR_MODULES_DIR)/drivers/char/brcm/fuse_ril/
obj-$(CONFIG_BRCM_FUSE_IPC_CIB) += $(CUR_MODULES_DIR)/drivers/char/brcm/fuse_ipc/
obj-$(CONFIG_BRCM_FUSE_RPC_CIB) += $(CUR_MODULES_DIR)/drivers/char/brcm/fuse_rpc/
obj-$(CONFIG_BRCM_FUSE_RIL_CIB) += $(CUR_MODULES_DIR)/drivers/char/brcm/fuse_ril/
obj-$(CONFIG_BRCM_FUSE_NET) += $(CUR_MODULES_DIR)/drivers/char/brcm/fuse_net/
obj-$(CONFIG_BRCM_FUSE_AT) += $(CUR_MODULES_DIR)/drivers/char/brcm/fuse_at/


obj-$(CONFIG_BRCM_FUSE_RIL_2153SDB) += $(CUR_MODULES_DIR)/drivers/char/brcm/fuse_rpc/

obj-$(CONFIG_BRCM_FUSE_RIL_2153SDB) += $(CUR_MODULES_DIR)/drivers/char/brcm/fuse_ril/
obj-$(CONFIG_BRCM_FUSE_LOG) += $(CUR_MODULES_DIR)/drivers/char/brcm/fuse_log/
obj-$(CONFIG_BRCM_FUSE_SYSPARM) += $(CUR_MODULES_DIR)/drivers/char/brcm/fuse_sysparm/
obj-$(CONFIG_BRCM_FUSE_VSP) += $(CUR_MODULES_DIR)/drivers/char/brcm/fuse_vsp/
obj-$(CONFIG_BRCM_XSCRIPT) += $(CUR_MODULES_DIR)/drivers/char/brcm/xscript/
obj-$(CONFIG_BRCM_V3D) += $(CUR_MODULES_DIR)/drivers/char/brcm/v3d/
ifeq ($(CONFIG_ARCH_BCM2157), y)
obj-$(CONFIG_BRCM_SOUND_ALSA) += $(CUR_MODULES_DIR)/drivers/sound/brcm/alsa/
endif

ifeq ($(CONFIG_ARCH_BCM215XX), y)
obj-$(CONFIG_BRCM_SOUND_ATHENA_ALSA) += $(CUR_MODULES_DIR)/drivers/sound/brcm/alsa_athena/
endif
# Headset module
obj-$(CONFIG_BRCM_HEADSET) += $(CUR_MODULES_DIR)/drivers/sound/brcm/headset/
#cntin
obj-$(CONFIG_BRCM_CNTIN) += $(CUR_MODULES_DIR)/drivers/gps/brcm/

