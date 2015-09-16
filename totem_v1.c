#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <windows.h>



typedef struct{
	int pos;
	int sts;
}fila;

FILE *prioritario;
FILE *comum;


int main(void) { setlocale(LC_ALL,"Portuguese");
	char opcao;
	fila com,prio;
	com.pos=prio.pos=com.sts=prio.sts=0;
	

for(;;){
	
	system("cls");
	printf("\t\tPRESSIONE (P) PARA PRIORITÁRIO E (C) PARA COMUM\n");
	fflush(stdin);
	opcao=getch();
	
	if(opcao == 'P' || opcao == 'p'){
										prio.pos++;
										printf("\n\n\t\tSENHA NÚMERO: P(%d)",prio.pos);
										_sleep(3000);
										
										if((prioritario = fopen("filaprio.bin","ab")) == NULL)
      									{
        									printf("Erro ao abrir arquivo!!!\n\n");
       										exit(1);
      									}
      									fwrite(&prio,sizeof(prio),1,prioritario);
      									fclose(prioritario);
										}
	
	if(opcao == 'C' || opcao == 'c'){
										com.pos++;
										printf("\n\n\t\tSENHA NÚMERO: C(%d)",com.pos);
										_sleep(3000);
										
										if((comum = fopen("filacom.bin","ab")) == NULL)
      									{
        									printf("Erro ao abrir arquivo!!!\n\n");
       										exit(1);
      									}
      									fwrite(&com,sizeof(com),1,comum);
      									fclose(comum);
		
										}
	
}
	
	return 0;
}
