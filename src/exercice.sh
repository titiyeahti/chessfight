#!/bin/bash

CLIENT=./exercli.sh

if [ $# -lt 1 -o $# -gt 2 ]; then
  echo $#
  echo "wrong number of arguments; 
  usage ./interface <your program> | <interpreter> <your script>"
  exit
fi

#Creating the two named pipes
rm -rf f1 f2;
mkfifo f1 f2;

echo $CLIENT
./exerprog.sh > f2 < f1 & ./exercli.sh < f2 > f1;

#if [ $# -eq 1 ]; then 
#  ./exercli.sh > f2 < f1 &
#  ./exerprog.sh > f1 < f2;
#else
#  $CLIENT > f2 < f1 &
#  $1 $2 > f1 < f2;
#fi

rm -rf f1 f2;
