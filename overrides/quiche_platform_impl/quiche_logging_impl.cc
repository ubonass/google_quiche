// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "quiche_platform_impl/quiche_logging_impl.h"

#include "absl/flags/flag.h"
#include "absl/log/absl_log.h"
#include "absl/log/initialize.h"
#include "absl/log/log_sink_registry.h"
#include "absl/strings/string_view.h"

#ifndef ABSL_VLOG
ABSL_FLAG(int, v, 0, "Show all QUICHE_VLOG(m) messages for m <= this.");
#endif

void QuicLogSink::Send(const absl::LogEntry& entry) {
  switch (entry.log_severity()) {
    case absl::LogSeverity::kInfo:
      OnLogMessage(INFO, entry.text_message());
      break;
    case absl::LogSeverity::kWarning:
      OnLogMessage(WARNING, entry.text_message());
      break;

    case absl::LogSeverity::kError:
      OnLogMessage(ERROR, entry.text_message());
      break;

    case absl::LogSeverity::kFatal:
      OnLogMessage(FATAL, entry.text_message());
      break;
    default:
      OnLogMessage(INFO, entry.text_message());
      break;
  }
}

static absl::LogSink* g_log_sink_ = nullptr;

void InitializeQuicLog(QuicLogSink* log_sink) {
  absl::InitializeLog();
  // absl::SetStderrThreshold(absl::LogSeverityAtLeast::kInfo);
  if (log_sink && !g_log_sink_) {
    g_log_sink_ = log_sink;
    absl::AddLogSink(log_sink);
  }
}

void DeInitializeQuicLog() {
  if (g_log_sink_ != nullptr) {
    absl::RemoveLogSink(g_log_sink_);
  }
}
