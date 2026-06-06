# Projeto 3 da matéria de Projetos de Sistemas Embarcados

> Implementação de um leitor de teclado matricial e exibição de caracteres no LCD + senha de cofre, escrito em linguagem C para a placa do professor Emerson Pedrino, com arquitetura intel 8051.

---

## Índice

* [Sobre o Projeto](#sobre-o-projeto)
* [Funcionalidades e Funções](#funcionalidades-e-funções)
* [Pré-requisitos](#pré-requisitos)
* [Fontes](#fontes)

---

## Sobre o Projeto

Este projeto consiste na reescrita e adaptação do sistema de validação de cofre digital (anteriormente feito em Assembly) para a linguagem C, utilizando a biblioteca `<reg51.h>`. O firmware realiza a leitura de um teclado matricial, exibe os dados em um display LCD e compara a sequência digitada com uma senha previamente definida no código.

A lógica estrutural é mantida: o sistema faz a varredura contínua dos pinos do teclado, armazena os dígitos inseridos em um buffer na memória RAM utilizando ponteiros e avalia a sequência de bytes para liberar ou bloquear o acesso.

---

## Funcionalidades e Funções

O código em C foi modularizado nas seguintes funções principais para controle do microcontrolador:

* **Inicialização e Controle do LCD (`FuncS`, `DispC`, `EntryMode`):** Configura a interface de comunicação de 4 bits com o display. Controla os pinos de Enable e Register Select, definindo o modo de entrada e configurações de exibição.
* **Exibição de Texto (`PrintString`, `SendChar`):** Envia caracteres individuais e strings completas para o display LCD, dividindo os bytes em nibble alto e nibble baixo para adequação à interface de 4 bits.
* **Varredura do Teclado (`ScanKey`):** Realiza a varredura das linhas (conectadas aos pinos P0.0 a P0.3) e colunas (P0.4 a P0.6) para identificar a tecla pressionada.
* **Tratamento de Ruído / Debounce (`ESPSOL`):** Utiliza um laço de repetição para aguardar a liberação da tecla e emprega o Timer 0 (registradores TH0 e TL0) para gerar um delay preciso que evita leituras duplicadas (debounce).
* **Armazenamento e Comparação (`Compare`):** Os dígitos pressionados são armazenados sequencialmente a partir do endereço de memória RAM `0x30` através do uso de ponteiros (`BufferPtr`). A função itera sobre este buffer verificando se a entrada corresponde ao array `Password[]` (definido como '4', '0', '4', '6').
* **Tratamento de Resultados:**
  * **Sucesso:** Se os caracteres conferem com a senha, exibe a mensagem "Access Granted" e entra em um laço infinito.
  * **Falha:** Se qualquer caractere for incorreto, exibe a mensagem de erro "ERROU KKK", aguarda um tempo longo utilizando loops aninhados (`delay_longo`), e reinicia o ponteiro de memória e o contador para uma nova tentativa.

---

## Pré-requisitos

Antes de começar, certifique-se de ter as seguintes ferramentas instaladas em sua máquina:

* Compilador C para arquitetura 8051 (ex: SDCC ou Keil uVision)
* [Simulador Edsim51](https://edsim51.com/)
* Caso seja aluno, o software gravador via Arduino disponibilizado para a disciplina.

---

## Fontes

* Aulas e slides do professor Emerson Pedrino.
* Este README foi criado com auxilio de IA