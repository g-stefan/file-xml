// File XML
// Copyright (c) 2016-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_FILEXML_MODE_HPP
#define XYO_FILEXML_MODE_HPP

#ifndef XYO_FILEXML_DEPENDENCY_HPP
#	include <XYO/FileXML/Dependency.hpp>
#endif

namespace XYO::FileXML {

	enum struct Mode {
		Normal = 0,
		Minified = 1,
		Indentation4Spaces = 2,
		IndentationTab = 3
	};

};

#endif
