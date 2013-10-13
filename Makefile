.PHONY: all install humdrum clean

TODAY = $(shell date "+%Y-%m-%d")

all: humdrum

include base.mk

install: $(prefix)/$(bindir)/helpscrn/
	for dir in toolkit/src/*; do $(MAKE) -C $$dir install; done
	$(INSTALL_PROGRAM) toolkit/sh/* $(prefix)/$(bindir)/
	$(INSTALL_PROGRAM) toolkit/awk/* $(prefix)/$(bindir)/
	$(INSTALL_PROGRAM) toolkit/helpscrn/* $(prefix)/$(bindir)/helpscrn/

humdrum:
	for dir in toolkit/src/*; do $(MAKE) -C $$dir; done

clean:
	for dir in toolkit/src/*; do $(MAKE) clean -C $$dir; done

