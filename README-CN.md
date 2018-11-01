# 编译说明


## Linux / Mac 

### 环境要求

+ 编译器 GCC / CLANG 
+ cmake >= 3.1
+ Boost 1.67
+ protobuf 3.6.0 (C++)
+ libusb 1.0.21 + 

### 编译步骤

1. git clone https://github.com/KeyboxWallet/keyboxd.git
1. cd keyboxd
1. git submodule update --init --recursive
1. cmake .
1. make

## Windows

### 环境要求

VS2017 (VS2015也应该可以，没试过)

### 编译步骤

1. git clone https://github.com/KeyboxWallet/keyboxd.git
1. cd keyboxd
1. git submodule update --init --recursive
1. 安装[vcpkg](https://github.com/Microsoft/vcpkg)
1. 使用vcpkg 安装三个包  libusb protobuf boost
1. 命令行指定环境变量 vcpkg_root 为 vcpkg的目录
1. cmake -DCMAKE_TOOLCHAIN_FILE=D:\src\vcpkg\scripts\buildsystems\vcpkg.cmake .  （替换为你的目录!!)
1. 打开VS2017编译项目
