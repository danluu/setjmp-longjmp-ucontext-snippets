#include <stdint.h>
#include <stdio.h>

#include <emmintrin.h>
#include "bit_rotation.h"

void print_matrix(uint64_t data[VECTORS]) {
	int i, j;

	for(i = 0; i < 8; i++) {
		for(j = 0; j < 8; j++){
			printf("%016llx ", data[i*8+j]);
		}
		printf("\n");
	}
}


union xmm {
	__m128i x;
	uint8_t b[16];
};

void bit_rotate(const uint8_t input[8*VECTORS], uint64_t output[VECTORS]) {
	int i, j, k, b;

	uint16_t *o = (uint16_t*)&output[0];

	for (b = 0; b < 8; b++) {
		for (i = 0; i < VECTORS/16; i++) {
			union xmm x;
			for (j = 0; j < 16; j++) {
				register int input_num = i*16+j;
				x.b[j] = input[(input_num)*8+b];
			}
			for (k = 7; k >= 0; k--) {
				register uint16_t v = _mm_movemask_epi8(x.x);
				x.x = _mm_slli_epi64(x.x, 1);
				int bit_num = b*8 + k;
				o[bit_num*4 + i] = v;
			}
		}
	}
}


static void btrans(uint8_t const *inp, uint8_t *out, int nrows, int ncols)
{
#   define Bytes(x) (((x) + 7) >> 3)
	int     rr, cc, i, cb = Bytes(ncols), rb = Bytes(nrows), left = nrows & 15;
	union { __m128i x; uint8_t b[16]; } minp;
#   define DoBlock(Rows, Bits)						\
        for (cc = 0; cc < ncols; cc += 8 ) {				\
		for (i = 0; i < Rows; ++i)				\
			minp.b[i] = inp[(rr + i)*cb + (cc >> 3)];	\
		for (i = 8; --i >= 0; ) {				\
			*(uint##Bits##_t*)&out[(rr >> 3) + (cc + i)*rb] = \
				_mm_movemask_epi8(minp.x);		\
			minp.x = _mm_slli_epi64(minp.x, 1);		\
		}							\
        }

	for (rr = 0; rr <= nrows - 16; rr += 16)
		DoBlock(16, 16);
	if (left > 8 ) {
		DoBlock(left, 16);
	} else if (left > 0) {
		DoBlock(left,  8 );
	}
}


void bit_rotate_std(uint8_t input[8*VECTORS], uint64_t output[VECTORS]) {
	btrans(input, (uint8_t*)output, VECTORS, 64);
}
