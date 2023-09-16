# for gnu compiler
CC=g++
MPICC=mpic++

# for intel compiler
#CC=icl
#MPICC=mpic++

# for MSVC compiler
#CC=cl
#MPICC=mpic++

# for clang compiler
#CC=clang++
#MPICC=mpic++

CFLAGS = -O3

all : mm mm_omp mm_mpi

mm : clean
	$(CC) $(CFLAGS) -o mm mm.cpp

mm_omp : clean
	$(CC) $(CFLAGS) -fopenmp -o mm_omp mm_omp.cpp

mm_mpi : clean
	$(MPICC) $(CFLAGS) -o mm_mpi mm_mpi.cpp

check : clean
	$(CC) $(CFLAGS) -D CHECK_ALWAYS -o mm mm.c

nobar : clean
	$(CC) $(CFLAGS) -D NO_BARRIER -o mm mm.c

debug : clean
	$(CC) $(CFLAGS) -g -O0 -o mm mm.c

clean :
	rm -f mm mm_omp mm_mpi *.o *.so
