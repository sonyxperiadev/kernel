# /*******************************************************************************
# Copyright 2009 Broadcom Corporation.  All rights reserved.
# This program is the proprietary software of Broadcom Corporation and/or its
# licensors, and may only be used, duplicated, modified or distributed pursuant
# to the terms and conditions of a separate, written license agreement executed
# between you and Broadcom (an "Authorized License").
#
# Except as set forth in an Authorized License, Broadcom grants no license(express
# or implied), right to use, or waiver of any kind with respect to the Software,
# and Broadcom expressly reserves all rights in and to the Software and all
# intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN
# YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
# BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#  Except as expressly set forth in the Authorized License,
# 1. This program, including its structure, sequence and organization, constitutes
# the valuable trade secrets of Broadcom, and you shall use all reasonable efforts
# to protect the confidentiality thereof, and to use this information only in
# connection with your use of Broadcom integrated circuit products.
#
# 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" AND
# WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES,
# EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.
# BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE,
# MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF
# VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
# CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
# PERFORMANCE OF THE SOFTWARE.
#
# 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
# LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
# EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE
# OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY
# PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
# SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED
# REMEDY.
# *******************************************************************************/
#
# This is part of the linux build for the bcmtest and subcomponents
# Declare all variables and paths that vary by build environment
#
# *****************************************************************************
CHIP=bcm21553

BRCM_PUBLIC_DIR := $(BRCM_ROOTDIR)/public
BRCM_INCLUDES += -I $(BRCM_ROOTDIR)/public -I $(BRCM_ROOTDIR)/public/kernel
BRCM_INCLUDES += -I $(BRCM_PUBLIC_DIR)/soc/public

INCLUDE_XSCRIPT := false

# including these AT specific variables here because they are environment/platform variables
TOOL_PATH = $(BRCM_ROOTDIR)/at/atc/tool
AT_INTERMEDIATE_DIR := $(BRCM_ROOTDIR)/at/atc/tool

BRCM_CFLAGS := -W -Wall -Wno-unused -Winit-self -Wpointer-arith \
			-Werror=return-type -Werror=non-virtual-dtor -Werror=address \
			-Werror=sequence-point -g -Wstrict-aliasing=2 -finline-functions \
			-fno-inline-functions-called-once -fgcse-after-reload \
			-frerun-cse-after-loop -frename-registers -Os -fomit-frame-pointer \
			-fno-strict-aliasing -finline-limit=64 -D__LINENUM__=__LINE__ -MD
# gcc flags for building host files
BRCM_HOST_CFLAGS = $(BRCM_CFLAGS)
# gcc flags for building target files
BRCM_TARGET_CFLAGS := $(BRCM_CFLAGS)
BRCM_TARGET_CFLAGS += -march=armv5te -mtune=xscale -D__ARM_ARCH_5__ \
					-D__ARM_ARCH_5T__ -D__ARM_ARCH_5E__ -D__ARM_ARCH_5TE__ \
					-mthumb -mthumb-interwork -fmessage-length=0
BRCM_TARGET_CFLAGS += -DFUSE_APPS_MODEM -DHAL_INCLUDED -DSTACK_wedge
BRCM_TARGET_CFLAGS += -DBRCM_LINUX_ONLY_BUILD

CC := /projects/mob_tools/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-linux-gnueabi-gcc
CXX := /projects/mob_tools/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-linux-gnueabi-g++ 
AR := /projects/mob_tools/CodeSourcery/Sourcery_G++_Lite/arm-none-linux-gnueabi/bin/ar
