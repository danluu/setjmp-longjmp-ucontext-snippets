CC=gcc

.PHONY: test coroutine

coroutine:
	$(CC) -O -Wall -Wextra -g -ggdb \
		coroutines.c coroutine_test.c -o coroutine && ./coroutine

test:
	$(CC) -O3 -Wall -Wextra -g -ggdb \
		stack_engine_ubenchmark.c -o test && ./test
