#!/bin/bash

while read line
do
  echo CLI : recv $line 1>&2
  s=A-${line: -1}
  echo $s
  echo CLI : sent $s 1>&2
done
