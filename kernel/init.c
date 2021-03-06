/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Kernel initialization module
 *
 * This module contains routines that are used to initialize the kernel.
 */

#include <zephyr.h>
#include <offsets_short.h>
#include <kernel.h>
#include <misc/printk.h>
#include <misc/stack.h>
#include <drivers/rand32.h>
#include <sections.h>
#include <toolchain.h>
#include <kernel_structs.h>
#include <device.h>
#include <init.h>
#include <linker-defs.h>
#include <ksched.h>
#include <version.h>
#include <string.h>

/* kernel build timestamp items */

#define BUILD_TIMESTAMP "BUILD: " __DATE__ " " __TIME__

#ifdef CONFIG_BUILD_TIMESTAMP
const char * const build_timestamp = BUILD_TIMESTAMP;
#endif

/* boot banner items */

#define BOOT_BANNER "BOOTING ZEPHYR OS v" KERNEL_VERSION_STRING

#if !defined(CONFIG_BOOT_BANNER)
#define PRINT_BOOT_BANNER() do { } while (0)
#elif !defined(CONFIG_BUILD_TIMESTAMP)
#define PRINT_BOOT_BANNER() printk("***** " BOOT_BANNER " *****\n")
#else
#define PRINT_BOOT_BANNER() \
	printk("***** " BOOT_BANNER " - %s *****\n", build_timestamp)
#endif

/* boot time measurement items */

#ifdef CONFIG_BOOT_TIME_MEASUREMENT
uint64_t __noinit __start_tsc; /* timestamp when kernel starts */
uint64_t __noinit __main_tsc;  /* timestamp when main task starts */
uint64_t __noinit __idle_tsc;  /* timestamp when CPU goes idle */
#endif

/* init/main and idle threads */

#define IDLE_STACK_SIZE CONFIG_IDLE_STACK_SIZE

#if CONFIG_MAIN_STACK_SIZE & (STACK_ALIGN - 1)
    #error "MAIN_STACK_SIZE must be a multiple of the stack alignment"
#endif

#if IDLE_STACK_SIZE & (STACK_ALIGN - 1)
    #error "IDLE_STACK_SIZE must be a multiple of the stack alignment"
#endif

/* Some projects may specify their main thread and parameters in the
 * MDEF file. In this case, we need to use the stack size specified there
 * and not in Kconfig
 */
#if defined(MDEF_MAIN_STACK_SIZE) && \
		(MDEF_MAIN_STACK_SIZE > CONFIG_MAIN_STACK_SIZE)
#define MAIN_STACK_SIZE MDEF_MAIN_STACK_SIZE
#else
#define MAIN_STACK_SIZE CONFIG_MAIN_STACK_SIZE
#endif

char __noinit __stack _main_stack[MAIN_STACK_SIZE];
char __noinit __stack _idle_stack[IDLE_STACK_SIZE];

k_tid_t const _main_thread = (k_tid_t)_main_stack;
k_tid_t const _idle_thread = (k_tid_t)_idle_stack;

/*
 * storage space for the interrupt stack
 *
 * Note: This area is used as the system stack during kernel initialization,
 * since the kernel hasn't yet set up its own stack areas. The dual purposing
 * of this area is safe since interrupts are disabled until the kernel context
 * switches to the init thread.
 */
#if CONFIG_ISR_STACK_SIZE & (STACK_ALIGN - 1)
    #error "ISR_STACK_SIZE must be a multiple of the stack alignment"
#endif
char __noinit __stack _interrupt_stack[CONFIG_ISR_STACK_SIZE];

#ifdef CONFIG_SYS_CLOCK_EXISTS
	#include <misc/dlist.h>
	#define initialize_timeouts() do { \
		sys_dlist_init(&_timeout_q); \
	} while ((0))
#else
	#define initialize_timeouts() do { } while ((0))
#endif

extern void idle(void *unused1, void *unused2, void *unused3);

