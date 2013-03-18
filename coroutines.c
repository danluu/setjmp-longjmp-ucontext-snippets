/* -*-mode:c; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#include <stdio.h>
#include <setjmp.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "coroutines.h"

int coro_pid;
  
jmp_buf* bufs;
 
void coro_yield(int pid)
{
  int saved_coro_pid = coro_pid;
  if (!setjmp(bufs[coro_pid]))
    {
      // before you do a longjmp, set current pid to new one
      coro_pid = pid;
      longjmp(bufs[pid], 1);
    }
  else
    {
      // if we return from setjmp, reset the coro_pid 
      // to what it used to be
      coro_pid = saved_coro_pid;
      return; // keep doing what we were doing!
    }
}
 
void (*spawned_fun)(int);
 
int coro_spawn(void (*f)(int))
{
  // funky off by one stuff here maybe
  static int pid_counter = 0;
  pid_counter++;
  spawned_fun = f;
  coro_yield(pid_counter);
  return pid_counter;
}

// have never exit so we get a pristine stack for our coroutines
static void grow_stack(int n, int num_coros)
{
  int p;
  char big_array[2048];
  memset(big_array, 0, sizeof(big_array));
 
  if (n == num_coros + 1)
    {
      longjmp(bufs[0],1);
      assert(0);
      return;
    }
 
  if (!setjmp(bufs[n]))
    {
      grow_stack(n + 1, num_coros);
    }
  else
    {
      // how does spawn/fork work?
      spawned_fun(n);
      assert(0);
    }
}
 
void coro_allocate(int num_coros)
{
  char big_array[2048];
  memset(big_array, 0, sizeof(big_array));

  // want n slots + slot '0' = num_coros + 1
  bufs = malloc(sizeof(jmp_buf) * (num_coros + 1));
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
