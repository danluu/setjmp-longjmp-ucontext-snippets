/* -*-mode:c; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#include <stdio.h>
#include <assert.h>

#include "coroutines.h"
#include "channels.h"

static void counter(void*);
static void print_primes(void*);

int main() {
  channels_allocate(10);
  channels_spawn(print_primes, NULL);
  channels_scheduler();
  return 0;
}

static void counter(void* _ch) {
  coro_yield(0);
  struct channel *ch = _ch;
  int i = 2;
  while (1) {
    channel_send(ch, i++);
  }
}

struct filter_params {
  int prime;
  struct channel *recv;
  struct channel *send;
};

static void filter(void *_params)
{
  struct filter_params *params = _params;
  int prime = params->prime;
  struct channel *recv = params->recv;
  struct channel *send = params->send;
  coro_yield(0);
  while (1) {
    int i = channel_recv(recv);
    if (i % prime) {
      channel_send(send, i);
    }
  }
}

static void sieve(void *_ch) {
  struct channel *primes = _ch;
  struct channel *c = channel_new();
  channels_spawn(counter, c);
  while (1) {
    int p = channel_recv(c);
    channel_send(primes, p);
    struct channel *newc = channel_new();
    struct filter_params fp = { p, c, newc };
    channels_spawn(filter, &fp);
    c = newc;
  }
}

static void print_primes(void* _) {
  int i = 0;

  coro_yield(0);
  struct channel *primes = channel_new();
  channels_spawn(sieve, primes);

  for(i = 0; i < 8; i++) {
    printf("%d\n", channel_recv(primes));
  }
}

