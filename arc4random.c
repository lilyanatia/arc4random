#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/param.h>
#include <sys/random.h>

#if !defined(__GNUC__) || __GNUC__ < 3
#define __builtin_expect(x, v) (x)
#endif

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
    ssize_t length = MAX(0, getrandom(buf, nbytes, 0));
    buf += length;
    nbytes -= length;
  }
  while(unlikely(nbytes));
}

__attribute__((visibility("default"))) uint32_t arc4random_uniform(uint32_t upper_bound)
 
{
  uint32_t out;
  for(uint32_t const limit = ~(UINT32_MAX % upper_bound);
      unlikely(getrandom(&out, sizeof(out), 0) < sizeof(out)) || unlikely(out >= limit););
  return out % upper_bound;
}
