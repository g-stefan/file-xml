// File XML
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/FileXML/Reader.hpp>
#include <XYO/FileXML/Input.hpp>

namespace XYO::FileXML {

	struct Reader {
			Input input;

			bool isSpace();
			bool ignoreSpace();
			bool readName(String &name);
			bool readAttributes(typename Document::Attributes &attributes);
			bool process(Document &document, Mode mode);
			TPointer<Document> read(Mode mode = Mode::Normal);

			inline bool is(const char char_) {
				return (input.input == char_);
			};

			inline bool is1(const char char_) {
				if (input.input == char_) {
					return input.read();
				};
				return false;
			};

			bool isN(const char *name);
	};

	bool load(const char *fileName, TPointer<Document> &document, Mode mode) {
		Reader Reader;
		File file;
		if (file.openRead(fileName)) {
			Reader.input.setIRead(&file);
			document = Reader.read(mode);
			return document;
		};
		return false;
	};

	bool loadFromString(const char *value, TPointer<Document> &document, Mode mode) {
		Reader Reader;
		MemoryRead memory;
		if (memory.open(value, strlen(value))) {
			Reader.input.setIRead(&memory);
			document = Reader.read(mode);
			return document;
		};
		return false;
	};

	bool Reader::isN(const char *name) {
		int k;
		for (k = 0; name[k] != 0; ++k) {
			if (input.input == name[k]) {
				if (input.read()) {
					continue;
				};
			};
			break;
		};
		if (name[k] == 0) {
			return true;
		};
		for (--k; k >= 0; --k) {
			input.push();
			input.input = name[k];
		};
		return false;
	};

	bool Reader::isSpace() {
		bool isOk = false;
		while (!input.isEof()) {

			if (is1('\x20')) {
				isOk = true;
				continue;
			};

			if (is1('\x09')) {
				isOk = true;
				continue;
			};

			if (is1('\x0D')) {
				isOk = true;
				continue;
			};

			if (is1('\x0A')) {
				isOk = true;
				continue;
			};

			return isOk;
		};
		return false;
	};

	bool Reader::ignoreSpace() {
		while (!input.isEof()) {

			if (is1('\x20')) {
				continue;
			};

			if (is1('\x09')) {
				continue;
			};

			if (is1('\x0D')) {
				continue;
			};

			if (is1('\x0A')) {
				continue;
			};

			return true;
		};
		return false;
	};

	bool Reader::readName(String &name) {
		while (!input.isEof()) {
			if (is1('\x20')) {
				return true;
			};
			if (is1('\x09')) {
				return true;
			};
			if (is1('\x0D')) {
				return true;
			};
			if (is1('\x0A')) {
				return true;
			};
			if (is('=')) {
				return true;
			};
			if (is('/')) {
				return true;
			};
			if (is('>')) {
				return true;
			};

			name += input.input;
			input.read();
		};
		return false;
	};

	bool Reader::readAttributes(typename Document::Attributes &attributes) {
		int index = 0;
		bool isOk;
		while (!input.isEof()) {
			if (!ignoreSpace()) {
				return false;
			};

			if (is('/')) {
				return true;
			};
			if (is('>')) {
				return true;
			};

			attributes[index].newMemory();
			if (!readName(attributes[index]->name)) {
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

			if (is1('"')) {
				isOk = false;
				while (!input.isEof()) {
					if (is1('"')) {
						isOk = true;
						break;
					};
					attributes[index]->value += input.input;
					input.read();
				};
				if (!isOk) {
					return false;
				};

				++index;
				continue;
			};

			if (is1('\'')) {
				isOk = false;
				while (!input.isEof()) {
					if (is1('\'')) {
						isOk = true;
						break;
					};
					attributes[index]->value += input.input;
					input.read();
				};

				if (!isOk) {
					return false;
				};

				++index;
				continue;
			};

			break;
		};
		return false;
	};

	bool Reader::process(Document &document, Mode mode) {
		TPointer<Node> node;
		bool isOk;
		while (!input.isEof()) {

			// BOM
			if (isN("\xEF\xBB\xBF")) {
				node.newMemory();
				node->type = NodeType::Bom;
				node->name = "\xEF\xBB\xBF";
				document.add(node);
				continue;
			};

			// Declaration / Processing instructions
			if (isN("<?")) {
				node.newMemory();
				node->type = NodeType::Declaration;
				isOk = false;
				while (!input.isEof()) {
					if (isN("?>")) {
						document.add(node);
						isOk = true;
						break;
					};
					node->name += input.input;
					input.read();
				};
				if (!isOk) {
					return false;
				};
				continue;
			};

			// Document Type Definition
			if (isN("<!DOCTYPE")) {
				if (isSpace()) {
					node.newMemory();
					node->type = NodeType::DocumentTypeDefinition;
					isOk = false;
					while (!input.isEof()) {
						if (is1('>')) {
							document.add(node);
							isOk = true;
							break;
						};
						node->name += input.input;
						input.read();
					};
					if (!isOk) {
						return false;
					};
					continue;
				};
				return false;
			};

			// Comment
			if (isN("<!--")) {
				node.newMemory();
				node->type = NodeType::Comment;
				isOk = false;
				while (!input.isEof()) {
					if (isN("-->")) {
						document.add(node);
						isOk = true;
						break;
					};
					node->name += input.input;
					input.read();
				};
				if (!isOk) {
					return false;
				};
				continue;
			};

			// CDATA
			if (isN("<![CDATA[")) {
				node.newMemory();
				node->type = NodeType::CDATA;
				isOk = false;
				while (!input.isEof()) {
					if (isN("]]>")) {
						document.add(node);
						isOk = true;
						break;
					};
					node->name += input.input;
					input.read();
				};
				if (!isOk) {
					return false;
				};
				continue;
			};

			// Element
			if (is1('<')) {
				// end tag ...
				if (is('/')) {
					input.push();
					input.input = '<';
					return true;
				};

				if (isSpace()) {
					return false;
				};

				node.newMemory();
				node->type = NodeType::Element;

				if (readName(node->name)) {
					node->attributes.newMemory();
					if (readAttributes(*(node->attributes))) {
						if (isN("/>")) {
							document.add(node);
							continue;
						};

						if (is1('>')) {
							Document branch(document.add(node));
							if (process(branch, mode)) {
								if (isN("</")) {
									if (isN(node->name)) {
										if (!ignoreSpace()) {
											return false;
										};
										if (is1('>')) {
											continue;
										};
									};
								};
							};
						};
					};
				};

				return false;
			};

			// Content
			node.newMemory();
			node->type = NodeType::Content;
			isOk = false;
			while (!input.isEof()) {
				if (is('<')) {
					isOk = true;
					break;
				};
				node->name += input.input;
				input.read();
			};

			if (mode == Mode::Minified) {
				String tmp = String::trimAscii(node->name);
				if (!tmp.isEmpty()) {
					node->name = tmp;
					document.add(node);
				};

				if (isOk) {
					continue;
				};

				break;
			};

			document.add(node);

			if (isOk) {
				continue;
			};

			// End of document
			break;
		};

		return true;
	};

	TPointer<Document> Reader::read(Mode mode) {
		TPointer<Document> retV;

		if (input.read()) {
			retV.newMemory();
			if (process(*retV, mode)) {
				return retV;
			};
		};

		return nullptr;
	};

};
