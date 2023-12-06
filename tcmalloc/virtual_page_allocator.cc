#include "tcmalloc/virtual_page_allocator.h"

#include <cstddef>
#include <sys/mman.h>

namespace tcmalloc::tcmalloc_internal {

namespace {
  std::uint64_t MarkPageAllocated(std::uint64_t old_bufferused) {
    // Subtract 1 from the upper half (the number of free pages) and
    // add 1 to the lower half (the index of the first free page), and
    // mask the lower half in case of an overflow.
    return (old_bufferused - 0x100000000 & 0xFFFFFFFF00000000) |
      ((old_bufferused + 1) & 0x003FFFFF);
  }

  static const std::ptrdiff_t page_size = 4096;
  static constexpr std::uint32_t num_buffer_slots = 4 << 20;
  static constexpr std::uint32_t flag_allocated = static_cast<std::uint32_t>(1) << 31;
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
    MarkPageAllocated(bufferused), std::memory_order_relaxed)) {

    }

  // At this point, the "first free page" in the old bufferused has been marked
  // as allocated and we can safely use it for ourselves.

  // Truncating bufferused to the bottom 32 bits gives the index of the first
  // allocated slot. The format has intentionally been designed to allow this.
  auto buffer_index = static_cast<std::uint32_t>(bufferused);
  std::uint32_t page_index;
  /* We loop here because it's possible (although it should be vanishingly unlikely)
  that this page was only just freed by another thread, and it has marked the slot in
  the ring buffer as allocated but hasn't yet written the page index into the buffer.
  We know this from the page index having the "allocated" flag set, which is
  impossible for free pages. */
  do {
    page_index = free_page_buffer_[buffer_index].
      load(std::memory_order_relaxed);
  } while (page_index & flag_allocated != 0)

  // Mark the page as allocated to ensure that the above loop works if another thread
  // attempts to allocate a page from this slot later.
  free_page_buffer_[buffer_index].
    store(page_index | flag_allocated, std::memory_order_relaxed);

  char* page = pages_ + page_index * page_size;

  // Mark the page as accessible.
  mprotect(page, page_size, PROT_READ | PROT_WRITE);

  // Return it.
  return page;
}

void VirtualPageAllocator::Free(char* page) {
  // Mark the memory as inaccessible.
  mprotect(page, page_size, PROT_NONE);

  std::uint32_t page_index = (page - pages_) / page_size;

  // Atomically increase the number of free pages by 1 and fetch the previous value of
  // page_bufferused_, from which we can compute the first index that was free before
  // this modification.
  std::uint64_t bufferused = page_bufferused_.
    fetch_add(0x100000000, std::memory_order_relaxed);

  /* We get the index of the first free slot by adding the first used index
  (bottom 32 bits) and the number of used elements (top 32 bits). Masking isn't used:
  this calculation would give a wrong answer without masking if it overflowed, but it
  cannot, because the buffer is restricted to four million elements, well below the
  limit for unsigned 32-bit numbers. Hence, omitting masking is safe. */
  auto buffer_index = (bufferused << 32 + bufferused) % num_buffer_slots;

  // Write the page index into the ring buffer.
  free_page_buffer_[buffer_index].
    store(page_index, std::memory_order_relaxed);
}

}