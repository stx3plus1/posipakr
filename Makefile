CC=cc
WASM_CC=/opt/wasi-sdk/bin/clang
C_FILES=$(wildcard src/*.c)
FLAGS=-O3 -Wall
OUTPUT=posipakr

.PHONY: $(OUTPUT)

$(OUTPUT): ${C_FILES}
        @$(CC) ${C_FILES} $(FLAGS) -o $(OUTPUT)
        @strip $@

wasm: ${C_FILES}
        @$(WASM_CC) ${C_FILES} $(FLAGS) -o $(OUTPUT).wasm

# "make package" for iOS with theos installed, cross compile
# for other unix-like/certified operating systems
all: $(OUTPUT) wasm

clean:
        @rm -f $(OUTPUT)*
        @rm -f src/*.pch