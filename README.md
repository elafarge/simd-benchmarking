# simd-benchmarking
A simple C program along with a Python script aiming at benchmarking SIMD and
multi-threaded computing versus a naïve, mono-threaded implementation.

## Compiling instructions

To compile the C program with GCC, simply run

```shell
gcc simdbmk.c -msse -lpthread -o simdbmk
```

This will create a binary named `simbdk` in the current folder. This binary can
be run with a various set of options that can be listed using the

```shell
./simdbmk --help
```

## Program description

Here's a basic description of what the program does:

### Goals

Given a *vector* of integers `U`, the core of the program is a `find` function
that returns the number of occurences of a given integer `val` between the
indexes `a_start` and `a_end` in the array. `find` will also allocate an array
containing the indexes where `val` was found in the array. By design, we were
asked to use standard C-arrays and only those (in particular, using linked
lists would indeed be much more efficient here but doing it with arrays makes
the use of SIMD functionnalities of processors to manipulate memory much more
efficient).

#### Dynamic reallocation of the result array is our main concern.

The algorithm for `find` is rather simple on the paper: we have no choice but
to run through every element of the array and compare it to the seeked value.
But what if we got a match ? Well, we need to increase our counter of matches
and... add the index of the match to our array... except that it is an array
and has therefore a fixed size. Here comes the performance-critical bit of our
application: we'll need to reallocate the entire array of results.

**There's a built-in C function for that**: `realloc`. Using it gives us
excellent results. After digging into the function's code (put a link there),
it turns out that it's using every possible optimisation: at first it checks if
there's enough space available in memory right after the preexisting array and
if so, it just reserve the next 4 bytes to welcome our new index: it's an
awesome constant-time operation and from the couple of tests we conducted, it
seems to happen most of the time and results in a tremendous performance
improvement. If it can do so, then a reallocation takes place which, given that
the distribution of integers in U is uniform, has a `O(m)` complexity where `m`
is the size of `U`. To perform that reallocation, the built-in `realloc`
function leverages SIMD capacities, which doesn't leave much room for
improvement on our side.

#### Creating a naive version of `realloc`

For the purpose of our study, we therefore re-implemented a totally naive
version of realloc, which does a copy of the whole array all the time and
copies the element one at a time. We named that one `simple_realloc`.

#### SIMDerizing our `simple_realloc` to make it a bit more efficient

Based on that naïve version of `realloc`, we built another one that copies the
elements 4 by 4, leveraging SIMD capabilities of our processor. It's been
called `vect_realloc` and the `vect_find` implementation of `find` uses it.

#### Parallelizing the whole thing to boost copy performance

