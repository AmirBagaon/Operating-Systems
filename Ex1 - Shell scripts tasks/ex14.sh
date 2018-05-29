#!/bin/bash
#Amir Bagaon 204313100

PERSON=$1
FILENAME=$2
awk -v person="$PERSON" '{b=$1 FS $2}
	{if (b == person) {sum += $3; print $0}} 
	END {if (sum) print "Total Balance: "sum}' $FILENAME
