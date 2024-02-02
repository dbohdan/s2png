#! /bin/sh
# Use s2png to produce a corrupted version of the input data (for fun).
# This requires GraphicsMagick or ImageMagick.
# The original file is not affected.
# Try it out on text.

set -eu

usage() {
    echo "usage: $0 input format [quality] [s2png-arg ...]"
    echo 'The format can be, e.g., "gif" or "jpeg".'
}

if [ "$#" -lt 2 ]; then
    usage
    exit 2
fi

for arg in "$@"; do
    if [ "$arg" = -h ] || [ "$arg" = --help ]; then
        usage
        exit 0
    fi
done

input_file="$1"
format="$2"
quality="${3:-100}"
if [ "$#" -ge 3 ]; then
    shift 3
else
    shift 2
fi
s2png_args="$*"

if command -v gm > /dev/null; then
    convert="gm convert"
elif command -v convert > /dev/null; then
    convert=convert
else
    echo "Can't find GraphicsMagick or ImageMagick."
    exit 1
fi

if [ ! -e "$input_file" ]; then
    echo "Not found: \"$1\""
    exit 1
fi

temp_png="$(mktemp /tmp/corruptsh-XXXXXXXX.png)"
temp_lossy="$(mktemp "/tmp/corruptsh-XXXXXXXX.$format")"

clean_up() {
    rm "$temp_png" "$temp_lossy"
}
trap clean_up EXIT HUP INT QUIT TERM

# shellcheck disable=SC2086
s2png "$input_file" $s2png_args -e -o "$temp_png"
$convert "$temp_png" -quality "$quality" "$temp_lossy"
$convert "$temp_lossy" "$temp_png"
s2png "$temp_png" -d -o /dev/stdout
