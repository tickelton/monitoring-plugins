DEBUG_FLAGS=-g -O0
COMPILER_FLAGS=-O2
WARNING_FLAGS=-Wall
SRCDIR=src
SRCS=$(SRCDIR)/check_twamp-light.c
BIN=check_twamp-light

all: build

build: $(SRCS)
	@clang $(DEBUG_FLAGS) $(WARNING_FLAGS) -o $(BIN) $(SRCS)

clean:
	@rm -f $(BIN)
