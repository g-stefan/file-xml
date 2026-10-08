// Created by Grigore Stefan <g_stefan@yahoo.com>
// Public domain (Unlicense) <http://unlicense.org>
// SPDX-FileCopyrightText: 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: Unlicense

#include <XYO/FileXML.hpp>

using namespace XYO::FileXML;

static int errorCount = 0;

static void check(bool condition, const char *message) {
	if (!condition) {
		printf("* Failed: %s\n", message);
		++errorCount;
	};
};

static String toString(Document *document, Mode mode) {
	String retV;
	if (!saveToString(retV, document, mode)) {
		printf("* Failed: saveToString\n");
		++errorCount;
	};
	return retV;
};

static void checkOutput(const char *label, const char *xml, Mode readMode, Mode writeMode, const char *expected) {
	TPointer<Document> document;
	if (!loadFromString(xml, document, readMode)) {
		printf("* Failed: %s: load\n", label);
		++errorCount;
		return;
	};
	String output = toString(document, writeMode);
	if (!(output == expected)) {
		printf("* Failed: %s\n  expected [%s]\n  output   [%s]\n", label, expected, output.value());
		++errorCount;
		return;
	};
	// output must load again
	TPointer<Document> reload;
	if (!loadFromString(output, reload)) {
		printf("* Failed: %s: reload of output\n", label);
		++errorCount;
	};
};

static void checkSame(const char *label, const char *xml) {
	checkOutput(label, xml, Mode::Normal, Mode::Normal, xml);
};

static void checkLoadFail(const char *label, const char *xml) {
	TPointer<Document> document;
	if (loadFromString(xml, document)) {
		printf("* Failed: %s: load must fail\n", label);
		++errorCount;
	};
	check(!document, "document must be empty on error");
};

static void testEndOfInput() {
	checkSame("self closing at end", "<a />");
	checkSame("attribute at end", "<a x=\"1\" />");
	checkSame("comment at end", "<a /><!-- c -->");
	checkSame("declaration at end", "<a /><?pi x?>");
	checkSame("cdata at end", "<![CDATA[x]]>");
	checkSame("end tag at end", "<a>x</a>");
	checkSame("text only", "text");
	checkSame("bom only", "\xEF\xBB\xBF");
	checkSame("bom + element", "\xEF\xBB\xBF<a />");
	checkSame("not a bom", "\xEF\xBB<a />");
	checkSame("comment with dashes", "<!-- a - b -- c ---->");
	checkSame("cdata with brackets", "<![CDATA[a]b]]c]]]>");
};

static void testInvalid() {
	checkLoadFail("empty", "");
	checkLoadFail("'<!' at end", "abc<!");
	checkLoadFail("'<' at end", "abc<");
	checkLoadFail("unclosed element", "<a>");
	checkLoadFail("unclosed element with text", "<a>x");
	checkLoadFail("unclosed start tag", "<a");
	checkLoadFail("unclosed start tag with attribute", "<a x=\"1\"");
	checkLoadFail("unclosed self closing", "<a/");
	checkLoadFail("mismatched end tag", "<a></b>");
	checkLoadFail("end tag longer", "<a></ab>");
	checkLoadFail("end tag shorter", "<ab></a>");
	checkLoadFail("unclosed end tag", "<a></a");
	checkLoadFail("empty element name", "<></>");
	checkLoadFail("space before element name", "< a/>");
	checkLoadFail("stray end tag", "</a>");
	checkLoadFail("stray end tag with trailing data", "<a></a></b><c/>");
	checkLoadFail("unknown markup", "<!x>");
	checkLoadFail("bad comment", "<!-x-->");
	checkLoadFail("unclosed comment", "<!-- x");
	checkLoadFail("unclosed cdata", "<![CDATA[x]]");
	checkLoadFail("unclosed declaration", "<?xml");
	checkLoadFail("doctype without space", "<!DOCTYPEx>");
	checkLoadFail("unclosed doctype", "<!DOCTYPE x [ <!ENTITY e \"v\"> ");
	checkLoadFail("attribute without value", "<a x/>");
	checkLoadFail("attribute without quotes", "<a x=1/>");
	checkLoadFail("attribute unclosed quote", "<a x=\"1/>");
	checkLoadFail("attribute without name", "<a =\"1\"/>");

	TPointer<Document> document;
	check(!loadFromString(nullptr, document), "loadFromString(nullptr)");
	check(!load(nullptr, document), "load(nullptr)");
	check(!load("test.03.does-not-exist.xml", document), "load missing file");
};

