#! /bin/sh

make release
cp "/tmp/$USER/cargo/s2png/x86_64-unknown-linux-musl/release/s2png" s2png-linux-x86_64
strip s2png-linux-x86_64

CARGO_PROFILE_RELEASE_LTO=true RUSTFLAGS="-C panic=abort -C lto" make release TARGET=i686-pc-windows-gnu
cp "/tmp/$USER/cargo/s2png/i686-pc-windows-gnu/release/s2png.exe" s2png-win32.exe
strip s2png-win32.exe
