/*

	//LEMBRETE - Esse código só é garantido de funcionar se for executado diretamente no terminal do linux em uma máquina local, não é garantido o funcionamento completo em compiladores online e NÃO funciona no windows!!

	Para fazer este código rodar no windows vc deve:

		Substituir: '#define limpar "clear"' por '#define limpar "cls"'

		Refazer a função 'identificar()', sugiro tentar getAsyncKeyState()

		Remapear as teclas para compatibilidade com getAsyncKeyState()

		Substituir os 'Caracteres especiais para desenho' por caracteres não unicode, você pode usar a 'Tabela ASCII' junto com os comandos 'ALT+numPad'

		Substituir '<termios.h>' por '<windows.h>'

		Acho que '<unistd.h>' e '<fcntl.h>' tambem não são necessarias (se é que existem) para funcionar no windows

	Boa sorte :v
*/

//Para compilar use gcc bingo.c -o bingo -lm

//Bibliotecas gerais
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

//Bibliotecas do identificador
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

//Tamanho do "caractere de impressão"
#define cellSize 4
//Tamanho da tela
#define telaLin 30
#define telaCol 76
//Quantidade de números
#define numBolas 99
//Tamanho da cartela
#define cartelaLin 4
#define cartelaCol 5
//Limite de jogadores
#define maxJogadores 20
//Limites de tempo (μs)
#define minTick 30000
#define maxTick 100000
//Corrigir posição do menu
#define CPM_X 20
#define CPM_Y 7
//Corrigir posição do número sorteado
#define CPS_X 56
#define CPS_Y 3
//Corrigir posição dos sorteios anteriores
#define CPA_X 52
#define CPA_Y 13
//Corrigir posição das cartelas
#define CPC_X 4
#define CPC_Y 12
//Código para limpar tela
#define limpar "clear"

//#Possibilidades → transformar 'ETAPA' em um tipo 'enum'
//Etapas válidas    0→ Configurar    1→ Menu    2→ Jogo    3→ Tela Vencedor    4→ Creditos iniciais    5→ Configurar Jogadores
int ETAPA = 0, RUN = 1, GANHOU = 0, DEBUG = 0, CRITICO = 0, CONF = 0, DEV = 0;

//Caracteres especiais para desenho
char UD[] = "\u2550", UC[] = "\u2566", DC[] = "\u2569", LR[] = "\u2551", LC[] = "\u2560", RC[] = "\u2563", CT[] = "\u256C", UL[] = "\u2554", UR[] = "\u2557", DL[] = "\u255A", DR[] = "\u255D", AR[] = "\u25B8", AL[] = "\u25C2", AD[] = "\u25BE", AU[] = "\u25B4", EP[] = "\u2573";

//Barras
char barra[5][cellSize] = {" ","\u2591","\u2592","\u2593","\u2588"};

//Matriz principal = TelaLin*TelaCol*cellSize
char ***tela;

//Matriz do número personalizado = 3x(3xdigitos)xcellSize
char ***numPers;

//Matriz dos algarismos personalizados = 10x3x3xcellSize
char ****algPers;

//Textos predefinidos e Vetores temporários dos textos predefinidos
char txtSorteadas[] = "Numeros  Sorteados", tempTxtSorteadas[32][cellSize],
txtJogadores01[] = "Jogador:", tempTxtJogadores01[32][cellSize],
txtJogadores02[] = "Total:", tempTxtJogadores02[32][cellSize],
txtProximo[] = "Prox", tempTxtProximo[32][cellSize],
txtAnterior[] = "Ante", tempTxtAnterior[32][cellSize],
txtVitoria[] = " (Ganhou!)",// tempTxtVitoria[32][cellSize],
txtMenuJogar[] = "Jogar", tempTxtMenuJogar[32][cellSize],
txtMenuConfigurar[] = "Configurar", tempTxtMenuConfigurar[32][cellSize],
txtMenuCreditos[] = "Creditos", tempTxtMenuCreditos[32][cellSize],
txtMenuSair[] = "Sair", tempTxtMenuSair[32][cellSize];

//Vetores e Matrizes temporárias para strings
char tituloLinhas[7][50][cellSize];

char dgt[4][cellSize]; //Número de exibição do jogador atual e do total de jogadores

char tempColID[cartelaCol][cellSize]; //Cartela
char tempLinID[cartelaLin][cellSize]; //Cartela

char tempSorteadosD1[numBolas+1][cellSize];	//Digito 1 dos números sorteados personalizados
char tempSorteadosD2[numBolas+1][cellSize];	//Digito 2 dos números sorteados personalizados

//Variáveis de execução
int digitos, rodadas = numBolas, acertos = 0, nJ = 0, aJ = 0, jID = 1, tick = 75000;

//Auxiliar de validação para nomes
char *nCheck;

//Vetor de números sorteados para desenho
int numerosSorteadosDesenho[numBolas];

//Vetor de números sorteados
int *numerosSorteados;

//Auxiliar
int tStep = 0, tStepDir = 1;

//Obrigatoriedade 1
//Estrutura do jogador
typedef struct JOGADOR{
	int id;
	char nome[500];
	int **cartela;
	int **marcacao;
	int vencedor;

	char tempNome[500][cellSize];
	char tempCartelaD1[cartelaLin*cartelaCol][cellSize];
	char tempCartelaD2[cartelaLin*cartelaCol][cellSize];
} JOGADOR;

//Pontuação extra
//Estrutura para lista de jogadores
typedef struct JOGADORES{
	JOGADOR jogador;
	struct JOGADORES *prox;
	struct JOGADORES *ante;
} JOGADORES;

//Pré-carregar
//int main();

//Executar
void rodarJogo();
void limparTelaPrincipal();
void telaPrincipal();
void prepararJogadores(int pOPT, int selecionar);
void adicionarJogadores();
void removerJogadores();
void mostrarJogadores(int rmv);
void adicionarSorteados(int num);
int proximaRodada();
int reiniciar();
void finalizar();
void help_me();

//Desenho
void menuPrincipal(int mOpt, int selecionar);
void desenharCampos();
void desenharTitulo();
void desenharCampoSorteados();
void intPers(int num);
void desenharCampoCartelas(JOGADORES *jogadorAtual);
void desenharMultiplasCartelas(JOGADORES *jogadorAtual, int sizeX, int sizeY, int corX, int corY, int interX, int interY);
void desenharCartelas(JOGADORES *jogadorAtual, int corX, int corY);
void configurarDelay();
void creditos();
void informarVencedor();

//Funções da Lista
JOGADORES* criar();
int estaVazia(JOGADORES *listaJogadores);
JOGADORES* inserir(JOGADORES *listaJogadores, JOGADOR jogador);
JOGADORES* remover(JOGADORES *listaJogadores, int id);
void esvaziar(JOGADORES *listaJogadores);
int posicao(JOGADORES *listaJogadores, JOGADORES *jogadorAtual);

//Funções Auxiliares
int unicoCartela(int **cartela, int nC, int nL, int num);
int **ordenarCartela(int **cartela, int nC, int nL);
int **gerarCoringa(int **marcacao, int nC, int nL, int num);
void marcarAcertos(JOGADORES *jogadorAtual, int nC, int nL, int num);
void verificarVencedor();
int unicoSorteados(int *vetSort, int tam, int num);
int *insertionSorteados(int *vetSort, int tam, int num);
float max(float n1, float n2);
float min(float n1, float n2);
int capturar();
int identificar();

//Lista de jogadores
JOGADORES *jogadores = NULL;

//Âncora de impressão
JOGADORES *jogadorRef = NULL;
JOGADORES *multJogadorRef = NULL;

  //////////////////////////////////
 //         Pré-Carregar         //
//////////////////////////////////

