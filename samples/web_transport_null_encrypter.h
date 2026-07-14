#ifndef GOOGLE_QUICHE_SAMPLES_WEB_TRANSPORT_NULL_ENCRYPTER_H_
#define GOOGLE_QUICHE_SAMPLES_WEB_TRANSPORT_NULL_ENCRYPTER_H_

#include <cstddef>

#include "absl/strings/string_view.h"
#include "quiche/quic/core/crypto/null_encrypter.h"

namespace google_quiche::samples {

// Adds a non-plaintext prefix to NullEncrypter output so short IETF QUIC
// packets still contain the 16-byte sample required by header protection.
class WebTransportNullEncrypter : public quic::NullEncrypter {
 public:
  explicit WebTransportNullEncrypter(quic::Perspective perspective);

  bool EncryptPacket(uint64_t packet_number, absl::string_view associated_data,
                     absl::string_view plaintext, char* output,
                     size_t* output_length,
                     size_t max_output_length) override;
  size_t GetMaxPlaintextSize(size_t ciphertext_size) const override;
  size_t GetCiphertextSize(size_t plaintext_size) const override;

 private:
  static constexpr size_t kSamplePrefixSize = 8;
};

}  // namespace google_quiche::samples

#endif  // GOOGLE_QUICHE_SAMPLES_WEB_TRANSPORT_NULL_ENCRYPTER_H_
