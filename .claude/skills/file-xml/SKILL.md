---
name: file-xml
description: >-
  How to use the file-xml C++ library (namespace XYO::FileXML), the XML
  reader / writer of the XYO C++ stack on top of xyo-system: load /
  loadFromString into a TPointer<Document>, save / saveToString with
  Mode::Normal / Minified / IndentationTab / Indentation4Spaces, the
  document model Document (a list of nodes: root, add, addDocument, get,
  find, findWithAttributeValue, length, getIndex, setIndex, removeIndex),
  Node (type, name, attributes, branch), NodeType (Element, Content, CDATA,
  Comment, Declaration, Bom, DocumentTypeDefinition), Attribute (name,
  value), Node::Attributes (TDynamicArray) and Node::Branch
  (TDoubleEndedQueue of TPointerX<Node>). Use when writing or reviewing
  code that includes <XYO/FileXML.hpp>, depends on "file-xml" in
  fabricare.json, reads or writes XML in XYO C++ code, uses any of these
  names, or when working inside the file-xml repository or libraries built
  on it (quantum-script--xml).
---

# file-xml

XML library of the XYO C++ libraries, on top of `xyo-system` (see the
`xyo-system`, `xyo-encoding`, `xyo-data-structures` and `xyo-managed-memory`
skills; their rules apply). Purpose: **read XML into a tree of managed
nodes, query and change it, write it back**, lossless and lenient: a
non-validating markup reader that keeps text raw.

Full documentation: `docs/` in the file-xml repository
(`X:\Storage\XYO\Gitea\CPP\file-xml\docs` on this machine): README,
getting-started, **document-model** (nodes, navigating, building, helpers,
memory rules, pitfalls), reading, writing, reference. Read the matching
page when you need more than this summary. When in doubt read the header in
`source/XYO/FileXML/`.

## API

```cpp
#include <XYO/FileXML.hpp>
using namespace XYO::FileXML;   // also brings in String, TPointer, TDynamicArray, ...

TPointer<Document> xml;
bool ok = load("file.xml", xml);                       // false -> xml == nullptr
bool ok = load("file.xml", xml, Mode::Minified);       // drop whitespace only text, trim text
bool ok = loadFromString(text, xml);                   // 0 terminated const char *
bool ok = save("file.xml", xml);                       // Mode::Normal: as it is
bool ok = save("file.xml", xml, Mode::IndentationTab); // or Indentation4Spaces, Minified
String out;
bool ok = saveToString(out, xml, Mode::Minified);      // replaces out; unchanged on error
```

```cpp
class Attribute : Object { String name; String value; };          // raw value
class Node : Object {
	typedef TDynamicArray<TPointer<Attribute>, 3> Attributes;
	typedef TDoubleEndedQueue<TPointerX<Node>> Branch;          // list nodes: Branch::Node {value, next, back}
	NodeType type;                                              // None after newMemory()
	String name;                                                // tag name, or the node's text
	TPointer<Attributes> attributes;                            // may be nullptr
	TPointerX<Branch> branch;                                   // children, may be nullptr
};
struct Document : Object { TPointerX<Branch> root; ... };       // a list of nodes
```

| `type` | `name` holds |
|--------|--------------|
| `Element` | tag name (`x:item`, no namespace processing) |
| `Content` | raw text, entities NOT decoded (`a &amp; b`) |
| `CDATA` | text inside `<![CDATA[ ]]>` |
| `Comment` | text inside `<!-- -->` |
| `Declaration` | text inside `<? ?>` (`xml version="1.0"`, any PI) |
| `DocumentTypeDefinition` | text after `<!DOCTYPE ` up to `>` |
| `Bom` | `"\xEF\xBB\xBF"` (first node when the input starts with a BOM) |

## Navigating

