# simd-benchmarking
A simple C program along with a Python script aiming at benchmarking SIMD and multi-threaded computing.

## Compiling instructions

Just simply run

```shell
gcc projet.c -msse -lpthread -o simdbmk
```

## Running the benchmark and seeing cool graphs

Here as well,it's just the matter of a command line to run :

```shell
python ./benchmark.py
```

It can take various amounts of time depending on your machine. On a 2.7GHz
Intel i7 with 4 cores and hyperthreading enabled, it should take around an
hour. You can edit the file to avoid the last steps of the computation. Oh and
beware, **running that same last step also requires a bit more than 4 free Gigs
of RAM** !
