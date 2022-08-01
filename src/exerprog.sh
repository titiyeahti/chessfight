#!/bin/bash

for (( i=0; i<10; i++))
do
  s=Q-$i
  echo $s
  echo PROG : sent $s 1>&2
  read line
  echo PROG : recv $line 1>&2
done

