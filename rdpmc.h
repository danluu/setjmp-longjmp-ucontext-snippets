#define rdpmc(counter,l,h) \
  __asm__ volatile("rdpmc" : "=a" (l), "=d" (h) : "c" (counter))
