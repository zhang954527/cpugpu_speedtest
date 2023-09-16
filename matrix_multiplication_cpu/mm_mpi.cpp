// Matrix decomposition with MPI
// A                   B                   C
// x x x x x x x x     x x|x x|x x|x x     c c x x x x x x
// x x x x x x x x     x x|x x|x x|x x     c c x x x x x x
// ---------------        |   |   |        ---------------
// x x x x x x x x     x x|x x|x x|x x     x x c c x x x x
// x x x x x x x x     x x|x x|x x|x x     x x c c x x x x
// ---------------        |   |   |        ---------------
// x x x x x x x x     x x|x x|x x|x x     x x x x c c x x
// x x x x x x x x     x x|x x|x x|x x     x x x x c c x x
// ---------------        |   |   |        ---------------
// x x x x x x x x     x x|x x|x x|x x     x x x x x x c c
// x x x x x x x x     x x|x x|x x|x x     x x x x x x c c

// 法1：矩阵 A、B、C 全部分布式存储，分割方法如上示意图。各进程按节拍对 B 各
//      大列进行全收集，向 my_id + k 发送，向 my_id - k 接受。
// 法2：矩阵 A 分布式存储，矩阵 B 全局存储全量分发到所有进程上，C分布式存储. 
//      这样只对A切分，这样每个进程能完整算一部分C，不进行通信。节约通信时间

#include <iostream>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <chrono>
#include <mpi.h>

template<typename T>
MPI_Datatype getMPIDataType();

template<>
MPI_Datatype getMPIDataType<float>() {
    return MPI_FLOAT;
}

template<>
MPI_Datatype getMPIDataType<double>() {
    return MPI_DOUBLE;
}

// Matrix multiplication function
template<typename T>
void matrixMultiply1(T* matA, T* matB, T* result, int n, int rank, int size, int rowsPerProcess) {
    for (int step = 0; step < size; step++) {  // total compute 4 times
        int startCol = (rank - step) % size * rowsPerProcess;
        int endCol = startCol + rowsPerProcess;
        for (int i = 0; i < rowsPerProcess; i++) {  // i is local col, and A is local
            for (int j = startCol; j < endCol; j++) {
                T sum = 0;
                int j_k = j - startCol;
                for (int k = 0; k < n; k++) {
                    sum += matA[i * n + k] * matB[k * n + j_k]  // j is global col, but B is local [k*n+j]?
                }
                result[i * n + j] = sum
            }
        }  // matrix C view
        int rank_send = (rank + 1) % size; // dynamic, thus only for + 1 process
        int rank_recv = (rank - 1) % size;
        if (rank_send != rank)
            MPI_Sendrecv_replace(matB, n * rowsPerProcess, getMPIDataType<T>(), rank_send, 0, rank_recv, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
}

// Function to print matrix
template<typename T>
void printMatrix(T* matrix, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            std::cout << matrix[i * n + j] << " ";
        }
        std::cout << std::endl;
    }
}

// Function to print local sub-matrix
template<typename T>
void printSubMatrix(T* matrix, int rowsPerProcess, int n) {
    for (int i = 0; i < rowsPerProcess; ++i) {
        for (int j = 0; j < n; ++j) {
            std::cout << matrix[i * n + j] << " ";
        }
        std::cout << std::endl;
    }
}

