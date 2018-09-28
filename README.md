# Build Process


## Linux / Mac 

### Prerequirements

+ GCC / CLANG 
+ cmake >= 3.1
+ Boost 1.67
+ protobuf 3.6.0 (C++)
+ libusb 1.0.21 + 

### steps

1. git clone https://github.com/KeyboxWallet/keyboxd.git
1. cd keyboxd
1. git submodule update --init --recursive
1. cmake .
1. make

## Windows

### Prerequirements

+ VS2017 (VS2015 should be ok, not tested.)

### steps

1. install [vcpkg](https://github.com/Microsoft/vcpkg)
1. using vcpkg to libusb protobuf boost
1. set environment var vcpkg_root to where vcpkg is installed
1. cmake -DCMAKE_TOOLCHAIN_FILE=D:\src\vcpkg\scripts\buildsystems\vcpkg.cmake .  （replaces with your dir)
1. open .sln file with VS2017, compile
