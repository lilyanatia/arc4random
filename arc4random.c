#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/param.h>
#include <sys/random.h>

#define GETRANDOM_SIZE 256
#define REKEY_INTERVAL 0x1000000

__attribute__((visibility("default"))) uint32_t arc4random(void)
{
  uint32_t out;
  while(getrandom(&out, sizeof(out), 0) < sizeof(out));
  return out;
}

__attribute__((visibility("default"))) void arc4random_buf(void *buf, size_t nbytes)
{
  for(ssize_t length = 0; nbytes; length = MAX(0, getrandom(buf += length, MIN(nbytes -= length, SSIZE_MAX), 0)));
}

__attribute__((visibility("default"))) uint32_t arc4random_uniform(uint32_t upper_bound)
 
{
  const uint32_t limit = upper_bound < UINT32_MAX >> 1 ? UINT32_MAX - UINT32_MAX % upper_bound : upper_bound;
  uint32_t out[GETRANDOM_SIZE / sizeof(uint32_t)] = { 0 };
  for(;;)
  {
    while(getrandom(out, GETRANDOM_SIZE, 0) < GETRANDOM_SIZE);
    for(int i = 0; i < GETRANDOM_SIZE / sizeof(uint32_t); ++i)
      if(out[i] < limit) return limit == upper_bound ? out[i] : out[i] % upper_bound;
  }
}
