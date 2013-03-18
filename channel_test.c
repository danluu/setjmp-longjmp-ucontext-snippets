/* -*-mode:c; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#include <stdio.h>
#include <assert.h>

#include "coroutines.h"
#include "channels.h"

static void test_writer(void*);
static void test_reader(void*);

int main()
{
  channels_allocate(10);
  printf("main: coro_allocate finished\n");
  struct channel *ch = channel_new();
  channels_spawn(test_writer, ch);
  channels_spawn(test_reader, ch);
  channels_scheduler();
  printf("main: finished\n");
  return 0;
}

static void test_writer(void* _ch)
{
  coro_yield(0);
  struct channel *ch = _ch;
  int i = 0;
  while (1)
    {
      channel_send(ch, i++);
    }
}

static void test_reader(void* ch)
{
  coro_yield(0);
  int read_val = channel_recv(ch);
  printf("test_reader: %i\n", read_val);

  read_val = channel_recv(ch);
  printf("test_reader: %i\n", read_val);

  read_val = channel_recv(ch);
  printf("test_reader: %i\n", read_val);
}

