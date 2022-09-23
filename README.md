# MyNetWorkCode

## 简介
> 本仓库主要是自己的网络编程学习代码   

***一、环境***   
1、Ubuntu 18.04   
2、VSCode   
3、CMake 3.10   
4、G++ 7.5.0   

***二、主要内容***    
1、学习盛延敏老师的网络编程实战课程，跟进完成每一章的编程内容；    
2、在学习中解决一些遇到的问题，由于自己用的C++，所以要解决一些与C代码的相互调用问题，比如C++调用C的函数或者库时需要extern "C"{相关内容}等；
  ```c++
  extern "C"
  {
    #include "common.h"
  }
  ```
  3、学习和掌握CMake的基本使用知识；    
  4、复习网络编程相关知识。    

***三、代码运行方式***    
本仓库已经将课程中用到的库编译完成，核心头文件也包含完毕，只需进入相关章节文件夹运行命令即可，比如：    
```bash
AA@AA:~/code/xxx$ cd chap-7
AA@AA:~/code/xxx/chap-7$ mkdir build && cd build
AA@AA:~/code/xxx/chap-7/build$ cmake ..
AA@AA:~/code/xxx/chap-7/build$ make
```
等待编译完成即可，然后运行可执行程序：
```bash
AA@AA:~/code/xxx/chap-7/build$ ./可执行文件名
```
