SMTP_ST_FSM_TEST = $(SMTP_ST_DIR)/fsm_test
SYSTEM_TEST_TARGETS += st_smtp_fsm

st_smtp_fsm: $(BIN_DIR)/libsmtpprotocol.a $(BIN_DIR)/libadditional.a
	$(CC) $(CCFLAGS) $(SMTP_ST_FSM_TEST)/main.c $(INCLUDES) -I$(CWD)/$(SMTP_FSM_SRC_PATH) -L$(BIN_DIR) -lsmtpprotocol -ladditional -o $(ST_BIN_DIR)/fsm_step.out