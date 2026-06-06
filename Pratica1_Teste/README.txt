# Projeto 1 da matéria de Projetos de Sistemas Embarcados

> Implementação e teste simples um blinker escrito em assembly para a placa do professor Emerson Pedrino, com arquitetura intel 8051

---


## Sobre o Projeto

Código simples para realizar o teste nas placas. Trata-se de um aquecimento. Para o código em assembly, se criou duas rotinas principais:
	back: Apaga o LED, fica em delay, logo em seguida acende o led e roda o delay de novo. ao fim do delay, pula devolta para esse label
	delay: Uma rotina de laços aninhados, complexidade n*m, ou seja, a quantidade de iterações do laço externo multiplicado pelo interno. Utilizado para manter o led acesso e apagado por intervalos regulares de tempo.

O LED é acesso nesses intervalos, setando o pino 1.0 para ligado e desligado.

---

## Pré-requisitos

Antes de começar, certifique-se de ter as seguintes ferramentas instaladas em sua máquina:

* Simulador Edsim (https://edsim51.com/)
* Caso seja aluno, O software gravador via arduino.

---

Fontes: Aulas e slides do professor Emerson