void k_call_stacks_analyze(void)
{
#if defined(CONFIG_INIT_STACKS) && defined(CONFIG_PRINTK)
	extern char sys_work_q_stack[CONFIG_SYSTEM_WORKQUEUE_STACK_SIZE];
#if defined(CONFIG_ARC)
	extern char _firq_stack[CONFIG_FIRQ_STACK_SIZE];
#endif /* CONFIG_ARC */

	printk("Kernel stacks:\n");
	stack_analyze("main     ", _main_stack, sizeof(_main_stack));
	stack_analyze("idle     ", _idle_stack, sizeof(_idle_stack));
#if defined(CONFIG_ARC)
	stack_analyze("firq     ", _firq_stack, sizeof(_firq_stack));
#endif /* CONFIG_ARC */
	stack_analyze("interrupt", _interrupt_stack,
		      sizeof(_interrupt_stack));
	stack_analyze("workqueue", sys_work_q_stack,
		      sizeof(sys_work_q_stack));

#endif /* CONFIG_INIT_STACKS && CONFIG_PRINTK */
}

/**
 *
 * @brief Clear BSS
 *
 * This routine clears the BSS region, so all bytes are 0.
 *
 * @return N/A
 */
void _bss_zero(void)
{
	memset(&__bss_start, 0,
		 ((uint32_t) &__bss_end - (uint32_t) &__bss_start));
}


unsigned int  curr_task=0;     // 当前执行任务
unsigned int *pcurr_task = &curr_task;
unsigned int  next_task=1;     // 下一个任务
unsigned int *pnext_task = &next_task;
unsigned int  task0_stack[1024];
unsigned int  task1_stack[1024];
unsigned int  PSP_array[4];
unsigned int *pPSP_arrary = PSP_array;

unsigned char task0_handle=1;
unsigned char task1_handle=1;
unsigned int dumy[20];
unsigned int adr1[20];
unsigned int adr2[20];
void task_switch(unsigned int, unsigned int);
void task1(void*);

extern W_TCB *gpstrTask1Tcb;
extern W_TCB *gpstrTask2Tcb;
extern W_TCB *gpstrTask3Tcb;
W_TCB *gpstrCurrTcb;

void switch_to_target_task(W_TCB* tar)
{
	STACKREG *p = &gpstrCurrTcb->strStackReg;
	gpstrCurrTcb = tar;

	task_switch(p, &tar->strStackReg);
}
void task0(void* p) 
{ 

    while(1)
    {
	printk("%s line %d. handle0 = %d. handle1 = %d.\n", __func__, __LINE__, task0_handle, task1_handle);
	/*
	 *if(task0_handle==1)
	 *{
	 *        printk("%s line %d. handle0 = %d. handle1 = %d.\n", __func__, __LINE__, task0_handle, task1_handle);
	 *        task0_handle=0;
	 *        printk("%s line %d. handle0 = %d. handle1 = %d.\n", __func__, __LINE__, task0_handle, task1_handle);
	 *        task1_handle=1;
	 *        printk("%s line %d. handle0 = %d. handle1 = %d.\n", __func__, __LINE__, task0_handle, task1_handle);
	 *}
	 */
	switch_to_target_task(gpstrTask2Tcb);
    }
}

void task1(void* p)
{
	while(1)
        {
		printk("%s line %d. handle0 = %d. handle1 = %d.\n", __func__, __LINE__, task0_handle, task1_handle);
		/*
		 *if(task1_handle==1)
		 *{
		 *        printk("%s line %d. handle0 = %d. handle1 = %d.\n", __func__, __LINE__, task0_handle, task1_handle);
		 *        task1_handle=0;
		 *        printk("%s line %d. handle0 = %d. handle1 = %d.\n", __func__, __LINE__, task0_handle, task1_handle);
		 *        task0_handle=1;
		 *        printk("%s line %d. handle0 = %d. handle1 = %d.\n", __func__, __LINE__, task0_handle, task1_handle);
		 *}
		 */
		switch_to_target_task(gpstrTask3Tcb);
	}
}

void task2(void* p)
{
	while(1)
        {
		printk("%s line %d. handle0 = %d. handle1 = %d.\n", __func__, __LINE__, task0_handle, task1_handle);
		/*
		 *if(task1_handle==1)
		 *{
		 *        printk("%s line %d. handle0 = %d. handle1 = %d.\n", __func__, __LINE__, task0_handle, task1_handle);
		 *        task1_handle=0;
		 *        printk("%s line %d. handle0 = %d. handle1 = %d.\n", __func__, __LINE__, task0_handle, task1_handle);
		 *        task0_handle=1;
		 *        printk("%s line %d. handle0 = %d. handle1 = %d.\n", __func__, __LINE__, task0_handle, task1_handle);
		 *}
		 */
		switch_to_target_task(gpstrTask1Tcb);
	}
}


