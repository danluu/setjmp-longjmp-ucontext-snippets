CC=gcc

.PHONY: test coroutine channel setjmp exception coop heapros assembly assembly64 sigaltstack

sigaltstack:
	$(CC) -Wall -O -Wno-unused-parameter -g -ggdb \
		sigaltstack.c -o sigaltstack && ./sigaltstack

assembly64:
	$(CC) -Wall -Wextra -Wno-unused-parameter -g -ggdb \
		assembly64.c  assembly_stuff_64.S -o assembly64 && ./assembly64

assembly:
	$(CC) -m32 -Wall -Wextra -Wno-unused-parameter -g -ggdb \
		assembly.c  assembly_stuff.S -o assembly && ./assembly

heapros:
	$(CC) -Wall -Wextra -Wno-unused-parameter -g -ggdb3 \
		heapros.c -o heapros && ./heapros

channel:
	$(CC) -U_FORTIFY_SOURCE -Wall -Wextra -Wno-unused-parameter -g -ggdb \
		channels.c coroutines.c channel_test.c -o channel && ./channel

coroutine:
	$(CC) -U_FORTIFY_SOURCE -Wall -Wextra -Wno-unused-parameter -g -ggdb \
		coroutines.c coroutine_test.c -o coroutine && ./coroutine

test:
	$(CC) -O3 -Wall -Wextra -g -ggdb \
		stack_engine_ubenchmark.c -o test && ./test

setjmp:
	$(CC) -U_FORTIFY_SOURCE -Wall -Wextra -Wno-unused-parameter -g -ggdb \
		setjmp.c -o setjmp && ./setjmp

exception:
	$(CC) -U_FORTIFY_SOURCE -Wall -Wextra -Wno-unused-parameter -g -ggdb \
		exception.c -o exception && ./exception

coop:
	$(CC) -U_FORTIFY_SOURCE -Wall -Wextra -Wno-unused-parameter -g -ggdb \
		coop.c -o coop && ./coop
