# WebTransport Demo 1-RTT Null Crypter 修改方案

## 1. 目标

为 `web_transport_test_client` 和 `web_transport_test_server` 增加一个仅用于本地
抓包分析的实验模式：TLS 握手保持正常，禁用 0-RTT，并将所有 1-RTT 包的真实
AEAD crypter 替换为 QUICHE 的 `NullEncrypter` 和 `NullDecrypter`。

目标加密级别如下：

```text
Initial    标准 QUIC Initial 加密
Handshake  标准 TLS Handshake 加密
0-RTT      禁用
1-RTT      Null Crypter
```

在该模式下，HTTP/3、Extended CONNECT、WebTransport stream、Datagram、Echo 和Devious Baton 数据均由 1-RTT 包承载，不具备机密性保护。Initial 和 Handshake仍保持标准协议行为，以保证连接建立过程稳定。本方案是设计文档，当前阶段不修改实现代码。

## 2. 约束

- 不修改 `quiche/` 目录中的上游实现。
- 所有定制实现放在 `samples/` 目录。
- Demo target 直接编译 `samples/` 下的定制源文件。
- 默认继续使用标准 QUIC 加密，必须显式开启实验模式。
- client 和 server 必须同时开启或同时关闭。
- 实验模式不保证与标准 QUIC/WebTransport 实现互操作。
- Null Crypter 仍附加 12 字节 FNV hash，但不提供密码学意义上的机密性。

IETF QUIC Header Protection 始终需要从密文中取得 16 字节 sample。原生
`NullEncrypter` 对非常短的 packet 可能无法提供足够的采样长度，因此 Demo 使用
配套的 `WebTransportNullEncrypter` 和 `WebTransportNullDecrypter`。Encrypter在
Null ciphertext 前增加8字节非明文采样前缀，Decrypter在校验前剥离该前缀；QUIC
plaintext、frame和 WebTransport应用数据保持逐字节不变。

## 3. 不直接原样复制同名类

当前 Demo target 已通过 `quiche::webtransport` 及 tools 源文件使用`QuicDefaultClient`、`QuicSimpleClientSession`、`QuicServer`、
`QuicSimpleDispatcher` 和 `QuicSimpleServerSession`。

如果把这些文件原样复制到 `samples/`，保留相同类名和符号，同时又链接包含相关实现的库或编译上游源文件，容易产生重复符号、头文件引用仍指向上游版本、后续同步困难等问题。

本方案在 `samples/` 中创建派生类，只覆盖 session 创建和密钥安装相关的虚函数。它达到“定制代码全部位于 samples”的目标，同时复用未修改的上游网络、TLS、HTTP/3和 WebTransport 实现。

## 4. 建议新增文件

```text
samples/web_transport_crypto_config.h
samples/web_transport_crypto_config.cc
samples/web_transport_client_session.h
samples/web_transport_client_session.cc
samples/web_transport_client_adapter.h
samples/web_transport_client_adapter.cc
samples/web_transport_server_session.h
samples/web_transport_server_session.cc
samples/web_transport_dispatcher.h
samples/web_transport_dispatcher.cc
samples/web_transport_server_adapter.h
samples/web_transport_server_adapter.cc
samples/web_transport_echo_visitor.h
samples/web_transport_echo_visitor.cc
samples/web_transport_null_encrypter.h
samples/web_transport_null_encrypter.cc
samples/web_transport_null_decrypter.h
samples/web_transport_null_decrypter.cc
```

这些文件不是独立复制一整套 QUICHE，而是以对应 tools 类为模板，在 samples 中实现最小派生版本。

## 5. 运行时配置

`web_transport_crypto_config` 保存 Demo 进程级配置：

```cpp
struct WebTransportCryptoConfig {
  bool use_null_one_rtt_crypter = false;
};
```

建议由 adapter 持有配置，并在创建每条 session 时按值传入，避免使用全局变量。server 的每条连接都获得相同配置，client 的当前连接获得 client 配置。

