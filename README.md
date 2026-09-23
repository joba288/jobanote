# jobanote
jobanote is a lightweight ODT document editor built from the ground up in C.  For rendering, this project uses my custom graphics library jbgl.

## Features
- Custom document data structure
- Paragraph and text-node representation
- Cursor movement and text editing
- Text selection (shift + arrow keys)
- Styled text
- - Bold
- - Italic
- - Underline
- - Strikethrough
- - Text colour
- OpenDocument Text (.odt) import
- OpenDocument Text (.odt) export
- XML parsing and generation using libxml2
- ZIP archive handling using miniz

## Example

![](https://github.com/joba288/jobanote/blob/main/jobanote_example_img.PNG)

## Project Structure
```text
jobanote/
├── resources/
│   ├── documents/
│   │   ├── test.odt
│   │   └── test2.odt
│   │
│   ├── fonts/
│   │   ├── AppleGaramond.ttf
│   │   ├── arial.ttf
│   │   └── MatchaMint.ttf
│   │
│   ├── shaders/
│   │   ├── fsDefaultBatch.glsl
│   │   └── vsDefaultBatch.glsl
│   │
│   └── textures/
│
├── src/
│   ├── document_renderer.c
│   ├── document_renderer.h
│   ├── jbgl.c
│   ├── jbgl.h
│   ├── jobanote.c
│   ├── jobanote.h
│   ├── main.c
│   ├── odt.c
│   ├── odt.h
│   ├── odt_styles.c
│   └── odt_styles.h
│
├── vendor/
│   ├── cglm/
│   ├── glad/
│   ├── miniz/
│   ├── nativefiledialog/
│   └── stb/
│
├── .gitignore
├── CMakeLists.txt
└── README.md
```
## Building
### Windows
Clone the repository:
```
git clone https://github.com/joba288/jobanote.git
cd jobanote
```
Create a build directory:
```
mkdir build
cd build
```
Configure the project:
```
cmake ..
```
Build:
```
cmake --build .
```

The project currently loads a test document from:
```
resources/documents/test.odt
```

## Testing

Includes automated tests for the core document and text-editing functionality.
The tests use a small custom test framework.

### Building the Tests

The tests are built automatically by CMake

### Running the Tests

Run the complete test suite with:

```
ctest --test-dir build --output-on-failure
```

## Still to Add
This project is still work in progress. Features still to be added include:
- A complete GUI
- Loading and saving files at any point
- Copy, Paste and Cut
- Undo / Redo
- Fix problems with word wrapping
- Searching for fonts based on fontname in style
- Fix bugs brought to light by testing
