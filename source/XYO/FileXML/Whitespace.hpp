// File XML
// Copyright (c) 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_FILEXML_WHITESPACE_HPP
#define XYO_FILEXML_WHITESPACE_HPP

#ifndef XYO_FILEXML_NODE_HPP
#	include <XYO/FileXML/Node.hpp>
#endif

namespace XYO::FileXML {

	// Same set of characters as String::trimASCII
	inline bool isSpaceASCII(char value) {
		return (value == '\x20') || (value == '\x09') || (value == '\x0D') || (value == '\x0A');
	};

	// [begin, end) is the value without leading and trailing space
	inline void trimRangeASCII(const char *value, size_t length, size_t &begin, size_t &end) {
		begin = 0;
		end = length;
		while ((begin < end) && isSpaceASCII(value[begin])) {
			++begin;
		};
		while ((end > begin) && isSpaceASCII(value[end - 1])) {
			--end;
		};
	};

	inline bool isContentNotEmpty(const String &value) {
		size_t begin;
		size_t end;
		trimRangeASCII(value.value(), value.length(), begin, end);
		return (begin < end);
	};

	// Branch with text (not only space) and other nodes, like <p>Hello <b>World</b></p>
	inline bool isMixedContent(const Node::Branch *branch) {
		bool hasText = false;
		bool hasOther = false;
		typename Node::Branch::Node *scan;
		if (!branch) {
			return false;
		};
		for (scan = branch->head; scan != nullptr; scan = scan->next) {
			if (!scan->value) {
				continue;
			};
			if (scan->value->type == NodeType::Content) {
				if (!hasText) {
					hasText = isContentNotEmpty(scan->value->name);
				};
			} else if (scan->value->type != NodeType::Bom) {
				hasOther = true;
			};
			if (hasText && hasOther) {
				return true;
			};
		};
		return false;
	};

	// Branch with only text nodes
	inline bool isContentBranch(const Node::Branch *branch) {
		typename Node::Branch::Node *scan;
		if (!branch) {
			return true;
		};
		for (scan = branch->head; scan != nullptr; scan = scan->next) {
			if (!scan->value) {
				continue;
			};
			if ((scan->value->type != NodeType::Content) && (scan->value->type != NodeType::Bom)) {
				return false;
			};
		};
		return true;
	};

	// Minified form of a text node: value[begin, end) with optional single space before and after.
	// Text is trimmed, but in mixed content a space between text and elements is kept,
	// so <p>Hello <b>World</b></p> is not changed to <p>Hello<b>World</b></p>
	struct MinifiedText {
			size_t begin;
			size_t end;
			bool spaceBefore;
			bool spaceAfter;

			inline bool isEmpty() const {
				return (begin == end) && (!spaceBefore) && (!spaceAfter);
			};
	};

	inline void minifyText(const String &value, bool mixed, bool first, bool last, MinifiedText &out) {
		size_t length = value.length();
		trimRangeASCII(value.value(), length, out.begin, out.end);
		out.spaceBefore = false;
		out.spaceAfter = false;
		if (!mixed) {
			return;
		};
		if (out.begin == out.end) {
			out.spaceBefore = (length > 0) && (!first) && (!last);
			return;
		};
		out.spaceBefore = (out.begin > 0) && (!first);
		out.spaceAfter = (out.end < length) && (!last);
	};

};

#endif
