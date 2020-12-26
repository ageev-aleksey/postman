CONFIG_SRC_PATH = $(SRC_DIR)/config
CONFIG_BUILD_DIR = $(BUILD_DIR)/config
CONFIG_OBJ = $(patsubst $(CONFIG_SRC_PATH)/%.c, $(CONFIG_BUILD_DIR)/%.o, $(wildcard $(CONFIG_SRC_PATH)/*.c))

$(CONFIG_BUILD_DIR)/%.o: $(CONFIG_SRC_PATH)/%.c
	$(CC) $(CCFLAGS) $< $(INCLUDES) $(CONFIG_INCLUDES) -o $@ -c

LOG_SRC_PATH = $(SRC_DIR)/log
LOG_BUILD_DIR = $(BUILD_DIR)/log
LOG_OBJ = $(patsubst $(LOG_SRC_PATH)/%.c, $(LOG_BUILD_DIR)/%.o, $(wildcard $(LOG_SRC_PATH)/*.c))

$(LOG_BUILD_DIR)/%.o: $(LOG_SRC_PATH)/%.c
	$(CC) $(CCFLAGS) $< $(INCLUDES) $(LOG_INCLUDES) -o $@ -c

SMTP_SRC_PATH = $(SRC_DIR)/smtp
SMTP_BUILD_DIR = $(BUILD_DIR)/smtp
SMTP_OBJ = $(patsubst $(SMTP_SRC_PATH)/%.c, $(SMTP_BUILD_DIR)/%.o, $(wildcard $(SMTP_SRC_PATH)/*.c))

$(SMTP_BUILD_DIR)/%.o: $(SMTP_SRC_PATH)/%.c
	$(CC) $(CCFLAGS) $< $(INCLUDES) $(SMTP_INCLUDES) -o $@ -c

UTIL_SRC_PATH = $(SRC_DIR)/util
UTIL_BUILD_DIR = $(BUILD_DIR)/util
UTIL_OBJ = $(patsubst $(UTIL_SRC_PATH)/%.c, $(UTIL_BUILD_DIR)/%.o, $(wildcard $(UTIL_SRC_PATH)/*.c))

$(UTIL_BUILD_DIR)/%.o: $(UTIL_SRC_PATH)/%.c
	$(CC) $(CCFLAGS) $< $(INCLUDES) $(UTIL_INCLUDES) -o $@ -c

CONTEXT_SRC_PATH = $(SRC_DIR)/context
CONTEXT_BUILD_DIR = $(BUILD_DIR)/context
CONTEXT_OBJ = $(patsubst $(CONTEXT_SRC_PATH)/%.c, $(CONTEXT_BUILD_DIR)/%.o, $(wildcard $(CONTEXT_SRC_PATH)/*.c))

$(CONTEXT_BUILD_DIR)/%.o: $(CONTEXT_SRC_PATH)/%.c
	$(CC) $(CCFLAGS) $< $(INCLUDES) $(CONTEXT_INCLUDES) -o $@ -c

MAILDIR_SRC_PATH = $(SRC_DIR)/maildir
MAILDIR_BUILD_DIR = $(BUILD_DIR)/maildir
MAILDIR_OBJ = $(patsubst $(MAILDIR_SRC_PATH)/%.c, $(MAILDIR_BUILD_DIR)/%.o, $(wildcard $(MAILDIR_SRC_PATH)/*.c))

$(MAILDIR_BUILD_DIR)/%.o: $(MAILDIR_SRC_PATH)/%.c
	$(CC) $(CCFLAGS) $< $(INCLUDES) $(MAILDIR_INCLUDES) -o $@ -c

$(BIN_DIR)/client.out: $(CONFIG_OBJ) $(LOG_OBJ) $(SMTP_OBJ) $(UTIL_OBJ) $(CONTEXT_OBJ) $(MAILDIR_OBJ)
	$(CC) $(CCFLAGS) $(INCLUDES) $^ $(SRC_DIR)/main.c $(LIBS) -o $@

$(SMTP_STATES_GENERATED_SRC): $(SMTP_STATES_DEF)
	cd $(SMTP_STATES_GENERATED_SRC) && autogen $(SMTP_STATES_DEF)

build_mkdir:
	mkdir -p $(CONFIG_BUILD_DIR)
	mkdir -p $(LOG_BUILD_DIR)
	mkdir -p $(SMTP_BUILD_DIR)
	mkdir -p $(UTIL_BUILD_DIR)
	mkdir -p $(CONTEXT_BUILD_DIR)
	mkdir -p $(MAILDIR_BUILD_DIR)

build: $(BIN_DIR)/client.out
