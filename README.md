s2png — “something to png”
==========================

This program converts binary data files of any kind into PNG images and back. It was originally developed by k0wax at <http://sourceforge.net/projects/s2png/>. This version was created to fix the problem that causes s2png 0.01 to segfault when compiled on modern GNU/Linux distributions. It has other minor improvements but remains compatible with the original.

s2png is licensed under GNU GPL 2.0. See the file LICENSE.

Building and installing
-----------------------

1. Install the dependencies. On Debian, Ubuntu and Linux Mint you can do so with
`sudo apt-get install libgd2-xpm-dev libgd2-xpm`.
    
2. Type `make` in your terminal and hit the enter key. Building has been tested on Linux Mint 13 and Ubuntu 12.10.

3. Install with `sudo make install`. On Debian-derived distributions use `sudo checkinstall` instead.

Usage
-----

    s2png ("something to png") version 0.04
    usage: s2png [-h] [-o filename] [-w width (600) | -s] [-b text]
                 [-p password] file

      -h            display this message and quit
      -o filename   output the converted data (image or binary) to filename
      -w width      set the width of PNG image output (600 by default)
      -s            make the output image roughly square
      -b text       custom banner text ("" for no banner)
      -p password   encrypt/decrypt the output with password using RC4
                    (Warning: do not use this if you need actual security!)

Basic examples
--------------

To convert 1.mp3 into an image type the following in the command line:

    s2png 1.mp3
   
You will now have the file 1.mp3.png in the same directory as 1.mp3.

Suppose you need decode DecodeMe.mp3.png. To do so type

    s2png DecodeMe.mp3.png

The original DecodeMe.mp3 will appear in the same directory as DecodeMe.mp3.png.

