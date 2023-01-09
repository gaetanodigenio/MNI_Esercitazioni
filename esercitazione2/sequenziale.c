/*
    Nome: Gaetano Di Genio
    Matricola: 0522501339
    Programma: prodotto matrice vettore sequenziale
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

int main(int argc, char* argv[]){
    int righe, colonne;
    double *A, *X, *Y;
    printf("Inserisci numero righe matrice:\n");
    scanf("%d", &righe);

    printf("Inserisci numero colonne matrice:\n");
    scanf("%d", &colonne);

    A = (double*)malloc(righe * colonne * sizeof(double));
    X = (double*)malloc(colonne * sizeof(double));
    Y = (double*)malloc(righe * sizeof(double));

    //inizializzo matrice
    for(int i = 0; i<righe; i++){
        for(int j = 0; j<colonne; j++){
            A[i * colonne + j] = rand() % 10;
        }
    }

    //inizializzo vettore
    for(int i = 0; i<colonne; i++){
        X[i] = rand() % 10;
    }

    clock_t start = clock();

    //algoritmo prodotto 
    for(int i = 0; i<righe; i++){
        Y[i] = 0;
        for(int j = 0; j<colonne; j++){
            Y[i] += A[i * colonne + j] * X[j];
        }
    }

    clock_t end = clock();

    printf("\nCalcolo completato, risultati...\n");

    if(righe <= 10 && colonne <= 10){
        printf("\nA = \n");
        for(int i = 0; i < righe; i++){
            for(int j = 0; j < colonne; j++){
                printf("%1.3f ", A[i * colonne + j]);
            }
            printf("\n");
        }

        printf("\nX = ");
        for(int i = 0; i<colonne; i++){
            printf("%1.3f ", X[i]);
        }
        printf("\n");

        printf("\nY = ");
        for(int i = 0; i<righe; i++){
            printf("%1.3f ", Y[i]);
        }
        printf("\n");

    }
    
    printf("Tempo di calcolo totale: %1.3f millisecondi\n", (double)(end-start)/CLOCKS_PER_SEC * 1000);

    free(A);
    free(X);
    free(Y);

    return 0;
}