Not only our modern processors have some basic (but yet pretty efficient)
vectorizing capabilities that can help us boost the performance of many
algorithm but they also cointain multiple cores (4 physical cores on our `i7`
test processor and 8 virtual ones since it has hyperthreading capabilities).
Therefore, we wrote two implementations of `find` that can be run on multiple
cores (they're just `pthread` compliant functions).

A wrapper for these named `thread_find` is going to take care of starting the
threads (we'll start `c` threads where `c` designated the number of cores seen
by the system, we're aware that `c+1` is usually the chosen value but it
doesn't have a huge academic importance here) over different chunks of the
initial array (each thread is going to take care of the `c*k+i` indexed
elements where `i` varies between `0` and `c` (excluded) and is different for
each started thread.

As the end, the wrapper groups the `c` array produced by each thread. In order
to obtain the exact same results as for the previous versions of `find` we
wanted the result array to be ordered as well. Using a standard sorting
algorithm (like the quick-sort) would result a complexity of `O(n*log(n))` but
we managed to narrow it down to `O(n)` by taking into account the fact that
each subarray was already sorted.

#### Program description

What the program does when it's ran is pretty clear since the steps are printed
on `stdout` as they are ran. We'll sum it up there quickly:

* Generate a random array of integers containing values between `a` and `b`
* Run the naive `find` on it
* Run it's vectorial counterpart and measure the performance gain.
* Run the non vectorial parralel version of `find` and compare it against the
  naive one
* Run the vectorized, parallel version of `find` and compare it against the
  naive `find` as well as the non-vectorial, parallel version of `find`


## The lab environment for the tests

There's one variable that we mainly want to play with during our tests: the
number of elements in the initial array `U`, therefore we've made it a
parameter of our C-program (as well as a bunch of other parameter we wanted to
play with "for fun").

Having the ability to pass parameters when calling our program on the command
line made it possible for us to write a script that would run `simdbmk` several
times with different values of `n` (where `n = #(U)`), parse the results, put
that in a CSV spreadsheet and display graphs allowing us to analyse the
performance gains further.

This script has been written in Python for coding efficiency's sake and also
take one parameter: a list of `floats` standing for the base-10 `log` of the
number of elements in the array.

Here as well,it's just the matter of a command line to run :

```shell
python ./benchmark.py 3 3.5 4
```

In that example, `simdbmk` will be ran for `n = 1000`, `n = 10^3.5` and `n =
10000`.

### Mind the RAM

Before running the program for extremely high values of `n`, please consider
the available amount of RAM that you have. Basically, the element that will
determing the amount of memory needed by our program is `U`. If you don't want
to kill yourself when your computer starts swapping, you therefore have to make
sure that `#(U)*sizeof(int)` stays inferior to the available amount of RAM.

Let's assume we don't want to go over 4GB of RAM occupation (our testing
computer has 8 gigs of RAM, this value seems fair) , since an `int` weighs 4
bytes, we can put up to **1 billion** ints in `U`.

Also, mind that it take around **1 hour** for the program to run on our test
machine with `n = 10⁸` (so it should take around 11 hours to run it with `n =
10^9`, the compelxity being O(n). We never tried that honestly... the interest
of it is rather limited).

## Results

<TODO>

## Questions about different aspects of the program

Here are some questions that we had to reply to regarding that project:

### Generic questions about the naive implementations

#### We do we need a pointer of pointer for `ind_val`

What we need to do is populate an array, which is represented by a pointer
(*int_val) and a size (called c and returned by the function find). At first
you'd think that just passing a pointer and make it change would be fine,
but the point is on a function call, the arguments are copied in memory.
Therefore, it's that copy of the pointer that will be modified by find, not
the pointer itself. That's why we need to get a pointer pointing on that
pointer: therefore the pointer gives us the address of the pointer pointing
at the beginning of the array and we we can use this "address" to actually
modify the array every time we call realloc.

In short the answer is: "because arguments get copied when a function is
called so we need to use pointers, and since the argument itself is a
pointer, we need a pointer on a pointer".

#### How to look only for even (resp. odd) positions in U ?

That could simply be achieved by using the following values for `find`'s
arguments:
* `i_start = 0` (resp. `i_start = 1`)
* `i_end = #(U)` (in both cases)
* `i_step = 2` (in both cases)

#### Why would one want to do that ?

Well, we didn't think of an earth-bound application for that to be honest. But
we have one regarding the study: Introducing that `i_start` and `i_step`
parameter actualy provides us with a good way to make our algorithm parallel:
you just have to set `c = i_step = number of worker threads` and have `i_start
= 0 ... i_step - 1` (one different value for each thread). That's not the only
way (we could also split our array into `c` contiguous parts for instance) but
that's one of them.

### Questions about the multi-threaded approach

#### Describe the data structure of a (POSIX) thread

<TODO>

#### The k-factor, or how do we make our running threads talk to each other ?

<TODO>

## Authors

* Etienne Lafarge (etienne.lafarge**_at_**mines-paristech.fr)
* Vincent Villet (vincent.villet**_at_**mines-paristech.fr)

Special thanks to our teacher, Claude Tadonki.