#ifdef CONFIG_XIP
/**
 *
 * @brief Copy the data section from ROM to RAM
 *
 * This routine copies the data section from ROM to RAM.
 *
 * @return N/A
 */
void _data_copy(void)
{
	memcpy(&__data_ram_start, &__data_rom_start,
		 ((uint32_t) &__data_ram_end - (uint32_t) &__data_ram_start));
}
#endif

void test_printk(unsigned int r0, unsigned int r1, unsigned r2, unsigned int r3)
{
	printk("%s line %d.r0 = 0x%08x, r1 = 0x%08x, r2 = 0x%08x, r3 = 0x%08x. pcurr_task = %p, pnext_task = %p, PSP_arrary=%p \n",
			__func__, __LINE__, r0, r1, r2, r3, pcurr_task, pnext_task, PSP_array);
}

extern void TriggerPendSV();
void SysTick_Handler(void)
{
	return;
	if(curr_task==0)
	        next_task=1;
        else if(curr_task == 1)
	        next_task=0;
	else
		return;
	/*printk("%s line %d. next_task = %d.\n", __func__, __LINE__, next_task);*/
	TriggerPendSV();
	printk("%s line %d. next_task = %d.\n", __func__, __LINE__, next_task);
}

#define TASKSTACK  1024
W_TCB *gpstrTask1Tcb;
W_TCB *gpstrTask2Tcb;
W_TCB *gpstrTask3Tcb;
unsigned int gauiTask1Stack[TASKSTACK];
unsigned int gauiTask2Stack[TASKSTACK];
unsigned int gauiTask3Stack[TASKSTACK];
unsigned int * TEST_GetTaskInitSp(unsigned char task)
{
	if(1 == task)
	{
		return gauiTask1Stack + TASKSTACK;
	}
	else if(2 == task)
	{
		return gauiTask2Stack + TASKSTACK;
	}
	else
		return gauiTask3Stack + TASKSTACK;
}
  
W_TCB *WLX_TaskInit(VFUNC vfFuncPointer, unsigned int *puiTaskStack)
{
	W_TCB *pstrTcb;
	STACKREG *pstrStackReg;

	pstrTcb = (W_TCB*)((unsigned int)puiTaskStack - sizeof(W_TCB));

	pstrStackReg = &pstrTcb->strStackReg;
	pstrStackReg->r4 = 0;
	pstrStackReg->r5 = 0;
	pstrStackReg->r6 = 0;
	pstrStackReg->r7 = 0;
	pstrStackReg->r8 = 0;
	pstrStackReg->r9 = 0;
	pstrStackReg->r10 = 0;
	pstrStackReg->r11 = 0;
	pstrStackReg->r12 = 0;
	pstrStackReg->r13 = (unsigned int)pstrTcb;
	pstrStackReg->r14 = (unsigned int)vfFuncPointer;
	pstrStackReg->xpsr = 0x01000000;

	return pstrStackReg;
}
/**
 *
 * @brief Mainline for kernel's background task
 *
 * This routine completes kernel initialization by invoking the remaining
 * init functions, then invokes application's main() routine.
 *
 * @return N/A
 */
