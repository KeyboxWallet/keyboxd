# Introduction

[keybox](https://keybox.magicw.net) is a hardware wallet for cryptocurrencies. It is like a modified version of [trezor](https://trezor.io), but with two main differences:

+ A full touch screen and keyboard on keybox, but not on trezor;
+ Simplified hardware functionalities: it just make signatures and don't know 'high level meaning', i.e. transaction .

keyboxd is a bridge between software wallet and hardware wallet.


````
    hardware wallet <=== usb ====> keyboxd   <==== json rpc ====> software wallet
````



# json rpc api formats

you can check [simple-daemon-client](https://github.com/KeyboxWallet/simple-daemon-client) for examples.

## generic rpc reply 

````
{ 
   "errcode": 0 | 1 | ... ,
   "errmessage": "error message",
   "data": <for diffent api, get different reply>
}
````

## public key 

````
{
   "ver":1
   "curve":"secp256k1",
   "data":"<base64encoded of big endian public key buffer>"
}
````

## signature format


````
{
  "ver":1,
  "curve": "secp256k1",
  "input-hash": "<base64encoded of hash data>",
  "pubkey": "<base64enocded of pubkey>",
  "data": { 
    "R": "<base64encoded of R>"
    "S": "<base64encoded of S>",
    "recovery-param": 0 | 1 | 2 | 3 
  }
}
````

the 'big endian public key buffer' is a 'XY' point without 0x04 prefix. 

## path format

````
 "bip32/m/44'/1'/1'/1/1"
````

bip32 is the only supported prefix now.


## hash format

````
"<base64 encoded of hash>"
````

## Signature request

````
{
   "ver": 1,
   "path": "path String defined above",
   "hash": "hash String defined above",
   "options": {
     "rfc6979": true | false,
     "graphene_canonize": true | false
   }
} 
````

## Multiply request

````
{
   "ver": 1,
   "path": "path String defined above",
   "pubkey": "<base64enocded of pubkey>",
} 
````



## RPC List



+ returns ReqResul when call server from client
+ returns void when notification

| function    |   rpc spec       |   extra info  |
|------|------------|----------------------------------|
| Get keybox Daemon version | ReqResult getServerVersion()         |                           |
| list connected devices             | ReqResult getDeviceList()           |   |
| connect Device             | ReqResult connectDevice(string devId)     |  
| disconnect Device             | ReqResult disconnectDevice(string devId) |
| get public key for a path | ReqResult getPublicKeyFromPath(string path) | should after onnect, device should be unlocked. path is somthing like bip32/m/44'/28'/1'/0'/0  |
| request signature            |  ReqResult signReq(SignReq req) |  |
| request multiply     |  ReqResult multiplyReq(MultiplyReq req) |            |
| device disconnected notify            |  void deviceDisconnected(string devId) |        |
| device connected notify           |  void deviceConnected(string devId)     |        | 

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

1. git clone https://github.com/KeyboxWallet/keyboxd.git
1. cd keyboxd
1. git submodule update --init --recursive
1. install [vcpkg](https://github.com/Microsoft/vcpkg)
1. use vcpkg to install these packages: libusb protobuf boost
1. set environment var vcpkg_root to where vcpkg is installed
1. cmake -DCMAKE_TOOLCHAIN_FILE=D:\src\vcpkg\scripts\buildsystems\vcpkg.cmake .  （replaces with your dir)
1. open .sln file with VS2017, compile
