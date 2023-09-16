# CPU/GPU Perforance Benchmarks

Times CPU/GPU performance according some computation benchmarks.

# Basic Usage

Build:

    make

Run:

    ./mm
    ./mm -n 200
    ./mm -n 2000 -double
    ./mm_omp -n 2000 -ntomp 4
    mpirun -np <procs> ./mm

# Usage Syntax

    Usage:  mm / mm_omp [options] [operations]
  
    Options:
      -n <size>      Square matrix size n*n (default 1000)
      -ntomp <nomp>  OpenMP threads number  (default 1)
      -float         Use single precision (default)
      -double        Use double precision

# Bench Record

## Test1 - 1 nodes, 1 procs, 1 threads (Intel(R) Xeon(R) Platinum 8336C CPU @ 2.30GHz)

    ./mm
    Matrix size: 1000 Precision: float
    Matrix multiplication time: 1.33137 second

    ./mm -double
    Matrix size: 1000 Precision: double
    Matrix multiplication time: 1.44273 seconds

    ./mm -n 500
    Matrix size: 500 Precision: float
    Matrix multiplication time: 0.157968 seconds

    ./mm -n 2000
    Matrix size: 2000 Precision: float
    Number of threads: 1
    Matrix multiplication time: 48.4952 seconds

## Test2 - 1 nodes, 1 procs, n threads (Intel(R) Xeon(R) Platinum 8336C CPU @ 2.30GHz)

The speed basically increases linearly with the number of threads.

    ./mm_omp -n 2000 -ntomp 2
    Matrix size: 2000 Precision: float
    Number of threads: 2
    Matrix multiplication time: 22.976 seconds

    ./mm_omp -n 2000 -ntomp 2 -double
    Matrix size: 2000 Precision: double
    Number of threads: 2
    Matrix multiplication time: 26.6082 seconds

    ./mm_omp -n 2000 -ntomp 4
    Matrix size: 2000 Precision: float
    Number of threads: 4
    Matrix multiplication time: 11.5409 seconds

    ./mm_omp -n 2000 -ntomp 8
    Matrix size: 2000 Precision: float
    Number of threads: 8
    Matrix multiplication time: 5.99805 seconds

