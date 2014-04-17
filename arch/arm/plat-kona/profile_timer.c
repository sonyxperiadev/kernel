/* Profile timer implementations */
#include <linux/init.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/clk.h>
#include <asm/io.h>

#include <mach/profile_timer.h>
#include <mach/rdb/brcm_rdb_glbtmr.h>
#include <mach/clock.h>

static void __iomem *proftmr_regbase = NULL;

void __init profile_timer_init(void __iomem *base)
{
	uint32_t reg;

	proftmr_regbase = base;

	/* Reset profile/global timer */
	writel(0, base + GLBTMR_GLOB_CTRL_OFFSET);

	/* Clear pending interrupts */
	reg = readl(base + GLBTMR_GLOB_STATUS_OFFSET);
	reg &= ~(GLBTMR_GLOB_STATUS_RESERVED_MASK);
	reg |= (1 << GLBTMR_GLOB_STATUS_EVENT_G_SHIFT);
	writel(reg, base + GLBTMR_GLOB_STATUS_OFFSET);

	/* Enable profile timer now with
	 *prescaler = 0, so timer freq = A9 PERIPHCLK
	 * IRQ disabled
	 * Comapre disabled
	 */

	reg = readl(base + GLBTMR_GLOB_CTRL_OFFSET);
	reg &= ~(GLBTMR_GLOB_CTRL_RESERVED_MASK);
	reg |= (1 << GLBTMR_GLOB_CTRL_TIMER_EN_G_SHIFT);
	writel(reg, base + GLBTMR_GLOB_CTRL_OFFSET);
}

static void
profile_timer_get_counter(void __iomem *base, uint32_t *msw, uint32_t *lsw)
{
	/* Read 64-bit free running counter
	 * 1. Read hi-word
	 * 2. Read low-word
	 * 3. Read hi-word again
	 * 4.1
	 *     if new hi-word is not equal to previously read hi-word, then
	 *     start from #1
	 * 4.2
	 *     if new hi-word is equal to previously read hi-word then stop.
	 */

	while (1) {
		*msw = readl(base + GLBTMR_GLOB_HI_OFFSET);
		*lsw = readl(base + GLBTMR_GLOB_LOW_OFFSET);
		if (*msw == readl(base + GLBTMR_GLOB_HI_OFFSET))
			break;
	}

	return;
}

timer_tick_rate_t timer_get_tick_rate(void)
{
	uint32_t prescaler;

	prescaler = readl(proftmr_regbase + GLBTMR_GLOB_CTRL_OFFSET);
	prescaler &= GLBTMR_GLOB_CTRL_PRESCALER_G_MASK;
	prescaler >>= GLBTMR_GLOB_CTRL_PRESCALER_G_SHIFT;

	return clk_get_rate(clk_get(NULL, ARM_CORE_CLK_NAME_STR)) /
		((1 + prescaler) * 2);
}

timer_tick_count_t timer_get_tick_count(void)
{
	uint32_t msw, lsw;
	uint64_t tick;

	profile_timer_get_counter(proftmr_regbase, &msw, &lsw);

	tick = (((uint64_t)msw << 32) | ((uint64_t)lsw));

	return (*(uint32_t *)(&tick));
}

timer_msec_t timer_ticks_to_msec(timer_tick_count_t ticks)
{
	return (ticks / (timer_get_tick_rate() / 1000));
}

timer_msec_t timer_get_msec(void)
{
	return timer_ticks_to_msec(timer_get_tick_count());
}

EXPORT_SYMBOL(timer_get_tick_count);
EXPORT_SYMBOL(timer_ticks_to_msec);
EXPORT_SYMBOL(timer_get_tick_rate);
EXPORT_SYMBOL(timer_get_msec);
