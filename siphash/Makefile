CC=gcc

all:
	$(CC) -O3 -Wall -Wextra -g -ggdb -fstack-protector-all \
		-mmmx -msse -msse2 -m32 \
		-Wno-unused-parameter -D_FORTIFY_SOURCE=2 \
		main.c csiphash.c asmsiphash.S -o test && ./test