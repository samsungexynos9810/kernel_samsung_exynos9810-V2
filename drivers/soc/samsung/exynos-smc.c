/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * EXYNOS SMC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/smc.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/device.h>

int exynos_smc(unsigned long cmd, unsigned long arg1, unsigned long arg2, unsigned long arg3)
{
	int32_t ret;
	ret = __exynos_smc(cmd, arg1, arg2, arg3);
	return ret;
}
EXPORT_SYMBOL(exynos_smc);
