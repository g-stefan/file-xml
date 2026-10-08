// File XML
// Copyright (c) 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/FileXML/Document.hpp>

namespace XYO::FileXML {

	static inline void documentPush(Document &document, Node *node) {
		if (!document.root) {
			document.root.newMemory();
		};
		document.root->pushToTail(node);
	};

	static inline bool documentHasAttributeValue(Node *node, const char *attribute, const char *value) {
		if (!node->attributes) {
			return false;
		};
		size_t length = node->attributes->length();
		for (size_t index = 0; index < length; ++index) {
			TPointer<Attribute> &attribute_(node->attributes->index(index));
			if (attribute_) {
				if (attribute_->name == attribute) {
					if (attribute_->value == value) {
						return true;
					};
				};
			};
		};
		return false;
	};

	static void documentFind(Document::Branch *branch, const char *name, Document &retV) {
		typename Document::Branch::Node *scan;
		for (scan = branch->head; scan != nullptr; scan = scan->next) {
			if (!scan->value) {
				continue;
			};
			if (scan->value->type == NodeType::Element) {
				if (scan->value->name == name) {
					documentPush(retV, scan->value);
				};
				if (scan->value->branch) {
					documentFind(scan->value->branch, name, retV);
				};
			};
		};
	};

	static void documentFindWithAttributeValue(Document::Branch *branch, const char *name, const char *attribute, const char *value, Document &retV) {
		typename Document::Branch::Node *scan;
		for (scan = branch->head; scan != nullptr; scan = scan->next) {
			if (!scan->value) {
				continue;
			};
			if (scan->value->type == NodeType::Element) {
				if (scan->value->name == name) {
					if (documentHasAttributeValue(scan->value, attribute, value)) {
						documentPush(retV, scan->value);
					};
				};
				if (scan->value->branch) {
					documentFindWithAttributeValue(scan->value->branch, name, attribute, value, retV);
				};
			};
		};
	};

	void Document::addDocument(Document &document) {
		if (document) {
			if (!root) {
				root.newMemory();
			};
			// document may share the same list, stop at its current tail
			typename Branch::Node *last = document.root->tail;
			typename Branch::Node *index;
			for (index = document.root->head; index != nullptr; index = index->next) {
				root->pushToTail(index->value);
				if (index == last) {
					break;
				};
			};
		};
	};

	Document Document::get(const char *name) {
		typename Branch::Node *scan;
		Document retV;
		if (!root) {
			return retV;
		};

		for (scan = root->head; scan != nullptr; scan = scan->next) {
			if (!scan->value) {
				continue;
			};
			if (scan->value->type == NodeType::Element) {
				if (scan->value->name == name) {
					documentPush(retV, scan->value);
				};
			};
		};

		return retV;
	};

	Document Document::find(const char *name) {
		Document retV;
		if (root) {
			documentFind(root, name, retV);
		};
		return retV;
	};

	Document Document::findWithAttributeValue(const char *name, const char *attribute, const char *value) {
		Document retV;
		if (root) {
			documentFindWithAttributeValue(root, name, attribute, value, retV);
		};
		return retV;
	};

	size_t Document::length() {

		if (!root) {
			return 0;
		};

		size_t retV;
		typename Branch::Node *scan;
		retV = 0;
		for (scan = root->head; scan != nullptr; scan = scan->next) {
			++retV;
		};
		return retV;
	};

	TPointer<Node> Document::getIndex(size_t index_) {
		typename Branch::Node *scan;
		size_t count_ = 0;
		if (!root) {
			return nullptr;
		};
		for (scan = root->head; scan != nullptr; scan = scan->next, ++count_) {
			if (count_ == index_) {
				return scan->value;
			};
		};
		return nullptr;
	};

	void Document::setIndex(size_t index_, Node *node) {
		typename Branch::Node *scan;
		size_t count_ = 0;
		if (!root) {
			return;
		};
		for (scan = root->head; scan != nullptr; scan = scan->next, ++count_) {
			if (count_ == index_) {
				scan->value = node;
				break;
			};
		};
	};

	void Document::removeIndex(size_t index_) {
		typename Branch::Node *scan;
		size_t count_ = 0;
		if (!root) {
			return;
		};
		for (scan = root->head; scan != nullptr; scan = scan->next, ++count_) {
			if (count_ == index_) {
				root->extractNode(scan);
				Branch::deleteNode(scan);
				break;
			};
		};
	};

};
