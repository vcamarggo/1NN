#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <mpi.h>

#define TRUE 1
#define FALSE 0
#define GERENTE 0
#define FROM_MASTER 1
#define TO_MASTER 2

double **matrizTreino;
double **matrizTeste;
char **classeTreino;
char **classeTeste;
char *conteudoDasLinhas[100000];
int numeroDeLinhasTreino, numeroDeLinhasTeste, qtdeAtributos;
char *numeroDeColunas;

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
    matriz = (double **) calloc(qtdeLinhasLidas, sizeof (double*));

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
    return pow(matrizTeste[i][k] - matrizTreino[j][k], 2);
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    int id, size;
    MPI_Comm_rank(MPI_COMM_WORLD,&id);
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    double somaEuclidiana;
    double menorEuclidiana = FLT_MAX;
    int indiceMenorEuclidiana;
	MPI_Status st;

    if(id == GERENTE){
    	int acerto = 0;
    	int erro = 0;
  	 
		numeroDeColunas = (char *) malloc(4 * sizeof (char));
		puts("Digite a quantidade de atributos do arquivo desejado:");
		scanf("%4s", numeroDeColunas);
		qtdeAtributos = strtol(numeroDeColunas, (char **) NULL, 10);
		preencheVetorTreino();
	    preencheVetorTeste();
		
		int i = 0;
		int	numeroDeLinhasTesteParaTrabalhar = numeroDeLinhasTeste/(size-1);
		for(int j =1; j < size ;j++){
			MPI_Send(&numeroDeLinhasTreino,1,MPI_INT,j,FROM_MASTER,MPI_COMM_WORLD);
			if(j==size-1 && j>1){
				numeroDeLinhasTesteParaTrabalhar = numeroDeLinhasTeste/(size-1)+numeroDeLinhasTeste%(size-1);
				MPI_Send(&numeroDeLinhasTesteParaTrabalhar,1,MPI_INT,j,FROM_MASTER,MPI_COMM_WORLD);	
			}else{
				numeroDeLinhasTesteParaTrabalhar = numeroDeLinhasTeste/(size-1);
				MPI_Send(&numeroDeLinhasTesteParaTrabalhar,1,MPI_INT,j,FROM_MASTER,MPI_COMM_WORLD);			
			}
			MPI_Send(&qtdeAtributos,1,MPI_INT,j,FROM_MASTER,MPI_COMM_WORLD);

			int w = i;
			for (; i < numeroDeLinhasTesteParaTrabalhar+w; i++) {	
				for (int x = 0; x < qtdeAtributos; x++) {
					MPI_Send(&matrizTeste[i][x], 1, MPI_DOUBLE, j, FROM_MASTER, MPI_COMM_WORLD);
				}
			}

			for (int k = 0; k < numeroDeLinhasTreino; k++) {	
				for (int x = 0; x < qtdeAtributos; x++) {
					MPI_Send(&matrizTreino[k][x], 1, MPI_DOUBLE, j, FROM_MASTER, MPI_COMM_WORLD);
				}
			}
		}
		
		for(int j =1; j < size ;j++){
			int nrLinhasTemp = numeroDeLinhasTeste/(size-1);
			if(j==size-1 && j>1){
				nrLinhasTemp = nrLinhasTemp+numeroDeLinhasTeste%(size-1);
			}
			for (int k=0; k < nrLinhasTemp; k++) {
			  	MPI_Recv(&i,1,MPI_INT, j,TO_MASTER,MPI_COMM_WORLD, &st);
				MPI_Recv(&indiceMenorEuclidiana,1,MPI_INT,j,TO_MASTER,MPI_COMM_WORLD, &st);
				if(j==size-1 && j>1){
					i = i + (nrLinhasTemp-numeroDeLinhasTeste%(size-1))*(j-1);					
				} else
					i = i + nrLinhasTemp*(j-1);
				if (strcmp(classeTreino[indiceMenorEuclidiana], classeTeste[i]) == 0) {
					acerto++;
					printf("Acertou! A musica %i e do genero %s\n", i, classeTreino[indiceMenorEuclidiana]);
				} else {
					erro++;
					printf("Errou! A musica %i e do genero %s, mas o algoritmo indicou ser do genero %s\n",
					i, classeTeste[i], classeTreino[indiceMenorEuclidiana]);
				}
			}
		}

		if(acerto+erro == numeroDeLinhasTeste){
			printf("O algoritmo acertou %i\n", acerto);
			printf("O algoritmo errou %i\n", erro);
		}
		
    }else{
		MPI_Recv(&numeroDeLinhasTreino,1,MPI_INT,GERENTE,FROM_MASTER,MPI_COMM_WORLD, &st);
		MPI_Recv(&numeroDeLinhasTeste,1,MPI_INT, GERENTE,FROM_MASTER,MPI_COMM_WORLD, &st);
		MPI_Recv(&qtdeAtributos,1,MPI_INT,GERENTE,FROM_MASTER,MPI_COMM_WORLD, &st);

		matrizTeste = (double **) calloc(numeroDeLinhasTeste, sizeof (double*));
		for (int i = 0; i < numeroDeLinhasTeste; i++) {
			matrizTeste[i] = (double *) calloc(qtdeAtributos, sizeof (double));
		}

		matrizTreino = (double **) calloc(numeroDeLinhasTreino, sizeof (double*));
		for (int i = 0; i < numeroDeLinhasTreino; i++) {
			matrizTreino[i] = (double *) calloc(qtdeAtributos, sizeof (double));
		}

		for (int i = 0; i < numeroDeLinhasTeste; i++) {
			for (int x = 0; x < qtdeAtributos; x++) {
				MPI_Recv(&matrizTeste[i][x], 1, MPI_DOUBLE, GERENTE, FROM_MASTER, MPI_COMM_WORLD, &st);
			}
		}	

		for (int i = 0; i < numeroDeLinhasTreino; i++) {
			for (int x = 0; x < qtdeAtributos; x++) {
				MPI_Recv(&matrizTreino[i][x], 1, MPI_DOUBLE, GERENTE, FROM_MASTER, MPI_COMM_WORLD, &st);
			}
		}	

	   	for (int i = 0; i < numeroDeLinhasTeste; i++) {
    	    menorEuclidiana = FLT_MAX;
    	    for (int j = 0; j < numeroDeLinhasTreino; j++) {
    	        somaEuclidiana = 0;
    	        for (int k = 0; k < qtdeAtributos; k++) {
    	            somaEuclidiana += calculaDistanciaEuclidianaSimplificada(i, j, k);
    	        }
    	        somaEuclidiana = sqrt(somaEuclidiana);
    	        if (menorEuclidiana > somaEuclidiana) {
    	            menorEuclidiana = somaEuclidiana;
    	            indiceMenorEuclidiana = j;
	 	        }
    	    }
			MPI_Send(&i, 1, MPI_INT, GERENTE, TO_MASTER, MPI_COMM_WORLD);
			MPI_Send(&indiceMenorEuclidiana, 1, MPI_INT, GERENTE, TO_MASTER, MPI_COMM_WORLD);
    	}
    }
	MPI_Finalize();
    return 0;
}
