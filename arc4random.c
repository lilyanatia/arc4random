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

#if(UINT_MAX >= UINT64_MAX)
#define ctz64(n) (uint64_t)__builtin_ctz(n)
#elif(ULONG_MAX >= UINT64_MAX)
#define ctz64(n) (uint64_t)__builtin_ctzl(n)
#else
#define ctz64(n) (uint64_t)__builtin_ctzll(n)
#endif

#define DBL_EXP_DIG (sizeof(double) * CHAR_BIT - DBL_MANT_DIG)
#define DBL_MANT_BITS (DBL_MANT_DIG - 1)               // 52
#define DBL_BASE_EXP  (DBL_MAX_EXP  - UINT64_C(1))     // 1022
#define DBL_ALT_EXP   (DBL_BASE_EXP - DBL_EXP_DIG - 1) // 1010

__attribute__((visibility("default"))) void arc4random_double_buf(double *buf, size_t length)
{
  arc4random_buf(buf, length * sizeof(double));
  for(int i = 0; i < length; ++i)
  {
    uint64_t *const p = (uint64_t *)&buf[i];
    uint64_t r = *p >> DBL_MANT_BITS;
    *p &= (UINT64_C(1) << DBL_MANT_BITS) - 1;
    *p |= DBL_BASE_EXP << DBL_MANT_BITS;
    if(unlikely(!r))
    {
      uint64_t extra;
      for(int i = DBL_ALT_EXP / (CHAR_BIT * sizeof(extra)) + 1; i; --i)
      {
        arc4random_buf(&extra, sizeof(extra));
        if(likely(r = extra)) break;
        else *p -= (CHAR_BIT * sizeof(extra)) << DBL_MANT_BITS;
        r |= UINT64_C(1) << (DBL_ALT_EXP % (CHAR_BIT * sizeof(extra)));
      }
    }
    *p -= ctz64(r) << DBL_MANT_BITS;
  }
}

__attribute__((visibility("default"))) double arc4random_double(void)
{
  double out;
  arc4random_double_buf(&out, 1);
  return out;
}

#define FLT_EXP_DIG (sizeof(float) * CHAR_BIT - FLT_MANT_DIG)
#define FLT_MANT_BITS (FLT_MANT_DIG - 1)               // 23
#define FLT_BASE_EXP  (FLT_MAX_EXP  - UINT32_C(2))     // 126
#define FLT_ALT_EXP   (FLT_BASE_EXP - FLT_EXP_DIG - 1) // 117

__attribute__((visibility("default"))) void arc4random_float_buf(float *buf, size_t length)
{
  arc4random_buf(buf, length * sizeof(float));
  for(int i = 0; i < length; ++i)
  {
    uint32_t *const p = (uint32_t *)&buf[i];
    uint64_t r = (uint64_t)*p >> FLT_MANT_BITS;
    *p &= (UINT32_C(1) << FLT_MANT_BITS) - 1;
    *p |= FLT_BASE_EXP << FLT_MANT_BITS;
    if(unlikely(!r))
    {
      uint64_t extra;
      for(int i = FLT_ALT_EXP / (CHAR_BIT * sizeof(extra)) + 1; i; --i)
      {
        arc4random_buf(&extra, sizeof(extra));
        if(likely(r = extra)) break;
        else *p -= (CHAR_BIT * sizeof(extra)) << FLT_MANT_BITS;
        r |= UINT64_C(1) << (FLT_ALT_EXP % (CHAR_BIT * sizeof(extra)));
      }
    }
    *p -= ctz64(r) << FLT_MANT_BITS;
  }
}
