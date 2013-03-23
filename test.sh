#!/bin/sh

runtest() {
    testfile=$1

    ./s2png $testfile -s -o $testfile.png || File encoding failed.
    ./s2png $testfile.png -o $testfile.out || File decoding failed.

    ./s2png $testfile -s -o $testfile.png -b "Hello" -p password || File ecryption failed.
    ./s2png $testfile.png -o $testfile.out2 -p password || File decryption failed.

    diff $testfile $testfile.out || echo Basic test failed.
    diff $testfile $testfile.out2 || echo Encryption test failed.

    rm $testfile.png $testfile.out $testfile.out2
}

echo -n -e \\0377\\0377\\0377\\0377\\0377\\0377\\0377\\0377 > test.bin
runtest test.bin
rm test.bin

dd if=/dev/urandom of=test.bin count=2k
runtest test.bin
rm test.bin

echo Tests done.
