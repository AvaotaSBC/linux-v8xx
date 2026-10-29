/*
 *  Copyright (C) 2020 Andes Technology Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <asm/andesv5/csr.h>
#include <asm/andesv5/proc.h>
#include <asm/sbi.h>


void sbi_andes_suspend_prepare(char main_core, char enable)
{
	sbi_ecall(SBI_EXT_ANDES, SBI_EXT_ANDES_SUSPEND_PREPARE, main_core, enable, 0, 0, 0, 0);
}
EXPORT_SYMBOL(sbi_andes_suspend_prepare);

void sbi_andes_suspend_mem(void)
{
	sbi_ecall(SBI_EXT_ANDES, SBI_EXT_ANDES_SUSPEND_MEM, 0, 0, 0, 0, 0, 0);
}
EXPORT_SYMBOL(sbi_andes_suspend_mem);

void sbi_andes_restart(void)
{
	sbi_ecall(SBI_EXT_ANDES, SBI_EXT_ANDES_RESTART, 0, 0, 0, 0, 0, 0);
}
EXPORT_SYMBOL(sbi_andes_restart);

void sbi_andes_write_powerbrake(int val)
{
  sbi_ecall(SBI_EXT_ANDES, SBI_EXT_ANDES_WRITE_POWERBRAKE, val, 0, 0, 0, 0, 0);
}
EXPORT_SYMBOL(sbi_andes_write_powerbrake);

int sbi_andes_read_powerbrake(void)
{
  struct sbiret ret;
  ret = sbi_ecall(SBI_EXT_ANDES, SBI_EXT_ANDES_READ_POWERBRAKE, 0, 0, 0, 0, 0, 0);
  return ret.value;
}
EXPORT_SYMBOL(sbi_andes_read_powerbrake);

void sbi_andes_set_suspend_mode(unsigned int suspend_mode)
{
	sbi_ecall(SBI_EXT_ANDES, SBI_EXT_ANDES_SET_SUSPEND_MODE, suspend_mode, 0, 0, 0, 0, 0);
}
EXPORT_SYMBOL(sbi_andes_set_suspend_mode);

void sbi_andes_enter_suspend_mode(int main_core, unsigned int wake_mask)
{
	sbi_ecall(SBI_EXT_ANDES, SBI_EXT_ANDES_ENTER_SUSPEND_MODE, main_core, wake_mask, 0, 0, 0, 0);
}
EXPORT_SYMBOL(sbi_andes_enter_suspend_mode);

void sbi_andes_set_reset_vec(int val)
{
	sbi_ecall(SBI_EXT_ANDES, SBI_EXT_ANDES_SET_RESET_VEC, val, 0, 0, 0, 0, 0);
}
EXPORT_SYMBOL(sbi_andes_set_reset_vec);

void sbi_andes_set_pma(void *arg)
{
	phys_addr_t offset = ((struct pma_arg_t*)arg)->offset;
	unsigned long vaddr = ((struct pma_arg_t*)arg)->vaddr;
	size_t size = ((struct pma_arg_t*)arg)->size;
	size_t entry_id = ((struct pma_arg_t*)arg)->entry_id;
	sbi_ecall(SBI_EXT_ANDES, SBI_EXT_ANDES_SET_PMA, offset, vaddr, size, entry_id, 0, 0);
}
EXPORT_SYMBOL(sbi_andes_set_pma);

void sbi_andes_free_pma(unsigned long entry_id)
{
	sbi_ecall(SBI_EXT_ANDES, SBI_EXT_ANDES_FREE_PMA, entry_id, 0, 0, 0, 0, 0);
}
EXPORT_SYMBOL(sbi_andes_free_pma);

long sbi_andes_probe_pma(void)
{
	struct sbiret ret;
	ret = sbi_ecall(SBI_EXT_ANDES, SBI_EXT_ANDES_PROBE_PMA, 0, 0, 0, 0, 0, 0);
	return ret.value;
}
EXPORT_SYMBOL(sbi_andes_probe_pma);

void sbi_andes_set_trigger(unsigned int type, uintptr_t data, int enable)
{
	sbi_ecall(SBI_EXT_ANDES, SBI_EXT_ANDES_TRIGGER, type, data, 0, 0, enable, 0);
}
EXPORT_SYMBOL(sbi_andes_set_trigger);

#define TRIGGER_TYPE_MCONTROL 2
int sbi_andes_set_trigger_mcontrol(uintptr_t addr, uintptr_t prop, int enable)
{
	struct sbiret ret;
	ret = sbi_ecall(SBI_EXT_ANDES, SBI_EXT_ANDES_TRIGGER, TRIGGER_TYPE_MCONTROL, addr, prop, 1, enable, 0);
	return ret.value;
}
EXPORT_SYMBOL(sbi_andes_set_trigger_mcontrol);

long sbi_get_marchid(void)
{
	struct sbiret ret;
	ret = sbi_ecall(SBI_EXT_BASE, SBI_EXT_BASE_GET_MARCHID, 0, 0, 0, 0, 0, 0);
	return ret.value;
}
EXPORT_SYMBOL(sbi_get_marchid);

int sbi_andes_set_wakeup_source(u32 wakeup_irq, unsigned int on)
{
	struct sbiret ret;
	if (on)
		ret = sbi_ecall(SBI_EXT_ANDES, SBI_EXT_ANDES_WAKEUP_SRC_SET, wakeup_irq, 0, 0, 0, 0, 0);
	else
		ret = sbi_ecall(SBI_EXT_ANDES, SBI_EXT_ANDES_WAKEUP_SRC_CLR, wakeup_irq, 0, 0, 0, 0, 0);
	return ret.value;
}
EXPORT_SYMBOL(sbi_andes_set_wakeup_source);

int sbi_andes_get_wakeup_source(void)
{
	struct sbiret ret;
	ret = sbi_ecall(SBI_EXT_BASE, SBI_EXT_ANDES_WAKEUP_SRC_QUERY, 0, 0, 0, 0, 0, 0);
	return ret.value;
}
EXPORT_SYMBOL(sbi_andes_get_wakeup_source);

int sbi_andes_set_wakeup_wakeup_io_num(u32 wakeup_io_num)
{
	struct sbiret ret;
	ret = sbi_ecall(SBI_EXT_ANDES, SBI_EXT_ANDES_WAKEUP_SRC_IO_NUM, wakeup_io_num, 0, 0, 0, 0, 0);
	return ret.value;
}
EXPORT_SYMBOL(sbi_andes_set_wakeup_wakeup_io_num);

int sbi_andes_set_ldo_onoff(u32 set_ldo_onoff)
{
	struct sbiret ret;
	ret = sbi_ecall(SBI_EXT_ANDES, SBI_EXT_ANDES_SET_LDO_ONOFF, set_ldo_onoff, 0, 0, 0, 0, 0);
	return ret.value;
}
EXPORT_SYMBOL(sbi_andes_set_ldo_onoff);

int sbi_andes_set_ultra_standby(u32 use_ultra_standby)
{
	struct sbiret ret;
	ret = sbi_ecall(SBI_EXT_ANDES, SBI_EXT_ANDES_SET_ULTRA_STANDBY, use_ultra_standby, 0, 0, 0, 0, 0);
	return ret.value;
}
EXPORT_SYMBOL(sbi_andes_set_ultra_standby);

int sbi_andes_awbase(u32 data)
{
	struct sbiret ret;
	ret = sbi_ecall(SBI_EXT_AWBASE, 0, data, 0, 0, 0, 0, 0);
	return ret.value;
}
EXPORT_SYMBOL(sbi_andes_awbase);

int sunxi_usb_ctrl(int id, u32 data)
{
	struct sbiret ret;
	ret = sbi_ecall(SBI_EXT_AWBASE, id, data, 0, 0, 0, 0, 0);
	return ret.value;
}
EXPORT_SYMBOL(sunxi_usb_ctrl);
