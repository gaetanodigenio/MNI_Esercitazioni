#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cuda_runtime.h>
#include <cublas_v2.h>

void scalareCPU(float *a, float *b, float *oracolo, int x, int y);


int main(void){
    cudaError_t cudaStat;
    cublasStatus_t stat;
    cublasHandle_t handle;
    float *a, *b, *c, *oracolo = 0;
    float *a_d, *b_d, *c_d;
    cudaEvent_t start, stop;
    int x, y;

    printf("Inserisci dimensione x matrice: ");
    scanf("%d", &x);

    printf("Inserisci dimensione y matrice: ");
    scanf("%d", &y);

    //allocazione variabili host
    a = (float*)malloc(x * y * sizeof(*a));
    if(!a){
        printf("Errore allocazione a\n");
        return EXIT_FAILURE;
    }

    b = (float*)malloc(y * sizeof(*b));
    if(!b){
        printf("Errore allocazione b\n");
        return EXIT_FAILURE;
    }

    c = (float*)malloc(x * sizeof(*c));
    if(!c){
        printf("Errore allocazione c\n");
        return EXIT_FAILURE;
    }

    oracolo = (float*)malloc(x * sizeof(*oracolo));
    if(!oracolo){
        printf("Errore allocazione oracolo\n");
        return EXIT_FAILURE;
    }

    //inizializzazione matrice e vettore
    srand(time(NULL));
    for(int i = 0; i<x; i++){
        for(int j = 0; j<y; j++){
            a[i * y + j] = rand() % 5 - 2;
        }
    }

    for(int i = 0; i<x; i++){
        b[i] = rand() % 5 - 2;
    }

    //allocazioni variabili device
    cudaStat = cudaMalloc ((void**)&a_d, x * y * sizeof(*a));   
    if (cudaStat != cudaSuccess) {
        printf ("device memory allocation failed");
        return EXIT_FAILURE;
    }
    
    cudaStat = cudaMalloc ((void**)&b_d, y * sizeof(*b));  
    if (cudaStat != cudaSuccess) {
        printf ("device memory allocation failed");
        return EXIT_FAILURE;
    }

    cudaStat = cudaMalloc ((void**)&c_d, x * sizeof(*c));  
    if (cudaStat != cudaSuccess) {
        printf ("device memory allocation failed");
        return EXIT_FAILURE;
    }

    //creazione handle per cublas
    stat = cublasCreate(&handle);
    if (stat != CUBLAS_STATUS_SUCCESS) {
        printf ("CUBLAS initialization failed\n");
        return EXIT_FAILURE;
    }

    stat = cublasSetMatrix(x, y, sizeof(float), a, x, a_d, x); // Setto h_a su d_a
    if (stat != CUBLAS_STATUS_SUCCESS) {
        printf ("data download failed matrix");
        cudaFree (a_d);
        cublasDestroy(handle);
        return EXIT_FAILURE;
    }
    
    stat = cublasSetVector(y, sizeof(float), b, 1, b_d, 1);    // Setto h_b su d_b
    if (stat != CUBLAS_STATUS_SUCCESS) {
        printf ("data download failed vector");
        cudaFree (b_d);
        cublasDestroy(handle);
        return EXIT_FAILURE;
    }

    //prodotto matrice vettore cublas
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventRecord(start);
    float alpha = 1.0;
    float beta = 1.0;
    stat = cublasSgemv(handle, CUBLAS_OP_T, x, y, &alpha, a_d, x, b_d, 1, &beta, c_d, 1); //trasposta op_t scalare
    if (stat != CUBLAS_STATUS_SUCCESS) {
        printf(" ");
        cudaFree (a_d);
        cudaFree (b_d);
        cudaFree (c_d);
        cublasDestroy(handle);
        return EXIT_FAILURE;
    }
    cudaEventRecord(stop);
    cudaEventSynchronize(stop);
    float elapsed_time;
    cudaEventElapsedTime(&elapsed_time, start, stop);
    printf("\nTempo cublas: %1.3f", elapsed_time);
    cudaEventDestroy(start);
    cudaEventDestroy(stop);

    //copia risultato device host
    stat = cublasGetVector(x, sizeof(float), c_d, 1, c, 1); 

    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventRecord(start);
    scalareCPU(a, b, oracolo, x, y);
    cudaEventRecord(stop);
    cudaEventSynchronize(stop);
    float elapsed_time1;
    cudaEventElapsedTime(&elapsed_time1, start, stop);
    printf("\nTempo CPU: %1.3f", elapsed_time1);
    cudaEventDestroy(start);
    cudaEventDestroy(stop);


    //stampo risultati
    if (x <= 10 && y <= 10) {
        printf("\nMatrice A:\n");
        for (int i = 0; i < x; i++) {
            for (int j = 0; j < y; j++) {
                printf("%f\t", a[i * y + j]);
            }
            printf("\n");
        }

        printf("\nVettore B:\n");
        for (int i = 0; i < y; i++) {
            printf("%f\n", b[i]);
        }
            
        printf("\nVettore Prodotto AxB:\n");
        for (int i = 0; i < x; i++) {
            printf("%f\n", c[i]);
        }

        printf("\nOracolo:\n");
        for (int i = 0; i < x; i++) {
            printf("%f\n", oracolo[i]);
        }
    }


    cudaFree (a_d);         
    cudaFree (b_d);         
    cudaFree (c_d);    
    
    cublasDestroy(handle);  
    
    free(a);     
    free(b);      
    free(c); 
    free(oracolo); 
    return EXIT_SUCCESS;
}

void scalareCPU(float *a, float *b, float *oracolo, int x, int y) {
    for (int i = 0; i < x; i++) {
        oracolo[i] = 0;
        for (int j = 0; j < y; j++) {
            oracolo[i] += a[i * y + j] * b[j];
        }
    }
}