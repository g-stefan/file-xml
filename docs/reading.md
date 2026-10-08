# Reading XML

```cpp
bool load(const char *fileName, TPointer<Document> &document, Mode mode = Mode::Normal);
bool loadFromString(const char *value, TPointer<Document> &document, Mode mode = Mode::Normal);
```

Both parse one complete text into a `Document` (see
[Document model](document-model.md)).

- On success they return `true` and `document` holds the top level list.
- On any error (file cannot be opened, syntax error, nesting too deep,
  `fileName` / `value` is `nullptr`, empty input) they return `false` and
  set `document` to `nullptr`. A partially parsed document is never
  returned.
- `fileName` is a UTF-8 path, opened with `XYO::System::File::openRead`. The
  file is streamed through a 64 KB buffer; its text is not loaded into
  memory first.
- `loadFromString` reads a 0 terminated string in place, without copying
  it. A `String` converts implicitly; the text ends at its first 0 byte.
- The whole document is built in memory.

```cpp
TPointer<Document> xml;
if (!load("settings.xml", xml, Mode::Minified)) {
	printf("settings.xml: missing or invalid\n");
	return false;
};
TPointer<Node> settings = xml->get("settings").getIndex(0);
if (!settings) {
	printf("settings.xml: a <settings> element is expected\n");
	return false;
};
```

There is no error position or message: the result is valid or not. When a
user needs to locate an error, check the file with another tool.

## What the reader is

A small, non-validating markup reader. It splits the text into elements,
text, comments, CDATA sections, processing instructions / declarations and
`DOCTYPE`, checks that every element is closed by the matching end tag, and
keeps everything else as raw text. In particular:

- **entities and character references are not decoded** (`&amp;`, `&lt;`,
  `&#65;`, `&#x41;`, `&nbsp;` stay as written, in text and in attribute
  values); a raw `&` is accepted;
- **names are not validated**: an element or attribute name is any run of
  characters up to whitespace, `=`, `/` or `>`; `x:item` is just a name,
  `xmlns` attributes are ordinary attributes;
- **the XML declaration is not interpreted**: `encoding="..."` is ignored,
  the bytes are kept as they are (UTF-8 and other ASCII compatible
  encodings work; UTF-16 / UTF-32 files do not);
- **the `DOCTYPE` is not processed**: no entity definitions, no external
  DTD, no default attributes. Nothing is fetched, nothing is expanded.

## Accepted syntax

**Top level.** Any sequence of nodes: several elements, text, comments,
declarations; nothing is required. Whitespace only input is a valid
document (one `Content` node, or an empty list with `Mode::Minified`).
Empty input is an error.

**Byte order mark.** A UTF-8 BOM (`EF BB BF`) at the very start becomes a
`Bom` node, the first of the document.

**Text.** Everything up to the next `<` is one `Content` node, as it is:
`>` and `&` are allowed, line breaks are kept (CR LF is not normalized).

**Elements.**

- Start tag: `<` immediately followed by the name, then attributes, then
  `>` or `/>`. Whitespace (space, tab, CR, LF) is allowed before `>` and
  `/>`.
- Attributes: `name="value"` or `name='value'`, optional whitespace around
  `=`, no whitespace required between attributes (`<a x="1"y="2"/>`). The
  value is everything up to the matching quote, raw: it may contain `<`,
  `>`, `&`, the other quote and line breaks. Order and duplicates are kept.
- End tag: `</` followed by exactly the start tag's name, optional
  whitespace, `>`.
- Nesting: up to 1024 levels (`Reader::maxDepth`, internal, not
  configurable).

**Other markup.**

| Input | Node |
|-------|------|
| `<?` ... `?>` | `Declaration`: the XML declaration and every processing instruction |
| `<!--` ... `-->` | `Comment`; `--` inside is accepted |
| `<![CDATA[` ... `]]>` | `CDATA` |
| `<!DOCTYPE` whitespace ... `>` | `DocumentTypeDefinition`; the `>` that ends it is the first one outside quotes and outside an internal subset `[ ... ]` (comments in the subset are skipped too) |

These are accepted anywhere, also inside elements.

Not accepted (each one makes the whole load fail):

| Input | Example |
|-------|---------|
| empty input | `""` |
| `<` not followed by a name, `/`, `?` or `!` | `a < b`, `< a/>`, `<></>` |
| unclosed or mismatched elements | `<a>`, `<a>x`, `<a></b>`, `<a></ab>`, `<ab></a>` |
| an end tag without start tag | `</a>`, `<a></a></b>` |
| unterminated tags | `<a`, `<a x="1"`, `<a/`, `<a></a` |
| attributes without value or quotes | `<a x/>`, `<a b c="1"/>`, `<a x=1/>`, `<a x="1/>` |
| attribute without name | `<a ="1"/>` |
| unknown `<!` markup | `<!x>`, `<!-x-->`, `<!DOCTYPEx>` |
| unterminated comment, CDATA, declaration, `DOCTYPE` | `<!-- x`, `<![CDATA[x]]`, `<?xml`, `<!DOCTYPE x [ ...` |
| nesting deeper than 1024 elements | 1025 nested `<a>` |

## Modes

The `mode` argument decides what happens to text:

| Mode | Effect on reading |
|------|-------------------|
| `Mode::Normal` (default) | every text node is kept exactly, including the whitespace between elements |
| `Mode::Minified` | text is trimmed (space, tab, CR, LF) and whitespace only text is dropped; in **mixed content** (text and elements side by side, like `<p>Hello <b>big</b> world</p>`) one space is kept where the text touched an element, so words do not run together |
| `Mode::IndentationTab`, `Mode::Indentation4Spaces` | same as `Mode::Normal` |

```
<a>\n  <b>  x  </b>\n  <c/>\n</a>

Normal:   a: [Content "\n  ", b: [Content "  x  "], Content "\n  ", c, Content "\n"]
Minified: a: [b: [Content "x"], c]
```

Use `Mode::Minified` to read data (configuration, records); use
`Mode::Normal` to edit a file and save it back with its layout. Comments,
CDATA and attribute values are never changed by the mode.

## Other sources

Only files and C strings are read directly. For other data, collect the
text first, for example with `XYO::System::Shell::fileGetContents` or a
`StringWrite`, then call `loadFromString`.
