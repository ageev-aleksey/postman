CWD_REPORT = $(ROOT)/report
UTILS = $(CWD_REPORT)/utils
PATH_TEX = $(CWD_REPORT)/tex
PATH_INCLUDE_TEX = $(PATH_TEX)/include
COV_REPORT = coverage_report.info
COV_TEXGEN =  $(UTILS)/lcov-latex/gentex
PDF = pdflatex
CFLOW = cflow --level "0= "
CFLOW2DOT = $(UTILS)/cflow2dot
CFLOWFILES = $(shell find $(ROOT)/$(SRC_DIR) -name "*.c")
FSM2DOT = $(UTILS)/fsm2dot
DOT = dot
#find src -name "*.c"


report: lcov_tex unit_tests_tex $(PATH_INCLUDE_TEX)/smtp_fsm.pdf cflow
	cd $(PATH_TEX) && $(PDF) $(PATH_TEX)/main.tex

lcov_tex: 
	$(COV_TEXGEN) $(ROOT)/$(COV_REPORT) -o $(PATH_INCLUDE_TEX)/lcov

unit_tests_tex: unit_tests 
	valgrind --log-file=$(PATH_INCLUDE_TEX)/valgrind_out.tex $(UNIT_TEST_EXE) > $(PATH_INCLUDE_TEX)/unit_test_out.tex

$(PATH_INCLUDE_TEX)/cflow.dot: $(CFLOWFILES)
	$(CFLOW) $^ | $(CFLOW2DOT) > $@ 

$(PATH_INCLUDE_TEX)/cflow.pdf: $(PATH_INCLUDE_TEX)/cflow.dot
	$(DOT) -Tpdf -o $@ $<

cflow: $(PATH_INCLUDE_TEX)/cflow.pdf

$(PATH_INCLUDE_TEX)/smtp_fsm.dot: $(SMTP_FSM_DEF)
	 $(FSM2DOT) $(SMTP_FSM_DEF) > $@

$(PATH_INCLUDE_TEX)/smtp_fsm.pdf: $(PATH_INCLUDE_TEX)/smtp_fsm.dot
	$(DOT) -Tpdf -o $@ $< 

smtp_fsm_pdf: $(PATH_INCLUDE_TEX)/smtp_fsm.pdf

.PHONY: report lcov_tex unit_tests_tex cflow smtp_fsm_pdf