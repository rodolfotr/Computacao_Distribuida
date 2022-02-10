#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <sys/time.h>
#include <unistd.h>

//Definição do ip e porta multicast.
#define IP_MULTICAST "230.1.2.3"
#define PORTA_MULTICAST 6543

void die(char *s){
    perror(s);
    exit(1);
}

//Main.
int main(int argc) {
    printf("INICIANDO ROTEADOR COM SERVIÇO DE PROVER IP PARA BACKUP!\n\n");

    struct sockaddr_in clienteAddr;
    struct in_addr local;
	unsigned int slen = 50, bytesLidos;
	int s, i;
	char mensagem[50];

	while(1) {
        if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
            die("socket()");
        
        memset((char *) &clienteAddr, 0, sizeof(clienteAddr));
        
        clienteAddr.sin_family = AF_INET;
        clienteAddr.sin_addr.s_addr = inet_addr(IP_MULTICAST);
        clienteAddr.sin_port = htons(PORTA_MULTICAST);
        
        local.s_addr = inet_addr("203.106.93.94");
        setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF, (char *)&local, sizeof(local));

        if (bind(s, (struct sockaddr *) &clienteAddr, sizeof(clienteAddr)) == -1)
            die("bind()");

		mensagem = malloc(sizeof(mensagem));
		fflush(stdout);

		//Recebe os dados.
		if ((recv_len = recvfrom(s, mensagem, 50, 0, (struct sockaddr *) &si_other, &slen)) == -1)
			die("recvfrom()");

		if (sendto(s, pck, BUFLEN, 0, (struct sockaddr *) &si_other, slen) == -1)
			die("sendto()");
	}
    die("socket()");
    return 0;
}



//Envia a mensagem para próximo roteador.
void envia_mensagem(packet* pck, vizinho* next) {
	struct sockaddr_in si_other;
	unsigned int slen = sizeof(si_other);
	int tries_until_disconnect = TRIES_UNTIL_DISCONNECT;
	int ack = 0;
	int s;
	
	if(pck->type == DATA_TYPE && (
			dv_table_.distance[LOCAL_ROUTER][next->id_dst].alcancavel == false ||
			dv_table_.distance[LOCAL_ROUTER][next->id_dst].custo == INFINITE)) {
		pthread_mutex_lock(&printf_mutex);
		printf("-----\nImpossível alcançar o roteador %d.\n", next->id_dst);
		pthread_mutex_unlock(&printf_mutex);
		return;
	}

	memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(next->port_dst);
    
    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		die("socket()");
	
    if (inet_aton(next->host_dst, &si_other.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
	//Envia o pacote.
	while (tries_until_disconnect--) {
		if (sendto(s, pck, BUFLEN, 0, (struct sockaddr *) &si_other, slen) == -1)
			die("sendto()");
		
		struct timeval timeout;
		timeout.tv_sec = TIMEOUT;
		timeout.tv_usec = TIMEOUT*2;
	
		setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
		recvfrom(s, &ack, sizeof(ack), 0, (struct sockaddr *) &si_other, &slen);
		
		if (ack)
			break;
	}

	if (!ack) {
		pthread_mutex_lock(&printf_mutex);
		printf("-----\nRoteador %d inalcançável após %d tentativas com estouro de timeout.\n", next->id_dst, TRIES_UNTIL_DISCONNECT);
		
		if (dv_table_.distance[next->id_dst][LOCAL_ROUTER].alcancavel == true) {
			atualiza_caso_erro(next->id_dst);
			imprime_dv_table();
		}
		printf("\n\n\n");
		pthread_mutex_unlock(&printf_mutex);
	}
}



//Função que recebe a mensagem e mostra em console.
void* receber_mensagem(void *data) {
	struct sockaddr_in si_me, si_other;
	unsigned int slen = sizeof(si_other), recv_len, vizinho_index;
	int s, ack, i;
	
	packet* pck;
	vizinho* next;
	
	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		die("socket()");
	
	memset((char *) &si_me, 0, sizeof(si_me));
	
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(LOCAL_ROUTER_PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if (bind(s, (struct sockaddr *) &si_me, sizeof(si_me)) == -1)
		die("bind()");

	while(1) {
		pck = (packet *)malloc(sizeof(packet));
		fflush(stdout);

		//Recebe os dados.
		if ((recv_len = recvfrom(s, pck, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
			die("recvfrom()");

		switch (pck->type) {
			case DATA_TYPE:
				if (pck->id_dst == LOCAL_ROUTER) {
					//Se este roteador for o destino, imprime a mensagem.
					pthread_mutex_lock(&printf_mutex);
					printf("\n\n||-- Mensagem recebida de %d:--||\n%s\n||----------------------------||\n\n", pck->id_src, pck->data);
					pthread_mutex_unlock(&printf_mutex);
				} else {
					//Se já alcançou a quantidade de saltos máxima, para de encaminhar.
					int qt_routers = 0;
					for (i = 0; i < MAX_ROUTERS; i++) {
						if (dv_table_.distance[LOCAL_ROUTER][i].alcancavel == true) 
							qt_routers++;
					}

					if (pck->jump > qt_routers) break;
					(pck->jump)++;
					
					//Busco vizinho na tabela de roteamento.
					vizinho_index = busca_prox_router(pck->id_dst);
					if (vizinho_index < 0)
						continue;

					next = &vizinhos[vizinho_index];

					//Se o roteador não é o destino, encaminha ao próximo roteador.
					pthread_mutex_lock(&printf_mutex);
					printf("\nEsse roteador não é o destino da mensagem. Reencaminhando pelo IP/Porta: %s/%d\n", next->host_dst, next->port_dst);
					pthread_mutex_unlock(&printf_mutex);

					//Envia para o roteador.
					envia_mensagem(pck, next);
				}
				break;
			case DV_TYPE: ;
				pthread_t thread;
				pthread_create(&thread, NULL, atualiza_dv, &pck->dv);
			break;
			default:
			break;
		}

		ack=1;
		if (sendto(s, &ack, sizeof(ack), 0, (struct sockaddr *) &si_other, slen) == -1)
			die("sendto()");
		free(pck);
	}

	pthread_exit(NULL);
}