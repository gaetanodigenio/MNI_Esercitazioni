/*
Cognome e nome: Di Genio Gaetano
Matricola: 0522501339
*/

#include <assert.h>
#include <stdio.h>
#include <cuda.h>
#include <time.h>

void prodottoCPU(double *a, double *b, double *c, int n);
__global__ void prodottoGPU(double* a, double* b, double* c, int n);

int main(void)
{
 double *a_h, *b_h, *c_h, c_h2 = 0.0; //host data
 double *a_d, *b_d, *c_d; // device data
 int N, nBytes, i ;
dim3 gridDim, blockDim;


printf("***\t PRODOTTO DI DUE VETTORI \t***\n");
printf("Inserisci il numero degli elementi dei vettori\n");
scanf("%d",&N); 

blockDim.x = 64;

//determinazione esatta del numero di blocchi
gridDim = N/blockDim.x + ((N%blockDim.x)==0?0:1);

printf("\nBlockDim: %d\n", blockDim.x);
printf("gridDim: %d\n", gridDim.x);

 nBytes = N*sizeof(float);
 a_h = (double *)malloc(nBytes);
 b_h = (double *)malloc(nBytes);
 c_h = (double *)malloc(nBytes);
 cudaMalloc((void **) &a_d, nBytes);
 cudaMalloc((void **) &b_d, nBytes);
 cudaMalloc((void **) &c_d, nBytes);
 // inizializzo i dati
 /*Inizializza la generazione random dei vettori utilizzando l'ora attuale del sistema*/                
srand((unsigned int) time(0)); 
        
 for (i=0; i<N; i++) {
	a_h[i] = rand()%5-2;
	b_h[i] = rand()%5-2;;
 }
  		
 cudaMemcpy(a_d, a_h, nBytes, cudaMemcpyHostToDevice);
 cudaMemcpy(b_d, b_h, nBytes, cudaMemcpyHostToDevice);
 

 //azzeriamo il contenuto del vettore c
memset(c_h, 0, nBytes);
cudaMemset(c_d, 0, nBytes);

//tempo esecuzione parallelo
cudaEvent_t start, stop; 
cudaEventCreate(&start);
cudaEventCreate(&stop);
cudaEventRecord(start); 

prodottoGPU<<<gridDim, blockDim>>>(a_d, b_d, c_d, N);

cudaEventRecord(stop); 
cudaEventSynchronize(stop);
float elapsed;  
cudaEventElapsedTime(&elapsed, start, stop);
cudaEventDestroy(start);
cudaEventDestroy(stop);

printf("Tempo totale GPU: %1.3f ", elapsed);

cudaMemcpy(c_h, c_d, nBytes, cudaMemcpyDeviceToHost);


 
 // calcolo prodotto seriale su CPU e tempi
cudaEventCreate(&start);
cudaEventCreate(&stop);
cudaEventRecord(start);

prodottoCPU(a_h, b_h, &c_h2, N);

cudaEventRecord(stop);
cudaEventSynchronize(stop); 
float elapsed1;  
cudaEventElapsedTime(&elapsed1, start, stop);
cudaEventDestroy(start);
cudaEventDestroy(stop);
printf("Tempo totale CPU: %1.3f ", elapsed1);

 
 
 if (N<20){
 	for(i=0;i<N;i++)
		printf("\na_h[%d]=%6.2f ",i, a_h[i]);
	printf("\n");

	for(i=0;i<N;i++)
		printf("\nb_h[%d]=%6.2f ",i, b_h[i]);
	printf("\n");

	for(i=0;i<N;i++)
		printf("\nc_h[%d]=%6.2f ",i, c_h[i]);
	printf("\n");
}


 free(a_h); 
 free(b_h); 
 free(c_h); 
 cudaFree(a_d); 
 cudaFree(b_d); 
 cudaFree(c_d);

 return 0;
}

//Seriale
void prodottoCPU(double *a, double *b, double *c, int n)
{
    for(int i=0;i<n;i++)
        *c +=a[i] * b[i];
} 

//Parallelo
__global__ void prodottoGPU(double* a, double * b, double* c, int n){
    int index=threadIdx.x + blockIdx.x*blockDim.x;
    if(index < n)
        c[index] = a[index] * b[index];
}
