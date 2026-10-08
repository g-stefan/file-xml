// File XML
// Copyright (c) 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/FileXML/Reader.hpp>
#include <XYO/FileXML/Input.hpp>
#include <XYO/FileXML/Whitespace.hpp>

namespace XYO::FileXML {

	// Growable text accumulator, reused for every name/value read
	struct ReaderText {
			XYO_PLATFORM_DISALLOW_COPY_ASSIGN_MOVE(ReaderText);

		public:
			char *value;
			size_t length;
			size_t size;

			inline ReaderText() {
				size = 256;
				length = 0;
				value = new char[size];
			};

			inline ~ReaderText() {
				delete[] value;
			};

			inline void empty() {
				length = 0;
			};

			inline void add(char value_) {
				if (length == size) {
					grow();
				};
				value[length] = value_;
				++length;
			};

			inline void grow() {
				size_t newSize = size * 2;
				char *newValue = new char[newSize];
				memcpy(newValue, value, length);
				delete[] value;
				value = newValue;
				size = newSize;
			};

			inline bool endsWith(const char *x, size_t xLength) const {
				if (length < xLength) {
					return false;
				};
				return (memcmp(value + length - xLength, x, xLength) == 0);
			};

			inline String toString(size_t begin, size_t end) const {
				if (begin >= end) {
					return String();
				};
				return String(value + begin, end - begin);
			};

			inline String toString() const {
				return toString(0, length);
			};
	};

	struct Reader {
			// Maximum element nesting, protects the stack against malicious input
			static constexpr size_t maxDepth = 1024;

			enum struct Result {
				Error,
				End,
				EndTag
			};

			Input input;
			ReaderText text;
			Mode mode;

			inline Reader(Mode mode_) {
				mode = mode_;
			};

			inline bool is(const char char_) {
				return (!input.isEof()) && (input.input == char_);
			};

			inline bool is1(const char char_) {
				if (is(char_)) {
					input.read();
					return true;
				};
				return false;
			};

			bool isN(const char *name);
			bool isSpace();
			bool ignoreSpace();
			bool readName(String &name);
			bool readUntil(const char *terminator);
			bool readDocumentTypeDefinition();
			bool readAttributes(typename Document::Attributes &attributes);
			void minify(Document &document);
			Result process(Document &document, size_t depth);
			TPointer<Document> read();
	};

	static inline void readerAddNode(Document &document, Node *node) {
		if (!document.root) {
			document.root.newMemory();
		};
		document.root->pushToTail(node);
	};

	bool load(const char *fileName, TPointer<Document> &document, Mode mode) {
		File file;
		Reader reader(mode);
		document = nullptr;
		if (fileName == nullptr) {
			return false;
		};
		if (file.openRead(fileName)) {
			reader.input.setIRead(&file);
			document = reader.read();
			reader.input.setIRead(nullptr);
			return document;
		};
		return false;
	};

	bool loadFromString(const char *value, TPointer<Document> &document, Mode mode) {
		Reader reader(mode);
		document = nullptr;
		if (value == nullptr) {
			return false;
		};
		reader.input.setMemory(value, strlen(value));
		document = reader.read();
		return document;
	};

	bool Reader::isN(const char *name) {
		size_t k;
		for (k = 0; name[k] != 0; ++k) {
			if (!is(name[k])) {
				break;
			};
			input.read();
		};
		if (name[k] == 0) {
			return true;
		};
		// restore consumed characters
		while (k > 0) {
			--k;
			input.pushBack(name[k]);
		};
		return false;
	};

	bool Reader::isSpace() {
		bool isOk = false;
		while (!input.isEof()) {
			if (isSpaceASCII(input.input)) {
				isOk = true;
				input.read();
				continue;
			};
			return isOk;
		};
		return false;
	};

	bool Reader::ignoreSpace() {
		while (!input.isEof()) {
			if (isSpaceASCII(input.input)) {
				input.read();
				continue;
			};
			return true;
		};
		return false;
	};

	bool Reader::readName(String &name) {
		text.empty();
		while (!input.isEof()) {
			if (isSpaceASCII(input.input)) {
				input.read();
				break;
			};
			if ((input.input == '=') || (input.input == '/') || (input.input == '>')) {
				break;
			};
			text.add(input.input);
			input.read();
		};
		if (input.isEof() || (text.length == 0)) {
			return false;
		};
		name = text.toString();
		return true;
	};

	bool Reader::readUntil(const char *terminator) {
		size_t terminatorLength = strlen(terminator);
		text.empty();
		while (!input.isEof()) {
			text.add(input.input);
			input.read();
			if (text.endsWith(terminator, terminatorLength)) {
				text.length -= terminatorLength;
				return true;
			};
		};
		return false;
	};

	bool Reader::readDocumentTypeDefinition() {
		char quote = 0;
		size_t depth = 0;
		bool comment = false;
		text.empty();
		while (!input.isEof()) {
			char char_ = input.input;
			if (comment) {
				text.add(char_);
				input.read();
				if (text.endsWith("-->", 3)) {
					comment = false;
				};
				continue;
			};
			if (quote) {
				if (char_ == quote) {
					quote = 0;
				};
			} else if ((char_ == '"') || (char_ == '\'')) {
				quote = char_;
			} else if (char_ == '[') {
				++depth;
			} else if (char_ == ']') {
				if (depth > 0) {
					--depth;
				};
			} else if ((char_ == '>') && (depth == 0)) {
				input.read();
				return true;
			};
			text.add(char_);
			input.read();
			if ((depth > 0) && (!quote) && text.endsWith("<!--", 4)) {
				comment = true;
			};
		};
		return false;
	};

