#!/bin/sh
# Generate README.md based on README.in and the output of ./s2png -h

awk '
/^S2PNGHELP$/ {
    command = "./s2png -h"
    for(len = 0; (command | getline line) > 0; len++) {
        usage[len] = line
    }
    for (i = 0; i < len - 2; i++) {
        printf "    %s\n", usage[i]
    }
}
!/^S2PNGHELP$/ { print }
'
