#include <stdio.h>
#include "rdtsc.h"

#define LOOP_MAX 100000000
#define RUNS 10

#define BENCHMARK_CODE(code)			\
	({					\
	uint64_t tsc_before, tsc_after;		\
	int i;					\
	RDTSC_START(tsc_before);		\
	for(i = 0; i < LOOP_MAX; ++i){		\
		code;				\
			}			\
	RDTSC_STOP(tsc_after);			\
	(tsc_after - tsc_before);		\
	})


static uint64_t test_nodep() {
	return BENCHMARK_CODE(
		asm volatile ("push %%rax;"
			      "mov %%rbx, %%rcx;"
			      "mov %%rbx, %%rcx;"
			      "pop %%rax;"
			      :
			      :
			      : "%rax", "%rcx")
			);
}

static uint64_t test_rodep() {
	return BENCHMARK_CODE(
		asm volatile ("push %%rax;"
			      "mov %%rsp, %%rcx;"
			      "mov %%rsp, %%rcx;"
			      "pop %%rax;"
			      :
			      :
			      : "%rax", "%rcx")
		);
}

static uint64_t test_rwdep() {
	return  BENCHMARK_CODE(
		asm volatile ("push %%rax;"
			      "mov %%rsp, %%rcx;"
			      "mov %%rcx, %%rsp;"
			      "pop %%rax;"
			      :
			      :
			      : "%rax", "%rcx")
		);
}

int main() {
	int j;
	uint64_t a = 0, b = 0, c = 0, t;

	for (j = 0; j < RUNS; ++j) {
		t = test_nodep();
		a += t / RUNS;
		printf("A: %llu\n", t);

		t = test_rodep();
		b += t / RUNS;
		printf("B: %llu\n", t);

		t = test_rwdep();
		c += t / RUNS;
		printf("C: %llu\n", t);
	}

	printf("no dependency / read dependency / read+write dependency: "
	       "%llu %llu %llu\n", a, b, c);

	return 0;
}
