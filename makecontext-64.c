#define _XOPEN_SOURCE 1 //kill annoying error on Mac OS

#include <stdio.h>
#include <ucontext.h>
#include <sys/mman.h>

void
assign(long a, int *b)
{
	*b = (int)a;
}

int
main(int argc, char **argv)
{
	ucontext_t uc, back;
	size_t sz = 0x10000;
	int value = 0;

	getcontext(&uc);

	uc.uc_stack.ss_sp = mmap(0, sz,
													 PROT_READ | PROT_WRITE | PROT_EXEC,
													 MAP_PRIVATE | MAP_ANON, -1, 0);
	uc.uc_stack.ss_size = sz;
	uc.uc_stack.ss_flags = 0;

	uc.uc_link = &back;

	makecontext(&uc, assign, 2, 100L, &value);
	swapcontext(&back, &uc);

	printf("done %d\n", value);

	return (0);
}
