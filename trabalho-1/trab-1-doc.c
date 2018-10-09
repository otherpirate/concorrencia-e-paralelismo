int values[999]; // Lista dos valores, cada posição representa um produtor
int reads[999][999]; // Matriz de valores vs leitores, identifica que o valor já foi lido pelo leitor


void *write(int me) {
    for (int i = 1; i <= MESSAGES; i++) { // Gerando todas as mensagens
        while (is_all_read(me) == FALSE) {} // Esperando todo mundo ler o valor produzido anteiormente
        sem_wait(&semaphores[me]); // espera o semaforo ser liberado (Algum leitor ainda pode estar lendo)
        set_not_reads(me); values[me] = i; // seta valor
        sem_post(&semaphores[me]); // signal, libera o semaforo para os leitores
    }
 }

void *read(int me) {
    while (is_done() == FALSE) { // Verifica que tudo foi produzido e lido
        for (int producer = 0; producer < PRODUCERS; producer++) { // Checando todos os produtores
            if (values[producer] > 0 && reads[producer][me] == FALSE){ // Verifica sem tem valor pra ser lido
                sem_wait(&semaphores[me]); // await, espera os demais leitores liberarem o semaforo
                reads[producer][me] = TRUE; // Marca como lido
                sem_post(&semaphores[producer]); // signal, libera para as outras threads
            }
        }
    }
}
