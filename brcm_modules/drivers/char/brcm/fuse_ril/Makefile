#Makefile for KRIL & CAPI2
#EXTRA_CFLAGS := -Werror

ifdef CONFIG_BRCM_FUSE_RIL_2153SDB
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_ipc/ipc/Implementation/Headers/
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_ril/CAPI2/inc/
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_ril/crypto/
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_rpc/rpc/inc/
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_log/
else
ifdef CONFIG_BRCM_FUSE_RIL_2157SDB
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_ipc/ipc_2157/Implementation/Headers/
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_ril/CAPI2_2157/inc/
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_ril/crypto/
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_rpc/rpc_2157/inc/
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_log/
else
ifdef CONFIG_BRCM_FUSE_RIL_CIB

ifdef CONFIG_MFD_MAX8986
EXTRA_CFLAGS += -DPMU_MAX8986
else
EXTRA_CFLAGS += -DPMU_BCM59038
endif

# common definitons for all capi2 related include paths
include $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_ril/CAPI2_CIB/capi2_include_paths.gmk

EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_log/
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_ipc/ipc_CIB/public/
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_rpc/rpc_CIB/public/
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_rpc/rpc_CIB/arpc/inc/
#EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_rpc/rpc_CIB/srpc/inc/
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_ril/crypto/

#**FIXME** MAG - used to prevent redefinition of U_CHAR in various CIB header files
EXTRA_CFLAGS += -D_BSDTYPES_DEFINED -DTYPEDEF_U_CHAR -DTYPEDEF_U_SHORT
EXTRA_CFLAGS += -DFUSE_APPS_PROCESSOR -DFUSE_DUAL_PROCESSOR_ARCHITECTURE
endif
endif
endif

# video telephony support
ifdef CONFIG_VIDEO_TELEPHONY_ENABLE
EXTRA_CFLAGS += -DVIDEO_TELEPHONY_ENABLE
endif

EXTRA_CFLAGS += -DRPC_INCLUDED
EXTRA_CFLAGS += -DUNDER_LINUX
EXTRA_CFLAGS += -D_BCM2153_
EXTRA_CFLAGS += -DUMTS


obj-$(CONFIG_BRCM_FUSE_RIL_2153SDB) += \
	bcm_kril_main.o \
	bcm_kril_ioctl.o \
	bcm_kril_cmd_handler.o \
	bcm_kril_capi2_handler.o \
	bcm_kril_notify_handler.o \
	bcm_kril_callctrl_handler.o \
	bcm_kril_suppsvc_handler.o \
	bcm_kril_msg_handler.o \
	bcm_kril_network_handler.o \
	bcm_kril_misc_handler.o \
	bcm_kril_sim_handler.o \
	bcm_kril_stk_handler.o \
	bcm_kril_pdp_handler.o \
	bcm_kril_simlockfun.o \
	bcm_cp_cmd_handler.o \
	CAPI2/ \
	crypto/

obj-$(CONFIG_BRCM_FUSE_RIL_2157SDB) += \
	bcm_kril_main.o \
	bcm_kril_ioctl.o \
	bcm_kril_cmd_handler.o \
	bcm_kril_capi2_handler.o \
	bcm_kril_notify_handler.o \
	bcm_kril_callctrl_handler.o \
	bcm_kril_suppsvc_handler.o \
	bcm_kril_msg_handler.o \
	bcm_kril_network_handler.o \
	bcm_kril_misc_handler.o \
	bcm_kril_sim_handler.o \
	bcm_kril_stk_handler.o \
	bcm_kril_pdp_handler.o \
	bcm_kril_simlockfun.o \
	bcm_cp_cmd_handler.o \
	CAPI2_2157/ \
	crypto/

obj-$(CONFIG_BRCM_FUSE_RIL_CIB) += \
	bcm_kril_main.o \
	bcm_kril_ioctl.o \
	bcm_kril_cmd_handler.o \
	bcm_kril_capi2_handler.o \
	bcm_kril_notify_handler.o \
	bcm_kril_callctrl_handler.o \
	bcm_kril_suppsvc_handler.o \
	bcm_kril_msg_handler.o \
	bcm_kril_network_handler.o \
	bcm_kril_misc_handler.o \
	bcm_kril_sim_handler.o \
	bcm_kril_stk_handler.o \
	bcm_kril_pdp_handler.o \
    bcm_kril_simlockfun.o \
    bcm_kril_capirpc_handler.o \
    bcm_kril_sysrpc_handler.o \
    bcm_kril_atrpc_handler.o \
	bcm_cp_cmd_handler.o \
	bcm_kril_adcrpc_handler.o \
	CAPI2_CIB/ \
	crypto/

