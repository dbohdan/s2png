#!/bin/sh
# Use s2png to produce a corrupted version of the input data (for fun).
# This requires GraphicsMagick or ImageMagick.
# The original file is not affected.  Try it out on text.

set -e

input_file="$1"
format="$2"
quality="$3"
[ "$quality" != "" ] && shift 3 && s2png_args="$*"


if which gm > /dev/null; then
    convert="gm convert"
elif which convert > /dev/null; then
    convert=convert
else
    echo "Can't find GraphicsMagick or ImageMagick."
    exit 1
fi

if [ ! -e "$input_file" ]; then
    if [ "$input_file" != "" ]; then
         echo "Not found: \"$1\""
    fi
    echo "usage: $0 input format [quality] [s2png-args...]"
    echo 'The format can be, e.g., "gif" or "jpeg".'
    exit 1
fi

if [ "$quality" = "" ]; then
    quality=100
fi


temp_png=/tmp/corruptsh-temp.png
temp_lossy="/tmp/corruptsh-temp.$format"

s2png "$input_file" $s2png_args -e -o "$temp_png"
$convert "$temp_png" -quality "$quality" "$temp_lossy"
$convert "$temp_lossy" "$temp_png"
s2png "$temp_png" -d -o /dev/stdout

rm "$temp_png" "$temp_lossy"
