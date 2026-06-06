# Projeto 2 da matéria de Projetos de Sistemas Embarcados

> Implementação de um leitor de teclado matricial e exibição de caracteres no LCD + senha de cofre, escrito em assembly para a placa do professor Emerson Pedrino, com arquitetura intel 8051.

---

## Índice

* [Sobre o Projeto](#sobre-o-projeto)
* [Funcionalidades e Rotinas](#funcionalidades-e-rotinas)
* [Pré-requisitos](#pré-requisitos)
* [Fontes](#fontes)

---

## Sobre o Projeto

O código foi desenvolvido em linguagem Assembly para realizar a leitura de um teclado matricial e exibir os dados em um display LCD, simulando o sistema de validação de um cofre digital. O programa faz a varredura contínua dos pinos de entrada, armazena os dígitos inseridos e faz a comparação com uma senha previamente gravada na memória, oferecendo feedback visual dependendo do resultado.

Para a construção da lógica, a ordem dos dígitos inseridos importa durante a verificação: 2]. O sistema garante que cada caractere pressionado adicione um valor específico na sequência de bytes que será avaliada posteriormente: 3].

---

## Funcionalidades e Rotinas

Para o funcionamento do código em Assembly, o projeto foi dividido nas seguintes rotinas e estruturas principais:

* **Inicialização e Controle do LCD (`FuncS`, `DispC`, `EntryMode`):** Configura a interface de comunicação de 4 bits com o display, definindo o modo de entrada e enviando comandos de controle, como a instrução que desliga o display.
* **Exibição de Texto (`PrintString`):** Lê os dados da memória de programa e os envia ao display, iterando e sempre apontando para o próximo caractere da string até encontrar o terminador nulo.
* **Varredura do Teclado (`ScanKey` e `IDCode`):** Mapeia as linhas e colunas do teclado matricial conectado à porta P0: 16]. Utiliza a rotina `ESPSOL` combinada com o Timer 0 para fazer o debounce e aguardar a liberação da tecla.
* **Armazenamento e Comparação (`Compare`):** A sequência digitada é gravada na memória RAM, utilizando o registrador R1 inicializado no endereço 30h como ponteiro:. A rotina compara a entrada dígito a dígito com a senha salva.
* **Tratamento de Resultados:**
  * **Sucesso:** Caso a sequência informada seja igual à senha, o sistema exibe a mensagem de liberação "Access Granted".
  * **Falha:** Caso a senha não confira, o display exibe a mensagem "ERROU KKK": 12]. Após a exibição do erro, o programa aguarda um intervalo de tempo e reinicia o ponteiro da RAM, permitindo que o usuário digite uma nova sequência.

---

## Pré-requisitos

Antes de começar, certifique-se de ter as seguintes ferramentas instaladas em sua máquina:

* [Simulador Edsim51](https://edsim51.com/)
* Caso seja aluno, o software gravador via Arduino disponibilizado para a disciplina.

---

## Fontes

* Aulas e slides do professor Emerson Pedrino.
* PS: Esse README foi gerado com auxilio de IA
