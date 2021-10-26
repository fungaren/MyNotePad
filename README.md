# MyNotePad

![logo](static/mynotepad.ico)

## Synopsis

A markdown editor that supports exporting HTML.

This program is initially written in pure win32 APIs just like [this](https://msdn.microsoft.com/en-us/library/windows/desktop/ms646268(v=vs.85).aspx). Later it's rewritten using wxWidgets, you can build it in Windows and Linux.

Download binaries: [Release](../../releases)

Try this markdown file: [test.md](test.md)

## Compiling instructions

### Linux

```bash
sudo apt update
sudo apt install build-essential cmake git
# Debian 9
sudo apt install libwxgtk3.0-dev
# Debian 10
sudo apt install libwxgtk3.0-gtk3-dev
git clone https://github.com/mooction/mynotepad
mkdir mynotepad/build
cd mynotepad/build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
sudo make install
```

That's all. Run `mynotepad` to launch the program.

### Windows

> Notice: No longer support 32-Bit (x86) Windows.

1. Download [CMake](https://cmake.org/download/) and [wxWidgets](http://www.wxwidgets.org/downloads/) source code or binaries. If you use wxWidgets binaries, download 3 archives:

- Header Files (`include` folder)
- **64-Bit** Development Files (`lib/vc14x_x64_dll` folder)
- **64-Bit** Release DLLs (`lib/vc14x_x64_dll` folder)

2. Extract them to the same folder and merge the `lib/vc14x_x64_dll` folder. For example, the directory tree is like:

```text
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

4. Click `Configure`, specify generator for this project. If you are using Visual Studio, then choose `Visual Studio 15 2017 Win64` or other version, but `Win64` is required.

5. Set `wxWidgets_ROOT_DIR` and `wxWidgets_LIB_DIR` to the wxWidgets library folder. For the instance above, set `wxWidgets_ROOT_DIR` to `D:\wxWidgets-3.1.3\` and set `wxWidgets_LIB_DIR` to `D:\wxWidgets-3.1.3\lib\vc14x_x64_dll`.

6. Click `Configure` again, then click `Generate`, finally we get a Visual Studio solution.

### Options

Use `cmake -D<VAR>=<VALUE>` to change any option.

| Option | Default | Description |
| - | - | - |
| `CMAKE_BUILD_TYPE` | Debug | Build Type for Unix (Release / Debug) |

## Known issue

1. (Linux) Copied text will disappear after the window closed

## TODO

- Search & replace
- Encode conversion (only support utf-8 currently)

## Programming Guide

See [wiki](../../wiki) page.
