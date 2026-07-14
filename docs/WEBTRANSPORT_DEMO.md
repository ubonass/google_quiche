# WebTransport Demo 介绍与使用指南

本文档详细介绍本项目 WebTransport Demo 的整体框架、工作原理、源码实现和
运行方法。Demo 包含一个原生服务端和一个原生周期 Echo 客户端：

- `web_transport_test_server`：提供 Echo 和 Devious Baton endpoint。
- `web_transport_test_client`：周期访问 Echo endpoint，并校验每次响应。

相关源码：

- [`web_transport_test_server.cc`](../samples/web_transport_test_server.cc)
- [`web_transport_test_client.cc`](../samples/web_transport_test_client.cc)
- [`network_initializer.h`](../samples/network_initializer.h)
- [`../cmake/quiche_webtransport.cmake`](../cmake/quiche_webtransport.cmake)

## 1. Demo 框架

### 1.1 构建目标

Demo 由 `GOOGLE_QUICHE_BUILD_WEBTRANSPORT` 和
`GOOGLE_QUICHE_BUILD_DEMOS` 共同控制：

```text
GOOGLE_QUICHE_BUILD_WEBTRANSPORT=ON
GOOGLE_QUICHE_BUILD_DEMOS=ON
```

构建依赖关系：

```text
quiche::core
    └── quiche::h3
          └── quiche::webtransport
                ├── web_transport_test_server
                └── web_transport_test_client
```

其中：

- `quiche::core` 提供 QUIC、TLS、UDP socket、拥塞控制和事件循环。
- `quiche::h3` 提供 HTTP/3、QPACK 和 CONNECT stream。
- `quiche::webtransport` 提供 WebTransport session、stream 和 Datagram。
- 两个可执行程序负责具体测试协议和命令行交互。

### 1.2 运行时框架

```text
web_transport_test_client
    │
    │  HTTPS URL + HTTP/3 CONNECT(:protocol=webtransport)
    ▼
QUIC / TLS / UDP
    │
    ▼
web_transport_test_server
    │
    ├── /webtransport/echo
    │       └── EchoWebTransportSessionVisitor
    │
    └── /webtransport/devious-baton
            └── DeviousBatonSessionVisitor
```

服务端通过 URL path 选择 session visitor。建立 WebTransport session 后，应用
数据不再作为普通 HTTP request body 传输，而是通过 WebTransport 双向流、
单向流或 Datagram 传输。

### 1.3 源码职责

#### `web_transport_test_server.cc`

- 初始化平台网络环境。
- 读取证书、私钥和监听端口参数。
- 创建 `QuicServer` 和 `WebTransportOnlyBackend`。
- 通过 `ProcessRequest()` 根据 URL 路径创建 session visitor。
- 在 UDP socket 上持续处理 QUIC/HTTP3/WebTransport 事件。

#### `web_transport_test_client.cc`

- 初始化平台网络环境和 QUIC event loop。
- 解析 WebTransport URL 并解析服务端地址。
- 建立 QUIC/TLS 连接并等待 HTTP/3 SETTINGS。
- 发送扩展 CONNECT 请求建立 WebTransport session。
- 在同一个 session 中周期创建双向流并执行 Echo。
- 校验每次响应、处理超时，并通过进程退出码报告结果。

#### `network_initializer.h`

- Windows 调用 `WSAStartup(MAKEWORD(2, 2))`。
- 对应实例销毁时调用 `WSACleanup()`。
- Linux/macOS 下为空操作。

## 2. Demo 原理和实现

### 2.1 WebTransport session 建立

客户端首先创建 `QuicDefaultClient`，启用 WebTransport：

```cpp
client.set_enable_web_transport(true);
```

QUIC/TLS 握手成功并收到服务端 HTTP/3 SETTINGS 后，客户端创建双向 HTTP/3
stream，发送扩展 CONNECT：

```text
:method    = CONNECT
:protocol  = webtransport
:scheme    = https
:authority = localhost:6121
:path      = /webtransport/echo
```

CONNECT 成功后，该 stream 持有 `WebTransportHttp3` session。之后创建的
WebTransport stream 都和这个 session 关联。

### 2.2 服务端请求分发

服务端使用 `WebTransportOnlyBackend` 接收 WebTransport CONNECT 请求，
`ProcessRequest()` 根据 path 分发：

