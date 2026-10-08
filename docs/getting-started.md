# Getting started

## 1. Build and install

The library is built with [fabricare](https://github.com/g-stefan/fabricare),
the build tool used by all XYO C++ projects. `xyo-platform`,
`xyo-managed-memory`, `xyo-data-structures`, `xyo-multithreading`,
`xyo-encoding` and `xyo-system` must be installed to the SDK first. From the
repository root:

```bash
fabricare make       # build into output/
fabricare test       # build and run test/test.*.cpp (run make first)
fabricare install    # copy output/{bin,include,lib} to ~/.fabricare/<platform>
fabricare clean      # remove output/ and temp/
```

On Windows run them from a shell where the Visual Studio C++ tools are set
up (`vcvarsall.bat x64`).

The project `file-xml` is built with `"make": "dll-or-lib"`:

| Platform | Result |
|----------|--------|
| dynamic, for example `win64-msvc-2026` | DLL / shared library (`file-xml.dll` + `file-xml.lib`) |
| static, for example `win64-msvc-2026.static` | static library; the platform sets `XYO_PLATFORM_COMPILE_STATIC`, so the export macros are empty |

The whole library is compiled; only the small inline members of
`Attribute`, `Node` and `Document` are header only.

## 2. Depend on it from another fabricare project

In the consumer's `fabricare.json`:

```json
{
	"name": "my-application",
	"make": "exe",
	"sourcePath": "XYO/MyApplication",
	"dependency": [
		"file-xml"
	]
}
```

The same name works on dynamic and static platforms. `xyo-system` and
everything below it come in as transitive dependencies.

## 3. Include

```cpp
#include <XYO/FileXML.hpp>
```

The umbrella header pulls in `<XYO/System.hpp>` (and through it the
encoding, multithreading, data structures, managed memory and platform
headers) and every public header of this library. C++17 or newer is
required, as for the rest of the XYO stack.

Namespace `XYO::FileXML` contains `using namespace` for
`XYO::ManagedMemory`, `XYO::DataStructures`, `XYO::Encoding` and
`XYO::System`, so `using namespace XYO::FileXML;` also brings in `String`,
`TPointer`, `TMemory`, `TDynamicArray`, `File`, ... The metadata namespaces
(`Version`, `Copyright`, `License`) then exist several times: write
`XYO::FileXML::Version::version()` in full.

`XYO::FileXML::Node` is a different type from the list node
`TDoubleEndedQueue<...>::Node` (spelled `Node::Branch::Node`) you meet when
walking a child list; see [Document model](document-model.md#walking-a-list).

In a library or application of your own, either use the namespace or
qualify (`FileXML::load`, `FileXML::Node`), as the XYO tools do:

```cpp
#include <XYO/FileXML.hpp>

namespace XYO::MyApplication {
	using namespace XYO::FileXML;
};
```

## 4. First program

Parse a document, read elements and attributes, add an element, print it and
save it.

```cpp
#include <XYO/FileXML.hpp>

using namespace XYO::FileXML;

// Text of an element: its Content and CDATA children, joined
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

static TPointer<Node> newNode(NodeType type, const char *name) {
	TPointer<Node> node;
	node.newMemory();
	node->type = type;
	node->name = name;
	return node;
};

int main(int, char *[]) {
	TPointer<Document> xml;
	if (!loadFromString(
	        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
	        "<menu>\n"
	        "  <food id=\"1\"><name>Waffles</name><price>5.95</price></food>\n"
	        "  <food id=\"2\"><name>French Toast</name><price>4.50</price></food>\n"
	        "</menu>\n",
	        xml, Mode::Minified)) {
		printf("invalid XML\n");
		return 1;
	};

	TPointer<Node> menu = xml->get("menu").getIndex(0);
	if (!menu) {
		printf("no <menu> element\n");
		return 1;
	};

	Document menuItems(menu->branch);
	Document foods = menuItems.get("food");
	for (size_t k = 0; k < foods.length(); ++k) {
		TPointer<Node> food = foods.getIndex(k);
		Document fields(food->branch);
		String id;
		getAttribute(food, "id", id);
		printf("%s: %s, %s\n", id.value(),
		       getText(fields.get("name").getIndex(0)).value(),
		       getText(fields.get("price").getIndex(0)).value());
	};

	// <food id="3"><name>Pancakes &amp; Syrup</name></food>
	TPointer<Node> food = newNode(NodeType::Element, "food");
	TPointer<Attribute> attribute;
	attribute.newMemory();
	attribute->name = "id";
	attribute->value = "3";
	food->attributes.newMemory();
	food->attributes->push(attribute);
	Document fields = menuItems.add(food);
	fields.add(newNode(NodeType::Element, "name")).add(newNode(NodeType::Content, "Pancakes &amp; Syrup"));

	String text;
	if (!saveToString(text, xml, Mode::Indentation4Spaces)) {
		return 1;
	};
	printf("%s", text.value());

	if (!save("menu.xml", xml, Mode::IndentationTab)) {
		printf("cannot write menu.xml\n");
		return 1;
	};
	return 0;
};
```

Output (the indented part has CRLF line endings):

```
1: Waffles, 5.95
2: French Toast, 4.50
<?xml version="1.0" encoding="UTF-8"?>
<menu>
    <food id="1">
        <name>Waffles</name>
        <price>5.95</price>
    </food>
    <food id="2">
        <name>French Toast</name>
        <price>4.50</price>
    </food>
    <food id="3">
        <name>Pancakes &amp; Syrup</name>
    </food>
</menu>
```

Things to notice, all explained in [Document model](document-model.md):

- the document is loaded with `Mode::Minified`, which drops the whitespace
  only text between elements; with the default `Mode::Normal` those text
  nodes stay in the tree and are counted by `length()` / `getIndex()`;
- `get` searches only the list it is called on: to go one level down, wrap
  the element's children in a `Document` (`Document menuItems(menu->branch)`);
- every lookup can fail: `getIndex` returns `nullptr` when nothing matched,
  and the helpers above accept `nullptr`;
- text is raw: `&amp;` is not decoded on read nor produced on write; the new
  text node above is written already escaped;
- `add` returns the view of the new node's children, so elements can be
  built in one chain;
- pass `String`s to `printf` with `.value()`.

As a fabricare test project:

```json
{
	"name": "test.04",
	"make": "exe",
	"category": "test",
	"SPDX-License-Identifier": "Unlicense",
	"dependency": [
		"file-xml"
	]
}
```

with the source in `test/test.04.cpp`.

## 5. Threads

Everything in `xyo-managed-memory` about threads applies here:

- **a document belongs to the thread that created it.** `Document`, `Node`
  and `Attribute` come from per-thread active pools and are reference
  counted with plain integers. Do not share a document between threads, and
  do not release one on another thread. To hand a document to another
  thread, pass its text (`saveToString`, then copy the characters into a
  `std::string` or a `TMemorySystem` object) and `loadFromString` it on the
  receiving thread;
- create threads and move data between them with **`xyo-multithreading`**;
- the main thread must use the library before any thread starts. Call
  `XYO::ManagedMemory::Registry::registryInit()` at the start of `main` when
  in doubt. The pools are created on first use;
  `TMemory<Document>::initMemory()` (which also creates the node and list
  pools) and `TMemory<Attribute>::initMemory()` create them up front in the
  current thread.

`load`, `loadFromString`, `save` and `saveToString` have no shared state;
different threads can parse and write their own documents at the same time.

## 6. Building without fabricare

Compile the seven amalgams with your sources:

1. Put `source/` of `xyo-platform`, `xyo-managed-memory`,
   `xyo-data-structures`, `xyo-multithreading`, `xyo-encoding`,
   `xyo-system` and `file-xml` on the include path.
2. Provide the configuration headers of `xyo-platform`, `xyo-managed-memory`
   and `xyo-system` (see their documentation; for the default configuration
   copy each `Config.Template.hpp` to `Config.hpp`).
3. Compile `Platform.Amalgam.cpp`, `ManagedMemory.Amalgam.cpp`,
   `DataStructures.Amalgam.cpp`, `Multithreading.Amalgam.cpp`,
   `Encoding.Amalgam.cpp`, `System.Amalgam.cpp` and `FileXML.Amalgam.cpp`
   together with your sources, and define `XYO_PLATFORM_LIBRARY`,
   `XYO_MANAGEDMEMORY_LIBRARY`, `XYO_DATASTRUCTURES_LIBRARY`,
   `XYO_MULTITHREADING_LIBRARY`, `XYO_ENCODING_LIBRARY`,
   `XYO_SYSTEM_LIBRARY` and `XYO_FILEXML_LIBRARY` everywhere (plain static
   linking).
4. Link `pthread` on Linux; `user32` on Windows (pulled in by a `#pragma`
   with MSVC).

Example on Linux, with the repositories side by side:

```bash
g++ -std=c++17 \
    -Ixyo-platform/source -Ixyo-managed-memory/source \
    -Ixyo-data-structures/source -Ixyo-multithreading/source \
    -Ixyo-encoding/source -Ixyo-system/source -Ifile-xml/source \
    -DXYO_PLATFORM_LIBRARY -DXYO_MANAGEDMEMORY_LIBRARY \
    -DXYO_DATASTRUCTURES_LIBRARY -DXYO_MULTITHREADING_LIBRARY \
    -DXYO_ENCODING_LIBRARY -DXYO_SYSTEM_LIBRARY -DXYO_FILEXML_LIBRARY \
    xyo-platform/source/XYO/Platform.Amalgam.cpp \
    xyo-managed-memory/source/XYO/ManagedMemory.Amalgam.cpp \
    xyo-data-structures/source/XYO/DataStructures.Amalgam.cpp \
    xyo-multithreading/source/XYO/Multithreading.Amalgam.cpp \
    xyo-encoding/source/XYO/Encoding.Amalgam.cpp \
    xyo-system/source/XYO/System.Amalgam.cpp \
    file-xml/source/XYO/FileXML.Amalgam.cpp \
    main.cpp -o main -pthread
```

On Windows with MSVC, also define `XYO_PLATFORM_COMPILE_STATIC`:

```bat
cl /EHsc /std:c++17 ^
   /Ixyo-platform\source /Ixyo-managed-memory\source ^
   /Ixyo-data-structures\source /Ixyo-multithreading\source ^
   /Ixyo-encoding\source /Ixyo-system\source /Ifile-xml\source ^
   /DXYO_PLATFORM_COMPILE_STATIC /DXYO_PLATFORM_LIBRARY /DXYO_MANAGEDMEMORY_LIBRARY ^
   /DXYO_DATASTRUCTURES_LIBRARY /DXYO_MULTITHREADING_LIBRARY ^
   /DXYO_ENCODING_LIBRARY /DXYO_SYSTEM_LIBRARY /DXYO_FILEXML_LIBRARY ^
   xyo-platform\source\XYO\Platform.Amalgam.cpp ^
   xyo-managed-memory\source\XYO\ManagedMemory.Amalgam.cpp ^
   xyo-data-structures\source\XYO\DataStructures.Amalgam.cpp ^
   xyo-multithreading\source\XYO\Multithreading.Amalgam.cpp ^
   xyo-encoding\source\XYO\Encoding.Amalgam.cpp ^
   xyo-system\source\XYO\System.Amalgam.cpp ^
   file-xml\source\XYO\FileXML.Amalgam.cpp ^
   main.cpp
```

With an installed static SDK (`~/.fabricare/<platform>.static`) you can
instead compile only your sources against its `include/` and link its
libraries (`file-xml`, `xyo-system`, `xyo-multithreading`, `xyo-encoding`,
`xyo-data-structures`, `xyo-managed-memory`, `xyo-platform`) with the static
CRT (`/MT`) and `XYO_PLATFORM_COMPILE_STATIC` defined.
