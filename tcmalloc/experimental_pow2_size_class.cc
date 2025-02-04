// Copyright 2019 The TCMalloc Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "absl/types/span.h"
#include "tcmalloc/common.h"
#include "tcmalloc/internal/config.h"
#include "tcmalloc/size_class_info.h"
#include "tcmalloc/sizemap.h"

GOOGLE_MALLOC_SECTION_BEGIN
namespace tcmalloc {

namespace tcmalloc_internal {

// <fixed> is fixed per-size-class overhead due to end-of-span fragmentation
// and other factors. For instance, if we have a 96 byte size class, and use a
// single 8KiB page, then we will hold 85 objects per span, and have 32 bytes
// left over. There is also a fixed component of 48 bytes of TCMalloc metadata
// per span. Together, the fixed overhead would be wasted/allocated =
// (32 + 48) / (8192 - 32) ~= 0.98%.
// There is also a dynamic component to overhead based on mismatches between the
// number of bytes requested and the number of bytes provided by the size class.
// Together they sum to the total overhead; for instance if you asked for a
// 50-byte allocation that rounds up to a 64-byte size class, the dynamic
// overhead would be 28%, and if <fixed> were 22% it would mean (on average)
// 25 bytes of overhead for allocations of that size.

// clang-format off
#if defined(__cpp_aligned_new) && __STDCPP_DEFAULT_NEW_ALIGNMENT__ <= 8
#if TCMALLOC_PAGE_SHIFT == 13
static_assert(kMaxSize == 262144, "kMaxSize mismatch");
static const int kCount = 17;
static_assert(kCount <= kNumBaseClasses);
static constexpr SizeClassInfo kExperimentalPow2SizeClassesList[kCount] = {
    // <bytes>, <pages>, <batch>, <capacity>    <fixed>
    {        0,       0,       0,          0},  // +Inf%
    {        8,       1,      32,       4712},  // 0.59%
    {       16,       1,      32,       4712},  // 0.59%
    {       32,       1,      32,       4713},  // 0.59%
    {       64,       1,      32,       4712},  // 0.59%
    {      128,       1,      32,       4712},  // 0.59%
    {      256,       1,      32,       2427},  // 0.59%
    {      512,       1,      32,       1337},  // 0.59%
    {     1024,       1,      32,        789},  // 0.59%
    {     2048,       2,      32,        513},  // 0.29%
    {     4096,       1,      16,        529},  // 0.59%
    {     8192,       1,       8,        384},  // 0.59%
    {    16384,       2,       4,        320},  // 0.29%
    {    32768,       4,       2,        318},  // 0.15%
    {    65536,       8,       2,        301},  // 0.07%
    {   131072,      16,       2,        299},  // 0.04%
    {   262144,      32,       2,        294},  // 0.02%
};
constexpr absl::Span<const SizeClassInfo> kExperimentalPow2SizeClasses(kExperimentalPow2SizeClassesList);
#elif TCMALLOC_PAGE_SHIFT == 15
static_assert(kMaxSize == 262144, "kMaxSize mismatch");
static const int kCount = 17;
static_assert(kCount <= kNumBaseClasses);
static constexpr SizeClassInfo kExperimentalPow2SizeClassesList[kCount] = {
    // <bytes>, <pages>, <batch>, <capacity>    <fixed>
    {        0,       0,       0,          0},  // +Inf%
    {        8,       1,      32,       4795},  // 0.15%
    {       16,       1,      32,       4795},  // 0.15%
    {       32,       1,      32,       4795},  // 0.15%
    {       64,       1,      32,       4795},  // 0.15%
    {      128,       1,      32,       4795},  // 0.15%
    {      256,       1,      32,       2276},  // 0.15%
    {      512,       1,      32,       1114},  // 0.15%
    {     1024,       1,      32,        780},  // 0.15%
    {     2048,       1,      32,        499},  // 0.15%
    {     4096,       1,      16,        492},  // 0.15%
    {     8192,       1,       8,        361},  // 0.15%
    {    16384,       1,       4,        332},  // 0.15%
    {    32768,       1,       2,        321},  // 0.15%
    {    65536,       2,       2,        307},  // 0.07%
    {   131072,       4,       2,        316},  // 0.04%
    {   262144,       8,       2,        299},  // 0.02%
};
constexpr absl::Span<const SizeClassInfo> kExperimentalPow2SizeClasses(kExperimentalPow2SizeClassesList);
#elif TCMALLOC_PAGE_SHIFT == 18
static_assert(kMaxSize == 262144, "kMaxSize mismatch");
static const int kCount = 17;
static_assert(kCount <= kNumBaseClasses);
static constexpr SizeClassInfo kExperimentalPow2SizeClassesList[kCount] = {
    // <bytes>, <pages>, <batch>, <capacity>    <fixed>
    {        0,       0,       0,          0},  // +Inf%
    {        8,       1,      32,       4635},  // 0.02%
    {       16,       1,      32,       4635},  // 0.02%
    {       32,       1,      32,       4635},  // 0.02%
    {       64,       1,      32,       4635},  // 0.02%
    {      128,       1,      32,       4635},  // 0.02%
    {      256,       1,      32,       2573},  // 0.02%
    {      512,       1,      32,       1405},  // 0.02%
    {     1024,       1,      32,        788},  // 0.02%
    {     2048,       1,      32,        600},  // 0.02%
    {     4096,       1,      16,        613},  // 0.02%
    {     8192,       1,       8,        378},  // 0.02%
    {    16384,       1,       4,        328},  // 0.02%
    {    32768,       1,       2,        339},  // 0.02%
    {    65536,       1,       2,        294},  // 0.02%
    {   131072,       1,       2,        289},  // 0.02%
    {   262144,       1,       2,        290},  // 0.02%
};
constexpr absl::Span<const SizeClassInfo> kExperimentalPow2SizeClasses(kExperimentalPow2SizeClassesList);
#elif TCMALLOC_PAGE_SHIFT == 12
static_assert(kMaxSize == 8192, "kMaxSize mismatch");
static const int kCount = 12;
static_assert(kCount <= kNumBaseClasses);
static constexpr SizeClassInfo kExperimentalPow2SizeClassesList[kCount] = {
    // <bytes>, <pages>, <batch>, <capacity>    <fixed>
    {        0,       0,       0,          0},  // +Inf%
    {        8,       1,      32,       4840},  // 1.17%
    {       16,       1,      32,       4840},  // 1.17%
    {       32,       1,      32,       4840},  // 1.17%
    {       64,       1,      32,       4840},  // 1.17%
    {      128,       1,      32,       4840},  // 1.17%
    {      256,       1,      32,       4050},  // 1.17%
    {      512,       1,      32,       1491},  // 1.17%
    {     1024,       2,      32,       1425},  // 0.59%
    {     2048,       4,      32,        612},  // 0.29%
    {     4096,       4,      16,        666},  // 0.29%
    {     8192,       4,       8,        302},  // 0.29%
};
constexpr absl::Span<const SizeClassInfo> kExperimentalPow2SizeClasses(kExperimentalPow2SizeClassesList);
#else
#error "Unsupported TCMALLOC_PAGE_SHIFT value!"
#endif
#else
#if TCMALLOC_PAGE_SHIFT == 13
static_assert(kMaxSize == 262144, "kMaxSize mismatch");
static const int kCount = 17;
static_assert(kCount <= kNumBaseClasses);
static constexpr SizeClassInfo kExperimentalPow2SizeClassesList[kCount] = {
    // <bytes>, <pages>, <batch>, <capacity>    <fixed>
    {        0,       0,       0,          0},  // +Inf%
    {        8,       1,      32,       4712},  // 0.59%
    {       16,       1,      32,       4712},  // 0.59%
    {       32,       1,      32,       4713},  // 0.59%
    {       64,       1,      32,       4712},  // 0.59%
    {      128,       1,      32,       4712},  // 0.59%
    {      256,       1,      32,       2427},  // 0.59%
    {      512,       1,      32,       1337},  // 0.59%
    {     1024,       1,      32,        789},  // 0.59%
    {     2048,       2,      32,        513},  // 0.29%
    {     4096,       1,      16,        529},  // 0.59%
    {     8192,       1,       8,        384},  // 0.59%
    {    16384,       2,       4,        320},  // 0.29%
    {    32768,       4,       2,        318},  // 0.15%
    {    65536,       8,       2,        301},  // 0.07%
    {   131072,      16,       2,        299},  // 0.04%
    {   262144,      32,       2,        294},  // 0.02%
};
constexpr absl::Span<const SizeClassInfo> kExperimentalPow2SizeClasses(kExperimentalPow2SizeClassesList);
#elif TCMALLOC_PAGE_SHIFT == 15
static_assert(kMaxSize == 262144, "kMaxSize mismatch");
static const int kCount = 17;
static_assert(kCount <= kNumBaseClasses);
static constexpr SizeClassInfo kExperimentalPow2SizeClassesList[kCount] = {
    // <bytes>, <pages>, <batch>, <capacity>    <fixed>
    {        0,       0,       0,          0},  // +Inf%
    {        8,       1,      32,       4795},  // 0.15%
    {       16,       1,      32,       4795},  // 0.15%
    {       32,       1,      32,       4795},  // 0.15%
    {       64,       1,      32,       4795},  // 0.15%
    {      128,       1,      32,       4795},  // 0.15%
    {      256,       1,      32,       2276},  // 0.15%
    {      512,       1,      32,       1114},  // 0.15%
    {     1024,       1,      32,        780},  // 0.15%
    {     2048,       1,      32,        499},  // 0.15%
    {     4096,       1,      16,        492},  // 0.15%
    {     8192,       1,       8,        361},  // 0.15%
    {    16384,       1,       4,        332},  // 0.15%
    {    32768,       1,       2,        321},  // 0.15%
    {    65536,       2,       2,        307},  // 0.07%
    {   131072,       4,       2,        316},  // 0.04%
    {   262144,       8,       2,        299},  // 0.02%
};
constexpr absl::Span<const SizeClassInfo> kExperimentalPow2SizeClasses(kExperimentalPow2SizeClassesList);
#elif TCMALLOC_PAGE_SHIFT == 18
static_assert(kMaxSize == 262144, "kMaxSize mismatch");
static const int kCount = 17;
static_assert(kCount <= kNumBaseClasses);
static constexpr SizeClassInfo kExperimentalPow2SizeClassesList[kCount] = {
    // <bytes>, <pages>, <batch>, <capacity>    <fixed>
    {        0,       0,       0,          0},  // +Inf%
    {        8,       1,      32,       4635},  // 0.02%
    {       16,       1,      32,       4635},  // 0.02%
    {       32,       1,      32,       4635},  // 0.02%
    {       64,       1,      32,       4635},  // 0.02%
    {      128,       1,      32,       4635},  // 0.02%
    {      256,       1,      32,       2573},  // 0.02%
    {      512,       1,      32,       1405},  // 0.02%
    {     1024,       1,      32,        788},  // 0.02%
    {     2048,       1,      32,        600},  // 0.02%
    {     4096,       1,      16,        613},  // 0.02%
    {     8192,       1,       8,        378},  // 0.02%
    {    16384,       1,       4,        328},  // 0.02%
    {    32768,       1,       2,        339},  // 0.02%
    {    65536,       1,       2,        294},  // 0.02%
    {   131072,       1,       2,        289},  // 0.02%
    {   262144,       1,       2,        290},  // 0.02%
};
constexpr absl::Span<const SizeClassInfo> kExperimentalPow2SizeClasses(kExperimentalPow2SizeClassesList);
#elif TCMALLOC_PAGE_SHIFT == 12
static_assert(kMaxSize == 8192, "kMaxSize mismatch");
static const int kCount = 12;
static_assert(kCount <= kNumBaseClasses);
static constexpr SizeClassInfo kExperimentalPow2SizeClassesList[kCount] = {
    // <bytes>, <pages>, <batch>, <capacity>    <fixed>
    {        0,       0,       0,          0},  // +Inf%
    {        8,       1,      32,       4840},  // 1.17%
    {       16,       1,      32,       4840},  // 1.17%
    {       32,       1,      32,       4840},  // 1.17%
    {       64,       1,      32,       4840},  // 1.17%
    {      128,       1,      32,       4840},  // 1.17%
    {      256,       1,      32,       4050},  // 1.17%
    {      512,       1,      32,       1491},  // 1.17%
    {     1024,       2,      32,       1425},  // 0.59%
    {     2048,       4,      32,        612},  // 0.29%
    {     4096,       4,      16,        666},  // 0.29%
    {     8192,       4,       8,        302},  // 0.29%
};
constexpr absl::Span<const SizeClassInfo> kExperimentalPow2SizeClasses(kExperimentalPow2SizeClassesList);
#else
#error "Unsupported TCMALLOC_PAGE_SHIFT value!"
#endif
#endif
// clang-format on

}  // namespace tcmalloc_internal
}  // namespace tcmalloc
GOOGLE_MALLOC_SECTION_END
