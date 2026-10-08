# Document model

A parsed document is a list of managed nodes; elements carry their children
in their own list, which makes a tree. Three classes, all derived from
`Object` (reference counted, allocated from per-thread active pools):

```cpp
class Attribute : public Object {
	String name;
	String value;
};

enum struct NodeType { None, Element, Content, CDATA, Comment, Declaration, Bom, DocumentTypeDefinition };

class Node : public Object {
	typedef TDynamicArray<TPointer<Attribute>, 3> Attributes;
	typedef TDoubleEndedQueue<TPointerX<Node>> Branch;

	NodeType type;                    // None by default
	String name;                      // tag name, or the text of the node
	TPointer<Attributes> attributes;  // elements only, may be nullptr
	TPointerX<Branch> branch;         // children, elements only, may be nullptr
};

struct Document : Object {
	typedef Node::Branch Branch;
	typedef Node::Attributes Attributes;

	TPointerX<Branch> root;           // the list, nullptr when empty
	...
};
```

`Attribute` and `Node` cannot be copied, assigned or moved; you work with
pointers to them. `Document` can be copied: a copy shares the same list.

## Nodes

What `name` holds depends on `type`:

| `type` | Source | `name` | `attributes`, `branch` |
|--------|--------|--------|------------------------|
| `Element` | `<x:item id="1">...</x:item>`, `<br/>` | tag name, `x:item` | always set by the reader (maybe empty) |
| `Content` | text between markup | the raw text, entities not decoded: `a &amp; b` | `nullptr` |
| `CDATA` | `<![CDATA[ a < b ]]>` | text between `<![CDATA[` and `]]>`: ` a < b ` | `nullptr` |
| `Comment` | `<!-- note -->` | text between `<!--` and `-->`: ` note ` | `nullptr` |
| `Declaration` | `<?xml version="1.0"?>`, any `<?...?>` | text between `<?` and `?>`: `xml version="1.0"` | `nullptr` |
| `DocumentTypeDefinition` | `<!DOCTYPE html>` | text after `<!DOCTYPE` and its spaces, up to the final `>`: `html` (internal subset included) | `nullptr` |
| `Bom` | UTF-8 byte order mark at the start | `"\xEF\xBB\xBF"` | `nullptr` |
| `None` | never produced by the reader | - | skipped by the writer |

Attributes keep their order and are not unique: `<a x="1" x="2"/>` has two
attributes named `x`. Values are raw text, as written between the quotes.

The text of an element is the `Content` (and `CDATA`) nodes of its branch.
`<name>Waffles</name>` is an `Element` named `name` whose branch holds one
`Content` node named `Waffles`.

## Document: a list of nodes

A `Document` is a list of nodes, nothing more. The same class is used for:

- **a parsed document**: the top level nodes (BOM, declaration, `DOCTYPE`,
  comments, the root element, text between them);
- **the children of an element**: `Document children(node->branch);` is a
  view that shares the element's list; adding to or removing from it
  changes the element;
- **a query result**: `get`, `find` and `findWithAttributeValue` return a
  **new list** holding the matching nodes, shared with the tree.

`operator bool` is `true` when the list exists. Queries create the list on
the first match, so `if (document->get("x"))` reads "found". A parsed
document, or a view of a parsed element, has a list even when it is empty.

| Member | Description |
|--------|-------------|
| `length()` | number of nodes in this list, of any type (walks the list) |
| `getIndex(k)` | `TPointer<Node>` to node `k`, `nullptr` past the end (walks the list) |
| `setIndex(k, node)` | replace node `k`; nothing happens past the end |
| `removeIndex(k)` | remove node `k` from this list; nothing happens past the end |
| `add(node)` | append `node` (creates the list if needed), creates `node->branch` if missing, returns a view of `node->branch` |
| `addDocument(other)` | append every node of `other` (the nodes are shared, not copied); `other` may be this document |
| `get(name)` | new list: the `Element`s named `name` **in this list** (not their children) |
| `find(name)` | new list: the `Element`s named `name` in this list and all their descendants, in document order |
| `findWithAttributeValue(name, attribute, value)` | as `find`, keeping elements having an attribute `attribute` equal to `value` (exact, case sensitive) |
| `empty()` | drop this document's reference to its list (`root = nullptr`) |

Names and values are compared as strings, case sensitive, without
namespace processing.

## Navigating

`get` looks at the list it is called on. To descend, wrap an element's
children:

