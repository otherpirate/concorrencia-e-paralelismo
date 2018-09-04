from collections import defaultdict

def validator():
    EMPTY = 0
    DOING = 1
    DONE = 2
    arquive = open('output.txt')
    line = next(arquive) and next(arquive)
    line = line.split(',')
    producers = int(line[0].replace("P: ", ""))
    consumers = int(line[1].replace("C: ", ""))
    messages = int(line[2].replace("M: ", ""))
    expected = []
    lines = (messages * 2 * producers * consumers) + (messages * 2 * producers) + 3
    for producer in range(producers):
        state = EMPTY
        arquive = open('output.txt')
        value = None
        reads = []
        creates = 0
        count = 0
        for line in arquive:
            count += 1
            line = line.strip()
            if 'P{} - Produzindo...'.format(producer) in line:
                state = DOING
                if value and len(reads) != consumers: 
                    raise Exception("NAO LEU TUDO {} {}".format(len(reads), consumers))
                continue

            if 'P{} - Produzido: '.format(producer) in line:
                state = DONE
                value = line.replace("P{} - Produzido: ".format(producer), "")
                reads = []
                creates += 1
                continue
            
            if 'Lendo P{}:'.format(producer) in line:
                if state != DONE:
                    raise Exception("LEU ANTES DE PRODUZIR {}".format(state))
                found = line.split("Lendo P{}: ".format(producer))[-1]
                if found != value:
                    print("LEU VALOR ERRADO {}-{}".format(value, found))
                consumer = line.split(" - ")[0]
                if consumer in reads:
                    raise Exception("LEU DUAS VEZES O MESMO {}".format(consumer))
                reads.append(consumer)
                continue

        if creates != messages:
            raise Exception("NAO PRODUZIU TUDO {} - {}".format(creates, messages))

        if count != lines:
            raise Exception("ARQUIVO INVALIDO {} - {}".format(count, lines))

validator()
