#include <mpi.h>
#include <stdio.h>
#include <string.h>

#define SIZE 128
#define ROOT 0

int main(int argc, char **argv){
	int my_rank;
	int nb_proc;
	int tag =0;
	char message[SIZE];
	
	MPI_Status status;
	
	MPI_Init(&argc, &argv);
	
	MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);
	MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
	
	if (my_rank == ROOT) {
		// Reception du message du predecesseur
		MPI_Recv(message, SIZE, MPI_CHAR, (my_rank - 1) % nb_proc, tag, MPI_COMM_WORLD, &status);
		printf("%s\n", message);
		
		// Envoie du message au successeur
		sprintf(message, "Hello neighbor from %d", my_rank);
		MPI_Ssend(message, strlen(message)+1, MPI_CHAR, (my_rank + 1) % nb_proc, tag, MPI_COMM_WORLD);
	} else {	
		// Envoie du message au successeur
		sprintf(message, "Hello neighbor from %d", my_rank);
		MPI_Ssend(message, strlen(message)+1, MPI_CHAR, (my_rank + 1) % nb_proc, tag, MPI_COMM_WORLD);
		
		// Reception du message du predecesseur
		MPI_Recv(message, SIZE, MPI_CHAR, (my_rank - 1) % nb_proc, tag, MPI_COMM_WORLD, &status);
		printf("%s\n", message);
	}
	
	MPI_Finalize();
	return 0;
}