//Carregamentos que so vão ocorrer uma vez por execução
int main() {
	int i, j, k;
	//Definir semente para função rand()
	srand(time(NULL));

	//Alternar modo de desenvolvedor
	char resp;
	do{
		system(limpar);

		for(i = 0; i < 12; i++) printf("\n");

		printf("%s",UL);
		for(i = 0; i < 75; i++)	printf("%s",UD);
		printf("%s",UR);

		printf("\n\u2551   Este programa não é compatível com compiladores online em seu modo de   \u2551\n\u2551 execução normal, porem, você ainda pode testar ele em compiladores online \u2551\n\u2551  desabilitando algumas configurações e ativando o modo de desenvolvedor.  \u2551\n");

		printf("%s",DL);
		for(i = 0; i < 75; i++)	printf("%s",UD);
		printf("%s",DR);

		printf("\n\n            Deseja realizar essas modificações? ('s' ou 'n'): ");
		scanf("%c",&resp);
	}while(resp != 's' && resp != 'n');
	if(resp == 's'){
		DEV = 1;
		ETAPA = 1;
		DEBUG = 1;
	} else{
		ETAPA = 4;
	}

	//Iniciar matriz principal
	tela = (char ***)calloc(telaLin,sizeof(char **));
	if(tela == NULL){
		printf("'tela' gerou um erro crítico ");
		getchar();
		return __LINE__;
	}
	for(j = 0; j < telaLin; j++){
		tela[j] = (char **)calloc(telaCol,sizeof(char *));
		if(tela[j] == NULL){
			printf("'tela[%d]' gerou um erro crítico ",j);
			getchar();
			return __LINE__;
		}
		for(i = 0; i < telaCol; i++){
			tela[j][i] = (char *)calloc(cellSize,sizeof(char));
			if(tela[j][i] == NULL){
				printf("'tela[%d][%d]' gerou um erro crítico ",j,i);
				getchar();
				return __LINE__;
			}
		}
	}

	//Iniciar matriz do número personalizado
	numPers = (char***)calloc(3,sizeof(char**));
	if(numPers == NULL){
		printf("'numPers' gerou um erro crítico ");
		getchar();
		return __LINE__;
	}
	for(j = 0; j < 3; j++){
		numPers[j] = (char**)calloc(3 * 2,sizeof(char*));
		if(numPers[j] == NULL){
			printf("'numPers[%d]' gerou um erro crítico ",j);
			getchar();
			return __LINE__;
		}
		for(i = 0; i < 3 * 2; i++){
			numPers[j][i] = (char *)calloc(cellSize,sizeof(char));
			if(numPers[j][i] == NULL){
				printf("'numPers[%d][%d]' gerou um erro crítico ",j,i);
				getchar();
				return __LINE__;
			}
		}
	}

	//Iniciar vetor de numeros sorteados
	numerosSorteados = (int *)calloc(1,sizeof(int));
	if(numerosSorteados == NULL){
		printf("'numerosSorteados' gerou um erro crítico ");
		getchar();
		return __LINE__;
	}

	//Iniciar matriz dos algarismos personalizados
	algPers = (char****)calloc(10,sizeof(char***));
	if(algPers == NULL){
		printf("'algPers' gerou um erro crítico ");
		getchar();
		return __LINE__;
	}
	for(k = 0; k < 10; k++){
		algPers[k] = (char***)calloc(3,sizeof(char**));
		if(algPers[k] == NULL){
			printf("'algPers[%d]' gerou um erro crítico ",k);
			getchar();
			return __LINE__;
		}
		for(j = 0; j < 3; j++){
			algPers[k][j] = (char**)calloc(3,sizeof(char*));
			if(algPers[k][j] == NULL){
				printf("'algPers[%d][%d]' gerou um erro crítico ",k,j);
				getchar();
				return __LINE__;
			}
			for(i = 0; i < 3; i++){
				algPers[k][j][i] = (char*)calloc(cellSize,sizeof(char));
				if(algPers[k][j][i] == NULL){
					printf("'algPers[%d][%d][%d]' gerou um erro crítico ",k,j,i);
					getchar();
					return __LINE__;
				}
			}
		}
	}
	//Preencher atriz de algarismos personalizados de 0 a 9
	//0
	strcpy(algPers[0][0][0],UL),			strcpy(algPers[0][0][1],UD),			strcpy(algPers[0][0][2],UR);
	strcpy(algPers[0][1][0],LR),			strcpy(algPers[0][1][1]," "),			strcpy(algPers[0][1][2],LR);
	strcpy(algPers[0][2][0],DL),			strcpy(algPers[0][2][1],UD),			strcpy(algPers[0][2][2],DR);
	//1
	strcpy(algPers[1][0][0]," "),  			strcpy(algPers[1][0][1],"\u2556"),		strcpy(algPers[1][0][2]," ");
	strcpy(algPers[1][1][0]," "),  			strcpy(algPers[1][1][1],LR),			strcpy(algPers[1][1][2]," ");
	strcpy(algPers[1][2][0]," "),  			strcpy(algPers[1][2][1],"\u2568"),		strcpy(algPers[1][2][2]," ");
	//2
	strcpy(algPers[2][0][0],UD),  			strcpy(algPers[2][0][1],UD),  			strcpy(algPers[2][0][2],UR);
	strcpy(algPers[2][1][0],UL),  			strcpy(algPers[2][1][1],UD),  			strcpy(algPers[2][1][2],DR);
	strcpy(algPers[2][2][0],DL),  			strcpy(algPers[2][2][1],UD),  			strcpy(algPers[2][2][2],UD);
	//3
	strcpy(algPers[3][0][0],UD),  			strcpy(algPers[3][0][1],UD),	 		strcpy(algPers[3][0][2],UR);
	strcpy(algPers[3][1][0],UD),  			strcpy(algPers[3][1][1],UD), 			strcpy(algPers[3][1][2],RC);
	strcpy(algPers[3][2][0],UD),  			strcpy(algPers[3][2][1],UD),			strcpy(algPers[3][2][2],DR);
	//4
	strcpy(algPers[4][0][0],"\u2556"),  	strcpy(algPers[4][0][1]," "), 	 		strcpy(algPers[4][0][2],"\u2553");
	strcpy(algPers[4][1][0],DL),  			strcpy(algPers[4][1][1],UD),  			strcpy(algPers[4][1][2],RC);
	strcpy(algPers[4][2][0]," "),  			strcpy(algPers[4][2][1]," "), 	 		strcpy(algPers[4][2][2],"\u2559");
	//5
	strcpy(algPers[5][0][0],UL),  			strcpy(algPers[5][0][1],UD),  			strcpy(algPers[5][0][2],UD);
	strcpy(algPers[5][1][0],DL),  			strcpy(algPers[5][1][1],UD),  			strcpy(algPers[5][1][2],UR);
	strcpy(algPers[5][2][0],UD),  			strcpy(algPers[5][2][1],UD),  			strcpy(algPers[5][2][2],DR);
	//6
	strcpy(algPers[6][0][0],UL),  			strcpy(algPers[6][0][1],UD),  			strcpy(algPers[6][0][2],UD);
	strcpy(algPers[6][1][0],LC),  			strcpy(algPers[6][1][1],UD),  			strcpy(algPers[6][1][2],UR);
	strcpy(algPers[6][2][0],DL),  			strcpy(algPers[6][2][1],UD),  			strcpy(algPers[6][2][2],DR);
	//7
	strcpy(algPers[7][0][0],UL),  			strcpy(algPers[7][0][1],UD),  			strcpy(algPers[7][0][2],UR);
	strcpy(algPers[7][1][0]," "), 			strcpy(algPers[7][1][1]," "),  			strcpy(algPers[7][1][2],LR);
	strcpy(algPers[7][2][0]," "),  			strcpy(algPers[7][2][1]," "),  			strcpy(algPers[7][2][2],"\u2559");
	//8
	strcpy(algPers[8][0][0],UL),  			strcpy(algPers[8][0][1],UD),  			strcpy(algPers[8][0][2],UR);
	strcpy(algPers[8][1][0],LC),  			strcpy(algPers[8][1][1],UD),  			strcpy(algPers[8][1][2],RC);
	strcpy(algPers[8][2][0],DL),  			strcpy(algPers[8][2][1],UD),  			strcpy(algPers[8][2][2],DR);
	//9
	strcpy(algPers[9][0][0],UL),  			strcpy(algPers[9][0][1],UD),  			strcpy(algPers[9][0][2],UR);
	strcpy(algPers[9][1][0],DL),  			strcpy(algPers[9][1][1],UD),  			strcpy(algPers[9][1][2],RC);
	strcpy(algPers[9][2][0],UD),  			strcpy(algPers[9][2][1],UD),  			strcpy(algPers[9][2][2],DR);

	//Iniciar vetor de bolas sorteadas
	for(i = 1; i < numBolas; i++) numerosSorteadosDesenho[i] = -1;

	//Definir Âncora
	jogadorRef = jogadores;
	multJogadorRef = jogadores;

	//Loop principal
	rodarJogo();

	//Finalizar jogo
	finalizar();

	return 0;
}

  //////////////////////////////
 //         Executar         //
//////////////////////////////

//Desenhar próximo frame
void rodarJogo(){
	int i, j;
  	int num = 0, comando = -1, oldComando = -1, oldNum = 0, sX = 2, sY = 2, mOpt = 0, pOPT = 0, selecionar = 0;

  	//Loop enquanto 'RUN==1'
	do{
		if(CRITICO) return;
		if(GANHOU) {
			if(ETAPA == 2) informarVencedor();
			ETAPA = 3;
		}
		if(rodadas <= 0 && ETAPA == 2) ETAPA = 3;

		if(comando!=-1) oldComando = comando;

		if(ETAPA == 0) configurarDelay();
		if(ETAPA == 4) creditos();
		if(ETAPA == 1){
			//Adicionar menu
			menuPrincipal(mOpt,selecionar);
			//Redefinir seleção
			selecionar = 0;
		}
		if(ETAPA == 5){
			prepararJogadores(pOPT,selecionar);
			selecionar = 0;
		}
		if(ETAPA == 2){
			//Adicionar linhas internas
			desenharCampos();
			//Adicionar número personalizado
			intPers(num);
			if(num != oldNum){
				//Adicionar número sorteado à lita
				adicionarSorteados(num);
				oldNum = num;

				//Verificar vencedor
				verificarVencedor();
			}
			//Adicionar campo de números sorteados
			desenharCampoSorteados();
			//Adicionar cartela
			desenharCampoCartelas(jogadorRef);
		}
		if(ETAPA == 1 || ETAPA == 2){
			//Adicionar informações
			desenharTitulo();
		}
		if(ETAPA == 3){
			//Adicionar multiplas cartelas para visualização final
			desenharMultiplasCartelas(multJogadorRef, sX, sY, 3, 1, 38, 14);
		}
		if(ETAPA != 5){
			//Imprimir tela principal
			telaPrincipal();
		}

		//Depuração
		if(DEBUG){
			printf("\nInformações adicionais:\n\n");
			printf("Delay = %dµs    comando = %d\n", tick, oldComando);
			printf("Jogadores = %d    Etapa = %d\n", nJ, ETAPA);
			printf("num = %d    oldNum = %d\n", num, oldNum);
			printf("Vetor sorteados { ");
			if(numBolas-rodadas > 0) for(i = 0; i < numBolas-rodadas; i++) printf("%d ",numerosSorteados[i]);
			printf("}\n");
			printf("Tamanho do vetor = %d\n", numBolas-rodadas);
		}

		if(!DEV){
			//Listar comando
			comando = capturar();
			switch(comando){
			//10 = ENTER
			case 10:
				if(ETAPA==0){
					ETAPA = 1;
				}else if(ETAPA==1){
					selecionar = 1;
				}else if(ETAPA==2){
					num = proximaRodada();
				}else if(ETAPA==3){
					//Reiniciar jogo
					if (reiniciar()) num = 0, comando = -1, oldComando = -1, oldNum = 0, sX = 2, sY = 2, mOpt = 0, pOPT = 0, selecionar = 0;
				}else if(ETAPA==5){
					selecionar = 1;
				}
			break;
			//32 = Barra de espaço
			case 32:
				DEBUG = (DEBUG+1)%2;
			break;
			//65 = Seta para cima / 'Shift + A'
			case 65:
				if(ETAPA==0){
					tick+=5000;
					if(tick > maxTick) tick = maxTick;
				}else if(ETAPA==1){
					mOpt--;
					if(mOpt < 0) mOpt = 3;
				}else if(ETAPA==2){
					ETAPA = 3;
				}else if(ETAPA==5){
					pOPT--;
					if(pOPT < 0) pOPT = 3;
				}
			break;
			//66 = Seta para baixo / 'Shift + B'
			case 66:
				if(ETAPA==0){
					tick-=5000;
					if(tick < minTick) tick = minTick;
				}else if(ETAPA==1){
					mOpt++;
					if(mOpt >= 4) mOpt = 0;
				}else if(ETAPA==3){
					if(rodadas>0 && !GANHOU) ETAPA = 2;
				}else if(ETAPA==5){
					pOPT++;
					if(pOPT >= 4) pOPT = 0;
				}
			break;
			//67 = Seta para a direita / 'Shift + C'
			case 67:
				if(ETAPA==2){
					jogadorRef = jogadorRef->prox;
				}else if(ETAPA==3){
					for(i = 0; i < sX*sY; i++){
						multJogadorRef = multJogadorRef->prox;
						if(multJogadorRef == jogadores) break;
					}					
				}
			break;
			//68 = Seta para a esquerda / 'Shift + D'
			case 68:
				if(ETAPA==2){
					jogadorRef = jogadorRef->ante;
				}else if(ETAPA==3){
					if(multJogadorRef == jogadores){
						for(i = 0; i < (nJ%(sX*sY)==0?4:nJ%(sX*sY)); i++) multJogadorRef = multJogadorRef->ante;
					}else{
						for(i = 0; i < sX*sY; i++) multJogadorRef = multJogadorRef->ante;
					}
				}
			break;
			//'Shift + H'
			case 72:
				help_me();
			break;
			}
		}else{
			int escolha = -1;
			int escolhaMax = 6;

			printf("\n  Escolha uma das seguintes ações: ");
			if(ETAPA == 0){

			}else if(ETAPA == 1 || ETAPA == 5){
				escolhaMax = 3;
				printf("\n   1%s Proxima Opção", AR);
				printf("\n   2%s Opção Anterior", AR);
				printf("\n   3%s Selecionar", AR);
			}else if(ETAPA == 2){
				escolhaMax = 6;
				printf("\n   1%s Visualizar proximo jogador", AR);
				printf("\n   2%s Visualizar jogador anterior", AR);
				printf("\n   3%s Proxima rodada", AR);
				printf("\n   4%s Finalizar partida", AR);
				printf("\n   5%s Jogar todos os numeros",AR);
				printf("\n   6%s Visualizar todos os jogadores", AR);
			}else if(ETAPA == 3){
				escolhaMax = 3;
				printf("\n   1%s Visualizar proximo página", AR);
				printf("\n   2%s Visualizar página anterior", AR);
				printf("\n   3%s Voltar ao jogo", AR);
			}

			printf("\n\n   0%s Finalizar", AR);

			while(escolha < 0 || escolha > escolhaMax){
				printf("\n\n  Escolha uma opção: ");
				scanf("%d",&escolha);
				getchar();
			}
			switch(escolha){
				case 1:
					if(ETAPA==1){
						mOpt++;
						if(mOpt >= 4) mOpt = 0;
					}else if(ETAPA==2){
						jogadorRef = jogadorRef->prox;
					}else if(ETAPA==3){
						for(i = 0; i < sX*sY; i++){
							multJogadorRef = multJogadorRef->prox;
							if(multJogadorRef == jogadores) break;
						}				
					}else if(ETAPA==5){
						pOPT++;
						if(pOPT >= 4) pOPT = 0;
					}
				break;
				case 2:
					if(ETAPA==1){
						mOpt--;
						if(mOpt < 0) mOpt = 3;
					}else if(ETAPA==2){
						jogadorRef = jogadorRef->ante;
					}else if(ETAPA==3){
						if(multJogadorRef == jogadores){
							for(i = 0; i < (nJ%(sX*sY)==0?4:nJ%(sX*sY)); i++) multJogadorRef = multJogadorRef->ante;
						}else{
							for(i = 0; i < sX*sY; i++) multJogadorRef = multJogadorRef->ante;
						}
					}else if(ETAPA==5){
						pOPT--;
						if(pOPT < 0) pOPT = 3;
					}
				break;
				case 3:
					if(ETAPA==1 || ETAPA==5){
						selecionar = 1;
					}else if(ETAPA==2){
						num = proximaRodada();
					}else if(ETAPA==3){
						if(rodadas>0 && !GANHOU){
							ETAPA = 2;
						}else{
							printf("\n\nO contador de rodadas zerou!");
						}
					}
				break;
				case 4:
					if(ETAPA==2){
						while(!GANHOU){
							do{
								num = rand() % numBolas + 1;
							}while(!unicoSorteados(numerosSorteados, numBolas-rodadas+1, num) || num > 99 || num < 1);
							acertos = 0;
							JOGADORES *jogadorAtual = jogadores;
							do{
								if(jogadorAtual == NULL) break;
								marcarAcertos(jogadorAtual, cartelaCol, cartelaLin, num);
								jogadorAtual = jogadorAtual->prox;
							}while(jogadorAtual != jogadores);
							rodadas--;
							adicionarSorteados(num);
							verificarVencedor();			
						}
					}
				break;
				case 5:
					if(ETAPA==2){
						JOGADORES *jogadorAtual = jogadores;
						for(i = 1; i <= numBolas; i++){
							jogadorAtual = jogadores;
							do{
								if(jogadorAtual == NULL) break;
								marcarAcertos(jogadorAtual, cartelaCol, cartelaLin, i);
								jogadorAtual = jogadorAtual->prox;
							}while(jogadorAtual != jogadores);
							rodadas--;
							adicionarSorteados(i);
						}
						verificarVencedor();
					}
				break;
				case 6:
					if(ETAPA==2){
						ETAPA = 3;
					}
				break;
				case 0:
					if(ETAPA==1){
						RUN = 0;
					}else{
						if (reiniciar()) num = 0, comando = -1, oldComando = -1, oldNum = 0, sX = 2, sY = 2, mOpt = 0, pOPT = 0, selecionar = 0;
					}
				break;
			}
		}
		//Verificar encerramento
		if(!RUN){
			char resp;
			do{
				printf("\n\nDeseja sair do jogo? ('s' ou 'n'): ");
				resp = fgetc(stdin);
				getchar();
			}while(resp != 's' && resp != 'n');
			if(resp == 'n') RUN = 1;
		}
		usleep(tick);
	}while(RUN);
}

