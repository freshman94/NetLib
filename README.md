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

[Linux多线程服务端编程学习（六）：基于timerfd实现定时器](https://blog.csdn.net/qq_22660775/article/details/88829789)

[Linux多线程服务端编程学习（七）：TCP客户端的设计](https://blog.csdn.net/qq_22660775/article/details/88832716)

[Linux多线程服务端编程学习（八）：压力测试](https://blog.csdn.net/qq_22660775/article/details/88943107)

## Build

NetLib采用cmake来build system，安装方法为：

`sudo apt-get install cmake`



NetLib 依赖于Boost，安装方法为：

`sudo apt-get install libboost-dev libboost-test-dev`



`./build.sh`

会在当前目录生成build文件夹，可执行文件在build/release/bin目录下，库文件在build/release/lib目录下

静态服务器的可执行文件为http，其运行命令为：

`./http   [n]`

其中n表示线程池中线程的数量，默认为0（不包含主线程）

在浏览器中输入http://192.168.6.115:8000/

即可访问该静态服务器。

`./build.sh install`

会在目录build/release-install/include中包含代码的所有头文件

## Test

添加了对网络库NetLib的压力测试，在一台机器上测试NetLib吞吐量和事件处理效率。

 用ping pong协议测试网络库在单机上的吞吐量。吞吐量测试包括两项测试：

- 单线程测试。客户端和服务端运行在同一台机器，均为单线程，各占一个CPU，测试并发连接数为1/10/100/1000/10000时的吞吐量。
- 多线程测试。并发连接数为100或1000，服务器和客户端的线程数同时设为/1/2/3/4。因为客户端和服务端运行在同一台8核机器上，因些线程数大于4，没有意义。



实验中，我们设置消息的大小为16K。测试的结果为：

- 单线程下，并发连接数为1/10/100/1000/10000时的吞吐量分别为：
   252.263125 MiB/s、2419.2265625 MiB/s、2174.88765625 MiB/s、1008.4684375 MiB/s 、372.551388855 MiB/s。
- 多线程下，并发连接数为100时的吞吐量分别为：
   - 线程数为1：2100.65625 MiB/s
   - 线程数为2：3167.59546875 MiB/s
   - 线程数为3：3851.68140625 MiB/s
   - 线程数为4：3980.46828125 MiB/s
- 多线程下，并发连接数为1000时的吞吐量分别为：
   - 线程数为1：1043.9846875 MiB/s
   - 线程数为2：1155.3471875 MiB/s
   - 线程数为3：1158.59734375 MiB/s
   - 线程数为4：1094.78140625 MiB/s



用“击鼓传花”测试网络库的事件处理效率：

击鼓传花的游戏规则是这样的：有100个人围成一圈，一开始第1个人手里有花，他把花传给右手边的人，那个人再继续把花传全右手边的人，当花转手100次之后游戏停止，记录从开始到结束的时间。

有100个网络连接（用socketpair创建成对的socket），数据在这些连接中顺次传递， 一开始往第1个连接里写一个字节，然后从这个连接的另一头读出这1个字节，再写入第2个连接，然后读出来继续写到第3个连接，直到一共写了100次之后程序停止，记录所有的时间。

程序中提供了三个可选参数：

- -p n	设置连接的数量，即多少人参与游戏
- -a n设置活动连接的数量，即“花”的数量
- -w n设置需要写的连接的数量，即“花”传递多少次。

实验中我们统计了游戏的总时间和事件处理的时间（单位微秒）。对于每个并发数，程序循环25次，刨去第一次的热身数据，算后24次的平均值。

测试结果：

**活动连接数100**

|             参数             | 总时间（微秒） | 事件处理时间（微秒） |
| :------------------------: | :-----: | :--------: |
|    -p 100 -a 100 -w 100    |  1262   |    688     |
|   -p 1000 -a 100 -w 1000   |  4311   |    2486    |
|  -p 10000 -a 100 -w 10000  |  44863  |   26659    |
| -p 100000 -a 100 -w 100000 | 423112  |   239220   |



**活动连接数1000**

|             参数              | 总时间（微秒） | 事件处理时间（微秒） |
| :-------------------------: | :-----: | :--------: |
|   -p 1000 -a 1000 -w 1000   |  5721   |    3137    |
|  -p 10000 -a 1000 -w 10000  |  52766  |   33055    |
| -p 100000 -a 1000 -w 100000 | 489577  |   305221   |

