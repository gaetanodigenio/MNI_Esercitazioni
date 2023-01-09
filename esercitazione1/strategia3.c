/*
    Nome: Gaetano Di Genio
    Matricola: 0522501339
    Programma: somma n numeri 3° strategia
*/

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char** argv){
    int me, nproc, dim, comunicaCon;
    double *vett;
    double test_somma = 0;
    int nlocgen, resto, nloc, displs;
    double *vett_loc;
    double inizio, fine_locale, fine, somma_locale = 0, somma_parziale;
    int p, passi = 0;
    int *potenze;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &me);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    //il processo 0 chiede quanti numeri sommare
    if(me == 0){
        printf("Quanti elementi sommare?\n");
        scanf("%d", &dim);

        //vettore che contiene tutti gli elementi da sommare
        vett = (double*)malloc(dim*sizeof(double));

        //inizializzo vettore 
        for(int i = 0; i<dim; i++){
            vett[i] = rand()%10;
            test_somma += vett[i];
        }
        printf("La somma totale dovrebbe essere %f\n", test_somma);
    }

    //invio in broadcast numero elementi da sommare
    MPI_Bcast(&dim, 1, MPI_INT, 0, MPI_COMM_WORLD);

    //quanti elementi dare ad ogni processore
    nlocgen = dim/nproc;
    resto = dim%nproc;

    //nloc var locale ogni processo
    if(me < resto){
        nloc = nlocgen + 1;
    }else{
        nloc = nlocgen;
    }

    //vettore somme parziali (contiene pezzetto di vett)
    vett_loc = (double*)malloc(nloc * sizeof(double));

    if(me == 0){
        for(int i = 0; i<nloc; i++){
            vett_loc[i] = vett[i];
        }

        displs = nloc;

        for(int i = 1; i<nproc; i++){
            if(i < resto){
                MPI_Send(vett + displs, nloc, MPI_DOUBLE, i, i, MPI_COMM_WORLD);
                displs += nloc;
            }else{
                MPI_Send(vett + displs, nlocgen, MPI_DOUBLE, i, i, MPI_COMM_WORLD);
                displs += nlocgen;
            }
        }
    }else{
        MPI_Recv(vett_loc, nloc, MPI_DOUBLE, 0, me, MPI_COMM_WORLD, NULL);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    inizio = MPI_Wtime();

    for(int i = 0; i<nloc; i++){
        somma_locale += vett_loc[i];
    }

    //log2
    for(p = nproc; p != 1; p = p>>1){
        passi++;
    }

    potenze = (int*) malloc((passi + 1) * sizeof(int));
    for(int i = 0; i<= passi; i++){
        potenze[i] = p<<i;
    }

    for(int i = 0; i< passi; i++){

        if(me % potenze[i+1] < potenze[i]){
            comunicaCon = me + potenze[i];
        }else{
            comunicaCon = me - potenze[i];
        }

        MPI_Send(&somma_locale, 1, MPI_DOUBLE, comunicaCon, comunicaCon, MPI_COMM_WORLD);
        MPI_Recv(&somma_parziale, 1, MPI_DOUBLE, comunicaCon, me, MPI_COMM_WORLD, NULL);
        somma_locale += somma_parziale;
    }

    MPI_Barrier(MPI_COMM_WORLD);
    fine_locale = MPI_Wtime() - inizio;

    MPI_Reduce(&fine_locale,&fine,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);

    if(me == 0){
        printf("Tempo totale: %lf ms\n", fine * 1000);
    }
    printf("Somma locale al processore P%d: %f\n", me, somma_locale);

    
    MPI_Finalize();
    return 0;
}