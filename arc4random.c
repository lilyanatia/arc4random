#include <limits.h>
#include <stddef.h>
#include <stdint.h>
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
  while(nbytes)
  { ssize_t length = getrandom(buf, nbytes, 0);
    if(length < 0) length = 0;
    buf += length;
    nbytes -= length;
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

#include <float.h>
#include <math.h>

#define DBL_EXP_DIG (sizeof(double) * CHAR_BIT - DBL_MANT_DIG)
#define DBL_MANT_BITS (DBL_MANT_DIG - 1)               // 52
#define DBL_BASE_EXP  (DBL_MAX_EXP  - 2ul)             // 1022
#define DBL_ALT_EXP   (DBL_BASE_EXP - DBL_EXP_DIG - 1) // 1010

#if(UINT_MAX >= UINT64_MAX)
#define ctz64(n) (uint64_t)__builtin_ctz(n)
#elif(ULONG_MAX >= UINT64_MAX)
#define ctz64(n) (uint64_t)__builtin_ctzl(n)
#else
#define ctz64(n) (uint64_t)__builtin_ctzll(n)
#endif

__attribute__((visibility("default"))) double arc4random_double(void)
{
  struct
  {
    double out;
    uint64_t extra[(int)ceil(DBL_ALT_EXP / 64.0)];
  } random;
  arc4random_buf(&random, sizeof(random));
  uint64_t *const p = (uint64_t *)&random.out;
  uint32_t r = *p >> DBL_MANT_BITS;
  if(r)
  {
    *p &= (1ul << DBL_MANT_BITS) - 1;
    *p |= DBL_BASE_EXP << DBL_MANT_BITS;
  }
  else
  {
    *p |= DBL_ALT_EXP << DBL_MANT_BITS;
    for(int i = DBL_ALT_EXP / 64; i; --i)
    {
      if(r = random.extra[i]) goto done;
      *p -= 64ul << DBL_MANT_BITS;
    }
    r = random.extra[0] | 1ul << (DBL_ALT_EXP % 64);
  }
  done:
  *p -= ctz64(r) << DBL_MANT_BITS;
  return random.out;
}
