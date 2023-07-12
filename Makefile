S2PNG_COMMAND ?= target/debug/s2png

test: test-unit test-integration

test-integration:
	S2PNG_COMMAND="$(S2PNG_COMMAND)" cargo test -- --ignored

test-unit:
	cargo test

PHONY: test test-integration test-unit
