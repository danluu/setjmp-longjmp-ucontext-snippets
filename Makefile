CC=gcc

.PHONY: test coroutine channel

channel:
	$(CC) -O -Wall -Wextra -Wno-unused-parameter -g -ggdb \
		channels.c coroutines.c channel_test.c -o channel && ./channel

coroutine:
	$(CC) -O -Wall -Wextra -Wno-unused-parameter -g -ggdb \
		coroutines.c coroutine_test.c -o coroutine && ./coroutine

test:
	$(CC) -O3 -Wall -Wextra -g -ggdb \
		stack_engine_ubenchmark.c -o test && ./test
