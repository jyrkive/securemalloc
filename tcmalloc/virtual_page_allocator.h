#pragma once

#include <atomic>
#include <cstdint>

#include "tcmalloc/pages.h"

namespace tcmalloc::tcmalloc_internal {

class VirtualPageAllocator {
  public:
    VirtualPageAllocator();

    /* Allocate a page. */
    char* Allocate();

    // Free a page.
    void Free(char* page);

  private:
    /* All the virtual pages this allocator manages.
    64 GB of virtual address space, but the pages only become mapped if they're
    allocated. */
    char* pages_;

    /* A ring buffer that stores indices of all free pages.
    page_bufferused_ stores the necessary information to determine which parts
    of the ring buffer are in use.
    The bottom 24 bits are the actual index: the top 8 bits are used for flags.
    Bit 31: allocated
    Bits 24-30: (reserved for future expansion) */
    std::atomic<std::uint32_t>* free_page_buffer_;

    /* Stores the first used index (bottom 32 bits) and number of used elements
    (top 32 bits) in free_page_buffer_. The information is packed into a single
    uint64_t to ensure both pieces of information can be loaded/stored
    atomically. */
    std::atomic<std::uint64_t> page_bufferused_;
};

}