#!/bin/bash

# make data folder
folder_name="./data"
if [ ! -d "$folder_name" ]; then
  mkdir "$folder_name"
fi

# make generator
gcc -Wall -Wextra -O3 generator.c save.c vct.c board.c bitmap256.c wtree.c rwlock.c spinlock.c cjson.c config.c -o generator -lpthread

# make test
gcc -Wall -Wextra -O3 test.c vct.c board.c bitmap256.c -o test
