This was originally a one-day project to explore implementing concurrency in C. This is all exploratory code which isn't even close to production ready. On the first day, we tried implementing the most naive possible implemenation of coroutines, allocating a fixed amount of extra stack space, fixing a particular amount of stack space for each coroutine, giving each coroutine EXTRA_SPACE / N stack space, for some fixed N. 

On top of that, we implemented channels (for something resembling Go style concurrency), again using the dumbest possible implementation. A send or receive on a channel simply marks a coroutine unrunnable in the channel scheduler until the channel sees a receive or a send, respectively, and the value sent over the channel is simply a heap allocated struct.

A few days later, we spent half a day creating the second most naive coroutine implementation possible: allocate space on the heap for each coroutine, and swap stacks when switching between coroutines. Turns out, this is what the Julia language does, so this idea turned out to be more practical than we realized.

A possible improvement would be to really allocate a different stack for each coroutine. The most obvious way to do that would be to use ucontext, but that doesn't appear to have good cross-platform support. In particular, the Mac OS X implementation seems to be fundamentally broken. I played around with sigaltstack as an alternative. That ought to work, and I may try implementing coroutines on top of sigaltstack at some point.


- coroutines.*: Implementation of coroutines using setjmp and longjmp
- coroutine_test.c: Simple usage of above
- heapros.c: Implementaion of coroutines using setjmp and longjmp, with stack copying to heap
- channels.*: Implementation of channels, on top of coroutines.*
- channel_test.c: Sieve of Eratosthenes using channels
- stack_engine_ubenchmark.c: Silly benchmark to figure out if manually messing with stack will cause performance degradation, by causing sync uops to be inserted to keep Intel's "stack engine" in sync with back-end stack registers
- sigaltstack.c: Use sigaltstack to change stack
- setjmp.c, exception.c, coop.c: Setjmp/longjmp examples from wikipedia




Run:

    git submodule update --init