```cpp
TPointer<Node> root = xml->get("menu").getIndex(0);          // top level element, nullptr if missing
if (!root) { /* ... */ };
Document children(root->branch);                             // view of root's children
Document foods = children.get("food");                       // direct children named food
for (size_t k = 0; k < foods.length(); ++k) {
	TPointer<Node> food = foods.getIndex(k);
	TPointer<Node> name = Document(food->branch).get("name").getIndex(0);
};
xml->find("food");                                           // any depth, document order
xml->findWithAttributeValue("food", "id", "2");              // any depth, exact match
if (xml->get("x")) { /* found: a query result is false when nothing matched */ };

for (Node::Branch::Node *scan = root->branch->head; scan; scan = scan->next) {
	Node *child = scan->value;                               // fast walk, check type
	if (!child || child->type != NodeType::Element) continue;
};
```

Element text = its `Content` + `CDATA` children joined; attributes =
`node->attributes->index(k)->name / ->value`. Ready made helpers (`getText`,
`getAttribute`, `setAttribute`, `childElement`, `newNode`, `removeChild`,
`escapeText`, `decodeEntities`) are in `docs/document-model.md`, section
"Helpers worth writing"; copy them instead of rewriting.

## Building / changing

```cpp
static TPointer<Node> newNode(NodeType type, const char *name) {
	TPointer<Node> node;
	node.newMemory();
	node->type = type;
	node->name = name;
	return node;
};

Document xml;                                                // stack or TPointer<Document>
xml.add(newNode(NodeType::Declaration, "xml version=\"1.0\" encoding=\"UTF-8\""));
Document config = xml.add(newNode(NodeType::Element, "config"));   // add returns view of node->branch
TPointer<Node> server = newNode(NodeType::Element, "server");
server->attributes.newMemory();                              // nullptr on new nodes
TPointer<Attribute> attribute;
attribute.newMemory();
attribute->name = "port";
attribute->value = "8080";                                   // raw: escape & < " yourself
server->attributes->push(attribute);
config.add(server);
config.add(newNode(NodeType::Element, "note")).add(newNode(NodeType::Content, "a &lt; b"));
save("config.xml", &xml, Mode::IndentationTab);

node->name = "item";                                          // rename element / change text node
Document(node->branch).removeIndex(k);                        // remove child k (counts text nodes)
node->branch->push(child);                                    // insert first (push = head)
node->branch->empty();                                        // remove all children
```

## Hard rules

1. **Check every lookup**: `getIndex` returns `nullptr` when nothing
   matched or past the end; `attributes` / `branch` may be `nullptr` on
   nodes you created (the reader always creates both for elements;
   `Document::add` creates `branch`). Never assume the shape of input XML.
2. **`get` does not descend.** It searches the list it is called on:
   `xml->get("a").get("b")` is always empty. Descend with
   `Document(node->branch).get("b")`, or use `find`.
3. **Whitespace text nodes.** With `Mode::Normal` (default) the indentation
   between elements is kept as `Content` nodes and counted by `length()` /
   `getIndex()`. Load with `Mode::Minified` for data access, or filter by
   `type == NodeType::Element`. `IndentationTab` / `Indentation4Spaces` read
   like `Normal`.
4. **Text is raw both ways.** Entities / character references are never
   decoded on read (`&amp;` stays `&amp;`) and the writer never escapes
   `<`, `&`, `>` in text or attributes (only `"` -> `&quot;` when a value
   contains both quote kinds; otherwise it picks `"` or `'`). Escape text
   before putting it in the tree, decode after reading; text `1 < 2` makes
   output that does not load again.
5. **`saveToString` replaces `out`** on success and leaves it unchanged on
   failure. `save` / `saveToString` fail only for `nullptr` arguments or I/O
   errors; a failed `save` may leave a partial file.
6. **Query results are new lists sharing nodes**: changing a node through
   a result changes the tree; `add` / `setIndex` / `removeIndex` on a
   result change only the result. `Document(node->branch)` is a view: its
   `add` / `removeIndex` change the element; its `empty()` only drops the
   view (use `node->branch->empty()`).
7. **`length()` / `getIndex(k)` walk the list** (`O(n)`): loops over long
   lists should walk `branch->head` / `next`.
