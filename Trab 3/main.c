/*
Universidade Federal da Fronteira Sul
Curso: Ciencia da Computação
Disciplina: Computação Distribuida
Trabalho: Implementação de Quick Sort em lista dupla com ordenação em multithread e na GPU.
Aluno: Rodolfo Trevisol, Murilo Olveira

Instalação do OpenMP:
sudo apt-get install gcc-multilib
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>

//OpenMP Include
#include <omp.h>

int N_NAMES = 50000;
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

char NamePrefix[][50] = {
    "leesd","jewel","chord","braid","ltain","cower","uncle","guard","hhaft","ridge",
    "osund","hhame","beard","ljake","climb","point","carry","fence","gweat","march",
    "noast","knife","relax","rough","guest","bheel","spray","lease","charm","draft",
    "morce","lweep","write","dsoof","bweet","colon","voter","grade","patch","north",
    "board","trend","grace","ksate","treat","grave","grain","quote","basis","mhnus"
};

char NameSuffix[][50] = {
    "axis","oven","thaw","fuss","road","leak","gain","like","tree","save",
    "game","loud","drop","hang","pony","fool","safe","sock","sale","loot",
    "wall","hour","view","glow","worm","wrap","bald","fire","slot","will",
    "role","part","copy","core","calf","ball","band","pass","live","stir",
    "zove","rule","salt","wash","open","need","gear","deck","moon","kick"
};

const char NameStems[][50] = {
    "new","win","owe","see","red","pay","tax","bow","cap","dip",
    "bus","law","raw","pie","oil","pop","arm","sun","joy","rib",
    "hen","eye","old","cry","fog","few","lie","pin","run","pit",
    "put","dog","gun","can","top","era","tie","lid","egg","vat",
    "add","get","fee","fat","beg","bin","end","fan","lay","kit"
};


void remover(head *cabeca);
void insert(head *cabeca,info *pessoa,list *lista);
info *criainfo();
int printar(head *cabeca);
list* partition_list(list *first, list *last);
void quickSortLista(list *first, list *last);

int main(){
    info *ovo;
    list *lista=NULL;
    head *cabeca=malloc(sizeof(head));
    cabeca->first=NULL;
    cabeca->last=NULL;
    int a;


    srand ( time(0) ^ getpid() );
    
    for(int i = 0; i < N_NAMES; i++){
        ovo = criainfo();
        insert(cabeca, ovo, lista);
    }




    time_t     now;
    struct tm  ts;
    char       createdAt[80], endedAt[80];

    //GPU
    //#pragma omp target teams
    //Parallel
    #pragma omp parallel shared(cabeca) private(createdAt, endedAt)
    {
        time(&now);
        ts = *localtime(&now);
        strftime(createdAt, sizeof(createdAt), "%Y-%m-%d %H:%M:%S", &ts);
        printf("\n\tThread %d iniciou: %s", omp_get_thread_num(), createdAt);
        
        //#pragma omp single nowait
        quickSortLista(cabeca->first, cabeca->last);

        time(&now);
        ts = *localtime(&now);
        strftime(endedAt, sizeof(endedAt), "%Y-%m-%d %H:%M:%S", &ts);
        printf("\n\tThread %d finalizou: %s", omp_get_thread_num(), endedAt);

    }
    
    if(omp_get_thread_num() == 0){
        printf("\nEsperando 10seg:");
        sleep(10);
        //printar(cabeca);
    }
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
    char *Player1Name = malloc(12);
    strcat(Player1Name, NamePrefix[(rand() % 50)]);
    strcat(Player1Name, NameStems[(rand() % 50)]);
    strcat(Player1Name, NameSuffix[(rand() % 50)]);
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
    list *pirvoPrev;
    list *pirvoNext;

    if(last != NULL && first != last && first != last->next){
        pivo = partition_list(first, last);
        
        pirvoPrev = pivo->prev;
        pirvoNext = pivo->next;
        
        /*#pragma omp parallel firstprivate(first, pirvoPrev)
        {   */
            quickSortLista(first, pirvoPrev);
        //}
    
        quickSortLista(pirvoNext, last);
        
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