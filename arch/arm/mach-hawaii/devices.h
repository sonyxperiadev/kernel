#ifndef __MACH_HAWAII_DEVICE_H
#define __MACH_HAWAII_DEVICE_H

void __init board_add_common_devices(void);
void __init board_add_sdio_devices(void);
void __init board_common_reserve(void);
/*
 * todo: 8250 driver has problem autodetecting the UART type -> have to
 * use FIXED type
 * confuses it as an XSCALE UART.  Problem seems to be that it reads
 * bit6 in IER as non-zero sometimes when it's supposed to be 0.
 */
#define KONA_UART0_PA	UARTB_BASE_ADDR
#define KONA_UART1_PA	UARTB2_BASE_ADDR
#define KONA_UART2_PA	UARTB3_BASE_ADDR

#define KONA_8250PORT(name, clk)				\
{								\
	.membase    = (void __iomem *)(KONA_##name##_VA),	\
	.mapbase    = (resource_size_t)(KONA_##name##_PA),	\
	.irq	    = BCM_INT_ID_##name,			\
	.uartclk    = 26000000,					\
	.regshift   = 2,				\
	.iotype	    = UPIO_DWAPB,			\
	.type	    = PORT_16550A,			\
	.flags	    = UPF_BOOT_AUTOCONF | UPF_FIXED_TYPE | UPF_SKIP_TEST | \
						UPF_LOW_LATENCY, \
	.private_data = (void __iomem *)((KONA_##name##_VA) + \
						UARTB_USR_OFFSET), \
	.clk_name = clk,	\
}

#ifdef CONFIG_KONA_AVS
extern u32 svt_pmos_bin[3 + 1];
extern u32 svt_nmos_bin[3 + 1];
extern u32 lvt_pmos_bin[3 + 1];
extern u32 lvt_nmos_bin[3 + 1];
extern u32 svt_silicon_type_lut[3 * 3];
extern u32 lvt_silicon_type_lut[3 * 3];
extern void avs_silicon_type_notify(u32 silicon_type);
#endif
extern struct platform_device board_serial_device;
extern struct platform_device board_i2c_adap_devices[];
extern struct platform_device pmu_device;
extern struct platform_device kona_pwm_device;
extern struct platform_device kona_sspi_spi0_device;
extern struct platform_device kona_sspi_spi2_device;
#ifdef CONFIG_SENSORS_KONA
extern struct platform_device tmon_device;
extern struct platform_device thermal_device;
#endif
#ifdef CONFIG_STM_TRACE
extern struct platform_device kona_stm_device;
#endif
#ifdef CONFIG_HW_RANDOM_KONA
extern struct platform_device rng_device;
#endif
#ifdef CONFIG_USB_DWC_OTG
extern struct platform_device board_kona_hsotgctrl_platform_device;
extern struct platform_device board_kona_otg_platform_device;
#endif
#ifdef CONFIG_KONA_AVS
extern struct platform_device kona_avs_device;
#endif
#ifdef CONFIG_KONA_CPU_FREQ_DRV
extern struct platform_device kona_cpufreq_device;
#endif
#ifdef CONFIG_CRYPTO_DEV_BRCM_SPUM_HASH
extern struct platform_device board_spum_device;
#endif
#ifdef CONFIG_CRYPTO_DEV_BRCM_SPUM_AES
extern struct platform_device board_spum_aes_device;
#endif
#ifdef CONFIG_UNICAM
extern struct platform_device board_unicam_device;
#endif
#ifdef CONFIG_VIDEO_UNICAM_CAMERA
extern struct platform_device unicam_camera_device;
#endif
#ifdef CONFIG_SND_BCM_SOC
extern struct platform_device caph_i2s_device;
extern struct platform_device caph_pcm_device;
#endif
extern struct platform_device android_pmem;

#endif /* __MACH_HAWAII_DEVICE_H */
