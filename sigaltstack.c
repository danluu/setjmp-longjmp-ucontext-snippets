#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <setjmp.h>

jmp_buf try;

void handler(int sig) {
  static int i = 0;

  printf("stack overflow %d\n", i);
  longjmp(try, ++i);
  assert(0);
}

unsigned recurse(unsigned x) {
    return recurse(x)+1;    
}

int main() {
  char* stack;
  stack = malloc(sizeof(stack) * SIGSTKSZ);
  stack_t ss = {
    .ss_size = SIGSTKSZ,
    .ss_sp = stack,
  };
  struct sigaction sa = {
    .sa_handler = handler,
    .sa_flags = SA_ONSTACK
  };

  sigaltstack(&ss, 0);
  sigfillset(&sa.sa_mask);
  sigaction(SIGSEGV, &sa, 0);

  if (setjmp(try) < 3) {
    recurse(0);
  } else {
    printf("caught exception!\n");    
  }

  return 0;
}
