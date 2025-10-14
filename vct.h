#ifndef VCT_H
#define VCT_H

#include "board.h"

#define VCT_PATTERN_NONE 0
#define VCT_PATTERN_FIVE_IN_A_ROW (1 << 0)
#define VCT_PATTERN_LIVE_FOUR (1 << 1)
#define VCT_PATTERN_BROKEN_FOUR (1 << 2)
#define VCT_PATTERN_LIVE_THREE (1 << 3)

int vct(board *bd);
int vct_check_pattern(board *bd, int pos);

#endif