```cpp
TPointer<Document> xml;
if (!load("menu.xml", xml, Mode::Minified)) {
	return false;
};

TPointer<Node> menu = xml->get("menu").getIndex(0);      // the root element
if (!menu) {
	return false;
};
Document foods = Document(menu->branch).get("food");    // <food> children of <menu>
for (size_t k = 0; k < foods.length(); ++k) {
	TPointer<Node> food = foods.getIndex(k);
	TPointer<Node> price = Document(food->branch).get("price").getIndex(0);
	...
};
```

What chaining does:

| Expression | Result |
|------------|--------|
| `xml->get("menu").get("food")` | **empty**: the second `get` looks at the `menu` elements themselves |
| `Document(menu->branch).get("food")` | the `food` children of `menu` |
| `xml->get("menu").find("food")` | `food` elements anywhere inside the top level `menu` elements (and the `menu` elements themselves, if named `food`) |
| `xml->find("food")` | every `food` element of the document |
| `xml->findWithAttributeValue("food", "id", "2")` | every `food` element with `id="2"` |

### Walking a list

`length()` and `getIndex(k)` walk the list from its head, so a loop over
`getIndex(k)` is quadratic. For long lists walk the nodes directly. The list
is a `TDoubleEndedQueue`; its nodes (type `Node::Branch::Node`, not to be
confused with `XYO::FileXML::Node`) have `value` (a `TPointerX<Node>`),
`next` and `back`:

```cpp
// every child element of node
if (node->branch) {
	for (Node::Branch::Node *scan = node->branch->head; scan != nullptr; scan = scan->next) {
		Node *child = scan->value;
		if (!child || (child->type != NodeType::Element)) {
			continue;
		};
		...
	};
};
```

The same works on any `Document`: `for (scan = document.root->head; ...)`
after checking `document.root`.

## Text and whitespace

With the default `Mode::Normal` the reader keeps **all** text, including the
indentation between elements. After

```cpp
loadFromString("<r>\n  <a/>\n  <b>x</b>\n</r>", xml);
```

the branch of `r` has 5 nodes: `Content "\n  "`, `Element a`,
`Content "\n  "`, `Element b`, `Content "\n"`. `get` / `find` skip text
nodes, but `length()` and `getIndex()` count them.

