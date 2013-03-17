#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
 
void first(void);
void second(void);
 
/* This program's output is:
 
calling first
calling second
entering second
second failed with type 3 exception; remapping to type 1.
first failed, exception type 1
 
*/
 
/* Use a file scoped static variable for the exception stack so we can access
 * it anywhere within this translation unit. */
static jmp_buf exception_env;
static int exception_type;
 
int main() {
	void *volatile mem_buffer;
 
	mem_buffer = NULL;
	if (setjmp(exception_env)) {
		/* if we get here there was an exception */
		printf("first failed, exception type %d\n", exception_type);
	} else {
		/* Run code that may signal failure via longjmp. */
		printf("calling first\n");
		first();
		mem_buffer = malloc(300); /* allocate a resource */
		printf("%s",strcpy((char*) mem_buffer, "first succeeded!")); /* ... this will not happen */
	}
	if (mem_buffer)
		free((void*) mem_buffer); /* carefully deallocate resource */
	return 0;
}
 
void first(void) {
	jmp_buf my_env;
 
	printf("calling second\n");
	memcpy(my_env, exception_env, sizeof(jmp_buf));
	switch (setjmp(exception_env)) {
	case 3:
		/* if we get here there was an exception. */
		printf("second failed with type 3 exception; remapping to type 1.\n");
		exception_type = 1;
 
	default: /* fall through */
		memcpy(exception_env, my_env, sizeof(jmp_buf)); /* restore exception stack */
		longjmp(exception_env, exception_type); /* continue handling the exception */
 
	case 0:
		/* normal, desired operation */
		second();
		printf("second succeeded\n");  /* not reached */
	}
	memcpy(exception_env, my_env, sizeof(jmp_buf)); /* restore exception stack */
}
 
void second(void) {
	printf("entering second\n" ); /* reached */
	exception_type = 3;
	longjmp(exception_env, exception_type); /* declare that the program has failed */
	printf("leaving second\n"); /* not reached */
}
