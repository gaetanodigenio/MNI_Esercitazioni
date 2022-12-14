/*
    Nome: Gaetano Di Genio
    Matricola: 0522501339
    Programma: prodotto matrice vettore strategia 1
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h> 

/**
* Funzione che esegue il prodotto matrice vettore
*/
void prod_mat_vett(double w[], double *a, int ROWS, int COLS, double v[])
{
    int i, j;
    
    for(i=0;i<ROWS;i++)
    {
        w[i]=0;
        for(j=0;j<COLS;j++)
        { 
            w[i] += a[i*COLS+j]* v[j];
        } 
    }    
}


int main(int argc, char **argv) {

int nproc;              // Numero di processi totale
int me;                 // Il mio id
int m,n;                  // Dimensione della matrice
int local_m;            // Dimensione dei dati locali
int i,j;                    // Iteratori vari 

// Variabili di lavoro
double *A, *v, *localA,*local_w, *w;

double T_inizio,T_fine,T_max;


/*Attiva MPI*/
MPI_Init(&argc, &argv);
/*Trova il numero totale dei processi*/
MPI_Comm_size (MPI_COMM_WORLD, &nproc);
/*Da ad ogni processo il proprio numero identificativo*/
MPI_Comm_rank (MPI_COMM_WORLD, &me);

// Se sono a radice
if(me == 0)
{
    printf("inserire numero di righe m = \n"); 
    scanf("%d",&m); 
    
    printf("inserire numero di colonne n = \n"); 
    scanf("%d",&n);
    // Numero di righe da processare
    local_m = m/nproc;  
    
    // Alloco spazio di memoria
    A = malloc(m * n * sizeof(double));
    v = malloc(sizeof(double)*n);
    w =  malloc(sizeof(double)*m); 
    
    
    for (i=0;i<m;i++)
    {
        for(j=0;j<n;j++)
        {
            if (j==0)
            A[i*n+j]= 1.0/(i+1)-1;
            else
                A[i*n+j]= 1.0/(i+1)-pow(1.0/2.0,j); 
        }
    }

    if(m <= 10 && n <= 10){
        printf("v = \n");     
        for (j=0;j<n;j++)
        {
            v[j]=j; 
            printf("%1.3f ", v[j]);
        }
        printf("\n");


        printf("A = \n"); 
        for(int i = 0; i<m; i++){
            for(int j = 0; j<n; j++){
                printf("%1.3f ", A[i*n+j] );
            }
            printf("\n");
        }

    }
     
} // fine me==0

// Spedisco m, n, local_m e v
MPI_Bcast(&m,1,MPI_INT,0,MPI_COMM_WORLD);  
MPI_Bcast(&n,1,MPI_INT,0,MPI_COMM_WORLD);            
MPI_Bcast(&local_m,1,MPI_INT,0,MPI_COMM_WORLD);            

// Se sono un figlio alloco v 
if(me != 0)
    v = malloc(sizeof(double)*n);
    
MPI_Bcast(&v[0],n,MPI_DOUBLE,0,MPI_COMM_WORLD);            

// tutti allocano A locale e il vettore dei risultati
localA = malloc(local_m * n * sizeof(double));
local_w = malloc(local_m * sizeof(double));

// Adesso 0 invia a tutti un pezzo della matrice
int num = local_m*n;
MPI_Scatter(
    &A[0], num, MPI_DOUBLE,
    &localA[0], num, MPI_DOUBLE,
    0, MPI_COMM_WORLD);

// Scriviamo la matrice locale ricevuta
if(m <= 10 && n <= 10){
    printf("localA %d = \n", me); 
    for(i = 0; i < local_m; i++)
    {
        for(j = 0; j < n; j++)
            printf("%1.3f\t", localA[i*n+j]);
        printf("\n");
    }
}

//inizio calcolo tempo
MPI_Barrier(MPI_COMM_WORLD);
T_inizio=MPI_Wtime();

// Effettuiamo i calcoli
prod_mat_vett(local_w,localA,local_m,n,v);
    
// 0 raccoglie i risultati parziali
MPI_Gather(&local_w[0],local_m,MPI_DOUBLE,&w[0],local_m,MPI_DOUBLE,0,MPI_COMM_WORLD);

//fine calcolo tempo
MPI_Barrier(MPI_COMM_WORLD);
T_fine=MPI_Wtime()-T_inizio;


//Calcolo del tempo totale
MPI_Reduce(&T_fine,&T_max,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);

// 0 stampa la soluzione
if(me==0)
{ 
    if(m <= 10 && n <= 10){
        printf("w = \n"); 
        for(i = 0; i < m; i++)
            printf("%1.3f ", w[i]);
        printf("\n");
    }
    printf("\nProcessori: %d\n", nproc);
    printf("Dimensioni matrice: %dx%d\n", m, n);
    printf("Lunghezza vettore: %d\n", n);
    printf("Tempo esecuzione: %f ms\n", T_max * 1000);
}

MPI_Finalize (); /* Disattiva MPI */
return 0;  
}