static void testEmptyValues() {
	checkSame("empty attribute value", "<a x=\"\" />");
	checkSame("empty comment", "<!----><a />");
	checkSame("empty cdata", "<![CDATA[]]>");
	checkSame("empty declaration", "<??>");
	checkOutput("empty attribute, file mode", "<a x=\"\"><!----></a>", Mode::Normal, Mode::Indentation4Spaces, "<a x=\"\">\r\n    <!---->\r\n</a>\r\n");

	TPointer<Document> document;
	String output;
	check(!saveToString(output, nullptr), "saveToString(nullptr)");
	check(!save("test.03.null.xml", nullptr), "save(nullptr)");

	// output is replaced on success, left unchanged on error
	check(loadFromString("<a />", document), "replace load");
	output = "previous";
	check(saveToString(output, document) && (output == "<a />"), "saveToString replaces output");
	check(saveToString(output, document) && (output == "<a />"), "saveToString replaces output again");
	output = "previous";
	check(!saveToString(output, nullptr) && (output == "previous"), "saveToString nullptr keeps output");
};

static void testDocumentTypeDefinition() {
	checkSame("doctype", "<!DOCTYPE note>\n<note />");
	checkSame("doctype public", "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">");
	checkSame("doctype internal subset", "<!DOCTYPE n [\n<!ENTITY e \"a > b\">\n<!-- it's > ] -->\n]>\n<n />");
	checkOutput("doctype spaces", "<!DOCTYPE \t note>", Mode::Normal, Mode::Normal, "<!DOCTYPE note>");

	TPointer<Document> document;
	check(loadFromString("<!DOCTYPE note><note/>", document), "doctype load");
	if (document) {
		TPointer<Node> node = document->getIndex(0);
		check(node && (node->type == NodeType::DocumentTypeDefinition) && (node->name == "note"), "doctype name");
	};
};

static void testAttributes() {
	checkOutput("single quotes", "<a x='1'/>", Mode::Normal, Mode::Normal, "<a x=\"1\" />");
	checkSame("single quotes with double quote", "<a x='say \"hi\"' />");
	checkSame("double quotes with single quote", "<a x=\"it's\" />");
	checkOutput("no space between attributes", "<a x=\"1\"y=\"2\"/>", Mode::Normal, Mode::Normal, "<a x=\"1\" y=\"2\" />");
	checkOutput("space around =", "<a x = \"1\"\n\ty\t=\t'2' />", Mode::Normal, Mode::Normal, "<a x=\"1\" y=\"2\" />");

	// both quote types
	Document document;
	TPointer<Node> node;
	node.newMemory();
	node->type = NodeType::Element;
	node->name = "a";
	node->attributes.newMemory();
	TPointer<Attribute> attribute;
	attribute.newMemory();
	attribute->name = "x";
	attribute->value = "it's \"q\"";
	node->attributes->set(0, attribute);
	document.add(node);
	String output = toString(&document, Mode::Normal);
	check(output == "<a x=\"it's &quot;q&quot;\" />", "both quote types");

	TPointer<Document> reload;
	check(loadFromString("<a b=\"x\" c='y' />", reload), "attributes load");
	if (reload) {
		TPointer<Node> element = reload->getIndex(0);
		check(element && element->attributes && (element->attributes->length() == 2), "attributes count");
		if (element && element->attributes && (element->attributes->length() == 2)) {
			check((*element->attributes)[0]->name == "b", "attribute 0 name");
			check((*element->attributes)[0]->value == "x", "attribute 0 value");
			check((*element->attributes)[1]->name == "c", "attribute 1 name");
			check((*element->attributes)[1]->value == "y", "attribute 1 value");
		};
	};
};

static void testModes() {
	const char *xml = "\xEF\xBB\xBF<?xml version=\"1.0\"?>\n<a>\n  <b>  x  </b>\n  <c/>\n  <!-- d -->\n  <e><f/></e>\n</a>\n";

	checkSame("normal", "<?xml version=\"1.0\"?>\n<a>\n  <b>  x  </b>\n  <c />\n</a>\n");
	checkOutput("indentation tab", xml, Mode::Normal, Mode::IndentationTab,
	            "\xEF\xBB\xBF<?xml version=\"1.0\"?>\r\n<a>\r\n\t<b>x</b>\r\n\t<c />\r\n\t<!-- d -->\r\n\t<e>\r\n\t\t<f />\r\n\t</e>\r\n</a>\r\n");
	checkOutput("indentation 4 spaces", xml, Mode::Normal, Mode::Indentation4Spaces,
	            "\xEF\xBB\xBF<?xml version=\"1.0\"?>\r\n<a>\r\n    <b>x</b>\r\n    <c />\r\n    <!-- d -->\r\n    <e>\r\n        <f />\r\n    </e>\r\n</a>\r\n");
	checkOutput("minified", xml, Mode::Normal, Mode::Minified,
	            "\xEF\xBB\xBF<?xml version=\"1.0\"?><a><b>x</b><c /><!-- d --><e><f /></e></a>");
	checkOutput("read minified", xml, Mode::Minified, Mode::Normal,
	            "\xEF\xBB\xBF<?xml version=\"1.0\"?><a><b>x</b><c /><!-- d --><e><f /></e></a>");

	// Mixed content keeps the space between words and elements
	const char *mixed = "<p>\n  Hello <b>big</b> <i>world</i> !\n</p>";
	checkOutput("mixed minified", mixed, Mode::Normal, Mode::Minified, "<p>Hello <b>big</b> <i>world</i> !</p>");
	checkOutput("mixed read minified", mixed, Mode::Minified, Mode::Normal, "<p>Hello <b>big</b> <i>world</i> !</p>");
	checkOutput("mixed indentation", "<r>\n<p>Hello <b>big</b> world</p>\n</r>", Mode::Normal, Mode::IndentationTab, "<r>\r\n\t<p>Hello <b>big</b> world</p>\r\n</r>\r\n");
};

static void testDocument() {
	Document empty;
	check(empty.length() == 0, "empty length");
	check(!empty.getIndex(0), "empty getIndex");
	empty.setIndex(0, nullptr);
	empty.removeIndex(0);
	check(!empty.get("a"), "empty get");
	check(!empty.find("a"), "empty find");
	check(!empty.findWithAttributeValue("a", "x", "1"), "empty findWithAttributeValue");

	// Elements created without attributes
	Document document;
	TPointer<Node> node;
	node.newMemory();
	node->type = NodeType::Element;
	node->name = "a";
	Document branch = document.add(node);
	node.newMemory();
	node->type = NodeType::Element;
	node->name = "a";
	branch.add(node);
	check(!document.findWithAttributeValue("a", "x", "1"), "findWithAttributeValue without attributes");
	check(document.find("a").length() == 2, "find nested");
	check(document.get("a").length() == 1, "get");
	check(!document.getIndex(5), "getIndex out of range");

	// Self add
	document.addDocument(document);
	check(document.length() == 2, "addDocument self");

	TPointer<Document> xml;
	check(loadFromString("<r><a x=\"1\" x=\"1\"/><b><a x=\"1\"><a x=\"2\"/></a></b><a/></r>", xml), "find load");
	if (xml) {
		check(xml->find("a").length() == 4, "find count");
		check(xml->findWithAttributeValue("a", "x", "1").length() == 2, "findWithAttributeValue count, duplicate attribute counted once");
		check(xml->findWithAttributeValue("a", "x", "2").length() == 1, "findWithAttributeValue nested");
		Document r = xml->get("r");
		check(r.length() == 1, "get root");
		xml->removeIndex(0);
		check(xml->length() == 0, "removeIndex");
	};
};

static void testDepth() {
	String xml;
	size_t k;
	for (k = 0; k < 1000; ++k) {
		xml += "<a>";
	};
	for (k = 0; k < 1000; ++k) {
		xml += "</a>";
	};
	TPointer<Document> document;
	check(loadFromString(xml, document), "depth 1000");

	xml = "";
	for (k = 0; k < 100000; ++k) {
		xml += "<a>";
	};
	for (k = 0; k < 100000; ++k) {
		xml += "</a>";
	};
	check(!loadFromString(xml, document), "depth 100000 must fail");
};

static void testFile() {
	// Larger than the read buffer, tokens cross buffer boundaries
	String xml;
	xml += "<?xml version=\"1.0\"?>\n<root>\n";
	for (int k = 0; k < 5000; ++k) {
		xml += "  <item id=\"";
		xml += "123";
		xml += "\" kind=\"x\"><name>Some item name</name><!-- comment --><![CDATA[<data>]]></item>\n";
	};
	String longName;
	for (int k = 0; k < 70000; ++k) {
		longName += "n";
	};
	xml += "  <";
	xml += longName;
	xml += ">x</";
	xml += longName;
	xml += ">\n</root>";

	{
		File file;
		check(file.openWrite("test.03.big.xml"), "big file open");
		check(Stream::write(file, xml) == xml.length(), "big file write");
	};

	TPointer<Document> fromFile;
	TPointer<Document> fromString;
	check(load("test.03.big.xml", fromFile), "big file load");
	check(loadFromString(xml, fromString), "big string load");
	if (fromFile && fromString) {
		check(toString(fromFile, Mode::Normal) == xml, "big file round trip");
		check(toString(fromString, Mode::Normal) == xml, "big string round trip");
	};

	check(save("test.03.big.out.xml", fromFile, Mode::Normal), "big file save");
	TPointer<Document> reload;
	check(load("test.03.big.out.xml", reload), "big file reload");
	if (reload) {
		check(toString(reload, Mode::Normal) == xml, "big file reload round trip");
	};

	Shell::remove("test.03.big.xml");
	Shell::remove("test.03.big.out.xml");
};

int main(int cmdN, char *cmdS[]) {

	try {
		testEndOfInput();
		testInvalid();
		testEmptyValues();
		testDocumentTypeDefinition();
		testAttributes();
		testModes();
		testDocument();
		testDepth();
		testFile();

		if (errorCount == 0) {
			printf("Done.\r\n");
			return 0;
		};
		printf("* Errors: %d\n", errorCount);
	} catch (const std::exception &e) {
		printf("* Error: %s\n", e.what());
	} catch (...) {
		printf("* Error: Unknown\n");
	};

	return 1;
};
