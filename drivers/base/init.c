// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2002-3 Patrick Mochel
 * Copyright (c) 2002-3 Open Source Development Labs
 */

#include <linux/device.h>
#include <linux/init.h>
#include <linux/memory.h>
#include <linux/of.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/sched/task.h>
#include <uapi/linux/sched/types.h>
#include <sunxi-chips.h>

#include "base.h"

extern int hrtimer_irq_remedy_init(void);

/**
 * driver_init - initialize driver model.
 *
 * Call the driver model init functions to initialize their
 * subsystems. Called early from init/main.c.
 */
void __init driver_init(void)
{
	if (sunxi_chip_alter_version() != SUNXI_CHIP_ALTER_VERSION_V821B)
		hrtimer_irq_remedy_init();

	/* These are the core pieces */
	devtmpfs_init();
	devices_init();
	buses_init();
	classes_init();
	firmware_init();
	hypervisor_init();

	/* These are also core pieces, but must come after the
	 * core core pieces.
	 */
	of_core_init();
	platform_bus_init();
	cpu_dev_init();
	memory_dev_init();
	container_dev_init();
}
