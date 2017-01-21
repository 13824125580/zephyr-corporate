/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <misc/printk.h>
#define STACK_SIZE 1024

static char __noinit __stack stack0[STACK_SIZE];
static char __noinit __stack stack1[STACK_SIZE];
static char __noinit __stack stack2[STACK_SIZE];
static char __noinit __stack stack3[STACK_SIZE];

static void task0(void *p1, void *p2, void *p3)
{
	/*last_prio = k_thread_priority_get(k_current_get());*/
	while(1)
		printk("%s line %d.\n", __func__, __LINE__);
}

static void task1(void *p1, void *p2, void *p3)
{
	/*last_prio = k_thread_priority_get(k_current_get());*/
	while(1)
		printk("%s line %d.\n", __func__, __LINE__);
}

static void task2(void *p1, void *p2, void *p3)
{
	/*last_prio = k_thread_priority_get(k_current_get());*/
	while(1)
		printk("%s line %d.\n", __func__, __LINE__);
}

static void task3(void *p1, void *p2, void *p3)
{
	/*last_prio = k_thread_priority_get(k_current_get());*/
	while(1)
	{
		printk("%s line %d.###################################################################.\n", __func__, __LINE__);
		/*k_yield();*/
		k_sleep(100);
	}
}
void main(void)
{
	k_sched_time_slice_set(100, 0);
	k_thread_spawn(stack0, STACK_SIZE, task0, NULL, NULL, NULL, 1, 0, 0);
	k_thread_spawn(stack1, STACK_SIZE, task1, NULL, NULL, NULL, 1, 0, 0);
	k_thread_spawn(stack2, STACK_SIZE, task2, NULL, NULL, NULL, 1, 0, 0);
	k_thread_spawn(stack3, STACK_SIZE, task3, NULL, NULL, NULL, -1, 0, 0);
	/*
	 *while(1)
	 *        printk("Hello World! %s\n", CONFIG_ARCH);
	 */

	return;
}
