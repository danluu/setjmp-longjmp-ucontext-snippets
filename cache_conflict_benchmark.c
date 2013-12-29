// Measure the cost of associativity misses

#define __STDC_FORMAT_MACROS

#include <stdio.h>
#include <inttypes.h>
#include "rdtsc.h"
#include "stddev.h"

#define RUNS 1000000

#define MAX_NUM_LINES 10000
#define LINE_SIZE 64
#define PG_SIZE 4096

inline uint64_t min(uint64_t a, uint64_t b) { return (a < b) ? a : b; }

// Do 'n' accesses with a relative offset of 'align'
uint64_t access_mem(int align, int n) {
  static char a[2 * MAX_NUM_LINES * PG_SIZE];
  int sum = 0;
  uint64_t tsc_before, tsc_after, tsc, min_tsc;
  uint64_t offset;

  min_tsc = 0;
  min_tsc--;

  int i, j;
  for (i = 0; i < n * PG_SIZE; i++) {
    a[i] = i % 17;
  }

  // Warmup
  offset = 0;
  for (j = 0; j < n; j++) {
    offset += PG_SIZE + align;
    sum += a[offset];
  }

  // Do accesses seperated by one page +/- alignment offset
  for (i = 0; i < RUNS; i++) {
    offset = 0;
    RDTSC_START(tsc_before);
    for (j = 0; j < n; j++) {
      offset += PG_SIZE + align;
      sum += a[offset];
    }
    RDTSC_START(tsc_after);
    tsc = tsc_after - tsc_before;
    min_tsc = min(min_tsc, tsc);

  }

  printf("Sum: %i\n", sum); // Dumb trick to prevent code from being optimized away

  return min_tsc;
}

void test_and_print(int n) {
  double diff;
  uint64_t aligned_time, unaligned_time;

  aligned_time = access_mem(0, n);
  unaligned_time = access_mem(LINE_SIZE, n);
  diff = (double)aligned_time / (double)unaligned_time;

  printf("----------%i accesses--------\n", n);
  printf("Page-aligned time:         %" PRIu64 "\n", aligned_time);
  printf("Page-unaligned (+64) time: %" PRIu64 "\n", unaligned_time);
  printf("Difference: %f\n", diff);
}

int main() {
  test_and_print(10);
  test_and_print(100);
  test_and_print(1000);
  test_and_print(10000);

  return 0;
}
