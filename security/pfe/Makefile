#
# Makefile for the MSM specific security device drivers.
#

ccflags-y += -Isecurity/selinux -Isecurity/selinux/include -Ifs/ecryptfs
ccflags-y += -Ifs/ext4

obj-$(CONFIG_PFT) += pft.o
obj-$(CONFIG_PFK) += pfk.o pfk_kc.o pfk_ice.o pfk_ext4.o pfk_ecryptfs.o

# TODO: remove me b/62057517
subdir-ccflags-y += \
	-Wno-ignored-attributes
