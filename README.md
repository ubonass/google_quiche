# Google QUICHE CMake Build

本仓库为 Google QUICHE 提供独立的 CMake 构建，当前源码版本对应 QUICHE
M137。构建系统支持按需生成 QUIC、HTTP/3、WebTransport 和 MoQT 静态库，
以及上游提供的示例程序。

QUICHE 是 Google 的 QUIC、HTTP/2、HTTP/3 及相关协议实现。上游项目：

- <https://quiche.googlesource.com/quiche>
- <https://github.com/google/quiche>

## 源码结构

| 路径 | 说明 |
| --- | --- |
| `quiche/` | Google QUICHE 子模块 |
| `google_url/` | Google URL 子模块 |
| `overrides/` | 本项目保留的 QUICHE platform override |
| `cmake/` | 依赖、源码清单和 feature 目标定义 |
| `certs/` | 测试证书相关工具 |

## 构建要求

- CMake 3.20 或更高版本
- Git（用于初始化子模块）
- 支持 C++17 的编译器
- 构建 MoQT 时需要 C++20
- Windows 推荐 Visual Studio 2022 和 ClangCL

首次构建前初始化子模块：

```bash
git submodule update --init --recursive
```

CMake 配置期间会准备 Abseil、BoringSSL、fmt、protobuf、zlib 和
google_url 等依赖。首次配置可能需要较长时间。

## Feature 开关

| CMake 选项 | 默认值 | 作用 |
| --- | --- | --- |
| `GOOGLE_QUICHE_BUILD_H3` | `ON` | 构建 HTTP/3 和 QPACK |
| `GOOGLE_QUICHE_BUILD_WEBTRANSPORT` | `ON` | 构建 WebTransport |
| `GOOGLE_QUICHE_BUILD_MOQT` | `ON` | 构建 MoQT |
| `GOOGLE_QUICHE_BUILD_DEMOS` | `ON` | 为已启用的 feature 创建 demo 可执行程序 |
| `GOOGLE_QUICHE_MSVC_STATIC_RUNTIME` | `ON` | Windows 使用静态 MSVC C/C++ runtime |

协议依赖关系：

```text
quiche_core
    └── quiche_h3
          └── quiche_webtransport
                └── quiche_moqt
```

开启 WebTransport 或 MoQT 时，即使 `GOOGLE_QUICHE_BUILD_H3=OFF`，仍会构建
它们必需的 H3 依赖。开启 MoQT 时也会自动构建 WebTransport。

`GOOGLE_QUICHE_BUILD_DEMOS=OFF` 只关闭可执行程序，不影响协议库。

## 构建目标

| CMake 目标 | 可链接别名 | 说明 |
| --- | --- | --- |
| `quiche_core` | `quiche::core` | QUICHE common 和 QUIC transport 核心 |
| `quiche_h3` | `quiche::h3` | HTTP/3 和 QPACK |
| `quiche_webtransport` | `quiche::webtransport` | WebTransport over HTTP/3 |
| `quiche_moqt` | `quiche::moqt` | Media over QUIC Transport |
| `web_transport_test_server` | — | WebTransport echo/devious-baton 测试服务端 |
| `web_transport_test_client` | — | WebTransport echo 原生测试客户端 |
| `moqt_chat_server` | — | MoQT chat demo 服务端 |
| `moqt_chat_client` | — | MoQT chat demo 客户端，仅支持 POSIX |

上游 `moqt_chat_client` 使用 `poll`、`termios` 和 `unistd`，因此 Windows
不会创建该目标。

## Windows：Visual Studio 2022 + ClangCL

以下命令可以在 PowerShell 或 CMD 中以单行形式执行。

### 配置全部 feature 和 demo

必须先执行配置命令创建构建目录：

```cmd
cmake -S . -B out\win-x64 -G "Visual Studio 17 2022" -A x64 -T ClangCL -DGOOGLE_QUICHE_BUILD_H3=ON -DGOOGLE_QUICHE_BUILD_WEBTRANSPORT=ON -DGOOGLE_QUICHE_BUILD_MOQT=ON -DGOOGLE_QUICHE_BUILD_DEMOS=ON
```

### 构建全部目标

```cmd
cmake --build out\win-x64 --config RelWithDebInfo --target ALL_BUILD --parallel
```

不指定 `--target` 也会构建默认的全部目标：

```cmd
cmake --build out\win-x64 --config RelWithDebInfo --parallel
```

其他常用配置：

```cmd
cmake --build out\win-x64 --config Debug --parallel
cmake --build out\win-x64 --config Release --parallel
```

