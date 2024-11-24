# s2png — “stuff to PNG”

This program converts arbitrary binary data to and from PNG images that look like noise.
It was originally developed by k0wax
[on SourceForge](http://sourceforge.net/projects/s2png/).
I started this fork to fix a problem that caused s2png 0.01 to segfault
when compiled on a 2010s operating system.
The fork has since accumulated various bug fixes and improvements.
Most recently, I have ported it from C to Rust.
Among other things, this makes it easier to distribute s2png as a static binary.
The fork remains data-compatible with the original
if you don't use the toy encryption feature.


## Installation

Prebuilt binaries are available for x86-64 Linux and i686 Windows.
They are attached to releases on the
["Releases"](https://github.com/dbohdan/s2png/releases) page.

### Installing with Cargo

```sh
cargo install s2png
```

### Building on Debian and Ubuntu

Follow the instructions to build a static Linux binary of s2png from the source code on recent Debian and Ubuntu.

1\. Install [Rustup](https://rustup.rs/).
    Through Rustup, add the stable musl libc target for your CPU.

```sh
rustup target add x86_64-unknown-linux-musl
```

2\. Install the build dependencies.

```sh
sudo apt install build-essential musl-tools
cargo install just
```

3\. Clone this repository.
    Build the binary.

```sh
git clone https://github.com/dbohdan/s2png
cd s2png
just test
just release-linux
```

### Cross-compiling for Windows

Follow the instructions to build a 32-bit Intel Windows binary of s2png on recent Debian and Ubuntu.

1\. Install [Rustup](https://rustup.rs/).
    Through Rustup, add the i686 GNU ABI Windows target.

```sh
rustup target add i686-pc-windows-gnu
```

2\. Install the build dependencies.

```sh
sudo apt install build-essential mingw-w64
cargo install just
```

3\. Configure Cargo for cross-compilation.
  Add the following in `~/.cargo/config`.

```toml
[target.i686-pc-windows-gnu]
linker = "/usr/bin/i686-w64-mingw32-gcc"
```

4\. Clone this repository.
    Build the binary.

```sh
git clone https://github.com/dbohdan/s2png
cd s2png
just release-windows
```

## Usage

```none
s2png ("stuff to png") version 1.0.0
usage: s2png [-h] [-o filename] [-w width (600) | -s] [-b text]
             [-p hex-key] [-e | -d] file

Store any data in a PNG image.
This version can encode files of up to 16777215 bytes.

  -h            display this message and quit
  -o filename   output the encoded or decoded data to filename
  -w width      set the width of the PNG image output (600 by default)
  -s            make the output image roughly square
  -b text       custom banner text ("" for no banner)
  -p hex-key    encrypt/decrypt the output with a hexadecimal key
                using RC4 (Warning: completely insecure! Do not use this if
                you want actual secrecy.)

Normally s2png detects which operation to perform by the file type. You can
override this behavior with the following switches:
  -e            force encoding mode
  -d            force decoding mode

See README.md for further details.
```


## Examples

To store `foo.mp3` in an image, enter the following command:

```sh
s2png foo.mp3
```

A file named `foo.mp3.png` will be created in the same directory as `foo.mp3`.

Add the `-s` switch to make the resulting image square (give or take a pixel)
and `-b "some text"` to change the text of the banner at the bottom.

```sh
s2png -s -b hello foo.mp3
```

To decode `decode_me.mp3.png` and retrieve the original file `decode_me.mp3`,
run the command

```sh
s2png decode_me.mp3.png
```

Decode `xyz.png` to `decoded.mp3` with

```sh
s2png -o decoded.mp3 xyz.png
```

## License

s2png is distributed under the GNU GPL 2.0.
See the file [`LICENSE`](LICENSE).
The implementation of the RC4 streaming cypher in [`src/rc4/mod.rs`](src/rc4/mod.rs)
is in the public domain.
The font from [libgd](https://github.com/libgd/libgd) is distributed under its BSD-like license.
See the file [`src/font/COPYING.libgd`](src/font/COPYING.libgd).
