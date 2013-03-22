
.align 16, 0x00
INITV0V1:
        .ascii "uespemosmodnarod"
        # .ascii "somepseudorandom"
        # .octa 0x736f6d6570736575646f72616e646f6d
INITV2V3:
        .ascii "arenegylsetybdet"
        # .ascii "lygeneratedbytes"
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

        /* Push eip to ecx */
        call .l0
.l0:    pop %ecx

        /* 0x08(%ebp) -> const void *src
         * 0x0c(%ebp) -> unsigned long src_sz
         * 0x10(%ebp) -> char key[16];
         * neither src and key don't need to be aligned
         */
        mov 0x10(%ebp), %eax
        movdqu (%eax), %xmm4	# load 128 key k0, k1
        pshufd $0x4e, %xmm4, %xmm4

        /* Preamble */
        movdqa INITV0V1-.l0(%ecx), %xmm0
        pxor %xmm4, %xmm0       # v0^k0, v1^k1
        movdqa INITV2V3-.l0(%ecx), %xmm1
        pxor %xmm4, %xmm1       # v2^k0, v3^k1

        mov 0x08(%ebp), %edx    # edx = src
        mov 0x0c(%ebp), %ecx    # eax = src_sz
        mov %ecx, %eax          # ecx = src_sz

        ROTATE(%xmm0, %xmm1, %xmm3)

        /* Compression rounds */
        cmp $8, %ecx
        jl srcsz_lt_8
srcsz_gte_8:
        movq (%edx), %xmm6       # xmm6(lo) = m
        add $8, %edx
        sub $8, %ecx

        pshufd $0x4e, %xmm6, %xmm7
        pxor %xmm7, %xmm1       # v3 ^= m

        DOUBLE_ROUND()
        pxor %xmm6, %xmm0       # v0 ^= m

        cmp $8, %ecx
        jge srcsz_gte_8
srcsz_lt_8:

        /* Padding round */
        # eax = src_sz
        movd %eax, %xmm5
        pslldq $15, %xmm5       # 15 bytes, 56+64 bits

        // add t
        pshufd $0x4e, %xmm5, %xmm7
        pxor %xmm5, %xmm1       # v3^b

        DOUBLE_ROUND()

        /* load 0xff to xmm4:hi */
        pcmpeqw %xmm5, %xmm5
        psrlq $56, %xmm5
        pshufd $0x45, %xmm5, %xmm5
        pxor %xmm7, %xmm0       # v0 ^= b
        pxor %xmm5, %xmm0       # v2 ^= 0xff

        DOUBLE_ROUND()
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

return_xmm0_lo:
        punpcklqdq %xmm0, %xmm0
        movq %xmm0, (%esp)
        mov 0x4(%esp), %edx
        mov 0x0(%esp), %eax
        leave
        ret
