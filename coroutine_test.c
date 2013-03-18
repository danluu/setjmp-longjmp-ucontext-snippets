/* -*-mode:c; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#include <stdio.h>
#include <assert.h>

#include "coroutines.h"

static void test_one(int pid);

int main()
{
  coro_allocate(10);
  printf("main: coro_allocate finished\n");
  int p;
  int pid = coro_spawn(test_one);
  printf("main: coro_spawn finished\n");
  assert(coro_pid == 0);
  coro_yield(pid);
  printf("main: never got here\n");
  return 0;
}

static void scheduler()
{

}

static void test_one(int pid)
{
  int p;
  for (p = 0; p < 10; p++)
		{
			printf("test_one(%d): &p --> %ld\n", pid, (long)&p);
			assert(coro_pid == pid);
			coro_yield(0); // yield to top context
			assert(coro_pid == pid);
		}
}