client 和 server 增加命令行参数：

```text
--null_one_rtt_crypter=false
```

开启时必须输出安全警告：

```text
WARNING: 1-RTT Null Crypter is enabled. Application data is not confidential.
```

## 6. Client 设计

### 6.1 Client adapter

`WebTransportClientAdapter` 继承 `quic::QuicDefaultClient`，覆盖：

```cpp
std::unique_ptr<quic::QuicSession> CreateQuicClientSession(
    const quic::ParsedQuicVersionVector& supported_versions,
    quic::QuicConnection* connection) override;
```

该方法创建 `WebTransportClientSession`，不再创建默认的`QuicSimpleClientSession`。其他 socket、event loop、地址解析和连接管理继续复用
`QuicDefaultClient`。

### 6.2 Client session

`WebTransportClientSession` 继承 `quic::QuicSimpleClientSession`，覆盖：

```cpp
void OnNewEncryptionKeyAvailable(
    quic::EncryptionLevel level,
    std::unique_ptr<quic::QuicEncrypter> encrypter) override;

bool OnNewDecryptionKeyAvailable(
    quic::EncryptionLevel level,
    std::unique_ptr<quic::QuicDecrypter> decrypter,
    bool set_alternative_decrypter,
    bool latch_once_used) override;
```

当且仅当以下条件同时成立时替换对象：

```text
use_null_one_rtt_crypter == true
level == ENCRYPTION_FORWARD_SECURE
```

发送方向替换为：

```cpp
std::make_unique<quic::NullEncrypter>(
    quic::Perspective::IS_CLIENT)
```

接收方向替换为：

```cpp
std::make_unique<quic::NullDecrypter>(
    quic::Perspective::IS_CLIENT)
```

替换后必须调用原基类实现，让基类继续完成以下工作：

- 将 crypter 安装到 `QuicConnection`。
- 更新默认加密级别。
- 触发 HTTP/3 control stream 和 SETTINGS 初始化。
- 保持 TLS 和 QUIC session 状态不变。

Initial、Handshake 和其他级别直接把 TLS 创建的原始 crypter 交给基类。

## 7. Server 设计

### 7.1 Server adapter

`WebTransportServerAdapter` 继承 `quic::QuicServer`，覆盖：

```cpp
quic::QuicDispatcher* CreateQuicDispatcher() override;
```

它创建 `WebTransportDispatcher`，其他 UDP socket、event loop、proof source 和backend 行为继续复用 `QuicServer`。

### 7.2 Dispatcher

`WebTransportDispatcher` 继承 `quic::QuicSimpleDispatcher`，覆盖：

```cpp
std::unique_ptr<quic::QuicSession> CreateQuicSession(...) override;
```

实现以 `QuicSimpleDispatcher::CreateQuicSession()` 为模板，但创建`WebTransportServerSession`。连接对象的创建参数、helper、alarm factory、writer、version、connection ID generator 和 backend 必须保持与上游实现一致。

### 7.3 Server session

`WebTransportServerSession` 继承 `quic::QuicSimpleServerSession`，覆盖与 client相同的两个密钥安装回调。

1-RTT 发送方向替换为：

```cpp
std::make_unique<quic::NullEncrypter>(
    quic::Perspective::IS_SERVER)
```

1-RTT 接收方向替换为：

```cpp
std::make_unique<quic::NullDecrypter>(
    quic::Perspective::IS_SERVER)
```

替换后继续调用 server session 的基类实现。必须确认调用链仍经过`QuicSpdySession::OnNewEncryptionKeyAvailable()`，以保留 HTTP/3 SETTINGS 的发送时机。

## 8. 禁用 0-RTT

Null 模式下不允许 client 发送 early data，避免一部分 HTTP/3/WebTransport 数据仍使用标准 0-RTT 加密。

建议同时采用以下措施：

