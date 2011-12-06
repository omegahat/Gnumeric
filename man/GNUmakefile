OMEGA_HOME=../../..
XSL_HELP_DIR=$(OMEGA_HOME)/XML/RSHelp

#USE_XALAN=1

XSLT_PROCESSOR=xsltproc

ifdef USE_XALAN

ifndef XSLT_PROCESSOR

 XSLT_PROCESSOR=LD_LIBRARY_PATH=/duncan/xml-xalan/c/lib ; export LD_LIBRARY_PATH ; testXSLT 

endif
%.Rd: %.xml ${XSL_HELP_DIR}/SFunctionHelp_Rd.xsl 
	($(XSLT_PROCESSOR) -xsl ${XSL_HELP_DIR}/SFunctionHelp_Rd.xsl -in $< -out $@)
%.html: %.xml GNUmakefile
	($(XSLT_PROCESSOR) -xsl ${XSL_HELP_DIR}/SFunctionHelp.xsl -in $< -out $@)

else

ifndef XSLT_PROCESSOR
 XSLT_PROCESSOR=/var/duncan/libxslt-0.13.0/libxslt/xsltproc
endif
%.Rd: %.xml ${XSL_HELP_DIR}/SFunctionHelp_Rd.xsl 
	$(XSLT_PROCESSOR) ${XSL_HELP_DIR}/SFunctionHelp_Rd.xsl $< > $@

%.html: %.xml GNUmakefile $(XSL_HELP_DIR)/SFunctionHelp.xsl
	$(XSLT_PROCESSOR) --param packageName "'RGnumeric'" ${XSL_HELP_DIR}/SFunctionHelp.xsl  $< > $@

endif

SRC=$(shell ls *.xml)

all: Rd $(SRC:%.xml=%.html)

Rd: $(SRC:%.xml=%.Rd)

clean:
	for f in $(SRC:%.xml=%.Rd) $(SRC:%.xml=%.html) ; do \
	 if test -e $$f ; then rm $$f ; fi ; \
	done
