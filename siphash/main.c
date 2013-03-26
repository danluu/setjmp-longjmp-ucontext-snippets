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
#define MAX_RETRIES 100
#define ACCURACY 0.4

static int do_noop_min() {
	int retry;
	for (retry = 0; retry < MAX_RETRIES; retry += 1) {
		struct stddev noop = INIT_STDDEV;
		int i;
		for (i = 0; i < ROUNDS*10; i++) {
			uint64_t c0, c1;
			RDTSC_START(c0);
			RDTSC_STOP(c1);
			stddev_add(&noop, c1 - c0);
		}
		int noop_min;
		double avg_noop, dev_noop;
		stddev_get(&noop, NULL, &noop_min, NULL, &avg_noop, &dev_noop);
		if (dev_noop > ACCURACY * avg_noop)
			continue;
		return noop_min;
	}
	printf("FATAL! Can't measure NOOP with decent accuracy\n");
	exit(1);
}


static int do_test(char *vector, int vector_sz, char key[16], int noop_min) {
	uint64_t out = 1;
	out = out;
	struct {
		int d0;
		int d1;
	} rounds[ROUNDS];

	/* The hot section */
	struct rusage rus_before;
	getrusage(RUSAGE_SELF, &rus_before);

	memset(rounds, 0, sizeof(rounds));

	out = siphash24(vector, vector_sz, key);
	out = siphash24_asm(vector, vector_sz, key);

	struct stddev native = INIT_STDDEV;
	struct stddev sse = INIT_STDDEV;

	int i;
	for (i = 0; i < ROUNDS; i++) {
		uint64_t c0, c1;
		RDTSC_START(c0);
		out = siphash24(vector, vector_sz, key);
		RDTSC_STOP(c1);
		stddev_add(&native, c1 - c0);

		RDTSC_START(c0);
		out = siphash24_asm(vector, vector_sz, key);
		RDTSC_STOP(c1);
		stddev_add(&sse, c1 - c0);
	}

	struct rusage rus_after;
	getrusage(RUSAGE_SELF, &rus_after);
	if ((rus_after.ru_minflt - rus_before.ru_minflt) ||
	    (rus_after.ru_majflt - rus_before.ru_majflt) ||
	    (rus_after.ru_nswap - rus_before.ru_nswap) ||
	    (rus_after.ru_msgsnd - rus_before.ru_msgsnd) ||
	    (rus_after.ru_msgrcv - rus_before.ru_msgrcv) ||
	    (rus_after.ru_nsignals - rus_before.ru_nsignals) ||
	    (rus_after.ru_nvcsw - rus_before.ru_nvcsw) ||
	    (rus_after.ru_nivcsw - rus_before.ru_nivcsw))
		return 0;

	double avg_sse, avg_native, dev_sse, dev_native;
	stddev_get(&native, NULL, NULL, NULL, &avg_native, &dev_native);
	stddev_get(&sse, NULL, NULL, NULL, &avg_sse, &dev_sse);
	if ((dev_native > ACCURACY * avg_native) ||
	    (dev_sse > ACCURACY * avg_sse))
		return 0;


	int min, max, a, b;
	double avg, dev;
	stddev_get(&native, NULL, &min, &max, &avg, &dev);
	a = min-noop_min;
	printf("native       min/max=%3i/%3i   avg=%7.3f  +/- %.3f\n",
	       a, max-noop_min, avg-noop_min, dev);
	stddev_get(&sse, NULL, &min, &max, &avg, &dev);
	b = min-noop_min;
	printf("sse          min/max=%3i/%3i   avg=%7.3f  +/- %.3f\n",
	       b, max-noop_min, avg-noop_min, dev);
	printf("gain: %.3f%%   CPB=%.3f  for sz=%i\n",
	       (1. - (double)b/a)*100., (double)b / vector_sz, vector_sz);
	return 1;
}

int main() {
	char key[16] = {0};
	uint64_t out1, out2;

	char *vector = "abcdefghabcdefghabcdefghabcdefgh";
//	char *vector = "aaaaaaaa";
	/* char *vector = ""; */
	int vector_sz = strlen(vector);

	/* sanity check */
	out1 = siphash24(vector, vector_sz, key);
	out2 = siphash24_asm(vector, vector_sz, key);

	printf("%016llx\n", out1);
	printf("%016llx\n", out2);
	assert(out1 == out2);

	/* Use 100% CPU for some time to make sure the processor has
	   time to turn off energy saving. */
	int i;
	for (i = 0; i < 10000000; i++) {
		out1 = siphash24(vector, vector_sz, key);
		out1 = siphash24_asm(vector, vector_sz, key);
	}

	int noop_min = do_noop_min();
	printf("[+] empty run: %i cycles\n", noop_min);
	int sz = 0; //vector_sz-1;
	for (; sz <= vector_sz; sz++) {
		out1 = siphash24(vector, sz, key);
		out2 = siphash24_asm(vector, sz, key);
		assert(out1 == out2);
		int retry;
		for (retry = 0; retry < MAX_RETRIES; retry += 1)
			if (do_test(vector, sz, key, noop_min) != 0)
				break;
		if (retry == MAX_RETRIES) {
			printf("FAILED!\n");
			exit(1);
		}
	}

	return 0;
}


