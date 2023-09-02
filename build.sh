#!/bin/sh

set -xe

CFLAGS="-O1 -Wall -std=c99 -Wno-missing-braces"
INCLUDE="-I include/"
LIBS="-L lib/ -lraylib -lopengl32 -lgdi32 -lwinmm"

gcc main.c -o fft_visx.exe $CFLAGS $INCLUDE $LIBS
