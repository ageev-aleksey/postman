ROOT= $(dir $(realpath $(firstword $(MAKEFILE_LIST))))

CONFIG_INCLUDES = -I$(ROOT)include/config
LOG_INCLUDES = -I$(ROOT)include/log
MAILDIR_INCLUDES = -I$(ROOT)include/maildir
MESSAGE_QUEUE_INCLUDES = -I$(ROOT)include/message_queue
SMTP_INCLUDES = -I$(ROOT)include/smtp
UTIL_INCLUDES = -I$(ROOT)include/util
INCLUDES = -I$(ROOT)include $(CONFIG_INCLUDES) $(LOG_INCLUDES) $(MAILDIR_INCLUDES) $(MESSAGE_QUEUE_INCLUDES) $(SMTP_INCLUDES) $(UTIL_INCLUDES)
LIBS = -lconfig -lpthread

SRC_DIR = $(ROOT)src
TEST_DIR = $(ROOT)test
BIN_DIR = $(ROOT)bin
BUILD_DIR = $(ROOT)build
REPORT_DIR = $(ROOT)report
RESOURCES_DIR = $(ROOT)resources
CC = gcc
CCFLAGS = -std=gnu99 -DDEBUG -D_GNU_SOURCE -g --coverage # -Wall -Werror
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