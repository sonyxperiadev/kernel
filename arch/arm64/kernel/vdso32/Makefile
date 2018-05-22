#
# Building a vDSO image for AArch32.
#
# Author: Kevin Brodsky <kevin.brodsky@arm.com>
# A mix between the arm64 and arm vDSO Makefiles.

ifeq ($(cc-name),clang)
  CC_ARM32 := $(CC) $(CLANG_TARGET_ARM32) -no-integrated-as
else
  CC_ARM32 := $(CROSS_COMPILE_ARM32)$(cc-name)
endif

# Same as cc-*option, but using CC_ARM32 instead of CC
cc32-option = $(call try-run,\
        $(CC_ARM32) $(1) -c -x c /dev/null -o "$$TMP",$(1),$(2))
cc32-disable-warning = $(call try-run,\
	$(CC_ARM32) -W$(strip $(1)) -c -x c /dev/null -o "$$TMP",-Wno-$(strip $(1)))
cc32-ldoption = $(call try-run,\
        $(CC_ARM32) $(1) -nostdlib -x c /dev/null -o "$$TMP",$(1),$(2))

# We cannot use the global flags to compile the vDSO files, the main reason
# being that the 32-bit compiler may be older than the main (64-bit) compiler
# and therefore may not understand flags set using $(cc-option ...). Besides,
# arch-specific options should be taken from the arm Makefile instead of the
# arm64 one.
# As a result we set our own flags here.

# From top-level Makefile
# NOSTDINC_FLAGS
VDSO_CPPFLAGS := -nostdinc -isystem $(shell $(CC_ARM32) -print-file-name=include)
VDSO_CPPFLAGS += $(LINUXINCLUDE)
VDSO_CPPFLAGS += $(KBUILD_CPPFLAGS)

# Common C and assembly flags
# From top-level Makefile
VDSO_CAFLAGS := $(VDSO_CPPFLAGS)
VDSO_CAFLAGS += $(call cc32-option,-fno-PIE)
ifdef CONFIG_DEBUG_INFO
VDSO_CAFLAGS += -g
endif
ifeq ($(shell $(CONFIG_SHELL) $(srctree)/scripts/gcc-goto.sh $(CC_ARM32)), y)
VDSO_CAFLAGS += -DCC_HAVE_ASM_GOTO
endif

# From arm Makefile
VDSO_CAFLAGS += $(call cc32-option,-fno-dwarf2-cfi-asm)
VDSO_CAFLAGS += -mabi=aapcs-linux -mfloat-abi=soft
ifeq ($(CONFIG_CPU_BIG_ENDIAN), y)
VDSO_CAFLAGS += -mbig-endian
else
VDSO_CAFLAGS += -mlittle-endian
endif

# From arm vDSO Makefile
VDSO_CAFLAGS += -fPIC -fno-builtin -fno-stack-protector
VDSO_CAFLAGS += -DDISABLE_BRANCH_PROFILING

# Try to compile for ARMv8. If the compiler is too old and doesn't support it,
# fall back to v7. There is no easy way to check for what architecture the code
# is being compiled, so define a macro specifying that (see arch/arm/Makefile).
VDSO_CAFLAGS += $(call cc32-option,-march=armv8-a -D__LINUX_ARM_ARCH__=8,\
                                   -march=armv7-a -D__LINUX_ARM_ARCH__=7)

VDSO_CFLAGS := $(VDSO_CAFLAGS)
# KBUILD_CFLAGS from top-level Makefile
VDSO_CFLAGS += -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs \
               -fno-strict-aliasing -fno-common \
               -Werror-implicit-function-declaration \
               -Wno-format-security \
               -std=gnu89
VDSO_CFLAGS  += -O2
# Some useful compiler-dependent flags from top-level Makefile
VDSO_CFLAGS += $(call cc32-option,-Wdeclaration-after-statement,)
VDSO_CFLAGS += $(call cc32-option,-Wno-pointer-sign)
VDSO_CFLAGS += $(call cc32-option,-fno-strict-overflow)
VDSO_CFLAGS += $(call cc32-option,-Werror=strict-prototypes)
VDSO_CFLAGS += $(call cc32-option,-Werror=date-time)
VDSO_CFLAGS += $(call cc32-option,-Werror=incompatible-pointer-types)

# The 32-bit compiler does not provide 128-bit integers, which are used in
# some headers that are indirectly included from the vDSO code.
# This hack makes the compiler happy and should trigger a warning/error if
# variables of such type are referenced.
VDSO_CFLAGS += -D__uint128_t='void*'
# Silence some warnings coming from headers that operate on long's
# (on GCC 4.8 or older, there is unfortunately no way to silence this warning)
VDSO_CFLAGS += $(call cc32-disable-warning,shift-count-overflow)
VDSO_CFLAGS += -Wno-int-to-pointer-cast

