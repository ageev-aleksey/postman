CWD_REPORT = $(ROOT)report
UTILS = $(CWD_REPORT)/utils
PATH_TEX = $(CWD_REPORT)/tex
PATH_INCLUDE_TEX = $(PATH_TEX)/include
COV_REPORT = coverage_report.info
COV_TEXGEN = perl $(UTILS)/lcov-latex/gentex
#PDF = pdflatex -interaction nonstopmode -halt-on-error
PDF = pdflatex -interaction nonstopmode
CFLOW = cflow --level "0= "
CFLOW2DOT = python $(UTILS)/cflow2dot
CFLOWFILES = $(shell find $(ROOT)/$(SRC_DIR) -name "*.c")
FSM2DOT = python $(UTILS)/fsm2dot
MAKE2DOT = $(UTILS)/makefile2graph/make2graph
RE2TEX = python $(UTILS)/re2tex
DOT = dot
DOXYGEN_LATEX_PATH = $(ROOT)doxygen/latex
DOXYGEN_FILES = $(PATH_INCLUDE_TEX)/doxygen/event_loop.h.tex $(PATH_INCLUDE_TEX)/doxygen/smtp_state.h.tex \
				$(PATH_INCLUDE_TEX)/doxygen/maildir.h.tex $(PATH_INCLUDE_TEX)/doxygen/maildir_server.h.tex \
				$(PATH_INCLUDE_TEX)/doxygen/maildir_user.h.tex $(PATH_INCLUDE_TEX)/doxygen/maildir_user.h.tex

#find src -name "*.c"


report: $(PATH_INCLUDE_TEX)/unit_test_out.tex $(PATH_INCLUDE_TEX)/smtp_fsm.pdf \
		$(PATH_INCLUDE_TEX)/cflow.pdf $(PATH_INCLUDE_TEX)/make.pdf regex_to_tex doxygen_tex
	cd $(PATH_TEX) && $(PDF) main.tex
	cd $(PATH_TEX) && $(PDF) main.tex

#lcov_tex: $(ROOT)/$(COV_REPORT)
#	$(COV_TEXGEN) $< -o $(PATH_INCLUDE_TEX)/lcov

$(ROOT)/$(COV_REPORT): 
	lcov -t "server testing code coverage" -o $@ -c -d .


$(PATH_INCLUDE_TEX)/unit_test_out.tex: $(UNIT_TEST_EXE)
	valgrind --log-file=$(PATH_INCLUDE_TEX)/valgrind_out.tex $< > $@

#unit_tests_tex: $(UNIT_TEST_EXE)
#	valgrind --log-file=$(PATH_INCLUDE_TEX)/valgrind_out.tex $< > $(PATH_INCLUDE_TEX)/unit_test_out.tex

$(PATH_INCLUDE_TEX)/cflow.dot: $(CFLOWFILES)
	$(CFLOW) $^ | $(CFLOW2DOT) > $@ 

$(PATH_INCLUDE_TEX)/cflow.pdf: $(PATH_INCLUDE_TEX)/cflow.dot
	$(DOT) -Tpdf -o $@ $<

#cflow: $(PATH_INCLUDE_TEX)/cflow.pdf

$(PATH_INCLUDE_TEX)/smtp_fsm.dot: $(SMTP_FSM_DEF)
	 $(FSM2DOT) $(SMTP_FSM_DEF) > $@

$(PATH_INCLUDE_TEX)/smtp_fsm.pdf: $(PATH_INCLUDE_TEX)/smtp_fsm.dot
	$(DOT) -Tpdf -o $@ $< 

#smtp_fsm_pdf: $(PATH_INCLUDE_TEX)/smtp_fsm.pdf

$(PATH_INCLUDE_TEX)/make.dot: Makefile
	export LC_ALL=C && make -Bnd all | $(MAKE2DOT) > $(PATH_INCLUDE_TEX)/make.dot

$(PATH_INCLUDE_TEX)/make.pdf: $(PATH_INCLUDE_TEX)/make.dot
	dot -Tpdf -o $@ $<

#doxygen

$(DOXYGEN_LATEX_PATH)/event__loop_8h.tex: 
	doxygen Doxyfile

$(DOXYGEN_LATEX_PATH)/maildir_8h.tex:
	doxygen Doxyfile

$(DOXYGEN_LATEX_PATH)/message_8h.tex:
	doxygen Doxyfile

$(DOXYGEN_LATEX_PATH)/server_8h.tex:
	doxygen Doxyfile

$(DOXYGEN_LATEX_PATH)/user_8h.tex:
	doxygen Doxyfile

$(DOXYGEN_LATEX_PATH)/state_8h.tex:
	doxygen Doxyfile



$(PATH_INCLUDE_TEX)/doxygen/event_loop.h.tex: $(DOXYGEN_LATEX_PATH)/event__loop_8h.tex
	cp $< $@

$(PATH_INCLUDE_TEX)/doxygen/smtp_state.h.tex: $(DOXYGEN_LATEX_PATH)/state_8h.tex
	cp $< $@

$(PATH_INCLUDE_TEX)/doxygen/maildir.h.tex: $(DOXYGEN_LATEX_PATH)/maildir_8h.tex
	cp $< $@

$(PATH_INCLUDE_TEX)/doxygen/maildir_server.h.tex: $(DOXYGEN_LATEX_PATH)/message_8h.tex
	cp $< $@

$(PATH_INCLUDE_TEX)/doxygen/maildir_server.h.tex: $(DOXYGEN_LATEX_PATH)/server_8h.tex
	cp $< $@

$(PATH_INCLUDE_TEX)/doxygen/maildir_user.h.tex: $(DOXYGEN_LATEX_PATH)/user_8h.tex
	cp $< $@

$(PATH_INCLUDE_TEX)/doxygen/smtp_state.h.tex: $(DOXYGEN_LATEX_PATH)/state_8h.tex
	cp $< $@

$(PATH_INCLUDE_TEX)/doxygen/maildir_user.h.tex: $(DOXYGEN_LATEX_PATH)/user_8h.tex
	cp $< $@

	

doxygen_tex: $(DOXYGEN_FILES)

regex_to_tex: 
	 $(RE2TEX) $(ROOT)/include/smtp/regex.h $(PATH_INCLUDE_TEX)

mkdir_report:
	mkdir -p $(PATH_TEX)/include
	mkdir -p $(PATH_TEX)/include/re
	mkdir -p $(PATH_TEX)/include/lcov
	mkdir -p $(PATH_TEX)/include/doxygen

#.PHONY: report lcov_tex unit_tests_tex cflow smtp_fsm_pdf $(PATH_INCLUDE_TEX)/make.dot

.PHONY: regex_to_tex mkdir_report lcov_tex