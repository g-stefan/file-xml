// File XML
// Copyright (c) 2016-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/FileXML/Writer.hpp>

namespace XYO::FileXML {

	struct Writer {
			TPointer<IWrite> iWrite;
			Mode mode;

			inline bool write(const String &value) {
				return StreamX::write(*iWrite, value);
			};

			bool writeIndentationBegin(int level);
			bool writeIndentationEnd();
			bool indentationIsEmpty(String &value);
			bool writeIndentationMinified(String &value);
			bool indentationIsContentBranch(Document &document);
			bool write(Document &document, int level = 0);
	};

	bool save(const char *fileName, Document *document, Mode mode) {
		Writer writer;
		File file;

		if (document == nullptr) {
			return false;
		};

		if (file.openWrite(fileName)) {
			writer.iWrite = &file;
			writer.mode = mode;
			return writer.write(*document);
		};
		return false;
	};

	bool saveToString(String &output, Document *document, Mode mode) {
		Writer writer;
		StringWrite str;
		str.use(output);
		writer.iWrite = &str;
		writer.mode = mode;
		writer.write(*document);
		return true;
	};

	bool Writer::writeIndentationBegin(int level) {
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
		if ((mode == Mode::Indentation4Spaces) || (mode == Mode::IndentationTab)) {
			if (!write("\x0D\x0A")) {
				return false;
			};
			return true;
		};
		return true;
	};

	bool Writer::indentationIsEmpty(String &value) {
		if ((mode == Mode::Indentation4Spaces) || (mode == Mode::IndentationTab)) {
			return ((StringX::trimAscii(value)).isEmpty());
		};
		return false;
	};

	bool Writer::writeIndentationMinified(String &value) {
		if ((mode == Mode::Indentation4Spaces) || (mode == Mode::IndentationTab) || (mode == Mode::Minified)) {
			String tmp = StringX::trimAscii(value);
			if (!tmp.isEmpty()) {
				if (!write(tmp)) {
					return false;
				};
			};
			return true;
		};
		if (!write(value)) {
			return false;
		};
		return true;
	};

	bool Writer::indentationIsContentBranch(Document &document) {
		if ((mode == Mode::Indentation4Spaces) || (mode == Mode::IndentationTab)) {
			typename Document::Branch::Node *node;
			if (!document.root) {
				return true;
			};
			for (node = document.root->head; node != nullptr; node = node->next) {
				if (!node->value) {
					continue;
				};
				if ((node->value->type != NodeType::Content) && (node->value->type != NodeType::Bom)) {
					return false;
				};
			};
			return true;
		};
		return false;
	};

	bool Writer::write(Document &document, int level) {
		typename Document::Branch::Node *node;
		if (!document.root) {
			return true;
		};
		for (node = document.root->head; node != nullptr; node = node->next) {
			if (!node->value) {
				continue;
			};

			if (node->value->type == NodeType::Content) {
				if (indentationIsEmpty(node->value->name)) {
					continue;
				};
			};

			if (!writeIndentationBegin(level)) {
				return false;
			};

			switch (node->value->type) {
			case NodeType::Bom:
				if (!write(node->value->name)) {
					return false;
				};
				break;
			case NodeType::Declaration:
				if (!write("<?")) {
					return false;
				};
				if (!write(node->value->name)) {
					return false;
				};
				if (!write("?>")) {
					return false;
				};
				break;
			case NodeType::DocumentTypeDefinition:
				if (!write("<!DOCTYPE")) {
					return false;
				};
				if (!write(node->value->name)) {
					return false;
				};
				if (!write(">")) {
					return false;
				};
				break;
			case NodeType::Comment:
				if (!write("<!--")) {
					return false;
				};
				if (!write(node->value->name)) {
					return false;
				};
				if (!write("-->")) {
					return false;
				};
				break;
			case NodeType::CDATA:
				if (!write("<![CDATA[")) {
					return false;
				};
				if (!write(node->value->name)) {
					return false;
				};
				if (!write("]]>")) {
					return false;
				};
				break;
			case NodeType::Content:
				if (!writeIndentationMinified(node->value->name)) {
					return false;
				};
				break;
			case NodeType::Element:
				if (!write("<")) {
					return false;
				};
				if (!write(node->value->name)) {
					return false;
				};
				if (node->value->attributes) {
					if (node->value->attributes->length() > 0) {
						for (int index = 0; index < node->value->attributes->length(); ++index) {
							TPointer<Attribute> &attribute_((*node->value->attributes)[index]);
							if (attribute_) {
								if (!write("\x20")) {
									return false;
								};
								if (!write(attribute_->name)) {
									return false;
								};
								if (!write("=\"")) {
									return false;
								};
								if (!write(attribute_->value)) {
									return false;
								};
								if (!write("\"")) {
									return false;
								};
							};
						};
					};
				};
				if (node->value->branch) {
					if (node->value->branch->isEmpty()) {
						if (!write("\x20")) {
							return false;
						};
						if (!write("/>")) {
							return false;
						};
						break;
					};

					if (!write(">")) {
						return false;
					};

					Document branch(node->value->branch);

					if (indentationIsContentBranch(branch)) {
						if (!write(branch, 0)) {
							return false;
						};
					} else {
						if (!writeIndentationEnd()) {
							return false;
						};

						if (!write(branch, level + 1)) {
							return false;
						};

						if (!writeIndentationBegin(level)) {
							return false;
						};
					};

					if (!write("</")) {
						return false;
					};

					if (!write(node->value->name)) {
						return false;
					};

					if (!write(">")) {
						return false;
					};

					break;
				};

				if (!write("\x20")) {
					return false;
				};
				if (!write("/>")) {
					return false;
				};

				break;
			default:
				break;
			};

			if (node->value->type != NodeType::Content) {
				if (!writeIndentationEnd()) {
					return false;
				};
			};
		};
		return true;
	};

};