Visual Studio 是多配置生成器。`CMAKE_BUILD_TYPE` 对该生成器不起作用，实际
配置应通过构建阶段的 `--config Debug|Release|RelWithDebInfo` 选择。

### 只构建指定目标

```cmd
cmake --build out\win-x64 --config RelWithDebInfo --target quiche_core --parallel
cmake --build out\win-x64 --config RelWithDebInfo --target quiche_h3 --parallel
cmake --build out\win-x64 --config RelWithDebInfo --target quiche_webtransport --parallel
cmake --build out\win-x64 --config RelWithDebInfo --target quiche_moqt --parallel
cmake --build out\win-x64 --config RelWithDebInfo --target web_transport_test_server --parallel
cmake --build out\win-x64 --config RelWithDebInfo --target web_transport_test_client --parallel
cmake --build out\win-x64 --config RelWithDebInfo --target moqt_chat_server --parallel
```

### 只构建库

关闭全局 demo 开关后重新配置：

```cmd
cmake -S . -B out\win-clang-libs -G "Visual Studio 17 2022" -A x64 -T ClangCL -DGOOGLE_QUICHE_BUILD_H3=ON -DGOOGLE_QUICHE_BUILD_WEBTRANSPORT=ON -DGOOGLE_QUICHE_BUILD_MOQT=ON -DGOOGLE_QUICHE_BUILD_DEMOS=OFF
cmake --build out\win-clang-libs --config RelWithDebInfo --parallel
```

### 使用动态 MSVC runtime

默认使用静态 runtime。需要 `/MD` 和 `/MDd` 时：

```cmd
cmake -S . -B out\win-clang-dynamic-crt -G "Visual Studio 17 2022" -A x64 -T ClangCL -DGOOGLE_QUICHE_MSVC_STATIC_RUNTIME=OFF
```

## Linux：Clang + Ninja

配置全部 feature 和 demo：

```bash
cmake -S . -B out/linux-clang \
  -G Ninja \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DGOOGLE_QUICHE_BUILD_H3=ON \
  -DGOOGLE_QUICHE_BUILD_WEBTRANSPORT=ON \
  -DGOOGLE_QUICHE_BUILD_MOQT=ON \
  -DGOOGLE_QUICHE_BUILD_DEMOS=ON
```

构建全部目标：

```bash
cmake --build out/linux-clang --parallel
```

只构建 demo：

```bash
cmake --build out/linux-clang --target web_transport_test_server web_transport_test_client
cmake --build out/linux-clang --target moqt_chat_client moqt_chat_server
```

## 运行 demo

WebTransport Demo 的架构、实现原理和完整使用方法请参阅
[`docs/WEBTRANSPORT_DEMO.md`](docs/WEBTRANSPORT_DEMO.md)。

### 准备测试证书

WebTransport 和 MoQT 服务端都必须通过 `--certificate_file` 和 `--key_file`
指定 PEM 证书链及 PKCS#8 PEM 私钥。可使用 OpenSSL 创建仅供本机测试的
自签名证书：

Windows CMD（需要先安装 OpenSSL 并确保 `openssl.exe` 位于 `PATH`）：

```cmd
if not exist out\certs mkdir out\certs
openssl genpkey -algorithm RSA -out out\certs\localhost.key
openssl req -new -x509 -key out\certs\localhost.key -out out\certs\localhost.crt -days 14 -subj "/CN=localhost" -addext "subjectAltName=DNS:localhost,IP:127.0.0.1"
```

Windows PowerShell：

```powershell
New-Item -ItemType Directory -Force out\certs | Out-Null
openssl genpkey -algorithm RSA -out out\certs\localhost.key
openssl req -new -x509 -key out\certs\localhost.key -out out\certs\localhost.crt -days 14 -subj "/CN=localhost" -addext "subjectAltName=DNS:localhost,IP:127.0.0.1"
```

Linux 或 macOS：

```bash
mkdir -p out/certs
openssl genpkey -algorithm RSA -out out/certs/localhost.key
openssl req -new -x509 -key out/certs/localhost.key \
  -out out/certs/localhost.crt -days 14 -subj "/CN=localhost" \
  -addext "subjectAltName=DNS:localhost,IP:127.0.0.1"
```

该证书默认不受系统或浏览器信任，仅用于开发测试。生产环境应使用可信 CA
签发的证书并妥善保护私钥。

### WebTransport test server

Windows RelWithDebInfo 输出目录下启动服务端：

```cmd
out\win-x64\RelWithDebInfo\web_transport_test_server.exe --port=6121 --certificate_file=out\certs\localhost.crt --key_file=out\certs\localhost.key
```

Linux：

