// File XML
// Copyright (c) 2016-2024 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2024 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_FILEXML_ATTRIBUTE_HPP
#define XYO_FILEXML_ATTRIBUTE_HPP

#ifndef XYO_FILEXML_DEPENDENCY_HPP
#	include <XYO/FileXML/Dependency.hpp>
#endif

namespace XYO::FileXML {

	class Attribute : public Object {
			XYO_PLATFORM_DISALLOW_COPY_ASSIGN_MOVE(Attribute);

		public:
			String name;
			String value;

			inline Attribute(){};

			inline ~Attribute(){};

			static inline void initMemory() {
				String::initMemory();
			};

			inline void activeDestructor() {
				name.activeDestructor();
				value.activeDestructor();
			};
	};

};

namespace XYO::ManagedMemory {
	template <>
	struct TMemory<XYO::FileXML::Attribute> : TMemoryPoolActive<XYO::FileXML::Attribute> {};
};

#endif
