/*
 *  linux/arch/arm/mach-bcm2708/bcm2708.c
 *
 */

#include <linux/init.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/serial_8250.h>
#include <linux/platform_device.h>
#include <linux/sysdev.h>
#include <linux/interrupt.h>
#include <linux/amba/bus.h>
#include <linux/amba/clcd.h>
#include <linux/clocksource.h>
#include <linux/clockchips.h>
#include <linux/cnt32_to_63.h>
#include <linux/io.h>
#include <mtd/mtd-abi.h>

#include <asm/clkdev.h>
#include <asm/system.h>
#include <mach/hardware.h>
#include <asm/irq.h>
#include <asm/leds.h>
#include <asm/mach-types.h>

#include <asm/mach/arch.h>
#include <asm/mach/flash.h>
#include <asm/mach/irq.h>
#include <asm/mach/time.h>
#include <asm/mach/map.h>
//#include <asm/mach/mmc.h>
#include <mach/ipc.h>
#include <mach/plat_nand.h>

#include "bcm2708.h"
#include "armctrl.h"
#include "clock.h"

// Effectively we have an IOMMU (Gert's ARM<->VideoCore map) that is set up to
// give us IO access only to 64Mbytes of physical memory (26 bits).  We could
// represent this window by setting our dmamasks to 26 bits but, in fact we're
// not going to use addresses outside this range (they're not in real memory)
// so we don't bother.
// In the future we might include code to use this IOMMU to remap other
// physical addresses onto VideoCore memory then the use of 32-bits would be
// more legitimate.
#define DMA_MASK_BITS_COMMON 32

void __init bcm2708_init_irq(void)
{
	armctrl_init(__io_address(ARMCTRL_IC_BASE), 0, 0, 0);
}

