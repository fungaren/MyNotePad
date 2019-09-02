# MyNotePad

![logo](static/MyNotePad.ico)

## Synopsis

A MarkDown editor support export HTML file.

This program is initially using pure Win32 APIs just like [this](https://msdn.microsoft.com/en-us/library/windows/desktop/ms646268(v=vs.85).aspx). Later it's rewritten using wxWidgets, you can build it on Windows and Linux. 

Download Binaries: [Release](../../releases)

Try this Markdown file: [test.md](test.md)

## Compiling instrctions

### Compile On Linux

For GTK+ 2, install dependency:

`sudo apt install build-essential cmake libwxgtk3.0-dev`

For GTK+ 3:

`sudo apt install build-essential cmake libwxgtk3.0-gtk3-dev`

Then build and install

```bash
git clone https://github.com/mooction/mynotepad
cd MyNotePad
cmake .
make
sudo make install
```

That's all. Run `mynotepad` to launch the program.

### Compile On Windows

Download [CMake](https://cmake.org/download/) and [wxWidgets](http://www.wxwidgets.org/downloads/) source code or binaries. If you use wxWidgets binaries, download 3 archives:

- Header Files (`include` folder)
- Development Files (`lib/vc141_dll` folder)
- Release DLLs (`lib/vc141_dll` folder)

Extract them to the same folder and merge the `lib/vc141_dll` folder. For example, NOW in `D:/wxWidgets-3.1.2/` there are two folders: `include` and `lib`. See [CMakeLists.txt](CMakeLists.txt) for more detail.

Then open CMake-GUI, click `Browse Source` and `Browse Build` to choose a correct place. Click `Configure` and `Generate` finally we get a Visual Studio solution.

### Options

Use `cmake -D<VAR>=<VALUE>` to change any option.

| Option | Default | Description |
| - | - | - |
| `CMAKE_BUILD_TYPE` | Debug | Build Type for Unix (Release / Debug) |
| `USE_NATIVE_EDIT_BOX` | ON | Use native or custom implemented edit control |

## Known bugs

1. (Linux)Texts copied to clipboard will disappear after the window closed
2. (Linux)Cannot drop a file to the window to open it
3. Applying fontface and color doesn’t work

## TODO

- undo/redo
- set font size
- scrollbar
- TAB key
- search & replace
- encode conversion(only support utf-8 currently)
- instant highlight
- Colorful Emoji

## Programming Guide

See our [wiki](./wiki) page.
