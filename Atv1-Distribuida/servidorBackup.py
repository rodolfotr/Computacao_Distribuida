import socket
import struct

IP_BACKUP = '127.0.0.1'
PORTA_BACKUP = 5000
ARQUIVO_BACKUP = "/home/aluno-uffs/Documentos/Trab_Final/Atv1-Distribuida/cliente_BACKUP.c"

#Recebe o arquivo.
sockReceber = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
sockReceber.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sockReceber.bind((IP_BACKUP, PORTA_BACKUP))


while (True):
    l = sockReceber.recv(1561651651)
    if (l):
        f = open(ARQUIVO_BACKUP,'wb')
        f.write(l)
        f.close()

sockReceber.close()