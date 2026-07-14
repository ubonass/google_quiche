// Copyright 2026 The rquic Authors.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "quiche_platform_impl/quiche_logging_impl.h"

#include <atomic>

#include "absl/flags/flag.h"

// Show all QUICHE_VLOG(m) messages for m <= this. The default -1 disables all
// verbose logs, including QUICHE_VLOG(0). QUICHE_DVLOG uses the same verbosity
// gate, then additionally follows debug-only DLOG semantics.
ABSL_FLAG(int, v, -1, "Show all QUICHE_VLOG(m) messages for m <= this.");

namespace quic {
namespace platform {

namespace {

// Temporary diagnostics default: preserve the previous compile-time enabled
// behavior. SDK callers can change it at runtime.
std::atomic_bool g_dlog_enabled{false};

}  // namespace

bool IsDLogEnabled() {
  return g_dlog_enabled.load(std::memory_order_relaxed);
}

void SetDLogEnabled(bool enabled) {
  g_dlog_enabled.store(enabled, std::memory_order_relaxed);
}

}  // namespace platform
}  // namespace quic
