/*
    Nome: Gaetano Di Genio
    Matricola: 0522501339
    Programma: somma n numeri 2° strategia
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

int main(int argc, char* argv[]){
    int me, nproc, n, tag, i, nlocgen, resto, nloc, ind, *potenze;
    int r, recvBy, sendTo, tmp, passi = 0, p;
    double sommaloc = 0;
    double *vett, *vett_loc; 
    double T_inizio,T_fine,T_max;

    //inizio MPI
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &me);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    //creazione vettore che contiene elementi da sommare
    if(me == 0){
        printf("Inserire numero di elementi da sommare: \n");
        fflush(stdout);
        scanf("%d", &n);

        vett = (double*)calloc(n, sizeof(double));
    }

	/*invio del valore di n a tutti i processori appartenenti a MPI_COMM_WORLD*/
    //invio per valutazione eventuale resto
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    /*numero di addendi da assegnare a ciascun processore*/
	nlocgen=n/nproc; // divisione intera
    resto=n%nproc; // resto della divisione

    /* Se resto è non nullo, i primi resto processori ricevono un addendo in più */
    //nloc variabile finale da considerare
	if(me<resto)
	{
		nloc=nlocgen+1;
	}
	else
	{
		nloc=nlocgen;
	}

    /*allocazione di memoria del vettore per le somme parziali */
	vett_loc=(double*)calloc(nloc, sizeof(double));

    //inizializzo vettore grande vett
    if(me == 0){
        /*Inizializza la generazione random degli addendi utilizzando l'ora attuale del sistema*/                
        srand((double) time(0)); 

        for(i = 0; i<n; i++){
            /*creazione del vettore contenente numeri casuali */
			vett[i]=(int)rand()%5-2;
        }

    // P0 inizializza vettore locale suo usando valore di vett
        for(i=0;i<nloc;i++)
		{
			vett_loc[i]=vett[i];
		}

	// ind è il numero di addendi gi� assegnati     
		ind=nloc;

        /* P0 assegna i restanti addendi agli altri processori */
        for(i=1; i<nproc; i++){
            tag = i; /* tag del messaggio uguale all'id del processo che riceve*/
            /*SE ci sono addendi in sovrannumero da ripartire tra i processori*/
            if (i<resto){
                MPI_Send(vett+ind, nloc, MPI_DOUBLE, i, tag, MPI_COMM_WORLD);
                ind += nloc;
            }else{
                MPI_Send(vett+ind, nlocgen, MPI_DOUBLE, i, tag, MPI_COMM_WORLD);
                ind += nlocgen;
            }
        
        }
    //se non siamo processore 0 riceviamo dati
    }else{
        tag = me;
        MPI_Recv(vett_loc, nloc, MPI_DOUBLE, 0, tag, MPI_COMM_WORLD, &status);
    }

    //blocca finchè tutti i processori non arrivano qui
    MPI_Barrier(MPI_COMM_WORLD);

    T_inizio = MPI_Wtime(); //start conteggio del tempo

    for (i = 0; i<nloc; i++){
        /*ogni processore effettua la somma parziale*/
        sommaloc = sommaloc + vett_loc[i];
    }

    // calcolo di p=log_2 (nproc)
    p = nproc;

    while(p!=1){
        /*shifta di un bit a destra*/
        p = p>>1;
        passi ++;
    }

    /* creazione del vettore potenze, che contiene le potenze di 2*/
	potenze=(int*)calloc(passi+1,sizeof(int));

    for(i=0;i<=passi;i++)
	{
		potenze[i]=p<<i;
	}

    /* calcolo delle somme parziali e combinazione dei risultati parziali */
	for(i=0;i<passi;i++)
	{
		// ... calcolo identificativo del processore
		r=me%potenze[i+1];
		
		// Se il resto � uguale a 2^i, il processore menum invia
		if(r==potenze[i])
		{
			// calcolo dell'id del processore a cui spedire la somma locale
			sendTo=me-potenze[i];
			tag=sendTo;
			MPI_Send(&sommaloc,1,MPI_DOUBLE,sendTo,tag,MPI_COMM_WORLD);
		}
		else if(r==0) // se il resto � uguale a 0, il processore menum riceve
		{
			recvBy=me+potenze[i];
			tag=me;
			MPI_Recv(&tmp,1,MPI_DOUBLE,recvBy,tag,MPI_COMM_WORLD,&status);
			/*calcolo della somma parziale al passo i*/
			sommaloc=sommaloc+tmp;
		}//end
	}// end for

    MPI_Barrier(MPI_COMM_WORLD); // sincronizzazione
	T_fine=MPI_Wtime()-T_inizio; // calcolo del tempo di fine
 
	/* calcolo del tempo totale di esecuzione*/
	MPI_Reduce(&T_fine,&T_max,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);

	/*stampa a video dei risultati finali*/
	if(me==0)
	{
		printf("\nProcessori impegnati: %d\n", nproc);
		printf("\nLa somma e': %1.4lf\n", sommaloc);
		printf("\nTempo calcolo locale: %lf millisecondi\n", T_fine * 1000);
		printf("\nMPI_Reduce max time: %f\n",T_max);
	}// end if

    MPI_Finalize();
    return 0;
}