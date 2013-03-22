#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include <sys/time.h>
#include <sys/resource.h>

#include "rdtsc.h"
#include "stddev.h"

void siphash24_half_round_asm(const uint64_t in[4], uint64_t out[4]);


void half_round(const uint64_t in[4], uint64_t out[4]);

#define ROUNDS 30

struct {
	int noop;
	int d0;
	int d1;
} rounds[ROUNDS];

int main() {
	int i;
	uint64_t in[4] = {1, 2, 3, 4};
	uint64_t out1[4], out2[4];

	/* sanity check */
	half_round(in, out1);
	siphash24_half_round_asm(in, out2);

	printf("lo=%016llx hi=%016llx\n", out1[0], out1[1]);
	printf("lo=%016llx hi=%016llx\n", out1[2], out1[3]);

	printf("lo=%016llx hi=%016llx\n", out2[0], out2[1]);
	printf("lo=%016llx hi=%016llx\n", out2[2], out2[3]);
	for (i = 0; i < 4; i++) {
		assert(out1[i] == out2[i]);
	}


	/* Use 100% CPU for some time to make sure the processor has
	   time to turn off energy saving. */
	for (i = 0; i < 50000000; i++) {
		half_round(in, out1);
		siphash24_half_round_asm(in, out1);
	}

	/* The hot section */
	struct rusage rus_before;
	getrusage(RUSAGE_SELF, &rus_before);

	for (i = 0; i < ROUNDS; i++) {
		rounds[i].d0 = 0;
		rounds[i].d1 = 0;
	}

	siphash24_half_round_asm(in, out1);
	half_round(in, out1);

	struct stddev noop = INIT_STDDEV;
	struct stddev native = INIT_STDDEV;
	struct stddev sse = INIT_STDDEV;

	for (i = 0; i < ROUNDS; i++) {
		uint64_t c0, c1;
		RDTSC_START(c0);
		RDTSC_STOP(c1);
		stddev_add(&noop, c1 - c0);

		RDTSC_START(c0);
		half_round(in, out1);
		half_round(in, out1);
		half_round(in, out1);
		half_round(in, out1);
		RDTSC_STOP(c1);
		stddev_add(&native, c1 - c0);

		RDTSC_START(c0);
		siphash24_half_round_asm(in, out1);
		siphash24_half_round_asm(in, out1);
		siphash24_half_round_asm(in, out1);
		siphash24_half_round_asm(in, out1);
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
	printf("noop round   min/max=%i/%i   avg=%.3f  +/- %.3f\n",
	       noop_min, max, avg, dev);
	stddev_get(&native, NULL, &min, &max, &avg, &dev);
	printf("native       min/max=%i/%i   avg=%.3f  +/- %.3f\n",
	       min-noop_min, max-noop_min, avg-noop_min, dev);
	a = min-noop_min;
	stddev_get(&sse, NULL, &min, &max, &avg, &dev);
	printf("sse          min/max=%i/%i   avg=%.3f  +/- %.3f\n",
	       min-noop_min, max-noop_min, avg-noop_min, dev);
	b = min-noop_min;
	printf("gain: %.3f%%\n", (1. - b/a)*100.);
	return 0;
}