//Limpar array principal e redesenhar borda
void limparTelaPrincipal(){
  	int i, j;

	system(limpar);
	for(j = 0; j < telaLin; j++){
		for(i = 0; i < telaCol; i++){
			strcpy(tela[j][i]," ");
			if(j == 0 || j == telaLin-1) strcpy(tela[j][i] , UD);
			if(i == 0 || i == telaCol-1) strcpy(tela[j][i] , LR);
		}
	}
	strcpy(tela[0][0] , UL);
	strcpy(tela[0][telaCol-1] , UR);
	strcpy(tela[telaLin-1][0] , DL);
	strcpy(tela[telaLin-1][telaCol-1] , DR);
}

//Printar array principal
void telaPrincipal(){
	int i, j;
	// 1 -> Print normal
	// 0 -> Print debug
	system(limpar);

	//Texto de configuração
	if(ETAPA == 0){
		printf("\nUse as setas 'cima' e 'baixo' para modificar o tempo de espera entre cada print da tela, quando a tela parar de piscar pressione 'Enter'\n");
		printf("\nTempo de espera atual %dμs\n\n",tick);
	}

	if(1){
		for(j = 0; j < telaLin; j++){
		for(i = 0; i < telaCol; i++){
			if(strlen(tela[j][i]) == 1 || strlen(tela[j][i]) == 3){
				printf("%s",tela[j][i]);
			}else{
				printf("?");
			}
		}
		printf("\n");
		}
	}else{
		for(j = 0; j < telaLin; j++){
			//Unicos valores permitidos são '1' e '3'
			for(i = 0; i < telaCol; i++) printf("%ld",strlen(tela[j][i]));
			printf("\n");
		}
	}
}

//Menu para manipulação de jogadores
void prepararJogadores(int pOPT, int selecionar){
	int i;

	char txtMenu[4][32] = {"Adicionar Jogadores", "Remover Jogadores", "Iniciar Jogo", "Voltar"};

	mostrarJogadores(0);

	//Validar seleção do menu
	if(selecionar){
		switch(pOPT){
			case 0:
				if(nJ >= maxJogadores){
					printf("\n\nJogadores cadastrados (%d/%d)\n\n",nJ,maxJogadores);
					usleep(500000);
					break;
				}
				adicionarJogadores();
			break;
			case 1:
				if(jogadores==NULL){
					printf("\n\nNenhum jogador cadastrado\n\n");
					usleep(500000);
					break;
				}
				removerJogadores();
			break;
			case 2:
				if(jogadores==NULL){
					printf("\n\nNenhum jogador cadastrado\n\n");
					usleep(500000);
					break;
				}
				ETAPA = 2;
				return;
			break;
			case 3:
				ETAPA = 1;
				return;
			break;
		}
	}

	//Imprimir menu
	for(i = 0; i < 4; i++){
		printf("	%s  %s\n",i==pOPT?"\u25BA":" ",txtMenu[i]);
	}
}

//Adicionar jogadores, gerar/ordenar cartela e gerar coringas
void adicionarJogadores(){
	int i, j;
	JOGADOR jogador;

	//Variável para respostas
	char resp;

	do{
		mostrarJogadores(0);

		printf("\nDeseja dar nome ao jogador? ('s' ou 'n'): ");
		resp = fgetc(stdin);
		getchar();
	}while(resp != 's' && resp != 'n');

	if(resp == 's'){
		printf("\nDê um nome, sem acentuação(Ex.: 'á') nem caracteres especiais(Ex.: 'ç'), ao jogador %d: ", jID);
		fgets(jogador.nome, 500, stdin);
		//Remover '\n' do texto recebido pela função fgets()
		if((nCheck=strchr(jogador.nome, '\n')) != NULL) *nCheck = '\0';
	}else{
		sprintf(jogador.nome,"Jogador %d",jID);
	}

	//Redefinir condição de vitória
	jogador.vencedor = 0;

	//Definir ID
	jogador.id = jID;

	//Adicionar cartela
	jogador.cartela = (int **)calloc(cartelaLin, sizeof(int *));
	if(jogador.cartela == NULL){
		printf("'jogador[%d].cartela' gerou um erro crítico %d",nJ,__LINE__);
		CRITICO = 1;
		getchar();
		return;
	}
	jogador.marcacao = (int **)calloc(cartelaLin, sizeof(int *));
	if(jogador.marcacao == NULL){
		printf("'jogador[%d].marcacao' gerou um erro crítico %d",nJ,__LINE__);
		CRITICO = 1;
		getchar();
		return;
	}
	//Iniciar nova cartela
	for(j = 0; j < cartelaLin; j++){
		jogador.cartela[j] = (int *)calloc(cartelaCol,sizeof(int));
		if(jogador.cartela[j] == NULL){
			printf("'jogador[%d].cartela[%d]' gerou um erro crítico %d",nJ,j,__LINE__);
			CRITICO = 1;
			getchar();
			return;
		}
		jogador.marcacao[j] = (int *)calloc(cartelaCol,sizeof(int));
		if(jogador.marcacao[j] == NULL){
			printf("'jogador[%d].marcacao[%d]' gerou um erro crítico %d",nJ,j,__LINE__);
			CRITICO = 1;
			getchar();
			return;
		}
	}

	//Preencher cartela
	for(j = 0; j < cartelaLin; j++){
		for(i = 0; i < cartelaCol; i++){
			int num = 0;
			//Enquanto o número aleatório não for unico...
			while(!unicoCartela(jogador.cartela,cartelaCol,cartelaLin,num) || num <= 0 || num > numBolas){
				//Gere um número aleatório
				num = rand() % numBolas + 1;
			}
			//Adicionar valor na cartela
			jogador.cartela[j][i] = num;
			jogador.marcacao[j][i] = 0;
		}
	}

	//Ordenar cartela
	jogador.cartela = ordenarCartela(jogador.cartela,cartelaCol,cartelaLin);
	//Gerar celulas coringa na cartela
	jogador.marcacao = gerarCoringa(jogador.marcacao,cartelaCol,cartelaLin,3);
	//Adicionar jogador
	jogadores = inserir(jogadores, jogador);
	//Atualizar âncora
	jogadorRef = jogadores;
	multJogadorRef = jogadores;
}

//Remover jogador pelo ID
void removerJogadores(){
	int id;

	if(jogadores == NULL) return;

	mostrarJogadores(1);

	printf("\nInforme o 'ID' do jogador a ser removido: ");
	scanf("%d",&id);
	getchar();

	//Remover jogador
	jogadores = remover(jogadores, id);
	//Atualizar âncora
	jogadorRef = jogadores;
	multJogadorRef = jogadores;
}

//Mostar jogadores existentes
void mostrarJogadores(int rmv){
	int i, j, k, pCol;
	JOGADORES *jogadorAtual = jogadores;

	char txtInicial[] = "::Jogadores atuais::";

	//Largura da lista
	pCol = strlen(txtInicial);
	do{
		if(jogadorAtual == NULL) break;
		pCol = max(pCol,strlen(jogadorAtual->jogador.nome)+(rmv*5));
		jogadorAtual = jogadorAtual->prox;
	}while(jogadorAtual != jogadores);

	system(limpar);

	//Lista de jogadores
	printf("\n	%s",UL);
	for(i = 0; i < pCol+2; i++) printf("%s",UD);
	printf("%s\n",UR);

	printf("	%s",LR);
	for(i = 0; i < pCol+2; i++) printf(" ");
	printf("%s\n",LR);

	printf("	%s ",LR);
	printf("%s",txtInicial);
	for(i = 0; i < pCol-strlen(txtInicial)+1; i++) printf(" ");
	printf("%s\n",LR);

	printf("	%s",LR);
	for(i = 0; i < pCol+2; i++) printf(" ");
	printf("%s\n",LR);

	jogadorAtual = jogadores;
	if(!rmv){
	do{
			if(jogadorAtual == NULL) break;
			printf("	%s ",LR);
			printf("%s",jogadorAtual->jogador.nome);
			for(j = 0; j < pCol-strlen(jogadorAtual->jogador.nome)+1; j++) printf(" ");
			printf("%s\n",LR);
			jogadorAtual = jogadorAtual->prox;
		}while(jogadorAtual != jogadores);
	}else{
		do{
			if(jogadorAtual == NULL) break;
			printf("	%s ",LR);
			printf("(%2d) %s",jogadorAtual->jogador.id, jogadorAtual->jogador.nome);
			for(j = 0; j < pCol-(strlen(jogadorAtual->jogador.nome)+(rmv*4)); j++) printf(" ");
			printf("%s\n",LR);
			jogadorAtual = jogadorAtual->prox;
		}while(jogadorAtual != jogadores);
	}

	printf("	%s",LR);
	for(i = 0; i < pCol+2; i++) printf(" ");
	printf("%s\n",LR);

	printf("	%s",DL);
	for(i = 0; i < pCol+2; i++) printf("%s",UD);
	printf("%s\n\n",DR);
}

