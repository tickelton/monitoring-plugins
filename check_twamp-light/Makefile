DEBUG_FLAGS=-g -O0
COMPILER_FLAGS=-O2
WARNING_FLAGS=-Wall
SRCS=check_twamp-light.c
BIN=check_twamp-light

all: build

build: $(SRCS)
	@clang $(WARNING_FLAGS) $(COMPILER_FLAGS) -o $(BIN) $(SRCS)

clean:
	@rm -f $(BIN)
