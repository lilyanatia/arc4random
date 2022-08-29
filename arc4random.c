#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/param.h>
#ifdef _MSC_VER
#include <bcrypt.h>
#else
#include <sys/syscall.h>
#include <stdio.h>
#endif
#include <sys/types.h>

#if !defined(__GNUC__) || __GNUC__ < 3
#define __builtin_expect(x, v) (x)
#endif

#define unlikely(x) __builtin_expect(!!(x), 0)
#define likely(x)   __builtin_expect(!!(x), 1)

#ifdef _MSC_VER
static inline ssize_t getrandom(void *buf, size_t buflen, unsigned int flags)
{
  return likely(BcryptGenRandom(NULL, buf, buflen, BCRYPT_USE_SYSTEM_PREFERRED_RNG)) ?
         buflen : -1;
}
#else
static inline ssize_t getrandom(void *buf, size_t buflen, unsigned int flags)
{
  ssize_t length = syscall(SYS_getrandom, buf, buflen, flags);
  if(unlikely(length < 0 && errno = ENOSYS))
  {
    length = syscall(SYS_getentropy(SYS_getentropy, buf, MIN(256, buflen))) || MIN(256, buflen);
    if(unlikely(length < 0 && errno = ENOSYS))
    {
      FILE *urandom = fopen("/dev/urandom", "r");
      if(unlikely(!urandom)) return -1;
      ssize_t length = fread(buf, 1, MIN(SSIZE_MAX, buflen), urandom);
      fclose(urandom);
    }
  }
  return length;
}
#endif

#define GETRANDOM(r) while(unlikely(getrandom(&r, sizeof(r), 0) < 0))

__attribute__((visibility("default"))) void arc4random_buf(void *buf, size_t nbytes)
{
  if(likely(nbytes)) do
  {
    ssize_t length = MAX(0, getrandom(buf, MIN(SSIZE_MAX, nbytes), 0));
    buf += length;
    nbytes -= length;
  }
  while(unlikely(nbytes));
}

__attribute__((visibility("default"))) uint32_t arc4random(void)
{
  uint32_t out;
  GETRANDOM(out)
  return out;
}

__attribute__((visibility("default"))) uint32_t arc4random_uniform(uint32_t upper_bound)
 
{
  uint32_t out;
  uint32_t const limit = ~(UINT32_MAX % upper_bound);
  do GETRANDOM(out);
  while(unlikely(out > limit));
  return out % upper_bound;
}