//Chamar proxima rodada e retornar o número sorteado
int proximaRodada(){
	int num = 0;
	//Sortear nova bola
	do{
		num = rand() % numBolas + 1;
	}while(!unicoSorteados(numerosSorteados, numBolas-rodadas+1, num) || num > 99 || num < 1);
	//Verificar acertos
	acertos = 0;
	JOGADORES *jogadorAtual = jogadores;
	do{
		if(jogadorAtual == NULL) break;
		marcarAcertos(jogadorAtual, cartelaCol, cartelaLin, num);
		jogadorAtual = jogadorAtual->prox;
	}while(jogadorAtual != jogadores);
	rodadas--;

	return num;
}

//Adicionar número sorteado ao vetor de desenho e ao vetor de números sorteados
void adicionarSorteados(int num){
	numerosSorteadosDesenho[num-1] = num;
	numerosSorteados = insertionSorteados(numerosSorteados, numBolas-rodadas-1, num);
}

//Reiniciar todas as variáveis de execução
int reiniciar(){
	int i;
	char resp;
	do{
		printf("\n\nDeseja reiniciar o jogo? ('s' ou 'n'): ");
		resp = fgetc(stdin);
		getchar();
	}while(resp != 's' && resp != 'n');
	if(resp == 'n') return 0;

	system(limpar);
	printf("Reiniciando {\n");

	rodadas = 99, acertos = 0, nJ = 0, aJ = 0, jID = 1, tStep = 0, tStepDir = 1, GANHOU = 0, CRITICO = 0, RUN = 1;
	printf("	Variáveis principais\n");

	esvaziar(jogadores);
	printf("	Jogadores\n");
	jogadores = criar();
	if(!estaVazia(jogadores)){
		printf("'esvaziar(jogadores) / jogadores = criar()' gerou um erro crítico %d",__LINE__);
		CRITICO = 1;
		getchar();
	}

	//Iniciar vetor de numeros sorteados
	numerosSorteados = (int *)realloc(numerosSorteados,sizeof(int));
	printf("	Números sorteados\n");

	//Iniciar vetor de bolas sorteadas
	for(i = 1; i <= 99; i++){
		numerosSorteadosDesenho[i] = -1;
	}
	numerosSorteadosDesenho[0] = 0;
	printf("	Matriz de desenho\n}\n\n");

	ETAPA = 1;
	return 1;
}

//Liberar todos os ponteiros
void finalizar(){
	int i, j, k, l;

	//Mensagem de finalização
	system(limpar);
	char msgFinal[] = "Obrigado por Jogar :D";
	for(i = 0; i < (telaLin-3)/2; i++)	printf("\n");

	for(i = 0; i < (telaCol-(strlen(msgFinal)+2))/2 - 1; i++) printf(" ");
	printf("%s",UL);
	for(i = 0; i < strlen(msgFinal)+2; i++)	printf("%s",UD);
	printf("%s\n",UR);

	for(i = 0; i < (telaCol-(strlen(msgFinal)+2))/2 - 1; i++) printf(" ");
	printf("%s %s %s\n",LR,msgFinal,LR);

	for(i = 0; i < (telaCol-(strlen(msgFinal)+2))/2 - 1; i++) printf(" ");
	printf("%s",DL);
	for(i = 0; i < strlen(msgFinal)+2; i++)	printf("%s",UD);
	printf("%s\n",DR);
	usleep(3000000);

	//Finalizar
	system(limpar);
	printf("LIMPANDO{\n");

	//Liberar matriz principal
	for(j = 0; j < telaLin; j++) for(i = 0; i < telaCol; i++) free(tela[j][i]);
	for(j = 0; j < telaLin; j++) free(tela[j]);
	free(tela);
	printf("	Matriz principal\n");

	//Liberar matriz do número personalizado
	for(j = 0; j < 3; j++) for(i = 0; i < 3 * 2; i++) free(numPers[j][i]);
	for(j = 0; j < 3; j++) free(numPers[j]);
	free(numPers);
	printf("	Matriz número personalizado\n");

	//Liberar vetor de numeros sorteados
	free(numerosSorteados);
	printf("	Números sorteados\n");

	//Liberar algarismos personalizados
	for(k = 0; k < 10; k++) for(j = 0; j < 3; j++) for(i = 0; i < 3; i++) free(algPers[k][j][i]);
	for(k = 0; k < 10; k++) for(j = 0; j < 3; j++) free(algPers[k][j]);
	for(k = 0; k < 10; k++) free(algPers[k]);
	free(algPers);
	printf("	Algarismos personalizados\n");

	//Liberar jogadores
	esvaziar(jogadores);
	printf("	Jogadores\n");
	printf("}\n");

	system(limpar);
}

//Informar todos os atalhos
void help_me(){
	system(limpar);

	printf("Teclas válidas globalmente:\n");
	printf("	%-20s tela de ajuda\n","'Shift+H'");
	printf("	%-20s  visualizar informações\n","'Barra de Espaço'");

	printf("\nTeclas válidas no menu:\n");
	printf("	%-20s confirma a opção selecionada\n","'Enter'");
	printf("	%-20s navegar no menu\n","'Seta cima'");
	printf("	%-20s navegar no menu\n","'Seta baixo'");

	printf("\nTeclas válidas nas configurações:\n");
	printf("	%-20s confirmar alterações\n","'Enter'");
	printf("	%-20s aumentar tempo de espera\n","'Seta cima'");
	printf("	%-20s diminuir tempo de espera\n","'Seta baixo'");

	printf("\nTeclas válidas no jogo:\n");
	printf("	%-20s proxima rodada\n","'Enter'");
	printf("	%-20s proximo jogador\n","'Seta direita'");
	printf("	%-20s jogador anterior\n","'Seta esquerda'");
	printf("	%-20s visualizar jogadores\n","'Seta cima'");

	printf("\nTeclas válidas na visualização de jogadores:\n");
	printf("	%-20s retorna ao menu (Reinicia o jogo)\n","'Enter'");
	printf("	%-20s proximos jogadores\n","'Seta direita'");
	printf("	%-20s jogadores anteriores\n","'Seta esquerda'");
	printf("	%-20s voltar para o jogo\n","'Seta baixo'");

	printf("\n\nPressione 'Enter' para continuar...");
	getchar();
}

  /////////////////////////////
 //         Desenho         //
/////////////////////////////

//Desenhar menu
void menuPrincipal(int mOpt, int selecionar){
	int i, j;
	int lSize = 31;

	//Validar opção
	mOpt %= 4;

	//Configurar alinhamento
	int mCorX = max(strlen(txtMenuJogar),max(strlen(txtMenuConfigurar), max(strlen(txtMenuCreditos), strlen(txtMenuSair))));
	mCorX += mCorX % 2;
	int mCorY = 9;

	//Bordas
	for(j = 0; j < telaLin; j++){
		for(i = 0; i < telaCol; i++){
			strcpy(tela[j][i] , " ");
			if((j == CPM_Y || j == CPM_Y+14) && i >= CPM_X && i <= CPM_X+lSize+3) strcpy(tela[j][i] , UD);
			if((i == CPM_X || i == CPM_X+lSize+3) && j >= CPM_Y && j <= CPM_Y+14) strcpy(tela[j][i] , LR);
		}
	}
	strcpy(tela[CPM_Y][CPM_X] , UL);
	strcpy(tela[CPM_Y][CPM_X+lSize+3] , UR);
	strcpy(tela[CPM_Y+14][CPM_X] , DL);
	strcpy(tela[CPM_Y+14][CPM_X+lSize+3] , DR);

	//Adicionar textos do menu
	for(i = 0; i < strlen(txtMenuJogar); i++){
		sprintf(tempTxtMenuJogar[i], "%c", txtMenuJogar[i]);
		strcpy(tela[mCorY+CPM_Y][i+(mCorX/2)+((lSize-mCorX)/2)+CPM_X] , tempTxtMenuJogar[i]);
	}
	for(i = 0; i < strlen(txtMenuConfigurar); i++){
		sprintf(tempTxtMenuConfigurar[i], "%c", txtMenuConfigurar[i]);
		strcpy(tela[mCorY+1+CPM_Y][i+(mCorX/2)+((lSize-mCorX)/2)+CPM_X] , tempTxtMenuConfigurar[i]);
	}
	for(i = 0; i < strlen(txtMenuCreditos); i++){
		sprintf(tempTxtMenuCreditos[i], "%c", txtMenuCreditos[i]);
		strcpy(tela[mCorY+2+CPM_Y][i+(mCorX/2)+((lSize-mCorX)/2)+CPM_X] , tempTxtMenuCreditos[i]);
	}
	for(i = 0; i < strlen(txtMenuSair); i++){
		sprintf(tempTxtMenuSair[i], "%c", txtMenuSair[i]);
		strcpy(tela[mCorY+3+CPM_Y][i+(mCorX/2)+((lSize-mCorX)/2)+CPM_X] , tempTxtMenuSair[i]);
	}
	//Adicionar seta do menu
	strcpy(tela[mCorY+mOpt+CPM_Y][(mCorX/2)+((lSize-mCorX)/2)-3+CPM_X] , "\u25BA");

	//Validar seleção
	if(selecionar){
		switch(mOpt){
			case 0:
				ETAPA = 5;
			break;
			case 1:
				if(DEV){
					printf("\n\nA configuração não esta disponível no modo DEV!\n\n");
					usleep(3000000);
					system(limpar);
					return;
				}
				ETAPA = 0;
			break;
			case 2:
				ETAPA = 4;
			break;
			case 3:
				RUN = 0;
			break;
		}
	}
}

