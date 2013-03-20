#include <stdio.h>
#include <setjmp.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef void (*coroutine_cb)();

static void f();
static void g();
static void h();
static void scheduler(coroutine_cb);
static int spawn(coroutine_cb);
static void yield(int);

jmp_buf scheduler_jmp;
void *scheduler_rbp;
int coro_pid;

coroutine_cb scheduler_next_coro;

#define MAX_COROS 10

struct {
  jmp_buf jmp;
  void *stack;
  long stack_sz;
} coroutines[MAX_COROS];

static void f()
{
  int local_var = 0x1234;
  printf("Hi, I'm f (local_var --> %d)!\n", local_var);
  spawn(g);
  printf("f just spawned g (local_var --> %d)\n", local_var);
  spawn(h);
  printf("f just spawned h (local_var --> %d)\n", local_var);
  exit(0);
}

static void g()
{
  printf("Hi, I'm g!\n");
  yield(1);
}

static void h()
{
  printf("Hi, I'm h!\n");
  yield(1);
}


static void scheduler(coroutine_cb coro)
{
  printf("starting the scheduler...\n");
  static int max_pid = 1;
  scheduler_rbp = __builtin_frame_address(0);
  scheduler_next_coro = coro;
  int value = setjmp(scheduler_jmp);
  // value == 0 means just starting
  // value == -1 means spawning new coroutine
  // value positive means hop back to a specific pid
  if (value == 0 || value == -1)
  {
    coro_pid = max_pid++;
    printf("about to run coro %d...\n", coro_pid);
    char *buf = alloca(0x1000);
    asm volatile("" :: "m" (buf));
    scheduler_next_coro();
    assert(0);
  }
  else
  {
    printf("jumped back to scheduler (pid --> %d) (coro_pid --> %d)...\n", value, coro_pid);
    // restore coroutine marked by value (pid)
    coro_pid = value;

    // make them definitely not on the stack
    static int i;
    static int stack_sz; 
    stack_sz = coroutines[coro_pid].stack_sz;
    static char *d; 
    d = (char *)scheduler_rbp - stack_sz;
    static char *s; 
    s = (char *)coroutines[coro_pid].stack;
    for (i = 0; i < stack_sz; i++)
    {
      *d++ = *s++;
    }

    longjmp(coroutines[coro_pid].jmp, 1);
    assert(0);
  }
}

static void yield(int pid)
{
  // take current rbp
  void *rbp = __builtin_frame_address(0);
  void *fudgy_rsp = (char *)rbp - 0x100;
  assert(scheduler_rbp > rbp);

  long stack_sz = (char *)scheduler_rbp - (char *)fudgy_rsp;
  void *stack = malloc(stack_sz);

  /*
   * Marek: check how overflowing stack actually works, because
   * we're actually copying data beyond our stack frame.
   */
  memcpy(stack, fudgy_rsp, stack_sz);
  coroutines[coro_pid].stack = stack;
  coroutines[coro_pid].stack_sz = stack_sz;

  if (!setjmp(coroutines[coro_pid].jmp))
  {
    longjmp(scheduler_jmp, pid);
    assert(0);
  }
  else
  {
    // our stack is already good to go at this point
    return;
  }
}

/*
 *
 */
static int spawn(coroutine_cb coro)
{
  scheduler_next_coro = coro;
  yield(-1);
  return 0; // need to get pid
}

int main()
{
  scheduler(f);
  assert(0);
  return 0;
}
