一级标题
===
二级标题
---

<h1>dffefe</h1>
<a href="1+2>5">dwwdd</a>

#一级标题无空格
# 一级标题
## 二级标题
### 三级标题
#### 四级标题
##### 五级标题
###### 六级标题

## 分割线

---

***

## 合法的样式

这是*斜体*，这是**粗体**，这是`代码`，这是***粗斜体***。

## 跨行的样式（无需支持）

这是*斜
体*，
这是**粗
体**，
这是`代
码`，
这是***粗斜
体***。

## 不对称的非法样式（不崩溃即可，建议当纯文本处理)

*这是斜体**

这是*粗体**

这是`代码``

**这是斜体*

这是**粗体*

这是``代码`


## 样式特殊字符单独出现

`
*
**`**
*`*
`*`
`**`

## 图片和超链接

百度一下，你就住院![](https://baidu.com/favicon.ico)[Baidu](https://baidu.com)

## 含特殊URL的超链接

[带括号的url](https://msdn.microsoft.com/en-us/library/windows/desktop/ms646268(v=vs.85).aspx)

[相对路径](blob/master/test.md)

[/相对路径](/blob/master/test.md)

[./相对路径](./blob/master/test.md)

[../相对路径](../blob/master/test.md)

[#](#)

## 不合法的图片和超链接

百度一下，你就住院[](https://baidu.com)

百度一下，你就住院![](https://bai`du`.com/favicon.ico)[Baidu](https://bai**du**.com)

百度一下，你就住院`!`[](https://baidu.com/favicon.ico)**[*Baidu*](https://baidu.com)**

## 带样式的图片和超链接

百度一下，你就住院`!`[](https://baidu.com/favicon.ico)**[*Baidu*](https://baidu.com)**

## 带超链接的图片

百度一下，你就住院[![](https://baidu.com/favicon.ico)](https://baidu.com)

## 列表（可单列，可组合）

+ 文本1

* 文本1

- 文本1

+ 文本1
* 文本2
- 文本3
+ 文本4
* 文本5
- 文本6

## 无需支持嵌套列表

- 文本1
- 文本2
- 文本3
1. 文本1
2. 文本2
3. 文本3

- 文本1
6666
- 文本2
6666
1. 文本1
6666
2. 文本2

## 列表强制从1开始计数

9. 文本9
10. 文本10
11. 文本11
100. 文本100

## 不合法列表（作为纯文本显示）

+文本1

*文本1

-文本1

## 引用和多行引用

> 这是引用

>这是不带空格的引用

> 这是引用1
> 这是引用2

> 这是引用
这一行也是引用

## 引用的嵌套（空行表示引用的结束）

> 这是引用
这一行也是引用
>> 这一行是嵌套引用
这一行在嵌套引用内
> 这一行在嵌套引用内
>> 这一行在嵌套引用内

> 这一行在引用内
这一行在引用内

这一行不在引用内

## 龟派气功

>>>>>>>>>>>>>>>>>>》

## 引用和列表的组合

>
- 文本1
- 文本2

>a
- 文本1
- 文本2

## 全部组合

>这是引用
这一行**也是**引用
- 文本1[![](https://baidu.com/favicon.ico)](https://baidu.com)
6666
- 文本2
[![](https://baidu.com/favicon.ico)](https://baidu.com)
1. 文本1
6666
2. 文本2

## 代码块及特殊字符

```cpp
#include <iostream>
int main(){
    // hello,world
}
```

```cpp
#include <iostream>
int ```main```(){
    // hello,`world`
}
```

## Github 中代码块的结束必须另起一行

```cpp
#include <iostream>
<<<<<<< HEAD
int main(){// hello,world}
=======
int main(){// hello,world}```
>>>>>>> ad69c082caf903bc11dbad1048a0a7c50dfcd26b
```

## 表格

Item     | Value
-|-
Computer | $1600
Phone    | $12

Item|Value
-|-

|    Item     |    Value      |
| :---------- | ------------: |
| Computer    |    $1600      |
| Phone       |    $12        |

| **Tables**  | *Are*         | Cool  |
| :---------- | :-----------: | ----: |



## 非法表格

| **Tables**    | *Are*         | Cool  |

Computer | $1600
Phone    | $12
-|-

**
Item     | Value
-|-
Computer | $1600
**

## 表格、引用、样式无法结合

>无法|结合
-|-

>
无法|结合
-|-

>a
无法|结合
-|-

`无法|结合
-|-


`无法|结合`
-|-

*无法|结合*
-|-

**无法|结合**
-|-

## HTML 支持

<script>alert(1)</script>

<h4>4级标题</h4>

<hr>

<button onclick="alert(1)">确定</button>

## 自动转义

1 + 1 < 3

<>

<<>>

123|45677777
-|-:
**789**|456
