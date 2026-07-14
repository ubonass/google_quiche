#ifndef GOOGLE_QUICHE_SAMPLES_WEB_TRANSPORT_ECHO_VISITOR_H_
#define GOOGLE_QUICHE_SAMPLES_WEB_TRANSPORT_ECHO_VISITOR_H_

#include <cstddef>
#include <deque>
#include <memory>
#include <string>

#include "absl/strings/string_view.h"
#include "quiche/web_transport/web_transport.h"

namespace google_quiche::samples {

std::string FormatEchoPayload(absl::string_view payload,
                              size_t maximum_bytes = 256);

class WebTransportEchoVisitor : public webtransport::SessionVisitor {
 public:
  explicit WebTransportEchoVisitor(
      webtransport::Session* session,
      bool open_server_initiated_echo_stream = true);

  void OnSessionReady() override;
  void OnSessionClosed(webtransport::SessionErrorCode error_code,
                       const std::string& error_message) override;
  void OnIncomingBidirectionalStreamAvailable() override;
  void OnIncomingUnidirectionalStreamAvailable() override;
  void OnDatagramReceived(absl::string_view datagram) override;
  void OnCanCreateNewOutgoingBidirectionalStream() override;
  void OnCanCreateNewOutgoingUnidirectionalStream() override;

 private:
  void TrySendingUnidirectionalStreams();

  webtransport::Session* session_;
  bool echo_stream_opened_;
  std::deque<std::string> streams_to_echo_back_;
};

}  // namespace google_quiche::samples

#endif  // GOOGLE_QUICHE_SAMPLES_WEB_TRANSPORT_ECHO_VISITOR_H_
