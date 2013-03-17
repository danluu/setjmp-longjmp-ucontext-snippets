#include <setjmp.h>
#include <stdio.h>
 
#define cpuid(func,ax,bx,cx,dx)\
	__asm__ __volatile__ ("cpuid":\
												"=a" (ax), "=b" (bx), "=c" (cx), "=d" (dx) : "a" (func));

#define LOOP_MAX 100000000

jmp_buf mainTask, childTask;
 
void call_with_cushion(void);
void child(void);

inline int rdtsc(void){
	int tsc = 0;

	volatile int dont_remove __attribute__((unused)); // volatile to stop optimizing
	unsigned tmp;
	cpuid(0, tmp, tmp, tmp, tmp);                   // cpuid is a serialising call
	dont_remove = tmp;                                // prevent optimizing out cpuid
	__asm__ __volatile__("rdtsc; "          // read of tsc
											 "shl $32,%%rdx; "  // shift higher 32 bits stored in rdx up
											 "or %%rdx,%%rax"   // and or onto rax
											 : "=a"(tsc)        // output to tsc
											 :
											 : "%rcx", "%rdx"); // rcx and rdx are clobbered
	return tsc;
}
 
int main(void) {
	int i = 0;
	int tsc_before, tsc_after;

	tsc_before = rdtsc();
	for(i = 0; i < LOOP_MAX; ++i){
		asm volatile ("push %%rax;"
								 "mov %%ecx, %%ebx;"
								 "pop %%rax;"
								 :
								 :
								 : "%rax", "%rcx", "%rsp");
	}
	tsc_after = rdtsc();

	

	printf("total A: %i\n", tsc_after - tsc_before);

	tsc_before = rdtsc();
	for(i = 0; i < LOOP_MAX; ++i){
		asm volatile ("push %%rax;"
								 "mov %%ecx, %%esp;"
								 "pop %%rax;"
								 :
								 :
								 : "%rax", "%rcx", "%rsp");
	}
	tsc_after = rdtsc();
	asm volatile("mov %esp, %ecx");

	printf("total B: %i\n", tsc_after - tsc_before);

}