//Desenhar divisões
void desenharCampos(){
	int i, j;

	limparTelaPrincipal();

	for(j = 0; j < telaLin; j++){
		for(i = 0; i < telaCol; i++){
		//Campo de titulo
		if((j == 1 || j == 9) && i >= 2 && i <= 48) strcpy(tela[j][i] , UD);
		if((i == 2 || i == 48) && j >= 1 && j <= 9) strcpy(tela[j][i] , LR);
		//Campo de cartelas
		if((j == 10 || j == 28) && i >= 2 && i <= 48) strcpy(tela[j][i] , UD);
		if((i == 2 || i == 48) && j >= 10 && j <= 28) strcpy(tela[j][i] , LR);
		//Campo de sorteio 
		if((j == 1 || j == 9) && i >= 50 && i <= 73) strcpy(tela[j][i] , UD);
		if((i == 50 || i == 73) && j >= 1 && j <= 9) strcpy(tela[j][i] , LR);
		//Campo de bolas sorteadas
		if((j == 10 || j == 28) && i >= 50 && i <= 73) strcpy(tela[j][i] , UD);
		if((i == 50 || i == 73) && j >= 10 && j <= 28) strcpy(tela[j][i] , LR);
		}
	}

	//Campo de titulo
	strcpy(tela[1][2] , UL);
	strcpy(tela[1][48] , UR);
	strcpy(tela[9][2] , DL);
	strcpy(tela[9][48] , DR);

	//Campo de cartelas
	strcpy(tela[10][2] , UL);
	strcpy(tela[10][48] , UR);
	strcpy(tela[28][2] , DL);
	strcpy(tela[28][48] , DR);

	//Campo de sorteio
	strcpy(tela[1][50] , UL);
	strcpy(tela[1][73] , UR);
	strcpy(tela[9][50] , DL);
	strcpy(tela[9][73] , DR);

	//Campo de bolas sorteadas
	strcpy(tela[10][50] , UL);
	strcpy(tela[10][73] , UR);
	strcpy(tela[28][50] , DL);
	strcpy(tela[28][73] , DR);
}

//Desenhar informações
void desenharTitulo(){
	int i;
	int l0, l1, l2, l3, l4, l5, l6;
	int tCorX = 4, tCorY = 2;
	int lSize = 43;

	if(ETAPA == 1){
		lSize = 31;
		tCorX = 2+CPM_X;
		tCorY = 2+CPM_Y;
	}

	for(i = 0; i < lSize; i++){
		sprintf(tituloLinhas[0][i], "%s", " ");
		sprintf(tituloLinhas[1][i], "%s", " ");
		sprintf(tituloLinhas[2][i], "%s", " ");
		sprintf(tituloLinhas[3][i], "%s", " ");
		sprintf(tituloLinhas[4][i], "%s", " ");
		sprintf(tituloLinhas[5][i], "%s", " ");
		sprintf(tituloLinhas[6][i], "%s", " ");
	}

	char txt5[] = "Rodadas restantes: ";
	char txt6[] = "Quantidade de acertos: ";

	for(l5 = 0; l5 < min(strlen(txt5),lSize); l5++){
		sprintf(tituloLinhas[5][l5+11], "%c", txt5[l5]);
	}
	sprintf(tituloLinhas[5][l5+11], "%d", (int)(rodadas / 10));
	sprintf(tituloLinhas[5][l5+12], "%d", (int)(rodadas % 10));

	for(l6 = 0; l6 < min(strlen(txt6),lSize); l6++){
		sprintf(tituloLinhas[6][l6+9], "%c", txt6[l6]);
	}
	sprintf(tituloLinhas[6][l6+9], "%d", (int)(acertos / 10));
	sprintf(tituloLinhas[6][l6+10], "%d", (int)(acertos % 10));

	//Isso é o que faz o nome "BINGO" ficar piscando

	char bl0[27][4], bl1[27][4], bl2[27][4], bl3[27][4], bl4[27][4];

	if(tStep == 5) tStepDir = -1;
	if(tStep == 0) tStepDir = 1;
	tStep += tStepDir;

	//barra[tStep==0?1:tStep==5?4:tStep];
	/*
		int x;
		if(tStep==0){
			x = 1;
		}else if(tStep==5){
			x = 4;
		}else{
			x = tStep;
		}
		bar[x];
	*/
	sprintf(bl0[0]  , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl0[1]  , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl0[2]  , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl0[3]  , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl0[4]  , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl0[5]  , " ");
	sprintf(bl0[6]  , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl0[7]  , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl0[8]  , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl0[9]  , " ");
	sprintf(bl0[10] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl0[11] , " ");
	sprintf(bl0[12] , " ");
	sprintf(bl0[13] , " ");
	sprintf(bl0[14] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl0[15] , " ");
	sprintf(bl0[16] , " ");
	sprintf(bl0[17] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl0[18] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl0[19] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl0[20] , " ");
	sprintf(bl0[21] , " ");
	sprintf(bl0[22] , " ");
	sprintf(bl0[23] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl0[24] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl0[25] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl0[26] , " ");

	sprintf(bl1[0]  , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl1[1]  , " ");
	sprintf(bl1[2]  , " ");
	sprintf(bl1[3]  , " ");
	sprintf(bl1[4]  , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl1[5]  , " ");
	sprintf(bl1[6]  , " ");
	sprintf(bl1[7]  , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl1[8]  , " ");
	sprintf(bl1[9]  , " ");
	sprintf(bl1[10] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl1[11] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl1[12] , " ");
	sprintf(bl1[13] , " ");
	sprintf(bl1[14] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl1[15] , " ");
	sprintf(bl1[16] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl1[17] , " ");
	sprintf(bl1[18] , " ");
	sprintf(bl1[19] , " ");
	sprintf(bl1[20] , " ");
	sprintf(bl1[21] , " ");
	sprintf(bl1[22] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl1[23] , " ");
	sprintf(bl1[24] , " ");
	sprintf(bl1[25] , " ");
	sprintf(bl1[26] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);

	sprintf(bl2[0]  , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl2[1]  , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl2[2]  , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl2[3]  , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl2[4]  , " ");
	sprintf(bl2[5]  , " ");
	sprintf(bl2[6]  , " ");
	sprintf(bl2[7]  , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl2[8]  , " ");
	sprintf(bl2[9]  , " ");
	sprintf(bl2[10] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl2[11] , " ");
	sprintf(bl2[12] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl2[13] , " ");
	sprintf(bl2[14] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl2[15] , " ");
	sprintf(bl2[16] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl2[17] , " ");
	sprintf(bl2[18] , " ");
	sprintf(bl2[19] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl2[20] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl2[21] , " ");
	sprintf(bl2[22] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl2[23] , " ");
	sprintf(bl2[24] , " ");
	sprintf(bl2[25] , " ");
	sprintf(bl2[26] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);

	sprintf(bl3[0]  , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl3[1]  , " ");
	sprintf(bl3[2]  , " ");
	sprintf(bl3[3]  , " ");
	sprintf(bl3[4]  , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl3[5]  , " ");
	sprintf(bl3[6]  , " ");
	sprintf(bl3[7]  , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl3[8]  , " ");
	sprintf(bl3[9]  , " ");
	sprintf(bl3[10] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl3[11] , " ");
	sprintf(bl3[12] , " ");
	sprintf(bl3[13] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl3[14] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl3[15] , " ");
	sprintf(bl3[16] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl3[17] , " ");
	sprintf(bl3[18] , " ");
	sprintf(bl3[19] , " ");
	sprintf(bl3[20] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl3[21] , " ");
	sprintf(bl3[22] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl3[23] , " ");
	sprintf(bl3[24] , " ");
	sprintf(bl3[25] , " ");
	sprintf(bl3[26] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);

	sprintf(bl4[0]  , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl4[1]  , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl4[2]  , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl4[3]  , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl4[4]  , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl4[5]  , " ");
	sprintf(bl4[6]  , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl4[7]  , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl4[8]  , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl4[9]  , " ");
	sprintf(bl4[10] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl4[11] , " ");
	sprintf(bl4[12] , " ");
	sprintf(bl4[13] , " ");
	sprintf(bl4[14] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl4[15] , " ");
	sprintf(bl4[16] , " ");
	sprintf(bl4[17] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl4[18] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl4[19] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl4[20] , " ");
	sprintf(bl4[21] , " ");
	sprintf(bl4[22] , " ");
	sprintf(bl4[23] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl4[24] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl4[25] , "%s", barra[tStep==0?1:tStep==5?4:tStep]);
	sprintf(bl4[26] , " ");

	for(i = 0; i < 27; i++){
		sprintf(tituloLinhas[0][i], "%s", bl0[i]);
		sprintf(tituloLinhas[1][i], "%s", bl1[i]);
		sprintf(tituloLinhas[2][i], "%s", bl2[i]);
		sprintf(tituloLinhas[3][i], "%s", bl3[i]);
		sprintf(tituloLinhas[4][i], "%s", bl4[i]);
	}

	for(i = 0; i < lSize; i++){
		if(strcmp(tituloLinhas[0][i]," ")) strcpy(tela[tCorY][i+tCorX+((lSize-27)/2)] , tituloLinhas[0][i]);
		if(strcmp(tituloLinhas[1][i]," ")) strcpy(tela[tCorY+1][i+tCorX+((lSize-27)/2)] , tituloLinhas[1][i]);
		if(strcmp(tituloLinhas[2][i]," ")) strcpy(tela[tCorY+2][i+tCorX+((lSize-27)/2)] , tituloLinhas[2][i]);
		if(strcmp(tituloLinhas[3][i]," ")) strcpy(tela[tCorY+3][i+tCorX+((lSize-27)/2)] , tituloLinhas[3][i]);
		if(strcmp(tituloLinhas[4][i]," ")) strcpy(tela[tCorY+4][i+tCorX+((lSize-27)/2)] , tituloLinhas[4][i]);
		if(ETAPA == 2){
			if(strcmp(tituloLinhas[5][i]," ")) strcpy(tela[tCorY+5][i+tCorX] , tituloLinhas[5][i]);
			if(strcmp(tituloLinhas[6][i]," ")) strcpy(tela[tCorY+6][i+tCorX] , tituloLinhas[6][i]);
		}
	}
}

//Desenhar campo de bolas sorteadas
void desenharCampoSorteados(){
	int i, j;
	//Texto inicial
	for(i = 0; i < strlen(txtSorteadas); i++){
		sprintf(tempTxtSorteadas[i], "%c", txtSorteadas[i]);
		strcpy(tela[CPA_Y-2][i+1+CPA_X] , tempTxtSorteadas[i]);
	}
  
	//Área de desenho
	for(j = 0; j < 15; j++) for(i = 0; i < 20; i++){
		//Definir espaços
		strcpy(tela[j+CPA_Y][i+CPA_X] , "-");
		if((i+1)%3==0){
			strcpy(tela[j+CPA_Y][i+CPA_X] , " ");
		}else{
			if((j*i)+i<294){
				//Desenhar cada número em sua respectiva posição se ex.: v[x] = x
				if((numerosSorteadosDesenho[(i/3 + (21*j)/3)] - 1) / 10 == (i/3 + (21*j)/3) / 10 &&
				(numerosSorteadosDesenho[(i/3 + (21*j)/3)] - 1) % 10 == (i/3 + (21*j)/3) % 10){
					//Definir dezenas
					sprintf(tempSorteadosD1[i/3 + (21*j)/3], "%d", numerosSorteadosDesenho[i/3 + (21*j)/3] / 10);
					//Definir unidades
					sprintf(tempSorteadosD2[i/3 + (21*j)/3], "%d", numerosSorteadosDesenho[i/3 + (21*j)/3] % 10);
					//Adicionar Dezenas
					if((i+3)%3==0) strcpy(tela[j+CPA_Y][i+CPA_X] , tempSorteadosD1[i/3 + (21*j)/3]);
					//Adicionar unidades;
					if((i+2)%3==0) strcpy(tela[j+CPA_Y][i+CPA_X] , tempSorteadosD2[i/3 + (21*j)/3]);
				}
			}
		}
	}
	//Personalizar ultima linha, pois ela deve ter o número centralizado
	strcpy(tela[14+CPA_Y][9+CPA_X] , tela[14+CPA_Y][0+CPA_X]);
	strcpy(tela[14+CPA_Y][10+CPA_X] , tela[14+CPA_Y][1+CPA_X]);
	strcpy(tela[14+CPA_Y][0+CPA_X] , " ");
	strcpy(tela[14+CPA_Y][1+CPA_X] , " ");
	strcpy(tela[14+CPA_Y][3+CPA_X] , " ");
	strcpy(tela[14+CPA_Y][4+CPA_X] , " ");
	strcpy(tela[14+CPA_Y][6+CPA_X] , " ");
	strcpy(tela[14+CPA_Y][7+CPA_X] , " ");
	strcpy(tela[14+CPA_Y][12+CPA_X] , " ");
	strcpy(tela[14+CPA_Y][13+CPA_X] , " ");
	strcpy(tela[14+CPA_Y][15+CPA_X] , " ");
	strcpy(tela[14+CPA_Y][16+CPA_X] , " ");
	strcpy(tela[14+CPA_Y][18+CPA_X] , " ");
	strcpy(tela[14+CPA_Y][19+CPA_X] , " ");
}

