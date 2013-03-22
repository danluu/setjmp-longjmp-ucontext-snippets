uint64_t siphash24_asm(const void *src, unsigned long src_sz, const char key[16]);

char *inputs[] = {
	"a",
	"abcdefg",
	"abcdefgabcdefg",
	"abcdefgabcdefgabcdefg",
	NULL
};


#define ROTATE(x, b) (uint64_t)( ((x) << (b)) | ( (x) >> (64 - (b))) )

#define HALF_ROUND(a,b,c,d,s,t)			\
	a += b; c += d;				\
	b = ROTATE(b, s) ^ a;			\
	d = ROTATE(d, t) ^ c;			\
	a = ROTATE(a, 32);



	/* int i; */
	/* char key[16] = {0,1,2,3,4,5,6,7, */
	/* 		0,1,2,3,4,5,6,7}; */

	/* char *a = "abcdefgh"; */
	/* printf("0x%016llx\n", siphash24(a, 0, key)); */
	/* printf("0x%016llx\n", siphash24_asm(a, 0, key)); */

	/* for (i=0; inputs[i]; i++) { */
	/* 	char *v = inputs[i]; */
	/* 	assert(siphash24(v, strlen(v), key) == */
	/* 	       siphash24_asm(v, strlen(v), key)); */
	/* } */
	/* return 0; */


	printf("hr:\n");
	printf("lo=%016llx hi=%016llx\n", a, b);
	printf("lo=%016llx hi=%016llx\n", c, d);

	uint64_t in[4] = {1, 2, 3, 4};
	uint64_t out[4] = {0,0,0,0};
	siphash24_half_round_asm(in, out);
	printf("asm:\n");
	printf("lo=%016llx hi=%016llx\n", out[0], out[1]);
	printf("lo=%016llx hi=%016llx\n", out[2], out[3]);
	/* printf("abc\n"); */
	/* printf("   %016llx = ROT(0x01, 61) \n", ROTATE(0xffULL, 63)); */


uint64_t siphash24(const void *src, unsigned long src_sz, const char key[16]);
