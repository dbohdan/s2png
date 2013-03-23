#!/bin/sh

runtest() {
    testfile=$1

    ./s2png $testfile -s -o $testfile.png
    ./s2png $testfile.png -o $testfile.out2

    ./s2png $testfile -s -o $testfile.png -b "Hello" -p password
    ./s2png $testfile.png -o $testfile.out -p password

    diff $testfile $testfile.out2 || echo Basic test failed.
    diff $testfile $testfile.out || echo Encryption test failed.

    rm $testfile.png $testfile.out $testfile.out2
}

echo -n -e \\0377\\0377\\0377\\0377\\0377\\0377\\0377\\0377 > test.bin
runtest test.bin
rm test.bin

dd if=/dev/urandom of=test.bin count=2k
runtest test.bin
rm test.bin

echo Tests done.
