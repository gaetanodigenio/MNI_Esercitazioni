#include <assert.h>
#include <stdio.h>
#include <cuda.h>
#include <time.h>

__global__ void prodottoGPU(double* a, double* b, double* c, int n);
void prodottoCPU(double* a, double *b, double *oracolo, int n);

int main(void){
    //host
    double *a, *b, *c, oracolo = 0.0, somma_totale = 0.0;
    //device
    double *a_d, *b_d, *c_d;
    int n;
    cudaEvent_t start, stop; 

    printf("Inserisci numero elementi vettori: \n");
    scanf("%d", &n);

    dim3 blockDim(64);
    dim3 gridDim = n/blockDim.x + ((n % blockDim.x) == 0 ? 0 : 1);
    printf("gridDim: %d\n", gridDim.x);
    printf("BlockDim: %d\n", blockDim.x);

    //allocazione variabili host
    a = (double*)malloc(n * sizeof(double));
    b = (double*)malloc(n * sizeof(double));
    c = (double*)malloc(n * sizeof(double));

    //allocazione variabili device
    cudaMalloc((void**) &a_d, n * sizeof(double));
    cudaMalloc((void**) &b_d, n * sizeof(double));
    cudaMalloc((void**) &c_d, n * sizeof(double));

    //inizializzazione variabili host
    srand((unsigned int) time(0)); 
    for(int i = 0; i<n; i++){
        a[i] = rand() % 5 - 2;
        b[i] = rand() % 5 - 2;
    }

   
    //copia variabili host -> device
    cudaMemcpy(a_d, a, n*sizeof(double), cudaMemcpyHostToDevice);
    cudaMemcpy(b_d, b, n*sizeof(double), cudaMemcpyHostToDevice);
    memset(c, 0, n * sizeof(double));
	cudaMemset(c_d, 0, n * sizeof(double));

    //funzione su GPU
    cudaEventCreate(&start);
	cudaEventCreate(&stop);
	cudaEventRecord(start); 

    prodottoGPU<<<gridDim, blockDim>>>(a_d, b_d, c_d, n);

    //copia risultato device -> host
    cudaMemcpy(c, c_d, n * sizeof(double), cudaMemcpyDeviceToHost);

    //somma sequenziale su host
    for(int i = 0; i<n; i++){
        somma_totale += c[i];
    }

    cudaEventRecord(stop); 
	cudaEventSynchronize(stop);
	float elapsed;  
	cudaEventElapsedTime(&elapsed, start, stop);
	cudaEventDestroy(start);
	cudaEventDestroy(stop);
	printf("Tempo totale GPU: %1.3f ", elapsed);

    

    //funzione CPU per calcolo speedup e oracolo
    cudaEventCreate(&start);
	cudaEventCreate(&stop);
	cudaEventRecord(start);

    prodottoCPU(a, b, &oracolo, n);

    cudaEventRecord(stop);
	cudaEventSynchronize(stop); 
	float elapsed1;  
	cudaEventElapsedTime(&elapsed1, start, stop);
	cudaEventDestroy(start);
	cudaEventDestroy(stop);
	printf("Tempo totale CPU: %1.3f ", elapsed1);

    if(n < 20){
        for(int i = 0; i<n; i++){
            printf("\na[%d] = %1.3f ", i, a[i]);
        }
        printf("\n");

        for(int i = 0; i<n; i++){
            printf("\nb[%d] = %1.3f ", i, b[i]);
        }
        printf("\n");

        for(int i = 0; i<n; i++){
            printf("\nc[%d] = %1.3f ", i, c[i]);
        }
        printf("\n");
    }

    printf("Somma totale: %1.3f\n", somma_totale);
    printf("Oracolo: %1.3f\n", oracolo);

    free(a);
    free(b);
    free(c);
    cudaFree(a_d);
    cudaFree(b_d);
    cudaFree(c_d);

    return 0;
}

__global__ void prodottoGPU(double* a, double* b, double* c, int n){
    int index = threadIdx.x + blockIdx.x * blockDim.x;
    if(index < n){
        c[index] = a[index] * b[index];
    }
}


void prodottoCPU(double* a, double *b, double *oracolo, int n){
    for(int i = 0; i<n; i++){
        *oracolo += a[i] * b[i];
    }
}