```text
/webtransport/echo
    → EchoWebTransportSessionVisitor

/webtransport/devious-baton
    → DeviousBatonSessionVisitor

其他路径
    → NotFound
```

无效 URL、无效查询参数或未知路径会导致 session 建立失败。

### 2.3 周期 Echo 原理

原生客户端只建立一个 WebTransport session，并在该 session 中重复执行：

```text
等待可创建双向流
    ↓
创建 WebTransport bidirectional stream
    ↓
发送 message + FIN
    ↓
服务端在同一个 stream 回写相同数据 + FIN
    ↓
客户端读取完整响应并逐字节比较
    ↓
等待 interval_ms
    ↓
发起下一次 Echo
```

每一次 Echo 使用独立的双向流，因此可以同时验证：

- WebTransport session 是否持续可用。
- 新 stream 是否可以持续创建。
- 双向 stream 的读写与 FIN 状态是否正确。
- 返回内容是否完整且与请求一致。
- QUIC 连接在连续应用操作期间是否保持稳定。

当前实现为串行周期测试：前一次 Echo 完成后等待 `interval_ms`，再发起下一次。
它不会在前一次 Echo 尚未完成时并发创建下一条测试流。

### 2.4 Echo 服务端行为

`EchoWebTransportSessionVisitor` 支持：

- 双向流：在同一个 stream 上回写收到的数据。
- 单向流：读取完整数据后创建新的单向流回写。
- Datagram：将收到的 Datagram 原样发送回客户端。

当前原生客户端使用双向流进行周期测试。

`/webtransport/echo` 是无状态的“原样返回”协议。服务端不解释 payload 的业务含义，
也不要求前后两次消息存在关联。对当前客户端使用的双向流，一次交互如下：

```text
Client                         Server
  |-- open bidirectional stream -->|
  |-- payload + FIN --------------->|
  |<------------- payload + FIN ----|
  |-- verify payload                |
```

它适合以下场景：

- 验证 UDP、QUIC、TLS、HTTP/3 和 WebTransport CONNECT 是否可以正常建立。
- 验证双向流、单向流或 Datagram 的基本收发与数据完整性。
- 周期性观察连接稳定性、请求成功率和往返耗时。
- 作为开发调试、冒烟测试和 CI 中的最小可用性检查。

Echo 不适合验证复杂的应用层状态机，也不能单独覆盖服务端主动发流、跨 stream
消息关联等行为。

### 2.5 Devious Baton 原理

Devious Baton 用于测试比普通 Echo 更复杂的 WebTransport 行为，包括：

- 服务端主动创建单向流。
- 客户端和服务端交替创建单向流、双向流。
- baton 数据解析与递增。
- stream flow-control credit。
- RESET_STREAM、STOP_SENDING 和 session error。

`/webtransport/devious-baton` 是有状态、带消息格式的互操作测试协议。每个 baton
消息由 QUIC variable-length integer 编码的随机 padding 长度、对应的 padding 字节，
以及一个 8-bit baton 值组成。接收方必须解析该结构，而不是简单回写原始字节。

服务端在 session ready 后主动创建 `count` 条单向流，并在每条流上发送初始 baton。
后续交互会在单向流和双向流之间切换：服务端收到单向流上的 baton 后创建双向流；
收到双向流上的 baton 后将数值递增，并通过同一条双向流继续传递。baton 是 8-bit
数值，递增回绕到 `0` 时结束该链路。概念时序如下：

```text
Client                              Server
  |<-- server-initiated unidirectional stream (baton N)
  |-- bidirectional stream (baton N+1) -->|
  |<---------------- baton N+2 ------------|
  |-- unidirectional stream (baton N+3) -->|
  |                    ...                  |
  |        stop when the value wraps to 0   |
```

它适合以下场景：

- 验证不同实现之间的 WebTransport 协议互操作性。
- 验证服务端主动创建 stream，以及单向流、双向流的切换。
- 检查 stream 数量额度、flow control、异步可写通知和 session 生命周期。
- 检查结构化消息解析、非法消息、stream reset 和 session error 的处理。
- 对 WebTransport 实现进行比 Echo 更深入的状态机和边界行为测试。

Devious Baton 不是吞吐量基准或业务 Echo 服务。当前实现也没有用 Baton 流程验证
Datagram；Datagram 原样返回是 Echo endpoint 提供的能力。

