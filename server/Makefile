INCLUDES = -Iinclude
SRC_DIR = src
OBJECTS_DIR = build
LIB_PATH = $(SRC_DIR)/lib
BIN_PATH = bin
TEST_PATH = test
SYSTEM_TEST_PATH = $(TEST_PATH)/system_test
TEST_SRC = $(wildcard $(TEST_PATH)/src/*.c)
SRC = $(wildcard $(SRC_DIR)/lib/*.c)
SRC += $(wildcard $(SRC_DIR)/lib/event_loop/*.c)
SRC += $(wildcard $(SRC_DIR)/lib/protocol/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJECTS_DIR)/%.o, $(SRC))
TEST_OBJS = $(patsubst $(TEST_PATH)/src/%.c, $(OBJECTS_DIR)/test/src/%.o, $(TEST_SRC))
CC = gcc-4.8
CCFLAGS = -std=c99 #-Wall -Werror
LFLAGS = -L$(BIN_PATH) -lserver -lpthread 

.PHONY: release debug mkdir clean test

$(OBJECTS_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CCFLAGS) -c $< -o $@  $(INCLUDES) $(ADDITIONAL_FLAGS) -D__FILENAME__='"$<"'

$(OBJECTS_DIR)/lib/%.o: $(SRC_DIR)/lib/%.c
	$(CC) $(CCFLAGS) -c $< -o $@  $(INCLUDES)  $(ADDITIONAL_FLAGS) -D__FILENAME__='"$<"'

$(OBJECTS_DIR)/test/%.o: $(TEST_PATH)/%.c
	$(CC) $(CCFLAGS) -c $< -o $@  $(INCLUDES) $(ADDITIONAL_FLAGS) -D__FILENAME__='"$<"'

$(OBJECTS_DIR)/test/src/%.o: $(TEST_PATH)/src/%.c
	$(CC) $(CCFLAGS) -c $< -o $@ $(INCLUDES) $(ADDITIONAL_FLAGS) -D__FILENAME__='"$<"'

all: debug test system_test

release: $(BIN_PATH)/server.out

debug: CCFLAGS += -DDEBUG -g
debug: ADDITIONAL_FLAGS += -fprofile-arcs -ftest-coverage
debug: $(BIN_PATH)/server.out

test: CCFLAGS += -DDEBUG -g -fprofile-arcs -ftest-coverage -D_TESTING_
test: INCLUDES += -I$(TEST_PATH)/include
test: $(TEST_OBJS) $(BIN_PATH)/libserver.a
	$(CC) $(CCFLAGS) $(INCLUDES) $(TEST_PATH)/main.c $(TEST_OBJS) -L$(BIN_PATH) -lserver  -lcunit -lpthread -lgcov -o $(BIN_PATH)/test.out

$(BIN_PATH)/system_test_echo_with_context.out: $(SYSTEM_TEST_PATH)/echo_with_context/main.c $(BIN_PATH)/libserver.a
	$(CC) $(CCFLAGS) $(INCLUDES) $<  -L$(BIN_PATH) -lserver -lpthread -o $@

system_test: CCFLAGS += -DDEBUG -g
system_test: $(BIN_PATH)/system_test_echo_with_context.out

echo: $(OBJS)
	echo $<


$(BIN_PATH)/libserver.a: $(OBJS)
	ar rc $@ $(OBJS)

$(BIN_PATH)/server.out: $(OBJECTS_DIR)/main.o $(BIN_PATH)/libserver.a
	$(CC) $(CCFLAGS) $(INCLUDES) $< -L$(BIN_PATH) -lserver -lpthread -o $@

mkdir:
	mkdir -p $(OBJECTS_DIR)/lib/event_loop
	mkdir -p $(OBJECTS_DIR)/lib/protocol
	mkdir -p $(OBJECTS_DIR)/test/src
	mkdir -p $(OBJECTS_DIR)/test/system_test/st_echo_with_context
	mkdir -p $(BIN_PATH)

clean:
	rm -rf $(OBJS) $(TEST_OBJS) $(OBJECTS_DIR)/main.o
