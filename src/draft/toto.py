#!/bin/python3
import sys

while(1) :
  entry = bytearray(sys.__stdin__.buffer.readline())
  l = len(entry)
 
  for i in range((l-1)//2):
    temp = entry[i]
    entry[i] = entry[l-2-i]
    entry[l-2-i] = temp

  count = sys.__stdout__.buffer.write(entry)
  sys.__stdout__.flush()
