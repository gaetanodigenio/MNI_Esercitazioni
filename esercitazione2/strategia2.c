/*
    Nome: Gaetano Di Genio
    Matricola: 0522501339
    Programma: somma n numeri blocchi di colonne (matrice trasposta)
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h> 

/**
* Funzione che esegue il prodotto matrice vettore
*/
void prod_mat_vett(double local_y[], double *local_at, int COLS, int ROWS, double local_x[])
{
    for(int i=0;i<COLS;i++)
    {
        local_y[i]=0;
        for(int j=0;j<ROWS;j++)
        { 
            local_y[i] += local_at[i*ROWS+j]* local_x[i];
        } 
    }    
}

int main(int argc, char* argv[]){
    int me, np;
    int righe, colonne;
    double *A, *At, *X, *Y, *local_X, *local_At, *local_Y;
    int local_col, nlocal_X;
    double T_inizio,T_fine,T_max;
    

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &np);
    MPI_Comm_rank(MPI_COMM_WORLD, &me);

    if(me == 0){
        printf("Inserire numero righe: \n");
        scanf("%d", &righe);

        printf("Inserire numero colonne: \n");
        scanf("%d", &colonne);

        A = (double*)malloc(righe * colonne * sizeof(double));
        At = (double*)malloc(colonne * righe * sizeof(double));
        X = (double*)malloc(colonne * sizeof(double));
        Y = (double*)malloc(colonne * sizeof(double));

        local_col = colonne/np;
        nlocal_X = colonne/np;

        //inizializzo matrice
        for(int i = 0; i<righe; i++){
            for(int j = 0; j<colonne; j++){
                A[i * colonne + j] = rand() % 6;
            }
        }

        //inizializzo X
        for(int i = 0; i<colonne; i++){
            X[i] = rand() % 6;
        }

        //trasposta A
        for(int i = 0; i<righe; i++){
            for(int j = 0; j < colonne; j++){
                At[j * righe + i] = A[i * colonne + j];
            }
        }

        if(righe <= 10 && colonne <= 10){
            printf("\nA= \n");
            for(int i = 0; i<righe; i++){
                for(int j = 0; j<colonne; j++){
                    printf("%1.3f ", A[i*colonne+j]);
                }
                printf("\n");
            }

            printf("\nA trasposta= \n");
            for(int i = 0; i<colonne;i++){
                for(int j = 0; j<righe; j++){
                    printf("%1.3f ", At[i*righe+j]);
                }
                printf("\n");
            }

            printf("\nX = ");
            for(int i = 0; i<colonne; i++){
                printf("%1.3f ", X[i]);
            }
            printf("\n");
        }
    }//fine if me == 0

    //broadcast a tutti dei valori utili
    MPI_Bcast(&local_col, 1,MPI_INT,0,MPI_COMM_WORLD); 
    MPI_Bcast(&nlocal_X,1,MPI_INT,0,MPI_COMM_WORLD); 
    MPI_Bcast(&righe, 1, MPI_INT, 0, MPI_COMM_WORLD);

    //alloco pezzetto di vettore locale da moltiplicare
    local_X = (double*)malloc(nlocal_X * sizeof(double));

    //alloco pezzetto vettore risultante
    local_Y = (double*)malloc(nlocal_X * sizeof(double));

    //alloco sottomatrice locale
    local_At = (double*)malloc(local_col * righe * sizeof(double));

    //scatter del vettore
    MPI_Scatter(X, nlocal_X, MPI_DOUBLE, local_X, nlocal_X, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    //scatter della matrice trasposta
    int num = local_col * righe;
    MPI_Scatter(At, num, MPI_DOUBLE, local_At, num, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    //STAMPE VARIE
    if(local_col <= 10 && righe <=10){
        //stampo matrice locale
        printf("\nProcesso %d, matrice At locale: \n", me);
        for(int i = 0; i<local_col; i++){
            for(int j = 0; j<righe; j++){
                printf("%1.3f ", local_At[i*righe + j]);
            }
            printf("\n");
        }

        //stampo vettore X locale
        printf("\nProcesso %d X locale: \n", me);
        for(int i = 0; i<nlocal_X; i++){
            printf("%1.3f ", local_X[i]);
        }
        printf("\n");
    }

     //inizio calcolo tempo
    MPI_Barrier(MPI_COMM_WORLD);
    T_inizio=MPI_Wtime();

    // Effettuiamo i calcoli
    prod_mat_vett(local_Y, local_At, local_col, righe, local_X);
    
    // 0 raccoglie i risultati parziali
    MPI_Gather(&local_Y[0], local_col,MPI_DOUBLE,&Y[0],local_col,MPI_DOUBLE,0,MPI_COMM_WORLD);

    //fine calcolo tempo
    MPI_Barrier(MPI_COMM_WORLD);
    T_fine=MPI_Wtime()-T_inizio;
    
    //Calcolo del tempo totale
    MPI_Reduce(&T_fine,&T_max,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);

    if(me == 0){
        if(righe <= 10 && colonne <= 10){
                printf("w = \n"); 
                for(int i = 0; i < colonne; i++)
                    printf("%1.3f ", Y[i]);
                printf("\n");
            }
            printf("\nProcessori: %d\n", np);
            printf("Dimensioni matrice: %dx%d\n", colonne, righe);
            printf("Lunghezza vettore: %d\n", colonne);
            printf("Tempo esecuzione: %f ms\n", T_max * 1000);
            free(Y);
            free(X);
            free(A);
            free(At);
    }

    free(local_At);
    free(local_X);
    free(local_Y);

    MPI_Finalize();
    return 0;
}