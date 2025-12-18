#ifndef C_RYPT
#define C_RYPT
#include <stdint.h>

#define ROTR(x,n) ((x >> n) | (x << (32 - n)))
#define ROTL(x,n) ((x << n) | (x >> (32 - n)))

#define CH(x,y,z) ((x & y) ^ ((~x) & z))
#define MAJ(x,y,z) ((x & y) ^ (x & z) ^ (y & z))

#define BSIG0(x) ROTR(x,2) ^ ROTR(x,13) ^ ROTR(x,22)
#define BSIG1(x) ROTR(x,6) ^ ROTR(x,11) ^ ROTR(x,25)
#define SSIG0(x) ROTR(x,7) ^ ROTR(x,18) ^ (x >> 3)
#define SSIG1(x) ROTR(x,17) ^ ROTR(x,19) ^ (x >> 10)

typedef struct {
  unsigned int N, R;
  unsigned int H[8];
  unsigned char buf[64];
  unsigned char T1, T2;
  unsigned char W[64];
} sha256_ctx;

void sha256(unsigned char *, unsigned int, unsigned char[32]);

#endif