```bash
out/linux-clang/web_transport_test_server \
  --port=6121 \
  --certificate_file=out/certs/localhost.crt \
  --key_file=out/certs/localhost.key
```

服务端提供 echo 和 devious-baton 两个独立 endpoint。

### WebTransport echo

Echo endpoint：

```text
https://localhost:6121/webtransport/echo
```

服务端会分别处理双向流、单向流和 Datagram，并将收到的数据原样返回。本项目
在 `samples/web_transport_test_client.cc` 提供非交互式原生 echo 客户端。
客户端建立一个 WebTransport session，然后按固定周期创建双向流、发送消息并
校验返回内容。

Windows：

```cmd
out\win-x64\RelWithDebInfo\web_transport_test_client.exe --count=10 --interval_ms=1000 --timeout_ms=5000 https://localhost:6121/webtransport/echo "hello webtransport"
```

Linux：

```bash
out/linux-clang/web_transport_test_client \
  --count=10 --interval_ms=1000 --timeout_ms=5000 \
  https://localhost:6121/webtransport/echo "hello webtransport"
```

客户端默认启用 `--disable_certificate_verification=true`，仅适用于本地测试。
相关参数：

- `--count`：echo 次数，必须大于零，默认 `10`。
- `--interval_ms`：前一次 echo 完成后到下一次发起前的等待时间，默认 `1000`
  毫秒。
- `--timeout_ms`：连接、建流和单次 echo 的超时时间，默认 `5000` 毫秒。
- `--disable_certificate_verification`：是否跳过证书验证，本地测试默认开启。

全部 echo 成功后返回退出码 `0`；连接失败、超时或响应不匹配时立即返回非零
退出码。

也可以在信任测试证书的浏览器中通过 JavaScript WebTransport API 连接 echo
endpoint：

```javascript
const transport = new WebTransport(
  "https://localhost:6121/webtransport/echo"
);
await transport.ready;
console.log("WebTransport connected");
```

### WebTransport devious-baton

Devious Baton endpoint：

```text
https://localhost:6121/webtransport/devious-baton
```

该 endpoint 用于测试 WebTransport 的单向流、双向流、流量控制和会话错误处理，
不是普通的字符串 echo。服务端会发起 baton，客户端需要按照 Devious Baton
协议解析、递增并通过指定类型的 stream 返回。

支持以下查询参数：

- `count`：服务端初始创建的 baton stream 数量，范围 `0..255`，默认 `1`。
- `baton`：初始 baton 值，范围 `0..255`；未指定时由服务端随机生成。

例如，让服务端创建 3 个初始 baton，初始值为 42：

```text
https://localhost:6121/webtransport/devious-baton?count=3&baton=42
```

当前项目的 `web_transport_test_client` 专门用于周期 echo，不实现 Devious Baton
状态机。测试该 endpoint 需要兼容
`draft-frindell-webtrans-devious-baton` 的客户端；仅建立 WebTransport session
并发送普通字符串不会得到 echo 响应。

### MoQT chat server

Windows：

```cmd
out\win-x64\RelWithDebInfo\moqt_chat_server.exe --bind_address=127.0.0.1 --port=9667 --certificate_file=out\certs\localhost.crt --key_file=out\certs\localhost.key
```

Linux：

```bash
out/linux-clang/moqt_chat_server \
  --bind_address=127.0.0.1 \
  --port=9667 \
  --certificate_file=out/certs/localhost.crt \
  --key_file=out/certs/localhost.key
```

可通过 `--output_file=<path>` 将服务端收到的聊天消息写入文件。

### MoQT chat client（POSIX）

客户端参数格式为：

```text
moqt_chat_client [options] <url> <username> <chat-id> <device-id>
```

启动两个客户端并加入同一个 `demo-room`：

```bash
out/linux-clang/moqt_chat_client \
  --disable_certificate_verification=true \
  https://127.0.0.1:9667/moq-chat alice demo-room alice-device
```

```bash
out/linux-clang/moqt_chat_client \
  --disable_certificate_verification=true \
  https://127.0.0.1:9667/moq-chat bob demo-room bob-device
```

输入消息后，同一 chat ID 中的其他客户端会收到消息；输入 `/exit` 退出。
`--output_file=<path>` 可以将消息写入文件。Windows 当前不会生成该客户端。

### 查看程序参数

四个 demo 均支持命令行帮助：

```text
web_transport_test_server --help
web_transport_test_client --help
moqt_chat_server --help
moqt_chat_client --help
```

## 上游限制

- QUICHE 仅支持小端平台。
- `moqt_chat_client` 当前仅支持 POSIX。
- demo 用于开发和互操作测试，不应直接作为生产服务部署。
