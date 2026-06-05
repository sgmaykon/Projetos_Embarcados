 org 0h ; Rotina do Blinker…
 back: clr p1.0 ; Reinicia a rotina | Desliga o led
 acall delay ; vai pra rotina de delay
 setb p1.0 ; liga o led
 acall delay ; Delay de novo
 sjmp back ; Volta pra rotina
 delay: mov r0,#0ffh  ;; Dois laços aninhados
 again: mov r1,#0ffh ;; Valor 255 * 255 microsegundos
 here: djnz r1,here
 djnz r0,again
 ret
 end