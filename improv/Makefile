## main improv GNU makefile for Linux on Intel computers.
##
## Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
## Creation Date: Fri Jan 15 05:00:19 PST 1999
## Last Modified: Fri May 28 00:20:41 PDT 1999
## Filename:      ...improv/Makefile
##
## Description: This Makefile can create the improv library or example programs
##              which use the improv library winth linux using g++ 
##              (gcc 2.7.2.1 or later)
##
## To run this makefile, type (without quotes) "make examples",
## "make library", "make improv", "make synthImprov", or "make batonImprov"
##

MAKE = make

# targets which don't actually refer to files
.PHONY : all clean examples library dynamic improv synthImprov batonImprov

###########################################################################
#                                                                         #
#                                                                         #

all:
	@echo ""
	@echo This makefile will create either the improv library file
	@echo or will compile the improv example programs.  You may
	@echo have to make the library first if it does not exist.
	@echo Type one of the following:
	@echo "   $(MAKE) library"
	@echo or
	@echo "   $(MAKE) examples"
	@echo ""
	@echo ""
	@echo To make a subset of example programs, type:
	@echo "   $(MAKE) [improv | synthImprov | batonImprov | etc.]"
	@echo ""
	@echo To compile a specific example program called xxx, type:
	@echo "   $(MAKE) xxx"
	@echo ""

library:
	$(MAKE) -f Makefile.library

clean:
	$(MAKE) -f Makefile.library clean

examples:
	$(MAKE) -f Makefile.examples

improv:
	$(MAKE) -f Makefile.examples improv

synthImprov:
	$(MAKE) -f Makefile.examples synthImprov

batonImprov:
	$(MAKE) -f Makefile.examples batonImprov

batonSynthImprov:
	$(MAKE) -f Makefile.examples batonSynthImprov

dynamic:
	@echo Making dynamic library
	$(MAKE) -f Makefile.dynamic

%: 
	@echo compiling file $@
	$(MAKE) -f Makefile.examples $@
	

#                                                                         #
#                                                                         #
###########################################################################



