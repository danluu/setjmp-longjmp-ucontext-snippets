#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <setjmp.h>

jmp_buf try;

void handler(int sig) {
  write(2, "stack overflow\n", 15);
  longjmp(try, 1);
  _exit(1);
}

unsigned recurse(unsigned x) {
    return recurse(x)+1;    
}

int main() {
  static char stack[SIGSTKSZ];
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
  if (!setjmp (try)) {
    recurse(0);
  } else {
    printf("caught exception!\n");    
  }

  return 0;
}
