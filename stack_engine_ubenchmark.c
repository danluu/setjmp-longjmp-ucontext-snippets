#include <stdio.h>
#include "rdtsc.h"
#include "stddev.h"

#define LOOP_MAX 100000000
#define WARMUP_RUNS 2
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
		struct stddev stddev;
	} tests[] = {
		{test_nodep, "no dependency", INIT_STDDEV},
		{test_rodep, "read dependency", INIT_STDDEV},
		{test_rwdep, "read+write dependency", INIT_STDDEV},
		{NULL, NULL, INIT_STDDEV}
	};

	printf("[*] Warming up the code\n");
	int test_no, run_no;
	for (run_no = 0; run_no < WARMUP_RUNS; run_no += 1)
		for (test_no = 0; tests[test_no].fun; test_no += 1)
			tests[test_no].fun();

	printf("[*] Running the benchmarks\n");
	for (run_no = 0; run_no < RUNS; run_no += 1) {
		printf("[ ] run %i/%i\n", run_no+1, RUNS);
		for (test_no = 0; tests[test_no].fun; test_no += 1) {
			uint64_t t = tests[test_no].fun();
			stddev_add(&tests[test_no].stddev, t);
		}
	}

	printf("[*] For %i iterations:\n", LOOP_MAX);
	for (test_no = 0; tests[test_no].fun; test_no += 1) {
		double avg, dev;
		stddev_get(&tests[test_no].stddev, NULL, &avg, &dev);
		printf("[=] %-25s cycles avg=%.3f dev=%.3f\n",
		       tests[test_no].name, avg, dev);
	}
	return 0;
}
