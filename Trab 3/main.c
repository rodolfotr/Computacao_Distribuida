/*
Universidade Federal da Fronteira Sul
Curso: Ciencia da Computação
Disciplina: Computação Distribuida
Trabalho: Implementação de Quick Sort em lista dupla com ordenação em multithread e na GPU.
Aluno: Rodolfo Trevisol

Instalação do OpenACC:
$ echo 'deb [trusted=yes] https://developer.download.nvidia.com/hpc-sdk/ubuntu/amd64 /' | sudo tee /etc/apt/sources.list.d/nvhpc.list
$ sudo apt-get update -y
$ sudo apt-get install -y nvhpc-22-3

https://www.pgroup.com/resources/docs/18.3/pdf/openacc18_gs.pdf
https://www.bu.edu/tech/files/2017/04/OpenACC-2017Spring.pdf
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>

typedef struct info{
    char *nome;
    int idade;
}info;

typedef struct list{
    info *data;
    struct list *next, *prev;
}list;

typedef struct head{
    list *first,*last;
}head;

char NamePrefix[][5] = {
    "cba", "tar", "xan", "koa", "nat",
    "asd", "rvf", "gjf", "aps", "sfp",
    "rge", "wmk"
};
char NameSuffix[][5] = {
    "abc", "aus", "isx", "dox", "ith",
    "ath", "ums", "aor", "orf", "xia",
    "imu", "ais", "iur", "ore", "ofs",
    "hsf"
};
const char NameStems[][10] = {
    "adu", "aes", "anm", "all", "ima",
    "edu", "eis", "exr", "gus", "han",
    "equ", "ara", "hum", "ice", "ill",
    "ine", "iuv", "obe", "oul", "orb"
};


void remover(head *cabeca);
void insert(head *cabeca,info *pessoa,list *lista);
info *criainfo();
int printar(head *cabeca);
list* partition_list(list *first, list *last);
void quickSortLista(list *first, list *last);

int main(){
    system("clear");
    info *ovo;
    list *lista=NULL;
    head *cabeca=malloc(sizeof(head));
    cabeca->first=NULL;
    cabeca->last=NULL;
    
    srand ( time(0) ^ getpid() );
    
    for(int i = 0; i < 20; i++){
        ovo = criainfo();
        insert(cabeca, ovo, lista);
    }
    printf("\nDesOrdenado:\n");
    int a;
    a = printar(cabeca);
    
    quickSortLista(cabeca->first, cabeca->last);

    printf("\nOrdenado:\n");
    a = printar(cabeca);
    return 0;
}


void insert(head *cabeca,info *pessoa,list *lista){
    list *novo = malloc(sizeof(list));
    novo->data = pessoa;
    list *aux;
    if(cabeca->first == NULL){
        cabeca->first = novo;
        cabeca->last = novo;
    }else{
        cabeca->first->prev = novo;
        novo->next = cabeca->first;
        cabeca->first = novo;
    }
}

info *criainfo(){
    info *novo=malloc(sizeof(info));
    char aux1;
    char *nome = "";
    int j = 0;
    int r;
    char *Player1Name = malloc(9);;
    strcat(Player1Name, NamePrefix[(rand() % 12)]);
    strcat(Player1Name, NameStems[(rand() % 20)]);
    strcat(Player1Name, NameSuffix[(rand() % 16)]);
    Player1Name[0]=toupper(Player1Name[0]);

    novo->nome = Player1Name;
    novo->idade = (rand() % 90);
    return novo;
}

int printar(head *cabeca){
    int i=1;
    list *aux;

    for(aux = cabeca->first; aux != NULL; aux = aux->next,i++){
        printf("%s\n", aux->data->nome);
    }
    return i;
}

void remover(head *cabeca){
    list *aux=cabeca->first;
    int delete,i=1,tamlista;
    tamlista=printar(cabeca);
    printf("\nDigite o código correspondente a pessoa que você quer deletar: " );
    scanf("%d",&delete );
    if(delete<=(tamlista-1)&&delete>0){
        for(;aux!=NULL&&i<=delete; aux=aux->next,i++){
            if(i==delete){
                if(aux->prev==NULL && aux->next==NULL){
                    cabeca->first=NULL;
                    cabeca->last=NULL;
                }else if(aux->prev==NULL){
                    aux->next->prev=NULL;
                    cabeca->first=aux->next;
                }else if(aux->next==NULL){
                    aux->prev->next=NULL;
                    cabeca->last=aux->prev;
                }else{
                    aux->prev->next=aux->next;
                    aux->next->prev=aux->prev;
                }
                free(aux->data);
                free(aux);
                printf("\nPessoa deletada com sucesso");
            }
        }
    }else
        printf("\nCódigo inválido\n" );
}

void quickSortLista(list *first, list* last){
    list *pivo;

    if(last != NULL && first != last && first != last->next){
        pivo = partition_list(first, last);
        //paraleliza
        quickSortLista(first, pivo->prev);
        //paraleliza
        quickSortLista(pivo->next, last);
    }
}

list* partition_list(list *first, list* last){
	list *i, *j, *pivo;
	info aux;

	pivo = last;
	j = first->prev;

	for (i = first; i != last; i = i->next){
		if (strcmp(i->data->nome, pivo->data->nome) <= 0){
            if(j == NULL)
				j = first;
            else
				j = j->next;

            aux.nome = malloc(sizeof(j->data->nome));
			strcpy(aux.nome, j->data->nome);
			aux.idade = j->data->idade;
			strcpy(j->data->nome, i->data->nome);
			j->data->idade = i->data->idade;
			strcpy(i->data->nome, aux.nome);
			i->data->idade = aux.idade;
		}
	}

	if(j == NULL)	j = first;
	else	j = j->next;

    aux.nome = malloc(sizeof(j->data->nome));
    strcpy(aux.nome, j->data->nome);
	aux.idade = j->data->idade;
	strcpy(j->data->nome, last->data->nome);
	j->data->idade = last->data->idade;
	strcpy(last->data->nome, aux.nome);
	last->data->idade = aux.idade;

  return j;
}




/*
Node *radixSort(Node *listHead){
    Node *ptrArray[10] = {0};
    Node *tempPtr;
    Node *headPtr =  listHead;

    for(int index=2; index >= 0; index--){
        for(int count=0; count <= 9; count++){
            headPtr = listHead;
            while(headPtr->next != NULL){
                if(headPtr->itemArray[index] == count){
                    tempPtr = ptrArray[count];
                    if(tempPtr == NULL){
                        int number = headPtr->item;
                        tempPtr = new Node(number);
                    }
                    else{
                        while(tempPtr->next != NULL){
                            tempPtr = tempPtr->next;
                        }
                        tempPtr->next = new Node(headPtr->item);
                    }
                }
                headPtr = headPtr->next;
            }
        }
            headPtr = listHead;
            while(headPtr->next != NULL){
                for(int i=0; i <= 9; i++){
                    tempPtr = ptrArray[i];
                    while(tempPtr->next != NULL){
                        headPtr->item = tempPtr->item;
                        headPtr = headPtr->next;
                        tempPtr = tempPtr->next;
                    }
                }
            }
    }
    return listHead;
}
*/
