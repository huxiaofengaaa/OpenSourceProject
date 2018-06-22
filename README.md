# OpenSourceProject

## Google Test
- 现在的googletest是以前单独的 GoogleTest 和 GoogleMock 两个工程的合并。
- 快速学习 googletest，查看文档 googletest/docs/Primer.md ；
- Google Mock 是Google test 的一个扩展，用来实现C ++ Mock类；快速学习 google mock，查看文档 googlemock/README.md ；


## memwatch
MemWatch由 Johan Lindh 编写，是一个开放源代码 C 语言内存错误检测工具。MemWatch支持 ANSI C，它提供结果日志纪录，能检测双重释放（double-free）、错误释放（erroneous free）、内存泄漏（unfreed memory）、溢出(Overflow)、下溢(Underflow)等等。

从MemWatch的使用可以得知，无法用于内核模块。因为MemWatch自身就使用了应用层的接口，而不是内核接口。但是，对于普通的应用层程序，还是比较有用，并且是开源的，可以自己修改代码实现；

网页名称 | 网址
---------|----------
memwatch主页 | http://www.linkdata.se/sourcecode/memwatch/
源码下载地址 | http://www.linkdata.se/downloads/sourcecode/memwatch/memwatch-2.71.tar.gz
linux下载命令 | wget http://www.linkdata.se/downloads/sourcecode/memwatch/memwatch-2.71.tar.gz

## cJSON

JSON(JavaScript Object Notation, JS 对象简谱) 是一种轻量级的数据交换格式。它基于 ECMAScript (欧洲计算机协会制定的js规范)的一个子集，采用完全独立于编程语言的文本格式来存储和表示数据。简洁和清晰的层次结构使得 JSON 成为理想的数据交换语言。 易于人阅读和编写，同时也易于机器解析和生成，并有效地提升网络传输效率。

- JSON 文件的文件类型是 ".json"
- JSON 文本的 MIME 类型是 "application/json"
- JSON在线解析工具：https://c.runoob.com/front-end/53

## Base64
Base64编码要求把3个8位字节（38=24）转化为4个6位的字节（46=24），之后在6位的前面补两个0，形成8位一个字节的形式。最后转换生成的Base64字符个数，应该为4的倍数，如果不是4的倍数，则在后面添加字符等号=，凑成4的倍数，因此编码后输出的文本末尾可能会出现1或2个‘=’。


