/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 */

#include <libfdt.h>
#include <sbi/riscv_asm.h>
#include <sbi/sbi_hartmask.h>
#include <sbi/sbi_platform.h>
#include <sbi/sbi_string.h>
#include <sbi_utils/fdt/fdt_domain.h>
#include <sbi_utils/fdt/fdt_fixup.h>
#include <sbi_utils/fdt/fdt_helper.h>
#include <sbi_utils/fdt/fdt_pmu.h>
#include <sbi_utils/irqchip/fdt_irqchip.h>
#include <sbi_utils/irqchip/imsic.h>
#include <sbi_utils/serial/fdt_serial.h>
#include <sbi_utils/timer/fdt_timer.h>
#include <sbi_utils/ipi/fdt_ipi.h>
#include <sbi_utils/reset/fdt_reset.h>
#include <sbi_utils/serial/semihosting.h>

extern struct sbi_platform platform;
static u32 generic_hart_index2id[SBI_HARTMASK_MAX_BITS] = { 0 };

/*
 * The fw_platform_init() function is called very early on the boot HART
 * OpenSBI reference firmwares so that platform specific code get chance
 * to update "platform" instance before it is used.
 *
 * The arguments passed to fw_platform_init() function are boot time state
 * of A0 to A4 register. The "arg0" will be boot HART id and "arg1" will
 * be address of FDT passed by previous booting stage.
 *
 * The return value of fw_platform_init() function is the FDT location. If
 * FDT is unchanged (or FDT is modified in-place) then fw_platform_init()
 * can always return the original FDT location (i.e. 'arg1') unmodified.
 * 通过fdt读取一些配置到platform结构中以灵活的通过设备树传入硬件信息,
 * 也是为了让platform代码适配vender厂商的一大类的板卡
 */
unsigned long fw_platform_init(unsigned long arg0, unsigned long arg1,
				unsigned long arg2, unsigned long arg3,
				unsigned long arg4)
{
	const char *model;
	void *fdt = (void *)arg1;
	u32 hartid, hart_count = 0;
	int rc, root_offset, cpus_offset, cpu_offset, len;

	root_offset = fdt_path_offset(fdt, "/");
	if (root_offset < 0)
		goto fail;

	model = fdt_getprop(fdt, root_offset, "model", &len);
	if (model)
		sbi_strncpy(platform.name, model, sizeof(platform.name) - 1);

	cpus_offset = fdt_path_offset(fdt, "/cpus");
	if (cpus_offset < 0)
		goto fail;

	fdt_for_each_subnode(cpu_offset, fdt, cpus_offset) {
		rc = fdt_parse_hart_id(fdt, cpu_offset, &hartid);
		if (rc)
			continue;

		if (SBI_HARTMASK_MAX_BITS <= hartid)
			continue;

		if (!fdt_node_is_enabled(fdt, cpu_offset))
			continue;

		generic_hart_index2id[hart_count++] = hartid;
	}

	platform.hart_count = hart_count;

	/* Return original FDT pointer */
	return arg1;

fail:
	while (1)
		wfi();
}

/*
 * quard star early initialization.
 */
static int quard_star_early_init(bool cold_boot)
{
  if (!cold_boot)
		return 0;

	fdt_reset_init();
  return 0;
}

/*
 * quard star final initialization.
 * 针对平台初始化完成后将设备树做以修订为platform执行完毕后的状态信息,
 * 反应的设备真实的配置, 是传入fdt与platform代码共同作用后的真正的设备信息
 */
static int quard_star_final_init(bool cold_boot)
{
  void *fdt;

	if (!cold_boot)
		return 0;

	fdt = sbi_scratch_thishart_arg1_ptr();

	fdt_cpu_fixup(fdt);
	fdt_fixups(fdt);
	fdt_domain_fixup(fdt);

	return 0;
}

static void quard_star_early_exit(void) {}

static void quard_star_final_exit(void) {}

static int quard_star_domains_init(void)
{
	return fdt_domains_populate(fdt_get_address());
}

static int quard_star_console_init(void)
{
	if (semihosting_enabled())
		return semihosting_init();
	else
		return fdt_serial_init();
}

/*
 * Platform descriptor.
 */
const struct sbi_platform_operations platform_ops = {
	.early_init		        = quard_star_early_init           ,
	.final_init		        = quard_star_final_init           ,
	.early_exit		        = quard_star_early_exit           ,
	.final_exit		        = quard_star_final_exit           ,
  .domains_init	        = quard_star_domains_init         ,
	.console_init	        = quard_star_console_init         ,
	.irqchip_init	        = fdt_irqchip_init                ,
	.irqchip_exit	        = fdt_irqchip_exit                ,
	.ipi_init		          = fdt_ipi_init                    ,
	.ipi_exit		          = fdt_ipi_exit                    ,
	.timer_init		        = fdt_timer_init                  ,
	.timer_exit		        = fdt_timer_exit                  ,
};

struct sbi_platform platform = {
	.opensbi_version	    = OPENSBI_VERSION                 ,
	.platform_version	    = SBI_PLATFORM_VERSION(0x0, 0x01) ,
	.name			            = "Quard-Star"                    ,
	.features		          = SBI_PLATFORM_DEFAULT_FEATURES   ,
	.hart_count		        = SBI_HARTMASK_MAX_BITS           ,
	.hart_stack_size	    = SBI_PLATFORM_DEFAULT_HART_STACK_SIZE,
	.platform_ops_addr	  = (unsigned long)&platform_ops
};
