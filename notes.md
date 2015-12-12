SIMD-erizing programs
=====================

Compared performance of different implementations of `find`:

Naive realloc: 787687µs

Vectorized naive realloc: 242731µs

Realloc with a memcpy all the time: 17286µs

Smart realloc that memcopies ony when necessary: 3229µs

Write two blog articles:
------------------------

- SIMD-erizing programs to improve computational performance in plain C

Goals:
  * Leverage the use of SIMD capabilities of a modern processor to improve the
    performance of an algorithm performing the sum of two n-sized vectors
  * Introduce the `find` problem and show how SIMD can be used to improve the
    performance of naive memory movement implementation

- SIMD-erizing the already smart memory allocation algorithms of the standard C
  library

Goals:
  * Give an introduction about memory management on Linux
  * Dive into the guts of `memcpy` and `realloc`
  * Vectorize these implementations !

TODO List
---------

* Compare against the built-in realloc function that uses a couple more
optimisations just for fun (we'll lose for sure anyway) and discuss the reasons
why it's way more efficient.
* Make sure vectorizing the comparison operation sucks, just sucks...
