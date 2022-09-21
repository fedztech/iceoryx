// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IOX_HOOFS_PLATFORM_BUILDING_BLOCKS_LOGCOMMON_HPP
#define IOX_HOOFS_PLATFORM_BUILDING_BLOCKS_LOGCOMMON_HPP

#include <cstdint>

namespace iox
{
namespace pbb
{
enum class LogLevel : uint8_t
{
    OFF = 0,
    FATAL,
    ERROR,
    WARN,
    INFO,
    DEBUG,
    TRACE
};

/// @brief converts LogLevel into a string literal
/// @return string literal of the LogLevel value
inline constexpr const char* asStringLiteral(const LogLevel value) noexcept;

/// @brief converts LogLevel into a string literal color code
/// @return string literal of the corresponding color code
inline constexpr const char* logLevelDisplayColor(const LogLevel value) noexcept;

/// @brief converts LogLevel into a string literal display text
/// @return string literal of the display text
inline constexpr const char* logLevelDisplayText(const LogLevel value) noexcept;

struct LogBuffer
{
    const char* buffer;
    uint64_t writeIndex;
};

} // namespace pbb
} // namespace iox

#include "iceoryx_hoofs/internal/log/platform_building_blocks/logcommon.inl"

#endif // IOX_HOOFS_PLATFORM_BUILDING_BLOCKS_CONSOLE_LOGGER_HPP