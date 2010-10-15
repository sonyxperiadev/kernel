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
        // The ARM_T_FREECNT free running counter increments at the rate of
        // the 2708's system clock (not the ARM clock)
        // that's normally a rate of 250MHz - a period of 4ns
	unsigned long long v = cnt32_to_63(readl(__io_address(ARM_T_FREECNT)));
        // For cnt32_to_63 to work correctly we MUST call this routine at least
        // once every half-32-bit-wraparound period - that's once every 8s
        // or so

#ifdef CONFIG_ARCH_BCM2708_CHIPIT
	v *= 1000;
#else
        v *= 4;
#endif

	return v;
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

static struct platform_device bcm2708_fb_device = {
	.name			= "bcm2708_fb",
	.id			= -1,  // only one bcm2708_fb
	.resource               = NULL,
	.num_resources          = 0,
	.dev			= {
                .dma_mask               = &fb_dmamask,
                .coherent_dma_mask      = DMA_BIT_MASK(DMA_MASK_BITS_COMMON),
	},							
};

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
        },
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

	for (i = 0; i < ARRAY_SIZE(lookups); i++)
		clkdev_add(&lookups[i]);

	bcm_register_device(&bcm2708_vcio_device);
	bcm_register_device(&bcm2708_systemtimer_device);
	bcm_register_device(&bcm2708_vuart_device);
	bcm_register_device(&bcm2708_mci_device);
	bcm_register_device(&bcm2708_fb_device);
   bcm_register_device(&vceb_fb_device);
	bcm_register_device(&bcm2708_usb_device);
#ifdef DEV_UART1
	bcm_register_device(&bcm2708_uart1_device);
#endif
        
	for (i = 0; i < ARRAY_SIZE(amba_devs); i++) {
		struct amba_device *d = amba_devs[i];
		amba_device_register(d, &iomem_resource);
	}

#ifdef CONFIG_LEDS
	leds_event = bcm2708_leds_event;
#endif
}


/*
 * How long is the timer interval?
 */
#define TIMER_INTERVAL	(TICKS_PER_uSEC * mSEC_10)
#if TIMER_INTERVAL >= 0x100000
#define TIMER_RELOAD	(TIMER_INTERVAL >> 8)
#define TIMER_DIVISOR	(ARM_T_CONTROL_DIV256)
#define TICKS2USECS(x)	(256 * (x) / TICKS_PER_uSEC)
#elif TIMER_INTERVAL >= 0x10000
#define TIMER_RELOAD	(TIMER_INTERVAL >> 4)		/* Divide by 16 */
#define TIMER_DIVISOR	(ARM_T_CONTROL_DIV16)
#define TICKS2USECS(x)	(16 * (x) / TICKS_PER_uSEC)
#else
#define TIMER_RELOAD	(TIMER_INTERVAL)
#define TIMER_DIVISOR	(ARM_T_CONTROL_DIV1)
#define TICKS2USECS(x)	((x) / TICKS_PER_uSEC)
#endif

/*
 * This is the Versatile sched_clock implementation.  This has
 * a resolution of 41.7ns, and a maximum value of about 35583 days.
 *
 * The return value is guaranteed to be monotonic in that range as
 * long as there is always less than 89 seconds between successive
 * calls to this function.
 */



static void timer_set_mode(enum clock_event_mode mode,
			   struct clock_event_device *clk)
{
	unsigned long ctrl;

   ctrl = TIMER_CTRL_32BIT | TIMER_CTRL_IE | TIMER_CTRL_DBGHALT | TIMER_CTRL_ENAFREE;
	switch(mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		writel(TIMER_RELOAD, __io_address(ARM_T_LOAD));
		ctrl |= TIMER_CTRL_PERIODIC | TIMER_CTRL_ENABLE;
		break;
	case CLOCK_EVT_MODE_ONESHOT:
		/* period set, and timer enabled in 'next_event' hook */
		ctrl |= TIMER_CTRL_ONESHOT;
		break;
	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_SHUTDOWN:
	default:
	   break;
	}

	writel(ctrl, __io_address(ARM_T_CONTROL));
}

static int timer_set_next_event(unsigned long evt,
				struct clock_event_device *unused)
{
	unsigned long ctrl = readl(__io_address(ARM_T_CONTROL));

	writel(evt, __io_address(ARM_T_LOAD));
	writel(ctrl | TIMER_CTRL_ENABLE, __io_address(ARM_T_CONTROL));

	return 0;
}

static struct clock_event_device timer0_clockevent =	 {
	.name		= "timer0",
	.shift		= 32,
	.features       = CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT,
	.set_mode	= timer_set_mode,
	.set_next_event	= timer_set_next_event,
};

/*
 * IRQ handler for the timer
 */
static irqreturn_t bcm2708_timer_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *evt = &timer0_clockevent;

	writel(1, __io_address(ARM_T_IRQCNTL));

	evt->event_handler(evt);

	return IRQ_HANDLED;
}

static struct irqaction bcm2708_timer_irq = {
	.name		= "Versatile Timer Tick",
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
	setup_irq(IRQ_ARM_TIMER, &bcm2708_timer_irq);
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
