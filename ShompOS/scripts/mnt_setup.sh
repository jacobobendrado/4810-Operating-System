#!/bin/bash

# Expected usage: $0 MNT_DIR LD MNT_OUT_DIR outfile_name
if [ "$#" -ne 5 ]; then
    echo "Usage: $0 <MNT_DIR> <LD> <MNT_OUT_DIR> <SCRIPTS_DIR> <outfile_name>"
    exit 1
fi

MNT_DIR="$1"
LD="$2"
MNT_OUT_DIR="$3"
SCRIPTS_DIR="$4"
OUTFILE="$5"

ech() { # text, file
    echo -n "$1 " >> "$2"
}

check_dir() { # dir_to_check, tempfile
    local parent_symbol="`echo "$1" | sed 's/[ \/\.]/_/g'`"
    for filename in "$1"/*; do
        # Check if the file exists
        [ -e "$filename" ] || continue

        # If the file is a file
        if [ -f "$filename" ]; then
            ech "f" "$2"
            ech "$parent_symbol" "$2"
            local filename_symbol="`echo $filename | sed 's/[ \/\.]/_/g'`"
            ech $(basename "$filename") "$2"
            ech "$filename_symbol" "$2"
            echo "" >> "$2"
            $LD -r -b binary -o "$MNT_OUT_DIR/"$filename_symbol".o" "$filename"
        fi

        # If the file is a directory
        if [ -d "$filename" ]; then
            ech "d" "$2"
            ech "$parent_symbol" "$2"
            local directory_symbol="`echo $filename | sed 's/[ \/\.]/_/g'`"
            ech $(basename "$filename") "$2"
            ech "$directory_symbol" "$2"
            echo "" >> "$2"
            check_dir "$filename" "$2"
        fi
    done
}

# tempfile takes format "type parent name symbol"
tempfile="`mktemp $MNT_OUT_DIR/tempfile_XXXXXX.txt`"
echo `echo "$MNT_DIR" | sed 's/[ \/\.]/_/g'` >> $tempfile
check_dir $MNT_DIR $tempfile
echo $OUTFILE
awk -f "$SCRIPTS_DIR"/mnt_setup_1.awk $tempfile > $OUTFILE
awk -f "$SCRIPTS_DIR"/mnt_setup_2.awk $tempfile >> $OUTFILE
rm $tempfile
