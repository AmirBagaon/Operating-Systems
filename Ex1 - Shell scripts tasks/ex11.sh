#!/bin/bash
#Amir Bagaon 204313100

if [ -d "$1" ]; then
	cd "$1"
	COUNTER=`ls *.txt 2>/dev/null | wc -l` 
	echo "Number of files in the directory that end with .txt is $COUNTER"
else
echo "Number of files in the directory that end with .txt is 0 (No such Directory)"
fi
