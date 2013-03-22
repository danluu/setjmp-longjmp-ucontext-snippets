
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

#define ROTQ(xmm1, s, t, xmm2, xmm3, xmm4)      \
        /* xmm1 = b, _; xmm2 = _, d */  \
        movdqa xmm1, xmm2;              \
        /* xmm2 = d, _ */               \
        punpckhqdq xmm2, xmm2;          \
                                        \
        /* b = ROTATE(b, s) */          \
        ROTLQ(xmm1, s, xmm3);           \
        /* d = ROTATE(d, t) */          \
        ROTLQ(xmm2, t, xmm4);           \
        /* xmm2 = b, d */               \
        punpcklqdq xmm2, xmm1


        /* xmm0: a, c
         * xmm1: b, d
         * s - b's shift
         * t - d's shift
         * xmm2, xmm3, xmm4: clobbered
         */
#define HALF_ROUND(s, t)	        \
        /* xmm0 = a, c; xmm1 = b, d */	\
        /* a += b; c += d */            \
        paddq %xmm1, %xmm0;             \
        ROTQ(%xmm1, s, t, %xmm2, %xmm3, %xmm4); \
                                        \
        /* b ^= a; d ^= c */            \
        pxor %xmm0, %xmm1;              \
        /* xmm0 = c, ROTATE(a, 32) */   \
        /* pshufd $0b 0001 1110, %xmm0, %xmm0; */ \
        pshufd $0x1E, %xmm0, %xmm0;

#define DOUBLE_ROUND()                  \
        /* xmm0 = a(lo), b(hi) */       \
        /* xmm1 = c(lo), d(hi) */       \
                                        \
        /* xmm0 = a, c; xmm1 = b, d */  \
        movdqa %xmm1, %xmm3;            \
        movdqa %xmm0, %xmm1;            \
        punpcklqdq %xmm3, %xmm0;        \
        punpckhqdq %xmm3, %xmm1;        \
                                        \
        HALF_ROUND(13, 16);             \
        HALF_ROUND(17, 21);             \
        HALF_ROUND(13, 16);             \
        HALF_ROUND(17, 21);             \
                                        \
        /* xmm0 = a, c; xmm1 = b, d */  \
        movdqa %xmm1, %xmm3;            \
        movdqa %xmm0, %xmm2;            \
        punpcklqdq %xmm3, %xmm0;        \
        punpckhqdq %xmm1, %xmm2;        \
        movdqa %xmm2, %xmm1

.align 16, 0x90
.globl siphash24_half_round_asm
.globl _siphash24_half_round_asm
siphash24_half_round_asm:
_siphash24_half_round_asm:
        push %ebp
        mov %esp, %ebp
        sub $0x30, %esp

        mov 0x08(%ebp), %eax
        mov 0x0c(%ebp), %ecx
        movdqu 0x00(%eax), %xmm0
        movdqu 0x10(%eax), %xmm1

        DOUBLE_ROUND()

        movdqu %xmm0, 0x00(%ecx)
        movdqu %xmm1, 0x10(%ecx)

        leave
        ret


.align 16, 0x90
.globl siphash24_asm
siphash24_asm:
        push %ebp
        mov %esp, %ebp
        sub $0x30, %esp

        /* 8(%ebp) -> const void *src
         * 12(%ebp) -> unsigned long src_sz
         * 16(%ebp) -> char key[16];
         * neither src and key don't need to be aligned
         */
        mov 0x10(%ebp), %eax
        movdqu (%eax), %xmm4	# load 128 key k0, k1

        /* Prologue */
        movdqa INITV0V1, %xmm0	# v0, v1
        movdqa INITV2V3, %xmm1	# v2, v3
        pxor %xmm4, %xmm0       # v0^k0, v1^k1
        pxor %xmm4, %xmm1       # v2^k0, v3^k1

        /* Intermediate rounds */

        /* Finalization */
        pxor %xmm2, %xmm2       # 0x0, b = 0
        pxor %xmm2, %xmm1       # v3^b

        DOUBLE_ROUND()

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
