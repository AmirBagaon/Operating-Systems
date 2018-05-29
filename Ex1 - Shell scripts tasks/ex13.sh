#!/bin/bash
#Amir Bagaon 204313100
if test "$#" -ne 1; then
    echo "‫‪error:‬‬ ‫‪only‬‬ ‫‪one‬‬ ‫‪argument‬‬ ‫‪is‬‬ ‫‪allowed‬‬"

elif [ ! -e "$1" ]; then
	echo "error: there is no such file"
else
FILE=$1
mkdir -p safe_rm_dir
cp "$FILE" safe_rm_dir
rm "$FILE"
echo "done!"
fi

