import socket
import struct
import filecmp
import os
import time

GRUPO_MULTICAST = '230.4.5.6'
PORTA_MULTICAST = 6543
PORTA_RECEBER = 5005
IP_RECEBER = "172.20.83.123"
ARQUIVO = "/home/aluno-uffs/Documentos/Trab_Final/Atv1-Distribuida/cliente.c"

#Peço pro servidor qual o IP da maquina de bkp
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 2)
sock.sendto(b"GIMME_IP_TO_BACKUPS", (GRUPO_MULTICAST, PORTA_MULTICAST))

#Recebo o ip da maquina de bkp
sockReceber = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
sockReceber.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sockReceber.bind((IP_RECEBER, PORTA_RECEBER))
msg, cliente = sockReceber.recvfrom(10240)

ip_porta = str(msg).split(':')
ip_backup = str(ip_porta[0]).replace("'", "").replace("b","")
porta_backup = str(ip_porta[1]).replace("'", "")
print(ip_backup)
print(porta_backup)


#Envio meu arquivo pro servidor de backup.
#Compara se arquivo mudou.
def read_file():
    with open(ARQUIVO, "r") as f:
        SMRF1 = f.readlines()
    return SMRF1

initial = read_file()

while True:
    time.sleep(1)
    current = read_file()

    if initial != current:
        print('diferente')
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 2)

        f = open(ARQUIVO, "rb")
        l = f.read(1561651651)

        print('enviando')
        nit = 1
        while (l):
            print(nit)
            sock.sendto(l, (ip_backup, int(porta_backup)))
            l = f.read(1561651651)
            nit = nit + 1

        #sock.close()

        initial = current