服务端接受两个查询参数：

- `count`：初始 baton stream 数量，范围 `0..255`，默认 `1`。
- `baton`：初始 baton 值，范围 `0..255`；未指定时随机生成。

当前 `web_transport_test_client` 专用于周期 Echo，没有实现 Devious Baton 状态机。
测试 Baton endpoint 需要兼容 `draft-frindell-webtrans-devious-baton` 的客户端。

### 2.6 `/echo` 与 `/devious-baton` 的区别

两者使用相同的底层网络栈：UDP 承载 QUIC，QUIC 上运行 TLS 和 HTTP/3，客户端通过
Extended CONNECT 建立 WebTransport session。选择不同 URL path 不会改变 QUIC
版本、加密、拥塞控制或 socket 实现；服务端根据 CONNECT 的 path 为 session 安装
不同的应用层 visitor。真正的区别从 WebTransport session 之上的应用协议开始。

| 对比项 | `/webtransport/echo` | `/webtransport/devious-baton` |
| --- | --- | --- |
| 主要目标 | 连通性、数据完整性、周期稳定性测试 | 互操作、状态机、flow control 和边界行为测试 |
| 应用层状态 | 无状态，每次 payload 可独立处理 | 有状态，baton 值在多次交互中递增直至回绕 |
| 消息格式 | 任意不透明字节，原样返回 | padding 长度、padding 和 8-bit baton 的结构化编码 |
| 发起方向 | 通常由客户端发起；服务端也可创建测试流 | session ready 后由服务端主动发送初始 baton |
| 双向流 | 同一条流读取并回写相同内容 | 解析 baton、递增，并在协议规定的流上继续传递 |
| 单向流 | 读完输入流后另开输出单向流回写 | 用于驱动下一阶段，和双向流交替使用 |
| Datagram | 支持原样返回 | 当前 Baton 状态机未实现 Datagram 测试 |
| 完成条件 | 收到相同 payload 和 FIN | baton 递增并回绕到 `0` |
| 错误关注点 | 超时、连接关闭、内容不一致 | 解析失败、额度不足、异常消息和 session/stream 错误 |
| 推荐用途 | 本地冒烟、健康检查、RTT 观察、基础 API 验证 | 实现验证、跨实现兼容测试、复杂 stream 行为测试 |

简单选择原则：如果目的是确认“连接能否建立、数据能否正确往返”，使用
`/webtransport/echo`；如果目的是确认“WebTransport 实现能否正确处理复杂 stream
拓扑、状态转换和错误条件”，使用 `/webtransport/devious-baton`。

### 2.7 超时和失败处理

客户端分别等待：

- QUIC connection 建立。
- HTTP/3 SETTINGS。
- WebTransport session ready。
- 新双向流额度。
- 单次 Echo 完整响应。

任一步骤超时、session 关闭或响应内容不一致，客户端都会输出错误并返回非零
退出码。所有 Echo 成功时返回 `0`。

## 3. 详细使用介绍

### 3.1 准备构建环境

Windows 推荐：

- Visual Studio 2022
- ClangCL
- CMake 3.20 或更高版本
- OpenSSL 命令行工具（用于生成测试证书）

初始化子模块：

```cmd
git submodule update --init --recursive
```

### 3.2 配置工程

```cmd
cmake -S . -B out\win-x64 -G "Visual Studio 17 2022" -A x64 -T ClangCL -DGOOGLE_QUICHE_BUILD_H3=ON -DGOOGLE_QUICHE_BUILD_WEBTRANSPORT=ON -DGOOGLE_QUICHE_BUILD_DEMOS=ON
```

MoQT 与 WebTransport Demo 无关，可以按需设置：

```text
-DGOOGLE_QUICHE_BUILD_MOQT=OFF
```

### 3.3 编译 Demo

只编译 WebTransport server 和 client：

```cmd
cmake --build out\win-x64 --config RelWithDebInfo --target web_transport_test_server web_transport_test_client --parallel
```

编译全部已配置目标：

```cmd
cmake --build out\win-x64 --config RelWithDebInfo --target ALL_BUILD --parallel
```

Windows 输出文件：

```text
out\win-x64\RelWithDebInfo\web_transport_test_server.exe
out\win-x64\RelWithDebInfo\web_transport_test_client.exe
```

