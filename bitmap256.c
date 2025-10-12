#include "bitmap256.h"

void bitmap256_init(bitmap256 *bm) {
  for (int i = 0; i < 8; ++i) {
    bm->data[i] = 0;
  }
}

void bitmap256_set(bitmap256 *bm, int pos) {
  bm->data[(pos) / 32] |= 1 << ((pos) % 32);
}

int bitmap256_test(const bitmap256 *bm, int pos) {
  return bm->data[(pos) / 32] & 1 << ((pos) % 32);
}

void bitmap256_reset(bitmap256 *bm, int pos) {
  return bm->data[(pos) / 32] &= ~(1 << ((pos) % 32));
}
