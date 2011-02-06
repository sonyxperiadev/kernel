#Makefile for IPC sources
#EXTRA_CFLAGS := -Werror

EXTRA_CFLAGS += -I ../modules/drivers/char/brcm/fuse_ril/CAPI2/inc/
EXTRA_CFLAGS += -I ../modules/drivers/char/brcm/fuse_rpc/rpc/inc/
EXTRA_CFLAGS += -Ifs/cifs/

EXTRA_CFLAGS += -DUNDER_LINUX
EXTRA_CFLAGS += -DRPC_INCLUDED
EXTRA_CFLAGS += -DSTACK_wedge
EXTRA_CFLAGS += -D_BCM2153_
EXTRA_CFLAGS += -DUMTS

obj-$(CONFIG_BRCM_FUSE_RIL_2153SDB) += \
          md5.o \
          capi2_phonebk_msg.o \
          capi2_sms_msg.o \
          capi2_sim_msg.o \
          capi2_stk_msg.o \
          capi2_reqrep.o  \
          capi2_cc_msg.o  \
          capi2_ss_msg.o  \
          capi2_sim_msg.o \
          capi2_isim_msg.o \
          capi2_pch_msg.o \
          capi2_phonectrl_msg.o \
          capi2_lcs_msg.o \
          capi2_lcs_cplane_msg.o \
          capi2_gen_msg.o \
          capi2_ds_msg.o \
          capi2_sys_common.o \
          capi2_dispatch.o \
          capi2_cp_msg.o \
          capi2_gen_switch.o \
          capi2_phonebk_api.o \
          capi2_sms_api.o \
          capi2_stk_api.o \
          capi2_app.o \
          capi2_cc_api.o \
          capi2_ss_api.o \
          capi2_pch_api.o \
          capi2_sim_api.o \
          capi2_isim_api.o \
          capi2_phonectrl_api.o \
          capi2_lcs_api.o \
          capi2_lcs_cplane_api.o \
          capi2_gen_api.o \
          capi2_ipconfig.o \
          capi2_gen_apps_switch.o
