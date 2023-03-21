// File XML
// Copyright (c) 2016-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_FILEXML_COPYRIGHT_HPP
#define XYO_FILEXML_COPYRIGHT_HPP

#ifndef XYO_FILEXML_DEPENDENCY_HPP
#	include <XYO/FileXML/Dependency.hpp>
#endif

namespace XYO::FileXML::Copyright {
	XYO_FILEXML_EXPORT std::string copyright();
	XYO_FILEXML_EXPORT std::string publisher();
	XYO_FILEXML_EXPORT std::string company();
	XYO_FILEXML_EXPORT std::string contact();
};

#endif
