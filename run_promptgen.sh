#!/bin/bash

prompt_gen_file="generate_prompt.cpp"
# change this to be the name of the main file
main_file="main.cpp"

filename=$(basename -- "$prompt_gen_file")
object_file_name="${filename%.*}"
object_file_name="$object_file_name.o"

exec_name="${main_file%.*}"

g++ -c $prompt_gen_file -o $object_file_name 
g++ $main_file $object_file_name -o $exec_name

if [ $? -eq 0 ]; then
  # echo "Enter the starting point:"
  # read start
  # echo "Enter the starting orientation"
  # read orientation
  # ./$exec_file_name $start $orientation
  ./$exec_name
  rm -f $object_file_name
  rm -f $exec_name
else
  echo "compilation failed"
fi