#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/random.h>

#define GETRANDOM_SIZE 256
#define REKEY_INTERVAL 0x1000000

#define KEYSTREAM_ONLY
#include "chacha_private.h"
#define KEYSZ 32
#define IVSZ 8

__attribute__((visibility("default"))) uint32_t arc4random(void)
{
  uint32_t out;
  while(getrandom(&out, sizeof(out), 0) < sizeof(out));
  return out;
}

__attribute__((visibility("default"))) void arc4random_buf(void *buf, size_t nbytes)
{
  if(nbytes <= GETRANDOM_SIZE) while(getrandom(buf, nbytes, 0) < nbytes);
  else
  {
    unsigned char rnd[KEYSZ + IVSZ];
    chacha_ctx ctx;
    while(getrandom(rnd, KEYSZ + IVSZ, 0) < KEYSZ + IVSZ);
    while(nbytes)
    {
      size_t length;
      chacha_keysetup(&ctx, rnd, KEYSZ * 8, 0);
      chacha_ivsetup(&ctx, rnd + KEYSZ);
      chacha_encrypt_bytes(&ctx, rnd, rnd, KEYSZ + IVSZ);
      chacha_encrypt_bytes(&ctx, buf, buf, length = nbytes > REKEY_INTERVAL ? REKEY_INTERVAL : nbytes);
      buf += length;
      nbytes -= length;
    }
  }
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
