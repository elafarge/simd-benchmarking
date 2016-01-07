SIMD-erizing programs
=====================

Notes about why parallelism might suck (and does in our case, at least if we
aren't aware of it.

When running our multithreaded program against the non multithreaded one, it
turns out that performance sucks, yeah really sucks !! It's up to three times
worse when we use several cores than when we don't.

After criticizing Linuxe's poor PThread implementations (that's not true ;-) ),
blaming it on the f****** context switches because it's really trendy to do so
at the moment, we took the thinking a bit further.

The use of the `perf stat` tool was rather useful in that matter: it can
provide the evolution of certain hardware and software metrics during the
execution of a program.

Since we had an idea about where it could be interesting to dig in, we
displayed a couple of (CPU) cache related metrics... and mutlithreading our
whole bowl introduces a HELL LOT of cache misses.

This is where it starts to get interesting. As you might know, the only CPU
cache that is shared amongst the cores of our CPU is the L3 cache.
We could visualize each running thread as a cursor going through U: with a
totally naïve multithreaded approach, each cursor would diverge and be
megabytes away from each other. The would therefore use the L3 cache in a very
selfish and unefficient way.

On the other hand we could imagine it would be damn cool if our cursors would
remain close. Why ? Because whole memory pages (4 Kilobytes) are loaded in
memory at a time, much more than the 4 to 32 bytes we compare at a time.
Therefore, it would be great if the first cursor (the one closer to the end of
U) could leave the elements it modifies at `t` in the cache so that at `t +
DT_k` (the moment where thread k works on the same 4KB chunk of U), the k-th
thread can go fetch it in L3 at worst (L2 with a bit of luck) instead of
fetching it in memory.

How to master a cache we can't directly act on ?
------------------------------------------------

The problem (actually it's a blessing more than a problem for other reasons but
anyway) is that we don't have control over the CPU cache, so we'll have to
cheat to obtain the expected behaviour. It's based on the following:

* The L3 cache as a fixed length CS (6MB in our case)
* Let's say our program can use 1/3 of it (it's really optimistic I know but we
  can always tweak that parameter later) and let's name 1/3 x, so we have xCS
  bytes of memory available for us in L3
* An integer is 4 bytes so we can put up to 500 000 ints in our L3 portion
* Therefore, if the "fastest" thread always remains at less than 500K elements
  from the slowest one, there will be only one memory fetch from RAM to L3,
  just like with the linear approach... except we run the search in parallel on
  4 cores with hyperthreading enabled on each one, so theoretically we can
  expect a more-than-4-times performance boost

### Downsides

* The multithreaded approach is really funky and cool, but at the end, we have
  to merge our `ind_vals` arrays together in the appropriate order. If we
  really have a x4 performance boost though, it won't be a huge deal at all.
* Adding instructions for threads to wait on each other decreases that
  performance boost and is not so easy to implement.

### How could we make our life much simpler ?

From our very minimalistic experience parallelizing algorithms (what's more, we
only did that in Python in the past :/) we could figure out a solution that
could work better. Instead of making the thread work on the whole U, we can
split that one into n (where n is, I'm sure you guessed, the number of threads)
adjacent chunks. For instance if `#U=100` those chunks would be `[0-24]`,
`[25-49]`, `[50-74]` and `[75-99]` (with 4 threads). This way each thread is
working on a independant chunk of data, each chunk is loaded into the cache
once (and released into memory once), we can even imagine that not only L3 will
be used but also L2 and L1, much much faster and on top of that, instead of
doing our nice-but-not-so-efficient merging of the `ind_val` arrays at the end,
we just have to concatenate them since we know they'll be ordered by design. We
just win on every aspect !!

**This is a good demonstration of why choosing the appropriate design is
essential when parallelizing algorithms.**

### How to handle the k-parameter then ?

* Just a little addition in our find methods to keep track of each counter will
  be enough
* We won't get the first `k` occurences, not even something close from it, if
  `k` is reached prematurely contrary to what we had with the other method.
