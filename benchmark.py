"""
A very simple Python script running the SIMD benchmarking binary (initially
produced from C code) with different array size and storing all the results
into a CSV. It also plots some interesting graphs.

Oh yeah it's in Python, we were a bit tired of writing C code :)
"""

# stl
import csv
import math
import argparse
import subprocess

# 3p
import numpy
import matplotlib.pyplot as plt

BENCHMARK_RESULTS = []


def run_step(binary_name, n):
    """
    Runs the benchmarking binary once and returns the results found at the last
    line of stdout or throws an exception in case the binary exits with a
    non-zero exit code
    """
    print("Running {0} with n={1}".format(binary_name, n))
    p = subprocess.Popen([binary_name, "--size={0}".format(n)],
            stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            stderr=subprocess.PIPE)

    out, err = p.communicate()
    rc = p.returncode

    if rc != 0:
        print("Ooops, {0} for n={1} exited with the error code {2}".format(
            binary_name, n, rc))
        print("------------- STDOUT ------------")
        print(out)
        print("------------- STDERR ------------")
        print(err)
        raise "Benchmark failed"
    else:
        print("{0} ran successfully with n={1}".format(binary_name, n))
        res = out.splitlines()[-1]
        bms = res.split()
        benchmark = [int(bms[0]), int(bms[1]), int(bms[2]), int(bms[3]),
                float(bms[4]), float(bms[5]), float(bms[6]), float(bms[7])]
        return benchmark


def run_benchmark(binary_name, smpls):
    """
    Runs the benchmark for n varying between 10^a to 10^b by powers of ten
    """
    benchmark = []
    for n in smpls:
        benchmark.append(run_step(binary_name, n))

    # Let's dump the results into a good old CSV file
    with open("./results/benchmark.csv", 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerows(benchmark)

    # And let's display some fancy graphs
    fig = plt.figure()
    timeplt = fig.add_subplot(121)

    n = smpls

    bcm_np = numpy.array(benchmark)
    timeplt.loglog(n, bcm_np[:, 0])
    timeplt.loglog(n, bcm_np[:, 1])
    timeplt.loglog(n, bcm_np[:, 2])
    timeplt.loglog(n, bcm_np[:, 3])

    timeplt.set_xlabel("n")
    timeplt.set_ylabel("t (µs)")
    timeplt.set_title("Computation time")

    timeplt.legend(["Naive", "Vectorized", "Multi-threaded",
        "Multi-threaded +Vectorized"], loc="upper left")

    indexplt = fig.add_subplot(122)
    indexplt.loglog(n, bcm_np[:, 4])
    indexplt.loglog(n, bcm_np[:, 5])
    indexplt.loglog(n, bcm_np[:, 6])
    indexplt.loglog(n, bcm_np[:, 7])

    indexplt.set_xlabel("n")
    indexplt.set_ylabel("Performance Gain")
    indexplt.set_title("Performance gain obtained with SIMD and "
                       "multi-threading")

    indexplt.legend(["Perf. vect.", "Perf. vect. bis", "Perf. MT",
        "Perf. Vect.+ MT"], loc="lower right")

    plt.show()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Takes a list of floats '
             'and runs our SIMD benchmarking binary ("./simdbmk") for every '
             '10^k, where k is an element of the list. It puts the results of '
             'the benchmark in "./benchmark.csv" and displays a graph using '
             'matplotlib.')
    parser.add_argument('powers', metavar='Ps', type=float, nargs='+',
            help='The list of powers of ten to run simdbmk with as an '
                'argument for the size of the generated array (in case the '
                'power of ten is not an integer it will be floored).')
    args = parser.parse_args()
    run_benchmark("./simdbmk", [math.floor(10**p) for p in args.powers])

