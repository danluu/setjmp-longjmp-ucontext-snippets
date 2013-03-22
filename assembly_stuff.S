
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
        sub $0x10, %esp

        mov %ebp, %esp
        add $4, %esp
        mov (%ebp), %ebp
        pop %eax
        jmp *%eax

ENTRY(simple_printing_fun):
        push %ebp
        mov %esp, %ebp
        sub $10, %esp

        push $STRING
        call printf

        push $0x1234
        push $0x00
        push $FORMAT
        call printf

        leave
        ret

