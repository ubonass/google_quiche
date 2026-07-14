// Copyright 2026 The quic Authors.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// rquic QUICHE logging override.
//
// QUICHE routes all logging through platform macros declared in
// quiche/common/platform/api/quiche_logging.h. By putting this directory before
// QUICHE's default platform directory in the include path, rquic can keep the
// public QUICHE macros unchanged while choosing its own implementation here.
//
// Intended behavior:
//   QUICHE_LOG / QUIC_LOG
//       Regular production logging. These are always compiled in and are
//       filtered later by the rquic SDK log sink.
//
//   QUICHE_VLOG / QUIC_VLOG
//       Verbose logging that is still available in Release builds. It is gated
//       by the Abseil --v flag. rquic owns the flag definition in
//       quiche_logging_impl.cc and the SDK updates it dynamically through
//       rquic::SetVerboseLevel().
//
//   QUICHE_DLOG / QUIC_DLOG
//       Debug-intent logging. In Debug builds this delegates to ABSL_DLOG and
//       follows normal Abseil debug-only semantics. In Release builds rquic
//       keeps the log expression compiled, but gates it with the runtime
//       quic::platform::IsDLogEnabled() switch. This is intentionally
//       a diagnostics escape hatch, not the default upstream QUICHE behavior.
//
//   QUICHE_DVLOG / QUIC_DVLOG
//       Debug-intent verbose logging. In Release builds it requires both gates:
//       the dynamic DLOG-in-Release switch and the --v verbosity level.
//
// Severity note:
//   Some QUICHE call sites use numeric severities 0/1/2. They map to
//   ERROR/WARNING/INFO. Keeping numeric aliases also avoids Windows' ERROR
//   macro interfering with code compiled after Windows headers.

#ifndef RQUIC_OVERRIDES_QUICHE_PLATFORM_IMPL_QUICHE_LOGGING_IMPL_H_
#define RQUIC_OVERRIDES_QUICHE_PLATFORM_IMPL_QUICHE_LOGGING_IMPL_H_

#include "absl/base/log_severity.h"
#include "absl/flags/declare.h"
#include "absl/flags/flag.h"
#include "absl/log/absl_check.h"
#include "absl/log/absl_log.h"

ABSL_DECLARE_FLAG(int, v);

namespace quic {
namespace platform {

// Runtime switch for rquic's Release-build DLOG override.
//
// The SDK forwards rquic::SetDLogEnabled() and rquic::IsDLogEnabled() to these
// functions. The implementation is in
// quiche_logging_impl.cc and uses an atomic, so callers may change it while the
// process is running. It only affects Release/NDEBUG builds; Debug builds still
// use ABSL_DLOG directly.
bool IsDLogEnabled();
void SetDLogEnabled(bool enabled);

}  // namespace platform
}  // namespace quic

// ---------------------------------------------------------------------------
// Production logging.
// ---------------------------------------------------------------------------
//
// QUICHE_LOG is intentionally mapped to ABSL_LOG, not to the rquic SDK sink
// directly. The SDK registers an Abseil LogSink in rquic_logging.cc, so the
// message still reaches application callbacks while preserving Abseil source
// location, prefix formatting, fatal behavior, and throttled logging support.
//
// Windows defines ERROR as 0 in WinGDI.h, so keep numeric equivalents for code
// compiled after Windows headers.
#define QUICHE_LOG_IMPL(severity) QUICHE_LOG_IMPL_##severity()
#define QUICHE_LOG_IMPL_FATAL() ABSL_LOG(FATAL)
#define QUICHE_LOG_IMPL_ERROR() ABSL_LOG(ERROR)
#define QUICHE_LOG_IMPL_WARNING() ABSL_LOG(WARNING)
#define QUICHE_LOG_IMPL_INFO() ABSL_LOG(INFO)
#define QUICHE_LOG_IMPL_0() ABSL_LOG(ERROR)
#define QUICHE_LOG_IMPL_1() ABSL_LOG(WARNING)
#define QUICHE_LOG_IMPL_2() ABSL_LOG(INFO)

