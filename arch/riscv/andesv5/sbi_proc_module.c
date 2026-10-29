/*
 *  Copyright (C) 2009 Andes Technology Corporation
 *  Copyright (C) 2019 Andes Technology Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/blkdev.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <asm/andesv5/csr.h>
#include <asm/andesv5/proc.h>
#include <asm/sbi.h>

#define INPUTLEN 32
#define MAX_TRIGGER_CNT 16

struct entry_struct{

	char *name;
	int perm;
	struct file_operations *fops;
};

static struct proc_dir_entry *proc_sbi;

#define DEBUG( enable, tagged, ...)				\
	do {							\
		if (enable) {					\
			if (tagged)				\
			printk("[ %30s() ] ", __func__);	\
			printk(__VA_ARGS__);			\
		}						\
	} while	(0)

static int debug = 0;
module_param(debug, int, 0);


static ssize_t proc_read_sbi_en(struct file *file, char __user *userbuf,
						size_t count, loff_t *ppos)
{
    int ret;
    char buf[18];
    if (!strncmp(file->f_path.dentry->d_name.name, "non_blocking", 12)) {
        ret = sprintf(buf, "non-blocking: %s\n", (sbi_andes_get_non_blocking_status() & MMISC_CTL_NON_BLOCKING_ENABLE) ? "Enabled" : "Disabled");
    } else if (!strncmp(file->f_path.dentry->d_name.name, "write_around", 12)) {
        ret = sprintf(buf, "write_around: %s\n", (sbi_andes_get_write_around_status() & MCACHE_CTL_DC_WAROUND_1_EN) ? "Enabled" : "Disabled");
    } else if (!strncmp(file->f_path.dentry->d_name.name, "l1i_prefetch", 12)) {
        ret = sprintf(buf, "l1i_prefetch: %s\n", (sbi_andes_get_write_around_status() & MCACHE_CTL_L1I_PREFETCH_EN) ? "Enabled" : "Disabled");
    } else if (!strncmp(file->f_path.dentry->d_name.name, "l1d_prefetch", 12)) {
        ret = sprintf(buf, "l1d_prefetch: %s\n", (sbi_andes_get_write_around_status() & MCACHE_CTL_L1D_PREFETCH_EN) ? "Enabled" : "Disabled");
    } else if (!strncmp(file->f_path.dentry->d_name.name, "mcache_ctl", 10)) {
        ret = sprintf(buf, "mcache_ctl: %lx\n", sbi_andes_get_write_around_status());
    } else if (!strncmp(file->f_path.dentry->d_name.name, "mmisc_ctl", 9)) {
        ret = sprintf(buf, "mmisc_ctl: %lx\n", sbi_andes_get_non_blocking_status());
    } else {
		return -EFAULT;
    }
    return simple_read_from_buffer(userbuf, count, ppos, buf, ret);
}

static ssize_t proc_write_sbi_en(struct file *file,
			const char __user *buffer, size_t count, loff_t *ppos)
{

	unsigned long en;
	char inbuf[INPUTLEN];

	if (count > INPUTLEN - 1)
		count = INPUTLEN - 1;

	if (copy_from_user(inbuf, buffer, count))
		return -EFAULT;

	inbuf[count] = '\0';

	if (!sscanf(inbuf, "%lx", &en))
		return -EFAULT;

	if (!strncmp(file->f_path.dentry->d_name.name, "non_blocking", 12)) {
		if (en && !(sbi_andes_get_non_blocking_status() & MMISC_CTL_NON_BLOCKING_ENABLE)) {
			sbi_andes_enable_non_blocking_load_store();
			DEBUG(debug, 1, "NON-blocking: Enabled\n");
		} else if (!en && (sbi_andes_get_non_blocking_status() & MMISC_CTL_NON_BLOCKING_ENABLE)) {
			sbi_andes_disable_non_blocking_load_store();
			DEBUG(debug, 1, "NON-blocking: Disabled\n");
		}
	} else if (!strncmp(file->f_path.dentry->d_name.name, "write_around", 12)) {
		if (en && !(sbi_andes_get_write_around_status() & MCACHE_CTL_DC_WAROUND_1_EN)) {
			sbi_andes_enable_write_around();
			DEBUG(debug, 1, "Write-around: Enabled\n");
		} else if (!en && (sbi_andes_get_write_around_status() & MCACHE_CTL_DC_WAROUND_1_EN)) {
			sbi_andes_disable_write_around();
			DEBUG(debug, 1, "Write-around: Disabled\n");
		}
	} else if (!strncmp(file->f_path.dentry->d_name.name, "l1i_prefetch", 12)) {
		if (en && !(sbi_andes_get_write_around_status() & MCACHE_CTL_L1I_PREFETCH_EN)) {
			sbi_andes_enable_l1i_cache();
			DEBUG(debug, 1, "L1I_cache_Prefetch: Enabled\n");
		} else if (!en && (sbi_andes_get_write_around_status() & MCACHE_CTL_L1I_PREFETCH_EN)) {
			sbi_andes_disable_l1i_cache();
			DEBUG(debug, 1, "L1I_cache_Prefetch: Disabled\n");
		}
	} else if (!strncmp(file->f_path.dentry->d_name.name, "l1d_prefetch", 12)) {
		if (en && !(sbi_andes_get_write_around_status() & MCACHE_CTL_L1D_PREFETCH_EN)) {
			sbi_andes_enable_l1d_cache();
			DEBUG(debug, 1, "L1D_cache_Prefetch: Enabled\n");
		} else if (!en && (sbi_andes_get_write_around_status() & MCACHE_CTL_L1D_PREFETCH_EN)) {
			sbi_andes_disable_l1d_cache();
			DEBUG(debug, 1, "L1D_cache_Prefetch: Disabled\n");
		}
	} else if (!strncmp(file->f_path.dentry->d_name.name, "mcache_ctl", 10)) {
		sbi_andes_set_mcache_ctl(en);
	} else if (!strncmp(file->f_path.dentry->d_name.name, "mmisc_ctl", 9)) {
		sbi_andes_set_mmisc_ctl(en);
	} else
		return -EFAULT;

	return count;
}

static struct file_operations en_fops = {
	.open = simple_open,
	.read = proc_read_sbi_en,
	.write = proc_write_sbi_en,
};

static struct {
	unsigned long addr;
	unsigned long prop;
} trigger_stat[MAX_TRIGGER_CNT];

static ssize_t proc_read_sbi_trigger(struct file *file, char __user *userbuf,
						size_t count, loff_t *ppos)
{
	int ret = 0, i;
	char buf[512];

	for (i = 0; i < MAX_TRIGGER_CNT; i++) {
		if (!trigger_stat[i].addr)
			continue;
		ret += snprintf(buf + ret, sizeof(buf) - ret, "0x%08lx %c%c%c\n",
						trigger_stat[i].addr,
						trigger_stat[i].prop & BREAKPOINT_PROP_LOAD ? 'R' : ' ',
						trigger_stat[i].prop & BREAKPOINT_PROP_STORE ? 'W' : ' ',
						trigger_stat[i].prop & BREAKPOINT_PROP_EXECUTE ? 'X' : ' ');
		if (ret >= sizeof(buf))
			break;
	}

	return simple_read_from_buffer(userbuf, count, ppos, buf, ret);
}

static ssize_t proc_write_sbi_trigger(struct file *file,
			const char __user *userbuf, size_t count, loff_t *ppos)
{
	int ret, i;
	char buf[64];
	unsigned long addr;
	const char *file_name = file->f_path.dentry->d_name.name;
	uintptr_t prop;
	int enable = 0;

	if (count > sizeof(buf) - 1)
		count = sizeof(buf) - 1;

	if (copy_from_user(buf, userbuf, count))
		return -EFAULT;
	buf[count] = '\0';

	ret = kstrtoul(buf, 0, &addr);
	if (ret) {
		pr_err("kstrtoul: %s error!, ret=%d\n", buf, ret);
		return count;
	}

	if (!strncmp(file_name, "set_watckpoint_access", sizeof("set_watckpoint_access") - 1)) {
		prop = BREAKPOINT_PROP_STORE | BREAKPOINT_PROP_LOAD;
		enable = 1;
	} else if (!strncmp(file_name, "set_watchpoint_write", sizeof("set_watchpoint_write") - 1)) {
		prop = BREAKPOINT_PROP_STORE;
		enable = 1;
	} else if (!strncmp(file_name, "set_watchpoint_read", sizeof("set_watchpoint_read") - 1)) {
		prop = BREAKPOINT_PROP_LOAD;
		enable = 1;
	} else if (!strncmp(file_name, "set_breakpoint", sizeof("set_breakpoint") - 1)) {
		prop = BREAKPOINT_PROP_EXECUTE;
		enable = 1;
	} else if (!strncmp(file_name, "clear_watchpoint", sizeof("clear_watchpoint") - 1)) {
		enable = 0;
	}

	if (enable) {
		for (i = 0; i < MAX_TRIGGER_CNT; i++)
			if (trigger_stat[i].addr == addr)
				break;
		if (i != MAX_TRIGGER_CNT) {
			pr_info("0x%08lx already watch at trigger%d\n", addr, i);
			goto out;
		}
	}
	ret = sbi_andes_set_trigger_mcontrol(addr, prop, enable);
	if (ret < 0)
		pr_info("%s 0x%08lx failed, ret = %d\n", file_name, addr, ret);
	else
		pr_info("%s 0x%08lx successed [trigger = %d]\n", file_name, addr, ret);

	if (ret >= 0) {
		if (enable) {
			trigger_stat[ret].addr = addr;
			trigger_stat[ret].prop = prop;
		} else {
			trigger_stat[ret].addr = 0;
		}
	}

out:
	return count;
}

static struct file_operations breakpoint_fops = {
	.open = simple_open,
	.read = proc_read_sbi_trigger,
	.write = proc_write_sbi_trigger,
};

static void create_seq_entry(struct entry_struct *e, mode_t mode,
			     struct proc_dir_entry *parent)
{

	struct proc_dir_entry *entry = proc_create(e->name, mode, parent, e->fops);

	if (!entry)
		printk(KERN_ERR "invalid %s register.\n", e->name);
}

static void install_proc_table(struct entry_struct *table)
{
	while (table->name) {

		create_seq_entry(table, table->perm, proc_sbi);
		table++;
	}
}

static void remove_proc_table(struct entry_struct *table)
{

	while (table->name) {
		remove_proc_entry(table->name, proc_sbi);
		table++;
	}
}

struct entry_struct proc_table_sbi[] = {

	{"non_blocking", 0644, &en_fops},	//sbi_ae350_non_blocking_load_store
	{"write_around", 0644, &en_fops},       //sbi_ae350_write_around
	{"l1i_prefetch", 0644, &en_fops},       //sbi_ae350_L1I-prefetch
	{"l1d_prefetch", 0644, &en_fops},       //sbi_ae350_L1D-prefetch
	{"mcache_ctl", 0644, &en_fops},		//sbi_ae350_mcache_ctl
	{"mmisc_ctl", 0644, &en_fops},		//sbi_ae350_mmisc_ctl

	{"clear_watchpoint", 0644, &breakpoint_fops },		// trigger_set_mcontrol
	{"set_watckpoint_access", 0644, &breakpoint_fops },		// trigger_set_mcontrol
	{"set_watchpoint_write", 0644, &breakpoint_fops },		// trigger_set_mcontrol
	{"set_watchpoint_read", 0644, &breakpoint_fops },		// trigger_set_mcontrol
	{"set_breakpoint", 0644, &breakpoint_fops },		// trigger_set_mcontrol
};
static int __init init_sbi(void)
{

	DEBUG(debug, 0, "SBI module registered\n");

	if (!(proc_sbi = proc_mkdir("sbi", NULL)))
		return -ENOMEM;

	install_proc_table(proc_table_sbi);

	return 0;
}

static void __exit cleanup_sbi(void)
{

	remove_proc_table(proc_table_sbi);
	remove_proc_entry("sbi", NULL);

	DEBUG(debug, 1, "SBI module unregistered\n");
}

module_init(init_sbi);
module_exit(cleanup_sbi);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Userspace SBI Module");