int main(int argc, char* argv[]) {
    int n = 1000; // Default matrix size
    bool useDouble = false; // Use single-precision float by default

    // Initialize MPI
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        // Parse command-line arguments
        for (int i = 1; i < argc; i++) {
            if (std::strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
                // Check if the next argument is a number
                if (std::isdigit(*argv[i + 1])) {
                    n = std::atoi(argv[i + 1]);
                } else {
                    throw std::runtime_error("Matrix size not specified. Please provide a valid matrix size after -n.");
                }
            } else if (std::strcmp(argv[i], "-float") == 0) {
                useDouble = false;
            } else if (std::strcmp(argv[i], "-double") == 0) {
                useDouble = true;
            }
        }

        std::cout << "Matrix size: " << n << " Precision: " << (useDouble ? "double" : "float") << std::endl;
    }

    // Broadcast matrix size and precision from rank 0 to all other processes
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&useDouble, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);

    // Calculate the number of rows each process will handle
    int rowsPerProcess = n / size;  // matrix A view
    int startRow = rowsPerProcess * rank;
    int endRow = startRow + rowsPerProcess;

    // Allocate memory for local matrices
    size_t localSize = static_cast<size_t>(rowsPerProcess) * static_cast<size_t>(n);
    void* localMatA = nullptr;
    void* localMatB = nullptr;
    void* localResult = nullptr;

    if (useDouble) {
        localMatA = new double[localSize];
        localMatB = new double[localSize];
        localResult = new double[localSize];
    } else {
        localMatA = new float[localSize];
        localMatB = new float[localSize];
        localResult = new float[localSize];
    }

    // Initialize local matrices with random values
    std::srand(std::time(nullptr) + rank); // Add rank to seed for different random values on each process
    for (int i = 0; i < rowsPerProcess; ++i) {
        for (int j = 0; j < n; ++j) {
            if (useDouble) {
                static_cast<double*>(localMatA)[i * n + j] = static_cast<double>(std::rand()) / RAND_MAX;
                static_cast<double*>(localMatB)[i * n + j] = static_cast<double>(std::rand()) / RAND_MAX;
            } else {
                static_cast<float*>(localMatA)[i * n + j] = static_cast<float>(std::rand()) / RAND_MAX;
                static_cast<float*>(localMatB)[i * n + j] = static_cast<float>(std::rand()) / RAND_MAX;
            }
        }
    }

    // Perform local matrix multiplication and measure time
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    if (useDouble) {
        matrixMultiply1(static_cast<double*>(localMatA), static_cast<double*>(localMatB),
                       static_cast<double*>(localResult), n, rank, size, rowsPerProcess);
    } else {
        matrixMultiply1(static_cast<float*>(localMatA), static_cast<float*>(localMatB),
                       static_cast<float*>(localResult), n, rank, size, rowsPerProcess);
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    // Gather the results from all processes
    if (rank == 0) {
        if (useDouble) {
            MPI_Gather(MPI_IN_PLACE, localSize, MPI_DOUBLE, static_cast<double*>(localResult), localSize, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        } else {
            MPI_Gather(MPI_IN_PLACE, localSize, MPI_FLOAT, static_cast<float*>(localResult), localSize, MPI_FLOAT, 0, MPI_COMM_WORLD);
        }
    } else {
        if (useDouble) {
            MPI_Gather(static_cast<double*>(localResult), localSize, MPI_DOUBLE, nullptr, localSize, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        } else {
            MPI_Gather(static_cast<float*>(localResult), localSize, MPI_FLOAT, nullptr, localSize, MPI_FLOAT, 0, MPI_COMM_WORLD);
        }
    }

    // if (n < 20) {
    //     std::cout << "Rank " << rank << std::endl;
    //     std::cout << "Matrix A" << std::endl;
    //     printSubMatrix(static_cast<float*>(localMatA), rowsPerProcess, n);
    //     std::cout << "Matrix B" << std::endl;
    //     printSubMatrix(static_cast<float*>(localMatB), rowsPerProcess, n);
    // }

    // Calculate and print elapsed time on rank 0
    if (rank == 0) {
        double duration = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
        std::cout << "Matrix multiplication time: " << duration << " seconds" << std::endl;

        // Example: Print the result matrix
        if (n < 20) {
            std::cout << "Matrix final result" << std::endl;
            printMatrix(static_cast<float*>(localResult), n);
        }
    }

    // Deallocate memory
    if (useDouble) {
        delete[] static_cast<double*>(localMatA);
        delete[] static_cast<double*>(localMatB);
        delete[] static_cast<double*>(localResult);
    } else {
        delete[] static_cast<float*>(localMatA);
        delete[] static_cast<float*>(localMatB);
        delete[] static_cast<float*>(localResult);
    }

    // Finalize MPI
    MPI_Finalize();

    return 0;
}