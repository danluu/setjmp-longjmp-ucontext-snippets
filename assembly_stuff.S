
#define ENTRY(name) \
  .globl name; \
  .globl _##name; \
  .align 4,0x90; \
  name: ; \
  _##name

ENTRY(simple):
        ret

ENTRY(simple_no_ret):
        pop %eax
        jmp *%eax


ENTRY(simple_stack_frame):
        push %ebp
        mov %esp, %ebp
        sub $10, %esp

        mov %ebp, %esp
        pop %ebp
        ret


ENTRY(simple_stack_frame_leave):
        push %ebp
        mov %esp, %ebp
        sub $10, %esp

        leave
        ret


ENTRY(simple_stack_frame_hack):
        push %ebp
        mov %esp, %ebp
        sub $10, %esp

        mov (%ebp), %ebp
        mov %ebp, %esp
        sub $4, %esp
        ret

