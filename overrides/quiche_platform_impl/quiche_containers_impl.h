// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef QUICHE_OVERRIDES_QUICHE_PLATFORM_IMPL_QUICHE_CONTAINERS_IMPL_H_
#define QUICHE_OVERRIDES_QUICHE_PLATFORM_IMPL_QUICHE_CONTAINERS_IMPL_H_

#include "quiche/common/platform/default/quiche_platform_impl/quiche_containers_impl.h"

namespace quiche {

// Represents a double-ended queue which may be backed by a list or a flat
// circular buffer.
//
// DOES NOT GUARANTEE POINTER OR ITERATOR STABILITY!
template <typename T>
using QuicheDequeImpl = std::deque<T>;

}  // namespace quiche

#endif  // QUICHE_OVERRIDES_QUICHE_PLATFORM_IMPL_QUICHE_CONTAINERS_IMPL_H_
