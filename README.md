# simd-benchmarking #
A simple C program along with a Python script aiming at benchmarking SIMD and
multi-threaded computing versus a naïve, mono-threaded implementation.

## Compiling instructions ##

To compile the C program with GCC, use make (in order to avoid compiling each
file manually). The command to run is `make`, simply `make`

This will create a binary named `simdbmk` using GCC as a C compiler. All the
generated object files and the `simdbmk` binary will end up in a `./gcc_build`
folder. The executable produced can be run with a various set of options that
can be listed using the

```shell
./gcc_build/simdbmk --help
```

For instance, to run the program with a `test_array` size of `100 000 000` and
test that introducing a `k`-factor (see below) works as expected, one could
type (to test with `k=500 000`) :

```
./simdbmk -n100000000 -k500000
```

## Program description

### Goals

Given a *vector* of integers `U`, the core of the program is a `find` function
that returns the number of occurrences of a given integer `val` between the
indexes `a_start` and `a_end` in the array. `find` will also allocate an array
containing the indexes where `val` was found in the array.

### Steps performed

* Generate a random array of integers containing values between `a` and `b`
* Run the naive `find` on it
* Run it's vectorial counterpart and measure the performance gain.
* Run the non vectorial parallel version of `find` and compare it against the
  naive one
* Run the vectorized, parallel version of `find` and compare it against the
  naive `find` as well as the non-vectorial, parallel version of `find`
* If `k` has been set on the command-line, test the two versions of
  `thread_find()` using that `k`-factor.

## Lab environment for the tests

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

By default, the benchmarking program is run for steps of 0.05 between 5 and 9
so be aware **that you'll need 4 Gigs of RAM available to run the program**.

#### Dependencies

You'll need to have the python packages `numpy` and `matplotlib` installed on
your machine. You can use `pip` (`pip install numpy matplotlib`) to install
them but it's also very likely that your Linux distributions has packages of
its own for those two python libraries. For instance, on Ubuntu, it's slightly
wiser to use `apt-get install python-matplotlib python-numpy`.

### Mind the RAM

Before running the program for extremely large values of `n`, please consider
the available amount of RAM that you have. Basically, the element that will
determine the amount of memory needed by our program is `U`. If you don't want
to kill yourself when your computer starts swapping, you therefore have to make
sure that `#(U)*sizeof(int)` stays inferior to the available amount of RAM.

Let's assume we don't want to go over 4GB of RAM occupation (our testing
computer has 8 gigs of RAM, this value seems fair) , since an `int` weighs 4
bytes, we can put up to **1 billion** integers in `U`.

## Results

We ran a benchmark with different array sizes. Here's a graph showing how the
performance response:

![alt text](./images/performance_graphs.png "Evolution of performance with the
size of the input array.")

As can be expected, the running time is proportional to the size of the input
array. The performance ratios seem to converge around 2 for the vectorial
implementation, 4 for the multithreaded version (which was to be expected since
we have 4 cores. We can note that hyperthreading doesn't have a huge influence
here). Finally with the multithreaded vectorial approach, we get a `x8.5`
performance gain, which is quite nice.

When the sought element is not in the array though, the performance factor for
the multithreaded vectorial approach reaches 12. This can be explained by the
fact that the gain from going vectorial is lost when an element is found. In
addition to that the `realloc()` sometimes involves a copy of the whole result
array in memory. Using linked lists instead of standard C arrays would probably
have been a better choice, but the memory occupied by the result would have
then been multiplied by 3.

## Questions about different aspects of the program

Here are some questions that we had to reply to regarding that project:

### Generic questions about the naive implementations

#### Define the `realloc()` function

`realloc()` is a function from the standard library whose aim is to expand the
memory allocated for an array. It's much smarter than the naïve implementation
we could think of: if there's space available after the array, the expansion
won't require a copy of the whole array. In addition to that, when a copy is
required, it relies a lot on the Linux kernel's memory paging system as well as
on SIMD extensions when the array to copy is aligned so that the copy takes a
minimal amount of time.

It takes as parameters:
  * a pointer on the array to expand (from there, it will have a look at the
    memory allocation how much size was allocated for that array)
  * the desired allocated space after the expansion

It's also possible to use `realloc()` to shrink the space allocated for the
array.

#### Why do we need a pointer of pointer for `ind_val` ?

What we need to do is populate an array, which is represented by a pointer
(*int_val) and a size (called c and returned by the function find). At first
you'd think that just passing a pointer and make it change would be fine,
but the point is on a function call, the arguments are copied in memory.
Therefore, it's that copy of the pointer that will be modified by `find`, not
the pointer itself. That's why we need to get a pointer pointing on that
pointer: therefore the pointer gives us the address of the pointer pointing
at the beginning of the array and we we can use this "address" to actually
modify the array every time we call `realloc()`.

In short the answer is: "**because arguments get copied when a function is
called so we need to use pointers, and since the argument itself is a
pointer, we need a pointer on a pointer**".

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
that's one of them. This being said, we won't use that in our implementation of
`thread_find()` because it's likely that every chunk will have to be fetched
from memory into the L3 cache `n` times (where `n` is the number of threads)
and this is also likely to trigger cache synchronisation between the L2 caches
on each core, which would slow down the execution time.

### Questions about the multi-threaded approach

#### Describe the data structure of a (POSIX) thread

<TODO>

#### The k-factor, or how do we make our running threads talk to each other ?

In order to limit the search to `k` matches we have to make the threads "talk
to each other". They all have to know how many matches have been found
**globally**. For that, we introduced a global variable `gc` (for **g**lobal
**c**ount) that is incremented every time a thread finds a match.

Concurrent write access to this variable (the `gc++` part) must be prevented,
as well as the `gc > k` part. We're using `mutextes` for that purpose:
everytime a thread finds a match, it exits if `k` has already been reached.
Otherwise, it adds the matches to the list of found occurences. **That's how we
stop threads when the limit is reached.**

Checking if the global counter has exceeded `k` and incrementing the former if
not is a mutex locked operation.

In order to avoid a performance drop when `k` isn't set, we're using two
different pieces of code depending on whether or not `k` is set: if it's not,
we don't perform the global counter check and incrementation part.

## Authors

* Etienne Lafarge (etienne.lafarge**_at_**mines-paristech.fr)
* Vincent Villet (vincent.villet**_at_**mines-paristech.fr)

Special thanks to our teacher, Claude Tadonki.
