
/*
 * Copyright (c) 2015 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _INIT_H_
#define _INIT_H_

#include <device.h>
#include <toolchain.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * System initialization levels. The PRE_KERNEL_1 and PRE_KERNEL_2 levels are
 * executed in the kernel's initialization context, which uses the interrupt
 * stack. The remaining levels are executed in the kernel's main task.
 *
 * PRIMARY, SECONDARY, NANOKERNEL, MICROKERNEL levels are currently deprecated
 * and will be removed in the future.
 */

#define _SYS_INIT_LEVEL_PRE_KERNEL_1	0
#define _SYS_INIT_LEVEL_PRE_KERNEL_2	1
#define _SYS_INIT_LEVEL_POST_KERNEL	2
#define _SYS_INIT_LEVEL_APPLICATION	3

/* Deprecated, remove eventually */
#define _SYS_INIT_LEVEL_PRIMARY		4
#define _SYS_INIT_LEVEL_SECONDARY	5
#define _SYS_INIT_LEVEL_NANOKERNEL	6
#define _SYS_INIT_LEVEL_MICROKERNEL	7


/* Counter use to avoid issues if two or more system devices are declared
 * in the same C file with the same init function
 */
#define _SYS_NAME(init_fn) _CONCAT(_CONCAT(sys_init_, init_fn), __COUNTER__)

/**
 * @def SYS_INIT
 *
 * @brief Run an initialization function at boot at specified priority
 *
 * @details This macro lets you run a function at system boot.
 *
 * @param init_fn Pointer to the boot function to run
 *
 * @param level The initialization level, See DEVICE_INIT for details.
 *
 * @param prio Priority within the selected initialization level. See
 * DEVICE_INIT for details.
 */
#define SYS_INIT(init_fn, level, prio) \
	DEVICE_INIT(_SYS_NAME(init_fn), "", init_fn, NULL, NULL, level, prio)

/**
 * @def SYS_INIT_PM
 *
 * @warning This macro is deprecated and will be removed in
 *        a future version, superseded by SYS_DEVICE_DEFINE.
 *
 * @brief Run an initialization function at boot at specified priority,
 * and define functions to run at suspend/resume.
 *
 * @copydetails SYS_INIT
 * @param device_pm_ops Pointer to power management functions.
 * @param drv_name Name of this system device
 */

#ifdef CONFIG_DEVICE_POWER_MANAGEMENT
#define SYS_INIT_PM(drv_name, init_fn, device_pm_ops, level, prio) \
	DEVICE_INIT_PM(_SYS_NAME(init_fn), drv_name, init_fn, device_pm_ops, \
		NULL, NULL, level, prio)
#else
#define SYS_INIT_PM(drv_name, init_fn, device_pm_ops, level, prio) \
	DEVICE_INIT(_SYS_NAME(init_fn), "", init_fn, NULL, NULL, level, prio)
#endif

/**
 * @def SYS_DEVICE_DEFINE
 *
 * @brief Run an initialization function at boot at specified priority,
 * and define device PM control function.
 *
 * @copydetails SYS_INIT
 * @param pm_control_fn Pointer to device_pm_control function.
 * Can be empty function (device_pm_control_nop) if not implemented.
 * @param drv_name Name of this system device
 */
#define SYS_DEVICE_DEFINE(drv_name, init_fn, pm_control_fn, level, prio) \
	DEVICE_DEFINE(_SYS_NAME(init_fn), drv_name, init_fn, pm_control_fn, \
		      NULL, NULL, level, prio, NULL)


typedef struct stackreg
{
	unsigned int r4;
	unsigned int r5;
	unsigned int r6;
	unsigned int r7;
	unsigned int r8;
	unsigned int r9;
	unsigned int r10;
	unsigned int r11;
	unsigned int r12;
	unsigned int r13;
	unsigned int r14;
	unsigned int xpsr;


}STACKREG;

typedef struct w_tcb
{
	STACKREG strStackReg;
}W_TCB;
typedef void (*VFUNC)(void);
#ifdef __cplusplus
}
#endif

#endif /* _INIT_H_ */
