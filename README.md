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
containing the indexes where `val` was found in the array. By design, we were
asked to use standard C-arrays and only those (in particular, using linked
lists would indeed be much more efficient to avoid memory reallocations but it
would also multiply the size of the result "array" by three).

### Step performed

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
its own for those two python library. For instance, on Ubuntu, it's slightly
wiser to use `apt-get install python-matplotlib python-numpy`.

### Mind the RAM

Before running the program for extremely high values of `n`, please consider
the available amount of RAM that you have. Basically, the element that will
determine the amount of memory needed by our program is `U`. If you don't want
to kill yourself when your computer starts swapping, you therefore have to make
sure that `#(U)*sizeof(int)` stays inferior to the available amount of RAM.

Let's assume we don't want to go over 4GB of RAM occupation (our testing
computer has 8 gigs of RAM, this value seems fair) , since an `int` weighs 4
bytes, we can put up to **1 billion** integers in `U`.

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
