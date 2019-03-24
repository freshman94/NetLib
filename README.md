# NetLib: High Performance Net Libarary in C++

## Introduction

本项目是仿照陈硕先生的muduo库实现的基于Reactor模型的多线程网络库，其中还实现了异步日志的功能，并且还实现了一个简洁的HTTP静态服务器，实现静态资源的访问，支持HTTP长连接。

## Environment

- OS : Ubuntu 16.04
- Compiler: g++5.4.0
- Build: cmake 3.5.1

## Tutorial

[Linux多线程服务端编程学习（一）：多线程日志库的设计](https://blog.csdn.net/qq_22660775/article/details/88703355)

[Linux多线程服务端编程学习（二）：封装互斥量与线程](https://blog.csdn.net/qq_22660775/article/details/88725019)

[Linux多线程服务端编程学习（三）：非阻塞网络编程中应用层Buffer的必需性](https://blog.csdn.net/qq_22660775/article/details/88728723)

[Linux多线程服务端编程学习（四）：muduo库的网络模型与实现原理](https://blog.csdn.net/qq_22660775/article/details/88737551)

[Linux多线程服务端编程学习（五）：多线程HTTP静态服务器](https://blog.csdn.net/qq_22660775/article/details/88769593)

## Build

`./build.sh`

会在当前目录生成build文件夹，可执行文件在build/release/bin目录下，库文件在build/release/lib目录下

静态服务器的可执行文件为http，其运行命令为：

`./http   [n]`

其中n表示线程池中线程的数量，默认为0（不包含主线程）

在浏览器中输入http://192.168.6.115:8000/，即可访问该静态服务器。

`./build.sh install`

会在目录build/release-install/include中包含代码的所有头文件

## More

后续将会对该网络库进行压力测试

