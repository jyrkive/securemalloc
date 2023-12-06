#include "tcmalloc/virtual_page_allocator.h"

#include <cstddef>
#include <sys/mman.h>

namespace tcmalloc::tcmalloc_internal {

namespace {
  std::uint64_t MarkPageAllocated(std::uint64_t old_bufferused) {
    // Subtract 1 from the upper half (the number of free pages) and
    // add 1 to the lower half (the index of the first free page)
    return (old_bufferused - 0x100000000 & 0xFFFFFFFF00000000) |
      ((old_bufferused + 1) & 0xFFFFFFFF);
  }

  static const std::ptrdiff_t page_size = 4096;
}

char* VirtualPageAllocator::Allocate() {
  std::uint64_t bufferused = page_bufferused_.load(std::memory_order_relaxed);

  if (bufferused >> 32 == 0) {
    // There are no free pages left. All four million allowed simultaneous
    // allocations are in use. Let's just crash.
    std::terminate();
  }

  // Here's the magic: atomically mark the first free page as allocated.
  // Keep going until the read-modify-write operation succeeds.
  while (!page_bufferused_.compare_exchange_weak(bufferused,
    MarkPageAllocated(bufferused),
    std::memory_order_acquire, std::memory_order_relaxed)) {

    }

  // At this point, the "first free page" in the old bufferused has been marked
  // as allocated and we can safely use it for ourselves.
  std::uint32_t page_index = free_page_buffer_[bufferused & 0xFFFFFFFF];
  char* page = pages_ + page_index * page_size;

  // Mark the page as accessible.
  mprotect(page, page_size, PROT_READ | PROT_WRITE);

  // Return it.
  return page;
}

}