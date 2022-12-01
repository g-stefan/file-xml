// File XML
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_FILEXML_COPYRIGHT_HPP
#define XYO_FILEXML_COPYRIGHT_HPP

#ifndef XYO_FILEXML_DEPENDENCY_HPP
#	include <XYO/FileXML/Dependency.hpp>
#endif

namespace XYO::FileXML::Copyright {
	XYO_FILEXML_EXPORT const char *copyright();
	XYO_FILEXML_EXPORT const char *publisher();
	XYO_FILEXML_EXPORT const char *company();
	XYO_FILEXML_EXPORT const char *contact();
	XYO_FILEXML_EXPORT const char *fullCopyright();
};

#endif
