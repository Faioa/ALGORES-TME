#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define TAGINIT 0
#define NB_SITE 6

void simulateur(void) {
   int i;

   /* nb_voisins[i] est le nombre de voisins du site i */
   int nb_voisins[NB_SITE+1] = {-1, 2, 3, 2, 1, 1, 1};
   int min_local[NB_SITE+1] = {-1, 3, 11, 8, 14, 5, 17};

   /* liste des voisins */
   int voisins[NB_SITE+1][3] = {{-1, -1, -1},
         {2, 3, -1}, {1, 4, 5}, 
         {1, 6, -1}, {2, -1, -1},
         {2, -1, -1}, {3, -1, -1}};
                               
   for(i=1; i<=NB_SITE; i++){
      MPI_Send(&nb_voisins[i], 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD);
      MPI_Send(voisins[i],nb_voisins[i], MPI_INT, i, TAGINIT, MPI_COMM_WORLD);
      MPI_Send(&min_local[i], 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD); 
   }
}

void calcul_min(int rang) {
      // Initialization
      int my_rank;
      MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);

      MPI_Status status;
      int flag;

      int nb_neighbors;
      MPI_Recv(&nb_neighbors, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);

      // Array containing all of this node's neighbors
      int neighbors[nb_neighbors];
      MPI_Recv(&neighbors, nb_neighbors, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
      // Array to keep in check which neighbor this node received from
      int received[nb_neighbors];
      for (int i = 0; i < nb_neighbors; i++)
            received[i] = 0;

      // Initializing the minimum from this node's perspective (+ the node that has it)
      int min;
      int min_rank = my_rank;
      MPI_Recv(&min, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);

      // Number of messages received (to save complexity on iteration over the array received
      int nb_recv = 0;
      // Sending condition
      int sent = 0;
      // Stopping condition
      int decide = 0;
      while (!decide) {
            // Check if we have an incoming message
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
            if (flag != 0) {
                  // Receive the incoming message
                  nb_recv++;
                  int min_remote;
                  MPI_Recv(&min_remote, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                  // Updating the minimum value
                  if (min_remote < min) {
                        min = min_remote;
                        min_rank = status.MPI_TAG;
                  }
                  // Saving which neighbor sent the message
                  for (int i = 0; i < nb_neighbors; i++) {
                        if (neighbors[i] == status.MPI_SOURCE) {
                              received[i] = 1;
                              break;
                        }
                  }
            }


            if (!sent && nb_recv == nb_neighbors - 1) {
                  // If there is only 1 neighbor this node didn't receive from, it sends its minimum to it
                  for (int i = 0; i < nb_neighbors; i++) {
                        if (!received[i]) {
                              MPI_Send(&min, 1, MPI_INT, neighbors[i], min_rank, MPI_COMM_WORLD);
                              break;
                        }
                  }
                  sent = 1;
            } else if (nb_recv == nb_neighbors)
              // If this node received a messages from all of its neighbors, it decides and propagates its decision to all others
              decide = 1;
      }

      // Send decision to children (last message received should be from this node's parent so we omit it)
      fprintf(stdout, "ID %d : min = %d on peer of ID %d\n", my_rank, min, min_rank);
      for (int i = 0; i < nb_neighbors; i++) {
            if (neighbors[i] != status.MPI_SOURCE) {
                  MPI_Send(&min, 1, MPI_INT, neighbors[i], min_rank, MPI_COMM_WORLD);
            }
      }
}

/******************************************************************************/

int main (int argc, char* argv[]) {
   int nb_proc,rang;
   MPI_Init(&argc, &argv);
   MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);

   if (nb_proc != NB_SITE+1) {
      printf("Nombre de processus incorrect !\n");
      MPI_Finalize();
      exit(2);
   }
  
   MPI_Comm_rank(MPI_COMM_WORLD, &rang);
  
   if (rang == 0) {
      simulateur();
   } else {
      calcul_min(rang);
   }
  
   MPI_Finalize();
   return 0;
}
