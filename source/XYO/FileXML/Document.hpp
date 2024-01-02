// File XML
// Copyright (c) 2016-2024 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2024 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_FILEXML_DOCUMENT_HPP
#define XYO_FILEXML_DOCUMENT_HPP

#ifndef XYO_FILEXML_Node_HPP
#	include <XYO/FileXML/Node.hpp>
#endif

namespace XYO::FileXML {

	struct Document : Object {

			typedef typename Node::Branch Branch;
			typedef typename Node::Attributes Attributes;

			static inline Node *newNode() {
				return TMemory<Node>::newMemory();
			};

			static inline void deleteNode(Node *this_) {
				return TMemory<Node>::deleteMemory(this_);
			};

			static inline void initMemory() {
				TMemory<Node>::initMemory();
			};

			TPointerX<Branch> root;

			inline Document() {
				root.pointerLink(this);
			};

			inline ~Document(){};

			inline Document(const Branch *root_) {
				root.pointerLink(this);
				root = root_;
			};

			inline Document(const TPointerX<Branch> &root_) {
				root.pointerLink(this);
				root = root_;
			};

			inline Document(const Document &value) {
				root.pointerLink(this);
				root = (const_cast<Document &>(value)).root;
			};

			inline Document(Document &&value) {
				root.pointerLink(this);
				root = std::move(value.root);
			};

			inline Document &operator=(const Branch *root_) {
				root = root_;
				return *this;
			};

			inline Document &operator=(const Document &value) {
				root = (const_cast<Document &>(value)).root;
				return *this;
			};

			inline Document &operator=(Document &&value) {
				root = std::move(value.root);
				return *this;
			};

			inline void empty() {
				root.deleteMemory();
			};

			inline void activeDestructor() {
				empty();
			};

			inline operator bool() const {
				return root;
			};

			inline Document add(Node *node) {
				if (!root) {
					root.newMemory();
				};
				if (!(node->branch)) {
					node->branch.newMemory();
				};
				root->pushToTail(node);
				return node->branch;
			};

			XYO_FILEXML_EXPORT void addDocument(Document &document);
			XYO_FILEXML_EXPORT Document get(const char *name);
			XYO_FILEXML_EXPORT Document find(const char *name);
			XYO_FILEXML_EXPORT Document findWithAttributeValue(const char *name, const char *attribute, const char *value);
			XYO_FILEXML_EXPORT size_t length();
			XYO_FILEXML_EXPORT TPointer<Node> getIndex(size_t index_);
			XYO_FILEXML_EXPORT void setIndex(size_t index_, Node *node);
			XYO_FILEXML_EXPORT void removeIndex(size_t index_);
	};

};

namespace XYO::ManagedMemory {
	template <>
	struct TMemory<XYO::FileXML::Document> : TMemoryPoolActive<XYO::FileXML::Document> {};
};

#endif