//Gerar número personalizado - Essa versão da função foi fixada para apenas 2 algarismos
void intPers(int num){
	int i, j, k;

	//Quantidade de digitos do número escolhido
	digitos = 2;
	//Vetor para os digitos do número escolhido
	int newInt[digitos];
	//Contagem de digitos
	int count = digitos;
	int numCheck, newDigit;

	//Redefinir
	for(i = 0; i < digitos; i++){
		newInt[i] = -1;
	}

	//Separar algarismos
	while(count > 0){
		numCheck = 0;
		newDigit = 0;
		for(i = 0; i < digitos - count; i++)	numCheck += newInt[i] * pow(10,digitos-i-1); 
		newDigit = (num - numCheck) / (int)pow(10,count-1);
		newInt[digitos-count] = newDigit;
		count--;
	}

	//Adicionar algarismos
	for(k = 0; k < digitos; k++) for(j = 0; j < 3; j++) for(i = 0; i < 3; i ++){
		switch(newInt[k]){
		case 0:
			strcpy(numPers[j][i + (3*k)] , algPers[0][j][i]);
			break;
		case 1:
			strcpy(numPers[j][i + (3*k)] , algPers[1][j][i]);
			break;
		case 2:
			strcpy(numPers[j][i + (3*k)] , algPers[2][j][i]);
			break;
		case 3:
			strcpy(numPers[j][i + (3*k)] , algPers[3][j][i]);
			break;
		case 4:
			strcpy(numPers[j][i + (3*k)] , algPers[4][j][i]);
			break;
		case 5:
			strcpy(numPers[j][i + (3*k)] , algPers[5][j][i]);
			break;
		case 6:
			strcpy(numPers[j][i + (3*k)] , algPers[6][j][i]);
			break;
		case 7:
			strcpy(numPers[j][i + (3*k)] , algPers[7][j][i]);
			break;
		case 8:
			strcpy(numPers[j][i + (3*k)] , algPers[8][j][i]);
			break;
		case 9:
			strcpy(numPers[j][i + (3*k)] , algPers[9][j][i]);
			break;
		}
	}

  	//Desenhar borda
	for(j = 0; j < 5; j++) for(i = 0; i < 3 * digitos + 4 + digitos; i++){
		strcpy(tela[j + CPS_Y][i + CPS_X] , " ");
		if(j == 0 || j == 4) strcpy(tela[j + CPS_Y][i + CPS_X] , UD);
		if(i == 0 || i == 3 * digitos + 3 + digitos) strcpy(tela[j + CPS_Y][i + CPS_X] , LR);
	}
	strcpy(tela[CPS_Y][CPS_X] , UL);
	strcpy(tela[CPS_Y][3 * digitos + 3 + digitos + CPS_X] , UR);
	strcpy(tela[CPS_Y + 4][CPS_X] , DL);
	strcpy(tela[CPS_Y + 4][3 * digitos + 3 + digitos + CPS_X] , DR);

  	//Desenhar número
	int dCount = 0;
	for(j = 0; j < 3; j++){
		for(i = 0; i < 3 * digitos; i++){
			strcpy(tela[j + CPS_Y + 1][i + 2 + CPS_X + (dCount*2)] , numPers[j][i]);
			//Espaçamento
			if((i+1)%3==0) dCount++;
		}
		dCount = 0;
	}
}

//Desenhar área das cartelas na exibição individual
void desenharCampoCartelas(JOGADORES *jogadorAtual){
	int i, j;

	//Nome do jogador (Max 32 char)
	for(i = 0; i < (int)min(strlen(jogadorAtual->jogador.nome),32); i++){
		snprintf(jogadorAtual->jogador.tempNome[i], 32, "%c", jogadorAtual->jogador.nome[i]);
		strcpy(tela[CPC_Y-1][i+CPC_X + ((45-(int)min(strlen(jogadorAtual->jogador.nome),32))/2) - 1] , jogadorAtual->jogador.tempNome[i]);
	}
	//Bordas
	for(i = 0; i < 47; i++){
		strcpy(tela[CPC_Y][CPC_X+i-2] , UD);
		strcpy(tela[CPC_Y+14][CPC_X+i-2] , UD);
	}
	strcpy(tela[CPC_Y][CPC_X-2] , LC);
	strcpy(tela[CPC_Y][CPC_X+44] , RC);
	strcpy(tela[CPC_Y+14][CPC_X-2] , LC);
	strcpy(tela[CPC_Y+14][CPC_X+44] , RC);

	for(j = 0; j < 13; j++) strcpy(tela[CPC_Y+j+1][CPC_X+33] , LR);

	strcpy(tela[CPC_Y][CPC_X+33] , UC);
	strcpy(tela[CPC_Y+14][CPC_X+33] , DC);

	//Jogador atual
	for(i = 0; i < strlen(txtJogadores01); i++){
		sprintf(tempTxtJogadores01[i], "%c", txtJogadores01[i]);
		strcpy(tela[CPC_Y+4][CPC_X+35+i] , tempTxtJogadores01[i]);
	}
	//Total de jogadores
	for(i = 0; i < strlen(txtJogadores02); i++){
		sprintf(tempTxtJogadores02[i], "%c", txtJogadores02[i]);
		strcpy(tela[CPC_Y+8][CPC_X+36+i] , tempTxtJogadores02[i]);
	}
	
	//Números Jogador atual / Total de jogadores
	sprintf(dgt[0],"%d",posicao(jogadores, jogadorAtual) / 10);
	sprintf(dgt[1],"%d",posicao(jogadores, jogadorAtual) % 10);
	sprintf(dgt[2],"%d",nJ / 10);
	sprintf(dgt[3],"%d",nJ % 10);
	strcpy(tela[CPC_Y+6][CPC_X+38] , dgt[0]);
	strcpy(tela[CPC_Y+6][CPC_X+38+1] , dgt[1]);
	strcpy(tela[CPC_Y+10][CPC_X+38] , dgt[2]);
	strcpy(tela[CPC_Y+10][CPC_X+38+1] , dgt[3]);

	//Desenhar cartela individual
	desenharCartelas(jogadorAtual, CPC_X, CPC_Y+1);

	//Texto proximo jogador
	for(i = 0; i < strlen(txtProximo); i++){
		sprintf(tempTxtProximo[i], "%c", txtProximo[i]);
		strcpy(tela[CPC_Y+15][CPC_X+i+32] , tempTxtProximo[i]);
	}
	//Texto jogador anterior
	for(i = 0; i < strlen(txtAnterior); i++){
		sprintf(tempTxtAnterior[i], "%c", txtAnterior[i]);
		strcpy(tela[CPC_Y+15][CPC_X+i+6] , tempTxtAnterior[i]);
	}
	//Setas de seleção
	strcpy(tela[CPC_Y+15][CPC_X+3] , "\u25C4");
	strcpy(tela[CPC_Y+15][CPC_X+38] , "\u25BA");
}

//Identificar as cartelas a serem adicionadas à tela de exibição multiplas
void desenharMultiplasCartelas(JOGADORES *jogadorAtual, int sizeX, int sizeY, int corX, int corY, int interX, int interY){
	int i, j, k, kMax = 0;
	JOGADORES *manipulador = jogadorAtual;

	//Limpar tela
	limparTelaPrincipal();

	//Preparar a quantidade de campos
	int nCampX = manipulador->prox==jogadores?1:2;
	int nCampY = manipulador->prox==jogadores->ante?1:2;

	//Selecionar cartelas
	for(j = 0; j < nCampY; j++) for(i = 0; i < nCampX; i++){
		if(manipulador != jogadores || i+j==0){
			//Texto de exibição
			if(manipulador->jogador.vencedor > 0){
				//Nome do jogador + informativo de vitória
				kMax = (int)min(strlen(manipulador->jogador.nome)+strlen(txtVitoria),32);
			}else{
				//Nome do jogador
				kMax = (int)min(strlen(manipulador->jogador.nome),32);
			}
			for(k = 0; k < kMax; k++){
				char tempLetter = manipulador->jogador.vencedor>0?k<kMax-strlen(txtVitoria)?manipulador->jogador.nome[k]:txtVitoria[k-(kMax-strlen(txtVitoria))]:manipulador->jogador.nome[k];
				/*
					char tempLetter;
					if(manipulador->jogador.vencedor != -1){
						if(k < kMax-strlen(txtVitoria)){
							tempLetter = manipulador->jogador.nome[k];
						}else{
							tempLetter = txtVitoria[k-(kMax-strlen(txtVitoria))];
						}
					}else{
						tempLetter = manipulador->jogador.nome[k];
					}
				*/

				//Adicionar os primeiros 32 caracteres do texto de exibição ao vetor temporário
				snprintf(manipulador->jogador.tempNome[k], 32, "%c", tempLetter);
				//Adicionar o texto do vetor temporário à tela principal
				strcpy(tela[(corY + (interY * j))][k+(corX + (interX * i)) + ((32-kMax)/2)] , manipulador->jogador.tempNome[k]);
			}
			//Desenhar a cartela selecionada
			desenharCartelas(manipulador, corX + (interX * i), corY + (interY * j));
		}
		manipulador = manipulador->prox;
	}

	//Print de bordas;
	for(j = 0; j < telaLin; j++) for(i = 0; i < telaCol; i++){
		if(i>0 && i < 75 && (j==27 || j==14 || j==13)) strcpy(tela[j][i] , UD);
		if((i==37 || i==38) && j > 0 && j < 27) strcpy(tela[j][i] , LR);
	}
	strcpy(tela[27][0] , LC);
	strcpy(tela[27][75] , RC);
	strcpy(tela[14][0] , LC);
	strcpy(tela[14][75] , RC);
	strcpy(tela[13][0] , LC);
	strcpy(tela[13][75] , RC);
	strcpy(tela[0][37] , UC);
	strcpy(tela[27][37] , DC);
	strcpy(tela[0][38] , UC);
	strcpy(tela[27][38] , DC);
	strcpy(tela[14][37] , CT);
	strcpy(tela[14][38] , CT);
	strcpy(tela[13][37] , CT);
	strcpy(tela[13][38] , CT);

	//Texto proximo jogador
	for(i = 0; i < strlen(txtProximo); i++){
		sprintf(tempTxtProximo[i], "%c", txtProximo[i]);
		strcpy(tela[28][i+60] , tempTxtProximo[i]);
	}
	//Texto jogador anterior
	for(i = 0; i < strlen(txtAnterior); i++){
		sprintf(tempTxtAnterior[i], "%c", txtAnterior[i]);
		strcpy(tela[28][i+11] , tempTxtAnterior[i]);
	}
	//Setas de seleção
	strcpy(tela[28][8] , "\u25C4");
	strcpy(tela[28][66] , "\u25BA");

}

