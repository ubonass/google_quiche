// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef QUICHE_COMMON_PLATFORM_IMPL_QUICHE_TIME_UTILS_IMPL_H_
#define QUICHE_COMMON_PLATFORM_IMPL_QUICHE_TIME_UTILS_IMPL_H_

#include <cstdint>

#include <optional>

namespace quiche {

std::optional<int64_t> QuicheUtcDateTimeToUnixSecondsImpl(int year,
                                                          int month,
                                                          int day,
                                                          int hour,
                                                          int minute,
                                                          int second);

}  // namespace quiche

#endif  // QUICHE_COMMON_PLATFORM_IMPL_QUICHE_TIME_UTILS_IMPL_H_
