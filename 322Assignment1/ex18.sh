#!/bin/bash

# Filename: ex18.sh
#
# Problem: Find the highest number for each line and print it. 
cat ex18.input | awk '{
	highest_number=$1;
	for (n=2; n<=NF; n++) {
		if ($n>highest_number) {
			highest_number=$n;
		}
	}
	print highest_number;	
	}'
