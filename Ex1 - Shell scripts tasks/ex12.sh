#!/bin/bash
#Amir Bagaon 204313100
if [ -d "$1" ]; then
	cd "$1"
	for ITEM in *; do
	if [[ -f $ITEM ]]; then
	    echo "$ITEM is a file"
	elif [[ -d $ITEM ]]; then
	    echo "$ITEM is a directory"
	fi
	done
	else
	echo "no such directory"
fi
