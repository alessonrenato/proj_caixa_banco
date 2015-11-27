/*********************************************************************************************************************************************************
**********************************************************************************************************************************************************
**********************************************************************************************************************************************************
****** 												UNIVERSIDADE FEDERAL RURAL DE PERNAMBUCO														******
****** 											DEPARTAMENTO DE ESTATÍSTICA E INFORMÁTICA [DEINFO]													******
****** 									CURSO DE BACHARELADO EM CIÊNCIA DA COMPUTAÇÃO - TURMA BC3 2014.1											******
******																																				******
****** PROJETO DE INTRODUÇÃO A PROGRAMAÇÃO PARA 3VA	- CAIXA	BANCÁRIO DE ATENDIMENTO	(11ª versão)													******
****** ALUNO PROGRAMADOR: ALESSON RENATO LOPES VALENÇA																								******
****** PROFESSOR ORIENTADOR: ROBSON MEDEIROS																										******
**********************************************************************************************************************************************************
**********************************************************************************************************************************************************
**********************************************************************************************************************************************************/

//o programa a seguir em conjunto com o programa "totem" tem como objetivo implementar uum controle de caixa bancário e controle de filas integrado no mesmo sistema
//teste
//bibliotecas
#include <stdio.h>
#include <time.h>//manipula tempo do sistema
#include <stdlib.h>
#include <locale.h>//muda idioma do prompt para português


//estruturas
typedef struct {
	int dia;
	int mes;
	int ano;
} Data;//estrutura data

typedef struct {
	int ficha;
	float doc;
	int operacao;
}Dados;//estrutura de gravação de dados em arquivo

typedef struct {
	int pos;
	int sts;
}fila;//estrutura de controle de fila do caixa

typedef struct {
	char nome[20];
	int mat;
	int senha;
	int sts;
}CAD;//estrutura de cadastro de funcionarios
//**********************************************************************************************************************************************************
//****************************VARIÁVEIS GLOBAIS*************************************************************************************************************
//**********************************************************************************************************************************************************


//variaveis de estrutura
Data hoje, vencimento;//datas
Dados relatorio;//organizador de gravação no arquivo binario
fila chamada;//organizador da fila de espera do caixa
CAD gerente,funcionario;//controladores dos funcionários e gerentes

