ROOT= $(dir $(realpath $(firstword $(MAKEFILE_LIST))))
INCLUDES = -I$(ROOT)include -I/usr/include
SRC_DIR = $(ROOT)src
TEST_DIR = $(ROOT)test
BIN_DIR = $(ROOT)bin
BUILD_DIR = $(ROOT)build
REPORT_DIR = $(ROOT)report
RESOURCES_DIR = $(ROOT)resources
CC = gcc
CCFLAGS = -std=gnu99 -DDEBUG -D_GNU_SOURCE -g --coverage -Wall -Werror
CLEAN_DIR = $(BIN_DIR) $(BUILD_DIR)

include $(SRC_DIR)/Makefile
include $(TEST_DIR)/Makefile
include $(REPORT_DIR)/Makefile

prepare: clean src_mkdir
	mkdir -p $(BIN_DIR)
	mkdir -p $(BUILD_DIR)
	cp -R $(RESOURCES_DIR) $(BIN_DIR)/resources

all: mkdir build # ut build_report

unit_tests: ut

report: build_report

clean:
	rm -R $(CLEAN_DIR) -f