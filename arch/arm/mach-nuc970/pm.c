/* linux/arch/arm/mach-nuc970/pm.c
 *
 * Copyright (c) 2016 Nuvoton technology corporation
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/io.h>
#include <linux/cpumask.h>
#include <linux/delay.h>
#include <linux/cpu_pm.h>
#include <linux/suspend.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <mach/regs-clock.h>
#include <mach/regs-aic.h>
#include <mach/regs-gcr.h>
#include <mach/map.h>

#include <linux/slab.h>

#ifdef CONFIG_PM_SLEEP
#define PM_FROM_SRAM

#ifdef PM_FROM_SRAM
#include <asm/cacheflush.h>

#define SRAM_BASE	NUC970_VA_SRAM
#define SRAM_LEN	NUC970_SZ_SRAM
#define TEMP_SRAM_AREA  (SRAM_BASE+0x7000)
/*
 * Pointers used for sizing and copying suspend function data
 */
extern int nuc970_sys_suspend(void);
extern int nuc970_sys_suspend_sz;
#endif

static int nuc970_suspend_enter(suspend_state_t state)
{

	#ifdef PM_FROM_SRAM
	int (*nuc970_suspend_ptr) (void);
	void *sram_swap_area;
	#endif

	if(state != PM_SUSPEND_MEM)
		return -EINVAL;

	__raw_writel(__raw_readl(REG_CLK_PMCON) & ~1, REG_CLK_PMCON);	// clear bit 0 so NUC970 enter pd mode instead of idle in next function call
	#ifndef PM_FROM_SRAM
		cpu_do_idle();
	#else
	/* Allocate some space for temporary SRAM storage */
	sram_swap_area = kmalloc(nuc970_sys_suspend_sz, GFP_KERNEL);
	if (!sram_swap_area) {
		printk(KERN_ERR"PM Suspend: cannot allocate memory to save portion of SRAM\n");
		return -ENOMEM;
	}

	__raw_writel(__raw_readl(REG_WKUPSER), REG_WKUPSSR);	// clear wake source flag

	/* Backup a small area of SRAM used for the suspend code */
	memcpy(sram_swap_area, (void *) TEMP_SRAM_AREA,nuc970_sys_suspend_sz);
	/*
	 * Copy code to suspend system into SRAM.
	 * The suspend code needs to run from SRAM.
	 */
	memcpy(TEMP_SRAM_AREA, (void *) nuc970_sys_suspend,nuc970_sys_suspend_sz);
	flush_icache_range((unsigned long)TEMP_SRAM_AREA,(unsigned long)(TEMP_SRAM_AREA) + nuc970_sys_suspend_sz);
	nuc970_suspend_ptr = (void *) TEMP_SRAM_AREA;
	flush_cache_all();
	(void) nuc970_suspend_ptr();

	/* Restore original SRAM contents */
	memcpy((void *) TEMP_SRAM_AREA, sram_swap_area,nuc970_sys_suspend_sz);
	#endif


	return 0;
}

static const struct platform_suspend_ops nuc970_suspend_ops = {
	.valid		= suspend_valid_only_mem,
	.enter		= nuc970_suspend_enter,
};



void __init nuc970_init_suspend(void)
{
	__raw_writel(__raw_readl(REG_CLK_PMCON) & ~0xFF000000, REG_CLK_PMCON);	// reduce wake up delay time waiting for HXT stable
	suspend_set_ops(&nuc970_suspend_ops);
}
#endif