#define HW32_REG(addr) (*(volatile unsigned int*)addr)
static void _main(void *unused1, void *unused2, void *unused3)
{
	ARG_UNUSED(unused1);
	ARG_UNUSED(unused2);
	ARG_UNUSED(unused3);
	pPSP_arrary = PSP_array;
	pnext_task = &next_task;
	pcurr_task = &curr_task;

	_sys_device_do_config_level(_SYS_INIT_LEVEL_POST_KERNEL);

	/* These 3 are deprecated */
	_sys_device_do_config_level(_SYS_INIT_LEVEL_SECONDARY);
	_sys_device_do_config_level(_SYS_INIT_LEVEL_NANOKERNEL);
	_sys_device_do_config_level(_SYS_INIT_LEVEL_MICROKERNEL);

	/* Final init level before app starts */
	_sys_device_do_config_level(_SYS_INIT_LEVEL_APPLICATION);

#ifdef CONFIG_CPLUSPLUS
	/* Process the .ctors and .init_array sections */
	extern void __do_global_ctors_aux(void);
	extern void __do_init_array_aux(void);
	__do_global_ctors_aux();
	__do_init_array_aux();
#endif

	_init_static_threads();

#ifdef CONFIG_BOOT_TIME_MEASUREMENT
	/* record timestamp for kernel's _main() function */
	extern uint64_t __main_tsc;

	__main_tsc = _tsc_read();
#endif
	PSP_array[0] = ((unsigned int) task0_stack) + (sizeof task0_stack) - 16*4;
	HW32_REG((PSP_array[0] + (14<<2))) = (unsigned int) task0;
	HW32_REG((PSP_array[0] + (15<<2))) = 0x01000000;

	PSP_array[1] = ((unsigned int) task1_stack) + (sizeof task1_stack) - 16*4;
	HW32_REG((PSP_array[1] + (14<<2))) = (unsigned long) task1;
	HW32_REG((PSP_array[1] + (15<<2))) = 0x01000000;
	printk("psp1 = 0x%08x, psp[2] = 0x%08x.\n", PSP_array[0], PSP_array[1]);

	curr_task = 0;

	extern void __set_PSP(unsigned int);
	/*__set_PSP((PSP_array[curr_task] + 16*4));*/

	extern void SetPendSVPro();
	SetPendSVPro();
	/*set_control(3);*/
	 __asm(" isb     ");

/*
 *        adr1[9] = (unsigned int) &task0_stack[1000];
 *        adr1[10] = (unsigned int) task0;
 *        adr1[11] = 0x01000000;
 *
 *    	adr2[9] = (unsigned int) &task1_stack[1000];
 *    	adr2[10] = (unsigned int) task1;
 *    	adr2[11] = 0x01000000;
 *
 *        task_switch(dumy, adr1);
 */

	gpstrTask1Tcb = WLX_TaskInit(task0, TEST_GetTaskInitSp(1));
	gpstrTask2Tcb = WLX_TaskInit(task1, TEST_GetTaskInitSp(2));
	gpstrTask3Tcb = WLX_TaskInit(task2, TEST_GetTaskInitSp(3));

	gpstrCurrTcb = gpstrTask1Tcb;
	switch2task(&gpstrTask1Tcb->strStackReg);
	while(1);
	extern void main(void);

	/* If we're going to load the MDEF main() in this context, we need
	 * to now set the priority to be what was specified in the MDEF file
	 */
#if defined(MDEF_MAIN_THREAD_PRIORITY) && \
		(MDEF_MAIN_THREAD_PRIORITY != CONFIG_MAIN_THREAD_PRIORITY)
	k_thread_priority_set(_main_thread, MDEF_MAIN_THREAD_PRIORITY);
#endif
	main();

	/* Terminate thread normally since it has no more work to do */
	_main_thread->base.thread_state &= ~K_ESSENTIAL;
}

void __weak main(void)
{
	/* NOP default main() if the application does not provide one. */
}

/**
 *
 * @brief Initializes kernel data structures
 *
 * This routine initializes various kernel data structures, including
 * the init and idle threads and any architecture-specific initialization.
 *
 * Note that all fields of "_kernel" are set to zero on entry, which may
 * be all the initialization many of them require.
 *
 * @return N/A
 */
static void prepare_multithreading(struct k_thread *dummy_thread)
{
#ifdef CONFIG_ARCH_HAS_CUSTOM_SWAP_TO_MAIN
	ARG_UNUSED(dummy_thread);
#else
	/*
	 * Initialize the current execution thread to permit a level of
	 * debugging output if an exception should happen during kernel
	 * initialization.  However, don't waste effort initializing the
	 * fields of the dummy thread beyond those needed to identify it as a
	 * dummy thread.
	 */

	_current = dummy_thread;

	dummy_thread->base.thread_state = K_ESSENTIAL;
#endif

	/* _kernel.ready_q is all zeroes */


	/*
	 * The interrupt library needs to be initialized early since a series
	 * of handlers are installed into the interrupt table to catch
	 * spurious interrupts. This must be performed before other kernel
	 * subsystems install bonafide handlers, or before hardware device
	 * drivers are initialized.
	 */

	_IntLibInit();

	/* ready the init/main and idle threads */

	for (int ii = 0; ii < K_NUM_PRIORITIES; ii++) {
		sys_dlist_init(&_ready_q.q[ii]);
	}

	/*
	 * prime the cache with the main thread since:
	 *
	 * - the cache can never be NULL
	 * - the main thread will be the one to run first
	 * - no other thread is initialized yet and thus their priority fields
	 *   contain garbage, which would prevent the cache loading algorithm
	 *   to work as intended
	 */
	_ready_q.cache = _main_thread;

	_new_thread(_main_stack, MAIN_STACK_SIZE,
		    _main, NULL, NULL, NULL,
		    CONFIG_MAIN_THREAD_PRIORITY, K_ESSENTIAL);
	_mark_thread_as_started(_main_thread);
	_add_thread_to_ready_q(_main_thread);

#ifdef CONFIG_MULTITHREADING
	_new_thread(_idle_stack, IDLE_STACK_SIZE,
		    idle, NULL, NULL, NULL,
		    K_LOWEST_THREAD_PRIO, K_ESSENTIAL);
	_mark_thread_as_started(_idle_thread);
	_add_thread_to_ready_q(_idle_thread);
#endif

	initialize_timeouts();

	/* perform any architecture-specific initialization */

	nanoArchInit();
}

