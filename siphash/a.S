        movdqa %xmm1, %xmm2;            \
        punpckhqdq %xmm2, %xmm2;        \

        movdqa xmm1, xmm3;	\
        psllq $0+s, xmm1;        \
        psrlq $64-s, xmm3;	\
        pxor xmm3, xmm1

        movdqa xmm2, xmm4;	\
        psllq $0+t, xmm2;        \
        psrlq $64-t, xmm4;	\
        pxor xmm4, xmm2

        punpcklqdq %xmm2, %xmm1;        \


        

        movdqa xmm1, xmm2
        punpcklqdq zero, xmm1; // b
        punpckhqdq zero, xmm2  // d

        movdqa xmm1, xmm3
        movdqa xmm2, xmm4

        psllq $0+s, xmm1;        \
        psrlq $64-s, xmm3;	\
        psllq $0+t, xmm2;        \
        psrlq $64-t, xmm4;	\

        pxor xmm3, xmm1
        pxor xmm4, xmm2
        pxor xmm2, xmm1
