#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include <sys/time.h>
#include <sys/resource.h>

#include "rdtsc.h"

void siphash24_half_round_asm(const uint64_t in[4], uint64_t out[4]);


void half_round(const uint64_t in[4], uint64_t out[4]);

#define ROUNDS 30

struct {
	int d0;
	int d1;
} rounds[ROUNDS];

int main() {
	int i;
	uint64_t in[4] = {1, 2, 3, 4};
	uint64_t out1[4], out2[4];

	/* Use 100% CPU for some time */
	for (i = 0; i < 10000000; i++) {
		half_round(in, out1);
		siphash24_half_round_asm(in, out2);
	}

	printf("lo=%016llx hi=%016llx\n", out1[0], out1[1]);
	printf("lo=%016llx hi=%016llx\n", out1[2], out1[3]);

	printf("lo=%016llx hi=%016llx\n", out2[0], out2[1]);
	printf("lo=%016llx hi=%016llx\n", out2[2], out2[3]);
	for (i = 0; i < 4; i++) {
		assert(out1[i] == out2[i]);
	}

	struct rusage rus_before;
	getrusage(RUSAGE_SELF, &rus_before);

	for (i = 0; i < ROUNDS; i++) {
		rounds[i].d0 = 0;
		rounds[i].d1 = 0;
	}

	siphash24_half_round_asm(in, out1);
	half_round(in, out1);

	for (i = 0; i < ROUNDS; i++) {
		uint64_t c0, c1, c2, c3;
		RDTSC_START(c2);
		siphash24_half_round_asm(in, out1);
		RDTSC_STOP(c3);
		RDTSC_START(c0);
		half_round(in, out1);
		RDTSC_STOP(c1);
		rounds[i].d0 = c1 - c0;
		rounds[i].d1 = c3 - c2;
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

	unsigned int d0 = -1, d1 = -1;
	for (i = 0; i < ROUNDS; i++) {
		if (d0 > rounds[i].d0)
			d0 = rounds[i].d0;
		if (d1 > rounds[i].d1)
			d1 = rounds[i].d1;
		printf("    C =%4i  ASM =%4i\n", rounds[i].d0, rounds[i].d1);
	}
	printf("MIN C=%4i ASM=%4i   diff=%.3f%%\n", d0, d1, (d0-d1)*100./d0);
	return 0;
}
