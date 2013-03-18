#include <stdio.h>
#include "rdtsc.h"

#define LOOP_MAX 100000000
#define RUNS 10



int main() {
	int i,j = 0;
	uint64_t tsc_before, tsc_after;
	uint64_t a = 0, b = 0, c = 0;


	for(j = 0; j < RUNS; ++j){

		RDTSC_START(tsc_before);
		for(i = 0; i < LOOP_MAX; ++i){
			asm volatile ("push %%rax;"
				      "mov %%rbx, %%rcx;"
				      "mov %%rbx, %%rcx;"
				      "pop %%rax;"
				      :
				      :
				      : "%rax", "%rcx");
		}
		RDTSC_STOP(tsc_after);
		a += (tsc_after - tsc_before) / RUNS;
		printf("A: %llu\n", tsc_after - tsc_before);

		RDTSC_START(tsc_before);
		for(i = 0; i < LOOP_MAX; ++i){
			asm volatile ("push %%rax;"
				      "mov %%rsp, %%rcx;"
				      "mov %%rsp, %%rcx;"
				      "pop %%rax;"
				      :
				      :
				      : "%rax", "%rcx");
		}
		RDTSC_STOP(tsc_after);

		printf("B: %llu\n", tsc_after - tsc_before);
		b += (tsc_after - tsc_before) / RUNS;

		RDTSC_START(tsc_before);
		for(i = 0; i < LOOP_MAX; ++i){
			asm volatile ("push %%rax;"
				      "mov %%rsp, %%rcx;"
				      "mov %%rcx, %%rsp;"
				      "pop %%rax;"
				      :
				      :
				      : "%rax", "%rcx");
		}
		RDTSC_STOP(tsc_after);

		printf("C: %llu\n", tsc_after - tsc_before);
		c += (tsc_after - tsc_before) / RUNS;
	}

	printf("no dependency / read dependency / read+write dependency: "
	       "%llu %llu %llu\n", a, b, c);

	return 0;
}
