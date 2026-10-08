# File XML

C++ library
- Read XML files and strings into a tree of managed nodes (`load`, `loadFromString`):
elements, attributes, text, CDATA, comments, declarations, `DOCTYPE`, BOM.
- Lossless and lenient: text and attribute values are kept raw, a document read and
written in `Mode::Normal` comes back almost byte for byte; nesting is limited for hostile input,
no entity expansion, no external DTD.
- Query the tree with `get`, `find` and `findWithAttributeValue`, change it, build new documents.
- Write the tree back (`save`, `saveToString`) as it is, minified, or indented with tabs or 4 spaces.

Built on `xyo-system`; used by `quantum-script--xml`,
the `xml` extension of Quantum Script.

## Documentation

- [Overview](docs/README.md) - purpose and design
- [Getting started](docs/getting-started.md) - build, depend on it, first program, threads
- [Document model](docs/document-model.md) - `Document`, `Node`, `Attribute`: navigate, query, build and change documents, helpers, pitfalls
- [Reading XML](docs/reading.md) - `load` / `loadFromString`: accepted syntax, modes, errors, limits
- [Writing XML](docs/writing.md) - `save` / `saveToString`: modes, output format, escaping, round trip
- [API reference](docs/reference.md)

A Claude Code skill for this library is in
[.claude/skills/file-xml](.claude/skills/file-xml/SKILL.md).

## License

Copyright (c) 2016-2026 Grigore Stefan
Licensed under the [MIT](LICENSE) license.
