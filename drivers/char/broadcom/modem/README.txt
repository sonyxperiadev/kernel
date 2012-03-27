The modem subdirectory builds the drivers/modules that interface to and control the COMMS 
processor.

The subdirectories of modem are:

       ipc           - interprocessor communications
       rpc           - remote procedure call support
       sysrpc        - rpc callback handler (CP->AP)
       net           - network support

The files in these directories were copied from the 'gb_brcm' branch and modified for the Rhea 
platform.  gb_brcm branch was synced to commit ff75a35f6d10e2311b5b4dcce78a5b89dcc967ca. 

In addition, files in kernel/include/linux/broadcom were updated based on gb_brcm:

       ipcinterface.h
       ipcproperties.h

kernel/arch/arm/configs/rhea_ray_defconfig was modified to include Broadcom char drivers
in the kernel build by default. 

