#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cuda_runtime.h>
#include <cublas_v2.h>

void scalareCPU(float *a, float *b, float *oracolo, int n);

int main(void){
    //host
    float *a = 0, *b = 0, oracolo = 0.0;
    //device
    float *a_d, *b_d;
    float result = 0;
    cudaEvent_t start, stop;
    cudaError_t cudaStat;
    cublasStatus_t stat;
    cublasHandle_t handle;
    int M;

    printf("Inserisci lunghezza vettori: \n");
    scanf("%d", &M);

    //allocazione variabili host
    a = (float*)malloc(M * sizeof(*a));
    if(!a){
        printf("Allocazione non riuscita\n");
        return EXIT_FAILURE;
    }
    a[0] = 3.0;
    a[1] = 10.0;
    a[2] = 20.0;

    b = (float*)malloc(M * sizeof(*b));
    if(!b){
        printf("Allocazione non riuscita\n");
        return EXIT_FAILURE;
    }
    b[0] = 5.0;
    b[1] = 10.0;
    b[2] = 15.0;

    //allocazione variabili device
    cudaStat = cudaMalloc((void**) &a_d, M * sizeof(*a));
    if(cudaStat != cudaSuccess){
        printf("Allocazione memoria device non riuscita\n");
        return EXIT_FAILURE;
    }
    
    
    cudaStat = cudaMalloc((void**) &b_d, M * sizeof(*b));
    if(cudaStat != cudaSuccess){
        printf("Allocazione memoria device non riuscita\n");
        return EXIT_FAILURE;
    }

    //creazione handle per cublas
    stat = cublasCreate(&handle);
    if(stat != CUBLAS_STATUS_SUCCESS){
        printf("Inizializzazione cublas errore\n");
        return EXIT_FAILURE;
    }

    //copia a da host a device
    stat = cublasSetVector(M, sizeof(float), a, 1, a_d, 1);
    if(stat != CUBLAS_STATUS_SUCCESS){
        printf("Fallimento passaggio dati\n");
        cudaFree(a_d);
        cublasDestroy(handle);
        return EXIT_FAILURE;
    }

    stat = cublasSetVector(M, sizeof(float), b, 1, b_d, 1);
    if(stat != CUBLAS_STATUS_SUCCESS){
        printf("Fallimento passagio dati\n");
        cudaFree(b_d);
        cublasDestroy(handle);
        return EXIT_FAILURE;
    }

    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventRecord(start);
    //calcolo del prodotto
    stat = cublasSdot(handle, M, a_d, 1, b_d, 1, &result);
    if(stat != CUBLAS_STATUS_SUCCESS){
        printf("Fallimento calcolo prodotto");
        cudaFree(a_d);
        cudaFree(b_d);
        cublasDestroy(handle);
        return EXIT_FAILURE;
    }

    cudaEventRecord(stop);
    cudaEventSynchronize(stop);
    float elapsed_time;
    cudaEventElapsedTime(&elapsed_time, start, stop);
    printf("\nTempo cublas: %1.3f", elapsed_time);


    cudaFree(a_d);
    cudaFree(b_d);
    
    cublasDestroy(handle);

    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventRecord(start);
    scalareCPU(a, b, &oracolo, M);
    cudaEventRecord(stop);
    cudaEventSynchronize(stop);
    float elapsed_time1;
    cudaEventElapsedTime(&elapsed_time1, start, stop);
    printf("\nTempo CPU: %1.3f", elapsed_time1);

    free(a);
    free(b);
    return EXIT_SUCCESS;
}

void scalareCPU(float *a, float *b, float *oracolo, int n){
    for(int i = 0; i<n; i++){
        *oracolo += a[i] * b[i];
    }
}