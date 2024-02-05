#!/bin/bash
set -xe

gcc -std=c11 -lm -lc -Wall -Wextra -o main main.c
