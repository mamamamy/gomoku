#!/bin/bash

gcc -Wall -Wextra -O3 generator.c save.c vct.c board.c bitmap256.c wtree.c -o generator -lpthread
