/* -*-mode:c; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#include <stdio.h>
#include <setjmp.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "coroutines.h"

int coro_pid;
int coro_max;

jmp_buf* bufs;
int* used_pids;

#define STACK_SIZE 0x1000

void coro_yield(int pid)
{
  int saved_coro_pid = coro_pid;
  if (!setjmp(bufs[coro_pid]))
    {
      // before you do a longjmp, set current pid to new one
      coro_pid = pid;
      longjmp(bufs[pid], 1);
      assert(0);
    }
  else
    {
      // if we return from setjmp, reset the coro_pid 
      // to what it used to be
      coro_pid = saved_coro_pid;
      return; // keep doing what we were doing!
    }
}

int coro_runnable(int pid)
{
  return used_pids[pid];
}
 
coro_callback spawned_fun;
void* spawned_user_state;
 
int coro_spawn(coro_callback f, void* user_state)
{
  int pid;
  spawned_fun = f;
  spawned_user_state = user_state;

  for (pid = 0; pid < coro_max; pid++)
    {
    if (used_pids[pid] == 0)
      {
        used_pids[pid] = 1;
        coro_yield(pid);
        return pid;        
      }
  }
  assert(0);
  return 0;
}

// have never exit so we get a pristine stack for our coroutines
static void grow_stack(int n, int num_coros)
{
  if (n == num_coros + 1)
    {
      longjmp(bufs[0],1);
      assert(0);
      return;
    }

  if (!setjmp(bufs[n]))
    {
      char *big_array;
      big_array = alloca(STACK_SIZE);
      big_array[0] = big_array[STACK_SIZE-1] = 0xFF;

      grow_stack(n + 1, num_coros);
    }
  else
    {
      // how does spawn/fork work?
      while(1)
        {
          assert(spawned_fun);
          coro_callback f = spawned_fun;
          spawned_fun = NULL;

          assert(n == coro_pid);
          f(spawned_user_state);
          used_pids[n] = 0;
          coro_yield(0);
        }
    }
}
 
void coro_allocate(int num_coros)
{
  char big_array[STACK_SIZE];
  /* Touch big_array twice to stop gcc from complaininig from being
   * set but not used. */
  big_array[0] = big_array[STACK_SIZE-1] = 0xFF;

  // want n slots + slot '0' = num_coros + 1
  coro_max = num_coros + 1;
  bufs = malloc(sizeof(jmp_buf) * (coro_max));
  used_pids = calloc(coro_max, sizeof(int));
  used_pids[0] = 1;
  coro_pid = 0;

  if (!setjmp(bufs[0]))
    {
      grow_stack(1, num_coros);
      assert(0);
    }
  else
    {
      return;
    }
}
