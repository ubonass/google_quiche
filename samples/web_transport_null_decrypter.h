#ifndef GOOGLE_QUICHE_SAMPLES_WEB_TRANSPORT_NULL_DECRYPTER_H_
#define GOOGLE_QUICHE_SAMPLES_WEB_TRANSPORT_NULL_DECRYPTER_H_

#include <cstddef>

#include "absl/strings/string_view.h"
#include "quiche/quic/core/crypto/null_decrypter.h"

namespace google_quiche::samples {

class WebTransportNullDecrypter : public quic::NullDecrypter {
 public:
  explicit WebTransportNullDecrypter(quic::Perspective perspective);

  bool DecryptPacket(uint64_t packet_number, absl::string_view associated_data,
                     absl::string_view ciphertext, char* output,
                     size_t* output_length,
                     size_t max_output_length) override;

 private:
  static constexpr size_t kSamplePrefixSize = 8;
};

}  // namespace google_quiche::samples

#endif  // GOOGLE_QUICHE_SAMPLES_WEB_TRANSPORT_NULL_DECRYPTER_H_
