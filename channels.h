struct channel;

void channels_scheduler(void);
struct channel *channel_new(void);
void channel_send(struct channel *ch, int);
int channel_recv(struct channel *ch);
void channels_allocate(int num_coros);
int channels_spawn(coro_callback f, void* user_state);
