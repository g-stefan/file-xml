// File XML
// Copyright (c) 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/FileXML/Writer.hpp>
#include <XYO/FileXML/Whitespace.hpp>

namespace XYO::FileXML {

	struct Writer {
			TPointer<IWrite> iWrite;
			Mode mode;

			inline bool write(const char *value, size_t length) {
				if (length == 0) {
					return true;
				};
				return (iWrite->write(value, length) == length);
			};

			inline bool write(const String &value) {
				return write(value.value(), value.length());
			};

			template <size_t N>
			inline bool write(const char (&value)[N]) {
				return write(value, N - 1);
			};

			inline bool isIndentation() const {
				return (mode == Mode::Indentation4Spaces) || (mode == Mode::IndentationTab);
			};

			bool writeIndentationBegin(size_t level);
			bool writeIndentationEnd();
			bool writeContent(const String &value, bool mixed, bool first, bool last);
			bool writeAttributeValue(const String &value);
			bool write(Document &document, size_t level, bool isInline);
	};

	bool save(const char *fileName, Document *document, Mode mode) {
		File file;
		Writer writer;

		if ((fileName == nullptr) || (document == nullptr)) {
			return false;
		};

		if (file.openWrite(fileName)) {
			writer.iWrite = &file;
			writer.mode = mode;
			bool retV = writer.write(*document, 0, false);
			writer.iWrite = nullptr;
			return retV;
		};
		return false;
	};

	bool saveToString(String &output, Document *document, Mode mode) {
		StringWrite str;
		Writer writer;

		if (document == nullptr) {
			return false;
		};

		// output is replaced only on success, on error it is left unchanged
		String retV;
		str.use(retV);
		writer.iWrite = &str;
		writer.mode = mode;
		bool isOk = writer.write(*document, 0, false);
		writer.iWrite = nullptr;
		if (!isOk) {
			return false;
		};
		output = retV;
		return true;
	};

	bool Writer::writeIndentationBegin(size_t level) {
		if (mode == Mode::Indentation4Spaces) {
			for (; level > 0; --level) {
				if (!write("\x20\x20\x20\x20")) {
					return false;
				};
			};
			return true;
		};
		if (mode == Mode::IndentationTab) {
			for (; level > 0; --level) {
				if (!write("\x09")) {
					return false;
				};
			};
			return true;
		};
		return true;
	};

	bool Writer::writeIndentationEnd() {
		if (isIndentation()) {
			return write("\x0D\x0A");
		};
		return true;
	};

	bool Writer::writeContent(const String &value, bool mixed, bool first, bool last) {
		if (mode == Mode::Normal) {
			return write(value);
		};
		MinifiedText minified;
		minifyText(value, mixed, first, last, minified);
		if (minified.spaceBefore) {
			if (!write("\x20")) {
				return false;
			};
		};
		if (!write(value.value() + minified.begin, minified.end - minified.begin)) {
			return false;
		};
		if (minified.spaceAfter) {
			if (!write("\x20")) {
				return false;
			};
		};
		return true;
	};

	// Quote with " or ', whichever is not in the value, escape " if both are used
	bool Writer::writeAttributeValue(const String &value) {
		const char *scan = value.value();
		size_t length = value.length();
		if (memchr(scan, '"', length) == nullptr) {
			return write("=\"") && write(value) && write("\"");
		};
		if (memchr(scan, '\'', length) == nullptr) {
			return write("='") && write(value) && write("'");
		};
		if (!write("=\"")) {
			return false;
		};
		size_t begin = 0;
		for (size_t k = 0; k < length; ++k) {
			if (scan[k] == '"') {
				if (!write(scan + begin, k - begin)) {
					return false;
				};
				if (!write("&quot;")) {
					return false;
				};
				begin = k + 1;
			};
		};
		if (!write(scan + begin, length - begin)) {
			return false;
		};
		return write("\"");
	};

	bool Writer::write(Document &document, size_t level, bool isInline) {
		typename Document::Branch::Node *node;
		if (!document.root) {
			return true;
		};

		// Mixed content is written as is, without indentation
		bool mixed = (mode != Mode::Normal) && isMixedContent(document.root);
		bool indent = isIndentation() && (!isInline) && (!mixed);

		for (node = document.root->head; node != nullptr; node = node->next) {
			if (!node->value) {
				continue;
			};

			Node *value = node->value;

			switch (value->type) {
			case NodeType::None:
				continue;
			case NodeType::Content:
				if (!writeContent(value->name, mixed, node == document.root->head, node->next == nullptr)) {
					return false;
				};
				continue;
			case NodeType::Bom:
				// Nothing is allowed before the XML declaration, not even a new line
				if (!write(value->name)) {
					return false;
				};
				continue;
			default:
				break;
			};

			if (indent) {
				if (!writeIndentationBegin(level)) {
					return false;
				};
			};

			switch (value->type) {
			case NodeType::Declaration:
				if (!(write("<?") && write(value->name) && write("?>"))) {
					return false;
				};
				break;
			case NodeType::DocumentTypeDefinition:
				if (!(write("<!DOCTYPE\x20") && write(value->name) && write(">"))) {
					return false;
				};
				break;
			case NodeType::Comment:
				if (!(write("<!--") && write(value->name) && write("-->"))) {
					return false;
				};
				break;
			case NodeType::CDATA:
				if (!(write("<![CDATA[") && write(value->name) && write("]]>"))) {
					return false;
				};
				break;
			case NodeType::Element:
				if (!write("<")) {
					return false;
				};
				if (!write(value->name)) {
					return false;
				};
				if (value->attributes) {
					size_t length = value->attributes->length();
					for (size_t index = 0; index < length; ++index) {
						TPointer<Attribute> &attribute_(value->attributes->index(index));
						if (attribute_) {
							if (!write("\x20")) {
								return false;
							};
							if (!write(attribute_->name)) {
								return false;
							};
							if (!writeAttributeValue(attribute_->value)) {
								return false;
							};
						};
					};
				};

				if ((!value->branch) || value->branch->isEmpty()) {
					if (!write("\x20/>")) {
						return false;
					};
					break;
				};

				if (!write(">")) {
					return false;
				};

				{
					Document branch(value->branch);

					if ((!indent) || isContentBranch(branch.root) || isMixedContent(branch.root)) {
						if (!write(branch, 0, true)) {
							return false;
						};
					} else {
						if (!writeIndentationEnd()) {
							return false;
						};

						if (!write(branch, level + 1, false)) {
							return false;
						};

						if (!writeIndentationBegin(level)) {
							return false;
						};
					};
				};

				if (!(write("</") && write(value->name) && write(">"))) {
					return false;
				};

				break;
			default:
				break;
			};

			if (indent) {
				if (!writeIndentationEnd()) {
					return false;
				};
			};
		};
		return true;
	};

};
