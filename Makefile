# Web Virtual Filesystem (WVFS) # Build Makefile

SRC_DIR := src
BUILD_DIR := build
EMSCRIPTEN := emcc

C_SRCS := $(SRC_DIR)/filesystem.c
DEV_SRCS := $(SRC_DIR)/dev/null.c
ALL_SRCS := $(C_SRCS) $(DEV_SRCS)

OUTPUT_JS := $(BUILD_DIR)/wvfs.js
OUTPUT_WASM := $(BUILD_DIR)/wvfs.wasm

CFLAGS := -O2 -Wall -I$(SRC_DIR)
EMFLAGS := -O3 -s MODULARIZE=1 -s EXPORT_NAME="WVFS" \
  -s EXPORTED_FUNCTIONS='["_wvfs_init","_wvfs_mkdir","_wvfs_touch","_wvfs_ls","_wvfs_cd","_wvfs_pwd","_wvfs_write_file","_wvfs_read_file"]' \
  -sEXPORTED_RUNTIME_METHODS='["cwrap","ccall"]'

.PHONY: all clean

all: $(OUTPUT_JS)

$(OUTPUT_JS): $(ALL_SRCS)
	@mkdir -p $(BUILD_DIR)
	$(EMSCRIPTEN) $(ALL_SRCS) $(EMFLAGS) -I$(SRC_DIR) -o $(OUTPUT_JS)

clean:
	rm -rf $(BUILD_DIR)
