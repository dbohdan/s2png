# s2png — “stuff to PNG”

[![Build Status](https://travis-ci.org/dbohdan/s2png.svg)](https://travis-ci.org/dbohdan/s2png)
[![AppVeyor CI build status](https://ci.appveyor.com/api/projects/status/github/dbohdan/s2png?branch=master&svg=true)](https://ci.appveyor.com/project/dbohdan/s2png)

This program encodes arbitrary binary data in PNG images that look like noise
and decodes it back.  It was originally developed by k0wax
[on SourceForge](http://sourceforge.net/projects/s2png/).  I started this fork
to fix a libgd-related problem that caused s2png 0.01 to segfault when compiled
on a modern operating system.  The fork has since accumulated various
improvements and has been ported to Rust.  It remains data-compatible with
the original (if you don't use the toy encryption feature).


## Building and installing

### *nix

### Windows


## Usage

```none
s2png ("stuff to png") version 0.10.0
usage: s2png [-h] [-o filename] [-w width (600) | -s] [-b text]
             [-p password] [-e | -d] file

Store any data in a PNG image.
This version can encode files of up to 16777215 bytes.

  -h            display this message and quit
  -o filename   output the encoded or decoded data to filename
  -w width      set the width of the PNG image output (600 by default)
  -s            make the output image roughly square
  -b text       custom banner text ("" for no banner)
  -p password   encrypt/decrypt the output with a hexadecimal password
                using RC4
                (Warning: do not use this if you want actual secrecy!)

Normally s2png detects which operation to perform by the file type. You can
override this behavior with the following switches:
  -e            force encoding mode
  -d            force decoding mode

See README.md for further details.
```


## Examples

To store `foo.mp3` in an image enter the following command:

    s2png foo.mp3

A file named `foo.mp3.png` will be created in the same directory as `foo.mp3`.

Add the `-s` switch to ensure the resulting image is square (give or take a
pixel) and `-b "some text"` to change the text of the banner at the bottom.

    s2png -s -b hello foo.mp3

To decode `decode_me.mp3.png` and retrieve the original file `decode_me.mp3` run
the command

    s2png decode_me.mp3.png

You can decode `xyz.png` to `decoded.mp3` with

    s2png -o decoded.mp3 xyz.png


## License

s2png is licensed under the GNU GPL 2.0.  See the file `LICENSE`.  The
implementation of the RC4 streaming cypher in `src/rc4/mod.rs` is in the public
domain.  The font from libgd is distributed under its BSD-like license.  See
the file `src/font/COPYING.libgd`.
