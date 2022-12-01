// File XML
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_FILEXML_READER_HPP
#define XYO_FILEXML_READER_HPP

#ifndef XYO_FILEXML_DOCUMENT_HPP
#	include <XYO/FileXML/Document.hpp>
#endif

#ifndef XYO_FILEXML_MODE_HPP
#	include <XYO/FileXML/Mode.hpp>
#endif

namespace XYO::FileXML {

	XYO_FILEXML_EXPORT bool load(const char *fileName, TPointer<Document> &document, Mode mode = Mode::Normal);
	XYO_FILEXML_EXPORT bool loadFromString(const char *value, TPointer<Document> &document, Mode mode = Mode::Normal);

};

#endif
