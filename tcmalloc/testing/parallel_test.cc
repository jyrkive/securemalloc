// Copyright 2023 The TCMalloc Authors
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

#include <atomic>
#include <cstddef>
#include <thread>
#include <vector>

#include "gtest/gtest.h"
#include "absl/base/config.h"
#include "absl/random/random.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"

extern "C" {

void* TCMallocInternalNew(size_t);
void TCMallocInternalDelete(void*);
void TCMallocInternalDeleteSized(void*, size_t);

}  // extern "C"

namespace tcmalloc {
namespace {

struct Allocator {
  Allocator(std::atomic<bool>& stop, bool do_sized_delete)
      : stop(stop), do_sized_delete(do_sized_delete) {}

  void operator()() {
    const int kNumAllocations = 65536;
    std::vector<void*> v;
    v.reserve(kNumAllocations);

    absl::BitGen rng;

    while (!stop.load(std::memory_order_acquire)) {
      const size_t size = 1u << absl::LogUniform(rng, 1, 12);
      for (int i = 0; i < kNumAllocations; ++i) {
        v.push_back(TCMallocInternalNew(size));
      }

      for (void* ptr : v) {
        if (do_sized_delete) {
          TCMallocInternalDeleteSized(ptr, size);
        } else {
          TCMallocInternalDelete(ptr);
        }
      }
      v.clear();
    }
  }

  std::atomic<bool>& stop;
  bool do_sized_delete;
};

TEST(ParallelTest, Stable) {
#ifdef ABSL_HAVE_MEMORY_SANITIZER
  // TODO(b/148986845):  Enable this.
  GTEST_SKIP() << "Skipping under msan.";
#endif
#ifdef ABSL_HAVE_THREAD_SANITIZER
  // TODO(b/274996721):  Enable this when Span::nonempty_index_ does not
  // conflict with other bitfields.
  GTEST_SKIP() << "Skipping under tsan.";
#endif
  std::atomic<bool> stop{false};
  Allocator a1(stop, /*do_sized_delete=*/true),
      a2(stop, /*do_sized_delete=*/true), a3(stop, /*do_sized_delete=*/false);

  std::thread t1(a1), t2(a2), t3(a3);

  absl::SleepFor(absl::Seconds(1));

  stop.store(true, std::memory_order_release);
  t1.join();
  t2.join();
  t3.join();
}

}  // namespace
}  // namespace tcmalloc