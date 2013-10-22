/* -*-mode:c; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#include <stdio.h>
#include <assert.h>

#include "coroutines.h"

static void test_one(void*);

int main() {
  // Make space for 10 coroutines and set coro_pid to 0
  // Note that this calls grow_stack(n) for each n, which does a setjmp for bufs[n]
  // The first yield will yield into this, which will fall into the second half of
  // grow_stack, which calls the function (which will be test_one)
  coro_allocate(10);
  printf("main: coro_allocate finished\n");
  int p;
  int pids[10];
  int valid_pid_count;

  // Each coro_spawn will:
  // 1. Set spawned_fun = test_one
  // 2. Set spawned_user_state = NULL
  // 3. Mark used_pids[p] = 1 == runnable
  // 4. Yield to the pid, p
  // 5. Yield sets saved_coro_pid = 0 == scheduler
  // 6. Setjmp locally for p
  // 7. longjmp back to scheduler
  for (p = 0; p < 10; p++) {
    pids[p] = coro_spawn(test_one, NULL);
    printf("main: coro_spawn %d\n", pids[p]);
  }
  assert(coro_pid == 0);
  do {
    valid_pid_count = 0;
    for (p = 0; p < 10; p++) {    
      printf("main: yielding %d\n", pids[p]);
      coro_yield(pids[p]);
      valid_pid_count += coro_runnable(pids[p]);
    }
  } while (valid_pid_count > 0);
  printf("main: finished\n");
  return 0;
}

// Each call to yield will, the first time through, pass through the !setjmp section and yield back to 0.
// When we're yielded to again it simply returns and execution continues.
static void test_one(void* _) {
  coro_yield(0);
  int p;
  for (p = 0; p < 2; p++) {
    printf("test_one(%d): %i\n", coro_pid, p);
    coro_yield(0); // yield to top context
  }
  printf("test_one(%d): done\n", coro_pid);
  coro_yield(0);
}

