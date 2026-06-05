#include <reg51.h>

// Definições de pinos
sbit RS = P1^3;
sbit EN = P1^2;
sbit Display = P0^7;

// Teclado - Linhas
sbit Row0 = P0^3;
sbit Row1 = P0^2;
sbit Row2 = P0^1;
sbit Row3 = P0^0;

// Teclado - Colunas
sbit Col0 = P0^4;
sbit Col1 = P0^5;
sbit Col2 = P0^6;

// Flags e variáveis globais
bit KeyPressFlag = 0;
bit CorrectFlag = 0;
unsigned char KeyBuffer[4];
unsigned char KeyIndex = 4;  // Contador de dígitos ao contrário (r0)
unsigned char *BufferPtr;    // Ponteiro para o buffer (r1)

// Tabela da senha
unsigned char code Password[] = {'4','0','4','6'};

// Strings
unsigned char code intro[] = "Senha:";
unsigned char code texto1[] = "Access Granted";
unsigned char code texto2[] = "ERROU KKK";

// Protótipos de funções
void delay(void);
void delay_longo(void);
void clk(void);
void FuncS(void);
void DispC(void);
void EntryMode(void);
void SendChar(unsigned char c);
void PrintString(unsigned char *str);
void TrocaLinha2(void);
unsigned char ScanKey(void);
void ESPSOL(void);
void Initial(void);
void Compare(void);

// Função de delay curto
void delay(void) {
    unsigned char r7 = 0x10;
    while(r7-- > 0);
}

// Função de delay longo
void delay_longo(void) {
    unsigned char r7, B;
    r7 = 0xFF;
    B = 0xFF;
    while(B-- > 0) {
        r7 = 0xFF;
        while(r7-- > 0);
    }
}

// Pulso do clock (Enable)
void clk(void) {
    EN = 1;
    EN = 0;
}

// Configuração inicial do LCD - Function Set
void FuncS(void) {
    RS = 0;
    P1 = (P1 & 0x0F) | 0x20;  // DAT = 02h
    clk();
    delay();
    
    clk();
    delay();
    
    P1 = (P1 & 0x0F) | 0x80;  // DAT = 08h
    clk();
    delay();
}

// Display Control
void DispC(void) {
    RS = 0;
    P1 = (P1 & 0x0F) | 0x00;  // DAT = 00h
    clk();
    P1 = (P1 & 0x0F) | 0xF0;  // DAT = F0h
    clk();
    delay();
}

// Entry Mode
void EntryMode(void) {
    RS = 0;
    P1 = (P1 & 0x0F) | 0x00;  // DAT = 00h
    clk();
    P1 = (P1 & 0x0F) | 0x60;  // DAT = 06h
    clk();
    delay();
}

// Envia caractere para o LCD
void SendChar(unsigned char c) {
    unsigned char temp;
    
    // Envia nibble alto
    temp = P1 & 0x0F;
    temp |= (c & 0xF0);
    P1 = temp;
    clk();
    
    // Envia nibble baixo
    temp = P1 & 0x0F;
    temp |= ((c << 4) & 0xF0);
    P1 = temp;
    clk();
    delay();
}

// Imprime string
void PrintString(unsigned char *str) {
    unsigned char c;
    while(c = *str++) {
        RS = 1;
        SendChar(c);
    }
}

// Pula linha no LCD
void TrocaLinha2(void) {
    RS = 0;
    P1 = (P1 & 0x0F) | 0xC0;  // DAT = C0h
    clk();
    delay();
}

// Delay para debounce do teclado
void ESPSOL(void) {
    // Espera soltar a tecla
    while((P0 & 0x70) != 0x70);
    
    // Timer 0 para delay
    TMOD = 0x01;
    TH0 = 0x8A;
    TL0 = 0xCF;
    TR0 = 1;
    while(!TF0);
    TR0 = 0;
    TF0 = 0;
}

// Escaneia o teclado
unsigned char ScanKey(void) {
    unsigned char key = 0;
    
    while(1) {
        KeyPressFlag = 0;
        
        // Verifica linha 0
        Row0 = 0;
        Row1 = 1;
        Row2 = 1;
        Row3 = 1;
        if(!Col0) { key = '3'; KeyPressFlag = 1; }
        else if(!Col1) { key = '2'; KeyPressFlag = 1; }
        else if(!Col2) { key = '1'; KeyPressFlag = 1; }
        Row0 = 1;
        if(KeyPressFlag) break;
        
        // Verifica linha 1
        Row0 = 1;
        Row1 = 0;
        Row2 = 1;
        Row3 = 1;
        if(!Col0) { key = '6'; KeyPressFlag = 1; }
        else if(!Col1) { key = '5'; KeyPressFlag = 1; }
        else if(!Col2) { key = '4'; KeyPressFlag = 1; }
        Row1 = 1;
        if(KeyPressFlag) break;
        
        // Verifica linha 2
        Row0 = 1;
        Row1 = 1;
        Row2 = 0;
        Row3 = 1;
        if(!Col0) { key = '9'; KeyPressFlag = 1; }
        else if(!Col1) { key = '8'; KeyPressFlag = 1; }
        else if(!Col2) { key = '7'; KeyPressFlag = 1; }
        Row2 = 1;
        if(KeyPressFlag) break;
        
        // Verifica linha 3
        Row0 = 1;
        Row1 = 1;
        Row2 = 1;
        Row3 = 0;
        if(!Col0) { key = '#'; KeyPressFlag = 1; }
        else if(!Col1) { key = '0'; KeyPressFlag = 1; }
        else if(!Col2) { key = '*'; KeyPressFlag = 1; }
        Row3 = 1;
        if(KeyPressFlag) break;
    }
    
    // Debounce e delay
    ESPSOL();
    
    // Armazena no buffer se ainda houver espaço
    if(KeyIndex > 0) {
        *BufferPtr = key;
        BufferPtr++;
        KeyIndex--;
    }
    
    return key;
}

// Compara a senha digitada com a senha correta
void Compare(void) {
    unsigned char i;
    unsigned char *p = (unsigned char *)0x30;  // Buffer na RAM
    
    for(i = 0; i < 4; i++) {
        if(*p != Password[i]) {
            // Senha incorreta
            TrocaLinha2();
            PrintString(texto2);
            
            // Reinicia variáveis
            KeyIndex = 4;
            BufferPtr = (unsigned char *)0x30;
            
            // Delay longo
            delay_longo();
            delay_longo();
            delay_longo();
            delay_longo();
            delay_longo();
            return;
        }
        p++;
    }
    
    // Senha correta
    TrocaLinha2();
    PrintString(texto1);
    while(1);  // Loop infinito
}

// Inicialização
void Initial(void) {
    PrintString(intro);
}

// Função principal
void main(void) {
    unsigned char key;
    
    Display = 0;  // Desliga display (P0.7)
    KeyIndex = 4;  // Contador de dígitos
    CorrectFlag = 0;
    BufferPtr = (unsigned char *)0x30;  // Ponteiro para RAM
    
    // Inicialização do LCD
    FuncS();
    DispC();
    EntryMode();
    Initial();
    
    // Loop principal
    while(1) {
        key = ScanKey();
        RS = 1;
        SendChar(key);
        
        if(key == '#') break;
    }
    
    // Fim do programa
    while(1);
}
