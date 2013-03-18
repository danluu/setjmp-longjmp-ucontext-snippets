#include <stdio.h>
#include <setjmp.h>
#include <libc.h>
#include <assert.h>
 
#define NUM_COROS 10
 
int current_pid;
 
jmp_buf bufs[NUM_COROS];
 
static int spawn(void (*)(int));
 
void yield(int pid)
{
  int saved_current_pid = current_pid;
  if (!setjmp(bufs[current_pid]))
		{
			// before you do a longjmp, set current pid to new one
			current_pid = pid;
			longjmp(bufs[pid], 1);
		}
  else
		{
			// if we return from setjmp, reset the current_pid 
			// to what it used to be
			current_pid = saved_current_pid;
			return; // keep doing what we were doing!
		}
}
 
void test_one(int pid)
{
  int p;
  for (p = 0; p < 10; p++)
		{
			printf("test_one(%d): &p --> %ld\n", pid, (long)&p);
			assert(current_pid == pid);
			yield(0); // yield to top context
			assert(current_pid == pid);
		}
}
 
static void scheduler()
{
  int p;
  current_pid = 0;
  printf("scheduler: p --> %ld\n", (long)&p);
  int pid = spawn(test_one);
  assert(current_pid == 0);
  yield(pid);
  printf("scheduler: never got here\n");
}
 
void (*spawned_fun)(int);
 
static int spawn(void (*f)(int))
{
  // funky off by one stuff here maybe
  static int pid_counter = 0;
  pid_counter++;
  spawned_fun = f;
  yield(pid_counter);
  return pid_counter;
}
 
// have never exit so we get a pristine stack for our coroutines
static void grow_stack(int n)
{
  int p;
  char big_array[2048];
 
  if (n == NUM_COROS)
		{
			scheduler();
			assert(0);
			return;
		}
 
  int jmp_result = setjmp(bufs[n]);
 
  if (!jmp_result)
		{
			memset(big_array, 0, sizeof(big_array));
			printf("#%d: p --> %ld\n", n, (long)&p);
			grow_stack(n + 1);
		}
  else
		{
			// how does spawn/fork work?
			spawned_fun(n);
			assert(0);
		}
}
 
int main()
{
  grow_stack(0);
  assert(0); // never get here
  return 0;
}
