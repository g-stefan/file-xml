// File XML
// Copyright (c) 2016-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/FileXML/Copyright.hpp>
#include <XYO/FileXML/Copyright.rh>

namespace XYO::FileXML::Copyright {

	static const char *copyright_ = XYO_FILEXML_COPYRIGHT;
	static const char *publisher_ = XYO_FILEXML_PUBLISHER;
	static const char *company_ = XYO_FILEXML_COMPANY;
	static const char *contact_ = XYO_FILEXML_CONTACT;

	std::string copyright() {
		return copyright_;
	};

	std::string publisher() {
		return publisher_;
	};

	std::string company() {
		return company_;
	};

	std::string contact() {
		return contact_;
	};

};