VDSO_AFLAGS := $(VDSO_CAFLAGS)
VDSO_AFLAGS += -D__ASSEMBLY__

VDSO_LDFLAGS := $(VDSO_CPPFLAGS)
# From arm vDSO Makefile
VDSO_LDFLAGS += -Wl,-Bsymbolic -Wl,--no-undefined -Wl,-soname=linux-vdso.so.1
VDSO_LDFLAGS += -Wl,-z,max-page-size=4096 -Wl,-z,common-page-size=4096
VDSO_LDFLAGS += -nostdlib -shared -mfloat-abi=soft
VDSO_LDFLAGS += $(call cc32-ldoption,-Wl$(comma)--hash-style=sysv)
VDSO_LDFLAGS += $(call cc32-ldoption,-Wl$(comma)--build-id)
VDSO_LDFLAGS += $(call cc32-ldoption,-fuse-ld=bfd)


# Borrow vdsomunge.c from the arm vDSO
# We have to use a relative path because scripts/Makefile.host prefixes
# $(hostprogs-y) with $(obj)
munge := ../../../arm/vdso/vdsomunge
hostprogs-y := $(munge)

c-obj-vdso := vgettimeofday.o
asm-obj-vdso := sigreturn.o

# Build rules
targets := $(c-obj-vdso) $(asm-obj-vdso) vdso.so vdso.so.dbg vdso.so.raw
c-obj-vdso := $(addprefix $(obj)/, $(c-obj-vdso))
asm-obj-vdso := $(addprefix $(obj)/, $(asm-obj-vdso))
obj-vdso := $(c-obj-vdso) $(asm-obj-vdso)

obj-y += vdso.o
extra-y += vdso.lds
CPPFLAGS_vdso.lds += -P -C -U$(ARCH)

# Force dependency (vdso.s includes vdso.so through incbin)
$(obj)/vdso.o: $(obj)/vdso.so

include/generated/vdso32-offsets.h: $(obj)/vdso.so.dbg FORCE
	$(call if_changed,vdsosym)

# Strip rule for vdso.so
$(obj)/vdso.so: OBJCOPYFLAGS := -S
$(obj)/vdso.so: $(obj)/vdso.so.dbg FORCE
	$(call if_changed,objcopy)

$(obj)/vdso.so.dbg: $(obj)/vdso.so.raw $(obj)/$(munge) FORCE
	$(call if_changed,vdsomunge)

# Link rule for the .so file, .lds has to be first
$(obj)/vdso.so.raw: $(src)/vdso.lds $(obj-vdso) FORCE
	$(call if_changed,vdsold)

# Compilation rules for the vDSO sources
$(filter-out vgettimeofday.o, $(c-obj-vdso)): %.o: %.c FORCE
	$(call if_changed_dep,vdsocc)
$(asm-obj-vdso): %.o: %.S FORCE
	$(call if_changed_dep,vdsoas)

# Actual build commands
quiet_cmd_vdsold = VDSOL32   $@
      cmd_vdsold = $(CC_ARM32) -Wp,-MD,$(depfile) $(VDSO_LDFLAGS) \
                   -Wl,-T $(filter %.lds,$^) $(filter %.o,$^) -o $@
quiet_cmd_vdsocc = VDSOC32   $@
      cmd_vdsocc = $(CC_ARM32) -Wp,-MD,$(depfile) $(VDSO_CFLAGS) -c -o $@ $<
quiet_cmd_vdsoas = VDSOA32   $@
      cmd_vdsoas = $(CC_ARM32) -Wp,-MD,$(depfile) $(VDSO_AFLAGS) -c -o $@ $<

quiet_cmd_vdsomunge = MUNGE   $@
      cmd_vdsomunge = $(obj)/$(munge) $< $@

# Generate vDSO offsets using helper script (borrowed from the 64-bit vDSO)
gen-vdsosym := $(srctree)/$(src)/../vdso/gen_vdso_offsets.sh
quiet_cmd_vdsosym = VDSOSYM $@
# The AArch64 nm should be able to read an AArch32 binary
      cmd_vdsosym = $(NM) $< | $(gen-vdsosym) | LC_ALL=C sort > $@

# Install commands for the unstripped file
quiet_cmd_vdso_install = INSTALL $@
      cmd_vdso_install = cp $(obj)/$@.dbg $(MODLIB)/vdso/vdso32.so

vdso.so: $(obj)/vdso.so.dbg
	@mkdir -p $(MODLIB)/vdso
	$(call cmd,vdso_install)

vdso_install: vdso.so