static struct map_desc bcm2708_io_desc[] __initdata = {
        {
                .virtual        = IO_ADDRESS(MCORESYNC_BASE),
                .pfn            = __phys_to_pfn(MCORESYNC_BASE),
                .length         = SZ_4K,
                .type           = MT_DEVICE
        }, {
		.virtual	= IO_ADDRESS(ARMCTRL_BASE),
		.pfn		= __phys_to_pfn(ARMCTRL_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
 	}, {
		.virtual	= IO_ADDRESS(UART0_BASE),
		.pfn		= __phys_to_pfn(UART0_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
 	}, {
		.virtual	= IO_ADDRESS(UART1_BASE),
		.pfn		= __phys_to_pfn(UART1_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual        = IO_ADDRESS(MMCI0_BASE),
		.pfn            = __phys_to_pfn(MMCI0_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual        = IO_ADDRESS(DMA_BASE),
		.pfn            = __phys_to_pfn(DMA_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual        = IO_ADDRESS(MCORE_BASE),
		.pfn            = __phys_to_pfn(MCORE_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual        = IO_ADDRESS(ST_BASE),
		.pfn            = __phys_to_pfn(ST_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
 	}, {
		.virtual	= IO_ADDRESS(USB_BASE),
		.pfn		= __phys_to_pfn(USB_BASE),
		.length		= SZ_128K,
		.type		= MT_DEVICE
	},
	{
		.virtual	= IO_ADDRESS(PM_BASE),
		.pfn		= __phys_to_pfn(PM_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}
};

#if 0
void debug_out(unsigned int value)
{
   while (readl(__io_address(ARM_0_MAIL1_STA)) & (1<<31))
      continue;
   writel(value, __io_address(ARM_0_MAIL1_WRT));
}
#endif

void __init bcm2708_map_io(void)
{
	iotable_init(bcm2708_io_desc, ARRAY_SIZE(bcm2708_io_desc));
}

// This routine is used in preference to the weak reference defined in
// kernel/sched_clock.c
// It is supposed to return the current time in nanosecond units and is used
// in generating the kernel's 'jiffies' clock count.
unsigned long long sched_clock(void)
{
        // The STC is a free running counter that increments at the rate of 1MHz
        unsigned long t = readl(__io_address(ST_BASE+0x04)) * 1000;
        // For cnt32_to_63 to work correctly we MUST call this routine at least
        // once every half-32-bit-wraparound period - that's once every 35minutes
        // or so
	return cnt32_to_63(t);
}

/*
 * These are fixed clocks.
 */
static struct clk ref24_clk = {
        .rate	= 3000000,  // The UART is clocked at 3MHz via APB_CLK
};
static struct clk osc_clk = {
#ifdef CONFIG_ARCH_BCM2708_CHIPIT
        .rate	= 27000000,
#else
        .rate	= 500000000,  // the ARM clock is set from the VideoCore booter
#endif
};
// warning - the USB needs a clock > 34MHz
static struct clk sdhost_clk = {
#ifdef CONFIG_ARCH_BCM2708_CHIPIT
   .rate	=   4000000, // 4MHz
#else
   .rate	= 250000000, // 250MHz
#endif
};

static struct clk_lookup lookups[] = {
	{	/* UART0 */
		.dev_id		= "dev:f1",
		.clk		= &ref24_clk,
	},
	{	/* USB */
		.dev_id		= "bcm2708_usb",
		.clk		= &osc_clk,
	},
	{	/* MCI */
		.dev_id		= "bcm2708_mci.0",
		.clk		= &sdhost_clk,
	}
};


#define UART0_IRQ	{ IRQ_UART, NO_IRQ }
#define UART0_DMA	{ 15, 14 }

AMBA_DEVICE(uart0, "dev:f1",  UART0,    NULL);

static struct amba_device *amba_devs[] __initdata = {
	&uart0_device,
};

#ifdef CONFIG_LEDS
#define VA_LEDS_BASE (__io_address(VERSATILE_SYS_BASE) + VERSATILE_SYS_LED_OFFSET)

static void bcm2708_leds_event(led_event_t ledevt)
{
	unsigned long flags;
	u32 val;

	local_irq_save(flags);
	val = readl(VA_LEDS_BASE);

	switch (ledevt) {
	case led_idle_start:
		val = val & ~VERSATILE_SYS_LED0;
		break;

	case led_idle_end:
		val = val | VERSATILE_SYS_LED0;
		break;

	case led_timer:
		val = val ^ VERSATILE_SYS_LED1;
		break;

	case led_halted:
		val = 0;
		break;

	default:
		break;
	}

	writel(val, VA_LEDS_BASE);
	local_irq_restore(flags);
}
#endif	/* CONFIG_LEDS */

static struct resource bcm2708_mci_resources[] = {
	{
		.start			= MMCI0_BASE,
		.end			= MMCI0_BASE + SZ_4K - 1,
		.flags			= IORESOURCE_MEM,
	}, {
		.start			= DMA_BASE,
		.end			= DMA_BASE + SZ_4K - 1,
		.flags			= IORESOURCE_MEM,
	}, {
		.start                  = IRQ_DMA4,
		.end                    = IRQ_DMA4,
		.flags                  = IORESOURCE_IRQ,
	}, {
		.start                  = IRQ_SDIO,
		.end                    = IRQ_SDIO,
		.flags                  = IORESOURCE_IRQ,
	}
};

static struct platform_device bcm2708_mci_device = {
	.name			= "bcm2708_mci",
	.id			= 0, // first bcm2708_mci
	.resource		= bcm2708_mci_resources,
	.num_resources		= 4,
	.dev			= {					
                .coherent_dma_mask      = DMA_BIT_MASK(DMA_MASK_BITS_COMMON),			
	},							
};

static u64 fb_dmamask = DMA_BIT_MASK(DMA_MASK_BITS_COMMON);
static struct platform_device vceb_fb_device = {
	.name			= "vceb_fb",
	.id			= -1,  // only one vceb_fb
	.resource               = NULL,
	.num_resources          = 0,
	.dev			= {
                .dma_mask               = &fb_dmamask,
                .coherent_dma_mask      = DMA_BIT_MASK(DMA_MASK_BITS_COMMON),

   },
};

#ifdef DEV_UART1
static struct plat_serial8250_port bcm2708_uart1_platform_data[] = {
	{
		.mapbase	= UART1_BASE,
		.irq		= 10,
		.uartclk	= 1843200,
		.regshift	= 2,
		.iotype		= UPIO_MEM,
		.flags		= UPF_BOOT_AUTOCONF | UPF_IOREMAP | UPF_SKIP_TEST,
	},
	{ },
};

static struct platform_device bcm2708_uart1_device = {
	.name			= "serial8250",
	.id			= PLAT8250_DEV_PLATFORM,
	.dev			= {
		.platform_data	= bcm2708_uart1_platform_data,
	},
};
#endif

static struct resource bcm2708_usb_resources[] = {
	[0] =	{
                .start			= USB_BASE,
                .end			= USB_BASE + SZ_128K - 1,
                .flags			= IORESOURCE_MEM,
		},
	[1] =	{
		.start                  = IRQ_USB,
		.end                    = IRQ_USB,
		.flags                  = IORESOURCE_IRQ,
		},
};

static u64 usb_dmamask = DMA_BIT_MASK(DMA_MASK_BITS_COMMON);

static struct platform_device bcm2708_usb_device = {
	.name			= "bcm2708_usb",
	.id			= -1, // only one bcm2708_usb
	.resource		= bcm2708_usb_resources,
	.num_resources		= ARRAY_SIZE(bcm2708_usb_resources),
        .dev			= {
                .dma_mask               = &usb_dmamask,
                .coherent_dma_mask      = DMA_BIT_MASK(DMA_MASK_BITS_COMMON),
        },							
};

static struct resource bcm2708_smi_resources[] = {
	[0] = {
		.start = SMI_BASE,
		.end   = SMI_BASE + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
};

static u64 smi_dmamask = DMA_BIT_MASK(DMA_MASK_BITS_COMMON);
static struct platform_device bcm2708_smi_device = {
	.name           = "bcm2708_smi",
	.id             = -1,
	.resource       = bcm2708_smi_resources,
	.num_resources  = ARRAY_SIZE(bcm2708_smi_resources),
	.dev            = {
		.dma_mask           = &smi_dmamask,
		.coherent_dma_mask  = DMA_BIT_MASK(DMA_MASK_BITS_COMMON),
	},
};

static struct mtd_partition bcm2708_default_nand_part[] = {
   [0] = {
      .name = "bootloaders",
      .size = 3 * SZ_4M,
      .offset = 0,
      .mask_flags = MTD_WRITEABLE, /* force read-only */
   },

   [1] = {
	   .name = "u-boot env and splash screen",
	   .size = SZ_2M,
	   .offset = 3 * SZ_4M,
	   .mask_flags = MTD_WRITEABLE, /* force read-only */
   },
   [2] = {
      .name = "kernel",
      .size = 3 * SZ_4M,
      .offset = (3 * SZ_4M + SZ_2M),
      .mask_flags = MTD_WRITEABLE, /* force read-only */
   },
   [3] = {
      .name = "firmware",
      .size = 3 * SZ_4M,
      .offset = (SZ_16M + SZ_8M + SZ_2M),
      .mask_flags = MTD_WRITEABLE, /* force read-only */
   },
   [4] = {
      .name = "system",
      .size = SZ_48M,
      .offset = (SZ_32M + SZ_4M + SZ_2M),
   },
   [5] = {
      .name = "data",
      .size = MTDPART_SIZ_FULL,
      .offset = (SZ_64M + SZ_16M + SZ_4M + SZ_2M),
   },
};

static struct bcm2708_platform_nand bcm2708_plat_nand_info = {
   .partitions    = bcm2708_default_nand_part,
   .nr_partitions = ARRAY_SIZE(bcm2708_default_nand_part),
};

static struct platform_device bcm2708_nand_device = {
   .name          = "bcm2708_nand",
   .id            = -1,
   .num_resources = 0,
   .dev           = {
      .platform_data = &bcm2708_plat_nand_info,
   }
};


static u64 vuart_dmamask = DMA_BIT_MASK(DMA_MASK_BITS_COMMON);

static struct platform_device bcm2708_vuart_device = {
	.name                   = "bcm2708_vuart",
	.id                     = -1, // only one bcm-vuart
	.resource               = NULL,
	.num_resources          = 0,
        .dev			= {
                .dma_mask               = &vuart_dmamask,
                .coherent_dma_mask      = DMA_BIT_MASK(DMA_MASK_BITS_COMMON),
        },							
};

static struct resource bcm2708_vcio_resources[] = {
	[0] =	{                       // mailbox/semaphore/doorbell access
                .start			= MCORE_BASE,
                .end			= MCORE_BASE + SZ_4K - 1,
                .flags			= IORESOURCE_MEM,
        },
};

static u64 vcio_dmamask = DMA_BIT_MASK(DMA_MASK_BITS_COMMON);

static struct platform_device bcm2708_vcio_device = {
	.name                   = "bcm2708_vcio",
	.id                     = -1, // only one VideoCore I/O area
	.resource               = bcm2708_vcio_resources,
	.num_resources          = ARRAY_SIZE(bcm2708_vcio_resources),
        .dev			= {					
                .dma_mask               = &vcio_dmamask,
                .coherent_dma_mask      = DMA_BIT_MASK(DMA_MASK_BITS_COMMON),
        },							
};

static struct resource bcm2708_systemtimer_resources[] = {
	[0] =	{                       // system timer access
                .start			= ST_BASE,
                .end			= ST_BASE + SZ_4K - 1,
                .flags			= IORESOURCE_MEM,
	}, {
		.start                  = IRQ_TIMER3,
		.end                    = IRQ_TIMER3,
		.flags                  = IORESOURCE_IRQ,
	}


};

static u64 systemtimer_dmamask = DMA_BIT_MASK(DMA_MASK_BITS_COMMON);

static struct platform_device bcm2708_systemtimer_device = {
	.name                   = "bcm2708_systemtimer",
	.id                     = -1, // only one VideoCore I/O area
	.resource               = bcm2708_systemtimer_resources,
	.num_resources          = ARRAY_SIZE(bcm2708_systemtimer_resources),
        .dev			= {					
                .dma_mask               = &systemtimer_dmamask,
                .coherent_dma_mask      = DMA_BIT_MASK(DMA_MASK_BITS_COMMON),
        },							
};

#ifdef CONFIG_MMC_SDHCI_BCM2708 /* Arasan emmc SD */
static struct resource bcm2708_emmc_resources[] = {
	[0] = {
		.start = EMMC_BASE,
		.end   = EMMC_BASE + SZ_256 - 1, // we only need this area
                // the memory map actually makes SZ_4K available 
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_ARASANSDIO,
		.end   = IRQ_ARASANSDIO,
		.flags = IORESOURCE_IRQ,
	}
};

static u64 bcm2708_emmc_dmamask = 0xffffffffUL;

struct platform_device bcm2708_emmc_device = {
	.name		= "bcm2708_sdhci",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(bcm2708_emmc_resources),
	.resource	= bcm2708_emmc_resources,
	.dev		= {
		.dma_mask		= &bcm2708_emmc_dmamask,
		.coherent_dma_mask	= 0xffffffffUL
	},
};
#endif /* CONFIG_MMC_SDHCI_BCM2708 */

int __init bcm_register_device(struct platform_device *pdev)
{
	int ret;

	ret = platform_device_register(pdev);
	if (ret)
		pr_debug("Unable to register platform device '%s': %d\n",
			 pdev->name, ret);

	return ret;
}

void __init bcm2708_init(void)
{
	int i;
	u32 ipc_base;

	for (i = 0; i < ARRAY_SIZE(lookups); i++)
		clkdev_add(&lookups[i]);

	bcm_register_device(&bcm2708_vcio_device);
	bcm_register_device(&bcm2708_systemtimer_device);
	bcm_register_device(&bcm2708_vuart_device);
	bcm_register_device(&bcm2708_mci_device);
	bcm_register_device(&vceb_fb_device);
	bcm_register_device(&bcm2708_usb_device);
	bcm_register_device(&bcm2708_smi_device);
	bcm_register_device(&bcm2708_nand_device);
#ifdef DEV_UART1
	bcm_register_device(&bcm2708_uart1_device);
#endif
#ifdef CONFIG_MMC_SDHCI_BCM2708
	bcm_register_device(&bcm2708_emmc_device);
#endif
	bcm2835_get_ipc_base(&ipc_base);
	bcm2835_ipc_device.resource[0].start += ipc_base;
	bcm2835_ipc_device.resource[0].end += ipc_base;

 	bcm_register_device(&bcm2835_ipc_device);

	for (i = 0; i < ARRAY_SIZE(amba_devs); i++) {
		struct amba_device *d = amba_devs[i];
		amba_device_register(d, &iomem_resource);
	}

#ifdef CONFIG_LEDS
	leds_event = bcm2708_leds_event;
#endif
}

#define TIMER_PERIOD 10000 /* HZ in microsecs */

static void timer_set_mode(enum clock_event_mode mode,
			   struct clock_event_device *clk)
{
	unsigned long stc;

	switch(mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		stc = readl(__io_address(ST_BASE+0x04));
		writel(stc + TIMER_PERIOD, __io_address(ST_BASE+0x18)); // stc3
		break;
	case CLOCK_EVT_MODE_ONESHOT:
	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_SHUTDOWN:
	default:
		printk(KERN_ERR "timer_set_mode: unhandled mode:%d\n", (int)mode);
		break;
	}

}

static int timer_set_next_event(unsigned long evt,
				struct clock_event_device *unused)
{
	unsigned long stc;

        stc = readl(__io_address(ST_BASE+0x04));
        writel(stc+ TIMER_PERIOD, __io_address(ST_BASE+0x18)); // stc3
	return 0;
}

static struct clock_event_device timer0_clockevent =	 {
	.name		= "timer0",
	.shift		= 32,
	.features       = CLOCK_EVT_FEAT_ONESHOT,
	.set_mode	= timer_set_mode,
	.set_next_event	= timer_set_next_event,
};

/*
 * IRQ handler for the timer
 */
static irqreturn_t bcm2708_timer_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *evt = &timer0_clockevent;

        writel(1<<3, __io_address(ST_BASE+0x00)); // stcs clear timer int

	evt->event_handler(evt);

	return IRQ_HANDLED;
}

static struct irqaction bcm2708_timer_irq = {
	.name		= "BCM2708 Timer Tick",
	.flags		= IRQF_DISABLED | IRQF_TIMER | IRQF_IRQPOLL,
	.handler	= bcm2708_timer_interrupt,
};

/*
 * Set up timer interrupt, and return the current time in seconds.
 */
static void __init bcm2708_timer_init(void)
{
	/*
	 * Initialise to a known state (all timers off)
	 */
	writel(0, __io_address(ARM_T_CONTROL));
	/* 
	 * Make irqs happen for the system timer
	 */
	setup_irq(IRQ_TIMER3, &bcm2708_timer_irq);
#if 0
	bcm2708_clocksource_init();
#endif
	timer0_clockevent.mult =
		div_sc(1000000, NSEC_PER_SEC, timer0_clockevent.shift);
	timer0_clockevent.max_delta_ns =
		clockevent_delta2ns(0xffffffff, &timer0_clockevent);
	timer0_clockevent.min_delta_ns =
		clockevent_delta2ns(0xf, &timer0_clockevent);

	timer0_clockevent.cpumask = cpumask_of(0);
	clockevents_register_device(&timer0_clockevent);
}

struct sys_timer bcm2708_timer = {
	.init		= bcm2708_timer_init,
};

MACHINE_START(BCM2708, "BCM2708")
	/* Maintainer: Broadcom Europe Ltd. */
	.phys_io	= 0x101f1000,
	.io_pg_offst	= ((0xf11f1000) >> 18) & 0xfffc,
	.boot_params	= 0x00000100,
	.map_io		= bcm2708_map_io,
	.init_irq	= bcm2708_init_irq,
	.timer		= &bcm2708_timer,
	.init_machine	= bcm2708_init,
MACHINE_END

MACHINE_START(VERSATILE_PB, "BCM2708")
	/* Maintainer: Broadcom Europe Ltd. */
	.phys_io	= 0x101f1000,
	.io_pg_offst	= ((0xf11f1000) >> 18) & 0xfffc,
	.boot_params	= 0x00000100,
	.map_io		= bcm2708_map_io,
	.init_irq	= bcm2708_init_irq,
	.timer		= &bcm2708_timer,
	.init_machine	= bcm2708_init,
MACHINE_END
