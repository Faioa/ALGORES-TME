#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define TAGINIT 0
#define NB_SITE 6

#define TAG_ACK 1
#define TAG_NACK 2

void simulateur(void) {
   int i;

   /* nb_voisins[i] est le nombre de voisins du site i */
   int nb_voisins[NB_SITE+1] = {-1, 3, 3, 2, 3, 5, 2};
   int min_local[NB_SITE+1] = {-1, 12, 11, 8, 14, 5, 17};

   /* liste des voisins */
   int voisins[NB_SITE+1][5] = {{-1, -1, -1, -1, -1},
            {2, 5, 3, -1, -1}, {4, 1, 5, -1, -1}, 
            {1, 5, -1, -1, -1}, {6, 2, 5, -1, -1},
            {1, 2, 6, 4, 3}, {4, 5, -1, -1, -1}};
                               
   for(i=1; i<=NB_SITE; i++){
      MPI_Send(&nb_voisins[i], 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD);    
      MPI_Send(voisins[i], nb_voisins[i], MPI_INT, i, TAGINIT, MPI_COMM_WORLD);    
      MPI_Send(&min_local[i], 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD); 
   }
}

/******************************************************************************/

void calcul_min(int rank) {
   // Initialization
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
   int min_rank = rank;
   MPI_Recv(&min, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);


   if (rank == 1) {
      // Arbitrary initiator => ID 1

      // Sending the initiator's value to all of its neighbors
      for (int i = 0; i < nb_neighbors; i++)
         MPI_Send(&min, 1, MPI_INT, neighbors[i], rank, MPI_COMM_WORLD);

      // Getting the children's answers
      for (int i = 0; i < nb_neighbors; i++) {
         int min_remote;
         MPI_Recv(&min_remote, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
         if (min_remote < min) {
            min = min_remote;
            min_rank = status.MPI_TAG;
         }
      }

      // Send decision to children
      fprintf(stdout, "ID %d : min = %d on peer of ID %d\n", rank, min, min_rank);
      for (int i = 0; i < nb_neighbors; i++)
         MPI_Send(&min, 1, MPI_INT, neighbors[i], min_rank, MPI_COMM_WORLD);
   } else {
      // Not initiator
      int parent = -1;

      // Getting first wave
      int min_remote;
      MPI_Recv(&min_remote, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      if (min_remote < min) {
         min = min_remote;
         min_rank = status.MPI_TAG;
      }
      parent = status.MPI_SOURCE;

      // Relaying first wave
      for (int i = 0; i < nb_neighbors; i++) {
         if (neighbors[i] != parent)
            MPI_Send(&min, 1, MPI_INT, neighbors[i], min_rank, MPI_COMM_WORLD);
      }

      // Getting the answer of the edges
      for (int i = 0; i < nb_neighbors; i++) {
         if (neighbors[i] == parent)
            continue;
         MPI_Recv(&min_remote, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
         if (min_remote < min) {
            min = min_remote;
            min_rank = status.MPI_TAG;
         }
      }

      // Relaying to parent
      MPI_Send(&min, 1, MPI_INT, parent, min_rank, MPI_COMM_WORLD);

      // Getting final answer and relaying it to neighbors
      MPI_Recv(&min, 1, MPI_INT, parent, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      min_rank = status.MPI_TAG;
      fprintf(stdout, "ID %d : min = %d on peer of ID %d\n", rank, min, min_rank);
      for (int i = 0; i < nb_neighbors; i++) {
         if (neighbors[i] == parent)
            continue;
         MPI_Send(&min, 1, MPI_INT, neighbors[i], min_rank, MPI_COMM_WORLD);
      }
   }
}

void calcul_min_tree(int rank) {
      // Initialization
      MPI_Status status;
      int flag;

      int nb_neighbors;
      MPI_Recv(&nb_neighbors, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);

      // Array containing all of this node's neighbors
      int neighbors[nb_neighbors];
      MPI_Recv(&neighbors, nb_neighbors, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);

      // Array to keep in check which neighbor answered
      int nb_received = 0;

      // Array to save which neighbors are children
      int children[nb_neighbors];

      // Initializing the minimum from this node's perspective
      int local_min;
      MPI_Recv(&local_min, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);

      // Global minimumm + node that has it
      int min[2] = {local_min, rank};

      int parent = 0;
      if (rank == 1) {
            // Arbitrary initiator => ID 1 to avoid leader election
            parent = -1;

            // Sending first message to all neighbors
            for (int i = 0; i < nb_neighbors; i++)
                  MPI_Send(min, 2, MPI_INT, neighbors[i], TAGINIT, MPI_COMM_WORLD);
      }

      int decided = 0;
      while (!decided) {
            // Receiving a message
            int min_remote[2];
            MPI_Recv(min_remote, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            if (status.MPI_TAG == TAGINIT) {
            // Message is not an answer
                  if (!parent) {
                  // If this node doesn't have a parent and isn't the root
                        parent = status.MPI_SOURCE;
                        for (int i = 0; i < nb_neighbors; i++) {
                              if (neighbors[i] != parent)
                                    MPI_Send(min, 2, MPI_INT, neighbors[i], TAGINIT, MPI_COMM_WORLD);
                        }
                  } else {
                        if (status.MPI_SOURCE == parent) {
                        // Received final decision
                              min[0] = min_remote[0];
                              min[1] = min_remote[1];
                              nb_received++;
                        } else {
                        // A neighbor sent a message, should answer with TAG_NACK
                              MPI_Send(min, 2, MPI_INT, status.MPI_SOURCE, TAG_NACK, MPI_COMM_WORLD);
                        }
                  }

            } else {
                  // Evaluating if neighbor is a child + marked as received + condition update of min
                  nb_received++;
                  for (int i = 0; i < nb_neighbors; i++) {
                        if (neighbors[i] == status.MPI_SOURCE) {
                              children[i] = status.MPI_TAG == TAG_ACK ? status.MPI_SOURCE != parent : 0;
                        }
                  }

                  // Updating min only if it is from subtree
                  if (status.MPI_TAG == TAG_ACK && min_remote[0] < min[0]) {
                        min[0] = min_remote[0];
                        min[1] = min_remote[1];
                  }

                  // Transfer the minimum to this node's parent if it is not the root
                  if (parent != -1 && nb_received == nb_neighbors - 1)
                        MPI_Send(min, 2, MPI_INT, parent, TAG_ACK, MPI_COMM_WORLD);
            }

            if (nb_received == nb_neighbors) {
                  decided = 1;
                  for (int i = 0; i < nb_neighbors; i++) {
                        if (children[i])
                              MPI_Send(min, 2, MPI_INT, neighbors[i], TAGINIT, MPI_COMM_WORLD);
                  }
            }
      }

      // Transfering final decision only to children
      fprintf(stdout, "ID %d : min = %d on peer of ID %d\n", rank, min[0], min[1]);
      for (int i = 0; i < nb_neighbors; i++) {
            if (children[i])
                  MPI_Send(min, 2, MPI_INT, neighbors[i], TAGINIT, MPI_COMM_WORLD);
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
      calcul_min_tree(rang);
   }
  
   MPI_Finalize();
   return 0;
}
