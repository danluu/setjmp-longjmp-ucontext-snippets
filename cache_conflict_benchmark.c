// Measure the cost of TLB thrashing from page-aligned accesses

#include <stdio.h>
#include "rdtsc.h"
#include "stddev.h"

#define RUNS 1000000

#define NUM_LINES 1000
#define LINE_SIZE 64
#define PG_SIZE 4096

inline uint64_t min(uint64_t a, uint64_t b) { return (a < b) ? a : b; }

uint64_t access_mem(int align) {
  static char a[2 * NUM_LINES * PG_SIZE];
  int sum = 0;
  uint64_t tsc_before, tsc_after, tsc, min_tsc;
  uint64_t offset;

  min_tsc = 0;
  min_tsc--;

  int i, j;
  for (i = 0; i < NUM_LINES * PG_SIZE; i++) {
    a[i] = i % 17;
  }

  // Warmup
  offset = 0;
  for (j = 0; j < NUM_LINES; j++) {
    offset += PG_SIZE + align;
    sum += a[offset];
  }

  // Do accesses seperated by one page +/- alignment offset
  for (i = 0; i < RUNS; i++) {
    offset = 0;
    RDTSC_START(tsc_before);
    for (j = 0; j < NUM_LINES; j++) {
      offset += PG_SIZE + align;
      sum += a[offset];
    }
    RDTSC_START(tsc_after);
    tsc = tsc_after - tsc_before;
    min_tsc = min(min_tsc, tsc);

  }

  printf("Sum: %i\n", sum);

  return min_tsc;
}

int main() {
  double diff;
  uint64_t aligned_time, unaligned_time;

  aligned_time = access_mem(0);
  unaligned_time = access_mem(LINE_SIZE);
  diff = (double)aligned_time / (double)unaligned_time;

  printf("Page-aligned time:          %llu\n", aligned_time);
  printf("Page-unaligned (+128) time: %llu\n", unaligned_time);
  printf("Difference: %f\n", diff);

  aligned_time = access_mem(0);
  unaligned_time = access_mem(LINE_SIZE);
  diff = (double)aligned_time / (double)unaligned_time;

  printf("Page-aligned time:          %llu\n", aligned_time);
  printf("Page-unaligned (+128) time: %llu\n", unaligned_time);
  printf("Difference: %f\n", diff);

  return 0;
}
