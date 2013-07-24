#!/bin/sh

export tempfile=$(tempfile)
./s2png -h | sed -e "s/^/    /" > $tempfile
sed -e "/S2PNGHELP/{
    r $tempfile
    d
}" README.in > README.md
echo README.md generated.
rm $tempfile
