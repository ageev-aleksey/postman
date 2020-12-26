EVENT_LOOP_ST_TIMER_TEST = $(EVENT_LOOP_ST_DIR)/timer
SYSTEM_TEST_TARGETS += event_loop_st_timer

event_loop_st_timer:
	$(CC) $(CCFLAGS) $(EVENT_LOOP_ST_TIMER_TEST)/main.c $(INCLUDES) -L$(BIN_DIR) -leventloop -ladditional -lpthread -o $(ST_BIN_DIR)/event_loop_timer.out