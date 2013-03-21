- coroutines.*: Implementation of coroutines using setjmp and longjmp
- coroutine_test.c: Simple usage of above
- heapros.c: Implementaion of coroutines using setjmp and longjmp, with stack copying to heap
- channels.*: Implementation of channels, on top of coroutines.*
- channel_test.c: Sieve of Eratosthenes using channels
- stack_engine_ubenchmark.c: Silly benchmark to figure out if manually messing with stack will cause performance degradation, by causing sync uops to be inserted to keep Intel's "stack engine" in sync with back-end stack registers
- setjmp.c, exception.c, coop.c: Setjmp/longjmp examples from wikipedia




Run:

    git submodule update --init
