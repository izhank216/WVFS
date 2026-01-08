# Web Virtual Filesystem (WVFS) # Build Makefile

SRC_DIR := src
BUILD_DIR := build
EMSCRIPTEN := emcc

C_SRCS := $(SRC_DIR)/filesystem.c $(SRC_DIR)/dev/null.c

OUTPUT_JS := $(BUILD_DIR)/wvfs.js

EMFLAGS := -O3 -s MODULARIZE=1 -s EXPORT_NAME="WVFS" \
  -s EXPORTED_FUNCTIONS='["_wvfs_init","_wvfs_mkdir","_wvfs_touch","_wvfs_ls","_wvfs_cd","_wvfs_pwd","_wvfs_write_file","_wvfs_read_file"]' \
  -sEXPORTED_RUNTIME_METHODS='["cwrap","ccall"]' \
  -Isrc \
  -sALLOW_MEMORY_GROWTH=1 \
  -sSTRICT=1

.PHONY: all clean

all: $(OUTPUT_JS)

$(OUTPUT_JS): $(C_SRCS)
	@mkdir -p $(BUILD_DIR)
	$(EMSCRIPTEN) $(C_SRCS) $(EMFLAGS) -o $(OUTPUT_JS)

clean:
	rm -rf $(BUILD_DIR)