static void switch_to_main_thread(void)
{
#ifdef CONFIG_ARCH_HAS_CUSTOM_SWAP_TO_MAIN
	_arch_switch_to_main_thread(_main_stack, MAIN_STACK_SIZE, _main);
#else
	/*
	 * Context switch to main task (entry function is _main()): the
	 * current fake thread is not on a wait queue or ready queue, so it
	 * will never be rescheduled in.
	 */

	_Swap(irq_lock());
#endif
}

#ifdef CONFIG_STACK_CANARIES
/**
 *
 * @brief Initialize the kernel's stack canary
 *
 * This macro initializes the kernel's stack canary global variable,
 * __stack_chk_guard, with a random value.
 *
 * INTERNAL
 * Depending upon the compiler, modifying __stack_chk_guard directly at runtime
 * may generate a build error.  In-line assembly is used as a workaround.
 */

extern void *__stack_chk_guard;

#if defined(CONFIG_X86)
#define _MOVE_INSTR "movl %1, %0"
#define _MOVE_MEM "=m"
#elif defined(CONFIG_ARM)
#define _MOVE_INSTR "str %1, %0"
#define _MOVE_MEM "=m"
#elif defined(CONFIG_ARC)
#define _MOVE_INSTR "st %1, %0"
#define _MOVE_MEM "=m"
#elif defined(CONFIG_RISCV32)
#define _MOVE_INSTR "sw %1, 0x00(%0)"
#define _MOVE_MEM "=r"
#else
#error "Unknown Architecture type"
#endif /* CONFIG_X86 */

#define STACK_CANARY_INIT()                                \
	do {                                               \
		register void *tmp;                        \
		tmp = (void *)sys_rand32_get();            \
		__asm__ volatile(_MOVE_INSTR ";\n\t" \
				 : _MOVE_MEM(__stack_chk_guard) \
				 : "r"(tmp));              \
	} while (0)

#else /* !CONFIG_STACK_CANARIES */
#define STACK_CANARY_INIT()
#endif /* CONFIG_STACK_CANARIES */

/**
 *
 * @brief Initialize kernel
 *
 * This routine is invoked when the system is ready to run C code. The
 * processor must be running in 32-bit mode, and the BSS must have been
 * cleared/zeroed.
 *
 * @return Does not return
 */
FUNC_NORETURN void _Cstart(void)
{
#ifdef CONFIG_ARCH_HAS_CUSTOM_SWAP_TO_MAIN
	void *dummy_thread = NULL;
#else
	/* floating point is NOT used during kernel init */

	char __stack dummy_stack[_K_THREAD_NO_FLOAT_SIZEOF];
	void *dummy_thread = dummy_stack;
#endif

	/*
	 * Initialize kernel data structures. This step includes
	 * initializing the interrupt subsystem, which must be performed
	 * before the hardware initialization phase.
	 */

	prepare_multithreading(dummy_thread);

	/* Deprecated */
	_sys_device_do_config_level(_SYS_INIT_LEVEL_PRIMARY);

	/* perform basic hardware initialization */
	_sys_device_do_config_level(_SYS_INIT_LEVEL_PRE_KERNEL_1);
	_sys_device_do_config_level(_SYS_INIT_LEVEL_PRE_KERNEL_2);

	/* initialize stack canaries */

	STACK_CANARY_INIT();

	/* display boot banner */

	PRINT_BOOT_BANNER();

	switch_to_main_thread();

	/*
	 * Compiler can't tell that the above routines won't return and issues
	 * a warning unless we explicitly tell it that control never gets this
	 * far.
	 */

	CODE_UNREACHABLE;
}