#define QUICHE_PLOG_IMPL(severity) QUICHE_PLOG_IMPL_##severity()
#define QUICHE_PLOG_IMPL_FATAL() ABSL_PLOG(FATAL)
#define QUICHE_PLOG_IMPL_ERROR() ABSL_PLOG(ERROR)
#define QUICHE_PLOG_IMPL_WARNING() ABSL_PLOG(WARNING)
#define QUICHE_PLOG_IMPL_INFO() ABSL_PLOG(INFO)
#define QUICHE_PLOG_IMPL_0() ABSL_PLOG(ERROR)
#define QUICHE_PLOG_IMPL_1() ABSL_PLOG(WARNING)
#define QUICHE_PLOG_IMPL_2() ABSL_PLOG(INFO)

// ---------------------------------------------------------------------------
// Debug-intent logging.
// ---------------------------------------------------------------------------
//
// Normal QUICHE/Abseil DLOG semantics remove or disable these logs in Release
// builds. For field diagnostics rquic keeps Release DLOGs available behind a
// runtime switch. That lets the SDK enable noisy QUIC internals without
// rebuilding a Debug binary.
//
// Debug build:
//   QUICHE_DLOG(...) -> ABSL_DLOG(...)
//
// Release build:
//   QUICHE_DLOG(...) -> ABSL_LOG_IF(..., IsDLogEnabled())
//
// This means message arguments in Release are still compiled into the binary.
// Keep this override scoped to diagnostics and avoid relying on it for normal
// production signal.
#define QUICHE_DLOG_IMPL(severity) QUICHE_DLOG_IMPL_##severity()

#if defined(NDEBUG)
#define QUICHE_DLOG_IMPL_FATAL() \
  ABSL_LOG_IF(FATAL, ::quic::platform::IsDLogEnabled())
#define QUICHE_DLOG_IMPL_ERROR() \
  ABSL_LOG_IF(ERROR, ::quic::platform::IsDLogEnabled())
#define QUICHE_DLOG_IMPL_WARNING() \
  ABSL_LOG_IF(WARNING, ::quic::platform::IsDLogEnabled())
#define QUICHE_DLOG_IMPL_INFO() \
  ABSL_LOG_IF(INFO, ::quic::platform::IsDLogEnabled())
#define QUICHE_DLOG_IMPL_0() \
  ABSL_LOG_IF(ERROR, ::quic::platform::IsDLogEnabled())
#define QUICHE_DLOG_IMPL_1() \
  ABSL_LOG_IF(WARNING, ::quic::platform::IsDLogEnabled())
#define QUICHE_DLOG_IMPL_2() \
  ABSL_LOG_IF(INFO, ::quic::platform::IsDLogEnabled())
#else
#define QUICHE_DLOG_IMPL_FATAL() ABSL_DLOG(FATAL)
#define QUICHE_DLOG_IMPL_ERROR() ABSL_DLOG(ERROR)
#define QUICHE_DLOG_IMPL_WARNING() ABSL_DLOG(WARNING)
#define QUICHE_DLOG_IMPL_INFO() ABSL_DLOG(INFO)
#define QUICHE_DLOG_IMPL_0() ABSL_DLOG(ERROR)
#define QUICHE_DLOG_IMPL_1() ABSL_DLOG(WARNING)
#define QUICHE_DLOG_IMPL_2() ABSL_DLOG(INFO)
#endif

#define QUICHE_LOG_IF_IMPL(severity, condition) \
  QUICHE_LOG_IF_IMPL_##severity(condition)
#define QUICHE_LOG_IF_IMPL_FATAL(condition) ABSL_LOG_IF(FATAL, condition)
#define QUICHE_LOG_IF_IMPL_ERROR(condition) ABSL_LOG_IF(ERROR, condition)
#define QUICHE_LOG_IF_IMPL_WARNING(condition) ABSL_LOG_IF(WARNING, condition)
#define QUICHE_LOG_IF_IMPL_INFO(condition) ABSL_LOG_IF(INFO, condition)
#define QUICHE_LOG_IF_IMPL_0(condition) ABSL_LOG_IF(ERROR, condition)
#define QUICHE_LOG_IF_IMPL_1(condition) ABSL_LOG_IF(WARNING, condition)
#define QUICHE_LOG_IF_IMPL_2(condition) ABSL_LOG_IF(INFO, condition)

