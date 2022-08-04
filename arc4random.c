#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/random.h>

#define unlikely(x) __builtin_expect(!!(x), 0)
#define likely(x)   __builtin_expect(!!(x), 1)

__attribute__((visibility("default"))) uint32_t arc4random(void)
{
  uint32_t out;
  while(unlikely(getrandom(&out, sizeof(out), 0) < sizeof(out)));
  return out;
}

__attribute__((visibility("default"))) void arc4random_buf(void *buf, size_t nbytes)
{
  if(likely(nbytes)) do
  {
    ssize_t length = getrandom(buf, nbytes, 0);
    if(unlikely(length < 0)) length = 0;
    buf += length;
    nbytes -= length;
  }
  while(unlikely(nbytes));
}

__attribute__((visibility("default"))) uint32_t arc4random_uniform(uint32_t upper_bound)
 
{
  uint32_t out, limit = UINT32_MAX - UINT32_MAX % upper_bound;
  for(;;)
  {
    while(unlikely(getrandom(&out, sizeof(out), 0) < sizeof(out)) || unlikely(out >= limit));
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
  if(unlikely(!r))
  {
    uint64_t extra;
    for(int i = DBL_ALT_EXP / (CHAR_BIT * sizeof(extra)) + 1; i; --i)
    {
      arc4random_buf(&extra, sizeof(extra));
      if(likely(r = extra)) break;
      else *p -= (CHAR_BIT * sizeof(extra)) << DBL_MANT_BITS;
      r |= 1ul << (DBL_ALT_EXP % (CHAR_BIT * sizeof(extra)));
    }
  }
  *p -= ctz64(r) << DBL_MANT_BITS;
  return out;
}

__attribute__((visibility("default"))) void arc4random_double_buf(double *buf, size_t length)
{
  arc4random_buf(buf, length * sizeof(double));
  for(int i = 0; i < length; ++i)
  {
    uint64_t *const p = (uint64_t *)&buf[i];
    uint64_t r = *p >> DBL_MANT_BITS;
    *p &= (1ul << DBL_MANT_BITS) - 1;
    *p |= DBL_BASE_EXP << DBL_MANT_BITS;
    if(unlikely(!r))
    {
      uint64_t extra;
      for(int i = DBL_ALT_EXP / (CHAR_BIT * sizeof(extra)) + 1; i; --i)
      {
        arc4random_buf(&extra, sizeof(extra));
        if(likely(r = extra)) break;
        else *p -= (CHAR_BIT * sizeof(extra)) << DBL_MANT_BITS;
        r |= 1ul << (DBL_ALT_EXP % (CHAR_BIT * sizeof(extra)));
      }
    }
    *p -= ctz64(r) << DBL_MANT_BITS;
  }
}
