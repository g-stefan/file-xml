// File XML
// Copyright (c) 2016-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_FILEXML_DEPENDENCY_HPP
#define XYO_FILEXML_DEPENDENCY_HPP

#ifndef XYO_SYSTEM_HPP
#	include <XYO/System.hpp>
#endif

// -- Export

#ifndef XYO_FILEXML_INTERNAL
#	ifdef FILE_XML_INTERNAL
#		define XYO_FILEXML_INTERNAL
#	endif
#endif

#ifdef XYO_FILEXML_INTERNAL
#	define XYO_FILEXML_EXPORT XYO_LIBRARY_EXPORT
#else
#	define XYO_FILEXML_EXPORT XYO_LIBRARY_IMPORT
#endif

// --

namespace XYO::FileXML {
	using namespace XYO::ManagedMemory;
	using namespace XYO::DataStructures;
	using namespace XYO::Encoding;
	using namespace XYO::System;
};

#endif