8. **Ownership**: `TPointer<Document>` / `TPointer<Node>` for locals,
   parameters, returns; raw `Node *` only while the tree holds the node;
   class members are `TPointerX<...>` with `pointerLink(this)` in the
   constructor. Never make a node its own descendant: `find` and the writer
   recurse without limit (stack overflow).
9. **One thread per document** (per-thread pools, plain refcounts). Move a
   document between threads as text: `saveToString` -> copy characters ->
   `loadFromString` on the other thread. Call
   `XYO::ManagedMemory::Registry::registryInit()` first in `main` when
   threads are used.
10. **Reader limits**: lenient (several top level elements, top level text,
    raw `&` / `>` in text, any name characters, duplicate attributes kept)
    but strict about structure: every element closed by an exactly matching
    end tag, attribute values quoted, `<` must start markup. Max element
    nesting 1024 (fixed). Empty input fails; whitespace only input loads.
    No error position. No DTD processing, no external entities (XXE safe).
    UTF-16 / UTF-32 input not supported; bytes are kept as they are.
11. **Normalizations on write** (`Mode::Normal` otherwise reproduces the
    source): childless elements become `<a />` (also `<a></a>`), attribute
    whitespace becomes ` x="v"`, single quotes become double unless the value
    has `"`, `<!DOCTYPE   x>` becomes `<!DOCTYPE x>`. Indented modes: CRLF,
    trailing CRLF after every node, text trimmed, text only elements and
    mixed content on one line. Minified / indented modes trim every text
    node: use `Mode::Normal` where whitespace matters.
12. `Version`, `Copyright`, `License` exist in every XYO library: qualify
    them (`XYO::FileXML::Version::version()`).

## Using it

- `#include <XYO/FileXML.hpp>`. C++17.
- fabricare consumer: `"dependency": ["file-xml"]`. The project is
  `dll-or-lib`: a DLL on dynamic platforms (`win64-msvc-2026`), a static
  library on static ones (`win64-msvc-2026.static`). Install
  `xyo-platform`, `xyo-managed-memory`, `xyo-data-structures`,
  `xyo-multithreading`, `xyo-encoding`, `xyo-system`, then this library to
  the SDK before building dependents (see the `fabricare` skill). On
  Windows run fabricare where `vcvarsall.bat x64` has been called.
- Without fabricare: compile the seven amalgams (`Platform`,
  `ManagedMemory`, `DataStructures`, `Multithreading`, `Encoding`, `System`,
  `FileXML` `.Amalgam.cpp`) with every `XYO_*_LIBRARY` defined (MSVC: also
  `/DXYO_PLATFORM_COMPILE_STATIC`), the `source/` dirs on the include path,
  `-pthread` on Linux.

## Code style (match the repository)

- Tabs (width 8), `.clang-format` in the repo, CRLF line endings; statements
  and blocks end with `};`.
- camelCase, `retV` for return values, trailing `_` for parameters that
  shadow members (`root_`, `index_`).
- Headers: include guard `XYO_FILEXML_<NAME>_HPP`, guarded includes
  (`#ifndef XYO_FILEXML_NODE_HPP #include ...`), a `TMemory<T>`
  specialization to `TMemoryPoolActive<T>` for each managed class, with
  `activeDestructor` resetting members and a static `initMemory` for member
  pools. Add new public headers to `source/XYO/FileXML.hpp`, new `.cpp`
  files to `source/XYO/FileXML.Amalgam.cpp`. Exported functions use
  `XYO_FILEXML_EXPORT`.
- Functions report errors by returning `bool` and leave outputs `nullptr` on
  failure; no exceptions.
- SPDX header: MIT for `source/`, Unlicense for `test/`.
- Tests: `test/test.NN.cpp` (see the `check` / `checkOutput` / `checkSame` /
  `checkLoadFail` pattern in `test.03.cpp`), plus a `"category": "test"`
  project in `fabricare.json`; run `fabricare make` then `fabricare test`.
