# MyNotePad

![logo](static/MyNotePad.ico)

## Synopsis

A markdown editor that supports exporting HTML.

This program is initially written in pure win32 APIs just like [this](https://msdn.microsoft.com/en-us/library/windows/desktop/ms646268(v=vs.85).aspx). Later it's rewritten using wxWidgets, you can build it on Windows and Linux. 

Download binaries: [Release](../../releases)

Try this markdown file: [test.md](test.md)

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

### Compile on Windows

> Notice: No longer support 32-Bit (x86) Windows.

1. Download [CMake](https://cmake.org/download/) and [wxWidgets](http://www.wxwidgets.org/downloads/) source code or binaries. If you use wxWidgets binaries, download 3 archives:

- Header Files (`include` folder)
- **64-Bit** Development Files (`lib/vc141_dll` folder)
- **64-Bit** Release DLLs (`lib/vc141_dll` folder)

2. Extract them to the same folder and merge the `lib/vc141_dll` folder. For example, the directory tree is like:

```
D:\
|----wxWidgets-3.1.3
     |---------------include
                     |--------msvc
                     |--------wx
     |---------------lib
                     |--------vc14x_x64_dll
                              |-------------mswu
                              |-------------mswud
                              |-------------wxbase31u.lib
                              ......
                              |-------------wxrc.exe
                              ......
```
See [CMakeLists.txt](CMakeLists.txt) for more detail.

3. Then open CMake-GUI, click `Browse Source` and `Browse Build` to choose a correct place.

4. Click `Configure`, set `wxWidgets_ROOT_DIR` and `wxWidgets_LIB_DIR` to the wxWidgets library folder. For the instance above, set `wxWidgets_ROOT_DIR` to `D:\wxWidgets-3.1.3\` and set `wxWidgets_LIB_DIR` to `D:\wxWidgets-3.1.3\lib\vc14x_x64_dll`.

5. Click `Configure` again, then click `Generate`, finally we get a Visual Studio solution. 

### Options

Use `cmake -D<VAR>=<VALUE>` to change any option.

| Option | Default | Description |
| - | - | - |
| `CMAKE_BUILD_TYPE` | Debug | Build Type for Unix (Release / Debug) |

## Known bugs

1. (Linux)Texts copied to clipboard will disappear after the window closed
2. (Linux)Cannot drop a file to the window to open it
3. (Linux)Fail to open/save file in non-ascii path 

## TODO

- undo/redo
- set font size
- scrollbar
- TAB key
- search & replace
- encode conversion(only support utf-8 currently)
- instant highlight

## Programming Guide

See our [wiki](./wiki) page.
