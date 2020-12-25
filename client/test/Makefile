TEST_SRC_PATH = $(TEST_DIR)/src
TEST_OBJ = $(patsubst $(TEST_SRC_PATH)/%.c, $(BUILD_DIR)/test/%.o, $(wildcard $(TEST_SRC_PATH)/*.c))
TEST_INCLUDES = $(INCLUDES) -I$(TEST_DIR)/include
CONFIG_OBJ = $(patsubst $(CONFIG_SRC_PATH)/%.c, $(CONFIG_BUILD_DIR)/%.o, $(wildcard $(CONFIG_SRC_PATH)/*.c))
LOG_OBJ = $(patsubst $(LOG_SRC_PATH)/%.c, $(LOG_BUILD_DIR)/%.o, $(wildcard $(LOG_SRC_PATH)/*.c))
SMTP_OBJ = $(patsubst $(SMTP_SRC_PATH)/%.c, $(SMTP_BUILD_DIR)/%.o, $(wildcard $(SMTP_SRC_PATH)/*.c))
UTIL_OBJ = $(patsubst $(UTIL_SRC_PATH)/%.c, $(UTIL_BUILD_DIR)/%.o, $(wildcard $(UTIL_SRC_PATH)/*.c))
CONTEXT_OBJ = $(patsubst $(CONTEXT_SRC_PATH)/%.c, $(CONTEXT_BUILD_DIR)/%.o, $(wildcard $(CONTEXT_SRC_PATH)/*.c))
MAILDIR_OBJ = $(patsubst $(MAILDIR_SRC_PATH)/%.c, $(MAILDIR_BUILD_DIR)/%.o, $(wildcard $(MAILDIR_SRC_PATH)/*.c))

UNIT_TEST_OUT = $(BIN_DIR)/unit_tests.out

$(BUILD_DIR)/test/%.o: $(TEST_SRC_PATH)/%.c
	$(CC) $(CCFLAGS) $< $(TEST_INCLUDES) -o $@ -c

$(UNIT_TEST_OUT): $(TEST_OBJ) $(CONFIG_OBJ) $(LOG_OBJ) $(SMTP_OBJ) $(UTIL_OBJ) $(CONTEXT_OBJ) $(MAILDIR_OBJ)
	$(CC) $(CCFLAGS) $(INCLUDES) $^ $(TEST_DIR)/main.c $(LIBS) -lcunit -o $@

CLEAN_DIR += $(BUILD_DIR)/test
CLEAN_DIR += $(BIN_DIR)/unit_tests.out

test_mkdir:
	mkdir -p $(BUILD_DIR)/test

ut: test_mkdir $(BIN_DIR)/unit_tests.out

.PHONY: test_mkdir ut