#ifndef C_RYPT
#define C_RYPT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <endian.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int uint32_t;

#define SHA256_BYTES 32

#define SHR(x, n) (x >> n)
#define SHL(x, n) (x << n)

#define ROTR(x, n) (SHR(x,n) | SHL(x,(32-n)))
#define ROTL(x, n) (SHL(x,n) | SHR(x,(32-n)))

#define CH(x,y,z) ((x & y) ^ ((~x) & z))
#define MAJ(x,y,z) ((x & y) ^ (x & z) ^ (y & z))
#define BSIG0(x) (ROTR(x,2) ^ ROTR(x,13) ^ ROTR(x,22))
#define BSIG1(x) (ROTR(x,6) ^ ROTR(x,11) ^ ROTR(x,25))
#define SSIG0(x) (ROTR(x,7) ^ ROTR(x,18) ^ SHR(x,3))
#define SSIG1(x) (ROTR(x,17) ^ ROTR(x,19) ^ SHR(x,10))

typedef struct {
  uint32_t buf[16];
  uint32_t W[64];
  uint32_t H[8];
  uint32_t T1;
  uint32_t T2;
  uint32_t out[8];
  uint32_t N;
  uint32_t R;
  char carry;
} sha256_ctx;

void sha256(unsigned char* in, unsigned int len, unsigned char out[32]);

static char* sha256_pad(char* in, int bytes);

#ifdef __cplusplus
}
#endif

#endif
