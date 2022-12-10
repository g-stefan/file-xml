// File XML
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/FileXML/Copyright.hpp>
#include <XYO/FileXML/Copyright.rh>

namespace XYO::FileXML::Copyright {

	static const char *copyright_ = XYO_FILEXML_COPYRIGHT;
	static const char *publisher_ = XYO_FILEXML_PUBLISHER;
	static const char *company_ = XYO_FILEXML_COMPANY;
	static const char *contact_ = XYO_FILEXML_CONTACT;

	const char *copyright() {
		return copyright_;
	};

	const char *publisher() {
		return publisher_;
	};

	const char *company() {
		return company_;
	};

	const char *contact() {
		return contact_;
	};

};
