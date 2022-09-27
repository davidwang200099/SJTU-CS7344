mpicc -o matrix_vector_mult matrix_vector_mult.c MyMPI.c
mpicc -o BinSearch BinSearch.c MyMPI.c
mpicc -o manager_worker_mult manager_worker_mult.c MyMPI.c
mpicc -o PerfectNumber PerfectNumber.c MyMPI.c -lm
