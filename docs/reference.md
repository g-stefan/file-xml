# API reference

Namespace `XYO::FileXML` unless noted. Everything is declared by
`#include <XYO/FileXML.hpp>`.

## Macros

| Macro | Meaning |
|-------|---------|
| `XYO_FILEXML_EXPORT` | dllexport while building the DLL (`XYO_FILEXML_INTERNAL`), dllimport for users, empty with `XYO_FILEXML_LIBRARY` or on static platforms (`XYO_PLATFORM_COMPILE_STATIC`) |
| `XYO_FILEXML_INTERNAL` | defined by the build when compiling this library (`FILE_XML_INTERNAL` is accepted too) |
| `XYO_FILEXML_LIBRARY` | plain static linking without export / import |

## enum struct NodeType

| Value | Node |
|-------|------|
| `None = 0` | not set; skipped by the writer |
| `Element = 1` | element; `name` is the tag name |
| `Content = 2` | text; `name` is the raw text |
| `CDATA = 3` | `<![CDATA[name]]>` |
| `Comment = 4` | `<!--name-->` |
| `Declaration = 5` | `<?name?>`, XML declaration or processing instruction |
| `Bom = 6` | UTF-8 byte order mark, `name` = `"\xEF\xBB\xBF"` |
| `DocumentTypeDefinition = 7` | `<!DOCTYPE name>` |

## enum struct Mode

| Value | Reading | Writing |
|-------|---------|---------|
| `Normal = 0` | keep all text | as it is |
| `Minified = 1` | trim text, drop whitespace only text | no line breaks, text trimmed |
| `Indentation4Spaces = 2` | as `Normal` | one node per line, 4 spaces per level, CRLF |
| `IndentationTab = 3` | as `Normal` | one node per line, tab per level, CRLF |

## Reading and writing

| Function | Description |
|----------|-------------|
| `bool load(const char *fileName, TPointer<Document> &document, Mode mode = Mode::Normal)` | parse a file; `false` and `document = nullptr` on error |
| `bool loadFromString(const char *value, TPointer<Document> &document, Mode mode = Mode::Normal)` | parse a 0 terminated string; `false` and `document = nullptr` on error (also for `nullptr` and `""`) |
| `bool save(const char *fileName, Document *document, Mode mode = Mode::Normal)` | write a file (created / truncated); `false` for `nullptr` arguments or I/O errors, file content then unspecified |
| `bool saveToString(String &output, Document *document, Mode mode = Mode::Normal)` | **replace** `output` with the XML text; `false` only for a `nullptr` document, `output` then unchanged |

See [Reading XML](reading.md) and [Writing XML](writing.md).

## class Attribute : public Object

Not copyable, assignable or movable. Allocated from `TMemoryPoolActive`
(per thread): `TPointer<Attribute> a; a.newMemory();`.

| Member | Description |
|--------|-------------|
| `String name` | attribute name |
| `String value` | raw value, without the quotes, entities not decoded |

## class Node : public Object

Not copyable, assignable or movable. Allocated from `TMemoryPoolActive`
(per thread): `TPointer<Node> n; n.newMemory();`.

| Member | Description |
|--------|-------------|
| `typedef TDynamicArray<TPointer<Attribute>, 3> Attributes` | attribute list |
| `typedef TDoubleEndedQueue<TPointerX<Node>> Branch` | child list; its list nodes are `Branch::Node` with `value`, `next`, `back` |
| `NodeType type` | `NodeType::None` after construction |
| `String name` | tag name for elements, text for the other types, see [Document model](document-model.md#nodes) |
| `TPointer<Attributes> attributes` | attributes of an element; created by the reader for every element, `nullptr` on new nodes |
| `TPointerX<Branch> branch` | children of an element; created by the reader for every element and by `Document::add`, `nullptr` on new nodes |

## struct Document : Object

A list of nodes: a whole document, a view of an element's children, or a
query result. Copy construction and assignment share the list.
`TMemory<Document>` is `TMemoryPoolActive<Document>`; a `Document` can also
live on the stack.

| Member | Description |
|--------|-------------|
| `typedef Node::Branch Branch`, `typedef Node::Attributes Attributes` | |
| `TPointerX<Branch> root` | the list, `nullptr` when there is none |
| `Document()` | no list |
| `Document(const Branch *root_)`, `Document(const TPointerX<Branch> &root_)` | view of an existing list, for example `Document(node->branch)` |
| `Document(const Document &)`, `Document(Document &&)`, `operator=` (also from `const Branch *`) | share / take the other's list |
| `operator bool() const` | `root != nullptr` |
| `void empty()` | `root = nullptr` (drops this reference only) |
| `Document add(Node *node)` | append `node`, create the list and `node->branch` if missing; returns `Document(node->branch)` |
| `void addDocument(Document &document)` | append the nodes of `document` (shared); safe when `document` is `*this` |
| `Document get(const char *name)` | new list of the `Element`s named `name` in this list |
| `Document find(const char *name)` | new list of the `Element`s named `name` in this list and below, document order |
| `Document findWithAttributeValue(const char *name, const char *attribute, const char *value)` | as `find`, only elements with an attribute `attribute` equal to `value` |
| `size_t length()` | number of nodes in the list (any type); walks the list |
| `TPointer<Node> getIndex(size_t index_)` | node `index_`, `nullptr` past the end; walks the list |
| `void setIndex(size_t index_, Node *node)` | replace node `index_`; no-op past the end |
| `void removeIndex(size_t index_)` | remove node `index_` from the list; no-op past the end |
| `static Node *newNode()`, `static void deleteNode(Node *)` | `TMemory<Node>::newMemory()` / `deleteMemory()` |

## Metadata

| Function | Returns |
|----------|---------|
| `XYO::FileXML::Version::version()` | `"7.0.0"` style version |
| `XYO::FileXML::Version::build()` | build number |
| `XYO::FileXML::Version::versionWithBuild()` | version and build |
| `XYO::FileXML::Version::datetime()` | build date and time |
| `XYO::FileXML::Copyright::copyright()`, `publisher()`, `company()`, `contact()` | copyright strings |
| `XYO::FileXML::License::license()`, `shortLicense()` | license text (`std::string`) |

## Internal

`Input` (buffered character input over an `IRead` or memory, `Input.hpp`)
and the text rules in `Whitespace.hpp` (`isSpaceASCII`, `trimRangeASCII`,
`isMixedContent`, `isContentBranch`, `minifyText`, ...) are compiled into
the library but not included by `<XYO/FileXML.hpp>`; the `Reader` and
`Writer` structures are local to `Reader.cpp` / `Writer.cpp`. They are
implementation details and may change.
