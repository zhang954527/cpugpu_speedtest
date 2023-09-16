#include <iostream>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <omp.h>
#include <stdexcept>
#include <chrono>

// Matrix multiplication function
template<typename T>
void matrixMultiply(T* matA, T* matB, T* result, int n) {
    #pragma omp parallel for
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            T sum = 0;
            for (int k = 0; k < n; ++k) {
                sum += matA[i * n + k] * matB[k * n + j];
            }
            result[i * n + j] = sum;
        }
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

int main(int argc, char* argv[]) {
    int n = 1000; // Default matrix size
    int numThreads = 1; // Default number of threads
    bool useDouble = false; // Use single-precision float by default

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
        } else if (std::strcmp(argv[i], "-ntomp") == 0 && i + 1 < argc) {
            // Check if the next argument is a number
            if (std::isdigit(*argv[i + 1])) {
                numThreads = std::atoi(argv[i + 1]);
            } else {
                throw std::runtime_error("Number of threads not specified. Please provide a valid thread number after -ntomp.");
            }
        }
    }

    std::cout << "Matrix size: " << n << " Precision: " << (useDouble ? "double" : "float") << std::endl;
    std::cout << "Number of threads: " << numThreads << std::endl;

    // Set number of threads
    omp_set_num_threads(numThreads);

    // Allocate memory for matrices
    size_t size = static_cast<size_t>(n) * static_cast<size_t>(n);
    void* matA = nullptr;
    void* matB = nullptr;
    void* result = nullptr;

    if (useDouble) {
        matA = new double[size];
        matB = new double[size];
        result = new double[size];
    } else {
        matA = new float[size];
        matB = new float[size];
        result = new float[size];
    }

    // Initialize matrices with random values
    std::srand(std::time(nullptr));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (useDouble) {
                static_cast<double*>(matA)[i * n + j] = static_cast<double>(std::rand()) / RAND_MAX;
                static_cast<double*>(matB)[i * n + j] = static_cast<double>(std::rand()) / RAND_MAX;
            } else {
                static_cast<float*>(matA)[i * n + j] = static_cast<float>(std::rand()) / RAND_MAX;
                static_cast<float*>(matB)[i * n + j] = static_cast<float>(std::rand()) / RAND_MAX;
            }
        }
    }

    // Perform matrix multiplication and measure time
    // std::clock_t start = std::clock(); // is cpu time, not wall time, this is larger than wall time when use multi threads
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    if (useDouble) {
        matrixMultiply(static_cast<double*>(matA), static_cast<double*>(matB),
                       static_cast<double*>(result), n);
    } else {
        matrixMultiply(static_cast<float*>(matA), static_cast<float*>(matB),
                       static_cast<float*>(result), n);
    }
    // std::clock_t end = std::clock();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    // Calculate and print elapsed time
    // double duration = static_cast<double>(end - start) / CLOCKS_PER_SEC;
    double duration = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    std::cout << "Matrix multiplication time: " << duration << " seconds" << std::endl;

    // Example: Print the result matrix
    if (n < 20) {
        printMatrix(static_cast<float*>(result), n);
    }

    // Deallocate memory
    if (useDouble) {
        delete[] static_cast<double*>(matA);
        delete[] static_cast<double*>(matB);
        delete[] static_cast<double*>(result);
    } else {
        delete[] static_cast<float*>(matA);
        delete[] static_cast<float*>(matB);
        delete[] static_cast<float*>(result);
    }

    return 0;
}