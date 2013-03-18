#include <stdio.h>
 
#define cpuid(func,ax,bx,cx,dx)\
	__asm__ __volatile__ ("cpuid":\
												"=a" (ax), "=b" (bx), "=c" (cx), "=d" (dx) : "a" (func));

#define LOOP_MAX 100000000
#define RUNS 10

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
	int i,j = 0;
	int tsc_before, tsc_after;
	int a, b, c;



	for(j = 0; j < RUNS; ++j){

		tsc_before = rdtsc();
		for(i = 0; i < LOOP_MAX; ++i){
			asm volatile (
                    "push %%rax;"
										"mov %%rbx, %%rcx;"
										"mov %%rbx, %%rcx;"
										"pop %%rdx;"
                    "push %%rax;"
										"mov %%rbx, %%rcx;"
										"mov %%rbx, %%rcx;"
										"pop %%rdx;"
                    "push %%rax;"
										"mov %%rbx, %%rcx;"
										"mov %%rbx, %%rcx;"
										"pop %%rdx;"
										:
										:
										: "%rax", "%rcx", "%rdx");
		}
		tsc_after = rdtsc();
		a += (tsc_after - tsc_before) / RUNS;
		printf("A: %i\n", tsc_after - tsc_before);
		
		tsc_before = rdtsc();
		for(i = 0; i < LOOP_MAX; ++i){
			asm volatile (
										"push %%rax;"
										"mov %%rsp, %%rcx;"
										"mov %%rsp, %%rcx;"
										"pop %%rdx;"
										"push %%rax;"
										"mov %%rsp, %%rcx;"
										"mov %%rsp, %%rcx;"
										"pop %%rdx;"
										"push %%rax;"
										"mov %%rsp, %%rcx;"
										"mov %%rsp, %%rcx;"
										"pop %%rdx;"
										:
										:
										: "%rax", "%rcx", "%rdx");
		}
		tsc_after = rdtsc();
		
		printf("B: %i\n", tsc_after - tsc_before);	
		b += (tsc_after - tsc_before) / RUNS;
		
		tsc_before = rdtsc();
		for(i = 0; i < LOOP_MAX; ++i){
			asm volatile (
										"push %%rax;"
										"mov %%rsp, %%rcx;"
										"mov %%rcx, %%rsp;"
										"pop %%rdx;"
										"push %%rax;"
										"mov %%rsp, %%rcx;"
										"mov %%rcx, %%rsp;"
										"pop %%rdx;"
										"push %%rax;"
										"mov %%rsp, %%rcx;"
										"mov %%rcx, %%rsp;"
										"pop %%rdx;"
										:
										:
										: "%rax", "%rcx", "%rdx");
		}
		tsc_after = rdtsc();
		
		printf("C: %i\n", tsc_after - tsc_before);
		c += (tsc_after - tsc_before) / RUNS;
	}

	printf("no dependency / read dependency / read+write dependency: %i %i %i\n", a, b, c);

}
