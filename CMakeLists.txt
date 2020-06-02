
cmake_minimum_required (VERSION 3.1)
set (CMAKE_CXX_STANDARD 11)
project (keybox_daemon)

if (STATIC_BUILD)
SET( Boost_USE_STATIC_LIBS ON  )
SET( Protobuf_USE_STATIC_LIBS ON )
endif (STATIC_BUILD)
set(CMAKE_MACOSX_BUNDLE TRUE)

FIND_PACKAGE( Boost 1.71 COMPONENTS system REQUIRED )
FIND_PACKAGE (Protobuf REQUIRED)

INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )
if (UNIX)
 set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
endif()
if (UNIX)
 find_package(PkgConfig)
 pkg_check_modules (LIBUSB REQUIRED libusb-1.0)
elseif (WIN32)
 set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -utf-8")
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -utf-8")
 if (NOT IS_DIRECTORY $ENV{vcpkg_root})
  message(FATAL_ERROR "you must specify vcpkg_root for seeking packages")
 endif()
 message(STATUS "win32 selected. seeking libusb")
 if( CMAKE_SIZEOF_VOID_P EQUAL 4)
     set (wintarget x86)
 else()
     set (wintarget x64)
 endif()
 if (NOT IS_DIRECTORY $ENV{vcpkg_root}/installed/${wintarget}-windows/include/libusb-1.0)
  message(FATAL_ERROR "please install libusb var vcpkg")
 endif()
 set(LIBUSB_CFLAGS -D_WIN32_WINNT=0x0601 -I$ENV{vcpkg_root}/installed/${wintarget}-windows/include/libusb-1.0)
 set(LIBUSB_LIBRARIES_DIR_DEBUG $ENV{vcpkg_root}/installed/${wintarget}-windows/debug)
 set(LIBUSB_LIBRARIES_DIR_REL $ENV{vcpkg_root}/installed/${wintarget}-windows)
 SET(LIBUSB_LIBRARIES_DEBUG ${LIBUSB_LIBRARIES_DIR_DEBUG}/lib/libusb-1.0.lib)
 SET(LIBUSB_LIBRARIES_REL ${LIBUSB_LIBRARIES_DIR_REL}/lib/libusb-1.0.lib)
endif()

SET(PROTO_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/protocpp)

file(MAKE_DIRECTORY ${PROTO_GEN_DIR})

INCLUDE_DIRECTORIES(depends/json depends/jsonrpc-server-lib )
INCLUDE_DIRECTORIES(depends/protodef)
INCLUDE_DIRECTORIES(${PROTO_GEN_DIR})


set(PROTOSRCS ${PROTO_GEN_DIR}/messages.pb.cc)
set(PROTOHDRS ${PROTO_GEN_DIR}/messages.pb.h)
message(STATUS "protobuf sources: ${PROTOSRCS}")
set_source_files_properties(${PROTOSRCS} ${PROTOHDRS} PROPERTIES generated TRUE)

add_custom_command(
    OUTPUT ${PROTOSRCS} ${PROTOHDRS}
    COMMAND ${Protobuf_PROTOC_EXECUTABLE} --cpp_out ${PROTO_GEN_DIR} -I${CMAKE_CURRENT_SOURCE_DIR}/depends/protodef messages.proto
)


add_executable( keyboxd main.cpp
                ${PROTOSRCS}
                rpc-server.cpp
                device-manager.cpp
                base-device.cpp
                local-device.cpp
                usb-device.cpp
                base64.cpp
                 )
TARGET_LINK_LIBRARIES(keyboxd ${Boost_LIBRARIES} ${Protobuf_LIBRARIES})

target_compile_options (keyboxd PUBLIC ${LIBUSB_CFLAGS} )
target_link_libraries (keyboxd debug ${LIBUSB_LDFLAGS} ${LIBUSB_LIBRARIES} ${LIBUSB_LIBRARIES_DEBUG})
TARGET_LINK_LIBRARIES(keyboxd optimized ${LIBUSB_LDFLAGS}  ${LIBUSB_LIBRARIES} ${LIBUSB_LIBRARIES_REL})
