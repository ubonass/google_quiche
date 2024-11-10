// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file implements logging using Abseil.

#ifndef QUICHE_OVERRIDES_QUICHE_PLATFORM_IMPL_QUICHE_LOGGING_IMPL_H_
#define QUICHE_OVERRIDES_QUICHE_PLATFORM_IMPL_QUICHE_LOGGING_IMPL_H_

#include "quiche/common/platform/default/quiche_platform_impl/quiche_logging_impl.h"

enum QuicLogLevel {
  VERBOSE = 0,
  DEBUG = 1,
  INFO = 2,
  WARNING = 3,
  ERROR = 4,
  FATAL = 5
};

// QuicLogSink is used to capture logs emitted from the QUICHE_LOG... macros.
class QuicLogSink : public absl::LogSink {
 public:
  virtual ~QuicLogSink() = default;
  // Called when |message| is emitted at |level|.
  virtual void OnLogMessage(QuicLogLevel level, absl::string_view message) = 0;

 protected:
  void Send(const absl::LogEntry& entry);
};

void InitializeQuicLog(QuicLogSink* log_sink);

void DeInitializeQuicLog();

#endif  // QUICHE_OVERRIDES_QUICHE_PLATFORM_IMPL_QUICHE_LOGGING_IMPL_H_
