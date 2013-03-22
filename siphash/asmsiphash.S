
.align 16, 0x00
INITV0V1:       .ascii "somepseudorandom"
        #.octa 0x736f6d6570736575646f72616e646f6d
INITV2V3:
        .ascii "lygeneratedbytes"
        # .octa 0x6c7967656e6572617465646279746573

#define ROTLQ(xmm, s, xmmtmp)    \
        movdqa xmm, xmmtmp;	\
        psllq $0+s, xmm;        \
        psrlq $64-s, xmmtmp;	\
        pxor xmmtmp, xmm

#define ROTQ(xmm1, rotb, rotd, xmm2, xmm3, xmm4)      \
        /* xmm1 = b, _; xmm2 = _, d */  \
        movdqa xmm1, xmm2;              \
        /* xmm2 = d, _ */               \
        punpckhqdq xmm2, xmm2;          \
                                        \
        /* b = ROTATE(b, s) */          \
        rotb(xmm1, xmm3);               \
        /* d = ROTATE(d, t) */          \
        rotd(xmm2, xmm4);               \
        /* xmm2 = b, d */               \
        punpcklqdq xmm2, xmm1

#define ROTLQ17(xmm, xmmtmp)    ROTLQ(xmm, 17, xmmtmp)
#define ROTLQ21(xmm, xmmtmp)    ROTLQ(xmm, 21, xmmtmp)
#define ROTLQ13(xmm, xmmtmp)    ROTLQ(xmm, 13, xmmtmp)
#define ROTLQ16(xmm, xmmtmp)    \
        pshuflw $0x93, xmm, xmm

#define ROTQ1721(xmm1, xmm2, xmm3, xmm4)        \
        ROTQ(xmm1, ROTLQ17, ROTLQ21, xmm2, xmm3, xmm4)
#define ROTQ1316(xmm1, xmm2, xmm3, xmm4)        \
        ROTQ(xmm1, ROTLQ13, ROTLQ16, xmm2, xmm3, xmm4)

        /* xmm0: a, c
         * xmm1: b, d
         * s - b's shift
         * t - d's shift
         * xmm2, xmm3, xmm4: clobbered
         */
#define HALF_ROUND(rot) 	        \
        /* xmm0 = a, c; xmm1 = b, d */	\
        /* a += b; c += d */            \
        paddq %xmm1, %xmm0;             \
        rot(%xmm1, %xmm2, %xmm3, %xmm4); \
                                        \
        /* b ^= a; d ^= c */            \
        pxor %xmm0, %xmm1;              \
        /* xmm0 = c, ROTATE(a, 32) */   \
        pshufd $0x1E, %xmm0, %xmm0;

#define ROTATE(xmm0, xmm1, xmmtmp)      \
        /* rotate (a,b), (c,d) to (a,c), (b,d) */ \
        movdqa xmm1, xmmtmp;            \
        movdqa xmm0, xmm1;              \
        punpcklqdq xmmtmp, xmm0;        \
        punpckhqdq xmmtmp, xmm1


#define DOUBLE_ROUND()                  \
        /* xmm0 = a(lo), c(hi) */       \
        /* xmm1 = b(lo), d(hi) */       \
                                        \
        HALF_ROUND(ROTQ1316);           \
        HALF_ROUND(ROTQ1721);           \
        HALF_ROUND(ROTQ1316);           \
        HALF_ROUND(ROTQ1721)


.align 16, 0x90
.globl siphash24_half_round_asm
.globl _siphash24_half_round_asm
siphash24_half_round_asm:
_siphash24_half_round_asm:
        push %ebp
        mov %esp,%ebp
        sub $0x30, %esp

        mov 0x08(%ebp), %eax
        movdqu 0x00(%eax), %xmm0
        movdqu 0x10(%eax), %xmm1

        ROTATE(%xmm0, %xmm1, %xmm3)
        DOUBLE_ROUND()
        ROTATE(%xmm0, %xmm1, %xmm3)

        mov 0x0c(%ebp), %ecx
        movdqu %xmm0, 0x00(%ecx)
        movdqu %xmm1, 0x10(%ecx)

        leave
        ret


.align 16, 0x90
.globl siphash24_asm
.globl _siphash24_asm
siphash24_asm:
_siphash24_asm:
        push %ebp
        mov %esp, %ebp
        sub $0x30, %esp
        and $0xfffffff0, %esp

        /* 8(%ebp) -> const void *src
         * 12(%ebp) -> unsigned long src_sz
         * 16(%ebp) -> char key[16];
         * neither src and key don't need to be aligned
         */
        mov 0x10(%ebp), %eax
        movdqu (%eax), %xmm4	# load 128 key k0, k1

        movl $0x736f6d65, 0x00(%esp)
        movl $0x70736575, 0x04(%esp)
        movl $0x646f7261, 0x08(%esp)
        movl $0x6e646f6d, 0x0c(%esp)
        movl $0x6c796765, 0x10(%esp)
        movl $0x6e657261, 0x14(%esp)
        movl $0x74656462, 0x18(%esp)
        movl $0x79746573, 0x1c(%esp)
        /* Prologue */
        movdqu 0x00(%esp), %xmm0	# v0, v1
        movdqu 0x10(%esp), %xmm1	# v2, v3
        pxor %xmm4, %xmm0       # v0^k0, v1^k1
        pxor %xmm4, %xmm1       # v2^k0, v3^k1

        /* Intermediate rounds */

        /* Finalization */
        pxor %xmm2, %xmm2       # b = (0,0)
        pxor %xmm2, %xmm1       # v3^b

        ROTATE(%xmm0, %xmm1, %xmm3)
        DOUBLE_ROUND()
        ROTATE(%xmm0, %xmm1, %xmm3)

        /* Epilogue */
        pxor %xmm1, %xmm0       # xmm0 = v0^v2, v1^v3
        movdqa %xmm0, %xmm1
        punpckhqdq %xmm0, %xmm0	# the same as psrldq $8, %xmm0
        pxor %xmm1, %xmm0	# lower 64 bits of xmm0 are final xor
        movq %xmm0, (%esp)
        mov 0x4(%esp), %edx
        mov 0x0(%esp), %eax
        leave
        ret
