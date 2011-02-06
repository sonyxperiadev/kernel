#Makefile for Encrypt and Decrypt sources

EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_ril/

obj-$(CONFIG_BRCM_FUSE_RIL_2153SDB) += \
          aes.o \
          brcm_hw_otp.o \
          crypto_api.o

obj-$(CONFIG_BRCM_FUSE_RIL_2157SDB) += \
          aes.o \
          brcm_hw_otp.o \
          crypto_api.o


obj-$(CONFIG_BRCM_FUSE_RIL_CIB) += \
          aes.o \
          brcm_hw_otp.o \
          crypto_api.o