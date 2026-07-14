#include "samples/web_transport_null_decrypter.h"

#include <string>

#include "quiche/quic/platform/api/quic_logging.h"

namespace google_quiche::samples {

WebTransportNullDecrypter::WebTransportNullDecrypter(
    quic::Perspective perspective)
    : quic::NullDecrypter(perspective) {}

bool WebTransportNullDecrypter::DecryptPacket(
    uint64_t packet_number, absl::string_view associated_data,
    absl::string_view ciphertext, char* output, size_t* output_length,
    size_t max_output_length) {
  if (ciphertext.size() < kSamplePrefixSize) {
    QUIC_LOG(ERROR) << "Null 1-RTT ciphertext is shorter than the sample "
                       "prefix: "
                    << ciphertext.size();
    return false;
  }
  // QuicFramer may decrypt in place. After removing our extra prefix, the
  // native NullDecrypter would copy from an overlapping region with memcpy.
  // Keep its input separate so the restored QUIC plaintext is byte-exact.
  const std::string null_ciphertext(ciphertext.substr(kSamplePrefixSize));
  const bool success = quic::NullDecrypter::DecryptPacket(
      packet_number, associated_data, null_ciphertext, output, output_length,
      max_output_length);
  if (!success) {
    QUIC_LOG(ERROR) << "Null 1-RTT FNV verification failed for packet "
                    << packet_number << ", ciphertext length "
                    << ciphertext.size();
  }
  return success;
}

}  // namespace google_quiche::samples
