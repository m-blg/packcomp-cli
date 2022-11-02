CLI_VERSION = 1.0.0
LIB_VERSION = 1.1.0

LIB_VERSION_MAJOR = $(word 1, $(subst ., , $(LIB_VERSION)))

ifndef debug
debug=0 # Default mode
endif

BUILD_DIR = target
BUILD_DIR_TESTS = target/tests
SRC_DIR = src
TEST_DIR = tests
INSTALL_PREFIX = /usr
INSTALL_PREFIX_BIN = /usr/bin

CC = g++
INCS = -Isrc
LIBS = -l:libcurl.so.4 -l:libjson-c.so.5 -l:libpackcomp.so.1
TEST_LIBS = -l:libcriterion.so.3
# LIBS = -lcurl -ljson-c

DEBFLAGS = -g -Wall
RELFLAGS = -O2
MODE_FLAGS = $(RELFLAGS)
ifeq ($(strip $(debug)),1)
MODE_FLAGS = $(DEBFLAGS)
endif

CPPFLAGS = -DVERSION=\"${CLI_VERSION}\"
CFLAGS = $(MODE_FLAGS) $(INCS) -fno-implicit-templates
LDFLAGS = $(LIBS)
TEST_LDFLAGS = $(LDFLAGS) $(TEST_LIBS)

CLI_BIN = packcomp

CLI_SRC = $(SRC_DIR)/main.cc

SRCS = $(shell find $(SRC_DIR) -name "*.cc")
HS = $(shell find $(SRC_DIR) -name "*.cc")
# SRCS_D1 = $(shell find $(SRC_DIR) -maxdepth 1 -name "*.cc")
SRCS_D1 = $(SRC_DIR)/packcomp.cc
TEST_SRCS = $(shell find $(TEST_DIR) -maxdepth 1 -name "*.cc")
TESTS = $(addprefix $(BUILD_DIR_TESTS)/, $(basename $(notdir $(TEST_SRCS))))


all: cli

cli: $(BUILD_DIR)/$(CLI_BIN)

test: lib $(TESTS)
	for test in $(TESTS) ; do \
		LD_LIBRARY_PATH=$(BUILD_DIR) ./$$test ; \
	done

test-bin: cli
	LD_LIBRARY_PATH=$(BUILD_DIR) $(BUILD_DIR)/$(CLI_BIN) -va armh,x86_64 p9 p10 -o $(TEST_DIR)/out.txt


run: 
ifdef test
	@echo "test: $(test)"
	@make $(BUILD_DIR_TESTS)/$(test)
	LD_LIBRARY_PATH=$(BUILD_DIR) $(BUILD_DIR_TESTS)/$(test)
endif

build: 
ifdef test
	@echo "test: $(test)"
	@make $(BUILD_DIR_TESTS)/$(test)
endif


install:
	mkdir -p $(INSTALL_PREFIX_BIN)
	cp -f $(BUILD_DIR)/$(CLI_BIN) $(INSTALL_PREFIX_BIN)
	chmod 755 $(INSTALL_PREFIX_BIN)/$(CLI_BIN)



uninstall:
	rm -f $(INSTALL_PREFIX_BIN)/$(CLI_BIN)

clean:
	$(RM) -r target


$(BUILD_DIR)/$(CLI_BIN): $(CLI_SRC)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(CLI_SRC) -o $@ $(LDFLAGS)

$(BUILD_DIR_TESTS)/%: $(TEST_DIR)/%.cc $(SRCS) $(HS)
	@mkdir -p $(BUILD_DIR_TESTS)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TEST_DIR)/$(notdir $@).cc -o $@ $(LDFLAGS) $(TEST_LDFLAGS)


.PHONY: all lib cli build run test test-bin 
.PHONY: install-lib install-cli install install-headers install-dev uninstall clean
