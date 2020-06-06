PROJECT ?= s2png

BUILD_USER ?= $(USER)
USER_TEMP ?= /tmp/$(BUILD_USER)
PROJECT_TEMP ?= $(USER_TEMP)/cargo/$(PROJECT)

TARGET ?= x86_64-unknown-linux-musl
BUILD_OPTS ?= --target $(TARGET)
BUILD_OPTS_WITH_DIR ?= $(BUILD_OPTS) --target-dir $(PROJECT_TEMP)

dev: temp-dir
	# A workaround for https://github.com/rust-lang/rust/issues/46981
	find Cargo.* src/ tests/ | entr -r sh -c 'cargo check $(BUILD_OPTS_WITH_DIR) < /dev/null'

debug: temp-dir
	cargo build $(BUILD_OPTS_WITH_DIR)

install:
	install $(PROJECT_TEMP)/$(TARGET)/release/$(PROJECT) /usr/local/bin
	strip /usr/local/bin/$(PROJECT)

release: temp-dir
	cargo build $(BUILD_OPTS_WITH_DIR) --release

run:
	cargo run $(BUILD_OPTS_WITH_DIR) -- $(ARGS)

temp-dir:
	@-mkdir -m 0700 $(USER_TEMP)/ 2> /dev/null
	@-mkdir -p $(PROJECT_TEMP)/ 2> /dev/null

test: test-unit test-integration

test-integration: debug
	S2PNG_COMMAND="$(PROJECT_TEMP)/$(TARGET)/debug/$(PROJECT)" cargo test $(BUILD_OPTS_WITH_DIR) -- --ignored

test-unit:
	cargo test $(BUILD_OPTS_WITH_DIR)

uninstall:
	rm /usr/local/bin/$(PROJECT)

PHONY: dev install release temp-dir test test-integration test-unit uninstall