//Adicionar a cartela informada na tela
void desenharCartelas(JOGADORES *jogadorAtual, int corX, int corY){
	int i, j;
	JOGADORES *manipulador = jogadorAtual;
	//Mais bordas
	for(j = 0; j < 9; j++) for(i = 0; i < 26; i++){
		if((j==0||j==2||j==4||j==6||j==8) && i >= 0 && i < 26) strcpy(tela[j+corY+3][i+corX+5] , UD);

		if((i==0||i==5||i==10||i==15||i==20||i==25) && j >= 0 && j < 9) strcpy(tela[j+corY+3][i+corX+5] , LR);

		if((j==2||j==4||j==6) && i==0) strcpy(tela[j+corY+3][i+corX+5] , LC);
		if((j==2||j==4||j==6) && i==25) strcpy(tela[j+corY+3][i+corX+5] , RC);

		if((i==5||i==10||i==15||i==20) && j == 0) strcpy(tela[j+corY+3][i+corX+5] , UC);
		if((i==5||i==10||i==15||i==20) && j == 8) strcpy(tela[j+corY+3][i+corX+5] , DC);

		if((i==5||i==10||i==15||i==20) && (j==2||j==4||j==6)) strcpy(tela[j+corY+3][i+corX+5] , CT);
	}
	//Identificação de Linhas
	for(i = 0; i < 4; i++){
		sprintf(tempLinID[i], "%d", i+1);
		strcpy(tela[corY+4 + (2*i)][corX+2] , ((int)(manipulador->jogador.vencedor/10))==(i+1)&&manipulador->jogador.vencedor>0?"\u25BA":AR);
		strcpy(tela[corY+4 + (2*i)][corX+1] , tempLinID[i]);
	}
	//Identificação de Colunas
	for(i = 0; i < 5; i++){
		sprintf(tempColID[i], "%c", (char)65+i);
		strcpy(tela[corY+2][corX+7 + (5*i)] , ((int)(manipulador->jogador.vencedor%10))==(i+1)&&manipulador->jogador.vencedor>0?"\u25BC":AD);
		strcpy(tela[corY+1][corX+7 + (5*i)] , tempColID[i]);
	}
	//Desenhar cartela
	for(j = 0; j < 4; j++) for(i = 0; i < 5; i++){
		//Verificar digito coringa
		if(manipulador->jogador.marcacao[j][i] != 2){
			sprintf(manipulador->jogador.tempCartelaD1[i + (j*5)],"%d",manipulador->jogador.cartela[j][i] / 10);
			sprintf(manipulador->jogador.tempCartelaD2[i + (j*5)],"%d",manipulador->jogador.cartela[j][i] % 10);
		}else{
			sprintf(manipulador->jogador.tempCartelaD1[i + (j*5)],"%s",barra[4]); //Originalmente EP
			sprintf(manipulador->jogador.tempCartelaD2[i + (j*5)],"%s",barra[4]); //Originalmente " "
		}
		//Adicionar Dezenas e Unidades
		strcpy(tela[corY+4+(j*2)][corX+7+(i*5)] , manipulador->jogador.tempCartelaD1[i + (j*5)]);
		strcpy(tela[corY+4+(j*2)][corX+8+(i*5)] , manipulador->jogador.tempCartelaD2[i + (j*5)]);
		//Destacar acertos
		if(manipulador->jogador.marcacao[j][i] != 0 && manipulador->jogador.cartela[j][i] != 0){
			if(!strcmp(tela[corY+4+(j*2)][corX+5+(i*5)],RC)){
				strcpy(tela[corY+4+(j*2)][corX+5+(i*5)] , CT);
			}else{
				strcpy(tela[corY+4+(j*2)][corX+5+(i*5)] , LC);
			}
			strcpy(tela[corY+4+(j*2)][corX+6+(i*5)] , UD);
			strcpy(tela[corY+4+(j*2)][corX+9+(i*5)] , UD);
			strcpy(tela[corY+4+(j*2)][corX+10+(i*5)] , RC);
		}
	}
	//Bordas...
	strcpy(tela[corY+3][corX+5] , UL);
	strcpy(tela[corY+3][25+corX+5] , UR);
	strcpy(tela[8+corY+3][corX+5] , DL);
	strcpy(tela[8+corY+3][25+corX+5] , DR);
}

//Tela de configuração de velocidade
void configurarDelay(){
	int i, j, k = 10, l = 0;
	CONF = (CONF+1)%10;
	if(CONF > 4) l = 1;
	for(j = 0; j < telaLin; j++) for( i = 0; i < telaCol; i++){
		strcpy(tela[j][i] , " ");
		if((l+i+(j%2))%2==0) strcpy(tela[j][i] , barra[4]);
	}
}

//Creditos iniciais
void creditos(){
	int i;
	char msgInicial[] = "Jogo feito por:";
	char ysaac[] = "Ysaac Serafim Di Florentino";
	char halan[] = "Halan Gomes de Melo";
	char txtHelp[] = "'Shift + H' para tela de ajuda";
	char msgFinal[] = "Aproveite o jogo!";
	int sZ = max(strlen(ysaac),max(strlen(halan),strlen(msgInicial)));
	sZ+=2;

	system(limpar);
	for(i = 0; i < (telaLin-9)/2; i++)	printf("\n");

	for(i = 0; i < (telaCol-sZ)/2 - 1; i++)	printf(" ");
	printf("%s",UL);
	for(i = 0; i < sZ; i++)	printf("%s",UD);
	printf("%s\n",UR);

	for(i = 0; i < (telaCol-sZ)/2 - 1; i++)	printf(" ");
	printf("%s",LR);
	for(i = 0; i < sZ; i++)	printf(" ");
	printf("%s\n",LR);

	for(i = 0; i < (telaCol-sZ)/2 - 1; i++)	printf(" ");
	printf("%s",LR);
	for(i = 0; i < (sZ-strlen(msgInicial))/2; i++) printf(" ");
	printf("%s",msgInicial);
	for(i = 0; i < (sZ-strlen(msgInicial))/2; i++) printf(" ");
	printf("%s\n",LR);

	for(i = 0; i < (telaCol-sZ)/2 - 1; i++)	printf(" ");
	printf("%s",LR);
	for(i = 0; i < sZ; i++)	printf(" ");
	printf("%s\n",LR);

	for(i = 0; i < (telaCol-sZ)/2 - 1; i++)	printf(" ");
	printf("%s",LR);
	for(i = 0; i < (sZ-strlen(ysaac))/2; i++) printf(" ");
	printf("%s",ysaac);
	for(i = 0; i < (sZ-strlen(ysaac))/2; i++) printf(" ");
	printf("%s\n",LR);

	for(i = 0; i < (telaCol-sZ)/2 - 1; i++)	printf(" ");
	printf("%s",LR);
	for(i = 0; i < sZ/2; i++) printf(" ");
	printf("e");
	for(i = 0; i < sZ/2; i++) printf(" ");
	printf("%s\n",LR);

	for(i = 0; i < (telaCol-sZ)/2 - 1; i++)	printf(" ");
	printf("%s",LR);
	for(i = 0; i < (sZ-strlen(halan))/2; i++) printf(" ");
	printf("%s",halan);
	for(i = 0; i < (sZ-strlen(halan))/2; i++) printf(" ");
	printf("%s\n",LR);

	for(i = 0; i < (telaCol-sZ)/2 - 1; i++)	printf(" ");
	printf("%s",LR);
	for(i = 0; i < sZ; i++)	printf(" ");
	printf("%s\n",LR);

	for(i = 0; i < (telaCol-sZ)/2 - 1; i++)	printf(" ");
	printf("%s",DL);
	for(i = 0; i < sZ; i++)	printf("%s",UD);
	printf("%s\n",DR);

	usleep(1000000);
	system(limpar);

	for(i = 0; i < (telaLin-3)/2; i++)	printf("\n");

	for(i = 0; i < (telaCol-(strlen(txtHelp)+2))/2 - 1; i++) printf(" ");
	printf("%s",UL);
	for(i = 0; i < strlen(txtHelp)+2; i++)	printf("%s",UD);
	printf("%s\n",UR);

	for(i = 0; i < (telaCol-(strlen(txtHelp)+2))/2 - 1; i++) printf(" ");
	printf("%s %s %s\n",LR,txtHelp,LR);

	for(i = 0; i < (telaCol-(strlen(txtHelp)+2))/2 - 1; i++) printf(" ");
	printf("%s",DL);
	for(i = 0; i < strlen(txtHelp)+2; i++)	printf("%s",UD);
	printf("%s\n",DR);
	
	usleep(1000000);
	system(limpar);

	for(i = 0; i < (telaLin-3)/2; i++)	printf("\n");

	for(i = 0; i < (telaCol-(strlen(msgFinal)+2))/2 - 1; i++) printf(" ");
	printf("%s",UL);
	for(i = 0; i < strlen(msgFinal)+2; i++)	printf("%s",UD);
	printf("%s\n",UR);

	for(i = 0; i < (telaCol-(strlen(msgFinal)+2))/2 - 1; i++) printf(" ");
	printf("%s %s %s\n",LR,msgFinal,LR);

	for(i = 0; i < (telaCol-(strlen(msgFinal)+2))/2 - 1; i++) printf(" ");
	printf("%s",DL);
	for(i = 0; i < strlen(msgFinal)+2; i++)	printf("%s",UD);
	printf("%s\n",DR);
	
	usleep(1000000);
	ETAPA = 1;
	system(limpar);
}

//Informar que temos um vencedor
void informarVencedor(){
	int i;
	char txtInformar[] = "Bingo!! Temos um vencedor!!";

	system(limpar);
	for(i = 0; i < (telaLin-3)/2; i++)	printf("\n");

	for(i = 0; i < (telaCol-(strlen(txtInformar)+2))/2 - 1; i++) printf(" ");
	printf("%s",UL);
	for(i = 0; i < strlen(txtInformar)+2; i++)	printf("%s",UD);
	printf("%s\n",UR);

	for(i = 0; i < (telaCol-(strlen(txtInformar)+2))/2 - 1; i++) printf(" ");
	printf("%s %s %s\n",LR,txtInformar,LR);

	for(i = 0; i < (telaCol-(strlen(txtInformar)+2))/2 - 1; i++) printf(" ");
	printf("%s",DL);
	for(i = 0; i < strlen(txtInformar)+2; i++)	printf("%s",UD);
	printf("%s\n",DR);
	
	usleep(2500000);
	system(limpar);
}

  //////////////////////////////////////
 //         Funções da Lista         //
//////////////////////////////////////

//Retorna um nó nulo
JOGADORES* criar() {return NULL;}

//Retorna 1 se vazia, caso contrario retorna 0
int estaVazia(JOGADORES *listaJogadores) {return (listaJogadores == NULL);}

