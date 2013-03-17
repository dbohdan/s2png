s2png - "something to png"
=========================

This program converts binary files of any kind into new PNG images and 
vice versa. It was originally developed by k0wax at
http://sf.net/projects/s2png/. This version fixes 

Building
--------

1. Install the dependecies. On Ubuntu 12.04 and Linux Mint 13 

Just type "make" in your terminal and hit the enter key.
s2png depends on libgd (http://libgd.org), you need to have it installed.

Installing
----------

As root type "make install".

Usage
=====

ch 1. Suppose you want encode 1.mp3

    $ s2png 1.mp3
    ... now you have 1.mp3.png in the same directory where 1.mp3

ch 2. Suppose you need decode file DecodeMe.mp3.png

    $ s2png DecodeMe.mp3.png
    ... now you have original DecodeMe.mp3 in the same directory where DecodeMe.mp3.png

You can also use additional options.