1. client adapter 不提供 session cache，构造 `QuicDefaultClient` 时传入空 cache。
2. client TLS context 显式调用 `SSL_CTX_set_early_data_enabled(..., 0)`。
3. server TLS context 同样禁用 early data，避免未来增加 session ticket 后意外接受0-RTT。
4. Null 模式下记录日志，确认连接没有使用 resumption 或 early data。

当前 client 默认构造路径已经使用空 session cache，但仍应在定制实现中明确保持，避免以后修改构造方式后重新启用 0-RTT。

## 9. Key Update

首次 1-RTT 密钥安装会经过 `OnNewEncryptionKeyAvailable()` 和`OnNewDecryptionKeyAvailable()`；后续 Key Update 则通过：

```cpp
AdvanceKeysAndCreateCurrentOneRttDecrypter()
CreateCurrentOneRttEncrypter()
```

如果不处理，Key Update 可能重新创建真实 AEAD crypter。

推荐在两个定制 session 中同时覆盖这两个方法：

- 标准模式调用基类实现。
- Null 模式返回对应 Perspective 的 `NullDecrypter` 或 `NullEncrypter`。

这比假设短时间 Demo 不会发生 Key Update 更可靠，也保留 QUIC key phase 状态机。测试阶段需要覆盖主动 Key Update 和延迟旧 key phase packet。

## 10. Sample 入口修改

`samples/web_transport_test_client.cc`：

- 定义并解析 `--null_one_rtt_crypter`。
- 使用 `WebTransportClientAdapter` 替换 `QuicDefaultClient`。
- 在 Null 模式下禁用 0-RTT。
- 输出安全警告和当前各加密级别策略。
- 每次发送 Echo 请求时输出 `echo_send` 日志。
- 每次收到服务端 Echo 响应时输出 `echo_receive` 日志，并记录内容校验结果。

`samples/web_transport_test_server.cc`：

- 定义并解析同名参数。
- 使用 `WebTransportServerAdapter` 替换 `QuicServer`。
- 把配置传给 dispatcher 和每条 server session。
- 在 Null 模式下禁用 0-RTT并输出安全警告。
- `/webtransport/echo` 使用 samples 中的 `WebTransportEchoVisitor`。
- 收到客户端 Echo 数据时输出 `echo_receive` 日志，回送数据时输出 `echo_send` 日志。

两个参数值不一致时，双方无法解密第一个 1-RTT 包，连接通常表现为 SETTINGS、CONNECT 或 session ready 超时。

## 11. Echo 收发日志

Echo client 和 server 都必须明确记录发送与接收事件，便于把程序日志与 pcap 中的
UDP/QUIC packet 对齐。不能只在一次 Echo 完成后输出汇总信息。

建议格式：

```text
[echo_send] side=client index=1 stream_id=0 length=18 payload="hello webtransport"
[echo_receive] side=server stream_id=0 length=18 payload="hello webtransport"
[echo_send] side=server stream_id=0 length=18 payload="hello webtransport"
[echo_receive] side=client index=1 stream_id=0 length=18 payload="hello webtransport" verify=ok rtt_ms=1.25
```

每条日志至少包含事件方向、client/server、stream ID、数据长度和安全显示的 payload。
client 收到完整响应后还应记录 Echo 序号、内容校验结果和 RTT。

Echo payload 可能包含二进制数据或终端控制字符，日志函数必须转义换行、制表符、
引号、反斜杠和不可打印字节。建议最多展示 256 字节；超过上限时保留原始总长度并
输出 `truncated=true`，但不能改变实际收发和校验使用的原始数据。

现有 server 使用的上游 `EchoWebTransportSessionVisitor` 主要记录 stream 事件，不能
稳定提供应用 payload 收发日志。因此在 `samples` 中增加
`WebTransportEchoVisitor`，保持原有双向流、单向流和 Datagram Echo 行为，并增加：

