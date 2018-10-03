/*
Usando a tecnica de passagem de bastao, implemente em C uma
estrutura de dados que e um buffer limitado com N posicoes,
usado para broadcast entre P produtores e C consumidores. O
buffer oferece operacoes deposita e consome. Ao chamar
deposita, o produtor deve ficar bloqueado ate conseguir inserir o
novo item, e ao chamar consome o consumidor deve ficar
bloqueado ate conseguir um item para consumir. Uma posicao so
pode ser reutilizada quando todos os C consumidores tiverem lido
a mensagem. Cada consumidor deve receber as mensagens na
ordem em que foram depositadas.
*/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

int PRODUCERS = 10;
int CONSUMERS = 50;
int MESSAGES = 3;

int FALSE = 0;
int TRUE = 1;

sem_t *semaphores[999];
pthread_t threads[999];
int values[999];
int reads[999][999];


struct Me {
    int id;
};

int is_all_read(int line) {
    for (int i = 0; i < CONSUMERS; i++) {
        if (reads[line][i] == FALSE) {
            return FALSE;
        }
    }
    return TRUE;
}

int is_done() {
    for (int producer = 0; producer < PRODUCERS; producer++) {
        if (values[producer] != MESSAGES) {
            return FALSE;
        }
        if (is_all_read(producer) == FALSE) {
            return FALSE;
        }
    }
    return TRUE;
}

void set_not_reads(int line) {
    for (int i = 0; i < CONSUMERS; i++) {
        reads[line][i] = FALSE;
    }
}

void *write(void *data) {
    struct Me *struct_me = data;
    int me = struct_me->id;
    for (int i = 1; i <= MESSAGES; i++) {
        while (is_all_read(me) == FALSE) {}
        if (i > 1) {
            sem_wait(semaphores[me]);
        }
        printf("P%d - Produzindo...\n", me);
        set_not_reads(me);
        values[me] = i;
        printf("P%d - Produzido: %d\n", me, values[me]);
        sem_post(semaphores[me]);
    }
    free(struct_me);
    return NULL;
 }

void *read(void *data) {
    struct Me *struct_me = data;
    int me = struct_me->id;
    while (is_done() == FALSE) {
        for (int producer = 0; producer < PRODUCERS; producer++) {
            if (values[producer] > 0 && reads[producer][me] == FALSE) {
                sem_wait(semaphores[producer]);
                printf("C%d - Lendo P%d: %d\n", me, producer, values[producer]);
                reads[producer][me] = TRUE;
                printf("C%d - Liberando P%d\n", me, producer);
                sem_post(semaphores[producer]);
            }
        }
    }
    free(struct_me);
    return NULL;
}


void define_values(char *argv[]) {
    PRODUCERS = atoi(argv[1]);
    CONSUMERS = atoi(argv[2]);
    MESSAGES = atoi(argv[3]);
}

int main(int argc, char *argv[]) {
    define_values(argv);
    printf("\nP: %d, C: %d, M: %d\n", PRODUCERS, CONSUMERS, MESSAGES);
    int producer, consumer;
    struct Me *struct_me;
    sem_t *semaphore;
    for (producer = 0; producer < PRODUCERS; producer++) {
        semaphore = malloc(sizeof(sem_t));;
        semaphores[producer] = semaphore;
        sem_init(semaphore, 0, 0);

        pthread_t thread;
        struct_me = malloc(sizeof(struct Me));
        struct_me->id = producer;
        pthread_create(&thread, NULL, write, struct_me);
        threads[producer] = thread;

        for (int consumer = 0; consumer < CONSUMERS; consumer++) {
            reads[producer][consumer] = TRUE;
        }
    }

    for (consumer = 0; consumer < CONSUMERS; consumer++) {
        pthread_t thread;
        struct_me = malloc(sizeof(struct Me));
        struct_me->id = consumer;
        pthread_create(&thread, NULL, read, struct_me);
        threads[producer + consumer] = thread;
    }

    for (int i = 0; i < producer + consumer; i++) {
        pthread_join(threads[i], NULL);
    }

    values[0] = 0;
    for (int producer = 0; producer < PRODUCERS + CONSUMERS; producer++) {
        sem_destroy(semaphores[producer]);
    }
    printf("Done\n");
    return 0;
}