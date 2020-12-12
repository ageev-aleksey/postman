CONFIG_SRC_PATH = $(SRC_DIR)/config
CONFIG_INCLUDE_PATH = $(INCLUDES)/config
CONFIG_BUILD_DIR = $(BUILD_DIR)/config

CONFIG_OBJ = $(patsubst $(CONFIG_SRC_PATH)/%.c, $(CONFIG_BUILD_DIR)/%.o, $(wildcard $(CONFIG_SRC_PATH)/*.c))

$(CONFIG_BUILD_DIR)/%.o: $(CONFIG_SRC_PATH)/%.c
	$(CC) $(CCFLAGS) $< $(CONFIG_INCLUDE_PATH) -lconfig -o $@ -c

$(BIN_DIR)/client.out: $(CONFIG_OBJ)
	$(CC) $(CCFLAGS) $(INCLUDES) $(CONFIG_INCLUDE_PATH) $< $(SRC_DIR)/main.c -lpthread -o $@

src_mkdir:
	mkdir -p $(CONFIG_BUILD_DIR)

build: $(BIN_DIR)/client.out