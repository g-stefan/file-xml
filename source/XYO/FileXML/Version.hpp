// File XML
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_FILEXML_VERSION_HPP
#define XYO_FILEXML_VERSION_HPP

#ifndef XYO_FILEXML_DEPENDENCY_HPP
#	include <XYO/FileXML/Dependency.hpp>
#endif

namespace XYO::FileXML::Version {

	XYO_FILEXML_EXPORT const char *version();
	XYO_FILEXML_EXPORT const char *build();
	XYO_FILEXML_EXPORT const char *versionWithBuild();
	XYO_FILEXML_EXPORT const char *datetime();

};

#endif
