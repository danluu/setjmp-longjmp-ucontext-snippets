
#define ENTRY(name) \
  .globl name; \
  .globl _##name; \
  .align 4,0x90; \
  name: ; \
  _##name

STRING:
  .ascii "hello assembly\n\0"

FORMAT:
  .ascii "%d %d\n\0"

ENTRY(simple):
        ret

ENTRY(simple_no_ret):
        pop %rax
        jmp *%rax


ENTRY(simple_stack_frame):
        push %rbp
        mov %rsp, %rbp
        sub $10, %rsp

        mov %rbp, %rsp
        pop %rbp
        ret


ENTRY(simple_stack_frame_leave):
        push %rbp
        mov %rsp, %rbp
        sub $10, %rsp

        leave
        ret


ENTRY(simple_stack_frame_hack):
        push %rbp
        mov %rsp, %rbp
        sub $10, %rsp

        leave
        ret
        // // restore ebp
        // mov (%rbp), %rbp

        // //restore esp (move to old ebp, then fix it)
        // mov %rbp, %rsp
        // add $4, %rsp

        // ret
        //pop %rax
        //jmp *%rax

ENTRY(simple_printing_fun):
        push %rbp
        mov %rsp, %rbp
        sub $10, %rsp

//	mov $STRING, %rdi
	//Note: on Mac OS X we must do PC-relative addressing for this sort of thing
	movq STRING@GOTPCREL(%rip), %rdi
	xor %rax, %rax
        call _printf

	mov $10, %rsi
	mov $0x1234, %rdx
	xor %rax, %rax
	movq FORMAT@GOTPCREL(%rip), %rdi
        call _printf

        leave
        ret


