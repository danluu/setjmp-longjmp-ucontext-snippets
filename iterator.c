#include <stdio.h>
#include <stdlib.h>
#define _XOPEN_SOURCE 1 //kill annoying error on Mac OS
#include <ucontext.h>


 
/* This is the iterator function. It is entered on the first call to
 * swapcontext, and loops from 0 to 9. Each value is saved in i_from_iterator,
 * and then swapcontext used to return to the main loop.  The main loop prints
 * the value and calls swapcontext to swap back into the function. When the end
 * of the loop is reached, the function exits, and execution switches to the
 * context pointed to by main_context1. */
void loop(ucontext_t *loop_context,
					ucontext_t *other_context,
					int *i_from_iterator){
	int i;
 
	for (i=0; i < 10; ++i) {
		/* Write the loop counter into the iterator return location. */
		*i_from_iterator = i;
 
		/* Save the loop context (this point in the code) into ''loop_context'',
		 * and switch to other_context. */
		printf("loop: %d\n", i);
		swapcontext(loop_context, other_context);
	}
 
	/* The function falls through to the calling context with an implicit
	 * ''setcontext(&loop_context->uc_link);'' */
} 
 
int main(void){
	/* The three contexts:
	 *    (1) main_context1 : The point in main to which loop will return.
	 *    (2) main_context2 : The point in main to which control from loop will
	 *                        flow by switching contexts.
	 *    (3) loop_context  : The point in loop to which control from main will
	 *                        flow by switching contexts. */
	ucontext_t main_context1, main_context2, loop_context;
 
	/* The stack for the iterator function. */
	char iterator_stack[SIGSTKSZ];
 
	/* Flag indicating that the iterator has completed. */
	volatile int iterator_finished;
 
	/* The iterator return value. */
	volatile int i_from_iterator;
 
	getcontext(&loop_context);
	printf("main: getcontext done\n");
	/* Initialise the iterator context. uc_link points to main_context1, the
	 * point to return to when the iterator finishes. */
	loop_context.uc_link          = &main_context1;
	loop_context.uc_stack.ss_sp   = iterator_stack;
	loop_context.uc_stack.ss_size = sizeof(iterator_stack);
 
	/* Fill in loop_context so that it makes swapcontext start loop. The
	 * (void (*)(void)) typecast is to avoid a compiler warning but it is
	 * not relevant to the behaviour of the function. */
#if defined(__x86_64__)
	int lc_high = (int)(&loop_context);
	int lc_low = (long)(&loop_context) >> 32;
	int mc2_high = (int)(&main_context2);
	int mc2_low = (long)(&main_context2) >> 32;
	int i_high = (int)(&i_from_iterator);
	int i_low = (long)(&i_from_iterator) >> 32;
	printf("high%i:low%i\n",lc_high, lc_low);

	makecontext(&loop_context, (void (*)(void)) loop, 6, lc_high, lc_low, mc2_high, mc2_low, i_high, i_low);
#else
	makecontext(&loop_context, (void (*)(void)) loop, 3, &loop_context, &main_context2, &i_from_iterator);
#endif

 
	/* Clear the finished flag. */      
	iterator_finished = 0;
 
	/* Save the current context into main_context1. When loop is finished,
	 * control flow will return to this point. */
	getcontext(&main_context1);
	printf("main: getcontext(main_context1) done\n",lc_high, lc_low);
 
	if (!iterator_finished) {
		/* Set iterator_finished so that when the previous getcontext is
		 * returned to via uc_link, the above if condition is false and the
		 * iterator is not restarted. */
		iterator_finished = 1;
 
		while (1) {
			/* Save this point into main_context2 and switch into the iterator.
			 * The first call will begin loop.  Subsequent calls will switch to
			 * the swapcontext in loop. */
			printf("main: about to try swapcontext\n",lc_high, lc_low);
			swapcontext(&main_context2, &loop_context);
			printf("main: %d\n", i_from_iterator);
		}
	}
 
	return 0;
}