	bool Reader::readAttributes(typename Document::Attributes &attributes) {
		size_t index = 0;
		char quote;
		while (ignoreSpace()) {
			if (is('/') || is('>')) {
				return true;
			};

			TPointer<Attribute> attribute;
			attribute.newMemory();
			if (!readName(attribute->name)) {
				return false;
			};

			if (!ignoreSpace()) {
				return false;
			};

			if (!is1('=')) {
				return false;
			};

			if (!ignoreSpace()) {
				return false;
			};

			if (!(is('"') || is('\''))) {
				return false;
			};

			quote = input.input;
			input.read();
			text.empty();
			for (;;) {
				if (input.isEof()) {
					return false;
				};
				if (input.input == quote) {
					input.read();
					break;
				};
				text.add(input.input);
				input.read();
			};
			attribute->value = text.toString();
			attributes.set(index, std::move(attribute));
			++index;
		};
		return false;
	};

	void Reader::minify(Document &document) {
		typename Document::Branch::Node *scan;
		typename Document::Branch::Node *next;
		MinifiedText minified;
		bool mixed;

		if (!document.root) {
			return;
		};

		mixed = isMixedContent(document.root);
		for (scan = document.root->head; scan != nullptr; scan = next) {
			next = scan->next;
			if (scan->value->type != NodeType::Content) {
				continue;
			};
			minifyText(scan->value->name, mixed, scan == document.root->head, next == nullptr, minified);
			if (minified.isEmpty()) {
				document.root->extractNode(scan);
				Document::Branch::deleteNode(scan);
				continue;
			};
			if (minified.spaceBefore || minified.spaceAfter) {
				text.empty();
				if (minified.spaceBefore) {
					text.add('\x20');
				};
				for (size_t k = minified.begin; k < minified.end; ++k) {
					text.add(scan->value->name.value()[k]);
				};
				if (minified.spaceAfter) {
					text.add('\x20');
				};
				scan->value->name = text.toString();
				continue;
			};
			if ((minified.begin > 0) || (minified.end < scan->value->name.length())) {
				scan->value->name = String(scan->value->name.value() + minified.begin, minified.end - minified.begin);
			};
		};
	};

	Reader::Result Reader::process(Document &document, size_t depth) {
		TPointer<Node> node;
		while (!input.isEof()) {

			// Content
			if (input.input != '<') {
				text.empty();
				do {
					text.add(input.input);
					input.read();
				} while (!input.isEof() && (input.input != '<'));
				node.newMemory();
				node->type = NodeType::Content;
				node->name = text.toString();
				readerAddNode(document, node);
				continue;
			};

			if (!input.read()) {
				return Result::Error;
			};

			// End tag
			if (is1('/')) {
				return Result::EndTag;
			};

			// Declaration / Processing instructions
			if (is1('?')) {
				if (!readUntil("?>")) {
					return Result::Error;
				};
				node.newMemory();
				node->type = NodeType::Declaration;
				node->name = text.toString();
				readerAddNode(document, node);
				continue;
			};

			if (is1('!')) {
				// Comment
				if (is1('-')) {
					if (!is1('-')) {
						return Result::Error;
					};
					if (!readUntil("-->")) {
						return Result::Error;
					};
					node.newMemory();
					node->type = NodeType::Comment;
					node->name = text.toString();
					readerAddNode(document, node);
					continue;
				};

				// CDATA
				if (isN("[CDATA[")) {
					if (!readUntil("]]>")) {
						return Result::Error;
					};
					node.newMemory();
					node->type = NodeType::CDATA;
					node->name = text.toString();
					readerAddNode(document, node);
					continue;
				};

				// Document Type Definition
				if (isN("DOCTYPE")) {
					if (!isSpace()) {
						return Result::Error;
					};
					if (!readDocumentTypeDefinition()) {
						return Result::Error;
					};
					node.newMemory();
					node->type = NodeType::DocumentTypeDefinition;
					node->name = text.toString();
					readerAddNode(document, node);
					continue;
				};

				return Result::Error;
			};

			// Element
			if (depth >= maxDepth) {
				return Result::Error;
			};

			node.newMemory();
			node->type = NodeType::Element;
			if (!readName(node->name)) {
				return Result::Error;
			};
			node->attributes.newMemory();
			if (!readAttributes(*(node->attributes))) {
				return Result::Error;
			};
			node->branch.newMemory();
			readerAddNode(document, node);

			if (is1('/')) {
				if (!is1('>')) {
					return Result::Error;
				};
				continue;
			};

			// readAttributes stops only on '/' or '>'
			input.read();

			{
				Document branch(node->branch);
				if (process(branch, depth + 1) != Result::EndTag) {
					return Result::Error;
				};
				if (mode == Mode::Minified) {
					minify(branch);
				};
			};

			// End tag name must match
			const char *name = node->name.value();
			size_t nameLength = node->name.length();
			for (size_t k = 0; k < nameLength; ++k) {
				if (!is1(name[k])) {
					return Result::Error;
				};
			};
			if (!ignoreSpace()) {
				return Result::Error;
			};
			if (!is1('>')) {
				return Result::Error;
			};
		};

		return Result::End;
	};

	TPointer<Document> Reader::read() {
		TPointer<Document> retV;

		if (!input.read()) {
			return nullptr;
		};

		retV.newMemory();

		// BOM
		if (isN("\xEF\xBB\xBF")) {
			TPointer<Node> node;
			node.newMemory();
			node->type = NodeType::Bom;
			node->name = "\xEF\xBB\xBF";
			readerAddNode(*retV, node);
		};

		// A stray end tag at top level is an error
		if (process(*retV, 0) != Result::End) {
			return nullptr;
		};

		if (mode == Mode::Minified) {
			minify(*retV);
		};

		return retV;
	};

};
