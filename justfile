set windows-shell := ["powershell.exe", "-NoLogo", "-Command"]

export S2PNG_COMMAND := "target/debug/s2png"

default: test

debug:
  cargo build

[unix]
release: release-linux release-windows

[unix]
release-linux:
  cargo build --release --target x86_64-unknown-linux-musl
  cp target/x86_64-unknown-linux-musl/release/s2png s2png-linux-x86_64
  strip s2png-linux-x86_64

[unix]
release-windows:
  cargo build --release --target i686-pc-windows-gnu
  cp target/i686-pc-windows-gnu/release/s2png.exe s2png-win32.exe
  strip s2png-win32.exe

test: debug test-unit test-integration

test-integration:
  cargo test -- --ignored

test-unit:
  cargo test
