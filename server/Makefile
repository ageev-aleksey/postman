ROOT= $(dir $(realpath $(firstword $(MAKEFILE_LIST))))
#ROOT= ./
INCLUDE_DIR = include
SRC_DIR = src
TEST_DIR = test
DOXYGEN_DIR = doxygen
BIN_DIR = bin
BUILD_DIR = build
REPORT_DIR = report
RESOURCE_PATH = resource
OPTION_COMPILE_UNIT_TESTS = ON
INCLUDES = -I$(ROOT)$(INCLUDE_DIR)
CC = gcc-4.8
CCFLAGS = -std=gnu99 -DDEBUG -D_GNU_SOURCE -g --coverage -Wall -Werror  -Wno-address -Wno-deprecated-declarations


include $(SRC_DIR)/Makefile
include $(TEST_DIR)/Makefile
include $(REPORT_DIR)/Makefile


mkdir: src_mkdir mkdir_report test_mkdir st_mkdir
	mkdir -p $(BIN_DIR)
	mkdir -p $(BUILD_DIR)
	mkdir -p doxygen

all: server unit_tests $(SYSTEM_TEST_TARGETS) report

ut: unit_tests

clean: report_clean 
	rm -R ./bin
	rm -R ./build
