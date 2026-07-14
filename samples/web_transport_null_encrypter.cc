#include "samples/web_transport_null_encrypter.h"

#include <cstring>
#include <string>

namespace google_quiche::samples {

WebTransportNullEncrypter::WebTransportNullEncrypter(
    quic::Perspective perspective)
    : quic::NullEncrypter(perspective) {}

bool WebTransportNullEncrypter::EncryptPacket(
    uint64_t packet_number, absl::string_view associated_data,
    absl::string_view plaintext, char* output, size_t* output_length,
    size_t max_output_length) {
  if (max_output_length < kSamplePrefixSize) {
    return false;
  }
  // QuicPacketCreator may encrypt in place. Preserve the QUIC plaintext before
  // writing the extra prefix into the output buffer.
  const std::string plaintext_copy(plaintext);
  std::memset(output, 0, kSamplePrefixSize);
  size_t null_output_length = 0;
  if (!quic::NullEncrypter::EncryptPacket(
          packet_number, associated_data, plaintext_copy,
          output + kSamplePrefixSize, &null_output_length,
          max_output_length - kSamplePrefixSize)) {
    return false;
  }
  *output_length = kSamplePrefixSize + null_output_length;
  return true;
}

size_t WebTransportNullEncrypter::GetCiphertextSize(
    size_t plaintext_size) const {
  return kSamplePrefixSize +
         quic::NullEncrypter::GetCiphertextSize(plaintext_size);
}

size_t WebTransportNullEncrypter::GetMaxPlaintextSize(
    size_t ciphertext_size) const {
  if (ciphertext_size < kSamplePrefixSize) {
    return 0;
  }
  return quic::NullEncrypter::GetMaxPlaintextSize(ciphertext_size -
                                                   kSamplePrefixSize);
}

}  // namespace google_quiche::samples
