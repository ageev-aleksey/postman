TEST_SRC_DIR = $(TEST_DIR)/src
TEST_SRC = ${shell find $(TEST_DIR)/src -name "*.c"}
TEST_OBJS = $(patsubst $(TEST_SRC_DIR)/%.c, $(BUILD_DIR)/test/%.o, $(TEST_SRC))
TEST_INCLUDES = $(INCLUDES) -I$(TEST_DIR)/include
ST_DIR = $(TEST_DIR)/system_test





$(BUILD_DIR)/test/%.o: $(TEST_SRC_DIR)/%.c
	$(CC) $(CCFLAGS) $< $(TEST_INCLUDES) -o $@ -c

$(BIN_DIR)/unit_tests.out: test_mkdir $(TEST_OBJS) $(ALL_SERVER_LIBS_PATH)
	$(CC) $(CCFLAGS) $(TEST_DIR)/main.c $(TEST_OBJS) $(TEST_INCLUDES) -L$(BIN_DIR) $(LINKING_SERVER_LIBS) -lcunit -lpthread -o $@

unit_tests: $(BIN_DIR)/unit_tests.out

test_mkdir: mkdir
	mkdir -p $(BUILD_DIR)/test/


.PHONY: unit_tests test_mkdir

CLEAN_FILES += $(TEST_OBJS) $(BIN_DIR)/unit_tests.out

include $(ST_DIR)/Makefile