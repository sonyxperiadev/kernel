#Makefile for Encrypt and Decrypt sources
EXTRA_CFLAGS += -Werror

EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/modem/public/
EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/modem/public/sysinterface/util/public/
EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/fuse_log/
EXTRA_CFLAGS += -I$(srctree)/include/linux/broadcom/

EXTRA_CFLAGS += -DUNDER_LINUX
EXTRA_CFLAGS += -DUNDER_LINUX_MODEM
EXTRA_CFLAGS += -D_RHEA_

obj-$(CONFIG_BCM_MODEM) += \
          aes.o \
          brcm_hw_otp.o \
          crypto_api.o
