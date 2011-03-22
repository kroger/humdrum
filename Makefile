.PHONY: all install humdrum clean humextra improv

TODAY = $(shell date "+%Y-%m-%d")

all: humdrum improv humextra

include base.mk

install: $(prefix)/$(bindir)/helpscrn/
	for dir in toolkit/src/*; do $(MAKE) -C $$dir install; done
	$(INSTALL_PROGRAM) toolkit/sh/* $(prefix)/$(bindir)/
	$(INSTALL_PROGRAM) toolkit/awk/* $(prefix)/$(bindir)/
	$(INSTALL_PROGRAM) toolkit/helpscrn/* $(prefix)/$(bindir)/helpscrn/
	$(INSTALL_PROGRAM) humextra/bin/* $(prefix)/$(bindir)/

humdrum:
	for dir in toolkit/src/*; do $(MAKE) -C $$dir; done

clean:
	for dir in toolkit/src/*; do $(MAKE) clean -C $$dir; done
	$(MAKE) -C humextra clean

humextra:
	$(MAKE) -C humextra library programs

improv:
	$(MAKE) -C improv library

get-improv:
	wget http://improv.sapp.org/cgi-bin/improv -O improv-$(TODAY).tar.bz2
	tar xvjf improv-$(TODAY).tar.bz2

get-humextra:
	wget http://extras.humdrum.org/cgi-bin/humextra -O humextra-$(TODAY).tar.bz2
	tar xvjf humextra-$(TODAY).tar.bz2
