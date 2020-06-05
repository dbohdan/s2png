#! /bin/sh

make release
cp "/tmp/$USER/s2png-rust/x86_64-unknown-linux-musl/release/s2png" s2png-linux-x86_64
strip s2png-linux-x86_64

make release TARGET=i686-pc-windows-gnu
cp "/tmp/$USER/s2png-rust/i686-pc-windows-gnu/release/s2png.exe" s2png-win32.exe
strip s2png-win32.exe
