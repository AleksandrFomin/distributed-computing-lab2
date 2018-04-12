#!/bin/bash

make

if [ $? -eq 0 ]
then

export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:~/Документы/ИТМО/3курс_весна/Распределенные_вычисления/pa2";

LD_PRELOAD=~/Документы/ИТМО/3курс_весна/Распределенные_вычисления/pa2/libruntime.so ./a.out -p 4 10 20 30 40
fi
