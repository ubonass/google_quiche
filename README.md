# QUICHE

QUICHE stands for QUIC, Http, Etc. It is Google's production-ready
implementation of QUIC, HTTP/2, HTTP/3, and related protocols and tools. It
powers Google's servers, Chromium, Envoy, and other projects. It is actively
developed and maintained.

There are two public QUICHE repositories. Either one may be used by embedders,
as they are automatically kept in sync:

*   https://quiche.googlesource.com/quiche
*   https://github.com/google/quiche

To embed QUICHE in your project, platform APIs need to be implemented and build
files need to be created. Note that it is on the QUICHE team's roadmap to
include default implementation for all platform APIs and to open-source build
files. In the meanwhile, take a look at open source embedders like Chromium and
Envoy to get started:

*   [Platform implementations in Chromium](https://source.chromium.org/chromium/chromium/src/+/main:net/third_party/quiche/overrides/quiche_platform_impl/)
*   [Build file in Chromium](https://source.chromium.org/chromium/chromium/src/+/main:net/third_party/quiche/BUILD.gn)
*   [Platform implementations in Envoy](https://github.com/envoyproxy/envoy/tree/master/source/common/quic/platform)
*   [Build file in Envoy](https://github.com/envoyproxy/envoy/blob/main/bazel/external/quiche.BUILD)

To contribute to QUICHE, follow instructions at
[CONTRIBUTING.md](CONTRIBUTING.md).

QUICHE is only supported on little-endian platforms.

### Features
- Easy building with cmake
- support Linux And Windows platform
- Easy to keep pace with Google quiche upgrading

### 源码树说明

- src目录对应google_quiche项目
- overrides目录为quiche项目中有部分接口需要用户自定义实现（需要自己实现）
- googleurl目录为quiche依赖谷歌的Url项目的源码，使用gitsubmodule进行管理
- base目录为自定义实现的一些工具类
- net目录为用户自定义实现如SDK等
- certs用于证书生成的工具
- examples为demo实现目录


## Getting Started

### Prerequisite  

```bash
apt-get install cmake build-essential protobuf-compiler libprotobuf-dev golang-go libunwind-dev libicu-dev
git submodule update --init
```

### Build  for Linux

-  Debug

```bash
mkdir buildDebug && cd buildDebug  
cmake -DCMAKE_C_COMPILER=gcc \
      -DCMAKE_CXX_COMPILER=g++ \
      -DCMAKE_BUILD_TYPE:STRING=Debug \
      -DBUILD_SHARED_LIBS=OFF \
      -DBUILD_TESTING=OFF \
      -Dprotobuf_BUILD_TESTS=OFF \
      -Dprotobuf_BUILD_EXAMPLES=OFF \
      -Dprotobuf_BUILD_PROTOC_BINARIES=OFF \
      -DBUILD_QUICHE_EXAMPLES=ON \
      -DCMAKE_INSTALL_PREFIX=./install \
      ..
make -j4
cd -
```

- Release

```bash
mkdir buildRelease && cd buildRelease  
cmake -DCMAKE_C_COMPILER=gcc \
      -DCMAKE_CXX_COMPILER=g++ \
      -DCMAKE_BUILD_TYPE:STRING=Release \
      -DBUILD_SHARED_LIBS=OFF \
      -DBUILD_TESTING=OFF \
      -Dprotobuf_BUILD_TESTS=OFF \
      -Dprotobuf_BUILD_EXAMPLES=OFF \
      -Dprotobuf_BUILD_PROTOC_BINARIES=OFF \
      -DBUILD_QUICHE_EXAMPLES=ON \
      -DCMAKE_INSTALL_PREFIX=./install \
      ..
make -j4
cd -
```



### Build for Android

- Compile In Linux  for android

```bash
export ANDROID_NDK=/tmp/android-ndk-r26b

CURRENT_DIR=$(cd $(dirname $0); pwd)

if [ ! -d "${CURRENT_DIR}/buildAndroid" ]; then
  mkdir -p ${CURRENT_DIR}/buildAndroid
fi

cd ${CURRENT_DIR}/buildAndroid

cmake -DCMAKE_TOOLCHAIN_FILE=${ANDROID_NDK}/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI="arm64-v8a" \
    -DANDROID_NDK=${ANDROID_NDK} \
    -DANDROID_PLATFORM=android-29 \
    -DCMAKE_BUILD_TYPE:STRING=Release \
    -DBUILD_SHARED_LIBS=OFF \
    -DBUILD_TESTING=OFF \
    -Dprotobuf_BUILD_TESTS=OFF \
    -Dprotobuf_BUILD_EXAMPLES=OFF \
    -Dprotobuf_BUILD_PROTOC_BINARIES=OFF \
    -DCMAKE_INSTALL_PREFIX=./install \
    ..

make

cd ..
```

- Compile In Windows for android

```bash
set ANDROID_NDK=D:/ideTools/android_sdk_windows/ndk/26.1.10909125
set TOOLCHAIN_FILE=%ANDROID_NDK%/build/cmake/android.toolchain.cmake
set BUILD_TYPE=Release
set GENERATOR="Ninja"

if not exist buildAndroid (
  md buildAndroid
)
cd buildAndroid

if not exist arm64-v8a (
  md arm64-v8a
)
cd arm64-v8a
cmake -DCMAKE_TOOLCHAIN_FILE=%TOOLCHAIN_FILE% ^
      -DANDROID_NDK=%ANDROID_NDK% ^
      -DANDROID_PLATFORM=android-29 ^
      -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
      -DANDROID_ABI="arm64-v8a" ^
      -DCMAKE_GENERATOR=%GENERATOR% ^
      -DBUILD_SHARED_LIBS=OFF ^
      -DBUILD_TESTING=OFF ^
      -Dprotobuf_BUILD_TESTS=OFF ^
      -Dprotobuf_BUILD_EXAMPLES=OFF ^
      -Dprotobuf_BUILD_PROTOC_BINARIES=OFF ../..
ninja
```

- Linux you can using build_android.sh Or windows using build_android.bat

- android may display the following error when building a cmake project

  ```cmake
  -- Configuring done
  CMake Error in buildAndroid/arm64-v8a/_deps/protobuf-src/cmake/CMakeLists.txt:
    export called with target "libprotobuf-lite" which requires target "log"
    that is not in any export set.
  
  
  CMake Error in buildAndroid/arm64-v8a/_deps/protobuf-src/cmake/CMakeLists.txt:
    export called with target "libprotobuf" which requires target "log" that is
    not in any export set.
  ```

- 按照如下进行修改

- 1）首先找到第三方库的`buildAndroid/arm64-v8a/_deps/protobuf-src/cmake/libprotobuf.cmake和buildAndroid/arm64-v8a/_deps/protobuf-src/cmakelibprotobuf-lite.cmake`文件,然后按照

  ```cmake
  #将如下块
  if(${CMAKE_SYSTEM_NAME} STREQUAL "Android")
  	target_link_libraries(libprotobuf-lite log)
  endif()
  #改成如下：
  ```

  ```cmake
  if(${CMAKE_SYSTEM_NAME} STREQUAL "Android")
    find_library(ANDROID_LOG_LIBRARY log)
    MESSAGE("ANDROID_LOG_LIBRARY:"${ANDROID_LOG_LIBRARY})
    target_link_libraries(libprotobuf-lite ${ANDROID_LOG_LIBRARY})
  endif()
  ```

- 2）然后再执行脚本

### Build  for Windows

- debug 

```bash
cd out
mkdir win-x64_debug && cd win-x64_debug  
cmake -G "Visual Studio 17 2022" -T ClangCL ^
	-DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\\scripts\\buildsystems\\vcpkg.cmake" ^
	-DCMAKE_BUILD_TYPE:STRING=Debug ^
	-DCMAKE_INSTALL_PREFIX=./install ../../
```

- release

```bash
mkdir win-x64_release && cd win-x64_release    
cmake -G "Visual Studio 17 2022" -T ClangCL ^
	-DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\\scripts\\buildsystems\\vcpkg.cmake" ^
	-DCMAKE_BUILD_TYPE:STRING=Release ^
	-DCMAKE_INSTALL_PREFIX=./install ../../
```



### Play examples

- A sample quic server and client implementation are provided in quiche. To use these you should build the binaries.

- Download a copy of www.example.org, which we will serve locally using the simple_quic_server binary.

```bash
mkdir -p /data/quic-root
wget -p --save-headers https://www.example.org -P /data/quic-root
```

- In order to run the simple_quic_server, you will need a valid certificate, and a private key is pkcs8 format. If you don't have one, there are scripts to generate them.

```bash
cd certs
./generate-certs.sh
mkdir -p /data/quic-cert
mv ./out/* /data/quic-cert/
cd -
```

- Run the quic server

```bash
./build/simple_quic_server \
  --quic_response_cache_dir=/data/quic-root/ \
  --certificate_file=/data/quic-cert/leaf_cert.pem \
  --key_file=/data/quic-cert/leaf_cert.key \
  --key_log_file=/data/ssl_key.log
```

- Request the file with quic client

```bash
./build/simple_quic_client \
  --disable_certificate_verification=true \
  --host=127.0.0.1 --port=6121 \
  "https://www.example.org/index.html"
```

- To test the same download using chrome

```c++
chrome \
  --user-data-dir=/tmp/chrome-profile \
  --no-proxy-server \
  --enable-quic \
  --origin-to-force-quic-on=www.example.org:443 \
  --host-resolver-rules='MAP www.example.org:443 127.0.0.1:6121' \
  https://www.example.org
```

- Run the quic raw server

```bash
./build/quic_echo_server --port=6122  --log_level=0 --dlog=true --vlog=true --dvlog=true
```

- Run the quic raw client

```bash
./build/quic_echo_client --host=127.0.0.1 --port=6122 --log_level=0 --dlog=true --vlog=true --dvlog=true --ssl_log=/data/ssl_key.log
```

You can also use chormium-based browsers to access simple_quic_server at `127.0.0.1:6121`, and check the request/response protocol by DevTools -> Network panel.
