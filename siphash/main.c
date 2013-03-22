#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include <sys/time.h>
#include <sys/resource.h>

#include "rdtsc.h"
#include "stddev.h"

uint64_t siphash24(const void *src, unsigned long src_sz, const char key[16]);
uint64_t siphash24_asm(const void *src, unsigned long src_sz, const char key[16]);


#define ROUNDS 60

struct {
	int noop;
	int d0;
	int d1;
} rounds[ROUNDS];

int main() {
	int i;
	char key[16] = {0};
	uint64_t out1, out2;

	char *vector = "abcdefgh";
	int vector_sz = strlen(vector);

//	char *vector2 = "hgfedcba";

	/* sanity check */
	out1 = siphash24(vector, vector_sz, key);
	out2 = siphash24_asm(vector, vector_sz, key);

	printf("%016llx\n", out1);
	printf("%016llx\n", out2);
	assert(out1 == out2);

	/* Use 100% CPU for some time to make sure the processor has
	   time to turn off energy saving. */
	for (i = 0; i < 20000000; i++) {
		out1 = siphash24(vector, vector_sz, key);
		out1 = siphash24_asm(vector, vector_sz, key);
	}

	/* The hot section */
	struct rusage rus_before;
	getrusage(RUSAGE_SELF, &rus_before);

	for (i = 0; i < ROUNDS; i++) {
		rounds[i].d0 = 0;
		rounds[i].d1 = 0;
	}

	out1 = siphash24(vector, vector_sz, key);
	out1 = siphash24_asm(vector, vector_sz, key);


	struct stddev noop = INIT_STDDEV;
	struct stddev native = INIT_STDDEV;
	struct stddev sse = INIT_STDDEV;

	for (i = 0; i < ROUNDS; i++) {
		uint64_t c0, c1;
		RDTSC_START(c0);
		RDTSC_STOP(c1);
		stddev_add(&noop, c1 - c0);

		RDTSC_START(c0);
		out1 = siphash24(vector, vector_sz, key);
		RDTSC_STOP(c1);
		stddev_add(&native, c1 - c0);

		RDTSC_START(c0);
		out1 = siphash24_asm(vector, vector_sz, key);
		RDTSC_STOP(c1);
		stddev_add(&sse, c1 - c0);
	}

	struct rusage rus_after;
	i = getrusage(RUSAGE_SELF, &rus_after);
	i = rus_after.ru_minflt - rus_before.ru_minflt;
	if (i) printf("minflt=%i\n", i);
	i = rus_after.ru_majflt - rus_before.ru_majflt;
	if (i) printf("majflt=%i\n", i);
	i = rus_after.ru_nswap - rus_before.ru_nswap;
	if (i) printf("nswap=%i\n", i);
	i = rus_after.ru_msgsnd - rus_before.ru_msgsnd;
	if (i) printf("msgsnd=%i\n", i);
	i = rus_after.ru_msgrcv - rus_before.ru_msgrcv;
	if (i) printf("msgrcv=%i\n", i);
	i = rus_after.ru_nsignals - rus_before.ru_nsignals;
	if (i) printf("nsignals=%i\n", i);
	i = rus_after.ru_nvcsw - rus_before.ru_nvcsw;
	if (i) printf("nvcsw=%i\n", i);
	i = rus_after.ru_nivcsw - rus_before.ru_nivcsw;
	if (i) printf("nivcsw=%i\n", i);

	printf("reporting in cycles:\n");

	int noop_min, min, max;
	double avg, dev, a, b;
	stddev_get(&noop, NULL, &noop_min, &max, &avg, &dev);
	printf("noop round   min/max=%3i/%3i   avg=%7.3f  +/- %.3f\n",
	       noop_min, max, avg, dev);
	stddev_get(&native, NULL, &min, &max, &avg, &dev);
	printf("native       min/max=%3i/%3i   avg=%7.3f  +/- %.3f\n",
	       min-noop_min, max-noop_min, avg-noop_min, dev);
	a = min-noop_min;
	stddev_get(&sse, NULL, &min, &max, &avg, &dev);
	printf("sse          min/max=%3i/%3i   avg=%7.3f  +/- %.3f\n",
	       min-noop_min, max-noop_min, avg-noop_min, dev);
	b = min-noop_min;
	printf("gain: %.3f%%\n", (1. - b/a)*100.);
	return 0;
}


