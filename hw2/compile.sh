mpicc -o PrimeSieve PrimeSieve.c -lm
mpicc -o MPI_Bcast MPI_Bcast.c 
mpicc -o My_Bcast MPI_Bcast.c
mpicc -o Life Life.c MyMPI.c 
