#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/random.h>

__attribute__((visibility("default"))) uint32_t arc4random(void)
{
  uint32_t out;
  while(__builtin_expect(getrandom(&out, sizeof(out), 0) < sizeof(out), 0));
  return out;
}

__attribute__((visibility("default"))) void arc4random_buf(void *buf, size_t nbytes)
{
  while(nbytes)
  { ssize_t length = getrandom(buf, nbytes, 0);
    if(__builtin_expect(length < 0, 0)) length = 0;
    buf += length;
    nbytes -= length;
  }
}

__attribute__((visibility("default"))) uint32_t arc4random_uniform(uint32_t upper_bound)
 
{
  uint32_t out, limit = UINT32_MAX - UINT32_MAX % upper_bound;
  for(;;)
  {
    while(__builtin_expect(getrandom(&out, sizeof(out), 0) < sizeof(out), 0) || out >= limit);
    return out % upper_bound;
  }
}

#include <float.h>

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
  double out;
  arc4random_buf(&out, sizeof(out));
  uint64_t *const p = (uint64_t *)&out;
  uint64_t r = *p >> DBL_MANT_BITS;
  *p &= (1ul << DBL_MANT_BITS) - 1;
  *p |= DBL_BASE_EXP << DBL_MANT_BITS;
  if(__builtin_expect(!r, 0))
  {
    uint64_t extra;
    for(int i = DBL_ALT_EXP / (CHAR_BIT * sizeof(extra)); i; --i)
    {
      arc4random_buf(&extra, sizeof(extra));
      if(!__builtin_expect(!(r = extra), 0)) goto done;
      *p -= (CHAR_BIT * sizeof(extra)) << DBL_MANT_BITS;
    }
    arc4random_buf(&extra, sizeof(extra));
    r = extra | 1ul << (DBL_ALT_EXP % (CHAR_BIT * sizeof(extra)));
  }
  done:
  *p -= ctz64(r) << DBL_MANT_BITS;
  return out;
}
