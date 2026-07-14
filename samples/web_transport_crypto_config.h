#ifndef GOOGLE_QUICHE_SAMPLES_WEB_TRANSPORT_CRYPTO_CONFIG_H_
#define GOOGLE_QUICHE_SAMPLES_WEB_TRANSPORT_CRYPTO_CONFIG_H_

namespace google_quiche::samples {

struct WebTransportCryptoConfig {
  bool use_null_one_rtt_crypter = false;
};

}  // namespace google_quiche::samples

#endif  // GOOGLE_QUICHE_SAMPLES_WEB_TRANSPORT_CRYPTO_CONFIG_H_
