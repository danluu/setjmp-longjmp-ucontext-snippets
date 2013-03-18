
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "coroutines.h"
#include "channels.h"

int* runnable_pids;
int coro_max;

static void mark_unrunnable(int);
static void mark_runnable(int);

struct channel {
	int val;
	int sender;
	int receiver;
};
	
void channels_scheduler(void)
{
	while (1)
		{
			int p;
			int counter = 0;
			for(p = 0; p < coro_max; p++)
				{
					if (runnable_pids[p])
						{
							counter++;
							coro_yield(p);
							if (!coro_runnable(p))
								{
									mark_unrunnable(p);
								}
						}
				}
			if (counter == 0)
				break;
		}
	printf("channels scheduler done!\n");
}

struct channel *channel_new(void)
{
	struct channel* ch = malloc(sizeof(struct channel));
	ch->val = 0;
	ch->sender = -1;
	ch->receiver = -1;

	return ch;
}

void channel_send(struct channel *ch, int val)
{
	assert(ch->sender == -1);
	ch->val = val;
	ch->sender = coro_pid;
	if (ch->receiver != -1)
		{
			mark_runnable(ch->receiver);
		}
	mark_unrunnable(coro_pid);
	coro_yield(0);
}

int channel_recv(struct channel *ch)
{
	assert(ch->receiver == -1);
	ch->receiver = coro_pid;
	while (1)
		{
			if (ch->sender != -1)
				{
					int val = ch->val;
					mark_runnable(ch->sender);
					ch->sender = ch->receiver = -1;
					return val;
				}
			mark_unrunnable(coro_pid);
			coro_yield(0);
		}
}

void channels_allocate(int num_coros)
{
	coro_max = num_coros;
	coro_allocate(coro_max);
	runnable_pids = calloc(coro_max, sizeof(int));
}

int channels_spawn(coro_callback f, void* user_state)
{
	int pid = coro_spawn(f, user_state);
	mark_runnable(pid);
	return pid;
}

static void mark_runnable(int pid)
{
	assert(runnable_pids[pid] == 0);
	runnable_pids[pid] = 1;
}

static void mark_unrunnable(int pid)
{
	assert(runnable_pids[pid] == 1);
	runnable_pids[pid] = 0;
}
