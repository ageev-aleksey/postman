ROOT= $(dir $(realpath $(firstword $(MAKEFILE_LIST))))
#ROOT= ./
INCLUDE_DIR = include
SRC_DIR = src
TEST_DIR = test
DOXYGEN_DIR = doxygen
BIN_DIR = bin
BUILD_DIR = build
REPORT_DIR = report
OPTION_COMPILE_UNIT_TESTS = ON
INCLUDES = -I$(ROOT)$(INCLUDE_DIR)
CC = gcc-4.8
CCFLAGS = -std=c99 -DDEBUG -g  #--coverage #-Wall -Werror

mkdir:
	mkdir -p $(BIN_DIR)
	mkdir -p $(BUILD_DIR)

include $(SRC_DIR)/Makefile
include $(TEST_DIR)/Makefile
include $(REPORT_DIR)/Makefile


all: server unit_tests $(SYSTEM_TEST_TARGETS) report

ut: unit_tests

clean:
	rm $(CLEAN_FILES)