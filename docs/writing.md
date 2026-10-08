# Writing XML

```cpp
enum struct Mode {
	Normal = 0,
	Minified = 1,
	Indentation4Spaces = 2,
	IndentationTab = 3
};

bool save(const char *fileName, Document *document, Mode mode = Mode::Normal);
bool saveToString(String &output, Document *document, Mode mode = Mode::Normal);
```

`document` is any `Document`: a loaded one (`TPointer<Document>` converts
implicitly), one built on the stack (`&xml`), the view of an element's
children, or a query result.

- `save` creates or truncates `fileName` (UTF-8 path, opened with
  `XYO::System::File::openWrite`) and writes the document.
- `saveToString` **replaces** the content of `output` with the document.
  The text is built in a separate string and assigned only on success: **on
  failure `output` is left unchanged**.
- Both return `false` when `document` (or `fileName`) is `nullptr`, and
  `save` when the file cannot be opened or written; **on failure `save` may
  leave part of the document in the file.** Render with `saveToString` and
  write the text, or write to a temporary file and rename it, when a half
  written file would hurt. There is no structural check: whatever is in the
  tree is written.

```cpp
String text;
if (!saveToString(text, xml, Mode::Minified)) {
	return false;
};
fwrite(text.value(), 1, text.length(), stdout);
```

## Modes

### `Mode::Normal` (default)

Writes the tree as it is: text nodes exactly, no line breaks or indentation
added. A document read with `Mode::Normal` and written with `Mode::Normal`
gives back its source, except for the normalizations listed under
[Round trip](#round-trip).

### `Mode::IndentationTab`, `Mode::Indentation4Spaces`

- every element, comment, CDATA, declaration and `DOCTYPE` on its own line,
  indented by one tab or 4 spaces per level;
- **CRLF** line endings, also after the last node;
- text is trimmed and whitespace only text is dropped, so the indentation
  of the source does not add up;
- an element whose children are only text is written on one line:
  `<name>Waffles</name>`;
- **mixed content** (text and elements side by side) is written on one line,
  as in `Mode::Minified`, so no whitespace is added inside a sentence:
  `<p>Hello <b>big</b> world</p>`;
- a BOM is written with no line break after it.

```xml
<?xml version="1.0"?>
<a>
	<b>x</b>
	<c />
	<!-- d -->
	<e>
		<f />
	</e>
</a>
```

### `Mode::Minified`

No line breaks, no indentation; text trimmed, whitespace only text dropped,
one space kept at the edges of text in mixed content.

```xml
<?xml version="1.0"?><a><b>x</b><c /><!-- d --><e><f /></e></a>
```

The trimming applies to every text node, also inside elements where
whitespace matters (`<pre>`, `xml:space="preserve"`): use `Mode::Normal` for
such documents.

## Nodes

| Node | Written as |
|------|------------|
| `Element` with children | `<name attributes>` children `</name>` |
| `Element` without children (`branch` `nullptr` or empty) | `<name attributes />`, with a space before `/>` |
| `Content` | `name` as it is (trimmed in the minified / indented modes) |
| `CDATA` | `<![CDATA[` `name` `]]>` |
| `Comment` | `<!--` `name` `-->` |
| `Declaration` | `<?` `name` `?>` |
| `DocumentTypeDefinition` | `<!DOCTYPE ` `name` `>` |
| `Bom` | `name` (the 3 BOM bytes) |
| `None`, `nullptr` list entries | nothing |

Attributes are written in order, each as ` name="value"`; `nullptr` entries
are skipped.

## Escaping

The writer does **not** escape text: `Content` nodes, attribute values and
the text of the other nodes are written byte for byte. The only change is
the choice of quotes for attribute values:

| Value contains | Written as |
|----------------|------------|
| no `"` | `x="value"` |
| `"` but no `'` | `x='say "hi"'` |
| both `"` and `'` | `x="it's &quot;q&quot;"` |

Text you put in the tree must already be XML: escape `&` and `<` (and `>`,
`"` in attribute values) yourself, for example with the `escapeText` helper
of [Document model](document-model.md#helpers-worth-writing). A text node
`1 < 2` is written as `1 < 2` and the result does not load again.

## Round trip

For a document read with `Mode::Normal`, `save` / `saveToString` with
`Mode::Normal` reproduces the source byte for byte, except:

| Source | Written |
|--------|---------|
| `<a></a>` | `<a />` |
| `<a/>`, `<a  x="1"/>` | `<a />`, `<a x="1" />` |
| `x='1'` | `x="1"` (single quotes only when the value contains `"`) |
| `x = "1"`, `x="1"y="2"` | `x="1"`, `x="1" y="2"` |
| `<b >`, `</b >` | `<b>`, `</b>` |
| `<!DOCTYPE  note>` | `<!DOCTYPE note>` |

Text, comments, CDATA, declarations, entity references, line endings and
the order of attributes are kept. Every document written by `save` from a
loaded document loads again.
