#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

float **matrizTreino;
float **matrizTeste;
char **classe;
char *conteudoDasLinhas[100000];
int numeroDeLinhasLidas;
int qtdeAtributos;
char *numeroDeColunas;

void leArquivo(char *s[], int *linhasLidas, char *nomeArquivo)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    FILE *fp;

    fp = fopen(nomeArquivo, "r");

    if (fp == NULL)
    {
        printf("Erro ao abrir o arquivo\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Preparando seus dados...\n");
    }

    int i = 0;

    while ((read = getline(&line, &len, fp)) != -1)
    {
        s[i] = (char *)malloc(len * sizeof(char));
        strcpy(s[i++], line);
    }

    *linhasLidas = i;

    fclose(fp);
}

void preparaMatriz(char *linhas[], int qtdeLinhasLidas, int qtdeAtributos,float **matriz, int isTreino)
{
    char *token;

    matriz = (float **)calloc(qtdeLinhasLidas, sizeof(float));
    classe = (char **) malloc(qtdeLinhasLidas * 128);
    for (int i = 0; i < qtdeLinhasLidas; i++)
    {
        matriz[i] = (float *)calloc(qtdeAtributos, sizeof(float));
    }

    for (int i = 0; i < qtdeLinhasLidas; i++)
    {
        int j = 0;
        do
        {

            if (j == 0)
            {
                token = strtok(linhas[i], ",");
            }
            else
            {
                token = strtok(NULL, ",");
            }
            //TODO
            matriz[i][j] = strtof(token, NULL);
            j++;
        } while (token != 0 && j < (qtdeAtributos));
		if(isTreino){
			classe[i] = strtok(NULL, ",");
		}
    }
}


void criaNomeArquivo(char *nomeArquivo)
{
    char extensaoArquivo[6] = ".data";
    strcat(nomeArquivo, numeroDeColunas);
    strcat(nomeArquivo, extensaoArquivo);
}

void preencheVetorTrain()
{
    char nomeArquivo[14] = "bases\\train_";
    criaNomeArquivo(nomeArquivo);
    leArquivo(conteudoDasLinhas, &numeroDeLinhasLidas, nomeArquivo);
    printf("Preparando matriz de treino...\n");
    preparaMatriz(conteudoDasLinhas, numeroDeLinhasLidas, qtdeAtributos, matrizTreino, TRUE);
}

void preencheVetorTest()
{
    char nomeArquivo[13] = "bases\\test_";
    criaNomeArquivo(nomeArquivo);
    leArquivo(conteudoDasLinhas, &numeroDeLinhasLidas, nomeArquivo);
    printf("Preparando matriz de teste...\n");
    preparaMatriz(conteudoDasLinhas, numeroDeLinhasLidas, qtdeAtributos, matrizTeste, FALSE);
}

int main()
{
	numeroDeColunas = (char *)malloc(4 * sizeof(char));
    printf("Digite a quantidade de atributos do arquivo desejado:");
    scanf("%4s", numeroDeColunas);
    qtdeAtributos = strtol(numeroDeColunas, (char **)NULL, 10);
     
    preencheVetorTrain();
    preencheVetorTest();
	
    return 0;
}
