/*
    Nome: Gaetano Di Genio
    Matricola: 0522501339
    Programma: somma n numeri sequenziale
*/

#include <stdio.h>
#include <mpi.h>

int main(int argc, char** argv){
    int n;
    double somma = 0;
    double T_inizio, T_fine;

    MPI_Init(&argc, &argv);
   
    printf("Inserisci numero elementi da sommare\n");
    scanf("%d", &n);

    T_inizio = MPI_Wtime();

    for(int i = 0; i<n; i++){
        somma += (0.2 + i);
    }

    T_fine = MPI_Wtime()-T_inizio;

    printf("Somma totale: %1.3lf\n", somma);
    printf("Tempo di calcolo totale: %lf millisecondi\n", T_fine * 1000);

    MPI_Finalize();
    return 0;
}