#define QUICHE_PLOG_IF_IMPL(severity, condition) \
  QUICHE_PLOG_IF_IMPL_##severity(condition)
#define QUICHE_PLOG_IF_IMPL_FATAL(condition) ABSL_PLOG_IF(FATAL, condition)
#define QUICHE_PLOG_IF_IMPL_ERROR(condition) ABSL_PLOG_IF(ERROR, condition)
#define QUICHE_PLOG_IF_IMPL_WARNING(condition) ABSL_PLOG_IF(WARNING, condition)
#define QUICHE_PLOG_IF_IMPL_INFO(condition) ABSL_PLOG_IF(INFO, condition)
#define QUICHE_PLOG_IF_IMPL_0(condition) ABSL_PLOG_IF(ERROR, condition)
#define QUICHE_PLOG_IF_IMPL_1(condition) ABSL_PLOG_IF(WARNING, condition)
#define QUICHE_PLOG_IF_IMPL_2(condition) ABSL_PLOG_IF(INFO, condition)

// Conditional DLOG follows the same Release-build runtime gate as plain DLOG,
// and then applies the call-site condition. The order keeps the diagnostic
// switch as the first cheap test in Release builds.
#define QUICHE_DLOG_IF_IMPL(severity, condition) \
  QUICHE_DLOG_IF_IMPL_##severity(condition)

#if defined(NDEBUG)
#define QUICHE_DLOG_IF_IMPL_FATAL(condition) \
  ABSL_LOG_IF(FATAL, (::quic::platform::IsDLogEnabled() && (condition)))
#define QUICHE_DLOG_IF_IMPL_ERROR(condition) \
  ABSL_LOG_IF(ERROR, (::quic::platform::IsDLogEnabled() && (condition)))
#define QUICHE_DLOG_IF_IMPL_WARNING(condition) \
  ABSL_LOG_IF(WARNING, (::quic::platform::IsDLogEnabled() && (condition)))
#define QUICHE_DLOG_IF_IMPL_INFO(condition) \
  ABSL_LOG_IF(INFO, (::quic::platform::IsDLogEnabled() && (condition)))
#define QUICHE_DLOG_IF_IMPL_0(condition) \
  ABSL_LOG_IF(ERROR, (::quic::platform::IsDLogEnabled() && (condition)))
#define QUICHE_DLOG_IF_IMPL_1(condition) \
  ABSL_LOG_IF(WARNING, (::quic::platform::IsDLogEnabled() && (condition)))
#define QUICHE_DLOG_IF_IMPL_2(condition) \
  ABSL_LOG_IF(INFO, (::quic::platform::IsDLogEnabled() && (condition)))
#else
#define QUICHE_DLOG_IF_IMPL_FATAL(condition) ABSL_DLOG_IF(FATAL, condition)
#define QUICHE_DLOG_IF_IMPL_ERROR(condition) ABSL_DLOG_IF(ERROR, condition)
#define QUICHE_DLOG_IF_IMPL_WARNING(condition) ABSL_DLOG_IF(WARNING, condition)
#define QUICHE_DLOG_IF_IMPL_INFO(condition) ABSL_DLOG_IF(INFO, condition)
#define QUICHE_DLOG_IF_IMPL_0(condition) ABSL_DLOG_IF(ERROR, condition)
#define QUICHE_DLOG_IF_IMPL_1(condition) ABSL_DLOG_IF(WARNING, condition)
#define QUICHE_DLOG_IF_IMPL_2(condition) ABSL_DLOG_IF(INFO, condition)
#endif

#define QUICHE_LOG_FIRST_N_IMPL(severity, n) \
  QUICHE_LOG_FIRST_N_IMPL_##severity(n)
