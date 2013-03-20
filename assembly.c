#include <stdio.h>

void simple();
void simple_no_ret();
void simple_stack_frame();
void simple_stack_frame_leave();
void simple_stack_frame_hack();


static void fun() {
	int i = 0;
	i = i + 1;
	printf("one\n");
	simple();
	printf("two\n");
	simple_no_ret();
	printf("three\n");
	simple_stack_frame();
	printf("four\n");
	simple_stack_frame_leave();
	printf("five\n");
	simple_stack_frame_hack();
	printf("hack\n");
}


int main() {
	fun();
	return 0;
}
