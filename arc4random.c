#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/param.h>
#ifdef _MSC_VER
#include <wincrypt.h>
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

#ifndef _MSC_VER
static FILE *urandom;
#endif

static inline ssize_t getrandom(void *buf, size_t buflen, unsigned int flags)
{
  if(unlikely(buflen > SSIZE_MAX)) buflen = SSIZE_MAX;
#if defined(_MSC_VER)
  // Windows NT 4.0+
  if(unlikely(buflen > UINT32_MAX)) buflen = UINT32_MAX;
  HCRYPTPROV hCryptProv;
  if(likely(CryptAcquireContext(&hCryptProv, NULL, (LPCWSTR)L"Microsoft Base Cryptographic Provider v1.0", PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) &&
            CryptGenRandom(hCryptProv, buflen, buf)))
    return buflen;
  else return -1;
#else
#if   defined(SYS_getrandom)
  // Linux 3.17+ with glibc 2.25+ or musl 1.1.4+, FreeBSD 12+, NetBSD 10+, DragonFly 5.7+, Solaris 11.3+, Illumos
  ssize_t length = syscall(SYS_getrandom, buf, buflen, flags);
  if(likely(length > 0 || errno != ENOSYS)) return length;
#elif defined(SYS_getentropy)
  // OpenBSD 5.6+, macOS 10.12+, Solaris 11.3+
  ssize_t length = syscall(SYS_getentropy(SYS_getentropy, buf, MIN(256, buflen))) || MIN(256, buflen);
  if(likely(length > 0 || errno != ENOSYS)) return length;
#endif
  // FreeBSD 2.2+, NetBSD 1.3+, DragonFly, OpenBSD 2.2+, macOS, Solaris 8/9+
  if(unlikely(!urandom)) urandom = fopen("/dev/urandom", "r");
  if(likely(urandom))
    return fread(buf, 1, buflen, urandom);
  else
    return -1;
#endif
}

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