//Insere o jogador na ultima posição da lista
JOGADORES* inserir(JOGADORES *listaJogadores, JOGADOR jogador)
{
	JOGADORES *novo = (JOGADORES*) malloc(sizeof(JOGADORES));
	novo->jogador = jogador;
	
	//Caso não existam jogadores
	if(estaVazia(listaJogadores)){
		listaJogadores = novo;
		listaJogadores->prox = novo;
		listaJogadores->ante = novo;
	//Caso exista 1 jogador
	}else if(listaJogadores->ante == listaJogadores && listaJogadores->prox == listaJogadores){
		novo->ante = listaJogadores;
		novo->prox = listaJogadores;
		listaJogadores->ante = novo;
		listaJogadores->prox = novo;
	//Qualquer outro caso
	}else{
		novo->ante = listaJogadores->ante;
		novo->prox = listaJogadores;
		listaJogadores->ante->prox = novo;
		listaJogadores->ante = novo;
	}
	nJ++;
	jID++;
	return listaJogadores;
}

//Procura o jogador pelo ID, remove se encontrar e retorna uma lista atualizada
JOGADORES *remover(JOGADORES *listaJogadores, int id)
{
	JOGADORES *no = listaJogadores;
	
	do{
		if(no == NULL) break;
		no = no->prox;
	}while(no->jogador.id != id && no != listaJogadores);
	//Caso não exista
	if(no == listaJogadores && (no == NULL || no->jogador.id != id)){
		//printf("Jogador não encontrado!\n");
		return listaJogadores;
	}
	//Caso seja o único jogador	
	if(no->prox == no && no->ante == no){
		listaJogadores->prox = NULL;
		listaJogadores->ante = NULL;
		free(listaJogadores);
		listaJogadores = NULL;
	//Caso não seja o único jogador
	}else{
		no->ante->prox = no->prox;
		no->prox->ante = no->ante;
		//Caso seja o primeiro jogador
		if(no == listaJogadores) listaJogadores = listaJogadores->prox;
		free(no);
		no = NULL;
	}

	nJ--;
	return listaJogadores;
}

//Remove todas as referencias da lista e libera todos os nós
void esvaziar(JOGADORES *listaJogadores)
{
	if(estaVazia(listaJogadores)) return;
	//Caso exista mais de 1 jogador
	JOGADORES *no = listaJogadores->prox;
	if(no != no->prox && no != no->ante){
		do{
			if(no == NULL) break;
			JOGADORES *temp = no->prox;
			free(no);
			no = NULL;
			no = temp;
		}while(no != listaJogadores);
	}
	listaJogadores->prox = NULL;
	listaJogadores->ante = NULL;
	free(listaJogadores);
	listaJogadores = NULL;
	nJ=0;
}

//Retorna a posição do elemento em relação ao inicio da lista
int posicao(JOGADORES *listaJogadores, JOGADORES *jogadorAtual){
	int count = 1;
	if(listaJogadores == NULL) return 0;
	JOGADORES *manipulador = jogadorAtual;

	while(manipulador != listaJogadores){
		manipulador = manipulador->ante;
		count++;
	}
	return count;
}

  ////////////////////////////////////////
 //         Funções Auxiliares         //
////////////////////////////////////////

//Gerar digitos coringa
//void gerarCoringa(int cartela, int colunas, int linhas, int quantidade de coringas)
int **gerarCoringa(int **marcacao, int nC, int nL, int num){
	int i;
	//A quantidade de cogingas deve ser menor que o min(colunas,linhas) e maior que 0
	if(num > min(nC,nL) ||  num <= 0) return marcacao;

	//Vetor para identificação
	int cLin[nL], cCol[nC], gLin, gCol;
	for(i = 0; i < nL; i++)	cLin[i] = 0;
	for(i = 0; i < nC; i++)	cCol[i] = 0;

	//A condição do 'if' pode substituir a do 'while'
	while(num > 0){
		int breakX = 0, breakY = 0;
		do{
			gLin = rand() % (nL);
			breakY = 1;
			if(cLin[gLin]) breakY = 0;
		}while(breakY == 0);
		do{
			gCol = rand() % (nC);
			breakX = 1;
			if(cCol[gCol]) breakX = 0;
		}while(breakX == 0);
		cLin[gLin] = 1;
		cCol[gCol] = 1;
		marcacao[gLin][gCol] = 2;
		num--;
	}
	return marcacao;
}

//Verificar vencedor
void verificarVencedor(){
	int i, j, count = 0;
	JOGADORES *jogadorAtual = jogadores;	
	//Conferir cada jogador
	do{
		if(jogadorAtual == NULL) break;
		//Conferir linhas
		for(j=0; j<cartelaLin; j++){
			for(i=0; i<cartelaCol; i++){
				if(jogadorAtual->jogador.marcacao[j][i] != 0){
					count++;
				}else{
					count = 0;
					break;
				}
			}
			if(count==5){
				count = 0;

				jogadorAtual->jogador.vencedor += (j+1)*10;
				GANHOU = 1;
				break;
			}
		}
		//Conferir colunas
		for(i=0; i<cartelaCol; i++){
			for(j=0; j<cartelaLin; j++){            
				if(jogadorAtual->jogador.marcacao[j][i] != 0){
					count++;
				}else{
					count = 0;
					break;
				}
			}
			if(count==4){
				count = 0;
				jogadorAtual->jogador.vencedor += i+1;
				GANHOU = 1;
				break;
			}
		}
		jogadorAtual = jogadorAtual->prox;
	}while(jogadorAtual != jogadores);
}

//Verificar se o valor informado é único no conjunto
//Se sim retorne 1 do contrário retorne 0
int unicoCartela(int **cartela, int nC, int nL, int num){
  	int i, j;
    for (j = 0; j < nL; j++) for (i = 0; i < nC; i++) if (num == cartela[j][i]) return 0;
  	return 1;
}

//Obrigatoriedade 2.1
//Unico binário
int unicoSorteados(int *vetSort, int tam, int num){
	int inicio = 0, fim = tam-1;
	while(inicio <= fim){
		int meio = (inicio+fim)/2;
		if(num > vetSort[meio]){
			inicio = meio+1;
		}else{
			if(num < vetSort[meio]){
				fim = meio-1;
			}else{
				return 0;
			}
		}
	}
	return 1;
}

//Obrigatoriedade 3
//Insertion Sort para números sorteados
int *insertionSorteados(int *vetSort, int tam, int num){
	vetSort = (int *)realloc(vetSort,(tam+1)*sizeof(int));
	if(vetSort == NULL){
		printf("'insertionSorteados' gerou um erro crítico %d",__LINE__);
		CRITICO = 1;
		getchar();
	}
	int i, j;
	vetSort[tam] = num;
	for(i = tam; i > 0 && num < vetSort[i-1]; i--) vetSort[i] = vetSort[i-1];
	vetSort[i] = num;
	
	return vetSort;
}

//Ordenar cartela
int **ordenarCartela(int **cartela, int nC, int nL){
	//Bubble Sort no vetor
	/*int tempV[nC*nL], i, j;
	for(j = 0; j < nL; j++) for(i = 0; i < nC; i++) tempV[i + nC*j] = cartela[j][i];
	for(i = 0; i < nC*nL; i++) for(j = i; j < nC*nL; j++){
		if(tempV[i] > tempV[j]){
			tempV[i] += tempV[j];
			tempV[j] = tempV[i] - tempV[j];
			tempV[i] -= tempV[j];
		}
	}
	for(j = 0; j < nL; j++) for(i = 0; i < nC; i++) cartela[j][i] = tempV[i + nC*j];*/

	//Bubble Sort na matriz sem variável auxiliar
	//'i' varia de 0 a nC*nL - i / nC == nL atual - i % nC == nC atual
	int i, j;
	for(i = 0; i < nC*nL; i++) for(j = i; j < nC*nL; j++){
		if(cartela[(int)i/nC][(int)i%nC] > cartela[(int)j/nC][(int)j%nC]){
			cartela[(int)i/nC][(int)i%nC] += cartela[(int)j/nC][(int)j%nC];
			cartela[(int)j/nC][(int)j%nC] = cartela[(int)i/nC][(int)i%nC] - cartela[(int)j/nC][(int)j%nC];
			cartela[(int)i/nC][(int)i%nC] -= cartela[(int)j/nC][(int)j%nC];
		}
	}

	//Poderia ser um Insertion Sort
	return cartela;
}

//Obrigatoriedade 2.2
//Marcação Binária
void marcarAcertos(JOGADORES *jogadorAtual, int nC, int nL, int num){
	if(num <= 0) return;
	JOGADORES *manipulador = jogadorAtual;
	int inicio = 0, fim = (nC*nL)-1;
	while(inicio <= fim){
		int meio = (fim+inicio)/2;
		//'meio' é algum valor entre 0 e nC*nL - meio / nC == nL atual - meio % nC == nC atual
		if(num > manipulador->jogador.cartela[(int)meio/nC][(int)meio%nC]){
			inicio = meio+1;
		}else{
			if(num < manipulador->jogador.cartela[(int)meio/nC][(int)meio%nC]){
				fim = meio-1;
			}else{
				if(manipulador->jogador.marcacao[(int)meio/nC][(int)meio%nC] == 0){
					manipulador->jogador.marcacao[(int)meio/nC][(int)meio%nC] = 1;
					acertos++;
				}
				return;
			}
		}
	}
}

//Função máximo
float max(float n1, float n2){
  if(n1>n2) return n1;
  return n2;
}

//Função mínimo
float min(float n1, float n2){
  if(n1<n2) return n1;
  return n2;
}

//Capturador de caracteres
int capturar(){
	int i;
	setbuf(stdin,NULL);
	int comando = -1;
	//Se o identificador retornar 'True'
	if(identificar()){
		//Busque o caractere que foi retornado ao fluxo de entrada
		comando = getchar(); 
		//printf("%d\n",comando);
		//getchar();
	}
	return comando;
}
//Identificador de caracteres
int identificar(){

	//Struct auxiliar para termios
	struct termios oldTerminal, newTerminal;
	int ch;
	int oldFlags;
	
	//Salvar configurações do terminal
	tcgetattr(STDIN_FILENO, &oldTerminal);
	newTerminal = oldTerminal;

	//Modificar configurações do terminal
	//ICANON -> Alternar leitura canônica do terminal
	//ECHO -> Alternar exibição do terminal
	newTerminal.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newTerminal);
	oldFlags = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldFlags | O_NONBLOCK);
	
	//Armazenar caractere da entrada
	ch = getchar();
	
	//Resetar configurações do terminal
	tcsetattr(STDIN_FILENO, TCSANOW, &oldTerminal);
	fcntl(STDIN_FILENO, F_SETFL, oldFlags);
	
	//Se o caractere armazenado for diferente de 'Final do Arquivo'
	if(ch != EOF){
		//Recoloque-o no fluxo de entrada e Retorne 'True'
 		ungetc(ch, stdin);
		return 1;
	}
	//Caso contrario Retorne 'False'
	return 0;
}

/*
Lista de mudanças da versão RC7

	A ação de chamar uma proxima rodada foi passada para uma função independente

	Cálculo para identificar jogador refeito para evita uma inconsistância caso o jogador ganhasse completando a primeira linha

	Atalho para ajuda alterado de 'F1' para 'Shift + H' devido a uma incompatibilidade com o 'Debian 8'

	Removido o digito 0 do campo de números sorteados

	Função 'esvaziar' agora verifica corretamente se existe apenas 1 jogador

	Os valores de correção de posicionamento agora são constantes ex.: '#define CPC_X 4'

	Adicionado um informativo para quando houver um vencedor

	Ordem de desenho modificada para se adequar ao informativo (Agora primeiro a tela e desenhada e depois o comando é identificado)


Lista de mudanças da versão Final

	Função 'ordenarCartela' agora trabalha diretamente com a matriz
*/