#define QUICHE_LOG_FIRST_N_IMPL_FATAL(n) ABSL_LOG_FIRST_N(FATAL, n)
#define QUICHE_LOG_FIRST_N_IMPL_ERROR(n) ABSL_LOG_FIRST_N(ERROR, n)
#define QUICHE_LOG_FIRST_N_IMPL_WARNING(n) ABSL_LOG_FIRST_N(WARNING, n)
#define QUICHE_LOG_FIRST_N_IMPL_INFO(n) ABSL_LOG_FIRST_N(INFO, n)
#define QUICHE_LOG_FIRST_N_IMPL_0(n) ABSL_LOG_FIRST_N(ERROR, n)
#define QUICHE_LOG_FIRST_N_IMPL_1(n) ABSL_LOG_FIRST_N(WARNING, n)
#define QUICHE_LOG_FIRST_N_IMPL_2(n) ABSL_LOG_FIRST_N(INFO, n)

#define QUICHE_LOG_EVERY_N_SEC_IMPL(severity, seconds) \
  QUICHE_LOG_EVERY_N_SEC_IMPL_##severity(seconds)
#define QUICHE_LOG_EVERY_N_SEC_IMPL_FATAL(seconds) \
  ABSL_LOG_EVERY_N_SEC(FATAL, seconds)
#define QUICHE_LOG_EVERY_N_SEC_IMPL_ERROR(seconds) \
  ABSL_LOG_EVERY_N_SEC(ERROR, seconds)
#define QUICHE_LOG_EVERY_N_SEC_IMPL_WARNING(seconds) \
  ABSL_LOG_EVERY_N_SEC(WARNING, seconds)
#define QUICHE_LOG_EVERY_N_SEC_IMPL_INFO(seconds) \
  ABSL_LOG_EVERY_N_SEC(INFO, seconds)
#define QUICHE_LOG_EVERY_N_SEC_IMPL_0(seconds) \
  ABSL_LOG_EVERY_N_SEC(ERROR, seconds)
#define QUICHE_LOG_EVERY_N_SEC_IMPL_1(seconds) \
  ABSL_LOG_EVERY_N_SEC(WARNING, seconds)
#define QUICHE_LOG_EVERY_N_SEC_IMPL_2(seconds) \
  ABSL_LOG_EVERY_N_SEC(INFO, seconds)

// ---------------------------------------------------------------------------
// DFATAL compatibility.
// ---------------------------------------------------------------------------
//
// QUICHE exposes DFATAL as a pseudo-severity: fatal in Debug, error in Release.
// Abseil's public macro set used here does not provide a direct DFATAL wrapper,
// so the mapping is implemented locally. DLOG(DFATAL) participates in the same
// Release DLOG runtime switch as the other DLOG severities.
#ifdef NDEBUG
#define QUICHE_LOG_IMPL_DFATAL() ABSL_LOG(ERROR)
#define QUICHE_PLOG_IMPL_DFATAL() ABSL_PLOG(ERROR)
#define QUICHE_DLOG_IMPL_DFATAL() \
  ABSL_LOG_IF(ERROR, ::quic::platform::IsDLogEnabled())
#define QUICHE_LOG_IF_IMPL_DFATAL(condition) ABSL_LOG_IF(ERROR, condition)
#define QUICHE_PLOG_IF_IMPL_DFATAL(condition) ABSL_PLOG_IF(ERROR, condition)
#define QUICHE_DLOG_IF_IMPL_DFATAL(condition) \
  ABSL_LOG_IF(ERROR, (::quic::platform::IsDLogEnabled() && (condition)))
#define QUICHE_LOG_FIRST_N_IMPL_DFATAL(n) ABSL_LOG_FIRST_N(ERROR, n)
#define QUICHE_LOG_EVERY_N_SEC_IMPL_DFATAL(seconds) \
  ABSL_LOG_EVERY_N_SEC(ERROR, seconds)
#else
#define QUICHE_LOG_IMPL_DFATAL() ABSL_LOG(FATAL)
#define QUICHE_PLOG_IMPL_DFATAL() ABSL_PLOG(FATAL)
#define QUICHE_DLOG_IMPL_DFATAL() ABSL_DLOG(FATAL)
#define QUICHE_LOG_IF_IMPL_DFATAL(condition) ABSL_LOG_IF(FATAL, condition)
#define QUICHE_PLOG_IF_IMPL_DFATAL(condition) ABSL_PLOG_IF(FATAL, condition)
#define QUICHE_DLOG_IF_IMPL_DFATAL(condition) ABSL_DLOG_IF(FATAL, condition)
#define QUICHE_LOG_FIRST_N_IMPL_DFATAL(n) ABSL_LOG_FIRST_N(FATAL, n)
#define QUICHE_LOG_EVERY_N_SEC_IMPL_DFATAL(seconds) \
  ABSL_LOG_EVERY_N_SEC(FATAL, seconds)
