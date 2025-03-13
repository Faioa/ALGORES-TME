#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char** argv) {
	int rank;
	int nb_neigh;
	
	MPI_Init(&argc, &argv);
	
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nb_neigh);
	
	fprintf(stdout, "Processus %d sur %d : Hello MPI\n", rank, nb_neigh);
	
	MPI_Finalize();
	return 0;
}
