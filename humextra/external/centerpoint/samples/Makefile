#
# makefile
#
# $Id: //depot/XML/Main/public/samples/Makefile#4 $
#
# Makefile for CenterPoint/XML Samples
#
# The contents of this file are subject to the 
# CenterPoint Public License Version 1.0 (the "License"); 
# you may not use this file except in compliance with the 
# License. You may obtain a copy of the License at 
# http://www.cpointc.com/CPL
#
# Software distributed under the License is distributed on 
# an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either
# express or implied. See the License for the specific 
# language governing rights and limitations under the License.
#
# The Original Code is CenterPoint/XML.
#
# The Initial Developer of the Original Code is CenterPoint - 
# Connective Software Engineering GmbH. Portions created by
# CenterPoint - Connective Software Engineering GmbH. are 
# Copyright (c) 1998-2002 CenterPoint - Connective Software
# Engineering GmbH. All Rights Reserved.
#
# Contributor(s):
#

# Directories
SRCDIR   = src
INCDIR   = ../include
LIBDIR   = ../lib
OBJDIR   = obj
BINDIR   = bin

# Tools
CC     = gcc
#CC    = cc
CXX    = g++
#CXX   = aCC +W361
#CXX   = c++
RM     = rm -rf
MKDIR  = mkdir -p

# Flags
CFLAGS      = -I$(INCDIR) -L$(LIBDIR)
CXXFLAGS    = -I$(INCDIR) -L$(LIBDIR)
DEBUGOPT    = -g -D_DEBUG
RELEASEOPT  = -O2 -DNDEBUG
#RELEASEOPT = +O2 -DNDEBUG

# Targets
all: dirs $(BINDIR)/BaseTest $(BINDIR)/SAXTest $(BINDIR)/DocTest $(BINDIR)/DOMTest $(BINDIR)/WriterTest


dirs: $(BINDIR)


clean:
	$(RM) $(BINDIR)
	

$(BINDIR):
	$(MKDIR) $(BINDIR)


$(BINDIR)/BaseTest: $(SRCDIR)/BaseTest.cpp
	$(CXX) $(INCLUDE) $(CXXFLAGS) $(DEBUGOPT) $< -o $@ $(LIBRARY) -lcslxmld

$(BINDIR)/SAXTest: $(SRCDIR)/SAXTest.cpp
	$(CXX) $(INCLUDE) $(CXXFLAGS) $(DEBUGOPT) $< -o $@ $(LIBRARY) -lcslxmld

$(BINDIR)/DocTest: $(SRCDIR)/DocTest.cpp
	$(CXX) $(INCLUDE) $(CXXFLAGS) $(DEBUGOPT) $< -o $@ $(LIBRARY) -lcslxmld

$(BINDIR)/DOMTest: $(SRCDIR)/DOMTest.cpp
	$(CXX) $(INCLUDE) $(CXXFLAGS) $(DEBUGOPT) $< -o $@ $(LIBRARY) -lcslxmld

$(BINDIR)/WriterTest: $(SRCDIR)/WriterTest.cpp
	$(CXX) $(INCLUDE) $(CXXFLAGS) $(DEBUGOPT) $< -o $@ $(LIBRARY) -lcslxmld

