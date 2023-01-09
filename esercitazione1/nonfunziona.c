#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

int main(int argc, char* argv[]){
    int me, nproc, ind, resto, *potenze, r;
    int passi = 0, p, comunicaCon;
    int n; //numero elementi da sommare dato da input utente
    double *vett; //vettore degli elementi random da sommare (uno solo)
    double *vett_loc; //vettore di ogni processo per somma (riceve pezzetto di vett)
    int nloc, nlocgen; //quanti elementi do ai processori
    double T_inizio,T_fine,T_max;
    double sommaloc = 0, tmp;

    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &me);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    //creo vettore con elementi da sommare
    if(me == 0){
        printf("Inserire il numero di elementi da sommare: \n");
        fflush(stdout);
        scanf("%d", &n);

        vett = (double*)malloc(n * sizeof(double));
    }

    //invio n a tutti i processori del gruppo
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    //quanti elementi processa ogni processore?
    nlocgen = n/nproc; //divisione intera n diviso i processi
    resto = n%nproc;  //resto divisione in caso non divisibile perfettamente

    if(me < resto){
        nloc = nlocgen + 1; //assegno valore in più a primi processori se divisione da' resto
    }else{
        nloc = nlocgen;
    }

    //vettore somme parziali
    vett_loc = (double*)malloc(nloc * sizeof(double));

    if(me == 0){
        //genero valori reali casuali da distribuire ai processi
        srand((double)time(0));
        

        //inizializzo vett
        printf("VETT : [ ");
        for(int i = 0; i<n; i++){
            vett[i] = (int)rand()%5;
            printf("%1.3f ", vett[i]);
        }
        printf("]\n");

        //P0 prende i primi elementi creati random per sommarli
        for(int i = 0; i<nloc; i++){
            vett_loc[i] = vett[i];
        }
        
        //offset per vett
        ind = nloc;

        printf("Sto qui 1\n");
        //invio altri elementi di vett agli altri processori
        for(int i = 1; i<nproc; i++){

            if(i<resto){
                MPI_Send(vett+ind, nloc, MPI_DOUBLE, i, i, MPI_COMM_WORLD);
                ind += nloc;
            }else{
                MPI_Send(vett+ind, nlocgen, MPI_DOUBLE, i, i, MPI_COMM_WORLD);
                ind += nlocgen;
            }
        }
        printf("Sto qui 2\n");

    }else{
        printf("Sto qui 3");

        MPI_Recv(vett_loc, nloc, MPI_DOUBLE, 0, me, MPI_COMM_WORLD, &status);
        printf("Sto qui 4\n");
    }

    //aspettiamo che tutti i processori arrivino qui
    MPI_Barrier(MPI_COMM_WORLD);

    printf("fin qui tutto ok");

    T_inizio = MPI_Wtime();

    //ogni processore fa la somma parziale dei propri elementi
    for(int i = 0; i<nloc; i++){
        sommaloc += vett_loc[i];
    }


    //calcolo di log_2 nproc
    p = nproc;

    while(p != 1){
        p = p>>1; //shifta bit a destra -> divisione intera per 2
        passi++;
    }

    //vettore potenze, contiene potenze di 2
    potenze = (int*)malloc((passi+1)* sizeof(int));

    for(int i = 0; i<=passi; i++){
        potenze[i] = p<<i; //shift a sinistra, moltiplicazione per 2
    }


    //algoritmo 3 strategia
    for(int i = 0; i < passi; i++){
        r = me%potenze[i+1]; //sarebbe resto (i, 2^k+1)

        if(r < potenze[i]){
            comunicaCon = me + potenze[i];
            MPI_Send(&sommaloc, 1, MPI_DOUBLE, comunicaCon, 1, MPI_COMM_WORLD);
            MPI_Recv(&tmp, 1, MPI_DOUBLE, comunicaCon, 1, MPI_COMM_WORLD, &status);
        }else{
            comunicaCon = me - potenze[i];
            MPI_Send(&sommaloc, 1, MPI_DOUBLE, comunicaCon, 1, MPI_COMM_WORLD);
            MPI_Recv(&tmp, 1, MPI_DOUBLE, comunicaCon, 1, MPI_COMM_WORLD, &status);     
        }

        sommaloc += tmp;
    }


    MPI_Barrier(MPI_COMM_WORLD);
    T_fine = MPI_Wtime() - T_inizio;

    MPI_Reduce(&T_fine, &T_max, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if(me == 0){
        printf("\nProcessori impegnati: %d\n", nproc);
		printf("\nTempo calcolo locale: %lf millisecondi\n", T_fine * 1000);
    }
    
    printf("Somma locale al processore %d: %1.4f\n", me, sommaloc);
    

    MPI_Finalize();
    return 0;
}