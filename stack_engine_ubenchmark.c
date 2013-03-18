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
	struct {
		uint64_t (*fun)();
		char *name;
		uint64_t t;
	} tests[] = {
		{test_nodep, "no dependency", 0},
		{test_rodep, "read dependency", 0},
		{test_rwdep, "read+write dependency", 0},
		{NULL, NULL, 0}
	};

	printf("[*] Warming up the code\n");
	int test_no, run_no;
	for (run_no = 0; run_no < 3; run_no += 1) {
		for (test_no = 0; tests[test_no].fun; test_no += 1) {
			tests[test_no].fun();
		}
	}

	printf("[*] Running the benchmarks\n");
	for (run_no = 0; run_no < RUNS; run_no += 1) {
		for (test_no = 0; tests[test_no].fun; test_no += 1) {
			uint64_t t = tests[test_no].fun();
			tests[test_no].t += t;
			printf("[ ] #%i %llu\n", test_no, t);
		}
	}

	for (test_no = 0; tests[test_no].fun; test_no += 1) {
		printf("[=] %-25s %llu\n",
		       tests[test_no].name,
		       tests[test_no].t
			);
	}

	return 0;
}
