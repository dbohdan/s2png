#! /bin/sh
set -eu

cargo build
make test
