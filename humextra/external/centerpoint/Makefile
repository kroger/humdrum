#
# Makefile
#
# $Id: //depot/XML/Main/public/Makefile#3 $
#
# Makefile for CenterPoint/XML
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
INCDIR   = include
LIBDIR   = lib
OBJDIR   = obj
ARCH     = -m32 -arch i386
#ARCH     = 

# Tools
CC     = gcc $(ARCH)
#CC    = cc
CXX    = g++ $(ARCH)
#CXX   = aCC +W361
#CXX   = c++
LIB    = ar -cr
RM     = rm -rf
MKDIR  = mkdir -p
RANLIB = ranlib

# Flags
CFLAGS      = -I$(INCDIR)
CXXFLAGS    = -I$(INCDIR)
DEBUGOPT    = -g -D_DEBUG
RELEASEOPT  = -O2 -DNDEBUG
#RELEASEOPT = +O2 -DNDEBUG

objects = AttributeListImp AttributesImpl DefaultHandler DOMAutoReleasePool\
	DOMBuilder DOMImpl DOMWriter EntityResolverImp \
	Event EventException HandlerBase InputSource LocatorImp MutationEvent NamespaceSupport \
	ParserFactory SAX2Parser SAX2ParserBase SAX2ParserNS SAX2ParserNSP SAXDocumentParser \
	SAXDOMParser SAXException SAXImpl SAXParser SAXParserBase \
	XMLDocument XMLDocumentBuilder XMLException XMLNamePool XMLParserBase XMLReaderFactory \
	XMLStringPool XMLUtils XMLWriter XMLWriter2 xmlparse xmlrole xmltok \
	DOMAutoReleasePool 

lib = cslxml

# Targets
OBJDIR_RELEASE_STATIC = $(OBJDIR)/release_static
OBJDIR_DEBUG_STATIC   = $(OBJDIR)/debug_static
LIB_RELEASE_STATIC    = $(LIBDIR)/lib$(lib).a
LIB_DEBUG_STATIC      = $(LIBDIR)/lib$(lib)d.a

all: static_release

dirs: $(OBJDIR_RELEASE_STATIC) $(OBJDIR_DEBUG_STATIC) $(LIBDIR)

$(OBJDIR_RELEASE_STATIC) $(OBJDIR_DEBUG_STATIC) $(LIBDIR):
	$(MKDIR) $@

clean:
	$(RM) $(OBJDIR)
	$(RM) $(LIBDIR)
	(cd samples; make clean)

static_debug:   dirs $(LIB_DEBUG_STATIC)
static_release: dirs $(LIB_RELEASE_STATIC)

# Rules
$(LIB_DEBUG_STATIC): $(foreach o,$(objects),$(OBJDIR_DEBUG_STATIC)/$(o).o)
	$(LIB) $@ $^
	$(RANLIB) $@

$(LIB_RELEASE_STATIC): $(foreach o,$(objects),$(OBJDIR_RELEASE_STATIC)/$(o).o)
	$(LIB) $@ $^
	$(RANLIB) $@
		
$(OBJDIR_DEBUG_STATIC)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(DEBUGOPT) -c $< -o $@

$(OBJDIR_RELEASE_STATIC)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(RELEASEOPT) -c $< -o $@

$(OBJDIR_DEBUG_STATIC)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(DEBUGOPT) -c $< -o $@

$(OBJDIR_RELEASE_STATIC)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(RELEASEOPT) -c $< -o $@
