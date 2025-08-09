This file contains instructions on how to build Cirrus Logic
Linux kernel drivers as modules for Android BSP.

Building for Android AOSP DragonBoard 845c reference board
==========================================================

Follow instructions from https://source.android.com/setup/build/devices
to setup default DragonBoard 845c build.

1. Clone this git repository to vendor/cirrus/kernel-modules folder in
   the kernel repo (repo-common)

2. Select driver to build
      cd  vendor/cirrus/kernel-modules

      # cs35l41 AMP
      git checkout -b  modules-5.10  origin/v5.10-cs35l41-modules

      -or-

      # cs40l25 Haptic driver
      git checkout -b  modules-5.10  origin/v5.10-cs40l25-modules

3. (Optional) Merge other driver(s) if needed
      git merge origin/v5.10-cs40l25-modules

4. Select kernel branch aosp/android12-5.10
      cd common
      git checkout -b android12-5.10 aosp/android12-5.10

5. Add EXT_MODULES to the build config
      Open common/build.config.db845c.cirrus

      Add:
         TOP="${ROOT_DIR}"

      Add for cs35l41 module:
         EXT_MODULES="vendor/cirrus/kernel-modules/sound/soc/codecs

      Add for cs40l25 module:
         EXT_MODULES="vendor/cirrus/kernel-modules/drivers/misc
                      vendor/cirrus/kernel-modules/sound/soc/codecs
                      "
6. Build kernel
      rm -rf out
      BUILD_CONFIG=common/build.config.db845c.cirrus ./build/build.sh


Building Cirrus drivers as Loadable Kernel Modules for Android
==============================================================

Google instructions for DLKM build are located here:
https://source.android.com/devices/architecture/kernel/loadable-kernel-modules

To build the drivers:

1. Clone this git to vendor/cirrus/kernel-modules folder in
      Android BSP repo

2. Select driver to build
      cd  vendor/cirrus/kernel-modules

      # cs35l41 AMP driver
      git checkout -b  modules-5.10  origin/v5.10-cs35l41-modules

      - or -

      # cs40l25 Haptic driver
      git checkout -b  modules-5.10  origin/v5.10-cs40l25-modules

3. Merge other driver if needed.
      git merge origin/v5.10-cs40l25-modules

4. In product's BoardConfig.mk, add Cirrus modules to BOARD_VENDOR_KERNEL_MODULES
      # For cs35l41 AMP driver
      BOARD_VENDOR_KERNEL_MODULES := \
               $(KERNEL_MODULES_OUT)/cirrus_wm_adsp.ko \
               $(KERNEL_MODULES_OUT)/cirrus_cs35l41.ko

      # For cs40l25 haptics driver
      BOARD_VENDOR_KERNEL_MODULES := \
               $(KERNEL_MODULES_OUT)/cirrus_cs40l2x.ko \
               $(KERNEL_MODULES_OUT)/cirrus_cs40l2x_codec.ko

5. Build Android
