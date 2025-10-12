#ifndef SAVE_H
#define SAVE_H

#include "wtree.h"

void save_to_file(const wtree *wt, int depth);
void load_from_file(wtree *wt, int depth);

#endif
