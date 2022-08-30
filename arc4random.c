#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/param.h>
#include <sys/types.h>

#ifdef _MSC_VER
#include <stdbool.h>
#include <wincrypt.h>
#else
#include <sys/syscall.h>
#include <openssl/rand.h>
#include <stdio.h>
#include <unistd.h>
#endif

#if !defined(__GNUC__) || __GNUC__ < 3
#define __builtin_expect(x, v) (x)
#endif

#define unlikely(x) __builtin_expect(!!(x), 0)
#define likely(x)   __builtin_expect(!!(x), 1)

__attribute__((visibility("default"))) void arc4random_buf(void *buf, size_t nbytes)
{
  if(likely(nbytes))
  {
#if defined(_MSC_VER)
    // Windows 95+, NT 4.0+
    // try RtlGenRandom first
    HMODULE advapi = GetModuleHandle("advapi32.dll");
    BOOLEAN (APIENTRY *RtlGenRandom)(void *, ULONG);
    if(advapi) RtlGenRandom = (BOOLEAN (APIENTRY *)(void *, ULONG))GetProcAddress(advapi, "SystemFunction036");
    if(RtlGenRandom)
    {
      while(unlikely(!RtlGenRandom(buf, nbytes)));
    }
    else
    {
      // fall back to CryptGenRandom
      static bool acquired = false;
      static HCRYPTPROV hCryptProv;
      if(unlikely(nbytes > UINT32_MAX)) nbytes = UINT32_MAX;
      if(unlikely(!acquired)) while(unlikely(!CryptAcquireContext(&hCryptProv)));
      acquired = true;
      while(unlikely(!CryptGenRandom(hCryptProv, nbytes, buf)));
    }
#else
#if   defined(SYS_getrandom) || defined(SYS_getentropy)
    do
    {
#if  defined(SYS_getrandom)
      // Linux 3.17+ with glibc 2.25+ or musl 1.1.4+, Android 6+, FreeBSD 12+, NetBSD 10+, DragonFly 5.7+, Solaris 11.3+, Illumos
      ssize_t length = syscall(SYS_getrandom, buf, MIN(SSIZE_MAX, nbytes), 0);
#elif defined(SYS_getentropy)
      // OpenBSD 5.6+, macOS 10.12+, Solaris 11.3+
      int length = syscall(SYS_getentropy(SYS_getentropy, buf, MIN(256, nbytes))) || MIN(256, nbytes);
#endif
      if(likely(length > 0 || errno != ENOSYS))
#endif
      {
        // try to use OpenSSL RAND_bytes
        if(unlikely(RAND_bytes(buf, nbytes) != 1))
        {
          // fall back to /dev/urandom
          // Android, FreeBSD 2.2+, NetBSD 1.3+, DragonFly, OpenBSD 2.2+, macOS, Solaris 8/9+
          static FILE *urandom;
          if(unlikely(!urandom)) urandom = fopen("/dev/urandom", "r");
          if(likely(urandom))
          {
            size_t length = fread(buf, 1, MIN(SIZE_MAX, nbytes), urandom);
          }
          else exit(-1); // TODO: figure out something better to do here
        }
      }
      buf += length;
      nbytes -= length;
    }
    while(unlikely(nbytes));
#endif
  }
  return;
}

#define GETRANDOM(r) arc4random_buf(&(r), sizeof(r))

__attribute__((visibility("default"))) uint32_t arc4random(void)
{
  uint32_t out;
  GETRANDOM(out);
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
