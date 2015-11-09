// Measure the cost of associativity misses

// #define USE_IACA

#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>
#include <sys/mman.h>
#include "../../rdtsc.h"
#include "../../stddev.h"
#include "iacaMarks.h"

#define ALLOC_SIZE 1000000000
#define LINE_SIZE 64
#define WORD_SIZE 8


uint64_t access_mem(int runs, long long nn) {
  uint64_t *a = mmap(NULL, nn, PROT_READ | PROT_WRITE,
		     MAP_PRIVATE| MAP_ANONYMOUS, -1, 0);
  madvise(a, nn, MADV_HUGEPAGE);

  long long loop_max = nn / WORD_SIZE;

  int sum = 0; // Prevent code from being optimized away
  uint64_t tsc_before, tsc_after, tsc, min_tsc;
  uint64_t offset;

  min_tsc = UINT64_MAX;

  int i, j;
  // Prevent optmization against array contents.
  for (i = 0; i < loop_max; i++) {
    a[i] = i % 17;
    asm volatile("" :: "m" (a[i]));    
  }

  
  // Do accesses seperated by one page +/- alignment offset
  for (i = 0; i < runs; i++) {
    offset = 0;
    RDTSC_START(tsc_before);
#ifdef USE_IACA
    IACA_START
#endif
    for (j = 0; j < loop_max; j++) {
      sum += a[offset];
    }
#ifdef USE_IACA
    IACA_END
#endif
    RDTSC_STOP(tsc_after);
    tsc = tsc_after - tsc_before;
    min_tsc = min_tsc < tsc ? min_tsc : tsc;
  }

  asm volatile("" :: "m" (sum));

  return min_tsc;
}

void test_and_print(int n) {
  uint64_t run_cycles;
  double bandwidth, run_time;
  int runs = 10;

  run_cycles = access_mem(runs, n);
  run_time = (1/3.4e9) * (double) run_cycles;
  bandwidth = (double) n / (double) run_time;

  printf("----------%i accesses--------\n", n);
  printf("cycles / time / bw %" PRIu64 " %e %e\n", run_cycles, run_time, bandwidth);
}

int main() {
  test_and_print(1000);
  test_and_print(1000000);
  test_and_print(1000000000);
  // test_and_print(10000000000LL);
  return 0;
}
