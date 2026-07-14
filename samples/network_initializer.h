// Copyright 2026 The google_quiche Authors.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GOOGLE_QUICHE_SAMPLES_NETWORK_INITIALIZER_H_
#define GOOGLE_QUICHE_SAMPLES_NETWORK_INITIALIZER_H_

#if defined(_WIN32)
#include <winsock2.h>
#endif

namespace google_quiche::samples {

// Owns the process-level networking initialization required by sample tools.
// WSAStartup is reference-counted by Winsock, so this composes safely with
// another library that also initializes Winsock in the same process.
class NetworkInitializer {
 public:
  NetworkInitializer() {
#if defined(_WIN32)
    error_code_ = WSAStartup(MAKEWORD(2, 2), &wsa_data_);
    initialized_ = error_code_ == 0;
#endif
  }

  NetworkInitializer(const NetworkInitializer&) = delete;
  NetworkInitializer& operator=(const NetworkInitializer&) = delete;

  ~NetworkInitializer() {
#if defined(_WIN32)
    if (initialized_) {
      WSACleanup();
    }
#endif
  }

  bool ok() const {
#if defined(_WIN32)
    return initialized_;
#else
    return true;
#endif
  }

  int error_code() const {
#if defined(_WIN32)
    return error_code_;
#else
    return 0;
#endif
  }

 private:
#if defined(_WIN32)
  WSADATA wsa_data_{};
  int error_code_ = 0;
  bool initialized_ = false;
#endif
};

}  // namespace google_quiche::samples

#endif  // GOOGLE_QUICHE_SAMPLES_NETWORK_INITIALIZER_H_
