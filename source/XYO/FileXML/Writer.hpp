// File XML
// Copyright (c) 2016-2024 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2024 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_FILEXML_WRITER_HPP
#define XYO_FILEXML_WRITER_HPP

#ifndef XYO_FILEXML_DOCUMENT_HPP
#	include <XYO/FileXML/Document.hpp>
#endif

#ifndef XYO_FILEXML_MODE_HPP
#	include <XYO/FileXML/Mode.hpp>
#endif

namespace XYO::FileXML {

	XYO_FILEXML_EXPORT bool save(const char *fileName, Document *document, Mode mode = Mode::Normal);
	XYO_FILEXML_EXPORT bool saveToString(String &output, Document *document, Mode mode = Mode::Normal);

};

#endif
