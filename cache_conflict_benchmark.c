// Measure the cost of associativity misses

#define __STDC_FORMAT_MACROS

#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>
#include "rdtsc.h"
#include "stddev.h"

//#define GRAPH

#define MAX_NUM_LINES 10000
#define LINE_SIZE 64
#define PG_SIZE 4096
#define WORD_SIZE 8

inline uint64_t min(uint64_t a, uint64_t b) { return (a < b) ? a : b; }

// Do 'n' accesses with a relative offset of 'align'
uint64_t access_mem(int align, int n, int runs) {
  static uint64_t a[2 * MAX_NUM_LINES * PG_SIZE];
  int sum = 0;
  uint64_t tsc_before, tsc_after, tsc, min_tsc;
  uint64_t offset;

  min_tsc = UINT64_MAX;

  int i, j;
  offset = 0;
  for (i = 0; i < n * PG_SIZE; i += (PG_SIZE+align)/WORD_SIZE) {
    offset += (PG_SIZE+align) / WORD_SIZE;
    a[i] = offset;
  }
  a[n * PG_SIZE / WORD_SIZE] = 0;

  // Warmup
  for (i = 0; i < 2; i++) {
    offset = 0;
    for (j = 0; j < n; j++) {
      sum += a[offset];
      offset = a[offset];
    }
  }

  // Do accesses seperated by one page +/- alignment offset
  for (i = 0; i < runs; i++) {
    offset = 0;
    RDTSC_START(tsc_before);
    for (j = 0; j < n; j++) {
      sum += a[offset];
      offset = a[offset];
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
  int runs = 1000000;

  aligned_time = access_mem(0, n, runs);
  unaligned_time = access_mem(LINE_SIZE, n, runs);
  diff = (double)aligned_time / (double)unaligned_time;

  printf("----------%i accesses--------\n", n);
  printf("Page-aligned time:         %" PRIu64 "\n", aligned_time);
  printf("Page-unaligned (+64) time: %" PRIu64 "\n", unaligned_time);
  printf("Difference: %f\n", diff);
}

void inefficient_csv_output(int n) {
  double diff;
  uint64_t *aligned_time, *unaligned_time;
  int runs = 1000000;
  int i;
  
  aligned_time = malloc(n * sizeof(uint64_t));
  unaligned_time = malloc(n * sizeof(uint64_t));

  for (i = 0; i < n; i++) {
    aligned_time[i] = access_mem(0, i, runs);
    unaligned_time[i] = access_mem(LINE_SIZE, i, runs);
  }

  for (i = 0; i < n; i++) {
    printf("%" PRIu64 ",", aligned_time[i]);
  }
  printf("\n");

  for (i = 0; i < n; i++) {
    printf("%" PRIu64 ",", unaligned_time[i]);
  }
  printf("\n");

  for (i = 0; i < n; i++) {
    diff = (double)aligned_time[i] / (double)unaligned_time[i];
    printf("%f,", diff);
  }
  printf("\n");
}

int main() {
  #ifndef GRAPH
  test_and_print(16);
  test_and_print(32);
  test_and_print(64);
  test_and_print(128);
  test_and_print(256);
  test_and_print(512);
  test_and_print(1024);
  #else
  inefficient_csv_output(1024);
  #endif
  return 0;
}
