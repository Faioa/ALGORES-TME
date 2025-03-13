# TP 1 - MPI

Matthieu DARTOIS
___

## Exercice 1 - Configuration

### Question 1
```
$ which mpicc
/usr/local/bin/mpicc
```

### Question 2

Ajout des lignes suivantes au .bashrc pour l'édition dynamique de liens de openmpi :
```
LD_LIBRARY_PATH=/usr/local/lib
export LD_LIBRARY_PATH
```

### Question 3

Création de clés ssh et ajout de ces clés à la liste de clés authorisées.
```
$ ssh-keygen -q -N ’’ -f ${HOME}/.ssh/id_rsa
$ cat ${HOME}/.ssh/id_rsa.pub >> ${HOME}/.ssh/authorized_keys
```

### Question 4

Vérification avec :
```
$ ssh localhost
```

### Question 5

Utiliser le schéma de la commande suivante pour lancer les prorgammes MPI :
```
$ mpicc -o <path_to_executable> <path_to_program.c>
$ mpirun -np <nombre_of_precesses> --hostfile <path_to_hostfile> <path_to_executable>
```

## Exercice 2

### Question 1

```
$ mpicc -o hello_mpi hello_mpi.c
$ mpirun --oversubscribe -np 5 hello_mpi
Processus 2 sur 5 : Hello MPI
Processus 3 sur 5 : Hello MPI
Processus 4 sur 5 : Hello MPI
Processus 0 sur 5 : Hello MPI
Processus 1 sur 5 : Hello MPI
```

## Exercice 3

### Question 1

Ce programme désigne le processus de rang 0 comme processus maître. Tous les autres lui envoie un message "Hello Master from <rang_proc>". Le maître lit dans l'ordre croissant des rangs les messages des processus esclaves et affiche leur message.

### Question 2

```
$ mpicc -o hello_master hello_master.c
$ mpirun --oversubscribe -np 5 hello_master
Hello Master from 1
Hello Master from 2
Hello Master from 3
Hello Master from 4

$ mpirun --oversubscribe -np 10 hello_master
Hello Master from 1
Hello Master from 2
Hello Master from 3
Hello Master from 4
Hello Master from 5
Hello Master from 6
Hello Master from 7
Hello Master from 8
Hello Master from 9
```

L'ordre des messages sera toujours le même, car c'est ainsi qu'est définie la lecture des messages par le processus maître.

### Question 3
```
$ mpicc -o hello_master hello_master.c
$ mpirun --oversubscribe -np 5 hello_master
Hello Master from 3
Hello Master from 4
Hello Master from 1
Hello Master from 2

$ mpirun --oversubscribe -np 10 hello_master
Hello Master from 7
Hello Master from 1
Hello Master from 2
Hello Master from 3
Hello Master from 4
Hello Master from 5
Hello Master from 6
Hello Master from 8
Hello Master from 9
```

On observe que l'ordre n'est plus conservé. Le processus maître se contente de lire autant de messages qu'il y a de processus esclaves, mais la lecture du tampon est aléatoire de part l'utilisation du filtre **MPI_ANY_SOURCE** qui lit simplement le premier message arrivé. Comme l'exécution des processus est parallèle, l'ordre des messages n'est pas prévisible.

## Exercice 4

### Question 1

```
$ mpicc -o hello_neighbor hello_neighbor.c
$ mpirun --oversubscribe -np 5 hello_neighbor
Hello neighbor from 4
Hello neighbor from 0
Hello neighbor from 3
Hello neighbor from 1
Hello neighbor from 2
```

### Question 2

```
$ mpicc -o hello_neighbor hello_neighbor.c
$ mpirun --oversubscribe -np 5 hello_neighbor
...
```

On observe que le programme se bloque sans aucun affichage. La primitive **MPI_Ssend** effectue un envoie synchrone. Ainsi, tous les processus font un envoie et attendent que leur message soit lu, mais cela n'arrive pas, car aucun processus ne commence par une lecture.

### Question 3

En choisissant un rang de processus particulier qui commencera par une lecture, on pourra débloquer le programme et on observera même un ordre de lecture décroissant des rangs modulo le nombre de processus (à partir du rang qui initie les lectures). Ici, je choisi le processus de rang 0. Néanmoins, l'affichage se fera un peu aléatoirement puisque l' affichage est toujours asynchrone.

```
$ mpicc -o hello_neighbor hello_neighbor.c
$ mpirun --oversubscribe -np 5 hello_neighbor
Hello neighbor from 4
Hello neighbor from 3
Hello neighbor from 0
Hello neighbor from 1
Hello neighbor from 2
```

## Exercice 5

### Question 1

Le processus qui gagne l'élection est le processus de rang maximal parmis tous les candidats. Le processus élu est au courant de son élection en recevant un jeton avec sa propre identité.

### Question 2

```
$ mpicc -o chang_roberts chang_roberts.c
$ mpirun --oversubscribe -np 5 chang_roberts
```