### 3.4 Windows 生成测试证书

确保 `openssl.exe` 位于 `PATH`。

CMD：

```cmd
if not exist out\certs mkdir out\certs
openssl genpkey -algorithm RSA -out out\certs\localhost.key
openssl req -new -x509 -key out\certs\localhost.key -out out\certs\localhost.crt -days 14 -subj "/CN=localhost" -addext "subjectAltName=DNS:localhost,IP:127.0.0.1"
```

PowerShell：

```powershell
New-Item -ItemType Directory -Force out\certs | Out-Null
openssl genpkey -algorithm RSA -out out\certs\localhost.key
openssl req -new -x509 -key out\certs\localhost.key -out out\certs\localhost.crt -days 14 -subj "/CN=localhost" -addext "subjectAltName=DNS:localhost,IP:127.0.0.1"
```

生成文件：

```text
out\certs\localhost.crt  PEM 自签名证书
out\certs\localhost.key  PKCS#8 PEM 私钥
```

这些文件只应用于本地开发测试。

### 3.5 启动服务端

```cmd
out\win-x64\RelWithDebInfo\web_transport_test_server.exe --port=6121 --certificate_file=out\certs\localhost.crt --key_file=out\certs\localhost.key
```

主要参数：

| 参数 | 默认值 | 说明 |
| --- | --- | --- |
| `--port` | `6121` | UDP 监听端口 |
| `--certificate_file` | 无 | PEM 证书链，必填 |
| `--key_file` | 无 | PKCS#8 PEM 私钥，必填 |

服务端启动成功后会持续运行，使用 `Ctrl+C` 停止。

### 3.6 运行周期 Echo

基本用法：

```text
web_transport_test_client [options] <url> <message>
```

执行 10 次 Echo，每次完成后等待 1 秒：

```cmd
out\win-x64\RelWithDebInfo\web_transport_test_client.exe --count=10 --interval_ms=1000 --timeout_ms=5000 https://localhost:6121/webtransport/echo "hello webtransport"
```

客户端参数：

| 参数 | 默认值 | 说明 |
| --- | --- | --- |
| `--count` | `10` | Echo 次数，必须大于零 |
| `--interval_ms` | `1000` | 前一次完成到下一次发起前的等待时间 |
| `--timeout_ms` | `5000` | 连接、建流和单次 Echo 的超时时间 |
| `--disable_certificate_verification` | `true` | 跳过证书验证，仅供本地测试 |

成功输出示例：

```text
Echo 1/10 succeeded: hello webtransport
Echo 2/10 succeeded: hello webtransport
...
Echo 10/10 succeeded: hello webtransport
```

若只测试一次：

```cmd
out\win-x64\RelWithDebInfo\web_transport_test_client.exe --count=1 https://localhost:6121/webtransport/echo "one shot"
```

### 3.7 使用 Devious Baton endpoint

服务端不需要额外启动参数。兼容客户端应连接：

```text
https://localhost:6121/webtransport/devious-baton
```

指定 3 个初始 stream 和初始值 42：

```text
https://localhost:6121/webtransport/devious-baton?count=3&baton=42
```

普通 Echo client 不能用于该 endpoint：

```text
web_transport_test_client ... /webtransport/devious-baton
```

客户端会主动拒绝该 URL，因为两种 endpoint 的应用层协议不同。

### 3.8 浏览器连接 Echo endpoint

在浏览器信任测试证书后，可以使用 JavaScript WebTransport API：

```javascript
const transport = new WebTransport(
  "https://localhost:6121/webtransport/echo"
);
await transport.ready;
console.log("WebTransport connected");
```

仅建立 session 不会自动执行 Echo。浏览器代码还需要创建双向流、写入数据、
关闭 writable 并从 readable 读取响应。

### 3.9 查看帮助

```cmd
out\win-x64\RelWithDebInfo\web_transport_test_server.exe --help
out\win-x64\RelWithDebInfo\web_transport_test_client.exe --help
```

### 3.10 运行注意事项

- server 和 client 必须使用相同端口。
- URL 必须使用 `https` scheme。
- Echo client 当前只接受 `/webtransport/echo`。
- Windows 程序已自动初始化 Winsock，无需调用额外脚本。
- 本地默认跳过客户端证书验证，不应将此行为用于生产环境。
- 测试证书和私钥不应提交到源码仓库。
