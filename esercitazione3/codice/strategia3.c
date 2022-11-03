/*
    Nome: Gaetano Di Genio
    Matricola: 0522501339
    Programma: prodotto matrice vettore strategia 3 (blocchi righe colonne)
*/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

void prod_mat_vett(double local_y[], double *local_at, int COLS, int ROWS, double local_x[])
{
    for(int i=0;i<COLS;i++)
    {
        local_y[i]=0;
        for(int j=0;j<ROWS;j++)
        { 
            local_y[i] += local_at[i*ROWS+j] * local_x[i];
        } 
    }    
}

int main(int argc, char **argv){
    int me, nproc;
    int righe, colonne;
    double *A, *X, *Y, *oracolo, *At;
    double T_inizio, T_fine, T_max;

    //per la griglia 2D, sottogriglie
    MPI_Comm com2D, com_righe, com_colonne; 
    int dims[2], period[2], reorder = 1;
    int me2D, coord2D[2];
    int subCut[2];
    int meRighe, meColonne, coordSubCol[1], coordSubRighe[1];

    //valori locali
    int local_righe, local_colonne;
    double *local_A, *local_X, *local_Y, *local_At, *local_At_temp, *local_Y_temp;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &me);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    if(me == 0){
        printf("Inserisci numero righe: \n");
        scanf("%d", &righe);

        printf("Inserisci numero colonne: \n");
        scanf("%d", &colonne);

        //creo A
        A = (double*)malloc(righe * colonne * sizeof(double));

        //inizializzo A
        for(int i = 0; i<righe; i++){
            for(int j = 0; j<colonne; j++){
                A[i * colonne + j] = rand() % 10;
            }
        }

        //creo X
        X = (double*)malloc(colonne * sizeof(double));

        //inizializzo X
        for(int i = 0; i<colonne; i++){
            X[i] = rand() % 10;
        }

        //creo Y
        Y = (double*)malloc(colonne * sizeof(double));

        //creo At
        At = (double*)malloc(colonne * righe * sizeof(double));

        //inizializzo At
        for(int i = 0; i<righe; i++){
            for(int j = 0; j < colonne; j++){
                At[j * righe + i] = A[i * colonne + j];
            }
        }

        //creo oracolo
        oracolo = (double*)malloc(righe * sizeof(double));

        //inizializzo oracolo */
            for (int i = 0; i < colonne; i++) {
                oracolo[i] = 0;
                for (int j = 0; j < righe; j++) {
                    oracolo[i] += At[i*righe+j] * X[i];
                }
            }

        //if colonne/righe <=10 stampo tutto
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


    }//fine me == 0

    //broadcast righe colonne
    MPI_Bcast(&righe, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&colonne, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    //creo griglia cartesiana 2D, assegno rank e coordinate
    dims[0] = nproc/2;
    dims[1] = nproc/dims[0];
    period[0] = period[1] = 0;
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, period, reorder, &com2D);
    MPI_Comm_rank(com2D, &me2D);
    MPI_Cart_coords(com2D, me2D, 2, coord2D);

    //creo sottogriglia righe
    subCut[0] = 0;
    subCut[1] = 1;
    MPI_Cart_sub(com2D, subCut, &com_righe);
    MPI_Comm_rank(com_righe, &meRighe);
    MPI_Cart_coords(com_righe, meRighe, 1, coordSubRighe);

    //creo sottogriglia colonne
    subCut[0] = 1;
    subCut[1] = 0;
    MPI_Cart_sub(com2D, subCut, &com_colonne);
    MPI_Comm_rank(com_colonne, &meColonne);
    MPI_Cart_coords(com_colonne, meColonne, 1, coordSubCol);

    //barrier
    MPI_Barrier(MPI_COMM_WORLD);

    //calcolo e allocazione A, X, Y locali
    local_righe = righe / dims[0];
    local_colonne = colonne / dims[1];
    //matrix_temp
    local_A = (double*)malloc(local_righe * colonne * sizeof(double));
    local_At_temp = (double*)malloc(colonne * local_righe * sizeof(double));
    //local_matrix
    local_At = (double*)malloc(local_colonne * local_righe * sizeof(double));
    local_X = (double*)malloc(local_colonne * sizeof(double));
    local_Y = (double*)malloc(local_colonne * sizeof(double));
    local_Y_temp = (double*)malloc(local_colonne * sizeof(double));


    //scatter X lungo prima riga (sottogriglia)
    if(coord2D[0] == 0){
        MPI_Scatter(X, local_colonne, MPI_DOUBLE, local_X, local_colonne, MPI_DOUBLE, 0, com_righe);
    }


    //broadcast X a tutta la colonna da parte dei P della prima riga
    MPI_Bcast(local_X, local_colonne, MPI_DOUBLE, 0, com_colonne);


    //scatter da P0 della matrice alla prima colonna (per righe) (righe/np righe ad ognuno)
    if(coord2D[1] == 0){
        MPI_Scatter(A, local_righe * colonne, MPI_DOUBLE, local_A, local_righe * colonne, MPI_DOUBLE, 0, com_colonne);
    }

    //calcolo At per ogni processo della prima colonna
    if(coord2D[1] == 0){
        for(int i = 0; i<local_righe; i++){
                for(int j = 0; j < colonne; j++){
                    local_At_temp[j * local_righe + i] = local_A[i * colonne + j];
                }
            }
    }
 
    //scatter matrice per colonne ad ognuno della propria riga (colonne/np colonne ad ognuno)
    MPI_Scatter(local_At_temp, local_colonne * local_righe, MPI_DOUBLE, local_At, local_colonne * local_righe , MPI_DOUBLE, 0, com_righe);

    //ricalcolo local_A dalla trasposta locale


    //tempo inizio
    MPI_Barrier(MPI_COMM_WORLD);
	T_inizio = MPI_Wtime();

    //algoritmo mat vet locale
    prod_mat_vett(local_Y_temp, local_At, local_colonne, local_righe, local_X);

    //reduce lungo le righe prodotti parziali
    MPI_Reduce(local_Y_temp, local_Y, local_colonne, MPI_DOUBLE, MPI_SUM, 0, com_colonne);

    //gather lungo prima colonna
    if (coord2D[0] == 0) {
        MPI_Gather(local_Y, local_colonne, MPI_DOUBLE, Y, local_colonne, MPI_DOUBLE, 0, com_righe);
    }

    //barrier + tempo fine
    MPI_Barrier(MPI_COMM_WORLD);
	T_fine = MPI_Wtime() - T_inizio;

    //calcolo tempo totale
    MPI_Reduce(&T_fine,&T_max,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);


    //stampa risultati
    if(me == 0){
        
        if(righe <= 10 && colonne <= 10){
            printf("Y = \n");
            for(int i = 0; i<righe; i++){
                printf("%1.2f ", Y[i]);
            }

            printf("\nOracolo: ");
            for (int i = 0; i < righe; i++) {
                printf("%1.2f\n", oracolo[i]);
            }
        }
        printf("\nProcessori: %d\n", nproc);
        printf("Dimensioni A: %dx%d\n", righe, colonne);
        printf("Tempo totale: %1.2f ms\n", T_max * 1000);

        free(A);
        free(X);
        free(Y);

    }
    free(local_A);
    free(local_At);
    free(local_At_temp);
    free(local_X);
    free(local_Y);
    free(local_Y_temp);

    MPI_Finalize();
    return 0;
}



//pxq -> 2x2, 3x3, 4x4