#endif  // NDEBUG

// ---------------------------------------------------------------------------
// Verbose logging.
// ---------------------------------------------------------------------------
//
// The --v flag is defined in quiche_logging_impl.cc with default -1. That
// default disables all VLOGs, including VLOG(0), until the SDK changes it via
// rquic::SetVerboseLevel().
//
// QUICHE_VLOG(n):
//   Enabled when n <= FLAGS_v.
//
// QUICHE_DVLOG(n):
//   Debug build: ABSL_DLOG_IF(INFO, n <= FLAGS_v).
//   Release build: LOG_IF(INFO, IsDLogEnabled() && n <= FLAGS_v).
#define QUICHE_VLOG_PREDICATE(verbose_level) \
  (verbose_level <= absl::GetFlag(FLAGS_v))

#define QUICHE_VLOG_IMPL(verbose_level) \
  QUICHE_LOG_IF_IMPL(INFO, QUICHE_VLOG_PREDICATE(verbose_level))
#define QUICHE_VLOG_IF_IMPL(verbose_level, condition) \
  QUICHE_LOG_IF_IMPL(INFO, (QUICHE_VLOG_PREDICATE(verbose_level) && condition))
#define QUICHE_DVLOG_IMPL(verbose_level) \
  QUICHE_DLOG_IF_IMPL(INFO, QUICHE_VLOG_PREDICATE(verbose_level))
#define QUICHE_DVLOG_IF_IMPL(verbose_level, condition) \
  QUICHE_DLOG_IF_IMPL(INFO, (QUICHE_VLOG_PREDICATE(verbose_level) && condition))

#define QUICHE_LOG_INFO_IS_ON_IMPL() 1
#define QUICHE_LOG_WARNING_IS_ON_IMPL() 1
#define QUICHE_LOG_ERROR_IS_ON_IMPL() 1

// Some QUICHE code checks whether DLOG(INFO) is on before doing expensive
// diagnostic formatting. In Release builds this must mirror the runtime DLOG
// switch; in Debug builds it stays enabled to match ABSL_DLOG behavior.
#if defined(NDEBUG)
#define QUICHE_DLOG_INFO_IS_ON_IMPL() \
  (::quic::platform::IsDLogEnabled() ? 1 : 0)
#else
#define QUICHE_DLOG_INFO_IS_ON_IMPL() 1
#endif

#define QUICHE_CHECK_IMPL ABSL_CHECK
#define QUICHE_CHECK_EQ_IMPL ABSL_CHECK_EQ
#define QUICHE_CHECK_NE_IMPL ABSL_CHECK_NE
#define QUICHE_CHECK_LE_IMPL ABSL_CHECK_LE
#define QUICHE_CHECK_LT_IMPL ABSL_CHECK_LT
#define QUICHE_CHECK_GE_IMPL ABSL_CHECK_GE
#define QUICHE_CHECK_GT_IMPL ABSL_CHECK_GT
#define QUICHE_CHECK_OK_IMPL ABSL_CHECK_OK

#define QUICHE_DCHECK_IMPL ABSL_DCHECK
#define QUICHE_DCHECK_EQ_IMPL ABSL_DCHECK_EQ
#define QUICHE_DCHECK_NE_IMPL ABSL_DCHECK_NE
#define QUICHE_DCHECK_LE_IMPL ABSL_DCHECK_LE
#define QUICHE_DCHECK_LT_IMPL ABSL_DCHECK_LT
#define QUICHE_DCHECK_GE_IMPL ABSL_DCHECK_GE
#define QUICHE_DCHECK_GT_IMPL ABSL_DCHECK_GT

#define QUICHE_NOTREACHED_IMPL() QUICHE_DCHECK_IMPL(false)

#endif  // RQUIC_OVERRIDES_QUICHE_PLATFORM_IMPL_QUICHE_LOGGING_IMPL_H_
