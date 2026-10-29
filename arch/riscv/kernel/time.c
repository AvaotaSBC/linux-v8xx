// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2012 Regents of the University of California
 * Copyright (C) 2017 SiFive
 */

#include <linux/of_clk.h>
#include <linux/clocksource.h>
#include <linux/delay.h>
#include <asm/sbi.h>
#include <asm/processor.h>
#include <asm/fixmap.h>

#define CCU_AON_DCXO_ST_REG	(0x4a010404)
unsigned long riscv_timebase;
EXPORT_SYMBOL_GPL(riscv_timebase);

static void __iomem *timestamp_base;

void __init time_init(void)
{
	void __iomem *dcxo_st;
	u32 val;

	dcxo_st = ioremap(CCU_AON_DCXO_ST_REG, 4);
	val = readl(dcxo_st);
	iounmap(dcxo_st);

	if(!!(val & BIT(31)))
		riscv_timebase = 24000000;
	else
		riscv_timebase = 40000000;

	lpj_fine = riscv_timebase / HZ;

	of_clk_init(NULL);
	timer_probe();
}

u64 notrace get_timestamp64(void)
{
	u32 *reg;
	u32 lo, hi;

	if (unlikely(!timestamp_base))
		timestamp_base = (void __iomem *)fix_to_virt(FIX_EARLYTS_MEM_BASE);

#ifdef CONFIG_JIFFIES_THEAD_CLOCKSOURCE
	reg = (u32 *)(timestamp_base + 4);
	lo = *(u32 *)(timestamp_base);
	lo |= (1 << 1);
	*(u32 *)(timestamp_base) = lo;
#else
	reg = (u32 *)timestamp_base;
#endif
	do {
		hi = reg[1];
		lo = reg[0];
	} while (hi != reg[1]);

	return ((u64)hi << 32) | (u64)lo;
}
