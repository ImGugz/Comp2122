#!/bin/bash
#
# File: daily-tests.sh
# Author: Dudu, Gugz
# Created on: Mon 20 Jun 2022 02:14
# Copyright (C) 2022, Dudu, Gugz
#

RED='\033[0;31m'
GREEN='\033[1;32m'
NC='\033[0m'

mkdir -p run
for file in auto-tests/*; do
  if [ ! -d "$file" ]; then
        filename=$(basename -- "$file")
        extension="${filename##*.}"
        filename="${filename%.*}"
        ./l22 --target asm auto-tests/$filename.l22 > /dev/null 2>&1
        yasm -felf32 auto-tests/$filename.asm
        ld -melf_i386 -o test $filename.o -lrts
        ./test > run/$filename.log
        rm auto-tests/$filename.asm
        rm $filename.o
        diff <( tr -d ' \t\n' <auto-tests/expected/$filename.out) <( tr -d ' \t\n' <run/$filename.log)
        rv_diff=$?
        if [ ${rv_diff} == 0 ]; then
            echo -e "${GREEN}Test $filename PASSED!\n${NC}"
        else
            echo -e "${RED}Test $filename FAILURE!\n${NC}"
        fi
  fi
done
rm -rf run