//guarda o numero de dias em cada mes para anos normais[0][x] e bissextos[1][x]
const int dias_mes[2][13] = {{0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
      		                 {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};
      		                 
//matriz-string usada para gravação da operação realizada no txt
const char operacoes[3][10] = {{"Pagamento"},
							   {"Depósito"},
							   {"Saque"}};

//matriz-string usada para status de funcionario
const char status[2][10] = {{"Inativo"},
							{"Ativo"}};
//ponteiros de arquivos
FILE *arquivot;//texto de movimentacao
FILE *prioritario;//bin da fila prioritaria
FILE *comum;//bin da fila comum
FILE *cadger;//bin do cadastro de gerentes
FILE *cadfun;//bin do cadastro de funcionarios

int senha=0,senhaini,opp=0;//senha de fila do caixa || senha inicial || qnt de operações
float em_caixa;//valor em dinheiro que esta no caixa
char tiposenha;//(P/C)

//menu de funções
int valorinteiro(char c);//subfunção de dataatual usada para transformar caracter numérico em valor inteiro
int bissexto (int ano);//verifica se o ano de [hoje] é bissexto
unsigned long dist_dias (Data inicio, Data fim);//calcula da diferença (hoje-vencimento) e retorna em nº de dias
float calc_juros(float doc);//calcula juros de um documento(percentual ou tarifa simples)
int seek_fila();//busca fichas em espera para atendimento
int gerencia();//rotina de controle e gerência do programa

//menu de procedimentos
void dataatual();//captura data atual do sistema e joga na estrutura [hoje]
void inicializacaixa();//Rotina para abrir o caixa [inserir dinheiro no caixa + senha inicial + data de hoje(automático)]
void cabecalho();//rotina para escrever o cabeçalho da tela do caixa
void nova_transacao();//incrementa senha ou chama outra operacao
void pagamento();//efetua operação de pagamento
void depositos();//efetua operação de deposito
void saques();//efetua operação de saque





//**********************************************************************************************************************************************************
//****************************ROTINA PRINCIPAL (MAIN)*******************************************************************************************************
//**********************************************************************************************************************************************************
int main(void) {
	setlocale(LC_ALL, "Portuguese");//muda o padrão de simbolos na tela para português
	
	gerencia();
	inicializacaixa();
	
seek_fila();

//menu principal do caixa em loop infinito
	for(;;){
			cabecalho();
			static char opcao;
			printf("Pressione a tecla para opção desejada:\n\nEFETUAR PAGAMENTO: (P)\nEFETUAR DEPÓSITO: (D)\nEFETUAR SAQUE: (S)\nFECHAR O CAIXA (F)\n");
			fflush(stdin);
			opcao=getch();
							
			if(opcao=='p'||opcao=='P'){
							  			pagamento();
							  			opp++;
			 				 			nova_transacao();
										}
								
			if(opcao=='d'||opcao=='D'){
							 			depositos();
							  			opp++;
										nova_transacao();
										}
			if(opcao=='s'||opcao=='S'){
							 			saques();
							 			opp++;
							  			nova_transacao();
										}
								

			if(opcao=='f'||opcao=='F')break;		
			
			

			

					}
//escreve rodapé do arquivo txt
	if((arquivot = fopen("REGISTRO_MOVIMENTACAO.txt","a")) == NULL)
      						{
        						printf("Erro ao abrir arquivo!!!\n\n");
        						exit(1);
      						}
				
fprintf(arquivot,"\t\t\tTOTAL EM CAIXA: RS %.2f\n\nFIM DO RELATORIO",em_caixa);

//fecha arquivos
fclose(arquivot);

//confirma em tela
printf("Arquivo de movimentação do caixa criado\n");
system("pause");
return 0;
}




//**********************************************************************************************************************************************************
//****************************FUNÇÃO DE BUSCA DA FILA*******************************************************************************************************
//**********************************************************************************************************************************************************

int seek_fila(){	static int vez=2;
					char opcao;
					
					for(;;){	//abre arquivo da fila prioritária
								if((prioritario = fopen("filaprio.bin","rb+")) == NULL){
								printf("Erro ao abrir arquivo!!!\n\n");
        						exit(1);
								}
					//chama 2 senhas prioritárias se existirem, senão passa para as senhas comuns
					
					if(vez!=0){//pula se 2 prioritárias em sequência foram chamadas
								
								while(!feof(prioritario)){//enquanto não atingir o fim do arquivo
								
															fread(&chamada,sizeof(chamada),1,prioritario);//lê arquivo da fila

															if(chamada.sts==0){//se o status da chamada for 0 (senha ainda não atendida)														
															vez--;				//2 chamadas prioritárias(se existir)/uma comum
															senha=chamada.pos;	//senha atual recebe valor da posição da fila
															tiposenha='P';		//senha do tipo prioritária
															chamada.sts=1;		//muda staus para atendido
															fseek(prioritario, ftell(prioritario) - sizeof(chamada),SEEK_SET);	//volta cursor para a senha que foi lida (posição atual do cursor - 1 posição do arquivo)
															fwrite(&chamada,sizeof(chamada),1,prioritario);//sobrescreve posição lida no arquivo mudando status para atendido
															fclose(prioritario);
															return 0;//fecha arquivo e encerra função
															}
														}
								}
					fclose(prioritario);//fecha caso fim do arquivo for atingido
					
					//abre arquivo da fila comum
					if((comum = fopen("filacom.bin","rb+")) == NULL){
								printf("Erro ao abrir arquivo!!!\n\n");
        						exit(1);
					
								}
					while(!feof(comum)){//enquanto nao atingir fim do arquivo
					
															fread(&chamada,sizeof(chamada),1,comum);//lê arquivo da fila

															if(chamada.sts==0){	//se status da chamada for 0(não atendida)
															vez=2;				//1 comum atendida, verifica-se novamente se existem prioritárias
															senha=chamada.pos;	//chamada recebe valor da fila
															chamada.sts=1;		//muda status para atendido
															tiposenha='C';		//senha do tipo comum
															fseek(comum, ftell(prioritario) - sizeof(chamada),SEEK_SET);//volta cursor para a senha que foi lida (posição atual do cursor - 1 posição do arquivo)
															fwrite(&chamada,sizeof(chamada),1,comum);//sobrescreve posição lida no arquivo mudando status para atendido
															fclose(comum);
															return 0;//fecha arquivo e encerra função
															}
										}	
															
										
					fclose(comum);//fecha caso fim do arquivo for atingido
					cabecalho();//escreve cabeçalho
					
					printf("\nCAIXA EM ESPERA...\nPRESSIONE QUALQUER TECLA PARA CHAMAR NOVA SENHA CASO EXISTA\n OU (S) PARA VOLTAR AO MENU");
					
					fflush(stdin);
					opcao=getch() ;
					if(opcao == 'S' || opcao == 's')return 0;
					
					}
}
				
//**********************************************************************************************************************************************************
//****************************ROTINA DA OPERAÇÃO SAQUE******************************************************************************************************
//**********************************************************************************************************************************************************
void saques(){	static float saque;
				cabecalho();
				printf("DIGITE O VALOR DO SAQUE: R$ ");
				scanf("%f",&saque);
				
				//reduz o valor total da operação ao montante em caixa
				em_caixa-=saque;
				
				//salva informações da operação na estrutura relatorio
				relatorio.doc=saque;
				relatorio.ficha=senha;
				relatorio.operacao=2;
				
				if((arquivot = fopen("REGISTRO_MOVIMENTACAO.txt","a")) == NULL)
      						{
        						printf("Erro ao abrir arquivo!!!\n\n");
        						exit(1);
      						}
				fprintf(arquivot,"\n\nFicha: %c%d\nOperacao: %s\nValor: RS %.2f\n\n\n",tiposenha,relatorio.ficha,operacoes[relatorio.operacao],relatorio.doc);
				fclose(arquivot);
}

//**********************************************************************************************************************************************************
//****************************ROTINA DA OPERAÇÃO DEPOSITO***************************************************************************************************
//**********************************************************************************************************************************************************
void depositos(){	static float deposito;
					cabecalho();
					
					//solicita o valor
					printf("DIGITE O VALOR DO DEPÓSITO: R$ ");
					scanf("%f",&deposito);
					
					//adiciona o valor total da operação ao montante em caixa
					em_caixa+=deposito;
					
					//salva informações da operação na estrutura relatorio
					relatorio.doc=deposito;
					relatorio.ficha=senha;
					relatorio.operacao=1;
					
					if((arquivot = fopen("REGISTRO_MOVIMENTACAO.txt","a")) == NULL)
      						{
        						printf("Erro ao abrir arquivo!!!\n\n");
        						exit(1);
      						}
					fprintf(arquivot,"Ficha: %c%d\nOperacao: %s\nValor: RS %.2f\n\n\n",tiposenha,relatorio.ficha,operacoes[relatorio.operacao],relatorio.doc);
					fclose(arquivot);
					
}

//**********************************************************************************************************************************************************
//****************************ROTINA DA OPERAÇÃO PAGAMENTO**************************************************************************************************
//**********************************************************************************************************************************************************
void pagamento(){   static float valor_doc,valor_pag,troco;
					cabecalho();
					
					//pergunta valor e vencimento
					printf("Valor do Documento:R$ ");
					scanf("%f",&valor_doc);
					printf("Vencimento: (DD/MM/AAAA) ");
					scanf("%d/%d/%d", &vencimento.dia, &vencimento.mes, &vencimento.ano);
					
					//verifica se a data do vencimento é menor que a data de hoje
					if((vencimento.dia < hoje.dia && vencimento.mes <= hoje.mes && vencimento.ano <= hoje.ano) || (vencimento.mes < hoje.mes && vencimento.ano <= hoje.ano) || (vencimento.ano < hoje.ano)){
					   valor_doc+=calc_juros(valor_doc);//soma o valor do documento com o juros
					}
					
					//pergunta o valor pago pelo cliente
					printf("\n\nValor Total:R$ %.2f\nValor Recebido:R$ ",valor_doc);
					scanf("%f",&valor_pag);
					
					//calcula troco caso exista
					if(valor_pag>valor_doc){ troco=valor_pag-valor_doc;
											 printf("Troco:R$ %.2f\n", troco);
											 system("pause");
										}
					else system("pause");
					
					//adiciona o valor total da operação ao montante em caixa
					em_caixa+=valor_doc;
					
					//salva informações da operação na estrutura relatorio
					relatorio.doc=valor_doc;
					relatorio.ficha=senha;
					relatorio.operacao=0;
					
					if((arquivot = fopen("REGISTRO_MOVIMENTACAO.txt","a")) == NULL)
      						{
        						printf("Erro ao abrir arquivo!!!\n\n");
        						exit(1);
      						}
					fprintf(arquivot,"Ficha: %c%d\nOperacao: %s\nValor: RS %.2f\n\n\n",tiposenha,relatorio.ficha,operacoes[relatorio.operacao],relatorio.doc);
					fclose(arquivot);
					
					}
					
//**********************************************************************************************************************************************************
//****************************FUNÇÃO QUE CALCULA JUROS******************************************************************************************************
//**********************************************************************************************************************************************************
float calc_juros(float doc){
							static char tipo;
							static float juros;
							
							for(;;){
							//pergunta se o calculo do juros será por percentual ou tarifa simples (ambos ao dia)
							printf("DIGITE:\n(T/Valor ao dia) se juros for tarifa simples por dia.\n(P/Valor ao dia) se for percentual por dia.\n");
							fflush(stdin);
							tipo=getch();
							if(tipo=='t'||tipo=='T'){ printf("Valor da taxa de juros ao dia = R$ ");
														scanf("%f",&juros);
													  return (dist_dias(vencimento,hoje) * juros);//Nº DE DIAS X TARIFA POR DIA
							}
							if(tipo=='p'||tipo=='P'){ printf("Valor do percentual de juros ao dia(0-100) = ");
														scanf("%f",&juros);
													  return (dist_dias(vencimento,hoje) * (juros/100 * doc));//(JUROS EM % AO DIA / POR 100) X Nº DE DIAS
													}
							else printf("\nDados incorretos");
							}
}

//**********************************************************************************************************************************************************
//****************************ROTINA NOVA TRANSAÇÃO*********************************************************************************************************
//**********************************************************************************************************************************************************
void nova_transacao(){
						for(;;){ 	static char opcao;
						  			cabecalho();
									printf("Nova transação? (N/S)\n");
						  			fflush(stdin);	//comando pra limpeza do buffer
									opcao=getch();
						  			if(opcao=='n'||opcao=='N'){seek_fila();break;}
						  			if(opcao=='s'||opcao=='S') break;
						}
}

//**********************************************************************************************************************************************************
//****************************ROTINA ESCRITA DO CABEÇALHO DO MENU*******************************************************************************************
//**********************************************************************************************************************************************************
void cabecalho(){
					system("cls");
					printf("\n\n\t\t\t\t<<CAIXA ABERTO>>\n\n\t\t\t\t<<SENHA:%c(%d)>>\n\n\t\t\t\tDATA DE HOJE: %2d/%2d/%2d\n\n\t\t\t\tDINHEIRO EM CAIXA: R$%.2f\n\n\nGerente: %s || Matrícula: %d\nAtendente: %s || Matrícula: %d\n\n",tiposenha,senha,hoje.dia,hoje.mes,hoje.ano,em_caixa,gerente.nome, gerente.mat,funcionario.nome,funcionario.mat);
				}
				
//**********************************************************************************************************************************************************
//****************************ROTINA DE INICIALIZAÇÃO DO CAIXA**********************************************************************************************
//**********************************************************************************************************************************************************
void inicializacaixa(){inic:
						system("cls");
						printf("\t\t\t\t<<ABRINDO CAIXA>>\n\nAUTENTICAR FUNCIONÁRIO\nmatrícula: ");
						int matricula,chave;
						scanf("%d",&matricula);
						printf("senha: ");
						scanf("%d",&chave);
						
						if((cadfun = fopen("funcionarios.bin","rb")) == NULL) exit(1);
									while(!feof(cadfun)){
															fread(&funcionario,sizeof(funcionario),1,cadfun);
															if(matricula==funcionario.mat && chave==funcionario.senha){ fclose(cadfun);
																														if(funcionario.sts==0){printf("CADASTRO INATIVO");_sleep(3000);fclose(cadfun);goto inic;}
																														printf("Confirmado");
																														_sleep(1500);
																														break;
																														}
															if(feof(cadfun)){printf("DADOS INCORRETOS");_sleep(1500);fclose(cadfun); goto inic;}
															}
						system("cls");								
						printf("\n\n\t\t\t\t<<ABRINDO CAIXA>>\n\nFuncionário:%s\n\nDigite o valor inicial depositado no caixa = R$ ",funcionario.nome);
						scanf("%f",&em_caixa);
						
						dataatual();//captura data de hoje inserida no sistema do PC
						//abre arquivo de texto para escrever o cabeçalho
 						if((arquivot = fopen("REGISTRO_MOVIMENTACAO.txt","w")) == NULL)
      						{
        						printf("Erro ao abrir arquivo!!!\n\n");
        						exit(1);
      						}
						fprintf(arquivot,"\t\t\t<<REGISTRO DE MOVIMENTACAO>>\n\t\t\tDATA:%d/%d/%d\n\nFUNCIONÁRIO RESPONSÁVEL: %s || MATRÍCULA: %d\n\n\GERENTE RESPONSÁVEL: %s || MATRÍCULA: %d\n\n\t\tVALOR DEPOSITADO NA ABERTURA: RS %.2f\n\n",hoje.dia,hoje.mes,hoje.ano,funcionario.nome,funcionario.mat,gerente.nome, gerente.mat,em_caixa);
						fclose(arquivot);
}

//**********************************************************************************************************************************************************
//****************************FUNÇÃO CONVERSORA DE CARACTER PRA INTEIRO*************************************************************************************
//**********************************************************************************************************************************************************
int valorinteiro(char c){
						switch (c){
						case '1':
						return 1;
						
						case '2':
						return 2;
						
						case '3':
						return 3;
						
						case '4':
						return 4;
						
						case '5':
						return 5;
						
						case '6':
						return 6;
						
						case '7':
						return 7;
						
						case '8':
						return 8;
						
						case '9':
						return 9;
						
						case '0':
						return 0;
						
						default:
						return 0;
						}
}

//**********************************************************************************************************************************************************
//****************************ROTINA PARA CAPTURAR DATA DO SISTEMA******************************************************************************************
//**********************************************************************************************************************************************************
void dataatual(){

//captura data do sistema e salva numa string
	char dateStr [9];
	_strdate(dateStr);
	
	//transfere o valor para um vetor de inteiros usando a subfunção valorinteiro();
	int i, datahoje[9];
	for(i=0;i<8;i++)datahoje[i]=valorinteiro(dateStr[i]);
	
	//transfere o valor para a estrutura data
	hoje.mes= (10 * datahoje[0]) + datahoje[1];
	hoje.dia= (10 * datahoje[3]) + datahoje[4];
	hoje.ano= 2000 + ((10 * datahoje[6]) + datahoje[7]);
}
//**********************************************************************************************************************************************************
//****************************FUNÇÃO DE CONTROLE&GERENCIA***************************************************************************************************
//**********************************************************************************************************************************************************


int gerencia(){
		int matricula,chave;
		char opcao;
		
		//PRIMEIRO ACESSO
		if((cadger = fopen("gerentes.bin","ab+")) == NULL){
															printf("Erro ao abrir arquivo!!!\n\n");
        													exit(1);
														}
		fseek(cadger,0,SEEK_END);
		if(ftell(cadger) < sizeof(gerente)){
											
											printf("PRIMEIRA INICIALIZAÇÃO - É NECESSÁRIO CADASTRAR UM GERENTE\nNome:(MAX 20 CARAC.) ");
											fflush(stdin);
											gets(gerente.nome);
											printf("Matrícula:(APENAS NÚMEROS) ");
											scanf("%d",&gerente.mat);
											printf("Senha: (APENAS NÚMEROS) ");
											scanf("%d",&gerente.senha);
											gerente.sts=1;
							
											fwrite(&gerente,sizeof(gerente),1,cadger);
											
											}
		fclose(cadger);
		
		
		//menu de gerência
		for(;;){ini:
					system("cls");
					printf("\t\t\t<<CAIXA BLOQUEADO>>\n\t\t\t<<GERÊNCIA>>\n\n\nMENU:\nPressione (1) para alterar senha de gerente\nPressione (2) para adicionar novo gerente\nPressione (3) para alterar status de funcionario ou gerente\nPressione (4) para imprimir na tela todos dados de funcionarios\nPressione (5) para adicionar funcionario\nPressione (S) para desbloquear caixa");
					fflush(stdin);
					opcao=getch();
					if(opcao=='1'){//alterar sua senha de gerente
									system("cls");
									printf("\t\t\t<<ALTERAÇÃO DE SENHA>>\nAUTENTICAR GERENTE\nDigite sua Matrícula: ");
									scanf("%d",&matricula);
									printf("Digite a senha antiga: ");
									scanf("%d",&chave);
									if((cadger = fopen("gerentes.bin","rb+")) == NULL) exit(1);
									while(!feof(cadger)){
															fread(&gerente,sizeof(gerente),1,cadger);
															
															if(matricula==gerente.mat && chave==gerente.senha){
																												if(gerente.sts==0){printf("CADASTRO INATIVO");_sleep(3000);fclose(cadger);goto ini;}
																												printf("Nova senha: ");
																												scanf("%d",&gerente.senha);
																												fseek(cadger, ftell(cadger) - sizeof(gerente),SEEK_SET);
																												
																												fwrite(&gerente,sizeof(gerente),1,cadger);
																												printf("SENHA ALTERADA COM SUCESSO");
																												_sleep(3000);
																											 break;
													 														}
										
														}
									fclose(cadger);
									
									
									
									}
					if(opcao=='2'){//adicionar um gerente 
									system("cls");
									printf("\t\t\t<<CADASTRO DE NOVO GERENTE>>\nAUTENTICAR GERENTE\n");
									printf("matricula: ");
									scanf("%d",&matricula);
									printf("senha: ");
									
									scanf("%d",&chave);
									if((cadger = fopen("gerentes.bin","rb")) == NULL) exit(1);
									while(!feof(cadger)){
															fread(&gerente,sizeof(gerente),1,cadger);
															if(matricula==gerente.mat && chave==gerente.senha){ fclose(cadger);
																												if(gerente.sts==0){printf("CADASTRO INATIVO");_sleep(3000);fclose(cadger);goto ini;}
																												printf("Confirmado");
																												_sleep(1500);
																												system("cls");
																												printf("\t\t\t<<CADASTRO DE NOVO GERENTE>>\n");
																												printf("nome: (MAX 20 CARAC.) ");
																												fflush(stdin);
																												gets(gerente.nome);
																												printf("matricula: ");
																												scanf("%d",&gerente.mat);
																												printf("senha: (APENAS NÚMEROS)");
																												scanf("%d",&gerente.senha);
																												gerente.sts=1;
																												if((cadger = fopen("gerentes.bin","ab")) == NULL){
																												printf("Erro ao abrir arquivo!!!\n\n");
        																										exit(1);}
        																										fwrite(&gerente,sizeof(gerente),1,cadger);
        																										printf("CADASTRO CONCLUÍDO");
        																										_sleep(3000);
																												break;
							
																												}
														}
									fclose(cadger);
																												
						
								}
					if(opcao=='3'){	system("cls");//mudar status de funcionario/gerente(ATIVO/INATIVO)
									printf("\t\t\t<MUDAR STATUS DE FUNCIONARIO/GERENTE>>\nAUTENTICAR GERENTE\nmatricula: ");
									scanf("%d",&matricula);
									printf("senha: ");
									scanf("%d",&chave);
									if((cadger = fopen("gerentes.bin","rb+")) == NULL) exit(1);
									while(!feof(cadger)){
															fread(&gerente,sizeof(gerente),1,cadger);
															if(matricula==gerente.mat && chave==gerente.senha){ if(gerente.sts==0){printf("CADASTRO INATIVO");_sleep(3000);fclose(cadger);goto ini;}
																												printf("Confirmado");
																												_sleep(3000);
																												system("cls");
																												printf("\t\t\t<MUDAR STATUS DE FUNCIONARIO/GERENTE>>\n");
																												printf("Digite a matricula do funcionario/gerente a ser alterado: ");
																												scanf("%d",&matricula);
																												if(matricula==gerente.mat){	fclose(cadger);
																																			printf("Impossível alterar o próprio status");
																																			_sleep(3000);
																																			goto ini;
																												}
																												else{	
																														
																														rewind(cadger);
																														
																														while(!feof(cadger)){
																																				fread(&gerente,sizeof(gerente),1,cadger);
																																				if(matricula==gerente.mat){
																																											printf("Encontrado seguinte cadastro\nNome: %s (Gerente)",gerente.nome);
																																											printf("Ativar/Desativar?(1/0)ou\nPressione qualquer tecla para voltar\n");
																																											fflush(stdin);
																																											opcao=getch();
																																											if(opcao=='0'||opcao=='0'){
																																																		gerente.sts=0;
																																																		fseek(cadger,ftell(cadger) - sizeof(gerente),SEEK_SET);
																																																		fwrite(&gerente,sizeof(gerente),1,cadger);
																																																		fclose(cadger);
																																																		printf("CADASTRO DESATIVADO");
																																																		_sleep(3000);
																																																		goto ini;
																																																	   }
																																											
																																											
																																											if(opcao=='1'||opcao=='1'){
																																																		gerente.sts=1;
																																																		fseek(cadger,ftell(cadger) - sizeof(gerente),SEEK_SET);
																																																		fwrite(&gerente,sizeof(gerente),1,cadger);
																																																		fclose(cadger);
																																																		printf("CADASTRO REATIVADO");
																																																		_sleep(3000);
																																																		goto ini;
																																																	   }
																																											fclose(cadger);
																																											printf("CADASTRO INALTERADO");
																																											_sleep(3000);
																																											goto ini;
																																											}																										
																																			}
																														fclose(cadger);
																														
																														if((cadfun = fopen("funcionarios.bin","rb+")) == NULL) exit(1);
																														while(!feof(cadfun)){
																																				fread(&funcionario,sizeof(funcionario),1,cadfun);
																																				if(matricula==funcionario.mat){
																																												printf("Encontrado seguinte cadastro\nNome: %s (Funcionário)",funcionario.nome);
																																												printf("Ativar/Desativar?(1/0)ou\nPressione qualquer tecla para voltar\n");
																																												fflush(stdin);
																																												opcao=getch();
																																												if(opcao=='0'||opcao=='0'){
																																																			funcionario.sts=0;
																																																			fseek(cadfun,ftell(cadfun) - sizeof(funcionario),SEEK_SET);
																																																			fwrite(&funcionario,sizeof(funcionario),1,cadfun);
																																																			fclose(cadfun);
																																																			printf("CADASTRO DESATIVADO");
																																																			_sleep(3000);
																																																			goto ini;
																																																	   	  }
																																											    if(opcao=='1'||opcao=='1'){
																																																			funcionario.sts=1;
																																																			fseek(cadfun,ftell(cadfun) - sizeof(funcionario),SEEK_SET);
																																																			fwrite(&funcionario,sizeof(funcionario),1,cadfun);
																																																			fclose(cadfun);
																																																			printf("CADASTRO REATIVADO");
																																																			_sleep(3000);
																																																			goto ini;
																																																	   	  }
																																												fclose(cadfun);
																																												printf("CADASTRO INALTERADO");
																																												_sleep(3000);
																																												goto ini;
																																												}																										
																																			}
																														fclose(cadfun);																												
																													}
																												}
														}
								   }
																														
									
					if(opcao=='5'){//adicona funcionarios
									system("cls");
									printf("\t\t\t<<CADASTRAR NOVO FUNCIONARIO>>\nAUTENTICAR GERENTE\nmatricula: ");
									scanf("%d",&matricula);
									printf("senha: ");
									scanf("%d",&chave);
									if((cadger = fopen("gerentes.bin","rb")) == NULL) exit(1);
									while(!feof(cadger)){
															fread(&gerente,sizeof(gerente),1,cadger);
															if(matricula==gerente.mat && chave==gerente.senha){ fclose(cadger);
																												if(gerente.sts==0){printf("CADASTRO INATIVO");_sleep(3000);fclose(cadger);goto ini;}
																												printf("Confirmado");
																												_sleep(1500);
																												system("cls");
																												printf("\t\t\t<<CADASTRAR NOVO FUNCIONARIO>>\n");
																												printf("Nome:(MAX 20 CARAC.) ");
																												fflush(stdin);
																												gets(funcionario.nome);
																												printf("Matricula: ");
																												scanf("%d",&funcionario.mat);
																												printf("Senha: (APENAS NÚMEROS) ");
																												scanf("%d",&funcionario.senha);
																												funcionario.sts=1;
																												
																												if((cadfun = fopen("funcionarios.bin","ab")) == NULL) exit(1);
																												fwrite(&funcionario,sizeof(funcionario),1,cadfun);
																												fclose(cadfun);
																												printf("CADASTRO CONCLUÍDO");
																												_sleep(3000);
																												break;
																												}
														}
									fclose(cadger);
								}
					if(opcao=='4'){//lista todos os funcionarios registrados	
									system("cls");
									printf("\t\t\t<<DETALHAR TODOS OS FUNCIONARIOS>>\nAUTENTICAR GERENTE\nmatricula: ");
									scanf("%d",&matricula);
									printf("senha: ");
									scanf("%d",&chave);
									if((cadger = fopen("gerentes.bin","rb")) == NULL) exit(1);
									while(!feof(cadger)){
															fread(&gerente,sizeof(gerente),1,cadger);
															if(matricula==gerente.mat && chave==gerente.senha){ fclose(cadger);
																												if(gerente.sts==0){printf("CADASTRO INATIVO");_sleep(3000);fclose(cadger);goto ini;}
																												if((cadfun = fopen("funcionarios.bin","rb+")) == NULL) exit(1);
																												do{
																																		fread(&funcionario,sizeof(funcionario),1,cadfun);
																																		if(feof(cadfun))continue;
																																		printf("\n\nNome:%s\nMatricula:%d\nSenha:%d\nstatus:%s\n\n\n",funcionario.nome,funcionario.mat,funcionario.senha,status[funcionario.sts]);
																																		
																																	}while(!feof(cadfun));
																												fclose(cadfun);
																												break;
																												}
															
														}
									fclose(cadger);
									system("pause");
									}
					if(opcao=='s'||opcao=='S'){//fecha menu de gerência, desbloqueia caixa para procedimento de abertura
					 system("cls");
									printf("\t\t\t<<DESBLOQUEAR CAIXA>>\nAUTENTICAR GERENTE\nmatricula: ");
									scanf("%d",&matricula);
									printf("senha: ");
									scanf("%d",&chave);
									if((cadger = fopen("gerentes.bin","rb")) == NULL) exit(1);
									while(!feof(cadger)){
															fread(&gerente,sizeof(gerente),1,cadger);
															if(matricula==gerente.mat && chave==gerente.senha){ fclose(cadger);
																												if(gerente.sts==0){printf("CADASTRO INATIVO");_sleep(3000);fclose(cadger);goto ini;}
																												return 0;}
					
			
														}
										}
		
}}

//**********************************************************************************************************************************************************
//****************************FUNÇAO BISSEXTO (RETORNA 1 SE BISSEXTO OU 0 SE NÃO) **************************************************************************
//**********************************************************************************************************************************************************

/*
 * Retorna 1 caso 'ano' seja bissexto, 0 caso contrário
 */
int bissexto (int ano) {
	return (ano % 4 == 0) && ((ano % 100 != 0) || (ano % 400 == 0));
}


//**********************************************************************************************************************************************************
//****************************FUNÇÃO PARA CALCULAR DIFERENÇA ENTRE DATAS (EM DIAS)**************************************************************************
//**********************************************************************************************************************************************************


/*
 * Retorna a distancia entre inicio e fim em dias.
 * Assume que inicio nao vem depois de fim.
 */
unsigned long dist_dias (Data inicio, Data fim) {
	unsigned long idias, fdias;	// guarda qtos dias tem da data  ate o comeco do ano 
	unsigned long def_anos = 0;	// guarda diferenca entre anos das datas inicio e fim medida em dias 
	register int i;
	int dbissexto;
	
	idias = inicio.dia;//guarda os dias
	dbissexto = bissexto (inicio.ano);//assume 0 ou 1 caso bissexto
	
	//acumula em dias os meses de diferença descritos na tabela [dias_mes]
	for (i = inicio.mes - 1; i > 0; --i)
		idias += dias_mes[dbissexto][i];
	
	fdias = fim.dia;
	dbissexto = bissexto (fim.ano);
	
	for (i = fim.mes - 1; i > 0; --i)
		fdias += dias_mes[dbissexto][i];
	
	//acumula em dias os anos de diferença entre as duas datas
	while (inicio.ano < fim.ano)
		def_anos += 365 + bissexto(inicio.ano++);
	//retorna a diferença das datas em dias
	return def_anos - idias + fdias;
}
