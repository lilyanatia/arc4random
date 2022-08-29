#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/param.h>
#if defined(USE_GETENTROPY)
#include <unistd.h>
#elif defined(USE_BCRYPTGENRANDOM)
#include <bcrypt.h>
#elif defined(USE_CRYPTGENRANDOM)
#include <wincrypt.h>
#elif defined(USE_URANDOM)
#include <stdio.h>
#else
#include <sys/random.h>
#endif
#include <sys/types.h>

#if !defined(__GNUC__) || __GNUC__ < 3
#define __builtin_expect(x, v) (x)
#endif

#define unlikely(x) __builtin_expect(!!(x), 0)
#define likely(x)   __builtin_expect(!!(x), 1)

#if defined(USE_GETENTROPY)
static inline ssize_t getrandom(void *buf, size_t buflen, unsigned int flags)
{
  if(buflen > 256) buflen = 256;
  return getentropy(buf, buflen) || buflen;
}
#elif defined(USE_BCRYPTGENRANDOM)
static inline ssize_t getrandom(void *buf, size_t buflen, unsigned int flags)
{
  return likely(BcryptGenRandom(NULL, buf, buflen, BCRYPT_USE_SYSTEM_PREFERRED_RNG)) ?
         buflen : -1;
}
#elif defined(USE_CRYPTGENRANDOM)
static inline ssize_t getrandom(void *buf, size_t buflen, unsigned int flags)
{
  HCRYPTPROV hCryptProv;
  return likely(CryptAcquireContext(&hCryptProv, NULL, (LPCWSTR)L"Microsoft Base Cryptographic Provider v1.0", PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) &&
                CryptGenRandom(hCryptProv, buflen, buf)) ?
         buflen : -1;
}
#elif defined(USE_URANDOM)
static inline ssize_t getrandom(void *buf, size_t buflen, unsigned int flags)
{
  FILE *urandom = fopen("/dev/urandom", "r");
  if(unlikely(!urandom)) return -1;
  ssize_t length = fread(buf, 1, MIN(SSIZE_MAX, buflen), urandom);
  fclose(urandom);
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