- 双向流读取和写回时分别记录 `echo_receive`、`echo_send`。
- 单向流读取完成和新输出单向流写回时分别记录接收、发送。
- Datagram 收到和回送时分别记录接收、发送，并增加 `transport=datagram`。
- 正确处理分片、部分写入和写阻塞，避免重复记录应用字节。

client 在成功提交 stream 写入后记录 `echo_send`；stream visitor 每次读到数据时记录
`echo_receive`。响应被分片时记录 `chunk_length` 和累计 `received_length`，收到 FIN
后再比较完整响应并输出 `verify=ok` 或 `verify=failed`。

日志默认开启，与 `--null_one_rtt_crypter` 无关，标准模式和 Null 模式使用相同格式。
Devious Baton 保持自己的协议日志，不输出 Echo 事件。

## 12. CMake 调整

Demo target 应直接编译 samples 中的定制文件：

```cmake
set(WEBTRANSPORT_DEMO_COMMON_SOURCES
  ${CMAKE_SOURCE_DIR}/samples/web_transport_crypto_config.cc
  ${CMAKE_SOURCE_DIR}/samples/web_transport_null_encrypter.cc
  ${CMAKE_SOURCE_DIR}/samples/web_transport_null_decrypter.cc
)

add_executable(web_transport_test_client
  ${CMAKE_SOURCE_DIR}/samples/web_transport_test_client.cc
  ${CMAKE_SOURCE_DIR}/samples/web_transport_client_adapter.cc
  ${CMAKE_SOURCE_DIR}/samples/web_transport_client_session.cc
  ${WEBTRANSPORT_DEMO_COMMON_SOURCES}
  # 未定制的上游 client tools sources
)

add_executable(web_transport_test_server
  ${CMAKE_SOURCE_DIR}/samples/web_transport_test_server.cc
  ${CMAKE_SOURCE_DIR}/samples/web_transport_server_adapter.cc
  ${CMAKE_SOURCE_DIR}/samples/web_transport_dispatcher.cc
  ${CMAKE_SOURCE_DIR}/samples/web_transport_server_session.cc
  ${CMAKE_SOURCE_DIR}/samples/web_transport_echo_visitor.cc
  ${WEBTRANSPORT_DEMO_COMMON_SOURCES}
  # 未定制的上游 backend/server stream tools sources
)
```

已经由 sample adapter 替代且会造成重复实现的源文件应从对应 target source list 中删除。仍由派生类复用的上游基类实现必须保留或由 `quiche::webtransport` 间接提供。

实施时需要用链接结果确认 `NullEncrypter` 和 `NullDecrypter` 已由现有库提供；如果已存在，不要再次把 `null_encrypter.cc` 和 `null_decrypter.cc` 加入 executable，避免重复符号。

## 13. 运行方式

以下示例假设当前目录为项目根目录，构建配置为 `RelWithDebInfo`，测试证书位于
`out\certs`。

标准模式启动 server：

```cmd
out\win-x64\RelWithDebInfo\web_transport_test_server.exe --port=6121 --certificate_file=out\certs\localhost.crt --key_file=out\certs\localhost.key --null_one_rtt_crypter=false
```

标准模式启动周期 Echo client：

```cmd
out\win-x64\RelWithDebInfo\web_transport_test_client.exe --null_one_rtt_crypter=false --disable_certificate_verification=true --count=10 --interval_ms=1000 --timeout_ms=5000 https://localhost:6121/webtransport/echo "hello webtransport"
```

Null 1-RTT 模式启动 server：

```cmd
out\win-x64\RelWithDebInfo\web_transport_test_server.exe --port=6121 --certificate_file=out\certs\localhost.crt --key_file=out\certs\localhost.key --null_one_rtt_crypter=true
```

Null 1-RTT 模式启动周期 Echo client：

```cmd
out\win-x64\RelWithDebInfo\web_transport_test_client.exe --null_one_rtt_crypter=true --disable_certificate_verification=true --count=10 --interval_ms=1000 --timeout_ms=5000 https://localhost:6121/webtransport/echo "hello webtransport"
```

