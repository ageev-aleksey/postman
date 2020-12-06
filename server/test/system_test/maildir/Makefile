MAILDIR_ST_INITIALIZE = $(MAILDIR_ST_DIR)/initialize
MAILDIR_ST_CREATE_USER_MESSAGE = $(MAILDIR_ST_DIR)/create_user_message
SYSTEM_TEST_TARGETS += maildir_st_initialize
SYSTEM_TEST_TARGETS += maildir_st_create_user_message

maildir_st_initialize: st_mkdir $(LIB_MAILDIR_PATH) $(LIB_ADDITIONAL_PATH)
	$(CC) $(CCFLAGS) $(MAILDIR_ST_INITIALIZE)/main.c $(INCLUDES) -L$(BIN_DIR) -lmaildir -ladditional -o $(ST_BIN_DIR)/maildir_init.out

maildir_st_create_user_message: st_mkdir $(LIB_MAILDIR_PATH) $(LIB_ADDITIONAL_PATH)
	$(CC) $(CCFLAGS) $(MAILDIR_ST_CREATE_USER_MESSAGE)/main.c $(INCLUDES) -L$(BIN_DIR) -lmaildir -ladditional -o $(ST_BIN_DIR)/maildir_create_user_message.out