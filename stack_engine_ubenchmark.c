#include <stdio.h>
#include "rdtsc.h"

#define LOOP_MAX 100000000
#define RUNS 10


int main(void) {
	int i,j = 0;
	int tsc_before, tsc_after;
	int a, b, c;



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
		printf("A: %i\n", tsc_after - tsc_before);

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

		printf("B: %i\n", tsc_after - tsc_before);
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

		printf("C: %i\n", tsc_after - tsc_before);
		c += (tsc_after - tsc_before) / RUNS;
	}

	printf("no dependency / read dependency / read+write dependency: %i %i %i\n", a, b, c);

}
