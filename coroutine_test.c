/* -*-mode:c; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#include <stdio.h>
#include <assert.h>

#include "coroutines.h"

static void test_one();

int main()
{
  coro_allocate(10);
  printf("main: coro_allocate finished\n");
  int p;
  int i;
  int pids[10];

  for (p = 0; p < 10; p++)
    {
      pids[p] = coro_spawn(test_one);
      printf("main: coro_spawn %d\n", pids[p]);
    }
  assert(coro_pid == 0);
  for (i = 0; i < 3; i++)
    {
      for (p = 0; p < 10; p++)
        {
          printf("main: yielding %d (i = %d)\n", pids[p], i);
          coro_yield(pids[p]);
        }
    }
  printf("main: finished\n");
  return 0;
}

static void test_one()
{
  coro_yield(0);
  int p;
  for (p = 0; p < 2; p++)
    {
      printf("test_one(%d): %i\n", coro_pid, p);
      coro_yield(0); // yield to top context
    }
  printf("test_one(%d): done\n", coro_pid);
  coro_yield(0);
}

