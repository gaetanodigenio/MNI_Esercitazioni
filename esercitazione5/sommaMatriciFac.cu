#include <assert.h>
#include <stdio.h>
#include <cuda.h>
#include <time.h>

__global__ void sommaGPU(double *a, double *b, double *c, int y);
void sommaCPU(double *a, double *b, double *o, int x, int y);

int main(void){
    //variabili host
    double *a, *b, *c, *o;
    //variabili device
    double *a_d, *b_d, *c_d;
    int x, y;
    cudaEvent_t start, stop;

    printf("Inserire dimensione x matrice: \n");
    scanf("%d", &x);

    printf("Inserire dimensione y matrice: \n");
    scanf("%d", &y);

    //configurazione kernel
    dim3 blockDim(8, 8); //ottimale
    dim3 gridDim(x/blockDim.x + (x%blockDim.x == 0 ? 0 : 1), y/blockDim.y + (y%blockDim.y == 0 ? 0 : 1));
    printf("blockDim = (%d,%d)\n", blockDim.x, blockDim.y);
    printf("gridDim = (%d,%d)\n", gridDim.x, gridDim.y);

    //allocazione variabili host
    a = (double*) malloc(x * y * sizeof(double));
    b = (double*) malloc(x * y * sizeof(double));
    c = (double*) malloc(x * y * sizeof(double));
    o = (double*) malloc(x * y * sizeof(double));

    //inizializzazione matrici host
    srand(time(NULL));
    for(int i = 0; i<x; i++){
        for(int j = 0; j<y; j++){
            a[i * y + j] = rand() % 5 - 2;
            b[i * y + j] = rand() % 5 - 2;
        }
    }

    //allocazione variabili device
    cudaMalloc((void **) &a_d, (x * y) * sizeof(double));
    cudaMalloc((void **) &b_d, (x * y) * sizeof(double));
    cudaMalloc((void **) &c_d, (x * y) * sizeof(double));

    //copia host device matrici
    cudaMemcpy(a_d, a, x * y * sizeof(double), cudaMemcpyHostToDevice);
    cudaMemcpy(b_d, b, x * y * sizeof(double), cudaMemcpyHostToDevice);

    //invoco funzione CPU       
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventRecord(start);
    sommaCPU(a, b, o, x, y);
    cudaEventRecord(stop); 
    cudaEventSynchronize(stop);
    float elapsed_time1;
    cudaEventElapsedTime(&elapsed_time1, start, stop);
    cudaEventDestroy(start);
    cudaEventDestroy(stop);
    printf("Tempo CPU: %1.3f\n", elapsed_time1);
    

    //chiamata funzione GPU
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventRecord(start);
    sommaGPU<<<gridDim, blockDim>>>(a_d, b_d, c_d, y);
    cudaEventRecord(stop); 
    cudaEventSynchronize(stop);
    float elapsed_time;
    cudaEventElapsedTime(&elapsed_time, start, stop);
    cudaEventDestroy(start);
    cudaEventDestroy(stop);
    printf("Tempo GPU: %1.3f\n", elapsed_time);

    //copio device host risultato vettore
    cudaMemcpy(c, c_d, (x * y) * sizeof(double), cudaMemcpyDeviceToHost);

    //stampo risultati
    if (x <= 5 && y <= 5) {
        printf("\nMatrice A:\n");
        for (int i = 0; i < x; i++) {
            for (int j = 0; j < y; j++) {
                printf("%f\t", a[i * y + j]);
            }
            printf("\n");
        }
        printf("\nMatrice B:\n");
        for (int i = 0; i < x; i++) {
            for (int j = 0; j < y; j++) {
                printf("%f\t", b[i * y + j]);
            }
            printf("\n");
        }
        printf("\nMatrice C:\n");
        for (int i = 0; i < x; i++) {
            for (int j = 0; j < y; j++) {
                printf("%f\t", c[i * y + j]);
            }
            printf("\n");
        }
        printf("\nOracolo:\n");
        for (int i = 0; i < x; i++) {
            for (int j = 0; j < y; j++) {
                printf("%f\t", o[i * y + j]);
            }
            printf("\n");
        }
    }

    free(a);
    free(b);
    free(c);
    free(o);
    cudaFree(a_d);
    cudaFree(b_d);
    cudaFree(c_d);
    return 0;
}


__global__ void sommaGPU(double *a, double *b, double *c, int y){
    int i = threadIdx.x + (blockDim.x * blockIdx.x);
    int j = threadIdx.y + (blockDim.y * blockIdx.y);
    c[i * y + j] = a[i * y + j] + b[i * y + j];
}

void sommaCPU(double *a, double *b, double *o, int x, int y){
    for(int i = 0; i<x; i++){
        for(int j = 0; j<y; j++){
            o[i * y + j] = a[i * y + j] + b[i * y + j];
        }
    }
}