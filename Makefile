.PHONY: all install humdrum clean humextra improv

all: humdrum improv humextra

include base.mk

install: $(prefix)/$(bindir)/
	for dir in toolkit/src/*; do $(MAKE) -C $$dir install; done
	$(INSTALL_PROGRAM) sh/* $(prefix)/$(bindir)/ 
	$(INSTALL_PROGRAM) awk/* $(prefix)/$(bindir)/
	$(INSTALL_PROGRAM) helpscrn/* $(prefix)/$(bindir)/helpscrn/
	$(INSTALL_PROGRAM) humextra/bin/* $(prefix)/$(bindir)/

humdrum:
	for dir in toolkit/src/*; do $(MAKE) -C $$dir; done

clean:
	for dir in toolkit/src/*; do $(MAKE) clean -C $$dir; done
	$(MAKE) -C humextra clean

humextra:
	$(MAKE) -C humextra library programs

improv:
	$(MAKE) -C improv library programs

get-improv:
	wget http://improv.sapp.org/cgi-bin/improv -O improv-$$(date --iso).tar.bz2
	tar xvjf improv-$$(date --iso).tar.bz2

get-humextra:
	wget http://extras.humdrum.org/cgi-bin/humextra -O humextra-$$(date --iso).tar.bz2
	tar xvjf humextra-$$(date --iso).tar.bz2
