// File XML
// Copyright (c) 2016-2024 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2024 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_FILEXML_NODE_HPP
#define XYO_FILEXML_NODE_HPP

#ifndef XYO_FILEXML_ATTRIBUTE_HPP
#	include <XYO/FileXML/Attribute.hpp>
#endif

namespace XYO::FileXML {

	enum struct NodeType {
		None = 0,
		Element = 1,
		Content = 2,
		CDATA = 3,
		Comment = 4,
		Declaration = 5,
		Bom = 6,
		DocumentTypeDefinition = 7
	};

	class Node : public Object {
			XYO_PLATFORM_DISALLOW_COPY_ASSIGN_MOVE(Node);

		public:
			typedef TDynamicArray<TPointer<Attribute>, 3> Attributes;
			typedef TDoubleEndedQueue<TPointerX<Node>> Branch;

			NodeType type;
			String name;
			TPointer<Attributes> attributes;
			TPointerX<Branch> branch;

			inline Node() {
				branch.pointerLink(this);
				type = NodeType::None;
			};

			inline ~Node(){};

			inline void activeDestructor() {
				attributes.activeDestructor();
				name.activeDestructor();
				type = NodeType::None;
			};

			static inline void initMemory() {
				String::initMemory();
				TPointer<TDynamicArray<Attribute>>::initMemory();
			};
	};

};

namespace XYO::ManagedMemory {
	template <>
	struct TMemory<XYO::FileXML::Node> : TMemoryPoolActive<XYO::FileXML::Node> {};
};

#endif
