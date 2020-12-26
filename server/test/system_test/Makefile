SMTP_ST_DIR = $(ST_DIR)/smtp
MAILDIR_ST_DIR = $(ST_DIR)/maildir
EVENT_LOOP_ST_DIR  = $(ST_DIR)/event_loop
ST_BIN_DIR = $(BIN_DIR)/system_test


include $(MAILDIR_ST_DIR)/Makefile
include $(SMTP_ST_DIR)/Makefile
include $(EVENT_LOOP_ST_DIR)/Makefile

st_mkdir:
	mkdir -p $(ST_BIN_DIR)