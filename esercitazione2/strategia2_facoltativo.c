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
void prod_mat_vett(double local_y[], double *local_A, int ROWS, int COLS, double local_x[]){
    for(int i=0;i<ROWS;i++)
    {
        local_y[i]=0;
        for(int j=0;j<COLS;j++)
        { 
            local_y[i] += local_A[i*COLS+j]* local_x[j];
        } 
    }    
}

int main(int argc, char* argv[]){
    int me, np;
    int righe, colonne;
    double *A, *X, *Y, *local_A, *local_Y;
    int local_righe;
    double T_inizio,T_fine,T_max;
    int resto;
    int *righe_send, *offset;
    

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &np);
    MPI_Comm_rank(MPI_COMM_WORLD, &me);

    if(me == 0){
        printf("Inserire numero righe: \n");
        scanf("%d", &righe);

        printf("Inserire numero colonne: \n");
        scanf("%d", &colonne);

        //alloco A X e Y
        A = (double*)malloc(righe * colonne * sizeof(double));
        X = (double*)malloc(colonne * sizeof(double));
        Y = (double*)malloc(righe * sizeof(double));


        //inizializzo matrice
        for(int i = 0; i<righe; i++){
            for(int j = 0; j<colonne; j++){
                A[i * colonne + j] = rand() % 10;
            }
        }

        //inizializzo X
        for(int i = 0; i<colonne; i++){
            X[i] = rand() % 10;
        }

        //print varie
        if(righe <= 10 && colonne <= 10){
            printf("\nA= \n");
            for(int i = 0; i<righe; i++){
                for(int j = 0; j<colonne; j++){
                    printf("%1.3f ", A[i*colonne+j]);
                }
                printf("\n");
            }

            printf("\nX = ");
            for(int i = 0; i<colonne; i++){
                printf("%1.3f ", X[i]);
            }
            printf("\n");
        }

        righe_send = (int*)malloc(np * sizeof(int));
        offset = (int*)malloc(np * sizeof(int));
        for(int i = 0; i<np; i++){
            if(i < (righe % np)){
                righe_send[i] = righe/np + 1;
            }else{
                righe_send[i] = righe/np;
            }
            righe_send[i] *= colonne;

            if(i == 0){
                offset[i] = 0;
            }else{
                offset[i] = offset[i-1] + (righe_send[i-1]);
            }
            //invio n righe ad ogni processo
            MPI_Send(&righe_send[i], 1, MPI_INT, i, 0, MPI_COMM_WORLD);  
        }

        //invio matrice ad ogni processo
        for(int i = 0; i<np; i++){
            //printf("&A[%d], righesend[i] * colonne: %d\n", offset[i] * colonne, righe_send[i] * colonne);
            //invio righe matrice ad ogni processo
            MPI_Send(A, righe_send[i], MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
        }

    }//fine if me == 0

    //ricevo numero righe locali 
    MPI_Recv(&local_righe, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, NULL);
   

    //broadcast a tutti delle colonne da processo 0 a tutti
    MPI_Bcast(&colonne, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if(me != 0){
        X = (double*)malloc(colonne * sizeof(double));
    }
    MPI_Bcast(X, colonne, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    
    
    //alloco At, X e Y
    local_A = (double*)malloc(local_righe * sizeof(double));
    local_Y = (double*)malloc((local_righe/colonne) * sizeof(double));

    //ricevo matrice
    int num = local_righe; 
    printf("%d num \n", num);
    MPI_Recv(local_A, num, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, NULL);
    
    //print
    /*
    if(colonne <= 10 && righe <=10){
        //stampo matrice locale
        printf("\nProcesso %d, matrice A locale: \n", me);
        for(int i = 0; i<local_righe/colonne; i++){
            for(int j = 0; j<colonne; j++){
                printf("%1.3f ", local_A[i*colonne + j]);
            }
            printf("\n");
        }

        //print local X 
        printf("\nProcesso %d X locale: \n", me);
        for(int i = 0; i<colonne; i++){
            printf("%1.3f ", X[i]);
        }
        printf("\n");
    }

    //start time
    MPI_Barrier(MPI_COMM_WORLD);
    T_inizio=MPI_Wtime();

    //algo
    prod_mat_vett(local_Y, local_A, local_righe/colonne, colonne, X);
    
    // 0 raccoglie i risultati parziali
    MPI_Gather(&local_Y[0], righe_send[me]/colonne, MPI_DOUBLE, Y, righe_send[me]/colonne, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    //fine calcolo tempo
    MPI_Barrier(MPI_COMM_WORLD);
    T_fine=MPI_Wtime()-T_inizio;
    
    //Calcolo del tempo totale
    MPI_Reduce(&T_fine,&T_max,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);

    if(me == 0){
        if(righe <= 10 && colonne <= 10){
                printf("Y = \n"); 
                for(int i = 0; i < righe; i++)
                    printf("%1.3f ", Y[i]);
                printf("\n");
            }
            printf("\nProcessori: %d\n", np);
            printf("Dimensioni matrice: %dx%d\n", colonne, righe);
            printf("Tempo esecuzione: %f ms\n", T_max * 1000);
            free(Y);
            free(X);
            free(A);
            
    }*/
    free(local_A);
    free(local_Y);

    MPI_Finalize();
    return 0;
}