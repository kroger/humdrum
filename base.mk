INSTALL_PROGRAM = install
prefix = /usr/local/humdrum
bindir = bin

CC = gcc
#STATIC = -static
STATIC = 
CFLAGS = -O2 -Wall -ansi $(STATIC)

SYSTEM = $(shell uname -s)

ifeq ($(SYSTEM), Linux)
	PROJ = $(NAME)
else ifeq ($(findstring CYGWIN,$(SYSTEM)), CYGWIN)
	PROJ = $(NAME).exe
endif

$(PROJ): $(SRC) $(INCS)
	$(CC) $(CFLAGS) -o $@ $<

$(prefix)/$(bindir)/helpscrn:
	mkdir -p $(prefix)/$(bindir)/helpscrn

install: $(prefix)/$(bindir)/helpscrn
	$(INSTALL_PROGRAM) -s $(PROJ) $(prefix)/$(bindir)/

clean:	
	-rm $(PROJ)
