ROOT= $(dir $(realpath $(firstword $(MAKEFILE_LIST))))

CONFIG_INCLUDES = -I$(ROOT)include/config
LOG_INCLUDES = -I$(ROOT)include/log
MAILDIR_INCLUDES = -I$(ROOT)include/maildir
SMTP_INCLUDES = -I$(ROOT)include/smtp
UTIL_INCLUDES = -I$(ROOT)include/util
CONTEXT_INCLUDES = -I$(ROOT)include/context
INCLUDES = -I$(ROOT)include
LIBS = -lconfig -lpthread -lresolv

SRC_DIR = $(ROOT)src
TEST_DIR = $(ROOT)test
BIN_DIR = $(ROOT)bin
BUILD_DIR = $(ROOT)build
REPORT_DIR = $(ROOT)report
RESOURCES_DIR = $(ROOT)resources
CC = gcc
CCFLAGS = -std=gnu99 -DDEBUG -D_GNU_SOURCE -g # -Wall -Werror
CLEAN_DIR = $(BIN_DIR) $(BUILD_DIR) $(ROOT)main.gcno

include $(SRC_DIR)/Makefile
include $(TEST_DIR)/Makefile
include $(REPORT_DIR)/Makefile

prepare: clean mkdir

all: prepare build # ut build_report

unit_tests: ut

report: build_report

mkdir: build_mkdir
	mkdir -p $(BIN_DIR)
	mkdir -p $(BUILD_DIR)
	cp -R $(RESOURCES_DIR) $(BIN_DIR)/resources

clean:
	rm -R $(CLEAN_DIR) -f