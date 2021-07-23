
#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/debug.h>
#include "pmu.h"

int testa(int a1, int a2){
	return a1 + a2;
}

int pmu_test(void){

	unsigned int s_pmu_counter, e_pmu_counter;

	pmu_start();
	pmu_enable_counter();
	pmu_reset_counter();

	int val = 0;

	s_pmu_counter = pmu_read_counter();

	val += testa(0x100, 0x500);
	val += testa(0x200, 0x500);

	e_pmu_counter = pmu_read_counter();

	ksceDebugPrintf("0x100 + 0x500 + 0x200 + 0x500 = 0x%X\n", val);
	ksceDebugPrintf("pmu : 0x%X\n", e_pmu_counter - s_pmu_counter);

	return 0;
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	pmu_test();

	return SCE_KERNEL_START_SUCCESS;
}
