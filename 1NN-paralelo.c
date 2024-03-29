#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <pthread.h>

#define TRUE 1
#define FALSE 0

double **matrizTreino;
double **matrizTeste;
char **classeTreino;
char **classeTeste;
char *conteudoDasLinhas[100000];
int numeroDeLinhasTreino, numeroDeThreads, numeroDeLinhasTeste, qtdeAtributos;
char *numeroDeColunas;
double menorEuclidiana = FLT_MAX;
int testeAtual, indiceMenorEuclidiana;
pthread_mutex_t mut =PTHREAD_MUTEX_INITIALIZER;

void leArquivo(char *s[], int *linhasLidas, char *nomeArquivo) {
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    FILE *fp;

    fp = fopen(nomeArquivo, "r");

    if (fp == NULL) {
        printf("Erro ao abrir o arquivo\n");
        exit(EXIT_FAILURE);
    } else {
        printf("Preparando seus dados...\n");
    }

    int i = 0;

    while ((read = getline(&line, &len, fp)) != -1) {
        s[i] = (char *) malloc(len * sizeof (char));
        strcpy(s[i++], line);
    }

    *linhasLidas = i;

    fclose(fp);
}

double **preparaMatriz(char *linhas[], int qtdeLinhasLidas, int qtdeAtributos, int isTreino) {
    char *token;
    double **matriz;
    matriz = (double **) calloc(qtdeLinhasLidas, sizeof (double));

    if (isTreino) {
        classeTreino = (char **) malloc(qtdeLinhasLidas * 128);
    } else {
        classeTeste = (char **) malloc(qtdeLinhasLidas * 128);
    }

    for (int i = 0; i < qtdeLinhasLidas; i++) {
        matriz[i] = (double *) calloc(qtdeAtributos, sizeof (double));
    }

    for (int i = 0; i < qtdeLinhasLidas; i++) {
        int j = 0;
        do {

            if (j == 0) {
                token = strtok(linhas[i], ",");
            } else {
                token = strtok(NULL, ",");
            }
            //TODO
            if (i == 3374) {

            }
            matriz[i][j] = strtod(token, NULL);
            j++;
        } while (token != 0 && j < (qtdeAtributos));
        if (isTreino) {
            classeTreino[i] = strtok(strtok(NULL, ","), "\n");
        } else {
            classeTeste[i] = strtok(strtok(NULL, ","), "\n");
        }
    }
    return matriz;
}

void criaNomeArquivo(char *nomeArquivo) {
    char extensaoArquivo[6] = ".data";
    strcat(nomeArquivo, numeroDeColunas);
    strcat(nomeArquivo, extensaoArquivo);
}

void preencheVetorTreino() {
    char nomeArquivo[21] = "bases/train_";
    criaNomeArquivo(nomeArquivo);
    leArquivo(conteudoDasLinhas, &numeroDeLinhasTreino, nomeArquivo);
    printf("Preparando matriz de treino...\n");
    matrizTreino = preparaMatriz(conteudoDasLinhas, numeroDeLinhasTreino, qtdeAtributos, TRUE);
}

void preencheVetorTeste() {
    char nomeArquivo[21] = "bases/test_";
    criaNomeArquivo(nomeArquivo);
    leArquivo(conteudoDasLinhas, &numeroDeLinhasTeste, nomeArquivo);
    printf("Preparando matriz de teste...\n");
    matrizTeste = preparaMatriz(conteudoDasLinhas, numeroDeLinhasTeste, qtdeAtributos, FALSE);
}

double calculaDistanciaEuclidianaSimplificada(int i, int j, int k) {
    return pow(matrizTeste[i][k] - matrizTreino[j][k],2);
}

void *treinaParalelamente(void *arg) {
    int indiceInicial = (int) arg;
    for (int j = indiceInicial; j < numeroDeLinhasTreino; j += numeroDeThreads) {
        double somaEuclidiana = 0;
        for (int k = 0; k < qtdeAtributos; k++) {
            somaEuclidiana += calculaDistanciaEuclidianaSimplificada(testeAtual, j, k);
        }
        somaEuclidiana = sqrt(somaEuclidiana);
        pthread_mutex_lock(&mut);
        if (menorEuclidiana > somaEuclidiana) {
            menorEuclidiana = somaEuclidiana;
            indiceMenorEuclidiana = j;
        }
        pthread_mutex_unlock(&mut);
    }
}

pthread_t *criaThreads(int numeroDeThreads) {
    pthread_t *t;
    t = (pthread_t *) malloc(numeroDeThreads * sizeof (pthread_t));
    for (int i = 0; i < numeroDeThreads; i++) {
        pthread_create(&t[i], NULL, treinaParalelamente, (void*) i);
    }

    return t;
}

int main() {
    pthread_t *t;

    numeroDeColunas = (char *) malloc(4 * sizeof (char));
    printf("Digite a quantidade de atributos do arquivo desejado:");
    scanf("%4s", numeroDeColunas);
    qtdeAtributos = strtol(numeroDeColunas, (char **) NULL, 10);

    printf("Digite a quantidade de threads desejadas:");
    scanf("%i", &numeroDeThreads);

    preencheVetorTreino();
    preencheVetorTeste();
    int acerto = 0;
    int erro = 0;

    for (testeAtual = 0; testeAtual < numeroDeLinhasTeste; testeAtual++) {
        menorEuclidiana = FLT_MAX;

        t = criaThreads(numeroDeThreads);
        
        for (int i = 0; i < numeroDeThreads; i++) {
            pthread_join(t[i],NULL);
        }

        if (strcmp(classeTreino[indiceMenorEuclidiana], classeTeste[testeAtual]) == 0) {
            acerto++;
            printf("Acertou! A musica %i e do genero %s\n", testeAtual, classeTreino[indiceMenorEuclidiana]);
        } else {
            erro++;
            printf("Errou! A musica %i e do genero %s, mas o algoritmo indicou ser do genero %s\n",
                    testeAtual, classeTeste[testeAtual], classeTreino[indiceMenorEuclidiana]);
        }

    }
    printf("O algoritmo acertou %i\n", acerto);
    printf("O algoritmo errou %i", erro);

    return 0;
}