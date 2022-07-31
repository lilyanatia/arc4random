#include <cpuid.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/random.h>

#if defined(__AES__)
#include "aes-stream/src/aes-stream.h"
#endif

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
#if defined(__AES__)
  unsigned char seed[AES_STREAM_SEEDBYTES];
  aes_stream_state st;
#endif
  if(nbytes <= 256) while(getrandom(buf, nbytes, 0) < nbytes);
  else
  {
#if defined(__AES__)
    uint32_t eax, ebx, ecx, edx;
    __get_cpuid(1, &eax, &ebx, &ecx, &edx);
    if(ecx & bit_AES)
    {
      while(getrandom(seed, AES_STREAM_SEEDBYTES, 0) < AES_STREAM_SEEDBYTES);
      aes_stream_init(&st, seed);
      aes_stream(&st, buf, nbytes);
    }
    else
#endif
    {
      unsigned char rnd[KEYSZ + IVSZ];
      chacha_ctx ctx;
      while(getrandom(rnd, KEYSZ + IVSZ, 0) < KEYSZ + IVSZ);
      chacha_keysetup(&ctx, buf, KEYSZ * 8, 0);
      chacha_ivsetup(&ctx, buf + KEYSZ);
      while(nbytes)
      {
        chacha_encrypt_bytes(&ctx, buf, buf, nbytes > UINT32_MAX ? UINT32_MAX : nbytes);
        buf += nbytes > UINT32_MAX ? UINT32_MAX : nbytes;
        nbytes -= nbytes > UINT32_MAX ? UINT32_MAX : nbytes;
      }
    }
  }
}

__attribute__((visibility("default"))) uint32_t arc4random_uniform(uint32_t upper_bound)
 
{
  #define LENGTH 256
  const uint32_t limit = upper_bound < UINT32_MAX >> 1 ? UINT32_MAX - UINT32_MAX % upper_bound : upper_bound;
  uint32_t out[LENGTH / sizeof(uint32_t)] = { 0 };
  for(;;)
  {
    while(getrandom(out, LENGTH, 0) < LENGTH);
    for(int i = 0; i < LENGTH / sizeof(uint32_t); ++i)
      if(out[i] < limit) return limit == upper_bound ? out[i] : out[i] % upper_bound;
  }
}
