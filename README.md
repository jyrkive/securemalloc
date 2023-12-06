A fork of Google's [TCMalloc](https://github.com/google/tcmalloc) with a work-in-progress
implementation of allocating a dedicated page for every small allocation and, more
importantly, marking the page as inaccessible immediately when the allocation is freed.
The idea is to maximize the probability that a use-after-free crashes the process.

I'm only writing this for fun, so I don't recommend using it in production.
