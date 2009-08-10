//
// XMLStd.h
//
// $Id: //depot/XML/Main/include/XMLStd.h#9 $
//
// General definitions for XML.
//
// The contents of this file are subject to the 
// CenterPoint Public License Version 1.0 (the "License"); 
// you may not use this file except in compliance with the 
// License. You may obtain a copy of the License at 
// http://www.cpointc.com/CPL
//
// Software distributed under the License is distributed on 
// an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either
// express or implied. See the License for the specific 
// language governing rights and limitations under the License.
//
// The Original Code is CenterPoint/XML.
//
// The Initial Developer of the Original Code is CenterPoint - 
// Connective Software Engineering GmbH. Portions created by
// CenterPoint - Connective Software Engineering GmbH. are 
// Copyright (c) 1998-2002 CenterPoint - Connective Software
// Engineering GmbH. All Rights Reserved.
//
// Contributor(s):
//


#ifndef XMLStd_H_INCLUDED
#define XMLStd_H_INCLUDED


#if !defined(CSL_XML_CENTERPOINT) && !defined(CSL_XML_STANDALONE)
#define CSL_XML_STANDALONE
#endif


#ifndef CSL_XML_STANDALONE
#ifndef CSApiStd_H_INCLUDED
#include "CSApiStd.h"
#endif
#endif


#ifdef CSL_XML_STANDALONE
#include <stdlib.h>
#include <assert.h>
#define csl_assert(x) assert(x)
#define csl_precondition(X) assert(X)
#define csl_check_ptr(X) assert(X)
#endif


#ifdef _MSC_VER
#pragma warning(disable:4786)  /* identifier truncation warning */
#pragma warning(disable:4251)  /* ...needs to have dll-interface warning */
#pragma warning(disable:4503)  /* decorated name length exceeded */
#endif // _MSC_VER


#ifdef WIN32
#ifdef CSL_DLL
#define XML_DLL
#endif
#ifdef XML_DLL
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the XML_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// XML_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef XML_EXPORTS
#define XML_API __declspec(dllexport)
#else
#define XML_API __declspec(dllimport)
#endif
#endif // XML_DLL
#endif // WIN32


#ifndef XML_API
#define XML_API
#endif

#define XML_BEGIN namespace CSL { namespace XML {
#define XML_END   } }


#ifndef __DECCXX_V5
namespace std { } // nothing is included yet, so make the std namespace known
using namespace std;
#endif


#ifndef CSL_XML_STANDALONE
CSL_DEFINE_VERSION_INFO(XML);
#endif


#endif // XMLStd_H_INCLUDED
