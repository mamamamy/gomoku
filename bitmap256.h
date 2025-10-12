#ifndef BITMAP256_H
#define BITMAP256_H

#include <stdint.h>

typedef struct bitmap256 bitmap256;

struct bitmap256 {
  uint32_t data[8];
};

void bitmap256_init(bitmap256 *bm);
void bitmap256_set(bitmap256 *bm, int pos);
int bitmap256_test(const bitmap256 *bm, int pos);
void bitmap256_reset(bitmap256 *bm, int pos);

#endif
