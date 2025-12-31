#!/bin/bash

# Filename: ex13.sh
#
# Problem: Add writing to an array and reading from the array.

echo Please type in 3 foods you like:

declare -a food_options

for xx in `seq 1 3`; do
	read food_name
	food_options[$xx]=$food_name
done

PS3='Now Select the food you like the best: '
select option in ${food_options[@]}
do
  echo "The option you have selected is: $option"
  break
done

