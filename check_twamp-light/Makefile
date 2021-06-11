.PHONY: clean
.PHONY: distclean
.PHONY: test
.PHONY: help
.PHONY: format
.PHONY: format-diff
.PHONY: tidy
.PHONY: tidy-full

PATHU = unity/
PATHS = src/
PATHT = test/
PATHB = build/
PATHD = build/depends/
PATHO = build/objs/
PATHR = build/results/

BUILD_PATHS = $(PATHB) $(PATHD) $(PATHO) $(PATHR)

SRCT = $(wildcard $(PATHT)*.c)
SRCS = $(wildcard $(PATHS)*.c)
HEADERS = $(wildcard $(PATHS)*.h)
OBJS = $(patsubst $(PATHS)%.c,$(PATHO)%.o,$(SRCS) )

CC=clang
LD=clang
COMPILE=${CC} -c
DEPEND=${CC} -MM -MG -MF
CFLAGS=-I. -I$(PATHU) -I$(PATHS)
RM=rm
RMF=$(RM) -f
RMFR=$(RMF) -r
TARGET_EXTENSION=out

CFLAGS_DEBUG=-g -O0
CFLAGS_TEST=-DUNIT_TESTING
CFLAGS_RELEASE=-O3
WARNING_FLAGS=-Wall -Wextra

RESULTS = $(patsubst $(PATHT)test_%.c,$(PATHR)test_%.txt,$(SRCT) )

PASSED = `grep -s PASS $(PATHR)*.txt`
FAIL = `grep -s FAIL $(PATHR)*.txt`
IGNORE = `grep -s IGNORE $(PATHR)*.txt`

BIN=$(PATHB)check_twamp-light

all: release

release: CFLAGS+=$(CFLAGS_RELEASE)
release: $(BUILD_PATHS) $(BIN)

debug: CFLAGS+=$(CFLAGS_DEBUG)
debug: $(BUILD_PATHS) $(BIN)

$(BIN): $(OBJS)
	$(LD) -o $@ $^

test: CFLAGS+=$(CFLAGS_RELEASE)
test: CFLAGS+=$(CFLAGS_TEST)
test: $(BUILD_PATHS) $(RESULTS)
	@echo "-----------------------\nIGNORES:\n-----------------------"
	@echo "$(IGNORE)"
	@echo "-----------------------\nFAILURES:\n-----------------------"
	@echo "$(FAIL)"
	@echo "-----------------------\nPASSED:\n-----------------------"
	@echo "$(PASSED)"
	@echo "\nDONE"

$(PATHR)%.txt: $(PATHB)%.$(TARGET_EXTENSION)
	-./$< > $@ 2>&1

$(PATHB)test_%.$(TARGET_EXTENSION): $(PATHO)test_%.o $(PATHO)%.o $(PATHO)unity.o #$(PATHD)test_%.d
	$(LD) -o $@ $^

$(PATHO)%.o:: $(PATHT)%.c
	$(COMPILE) $(CFLAGS) $(WARNING_FLAGS) $< -o $@

$(PATHO)%.o:: $(PATHS)%.c
	$(COMPILE) $(CFLAGS) $(WARNING_FLAGS) $< -o $@

$(PATHO)%.o:: $(PATHU)%.c $(PATHU)%.h
	$(COMPILE) $(CFLAGS) $(WARNING_FLAGS) $< -o $@

$(PATHD)%.d:: $(PATHT)%.c
	$(DEPEND) $@ $<

$(PATHB):
	mkdir $(PATHB)

$(PATHD):
	mkdir $(PATHD)

$(PATHO):
	mkdir $(PATHO)

$(PATHR):
	mkdir $(PATHR)

clean:
	$(RMF) $(PATHO)*.o
	$(RMF) $(PATHB)*.$(TARGET_EXTENSION)
	$(RMF) $(PATHR)*.txt
	$(RMF) $(BIN)

distclean: clean
	$(RMFR) $(PATHB)

help:
	@echo "available targets:"
	@echo "release     :    release build"
	@echo "debug       :    debug build"
	@echo "test        :    build and run unit tests"
	@echo "help        :    show this help message"
	@echo "clean       :    delete build artifacts"
	@echo "distclean   :    delete entire build directory"
	@echo "format-diff :    show changes clang-format would make"
	@echo "format      :    format source files with clang-format"
	@echo "tidy        :    run static analysis with clang-tidy"
	@echo "tidy-full   :    run clang-tidy with more checks"

format-diff:
	@$(foreach infile,$(SRCT) $(SRCS) $(HEADER), clang-format \
		--style=Google \
		$(infile) \
		| diff -u $(infile) - \
		|| true \
		;)

format:
	@$(foreach infile,$(SRCT) $(SRCS) $(HEADER), clang-format \
		-i \
		--style=Google \
		$(infile) \
		|| true \
		;)

tidy:
	@$(foreach infile,$(SRCS), clang-tidy \
		-checks=-*,clang-analyzer-*,bugprone*,google*,misc*,modernize*,performance*,readability*,-*DeprecatedOrUnsafeBufferHandling,-*readability-magic-numbers \
		$(infile) \
		-- \
		|| true \
		;)

tidy-full:
	@$(foreach infile,$(SRCS), clang-tidy \
		-checks=-*,clang-analyzer-*,bugprone*,google*,misc*,modernize*,performance*,readability*,-*DeprecatedOrUnsafeBufferHandling \
		$(infile) \
		-- \
		|| true \
		;)

.PRECIOUS: $(PATHB)test_%.$(TARGET_EXTENSION)
.PRECIOUS: $(PATHD)%.d
.PRECIOUS: $(PATHO)%.o
.PRECIOUS: $(PATHR)%.txt
