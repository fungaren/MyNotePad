# MyNotePad

![logo](static/MyNotePad.ico)

## Synopsis

A MarkDown editor support export HTML file.

This program is initially using pure Win32 APIs just like [this](https://msdn.microsoft.com/en-us/library/windows/desktop/ms646268(v=vs.85).aspx). Later it's rewritten using wxWidget, you can build it on Windows and Linux. 

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
git clone https://github.com/mooction/MyNotePad.git .
cd MyNotePad
cmake .
make
sudo make install
```

That's all. Run `mynotepad` to launch the program.

### Compile On Windows

Download [CMake](https://cmake.org/download/) and [wxWidgets](http://www.wxwidgets.org/downloads/) source code or binaries. If you use wxWidgets binaries, just download:

- Header Files
- Development Files

Extract them to the same folder. For example, NOW in `D:/wxWidgets-3.1.2/` there are two folders: `include` and `lib`. See [CMakeLists.txt](CMakeLists.txt) for more detail.

Then open CMake-GUI, click `Browse Source` and `Browse Build` to choose a correct place. Click `Configure` and `Generate` finally we get a Visual Studio solution.

### Options

Use `cmake -D VAR=VALUE` to change any option.

| Option | Default | Description |
| - | - | - |
| `USE_NATIVE_EDIT_BOX` | ON | Use native or custom implemented edit control |

## Known bugs

1. Texts will disappear after the window closed
2. Applying fontface and color do not work

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

- [CMAKE手册](https://www.zybuluo.com/khan-lau/note/254724)
- [wxWidgets 跨平台 GUI 编程](https://www.ctolib.com/docs/sfile/wxwidgets-book/index.html)
- [wxWidgets对资源文件的引用](https://blog.csdn.net/h19861104/article/details/28701793)
