int main() {
	uint8_t input[8*VECTORS];
	uint64_t output[VECTORS];
	uint64_t output2[VECTORS];

	memset(input, 0, sizeof(input));

	int i, j;
	for(i = 0; i < VECTORS; i++){
		input[i*8] = 0x80;
	}

	input[0] = 0x0A;

	bit_rotate(input, output);
	bit_rotate((uint8_t*)output, output2);

	int r = memcmp(input,output2,8*VECTORS);
	printf("memcmp result: %i\n", r);


	print_matrix((uint64_t*)input);
	printf("\n");
	print_matrix(output);
	printf("\n");
	print_matrix(output2);
	return 0;
}
