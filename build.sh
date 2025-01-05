#!/usr/bin/env bash

set -xe

gcc -o main main.c -I/usr/include/SDL2 -lSDL2 -lSDL2_ttf -lm -Wall -Wextra -g
