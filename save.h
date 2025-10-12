#ifndef SAVE_H
#define SAVE_H

#include "wtree.h"

void save_to_file(wtree *wt, uint64_t id);
void load_from_file(wtree *wt, uint64_t id);

#endif