参数说明：

| 参数 | 程序 | 说明 |
| --- | --- | --- |
| `--null_one_rtt_crypter` | client/server | `true` 使用 Null 1-RTT；两端必须一致 |
| `--port` | server | UDP 监听端口，默认 `6121` |
| `--certificate_file` | server | PEM 格式测试证书路径 |
| `--key_file` | server | PEM 格式测试私钥路径 |
| `--disable_certificate_verification` | client | 本地自签名证书测试时设为 `true` |
| `--count` | client | 周期 Echo 次数 |
| `--interval_ms` | client | 前一次 Echo 完成后到下一次 Echo 的间隔 |
| `--timeout_ms` | client | 建连、SETTINGS、session ready 和单次 Echo 的等待上限 |
| URL 位置参数 | client | WebTransport endpoint；当前 client 使用 `/webtransport/echo` |
| payload 位置参数 | client | 每次 Echo 发送的应用数据 |

该模式只能用于隔离的本地开发网络，不得用于生产环境、互联网或承载敏感数据。

## 14. 验证计划

### 14.1 模式组合

| Client | Server | 预期结果 |
| --- | --- | --- |
| 标准 | 标准 | 正常建立标准 QUIC/WebTransport 连接 |
| Null | Null | 正常建立连接，1-RTT 应用数据无机密性 |
| Null | 标准 | 1-RTT 解密失败，连接不能进入可用状态 |
| 标准 | Null | 1-RTT 解密失败，连接不能进入可用状态 |

### 14.2 功能验证

- TLS Initial 和 Handshake 正常完成。
- 没有 0-RTT packet 和 early data。
- HTTP/3 SETTINGS 正常交换。
- WebTransport Extended CONNECT 成功。
- `/webtransport/echo` 周期 Echo 正常。
- 单向流、双向流和 Datagram 正常。
- `/webtransport/devious-baton` stream 状态机正常。
- FIN、RESET_STREAM、STOP_SENDING 和 CONNECTION_CLOSE 正常。
- Key Update 后仍使用 Null Crypter。
- client 和 server 均输出成对的 `echo_send`、`echo_receive` 日志。
- client 输出最终的 `verify=ok`；内容不一致时输出 `verify=failed` 并返回非零退出码。
- 二进制、超长及包含控制字符的 payload 日志经过转义或截断。

### 14.3 抓包验证

- Wireshark仍可按标准 QUIC 解析 Initial。
- Handshake 保持标准加密。
- 1-RTT UDP payload 中能够搜索到已知 Echo 字符串。
- 1-RTT Header Protection mask 实际为零。
- 1-RTT payload 包含 Null Crypter 的 12 字节 FNV hash。
- 不应期望标准 Wireshark QUIC dissector 自动展开 Null 1-RTT frame；如需自动解析，
  需要单独的 dissector 或离线解析工具。

## 15. 实施顺序

1. 添加 sample-local crypto 配置和命令行参数。
2. 实现 client session 和 client adapter。
3. 实现 server session、dispatcher 和 server adapter。
4. 调整 CMake，使 Demo 编译 samples 中的定制实现。
5. 显式禁用 0-RTT。
6. 处理 Key Update。
7. 由用户完成编译和四种模式组合验证。
8. 完成抓包检查并补充 Demo 使用文档。

## 16. 验收标准

- `quiche/` 目录没有为该功能产生修改。
- 不开启参数时，现有 Demo 行为完全不变。
- 两端开启参数时，TLS 握手和 WebTransport session 正常建立。
- 抓包可以直接观察已知 WebTransport应用 payload。
- 抓包中不存在 0-RTT 应用数据。
- Echo client 和 server 都记录收到的对端数据及本端发送数据。
- 日志中的长度、payload 摘要和收发顺序能够与抓包对应。
- client/server 参数不一致时有明确日志或超时诊断。
- 文档明确实验模式不安全、非标准且仅用于本地分析。
