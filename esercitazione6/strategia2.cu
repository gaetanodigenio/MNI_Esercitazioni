#include<stdio.h>
#include<stdlib.h>
#include<cuda.h>

__global__ void prodottoGPU(float* a, float* b, float* c, int n);
void prodottoCPU(float* a, float* b, float* oracolo, int n);

int main(void){
    //host
    float *a, *b, *c, oracolo = 0.0, somma_totale = 0.0;
    //device
    float *a_d, *b_d, *c_d;
    int n;
    cudaEvent_t start, stop;

    printf("Inserisci dimensione dei vettori: \n");
    scanf("%d", &n);

    //configurazione kernel
    dim3 blockDim(64);
    dim3 gridDim = n/blockDim.x + (n%blockDim.x == 0 ? 0 : 1);
    printf("blockDim = %d\n", blockDim.x);
    printf("gridDim = %d\n", gridDim.x);

    //allocazione host
    a = (float*)malloc(n * sizeof(float));
    b = (float*)malloc(n * sizeof(float));
    c = (float*)malloc(gridDim.x * sizeof(float));

    //inizializzazione host
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        a[i] = rand() % 5 - 2; 
        b[i] = rand() % 5 - 2;
    }

    //allocazione device
    cudaMalloc((void**) &a_d, n * sizeof(float));
    cudaMalloc((void**) &b_d, n * sizeof(float));
    cudaMalloc((void**) &c_d, gridDim.x * sizeof(float));
    
    //copia dati host -> device
    cudaMemcpy(a_d, a, n * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(b_d, b, n * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(c_d, c, gridDim.x * sizeof(float), cudaMemcpyHostToDevice);

    //invocazione funzione GPU e invio risultato device -> host
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventRecord(start);

    prodottoGPU<<<gridDim, blockDim, blockDim.x>>>(a_d, b_d, c_d, n);

    cudaMemcpy(c, c_d, gridDim.x * sizeof(float), cudaMemcpyDeviceToHost);

    /* somma sull'host */
    for (int i = 0; i < gridDim.x; i++) {
        somma_totale += c[i];
    }

    cudaEventRecord(stop); 
    cudaEventSynchronize(stop);
    float elapsed_time;
    cudaEventElapsedTime(&elapsed_time, start, stop);
    cudaEventDestroy(start);
    cudaEventDestroy(stop);
    printf("Tempo GPU: %1.3f\n", elapsed_time);

    


    //invocazione funzione CPU per confronto e calcolo tempi
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventRecord(start);

    prodottoCPU(a, b, &oracolo, n);

    cudaEventRecord(stop); 
    cudaEventSynchronize(stop);
    float elapsed_time1;
    cudaEventElapsedTime(&elapsed_time1, start, stop);
    cudaEventDestroy(start);
    cudaEventDestroy(stop);
    printf("Tempo CPU: %1.3f\n", elapsed_time1);

    //stampo risultati
    if(n <= 10){
        printf("\nA: ");
        for (int i = 0; i < n; i++) {
            printf("%1.3f\t", a[i]);
        }
        printf("\nB: ");
        for (int i = 0; i < n; i++) {
            printf("%1.3f\t", b[i]);
        }
        printf("\nC: ");
        for(int i = 0; i<gridDim.x; i++){
            printf("%1.3f\t", c[i]);
        }
    }
    
    printf("Oracolo: %1.3f\n", oracolo);
    printf("Somma su host totale: %1.3f\n", somma_totale);

    free(a);
    free(b);
    free(c);
    cudaFree(a_d);
    cudaFree(b_d);
    cudaFree(c_d);
    return 0;
}

__global__ void prodottoGPU(float* a, float* b, float* c, int n){
    extern __shared__ float v[]; //allocazione dinamica 3 parametro invocazione kernel

    int index = threadIdx.x + blockIdx.x * blockDim.x;
    int id = threadIdx.x;

    if(index < n){
        v[id] = a[index] * b[index];

        __syncthreads();

        //somma parallela
        for(int dist = 1; dist < blockDim.x; dist = dist << 1){
            if((id % (dist * 2)) == 0){
                v[id] = v[id] + v[id + dist];
            }
            __syncthreads();
        }

        if(id == 0){
            c[blockIdx.x] = v[0];
        }

    }

}

void prodottoCPU(float* a, float* b, float* oracolo, int n){
    for(int i = 0; i<n; i++){
        *oracolo += a[i] * b[i];
    }
}
