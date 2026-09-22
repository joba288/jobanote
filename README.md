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

## Still to Add
This project is still work in progress. Features still to be added include:
- A complete GUI
- Loading and saving files at any point
- Copy, Paste and Cut
- Undo / Redo
- Fix problems with word wrapping
- Searching for fonts based on fontname in style