Read with `Mode::Minified` when you want the data, not the layout: text is
trimmed and whitespace only text is dropped, so the branch of `r` above has
2 nodes. In mixed content (`<p>Hello <b>big</b> world</p>`) one space is
kept between words and elements. See [Reading XML](reading.md#modes).

**Entities are not decoded.** `<a x="&amp;1">&lt;b&gt;</a>` gives the
attribute value `&amp;1` and the text `&lt;b&gt;`. Decode them yourself
when you need the characters (see `decodeEntities` below). CDATA text needs
no decoding.

## Building and changing documents

Create nodes with `newMemory()`, set `type` and `name`, and hand them to a
list; the list keeps them alive.

```cpp
Document xml;
xml.add(newNode(NodeType::Declaration, "xml version=\"1.0\" encoding=\"UTF-8\""));

Document config = xml.add(newNode(NodeType::Element, "config"));

TPointer<Node> server = newNode(NodeType::Element, "server");
setAttribute(server, "host", "localhost");
setAttribute(server, "port", "8080");
config.add(server);

config.add(newNode(NodeType::Element, "note")).add(newNode(NodeType::Content, escapeText("a < b & c")));
config.add(newNode(NodeType::Comment, " end "));

save("config.xml", &xml, Mode::IndentationTab);
```

`config.xml` (CRLF line endings):

```xml
<?xml version="1.0" encoding="UTF-8"?>
<config>
	<server host="localhost" port="8080" />
	<note>a &lt; b &amp; c</note>
	<!-- end -->
</config>
```

(`newNode`, `setAttribute` and `escapeText` are the helpers below.) A
`Document` can live on the stack, as here, or be held by a
`TPointer<Document>` as returned by `load`.

Changing a loaded document:

| Task | Code |
|------|------|
| rename an element | `node->name = "item";` |
| change text | `textNode->name = "new text";` (raw: escape `<` and `&`) |
| change / add an attribute | `setAttribute(node, "id", "7")` (below) |
| remove attribute `k` | `node->attributes->remove(k)` |
| append a child | `Document(node->branch).add(child)` or `node->branch->pushToTail(child)` (check `node->branch` first) |
| insert a child first | `node->branch->push(child)` (`push` adds at the head of the list) |
| replace / remove child `k` | `Document(node->branch).setIndex(k, child)` / `.removeIndex(k)` |
| remove all children | `node->branch->empty()` |
| remove a node found by a query | remove it from its parent's branch, see `removeChild` below |

## Helpers worth writing

The library stays minimal; a few helpers make application code short. These
compile as they are:

```cpp
static TPointer<Node> newNode(NodeType type, const char *name) {
	TPointer<Node> node;
	node.newMemory();
	node->type = type;
	node->name = name;
	return node;
};

// first child element named name, nullptr if none
static TPointer<Node> childElement(Node *node, const char *name) {
	if (!node || !node->branch) {
		return nullptr;
	};
	return Document(node->branch).get(name).getIndex(0);
};

// text of an element: its Content and CDATA children, joined (raw, entities not decoded)
static String getText(Node *node) {
	String retV;
	if (!node || !node->branch) {
		return retV;
	};
	for (Node::Branch::Node *scan = node->branch->head; scan != nullptr; scan = scan->next) {
		if (!scan->value) {
			continue;
		};
		if ((scan->value->type == NodeType::Content) || (scan->value->type == NodeType::CDATA)) {
			retV += scan->value->name;
		};
	};
	return retV;
};

static bool getAttribute(Node *node, const char *name, String &value) {
	if (!node || !node->attributes) {
		return false;
	};
	for (size_t k = 0; k < node->attributes->length(); ++k) {
		Attribute *attribute = node->attributes->index(k);
		if (attribute && (attribute->name == name)) {
			value = attribute->value;
			return true;
		};
	};
	return false;
};

// replace the first attribute named name, or append a new one; value is raw text
static void setAttribute(Node *node, const char *name, const char *value) {
	if (!node->attributes) {
		node->attributes.newMemory();
	};
	for (size_t k = 0; k < node->attributes->length(); ++k) {
		Attribute *attribute = node->attributes->index(k);
		if (attribute && (attribute->name == name)) {
			attribute->value = value;
			return;
		};
	};
	TPointer<Attribute> attribute;
	attribute.newMemory();
	attribute->name = name;
	attribute->value = value;
	node->attributes->push(attribute);
};

// remove child from parent's list, false if it is not a child of parent
static bool removeChild(Node *parent, Node *child) {
	if (!parent || !parent->branch) {
		return false;
	};
	for (Node::Branch::Node *scan = parent->branch->head; scan != nullptr; scan = scan->next) {
		if (scan->value == child) {
			parent->branch->extractNode(scan);
			Node::Branch::deleteNode(scan);
			return true;
		};
	};
	return false;
};

// text -> XML text or attribute value
static String escapeText(const String &value) {
	String retV;
	for (size_t k = 0; k < value.length(); ++k) {
		char char_ = value[k];
		switch (char_) {
		case '&':
			retV += "&amp;";
			break;
		case '<':
			retV += "&lt;";
			break;
		case '>':
			retV += "&gt;";
			break;
		case '"':
			retV += "&quot;";
			break;
		default:
			retV += char_;
			break;
		};
	};
	return retV;
};

static void appendUTF8(String &output, uint32_t code) {
	if (code < 0x80) {
		output += static_cast<char>(code);
	} else if (code < 0x800) {
		output += static_cast<char>(0xC0 | (code >> 6));
		output += static_cast<char>(0x80 | (code & 0x3F));
	} else if (code < 0x10000) {
		output += static_cast<char>(0xE0 | (code >> 12));
		output += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
		output += static_cast<char>(0x80 | (code & 0x3F));
	} else {
		output += static_cast<char>(0xF0 | (code >> 18));
		output += static_cast<char>(0x80 | ((code >> 12) & 0x3F));
		output += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
		output += static_cast<char>(0x80 | (code & 0x3F));
	};
};

// &amp; &lt; &gt; &quot; &apos; &#NN; &#xHH; -> characters (UTF-8); other references are kept
static String decodeEntities(const String &value) {
	String retV;
	const char *text = value.value();
	size_t length = value.length();
	for (size_t k = 0; k < length; ++k) {
		if (text[k] == '&') {
			const char *end = static_cast<const char *>(memchr(text + k, ';', length - k));
			if (end) {
				String name(text + k + 1, end - (text + k + 1));
				const char *replace = nullptr;
				if (name == "amp") {
					replace = "&";
				} else if (name == "lt") {
					replace = "<";
				} else if (name == "gt") {
					replace = ">";
				} else if (name == "quot") {
					replace = "\"";
				} else if (name == "apos") {
					replace = "'";
				};
				if (replace) {
					retV += replace;
					k = end - text;
					continue;
				};
				if ((name.length() > 1) && (name[0] == '#')) {
					char *last = nullptr;
					bool hex = (name[1] == 'x') || (name[1] == 'X');
					unsigned long code = strtoul(name.value() + (hex ? 2 : 1), &last, hex ? 16 : 10);
					if (last && (*last == 0) && (code > 0) && (code <= 0x10FFFF)) {
						appendUTF8(retV, static_cast<uint32_t>(code));
						k = end - text;
						continue;
					};
				};
			};
		};
		retV += text[k];
	};
	return retV;
};
```

Usage:

```cpp
TPointer<Node> food = xml->findWithAttributeValue("food", "id", "2").getIndex(0);
String name = decodeEntities(getText(childElement(food, "name")));
```

Returning a `TPointer<Node>` from `childElement` keeps the node alive even
if the query result that found it goes away; a raw `Node *` would also be
fine while the tree still holds the node.

## Memory rules

The rules of `xyo-managed-memory` apply:

- **`TPointer<Document>` / `TPointer<Node>`** for locals, parameters and
  return values. `load` gives you a `TPointer<Document>`; it keeps the whole
  tree alive.
- **Raw pointers** (`Node *`, `Attribute *`) into the tree are fine while
  the tree holds the node. They dangle once the node is removed or replaced
  (`removeIndex`, `setIndex`, `branch->empty()`, ...) and nothing else holds
  it. Take a `TPointer` if you need the node to outlive that.
- **Query results share nodes.** A `Document` returned by `get` / `find`
  holds references to the matching nodes: changing a node through it
  (`name`, `attributes`, `branch`) changes the tree, but `add`,
  `setIndex`, `removeIndex` on the result change only the result list.
  Keeping a result also keeps its nodes alive after they are removed from
  the tree.
- **Views share lists.** `Document(node->branch)` and the `Document`
  returned by `add` operate on the element's own list. Their `empty()`
  only drops the view's reference; use `node->branch->empty()` to remove
  the children.
- **As a class member** use `TPointerX<Document>` / `TPointerX<Node>` and
  link it in the constructor: `document.pointerLink(this);`.
- **Never make a node its own descendant.** The writer and `find` recurse
  without a depth limit; a cycle overflows the stack. A node added in two
  places of the tree is written twice.
- One thread per document, see [Getting started](getting-started.md#5-threads).

## Pitfalls

1. **Whitespace text nodes.** With `Mode::Normal` the indentation between
   elements is kept as `Content` nodes; `length()` and `getIndex(k)` count
   them. Read with `Mode::Minified` for data access, use `get` / `find`, or
   skip nodes whose `type` is not `Element`.
2. **`get` does not descend.** `xml->get("a").get("b")` looks for `b` among
   the `a` elements themselves and is empty. Use
   `Document(node->branch).get("b")`, or `find`.
3. **Entities are raw in both directions.** The reader does not decode
   `&amp;` / `&lt;` / `&#...;`; the writer does not escape `<`, `&` or `>`
   in text nodes and attribute values (only `"` in an attribute value that
   contains both quote kinds). Escape text you put in the tree
   (`escapeText`) or the output will not load again (`1 < 2` in text is a
   syntax error for the reader).
4. **`getIndex` / `length` walk the list**: `O(k)` / `O(n)`. Walk
   `branch->head` ... `next` in loops over long lists.
5. **Built nodes may have `nullptr` `attributes` / `branch`.** The reader
   always creates both for elements; `newMemory()` does not. Check before
   dereferencing (the helpers above do). `Document::add` creates the branch.
6. **An element without children is written as `<a />`**, also when it was
   read as `<a></a>`. An element with an empty `Content` child is written
   as `<a></a>`, but reading that back gives a childless element again.
7. **Changing text is changing `name`.** A `Content` node's text is its
   `name`; an element's text is in its children, not in its `name`.
8. **Duplicate attributes are kept** on read and on write;
   `findWithAttributeValue` matches if any of them matches.
9. `Version`, `Copyright`, `License` exist in every XYO library: qualify
   them (`XYO::FileXML::Version::version()`).
