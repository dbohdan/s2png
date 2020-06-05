#! /bin/sh
# Test s2png encoding and decoding operations, as well as encryption, for
# causing data corruption.

opstest() {
    testfile="$1"

    "$S2PNG_COMMAND" -s -o "$testfile.png" "$testfile" || echo File encoding failed.
    "$S2PNG_COMMAND" -o "$testfile.out" "$testfile.png" || echo File decoding failed.

    # printf password | md5sum | cut -c 1-32
    password=5f4dcc3b5aa765d61d8327deb882cf99
    "$S2PNG_COMMAND" -s -o "$testfile.png" -b "Hello" -p "$password" "$testfile" || echo File encryption failed.
    "$S2PNG_COMMAND" -o "$testfile.out2" -p "$password" "$testfile.png" || echo File decryption failed.

    diff "$testfile" "$testfile.out" || echo Basic test failed.
    diff "$testfile" "$testfile.out2" || echo Encryption test failed.

    rm "$testfile.png" "$testfile.out" "$testfile.out2"
}

sizetest() {
    n="$1"
    testfile="$2"
    encoded="$3"
    decoded="$4"

    dd "bs=$n" count=1 if=/dev/urandom of="$testfile" 2> /dev/null
    "$S2PNG_COMMAND" -e -o "$encoded" -b "" -w 10 "$testfile"
    "$S2PNG_COMMAND" -d -o "$decoded" "$encoded"
    if ! diff "$testfile" "$decoded" > /dev/null; then
        echo Corruption at file size "$n".
        identify "$testfile"
    fi
}

echo Running basic operations tests.

# Test for data corruption with an 0xFFFF pattern.
printf '\\0377\\0377\\0377\\0377\\0377\\0377\\0377\\0377' > test.bin
opstest test.bin
rm test.bin

# Test for data corruption with random data.
dd if=/dev/urandom of=test.bin count=2k
opstest test.bin
rm test.bin

echo Running integrity tests for bannerless images.

# Test for data corruption by file size pixel.
t1="$(mktemp -t s2pngXXXXX)"
t2="$(mktemp -t s2pngXXXXX)"
t3="$(mktemp -t s2pngXXXXX)"
i=10
while [ $i -le 256 ]; do
    sizetest "$i" "$t1" "$t2" "$t3"
    i="$((i + 1))"
done
rm "$t1" "$t2" "$t3"

echo Tests done.
