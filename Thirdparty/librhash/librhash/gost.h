/* gost.h */
#ifndef GOST_H
#define GOST_H
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

#define gost_block_size 32
#define gost_hash_length 32

/* algorithm context */
typedef struct gost_ctx
{
  unsigned hash[8]; /* algorithm 256-bit state */
  unsigned sum[8];  /* sum of processed message blocks */
  unsigned char message[gost_block_size]; /* 256-bit buffer for leftovers */
  uint64_t length;  /* number of processed bytes */
  unsigned cryptpro; /* flag, type of sbox to use */
} gost_ctx;

/* hash functions */

void gost_init(gost_ctx *ctx);
void gost_cryptopro_init(gost_ctx *ctx);
void gost_update(gost_ctx *ctx, const unsigned char* msg, size_t size);
void gost_final(gost_ctx *ctx, unsigned char result[32]);

#ifdef GENERATE_GOST_LOOKUP_TABLE
void gost_init_table(void); /* initialize algorithm static data */
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* GOST_H */
