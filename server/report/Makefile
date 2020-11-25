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
#find src -name "*.c"


report: lcov_tex unit_tests_tex
	cd $(PATH_TEX) && $(PDF) $(PATH_TEX)/main.tex

lcov_tex: 
	$(COV_TEXGEN) $(ROOT)/$(COV_REPORT) -o $(PATH_INCLUDE_TEX)/lcov

unit_tests_tex: unit_tests
	$(UNIT_TEST_EXE) > $(PATH_INCLUDE_TEX)/unit_test_out.tex

$(PATH_INCLUDE_TEX)/cflow.dot: $(CFLOWFILES)
	$(CFLOW) $^ | $(CFLOW2DOT) > $@ 

cflow: $(PATH_INCLUDE_TEX)/cflow.dot


.PHONY: report lcov_tex unit_tests_tex cflow