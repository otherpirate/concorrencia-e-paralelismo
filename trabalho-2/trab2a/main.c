#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include "stack.c"

#define F(arg) cosh(arg)*cosh(arg)*cosh(arg)*cosh(arg)
#define FALSE 0
#define TRUE 1
#define PROCESSAR 1
#define LIBERAR 2
#define PROCESSADO 3


int is_done(stack* bag, int* executors, int total_executors){
    if (!is_empty(bag)) {
        return FALSE;
    }
    for (int i=0; i<total_executors; i++){
        if (executors[i] == TRUE){
            return FALSE;
        }
    }
    return TRUE;
}

void feed(stack* bag, int* executors, int total_executors) {
    for (int i=0; i<total_executors; i++){
        if (!is_empty(bag) && executors[i] == FALSE) {
            printf("LIDER - ENVIANDO PARA %d\n", i+1);
            MPI_Send(pop(bag), 2, MPI_DOUBLE, i+1, PROCESSAR, MPI_COMM_WORLD);
            executors[i] = TRUE;
        }
    }
}

void release(int* executors, int total_executors) {
    for (int i=0; i<total_executors; i++){
        printf("LIDER - LIBERANDO %d\n", i+1);
        MPI_Send(NULL, 0, MPI_DOUBLE, i+1, LIBERAR, MPI_COMM_WORLD);
    }
}

void split(stack* bag, double start_at, double end_at, double interval) {
    double sync[2];
    for (double i=start_at; i<end_at; i=i+interval){
        sync[0] = i;
        sync[1] = i+interval;
        if (sync[1] > end_at) {
            sync[1] = end_at;
        }
        push(sync, bag);
        printf("X - %f %f\n", sync[0], sync[1]);
    }
}

double coordinator(int process, double start_at, double end_at, double interval) {
    double result = 0;
    MPI_Status status;

    stack* bag;
    bag = new_stack();
    double sync[2]; 
    split(bag, start_at, end_at, interval);

    int total_executors = process - 1;
    int* executors = (int*) malloc(sizeof(int)*(total_executors));
    for (int i=0; i<total_executors; i++){
        executors[i] = FALSE;
    }

    feed(bag, executors, total_executors);
    while (is_done(bag, executors, total_executors) == FALSE) {
        printf("LIDER - ESPERANDO RESPOSTAS...\n");
        MPI_Recv(sync, 2, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        executors[status.MPI_SOURCE - 1] = FALSE;
        printf("LIDER - RECEBI DE %d %d\n", status.MPI_SOURCE, status.MPI_TAG);
        result = result + sync[0];
        feed(bag, executors, total_executors);
    }
    release(executors, total_executors);
    return result;
}

void executor(me) {
    MPI_Status status;
    double sync[2];
    printf("%d - ESPERANDO\n", me);
    MPI_Recv(sync, 2, MPI_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    printf("%d - RECEBI %d\n", me, status.MPI_TAG);
    while (status.MPI_TAG == PROCESSAR) {
        double left = sync[0];
        double right = sync[1];
        double lrarea = (F(left) + F(right)) * (right - left)/2;
        double mid, fmid, larea, rarea;

        mid = (left + right)/2;
        fmid = F(mid);
        larea = (F(left) + fmid) * (mid - left)/2;
        rarea = (fmid + F(right)) * (right - mid)/2;
        double total_area = larea + rarea;

        sync[0] = total_area; 
        sync[1] = 0;
        MPI_Send(sync, 2, MPI_DOUBLE, 0, PROCESSADO, MPI_COMM_WORLD);
        printf("%d - ESPERANDO\n", me);
        MPI_Recv(sync, 2, MPI_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        printf("%d - RECEBI %d\n", me, status.MPI_TAG);
    }
    printf("%d - FUI LIBERADO\n", me);
}

int main(int argc, char **argv ) {
    int i, myid, numprocs;
    double area;
    double start_at = atof(argv[1]);
    double end_at = atof(argv[2]);
    double interval = atof(argv[3]);

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    
    if (myid == 0) {
        printf("LIDER %d\n", myid);
        area = coordinator(numprocs, start_at, end_at, interval);
    } else {
        printf("EXECUTOR %d\n", myid);
        executor(myid);
    }

    if(myid == 0) {
        printf("Area=%0.2f\n", area);
    }
    MPI_Finalize();
    return 0;
}
