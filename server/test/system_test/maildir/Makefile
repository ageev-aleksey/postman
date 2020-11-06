MAILDIR_ST_INITIALIZE = $(MAILDIR_ST_DIR)/initialize
SYSTEM_TEST_TARGETS += maildir_st_initialize


maildir_st_initialize: st_mkdir $(LIB_MAILDIR_PATH) $(LIB_ADDITIONAL_PATH)
	$(CC) $(CCFLAGS) $(MAILDIR_ST_INITIALIZE)/main.c $(INCLUDES) -L$(BIN_DIR) -lmaildir -ladditional -o $(ST_BIN_DIR)/maildir_init.out