# Projeto 4 da matéria de Projetos de Sistemas Embarcados

> Conjunto de aplicações gráficas e jogos desenvolvidos em linguagem C para a arquitetura Intel 8051 (placa do professor Emerson Pedrino). O repositório reúne três projetos do Keil uVision divididos em subpastas, focando no controle de displays LCD de caracteres e displays gráficos (GLCD).

---

## Índice

* [Estrutura do Repositório](#estrutura-do-repositório)
* [1. Rex no LCD (rex-lcd)](#1-rex-no-lcd-rex-lcd)
* [2. Rex no GLCD (rex-glcd)](#2-rex-no-glcd-rex-glcd)
* [3. Stock Car no GLCD (stock car)](#3-stock-car-no-glcd-stock-car)
* [Pré-requisitos e Compilação](#pré-requisitos-e-compilação)
* [Fontes](#fontes)

---

## Estrutura do Repositório

O projeto está organizado nas seguintes pastas de desenvolvimento:
* **rex-lcd/**: Jogo do dinossauro adaptado para display LCD alfanumérico convencional.
* **rex-glcd/**: Jogo do dinossauro portado com gráficos completos para display LCD Gráfico.
* **stock car/**: Protótipo de jogo de corrida de carros em display gráfico (Trabalho em andamento).

---

## 1. Rex no LCD (rex-lcd)

Implementação do clássico jogo do dinossauro corredor adaptada para as limitações de um display LCD alfanumérico comum (16x2) controlado via portas `P1` e `P90` (mapeada como porta de dados do LCD).

### Detalhes Técnicos e Rotinas
* **Caracteres Customizados:** Devido à limitação de caracteres de texto do display, foram criados mapas de bits gravados na CGRAM (`store_custom_char`) para desenhar os quadros de animação do Rex (`rex`, `rex2`) e os obstáculos.
* **Lógica de Jogo:** Arrays dinâmicos representam o chão (`floor_arr`) e o céu (`sky`) para deslocar os obstáculos lateralmente usando shifts (`left_shift`).
* **Mecânicas:** O sistema monitora pinos da porta P0 para registrar a ação de pulo (`jump`), realiza detecção de colisões baseada em matriz e gerencia uma pontuação com semente pseudo-aleatória variável a cada partida.

---

## 2. Rex no GLCD (rex-glcd)

Versão do jogo adaptada para um Display LCD Gráfico (GLCD) de 128x64 pixels, permitindo maior fidelidade visual e movimentação mais fluida dos elementos na tela.

### Detalhes Técnicos e Rotinas
* **Frame Buffer (`line_buf`):** Utiliza um buffer de linha na memória RAM para realizar a manipulação de pixels (`set_px`) de forma otimizada por operações de bitwise antes do envio dos bytes para o barramento do display (`P3`).
* **Interface de Controle:** Comunicação feita através dos pinos de controle RS (`P2.0`), RW (`P2.1`), EN (`P2.2`), PSB (`P2.3`) e RST (`P2.5`).
* **Geração de Obstáculos:** Utiliza uma função customizada de geração de números pseudo-aleatórios (`my_rand`) alimentada por operações de deslocamento de bits (Xorshift) para definir o surgimento de múltiplos obstáculos simultâneos mapeados por uma estrutura de dados dedicada.

---

## 3. Stock Car no GLCD (stock car)

Status do subprojeto: **Não finalizado / Em desenvolvimento**

Protótipo de um jogo de corrida vertical utilizando o Display LCD Gráfico (GLCD) de 128x64. O código base estabelece a estrutura física da pista e o comportamento dos veículos.

### Elementos Implementados
* **Pista Dinâmica:** Estrutura de pista renderizada via código através de um buffer circular simulado (`topo_pista`), gerando as bordas e as faixas centrais tracejadas (`dash_pattern`).
* **Controle do Jogador e Inimigo:** Variáveis globais para monitorar coordenadas X/Y do jogador e do veículo inimigo, além de variáveis estruturadas para o cálculo de pontuação (`points`) e controle do ciclo de renderização dia/noite (`night`).
* **Scroll Híbrido:** Esboço de rotinas gráficas baseadas em comandos de escrita do LCD (`WriteCmdLcd` e `WriteDataLcd`) para atualizar os sprites dos carros em sincronia com o avanço da pista.

---

## Pré-requisitos e Compilação

Cada subpasta contém um ambiente de projeto independente que pode ser aberto e modificado.

1. **IDE de Desenvolvimento:** [Keil uVision](https://www.keil.com/) configurado para o compilador C51 (alvo Intel 8051).
2. **Simulação:** [Simulador Edsim51](https://edsim51.com/) configurado com os respectivos periféricos (LCD) e Simulador Proteus para os glcd conforme esquemático
3. **Hardware Real:** Caso os testes sejam feitos fisicamente, utilize o software gravador via Arduino próprio da disciplina.

---

## Fontes

* Aulas e slides do professor Emerson Pedrino.
* Este README foi feito com auxilio de IA