#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <sys/time.h>
#include <sys/resource.h>

#include "rdtsc.h"
#include "bit_rotation.h"
#include "stddev.h"


#define MAX_RETRIES 10

#define ROUNDS 10000


#define ACCURACY 5.

static int do_test(uint8_t input[VECTORS*8], uint64_t output[VECTORS], int noop_min) {
	struct {
		int d0;
		int d1;
	} rounds[ROUNDS];

	/* The hot section */
	struct rusage rus_before;
	getrusage(RUSAGE_SELF, &rus_before);

	memset(rounds, 0, sizeof(rounds));

	int j;
	for (j =0; j < 100; j++) {
		bit_rotate_std(input, output);
		bit_rotate(input, output);
	}

	struct stddev std = INIT_STDDEV;
	struct stddev opt = INIT_STDDEV;

	int i;
	for (i = 0; i < ROUNDS; i++) {
		uint64_t c0, c1;
		RDTSC_START(c0);
		bit_rotate_std(input, output);
		RDTSC_STOP(c1);
		stddev_add(&std, c1 - c0);

		RDTSC_START(c0);
		bit_rotate(input, output);
		RDTSC_STOP(c1);
		stddev_add(&opt, c1 - c0);
	}

	struct rusage rus_after;
	getrusage(RUSAGE_SELF, &rus_after);
	if ((rus_after.ru_minflt - rus_before.ru_minflt) ||
	    (rus_after.ru_majflt - rus_before.ru_majflt)) {
		printf("page fault! \n");
	}

	if ((rus_after.ru_nswap - rus_before.ru_nswap) ||
	    (rus_after.ru_msgsnd - rus_before.ru_msgsnd) ||
	    (rus_after.ru_msgrcv - rus_before.ru_msgrcv) ||
	    (rus_after.ru_nsignals - rus_before.ru_nsignals)) {
		printf("signals stuff\n");
		return 1;
	}

	if ((rus_after.ru_nvcsw - rus_before.ru_nvcsw) ||
	    (rus_after.ru_nivcsw - rus_before.ru_nivcsw)) {
		printf("ctx switch\n");
		return 1;
	}

	double avg_opt, avg_std, dev_opt, dev_std;
	stddev_get(&std, NULL, NULL, NULL, &avg_std, &dev_std);
	stddev_get(&opt, NULL, NULL, NULL, &avg_opt, &dev_opt);
	if ((dev_opt > ACCURACY * avg_opt) ||
	    (dev_std > ACCURACY * avg_std)) {
		printf("accuracy\n");
		return 0;
	}


	int min, max, a, b;
	double avg, dev;
	stddev_get(&std, NULL, &min, &max, &avg, &dev);
	a = min-noop_min;
	printf("std       min/max=%3i/%3i   avg=%7.3f  +/- %.3f\n",
	       a, max-noop_min, avg-noop_min, dev);
	stddev_get(&opt, NULL, &min, &max, &avg, &dev);
	b = min-noop_min;
	printf("opt       min/max=%3i/%3i   avg=%7.3f  +/- %.3f\n",
	       b, max-noop_min, avg-noop_min, dev);
	printf("gain: %.3f%%\n",
	       (1. - (double)b/a)*100.);
	return 1;
}



int main() {
	uint8_t input[8*VECTORS];
	uint64_t output0[VECTORS];
	uint64_t output1[VECTORS];

	/* Sanity check */
	memset(input, 0, sizeof(input));
	input[01] = 0xBE;
	input[32] = 0xAD;
	input[77] = 0xDE;

	bit_rotate(input, output0);
	bit_rotate((uint8_t *)output0, output1);
	assert(memcmp(input, output1, sizeof(input)) == 0);

	/* Do both algos give the same answer? */
	bit_rotate(input, output0);
	bit_rotate_std(input, output1);
	assert(memcmp(output0, output1, sizeof(output0)) == 0);

	/* Warm up! */
	printf("[.] warming up!\n");
	int i;
	for (i = 0; i < 10000; i++) {
		bit_rotate_std(input, output0);
		bit_rotate(input, output0);
	}

	int retry;
	for (retry = 0; retry < MAX_RETRIES; retry += 1) {
		do_test(input, output0, 37);
//		if (do_test(input, output0, 37) != 0) // Don't hardcode noop
			//break;
	}
	if (retry == MAX_RETRIES) {
		printf("FAILED!\n");
		exit(1);
	}

	return 0;
}
