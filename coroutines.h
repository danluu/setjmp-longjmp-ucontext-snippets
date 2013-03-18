typedef void(*coro_callback)(void* user_state);

void coro_yield(int pid);
int coro_spawn(coro_callback f, void* user_state);
void coro_allocate(int num_coros);
int coro_runnable(int pid);

extern int coro_pid;
