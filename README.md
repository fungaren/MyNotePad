# MyNotePad

## 简介

一个支持 MarkDown 转换 HTML 的文本编辑器，仅一个 exe。

本项目的重点在于自己实现一个文本框。

[微软的例子](https://msdn.microsoft.com/en-us/library/windows/desktop/ms646268(v=vs.85).aspx)

## 已实现的功能 implemented

- 按 F5 快速在浏览器中查看
- 支持黑白两色主题
- 支持行号显示
- 支持 Latex 渲染

## 未实现的功能 not implemented

- 撤销
- 设置字号
- 中英文不同字体
- 鼠标拖动滚动条
- TAB 对齐
- 搜索、替换
- 编码转换（目前仅支持 UTF-8）
- 即时代码着色
- 彩色 Emoji

## 编译

对 GTK+ 2，执行（Deepin 选这个）

`sudo apt install build-essential cmake libwxgtk3.0-dev`

对于 GTK+ 3，执行

`sudo apt install build-essential cmake libwxgtk3.0-gtk3-dev`

## 下载

[Release](../../releases)

## 测试

[test.md](test.md)

## 学习资料

- [CMake 入门实战](https://www.hahack.com/codes/cmake/)
- [wxWidgets 跨平台 GUI 编程](https://www.ctolib.com/docs/sfile/wxwidgets-book/index.html)
- [wxWidgets对资源文件的引用](https://blog.csdn.net/h19861104/article/details/28701793)
