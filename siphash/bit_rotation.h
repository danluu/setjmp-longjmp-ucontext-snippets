#define VECTORS 64

void print_matrix(uint64_t data[VECTORS]);
void bit_rotate(const uint8_t input[8*VECTORS], uint64_t output[VECTORS]);
void bit_rotate_std(uint8_t input[8*VECTORS], uint64_t output[VECTORS]);
