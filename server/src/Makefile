CWD = $(ROOT)/src
LIB_DIR = lib
EVENT_LOOP_SRC_PATH = $(LIB_DIR)/event_loop
LOG_SRC_PATH = $(LIB_DIR)/log
MAILDIR_SRC_PATH = $(LIB_DIR)/maildir
ADDITIONAl_SRC_PATH = $(LIB_DIR)
SMTP_FSM_SRC_PATH = $(LIB_DIR)/smtp

EVENT_LOOP_SRC = ${shell find  $(CWD)/$(EVENT_LOOP_SRC_PATH) -name "*.c"}
LOG_SRC = ${shell find  $(CWD)/$(LOG_SRC_PATH) -name "*.c"}
MAILDIR_SRC = ${shell find  $(CWD)/$(MAILDIR_SRC_PATH) -name "*.c"}
ADDITIONAl_SRC = ${shell find  $(CWD)/$(ADDITIONAl_SRC_PATH) -maxdepth 1  -name "*.c"}
SMTP_FSM_SRC = ${shell find  $(CWD)/$(SMTP_FSM_SRC_PATH) -name "*.c"}
SMTP_FSM_DEF = $(CWD)/$(SMTP_FSM_SRC_PATH)/smtp-states.def

EVENT_LOOP_OBJS = $(patsubst $(CWD)/$(EVENT_LOOP_SRC_PATH)/%.c, $(BUILD_DIR)/event_loop/%.o, $(EVENT_LOOP_SRC))
LOG_OBJS = $(patsubst $(CWD)/$(LOG_SRC_PATH)/%.c, $(BUILD_DIR)/log/%.o, $(LOG_SRC))
MAILDIR_OBJS = $(patsubst $(CWD)/$(MAILDIR_SRC_PATH)/%.c, $(BUILD_DIR)/maildir/%.o, $(MAILDIR_SRC))
ADDITIONAL_OBJS = $(patsubst $(CWD)/$(ADDITIONAl_SRC_PATH)/%.c, $(BUILD_DIR)/%.o, $(ADDITIONAl_SRC))
SMTP_FSM_OBJS = $(patsubst $(CWD)/$(SMTP_FSM_SRC_PATH)/%.c, $(BUILD_DIR)/smtp/%.o, $(SMTP_FSM_SRC))


LIB_EVENT_LOOP_PATH = $(BIN_DIR)/libeventloop.a
LIB_LOG_PATH = $(BIN_DIR)/liblog.a
LIB_MAILDIR_PATH = $(BIN_DIR)/libmaildir.a
LIB_ADDITIONAL_PATH = $(BIN_DIR)/libadditional.a

ALL_SERVER_LIBS_PATH = $(BIN_DIR)/libeventloop.a $(BIN_DIR)/liblog.a  $(BIN_DIR)/libadditional.a
LINKING_SERVER_LIBS = -leventloop -llog -ladditional
AUTOGEN_FLAGS = -DTEST_CHECK_OPTS `autoopts-config cflags`

#event_loop
$(BUILD_DIR)/event_loop/%.o: $(CWD)/$(EVENT_LOOP_SRC_PATH)/%.c
	$(CC) $(CCFLAGS) $< $(INCLUDES) $(LINKS) -o $@ -c

$(BIN_DIR)/libeventloop.a: src_mkdir $(EVENT_LOOP_OBJS)
	ar rc $@ $(EVENT_LOOP_OBJS)

lib_event_loop: $(BIN_DIR)/lib_event_loop.a


#log
$(BUILD_DIR)/log/%.o: $(CWD)/$(LOG_SRC_PATH)/%.c
	$(CC) $(CCFLAGS) $< $(INCLUDES) $(LINKS) -o $@ -c

$(BIN_DIR)/liblog.a: src_mkdir $(LOG_OBJS)
	ar rc $@ $(LOG_OBJS)

lib_log: $(BIN_DIR)/lib_log.a


#maildir
$(BUILD_DIR)/maildir/%.o: $(CWD)/$(MAILDIR_SRC_PATH)/%.c
	$(CC) $(CCFLAGS) $< $(INCLUDES) $(LINKS) -o $@ -c

$(BIN_DIR)/libmaildir.a: src_mkdir $(MAILDIR_OBJS)
	ar rc $@ $(MAILDIR_OBJS)

lib_maildir: $(BIN_DIR)/lib_maildir.a


#additional
$(BUILD_DIR)/%.o: $(CWD)/$(ADDITIONAl_SRC_PATH)/%.c
	$(CC) $(CCFLAGS) $< $(INCLUDES) $(LINKS) -o $@ -c

$(BIN_DIR)/libadditional.a: src_mkdir $(ADDITIONAL_OBJS)
	ar rc $@ $(ADDITIONAL_OBJS)

lib_additional: $(BIN_DIR)/lib_additional.a


#smpt_fsm
$(CWD)/$(SMTP_FSM_SRC_PATH)/smtp-state-fsm.c: $(SMTP_FSM_DEF)
	cd $(CWD)/$(SMTP_FSM_SRC_PATH) && autogen $(SMTP_FSM_DEF)

$(BUILD_DIR)/smtp/%.o: $(CWD)/$(SMTP_FSM_SRC_PATH)/%.c
	$(CC) $(CCFLAGS) $(AUTOGEN_FLAGS) $< $(INCLUDES) -I$(SMTP_FSM_SRC) $(LINKS) -o $@ -c

$(BIN_DIR)/libsmtpfsm.a: src_mkdir $(SMTP_FSM_OBJS)
	ar rc $@ $(SMTP_FSM_OBJS)

lib_smtpfsm: $(BIN_DIR)/lib_smtpfsm.a


#server
$(BIN_DIR)/server.out: $(ALL_SERVER_LIBS_PATH)
	$(CC) $(CCFLAGS) $(INCLUDES) $(CWD)/main.c -L$(BIN_DIR) $(LINKING_SERVER_LIBS) -lpthread -o $@

server: $(BIN_DIR)/server.out


src_mkdir: mkdir
	mkdir -p $(BUILD_DIR)/event_loop
	mkdir -p $(BUILD_DIR)/log
	mkdir -p $(BUILD_DIR)/maildir

.PHONY: lib_event_loop lib_log lib_maildir lib_additional mkdir

CLEAN_FILES += $(EVENT_LOOP_OBJS) $(LOG_OBJS) $(MAILDIR_OBJS) $(ADDITIONAl_OBJS) $(BIN_DIR)/server.out \
$(BIN_DIR)/libadditional.a  $(BIN_DIR)/liblog.a $(BIN_DIR)/libevent_loop.a