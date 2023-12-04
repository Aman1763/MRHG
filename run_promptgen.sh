#!/bin/bash

g++ -o genPrompt generate_prompt.cpp

if [ $? -eq 0 ]; then
  # echo "Enter the starting point:"
  # read start
  start="intersection1"
  # echo "Enter the starting orientation"
  # read orientation
  orientation="east"
  ./genPrompt $start $orientation
  rm -f genPrompt
else
  echo "compilation failed"
fi