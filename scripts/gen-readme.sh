#!/bin/sh
# Generate README.md based on README.in and the output of ./s2png -h

export tempfile=$(mktemp -t s2pngXXXXX)
# Below sed strips "See README.md for details." and the preceding
# empty line from from s2png's help.
./s2png -h | sed -e '$d' | sed -e 's/^/    /;$d' > $tempfile
sed -e "/S2PNGHELP/{
    r $tempfile
    d
}" README.in > README.md
echo README.md generated.
rm $tempfile
