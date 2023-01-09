/*
    Nome: Gaetano Di Genio
    Matricola: 0522501339
    Programma: strategia2 scatterv
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h> 

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
    int me, nproc;
    int righe, colonne;
    double *A, *X, *Y, *local_A, *local_Y, *oracolo;
    int resto, somma = 0;
    int *sendcounts, *offset;
    double T_inizio,T_fine,T_max;


    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);
    MPI_Comm_rank(MPI_COMM_WORLD, &me);

    if (me == 0){
        printf("Inserire numero righe: \n");
        scanf("%d", &righe);

        printf("Inserire numero colonne: \n");
        scanf("%d", &colonne);

        //alloco A
        A = (double*)malloc(righe * colonne * sizeof(double));

        //inizializzo A
        for(int i = 0; i<righe; i++){
            for(int j = 0; j<colonne; j++){
                A[i * colonne + j] = rand() % 10;
            }
        }

        //alloco X
        X = (double*)malloc(colonne * sizeof(double));

        //inizializzo X
        for(int i = 0; i<colonne; i++){
            X[i] = rand() % 10;
        }

        //alloco Y
        Y = (double*)malloc(righe * sizeof(double));
        
    
        //stampo tutto se <= 10
        if(righe <= 10 && colonne <= 10){
            printf("\nA= \n");
            for(int i = 0; i<righe; i++){
                for(int j = 0; j<colonne; j++){
                    printf("%1.2f ", A[i*colonne+j]);
                }
                printf("\n");
            }

            printf("\nX = ");
            for(int i = 0; i<colonne; i++){
                printf("%1.2f ", X[i]);
            }
            printf("\n");
        }

        //oracolo
        oracolo = (double*)malloc(righe * sizeof(double));
        for (int i = 0; i < righe; i++) {
                oracolo[i] = 0;
                for (int j = 0; j < colonne; j++) {
                    oracolo[i] += A[i*colonne+j] * X[j];
                }
        }


    }//fine me == 0

    //bcast righe colonne
    MPI_Bcast(&righe, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&colonne, 1, MPI_INT, 0, MPI_COMM_WORLD);

    //calcolo sendcount e offset
    resto = (righe)%nproc;
    
    sendcounts = (int*)malloc(nproc * sizeof(int)); 
    offset = (int*)malloc(nproc * sizeof(int));  

    for (int i = 0; i < nproc; i++) {
        sendcounts[i] = (righe/nproc);
        if (resto > 0) {  
            sendcounts[i]++;
            resto--;
        }
        sendcounts[i] *= colonne;
        

        offset[i] = somma; 
        somma += sendcounts[i]; 
    }

    //li stampa il processo 0
    if(me == 0){
        for (int i = 0; i < nproc; i++) {
            printf("sendcounts[%d] = %d\toffset[%d] = %d\n", i, sendcounts[i], i, offset[i]);
        }
    }


    //alloco X locale
    if(me != 0){
        X = (double*)malloc(colonne * sizeof(double));
    }

    //alloco local Y
    local_Y = (double*)malloc((sendcounts[me]/colonne) * sizeof(double));


    //bcast X
    MPI_Bcast(X, colonne, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    //alloco local_A
    local_A = (double*)malloc(sendcounts[me] * sizeof(double));

    //scatterv A
    MPI_Scatterv(A, sendcounts, offset, MPI_DOUBLE, local_A, sendcounts[me], MPI_DOUBLE, 0, MPI_COMM_WORLD);

    //stampo matrice locale processi
    if(righe <=10){
        printf("PROCESSO %d: ", me);
        for (int i = 0; i < sendcounts[me]; i++) {
            printf("%1.2f\t", local_A[i]);
        }
        printf("\n");
    }

    //tempo inizio
    MPI_Barrier(MPI_COMM_WORLD);
    T_inizio=MPI_Wtime();

    //prodotto
    prod_mat_vett(local_Y, local_A, sendcounts[me]/colonne, colonne, X);

    

    MPI_Gather(local_Y, sendcounts[me]/colonne, MPI_DOUBLE, &Y[0], sendcounts[me]/colonne, MPI_DOUBLE, 0, MPI_COMM_WORLD);



    //tempo fine
    MPI_Barrier(MPI_COMM_WORLD);
    T_fine=MPI_Wtime()-T_inizio;

    MPI_Reduce(&T_fine,&T_max,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);

    if(me == 0){
        if(righe <=10 && colonne <= 10){
            printf("\nY: ");
            for(int i = 0; i<righe; i++){
                printf("%1.2f ", Y[i]);
            }
            printf("\n");

            printf("\nOracolo: ");
                for (int i = 0; i < righe; i++) {
                    printf("%1.2f ", oracolo[i]);
            }
            printf("\n");
        }
        printf("Tempo totale: %1.2fms \n", T_max*1000);
    }

    free(local_A);
    free(local_Y);
    free(X);


    MPI_Finalize();
    return 0;
}