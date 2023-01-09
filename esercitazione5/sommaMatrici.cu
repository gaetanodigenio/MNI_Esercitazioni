/*
Cognome e nome: Di Genio Gaetano
Matricola: 0522501339
*/

#include <assert.h>
#include <stdio.h>
#include <cuda.h>
#include <time.h>

__global__ void sommaMatriciGPU(double *a, double *b, double *s, int n);
void sommaMatriciCPU(double *a, double *b, double *s, int n);

int main(void){
    //host
    double *a_h, *b_h, *s_h, *s_h2;
    //device
    double *a_d, *b_d, *s_d;
    int n;

    printf("Inserisci dimensione matrice: \n");
    scanf("%d", &n);

    //configurazione kernel
    dim3 blockDim(16, 16);
    dim3 gridDim(n/blockDim.x + ((n%blockDim.x)==0?0:1), n/blockDim.y + ((n%blockDim.y)==0?0:1));

    //allocazione variabili host
    a_h = (double*) malloc(n * n * sizeof(double));
    b_h = (double*) malloc(n * n * sizeof(double));
    s_h = (double*) malloc(n * n * sizeof(double));
    s_h2 = (double*) malloc(n * n * sizeof(double));

    //inizializzazione matrici host
    srand(time(NULL));
    for(int i = 0; i<n; i++){
        for(int j = 0; j<n; j++){
            a_h[i * n + j] = rand() % 5 - 2;
            b_h[i * n + j] = rand() % 5 - 2;
        }
    }

    //allocazione variabili device
    cudaMalloc((void**) &a_d, n * n * sizeof(double));
    cudaMalloc((void**) &b_d, n * n * sizeof(double));
    cudaMalloc((void**) &s_d, n * n * sizeof(double));

    //copia da host a device dei vettori
    cudaMemcpy(a_d, a_h, n * n * sizeof(double), cudaMemcpyHostToDevice);
    cudaMemcpy(b_d, b_h, n * n * sizeof(double), cudaMemcpyHostToDevice);

    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventRecord(start);
    //invoco kernel
    sommaMatriciGPU<<<blockDim, gridDim>>>(a_d, b_d, s_d, n);
    //copio risultato da device a host
    cudaMemcpy(s_h, s_d, n * n * sizeof(double), cudaMemcpyDeviceToHost);
    cudaEventRecord(stop); 
    cudaEventSynchronize(stop);
    float elapsed_time;
    cudaEventElapsedTime(&elapsed_time, start, stop);
    if (n <= 10) {
        printf("\nMatrice A:\n");
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                printf("%f\t", a_h[i * n + j]);
            }
            printf("\n");
        }
        printf("\nMatrice B:\n");
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                printf("%f\t", b_h[i * n + j]);
            }
            printf("\n");
        }
        printf("\nMatrice Somma:\n");
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                printf("%f\t", s_h[i * n + j]);
            }
            printf("\n");
        }
    }
    printf("\n\nTempo di esecuzione GPU: %f\n", elapsed_time);

    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventRecord(start);
    //invoco funzione CPU
    sommaMatriciCPU(a_h, b_h, s_h2, n);
    
    //copio risultato da device a host
    cudaEventRecord(stop); 
    cudaEventSynchronize(stop);
    float elapsed_time1;
    cudaEventElapsedTime(&elapsed_time1, start, stop);
    printf("\n\nTempo di esecuzione CPU: %f\n", elapsed_time1);

    free(a_h);
    free(b_h);
    free(s_h);
    cudaFree(a_d);
    cudaFree(b_d);
    cudaFree(s_d);
    return 0;
}

__global__ void sommaMatriciGPU(double *a, double *b, double *s, int n){
    int i = threadIdx.x + (blockDim.x * blockIdx.x);
    int j = threadIdx.y + (blockDim.y * blockIdx.y);
    s[i * n + j] = a[i * n + j] + b[i * n + j];
}

void sommaMatriciCPU(double *a, double *b, double *s, int n){
    for(int i = 0; i<n; i++){
        for(int j= 0; j<n; j++){
            s[i * n + j] = a[i * n + j] + b[i * n +j];
        }
    }

}