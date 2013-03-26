#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <emmintrin.h>

#define VECTORS 64

static void bit_rotate(uint8_t input[8*VECTORS], uint64_t output[VECTORS]);

int main() {
  uint8_t input[8*VECTORS];
  uint64_t output[VECTORS];
  uint64_t output2[VECTORS];

  memset(input, 0, sizeof(input));
  
  int i = 0;
  for(i = 0; i < VECTORS; i++){
    input[i*8] = 0x80;
  }
  
  input[78] = 0xA5;

  bit_rotate(input, output);
  bit_rotate((uint8_t*)output, output2);

  int r = memcmp(input,output2,8*VECTORS);
  printf("output[0]: %llx\n", output[0]);
  printf("memcmp result: %i\n", r);

  return 0;
}

union xmm {
  __m128i x;
  uint8_t b[16];
};

static void bit_rotate(uint8_t input[8*VECTORS], uint64_t output[VECTORS]) {
  int i, j, k, b;

  uint16_t* o = (uint16_t*)&output[0];

  for(b = 0; b < 8; b++) {
    for(i = 0; i < VECTORS/16; i++) {    
      union xmm x;
      for(j = 0; j < 16; j++) {
	x.b[j] = input[(i*16+j)*8+b];
      }
      for(k = 0; k < 8; k++){
	uint16_t v = _mm_movemask_epi8(x.x);
	x.x = _mm_slli_epi64(x.x, 1);
	o[(b*8 + k)*4 + i] = v;
      }
    }
  }

}
