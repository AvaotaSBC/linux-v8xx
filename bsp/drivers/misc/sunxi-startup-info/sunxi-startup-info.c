// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2023 - 2025 Allwinner Technology Co.,Ltd. All rights reserved.
 * Copyright (c) 2025 zhangyuanjing <zhangyuanjing@allwinnertech.com>
 */

#include <asm/io.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/sysfs.h>
#include <linux/device.h>
#include <sunxi-chips.h>
#include <sunxi-log.h>

#define SUNXI_PRCM_BASE (0x4A000000)
#define SUNXI_PMU_RTC_BASE (0x4A000800)

#define SUNXI_RESET_SRC_STATUS (SUNXI_PRCM_BASE + 0x1D4)
#define SUNXI_RTC_REG (SUNXI_PRCM_BASE + 0x21c)
#define SUNXI_WAKEUP_REG_STATUS (SUNXI_PMU_RTC_BASE + 0x80)

static u32 sunxi_reset_src_status;
static u32 sunxi_wakeup_reg_status;
static u32 sunxi_rtc_reboot_reg;

static ssize_t wakeup_source_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	u32 startup_reg_data;
	ssize_t len = 0;

	startup_reg_data = sunxi_wakeup_reg_status;
	if (startup_reg_data & BIT(0))
		len += snprintf(buf + len, PAGE_SIZE - len, "rtc_timer\n");

	if (startup_reg_data & BIT(1))
		len += snprintf(buf + len, PAGE_SIZE - len, "sysrtc\n");

	if (startup_reg_data & BIT(2))
		len += snprintf(buf + len, PAGE_SIZE - len, "wakeup_io\n");

	return len;
}

static ssize_t reset_source_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	u32 startup_reg_data;
	ssize_t len = 0;

	startup_reg_data = sunxi_reset_src_status;

	if (startup_reg_data & BIT(0) && startup_reg_data & BIT(1) &&
	    startup_reg_data & BIT(3)) {
		len += snprintf(buf + len, PAGE_SIZE - len, "cold_boot\n");
		return len;
	}

	if (startup_reg_data & BIT(3)) {
		len += snprintf(buf + len, PAGE_SIZE - len, "rtc_wdg_rst\n");
		if (sunxi_rtc_reboot_reg == 0x2)
			len += snprintf(buf + len, PAGE_SIZE - len,
					"user_reboot\n");
	}

	if (startup_reg_data & BIT(1))
		len += snprintf(buf + len, PAGE_SIZE - len, "det_rst\n");

	if (startup_reg_data & BIT(0))
		len += snprintf(buf + len, PAGE_SIZE - len, "pwron_rst\n");

	return len;
}

static DEVICE_ATTR(wakeup_source, 0444, wakeup_source_show, NULL);
static DEVICE_ATTR(reset_source, 0444, reset_source_show, NULL);

static struct class *sunxi_class;
static struct device *sunxi_device;

static int __init sunxi_startup_info_record(void)
{
	void __iomem *rom_reg;

	rom_reg = ioremap(SUNXI_RESET_SRC_STATUS, 0x4);
	if (!rom_reg) {
		pr_err("Failed to map register address 0x%x\n",
		       SUNXI_RESET_SRC_STATUS);
		return -ENOMEM;
	}
	sunxi_reset_src_status = readl(rom_reg);
	/* Write 1 to reset for next boot record */
	writel(0xf, rom_reg);
	iounmap(rom_reg);

	rom_reg = ioremap(SUNXI_WAKEUP_REG_STATUS, 0x4);
	if (!rom_reg) {
		pr_err("Failed to map register address 0x%x\n",
		       SUNXI_WAKEUP_REG_STATUS);
		return -ENOMEM;
	}
	sunxi_wakeup_reg_status = readl(rom_reg);
	iounmap(rom_reg);

	rom_reg = ioremap(SUNXI_RTC_REG, 0x4);
	if (!rom_reg) {
		pr_err("Failed to map register address 0x%x\n", SUNXI_RTC_REG);
		return -ENOMEM;
	}
	sunxi_rtc_reboot_reg = readl(rom_reg);
	iounmap(rom_reg);

	return 0;
}
early_initcall(sunxi_startup_info_record);

static int __init sunxi_startup_info_init(void)
{
	/* Create sysfs entries */
	sunxi_class = class_create(THIS_MODULE, "sunxi_startup_info");
	if (IS_ERR(sunxi_class)) {
		pr_err("Failed to create sunxi class\n");
		return PTR_ERR(sunxi_class);
	}

	sunxi_device =
		device_create(sunxi_class, NULL, 0, NULL, "startup_info");
	if (IS_ERR(sunxi_device)) {
		class_destroy(sunxi_class);
		pr_err("Failed to create device\n");
		return PTR_ERR(sunxi_device);
	}

	/* Create sysfs files for wakeup source and reset source */
	if (device_create_file(sunxi_device, &dev_attr_wakeup_source)) {
		pr_err("Failed to create wakeup_source sysfs\n");
		device_unregister(sunxi_device);
		class_destroy(sunxi_class);
		return -EINVAL;
	}

	if (device_create_file(sunxi_device, &dev_attr_reset_source)) {
		pr_err("Failed to create reset_source sysfs\n");
		device_remove_file(sunxi_device, &dev_attr_wakeup_source);
		device_unregister(sunxi_device);
		class_destroy(sunxi_class);
		return -EINVAL;
	}

	return 0;
}

static void __exit sunxi_startup_info_exit(void)
{
	/* Remove sysfs files and clean up */
	device_remove_file(sunxi_device, &dev_attr_reset_source);
	device_remove_file(sunxi_device, &dev_attr_wakeup_source);
	device_unregister(sunxi_device);
	class_destroy(sunxi_class);
}
module_init(sunxi_startup_info_init);
module_exit(sunxi_startup_info_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("zhangyuanjing <zhangyuanjing@allwinnertech.com>");
MODULE_DESCRIPTION("sunxi startup info driver");
MODULE_VERSION("0.0.1");
