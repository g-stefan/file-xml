# File XML — Documentation

`file-xml` is the XML library of the XYO C++ stack. It sits on top of
`xyo-system` and turns an XML file or string into a tree of managed nodes,
and a tree back into XML text:

- **Read.** `load(fileName, document)` and `loadFromString(text, document)`
  parse markup into a `TPointer<Document>`. They return `bool`; no
  exceptions, no partial documents.
- **A small document model.** A `Document` is a list of `Node`s. A node has a
  `type` (`Element`, `Content`, `CDATA`, `Comment`, `Declaration`,
  `DocumentTypeDefinition`, `Bom`), a `name` (the tag name, or the text of
  the node), `attributes` (an ordered list of `Attribute { name, value }`)
  and a `branch` (its children, again a list of nodes).
- **Query.** `get(name)` (direct children), `find(name)` (all descendants)
  and `findWithAttributeValue(name, attribute, value)` return new
  `Document`s that share the matching nodes with the tree.
- **Write.** `save(fileName, document, mode)` and
  `saveToString(output, document, mode)` write the tree back: as it is
  (`Mode::Normal`, default), minified, or indented with tabs or 4 spaces.
- **Lossless and lightweight.** The reader is a markup tokenizer, not a
  validating parser: text, attribute values, comments, CDATA, declarations
  and `DOCTYPE` are kept as raw text, entities are neither decoded nor
  encoded, and a document read and written in `Mode::Normal` comes back
  almost byte for byte.
- **Safe on hostile input.** Element nesting is limited to 1024 levels, so
  deep documents fail cleanly instead of overflowing the stack. Nothing is
  fetched or expanded: no external entities, no entity expansion, no DTD
  processing.

```
quantum-script--xml, applications ...
file-xml              <-- this library
xyo-system            (File, StringWrite, Shell, ...)
xyo-encoding          (String, UTF)
xyo-multithreading
xyo-data-structures   (TDynamicArray, TDoubleEndedQueue, IRead / IWrite)
xyo-managed-memory    (Object, TPointer, TPointerX, pools)
xyo-platform          (macros, TAtomic, CriticalSection, Thread)
```

## Why use it

- **It is the XML of the XYO stack.** The `xml` extension of Quantum Script
  (`quantum-script--xml`) is built on it, and it ships in the XYO SDK.
- **It speaks the XYO types.** Names and text are `String`, attribute lists
  are `TDynamicArray`, child lists are `TDoubleEndedQueue`, nodes are
  `Object`s held by `TPointer` / `TPointerX`. No conversion layer between the
  parser and your code.
- **Round trip preserving.** Comments, processing instructions, `DOCTYPE`
  (with its internal subset), CDATA, whitespace, entity references and the
  order of attributes are all kept, so a load / change / save cycle of a
  configuration file produces a minimal diff.
- **Lenient.** Besides well-formed XML it reads markup that strict parsers
  refuse: several top level elements, text at the top level, a raw `&` or
  `>` in text. Element and attribute names are not validated; `x:item` is
  simply a name.

What it does **not** do: decode `&amp;`, `&lt;`, `&#65;` ... (the text keeps
them as written), escape text on output, process namespaces, validate
against a DTD or schema, run XPath queries, or read UTF-16 / UTF-32 files.
Bytes are passed through unchanged, so UTF-8 and other ASCII compatible
encodings work.

## Concepts at a glance

| Need | Use | Notes |
|------|-----|-------|
| Parse a file | `load("file.xml", document)` | `TPointer<Document> document`; `false` on I/O or syntax error, `document` becomes `nullptr` |
| Parse text | `loadFromString(text, document)` | 0 terminated `const char *` |
| Parse for data access | `load("file.xml", document, Mode::Minified)` | drops whitespace only text between elements, trims text |
| Write a file | `save("file.xml", document, Mode::IndentationTab)` | default `Mode::Normal` writes the tree as it is |
| Write to a string | `saveToString(output, document)` | replaces `output`; unchanged on error |
| Top level elements named `x` | `document->get("x")` | a new `Document`, `false` when nothing matched |
| Go into an element | `Document children(node->branch);` | a view of the element's children |
| All elements named `x`, any depth | `document->find("x")` | document order |
| Element by attribute | `document->findWithAttributeValue("x", "id", "7")` | exact, case sensitive |
| Node `k` of a list | `list.getIndex(k)` | `TPointer<Node>`, `nullptr` past the end; counts text nodes too |
| What is this node? | `node->type` | `NodeType::Element`, `Content`, `CDATA`, `Comment`, ... |
| Tag name / text | `node->name` | for text nodes `name` holds the text |
| Attribute `k` | `node->attributes->index(k)->name`, `->value` | `attributes` may be `nullptr` on built nodes |
| New node | `TPointer<Node> node; node.newMemory(); node->type = NodeType::Element; node->name = "x";` | |
| Append a node | `list.add(node)` | returns the view of `node`'s children, to add to them |

## Contents

| Document | What it covers |
|----------|----------------|
| [Getting started](getting-started.md) | Build it, depend on it, first program, threads, building without fabricare |
| [Document model](document-model.md) | `Document`, `Node`, `Attribute`, navigating, querying, building and changing documents, memory rules, pitfalls |
| [Reading XML](reading.md) | `load` / `loadFromString`: accepted syntax, what each construct becomes, modes, errors, limits |
| [Writing XML](writing.md) | `save` / `saveToString`: modes, exact output format, escaping, round trip |
| [API reference](reference.md) | Every public symbol on one page |

The memory model (`Object`, `TPointer`, `TPointerX`, pools, threads) is
documented in the `xyo-managed-memory` repository, `docs/`; the containers
(`TDynamicArray`, `TDoubleEndedQueue`) in the `xyo-data-structures`
repository, `docs/`; `String` in the `xyo-encoding` repository, `docs/`.

## Source map

```
source/XYO/FileXML.hpp                   umbrella header, include this
source/XYO/FileXML.Amalgam.cpp           the whole library in one translation unit
source/XYO/FileXML/
    Dependency.hpp                       xyo-system, export macros
    Attribute.hpp                        Attribute (name, value)
    Node.hpp                             Node, NodeType, Node::Attributes, Node::Branch
    Document[.cpp]                       Document: list of nodes, add, get, find, index access
    Mode.hpp                             read / write modes
    Input[.cpp]                          buffered character input over an IRead or memory (internal)
    Whitespace.hpp                       text trimming and mixed content rules (internal)
    Reader[.cpp]                         load, loadFromString
    Writer[.cpp]                         save, saveToString
    Copyright / License / Version        library metadata
input/test.01.xml                        sample document used by test.01
input/test.02.xml                        sample document used by test.02
test/test.01.cpp                         load a file, save it in the four modes
test/test.02.cpp                         load and save a small file
test/test.03.cpp                         reader edge cases and errors, writer output per mode, queries, depth, large files
```

## AI assistant skill

A Claude Code skill describing how to use this library lives in
[`.claude/skills/file-xml/`](../.claude/skills/file-xml/SKILL.md).
It is picked up automatically inside this repository; copy the folder to
`~/.claude/skills/` to have it available in the projects that depend on
`file-xml`.
