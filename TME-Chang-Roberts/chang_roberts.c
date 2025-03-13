#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define CANNOT_CANDIDATE 0
#define CAN_CANDIDATE 1
#define CANDIDATE 2
#define ELECTED 3
#define BEATEN 4
#define SHOULD_STOP 5

int main(int argc, char **argv){
	int my_rank;
	int nb_proc;
	int flag;
	int candidate;
	int state = CAN_CANDIDATE;
	
	MPI_Status status;
	
	MPI_Init(&argc, &argv);
	
	MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);
	MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);

	do {
		if (state == CAN_CANDIDATE) {
			// Verification de si notre predecesseur nous envoie deja une candidature ou une election
			MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
			if (flag != 0)
				state = CANNOT_CANDIDATE;
			else {
				// Decide s'il est candidat
				srand(getpid());
				state = rand() % 2 ? CANDIDATE : CAN_CANDIDATE;
				if (state == CANDIDATE) {
					fprintf(stdout, "I, %d, am candidating !\n", my_rank);
					// Envoie au successeur de sa candidature
					MPI_Send(&my_rank, 1, MPI_INT, (my_rank + 1) % nb_proc, CANDIDATE, MPI_COMM_WORLD);
				}
			}
		}
		
		// Reception des messages sur l'anneau
		MPI_Recv(&candidate, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		if (candidate == my_rank) {
			// Le processus reçoit son propre jeton : il est elu
			state = SHOULD_STOP;
			fprintf(stdout, "I, %d, am the chosen one !\n", my_rank);
			MPI_Send(&my_rank, 1, MPI_INT, (my_rank + 1) % nb_proc, ELECTED, MPI_COMM_WORLD);
		} else if (status.MPI_TAG == CANDIDATE) {
			if ((state == CANDIDATE && my_rank < candidate) || state != CANDIDATE) {
				// Le processus a un rang plus petit et est candidat, ou si il n'est pas candidat : il transmet la candidature et ne peut plus candidater
				if (state == CANDIDATE)
					state = BEATEN;
				MPI_Send(&candidate, 1, MPI_INT, (my_rank + 1) % nb_proc, status.MPI_TAG, MPI_COMM_WORLD);
			}
		} else if (status.MPI_TAG == ELECTED) {
			state = SHOULD_STOP;
			fprintf(stdout, "I, %d, acknowledge %d as the chosen one.\n", my_rank, candidate);
			int dest = (my_rank + 1) % nb_proc;
			if (dest != candidate)
				MPI_Send(&candidate, 1, MPI_INT, dest, status.MPI_TAG, MPI_COMM_WORLD);
		}
	} while (state != SHOULD_STOP);
	
	MPI_Finalize();
	
	return 0;
}
