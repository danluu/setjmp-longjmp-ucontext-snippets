#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#  define ALIGN_LOG
#endif

#ifdef ALIGN_LOG
#  define ALIGN(log) .align (log), 0x90;
#else
#  define ALIGN(log) .align 1 << (log), 0x90;
#endif

ALIGN(5)
INITV0V2:
        .ascii "uespemosarenegyl"
INITV1V3:
        .ascii "modnarodsetybdet"

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
        /* a = c, c = ROTATE(a, 32) */   \
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

#ifdef DRY_RUN
# undef DOUBLE_ROUND
# define DOUBLE_ROUND()
#endif

ALIGN(5)
.globl siphash24_half_round_asm
.globl _siphash24_half_round_asm
siphash24_double_round_asm:
_siphash24_double_round_asm:
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


ALIGN(5)
.globl siphash24_asm
.globl _siphash24_asm
siphash24_asm:
_siphash24_asm:
        push %ebp
        mov %esp, %ebp
        mov %esi, -0x4(%ebp)
        mov %edi, -0x8(%ebp)

        /* 0x08(%ebp) -> const void *src
         * 0x0c(%ebp) -> unsigned long src_sz
         * 0x10(%ebp) -> char key[16];
         * neither src and key may not be aligned */

        /* Stack frame:
         *   8 - -0x4(%ebp) esi, -0x8(%ebp) edi
         *  16 - alignment
         *  24 - 0x0(%esp) scratch area */
        sub $0x30, %esp
        and $0xfffffff0, %esp

        /* Load key */
        mov 0x10(%ebp), %eax
        movdqu (%eax), %xmm0    # k0, k1
        movdqu %xmm0, %xmm1
        punpcklqdq %xmm0, %xmm0 # k0, k0
        punpckhqdq %xmm1, %xmm1 # k1, k1

        /* Preamble */
        call .l0
.l0:    pop %ecx                # ecx = eip
        pxor INITV0V2-.l0(%ecx), %xmm0       # v0^k0, v2^k0
        pxor INITV1V3-.l0(%ecx), %xmm1       # v1^k1, v3^k1

        mov 0x08(%ebp), %esi    # esi = src
        mov 0x0c(%ebp), %eax    # eax = src_sz
        mov %eax, %edx          # edx = src_sz
        shr $3, %edx            # edx /= 8
        mov %eax, %ecx          # ecx = src_sz
        and $0x7, %ecx          # ecx %= 8

        movd %eax, %xmm7
        psllq $56, %xmm7        # size in top byte of eax

        /* Compression rounds */
        /* powers of 16? */
        cmp $2, %edx
        jl srcsz_lt_16
ALIGN(4)
srcsz_gte_16:
        movdqu (%esi), %xmm6       # xmm6 = m0, m1
        add $16, %esi
        sub $2, %edx
        movdqu %xmm6, %xmm5
        pslldq $8, %xmm5           # 0, m0
        psrldq $8, %xmm6           # m1, 0
        pslldq $8, %xmm6           # 0, m1

        pxor %xmm5, %xmm1       # v3 ^= m0
        DOUBLE_ROUND()
        psrldq $8, %xmm5
        pxor %xmm5, %xmm0       # v0 ^= m0

        pxor %xmm6, %xmm1       # v3 ^= m1
        DOUBLE_ROUND()
        psrldq $8, %xmm6
        pxor %xmm6, %xmm0       # v0 ^= m1

        cmp $2, %edx
        jge srcsz_gte_16

srcsz_lt_16:
        /* remainding 8 bytes */
        cmp $0, %edx
        je srcsz_lt_8
srcsz_gte_8:
        movq (%esi), %xmm6         # xmm6_lo = m
        pshufd $0x4e, %xmm6, %xmm5 # xmm5_hi = m
        add $8, %esi
        dec %edx

        pxor %xmm5, %xmm1       # v3 ^= m
        DOUBLE_ROUND()
        pxor %xmm6, %xmm0       # v0 ^= m

//        cmp $0, %edx
//        jg srcsz_gte_8
srcsz_lt_8:

        /* load the remainder (max 7 bytes) to xmm6 */
        cmp $6, %ecx
        jg eq7
        je eq6
        cmp $4, %ecx
        jg eq5
        je eq4
        cmp $2, %ecx
        jg eq3
        je eq2
        cmp $0, %ecx
        jg eq1
ex0:
        pxor %xmm6, %xmm6
        jmp done
eq7:
        movl   0(%esi), %eax
        movzwl 4(%esi), %edx
        movzbl 6(%esi), %ecx
        shl $16, %ecx
        xor %ecx, %edx
        movd %eax, %xmm6
        movd %edx, %xmm5
        psllq $32, %xmm5
        pxor %xmm5, %xmm6
        jmp done
eq5:
        movl   0(%esi), %eax
        movzbl 4(%esi), %edx
        movd %eax, %xmm6
        movd %edx, %xmm5
        psllq $32, %xmm5
        pxor %xmm5, %xmm6
        jmp done
eq6:
        movzwl 4(%esi), %edx
eq4:
        movl   0(%esi), %eax
        movd %eax, %xmm6
        movd %edx, %xmm5
        psllq $32, %xmm5
        pxor %xmm5, %xmm6
        jmp done
eq3:
        movzwl 0(%esi), %eax
        movzbl 2(%esi), %ecx
        shl $16, %ecx
        xor %ecx, %eax
        movd %eax, %xmm6
        jmp done
eq2:
        movzwl 0(%esi), %eax
        movd %eax, %xmm6
        jmp done
eq1:
        movzbl 0(%esi), %eax
        movd %eax, %xmm6
done:
        pxor %xmm6, %xmm7
        movdqa %xmm7, %xmm6

        pshufd $0x4e, %xmm6, %xmm5      # xmm5_hi = b

        /* Padding round */
        pxor %xmm5, %xmm1       # v3 ^= b
        DOUBLE_ROUND()
        pxor %xmm6, %xmm0       # v0 ^= b

        /* load 0xff to xmm4:hi */
        pcmpeqw %xmm7, %xmm7
        psrlq $56, %xmm7
        pshufd $0x45, %xmm7, %xmm7
        pxor %xmm7, %xmm0       # v2 ^= 0xff

        DOUBLE_ROUND()
        DOUBLE_ROUND()

        /* Epilogue */
        pxor %xmm1, %xmm0       # xmm0 = v0^v1, v2^v3
        movdqa %xmm0, %xmm1
        punpckhqdq %xmm0, %xmm0	# the same as psrldq $8, %xmm0
        pxor %xmm1, %xmm0	# xmm0_lo is the result
        movq %xmm0, (%esp)
        mov 0x4(%esp), %edx
        mov 0x0(%esp), %eax

        mov -0x4(%ebp), %esi
        mov -0x8(%ebp), %edi
        leave
        ret
