# Entrega 01 Fundamento de Redes de Computadores

| Aluno   |      Matrícula    
|----------|:-------------:
| Thiago Luiz de Souza Gomes |  180028324 
| Wagner Martins da Cunha |    180029177 

<br>

## Como rodar

### Dependências
```
gcc 
```

### Script de compilação

Para compilar todos os arquivos necessários, utilize o comando:

```sh
$ bash compile.sh
```

Este script criará os binários: server, client, server_app e client_app. Inicie os binários na ordem das etapas  descritas a seguir (se usado o script de compilação, pular os comandos com gcc).

### Primeira etapa

Crie um arquivo como teste.c

Exemplo:
```c
#include <stdio.h>
#include <stdlib.h>

int main(){
    printf("Hello World");
    return 0;
}
```

### Segunda etapa
Abra um terminal e dentro da pasta do projeto execute os seguintes comandos
```
gcc utils.c link_layer_client.c -o client -lrt
./client <int tamanho_PDU>
```
Resultado:

![primeiro print](./img/primeiro_print.png)

**Obs**: Nesta exemplo não foi colocado um tamanho especifíco para o PDU então setou 100 como padrão



### Terceira etapa
Abra um terminal e dentro da pasta do projeto execute os seguintes comandos
```
gcc utils.c link_layer_server.c -o server -lrt
./server <int tamanho_PDU>
```
Resultado após a recepção da mensagem do cliente:

![segundo print](./img/segundo_print.png)

**Obs**: Nesta exemplo não foi colocado um tamanho especifíco para o PDU então setou 100 como padrão

## Quarta etapa
Abra um terminal e dentro da pasta do projeto execute os seguintes comandos
```
gcc utils.c server_app.c -o server_app -lrt
./server_app
```
Resultado:

![quinto print](./img/quinto_print.png)


### Quinta etapa
Abra um terminal e dentro da pasta do projeto execute os seguintes comandos
```
gcc utils.c client_app.c -o client_app -lrt
./client_app teste.c
```
Resultado:

![terceiro print](./img/terceiro_print.png)

**Observe o terminal da terceira etapa:**

![quarto print](./img/quarto_print.png)


## Sobre o projeto

### Descrição da arquitetura

A primeira versão deste projeto se trata de um simulação da camada de enlace ponto-a-ponto, simplex, não orientado à conexão, e stop and wait.

O projeto é composto de 2 camadas, a camada de enlace(N-1), e a camada de aplicação(N).

A camada de enlace é composta pelo cliente, que envia as mensagens e pelo servidor, que recebe as mensagens.

Cada aplicação tem sua respectiva camada de enlace (os dois estão no mesmo host, e a interface do enlace é usada pela aplicação). Para comunicação entre estes processos são usadas filas de mensagem (man mq_overview para o detalhamento).

O fluxo do projeto no momento pode ser descrito da seguinte forma:

||Aplicação Cliente|Camada de enlace Cliente|Camada de enlace Server|Aplicação Server|
|-|-|-|-|-|
|1||Inicializada com DPU de tamanho N (escolhido pelo usuário);<br>Abre uma fila de escrita e uma de leitura|Inicializada com DPU de tamanho N(escolhido pelo usuário);<br>Abre uma fila de escrita e uma de leitura|
|2||Espera por novas mensagens da fila|Espera por novas mensagens do socket|
|3|Lê um arquivo e envia para a fila;<br>Espera mensagem da fila |||Inicializa e espera por novas mensagens da fila
|4||Recebe mensagens da fila (uma por vez);<br>Adiciona hash (simula crc);<br>Adiciona erro aleatoriamente;<br>Envia para o server;<br>Espera confirmação.||
|5|||Recebe mensagem do socket;<br>Remove hash;<br>Verifica erros;<br>Envia mensagem pra fila;<br>Envia confirmação da mensagem pro cliente;<br>Volta a esperar novas mensagens do cliente.|
|6||Se confirmação positiva: envia próximo quadro;<br>Se confirmação negativa: envia o quadro anterior novamente;<br>Ao terminar de enviar mensagens, envia confirmação para a fila.||Recebe mensagem da fila e escreve no terminal
|7|Encerra|Volta para a linha 2|Volta para a linha 2|Volta a esperar mensagens da fila.
