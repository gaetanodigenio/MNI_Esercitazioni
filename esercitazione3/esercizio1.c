/*
    Nome: Gaetano Di Genio
    Matricola: 0522501339
    Programma: esercizio slide
*/

#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>

int main(int argc, char* argv[]){
    int me, nproc, reorder, me_grid, coordinate[2], a;
    int row = 3;
    int column;
    int *ndim, *period;
    int dim = 2;

    MPI_Comm comm_grid;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&me);  
    MPI_Comm_size(MPI_COMM_WORLD,&nproc);

    //numero dimensioni (2 in questo caso)
    ndim = (int*)calloc(dim, sizeof(int));
    ndim[0] = ndim[1] = row;
    
    //periodicità (nessuna in questo caso -> 0)
    period = (int*)calloc(dim, sizeof(int));
    period[0] = period[1] = 0;

    //identificativi processi non riordinabili in questo caso (0)
    reorder = 0;
    
    MPI_Cart_create(MPI_COMM_WORLD, dim, ndim, period, reorder, &comm_grid);
    MPI_Comm_rank(comm_grid, &me_grid);
    MPI_Cart_coords(comm_grid, me_grid, dim, coordinate);

    if(coordinate[0] == coordinate[1]){
        a = coordinate[0] * coordinate[0];
    }else{
        a = coordinate[0] + 2 * coordinate[1];
    }

    printf("P%d, (%d, %d), a: %d\n", me_grid, coordinate[0], coordinate[1], a);
    //fflush(stdout);

    MPI_Finalize();
    return 0;
}