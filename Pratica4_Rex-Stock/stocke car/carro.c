#include <reg52.h>
#define BOUNDING 8
#define PATH_SIZE 16
// ==========================================================
// DEFINIÇÕES DE PINOS
// ==========================================================
#define GlcdDataBus P3
sbit RS  = P2^0;
sbit RW  = P2^1;
sbit EN  = P2^2;
sbit PSB = P2^3;
sbit RST = P2^5;

// ==========================================================
// PROTÓTIPOS
// ==========================================================
void DelayUs(unsigned int us);
void DelayMs(unsigned int ms);
void WriteCmdLcd(unsigned char cmd);
void WriteDataLcd(unsigned char dat);
void GLCD_Init(void);
void Draw_Track(void);
void Draw_Sprite(unsigned char screen_y, unsigned char screen_x, unsigned char scroll_offset);
void Hybrid_Scroll_Two_Sprites(void);

// ==========================================================
// GLOBAIS
// ==========================================================

char speed = 1;
char x_player = 6;
char y_ player = 54; // fixo

char x_enemy = 0;
char y_enemy = 0;
int points = 0;
char night = 0; // False
unsigned char topo_pista = 0; // Pro buffer circular

char path[64];

// Carro sprite (16x16)  ( Só metade para eu espelhar e economizar memoria)
    unsigned char code car_sprite[] = {
        0x01,   // Linha 0   00000001    // Espelhado fica..
        0x03,   // Linha 1:  00000111
        0x01,  // Linha 2:   00000001
        0x1B,   // Linha 3:  00011011
        0x1F,   // Linha 4:  00011111
        0x1B,   // Linha 5:  00011011
        0x03,   // Linha 6:  00000111
        0x11,   // Linha 7:  00001111
        0x11,   // Linha 8:  00001111
        0x03,   // Linha 9:  00000111
        0x02,   // Linha 10: 00000011
        0x1B,   // Linha 11: 00011011
        0x1F,   // Linha 12: 00011111
        0x19,   // Linha 13: 00011001
				0x0F,   // Linha 14: 00001111
				0x1F,   // Linha 15: 00011111
    };

unsigned char reverse(unsigned char b) {  // Como descobrem essas coisas?
    return (b * 0x0202020202ULL & 0x010884422010ULL) % 0x3ff;
}

/* 8 bit version 
unsigned char reverse(unsigned char b) {
	  return ((b * 0x0802LU & 0x22110LU) | (b * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16; 
*/

// https://graphics.stanford.edu/~seander/bithacks.html#BitReverseObvious

// ==========================================================
// DELAYS
// ==========================================================
void DelayUs(unsigned int us) {
    unsigned int i;
    while(us--) {
        for(i = 0; i < 2; i++);
    }
}

void DelayMs(unsigned int ms) {
    unsigned int i, j;
    for(j = 0; j < ms; j++) {
        for(i = 0; i < 114; i++);
    }
}

// ==========================================================
// COMUNICAÇÃO RÁPIDA
// ==========================================================
void WriteCmdLcd(unsigned char cmd) {
    RS = 0;
    RW = 0;
    GlcdDataBus = cmd;
    EN = 1;
    DelayUs(2);
    EN = 0;
    DelayUs(30);
}

void WriteDataLcd(unsigned char dat) {
    RS = 1;
    RW = 0;
    GlcdDataBus = dat;
    EN = 1;
    DelayUs(2);
    EN = 0;
    DelayUs(10);
}

// ==========================================================
// INICIALIZAÇÃO
// ==========================================================
void GLCD_Init() {
    PSB = 1;
    RST = 0; DelayMs(50);
    RST = 1; DelayMs(50);
    
    WriteCmdLcd(0x30); DelayMs(5);
    WriteCmdLcd(0x30); DelayMs(5);
    WriteCmdLcd(0x0C); DelayMs(2);
    WriteCmdLcd(0x01); DelayMs(20); // Clear
    WriteCmdLcd(0x06); DelayMs(2);
}

// ==========================================================
// DESENHA PISTA NA GDRAM
// ==========================================================
// Como eu guardo as informaçoes da pista de maneira econômica? 
// A distância da pista de uma ponta a outra é sempre a mesma.  Para simplificar, posso ter um
// Vetor que eu só dou shift a cada certo tempos> Isso conserva a distância da pista
// Problema: O estado mais antigo da pista está sempre embaixo. Ler do glcd é muito ruim, inviável.
// 3 vetores.... ? Um pra replicar
// Guarda só quando a pista começa. 64 linhas mesmo :P
// Dai eu somo + 8 por exemplo, pra fazer o resto da pista

// Da pra usar um truque aqui pra desenhar um caracter / ou |. Só verificar o proximo. Se for diferente faz / ou \ (verifica maior ou menor)
// Path[64] = { 20,19,18,18,18,18,17,16,16,0,0,0,0,0,0,0}  -> Indica qual bit começa a estrada (lado esquerdo)
//            ^topo
//ponteiro da pilha vai incrementando, modulo 64
// o GLCD move pra direita a cada caracter. Isso é bom.

char get_x_start_path(char y){
	valor = path[(y + topo)%64];
	return valor;
}

void Draw_Track() {
    unsigned char y;
    unsigned char x;
		unsigned char pos = 0;
		unsigned char pos_flag = 0;
		unsigned char actual_y = 0;
		unsigned char local_x = 0;
		unsigned char nibble_1 = 0;
		unsigned char nibble_2 = 0;
    WriteCmdLcd(0x34);
    WriteCmdLcd(0x36);
 
// path[topo%64]
// O valor no vetor indica onde começa a estrada.
// Primeiro nibble
// Segundo nibble é... o atual + 8? (largura da pista. Pode ser 16.. acho que pode até ficar melor

    for(y = 0; y < 32; y++) {
			for(x = 0; x < 256; x=x+16){ 
				
       if(x == 0){
				WriteCmdLcd(0x80 | y);
        WriteCmdLcd(0x80);
			 }
			 if(x == 128){
				 WriteCmdLcd(0x80 | y);
         WriteCmdLcd(0x88);
			 }
			 actual_y = (x < 128) ? y : (y + 32);
			 pos = get_x_start(y);
			 local_x = x % 128;  // Essas divisões são bem confusas
			 nibble_1 = 0;
			 nibble_2 = 0;
			 
			 if(pos >  x){
				 WriteDataLcd(0x00); WriteDataLcd(0x00);
			 }
			 else if( pos < x && !pos_flag ){ 
				 // nibble_1 = x_atual + (pos%8);
				 // nibble_2 = nibble_1 + PATH_SIZE;
				 // WriteDataLcd(nibble1); WriteDataLcd(nibble2);
				 pos_flag == 1;
			 }
			 else{
				 WriteDataLcd(0x00); WriteDataLcd(0x00);
			 }
    }
	}
    
    WriteCmdLcd(0x30);
		topo_pista = (topo_pista+63)%64;
}

// ==========================================================
// SPRITE DO CARRO (16x16 pixels)
// ==========================================================
void Draw_Sprite(unsigned char screen_y, unsigned char screen_x, unsigned char scroll_offset) {
    unsigned char gdram_y = (screen_y + scroll_offset) & 0x3F;
    
    unsigned char i;
    unsigned char byte_x;
    
    WriteCmdLcd(0x34);
    WriteCmdLcd(0x36);
    
    for(i = 0; i < 16; i++) {
        unsigned char current_y = (gdram_y + i) & 0x3F;
        
        // Desenha na metade esquerda (X=0-7)
        if(screen_x < 8) {
            byte_x = screen_x;
            WriteCmdLcd(0x80 | current_y);
            WriteCmdLcd(0x80 | byte_x);
            WriteDataLcd(car_sprite[i*2]);
            WriteDataLcd(car_sprite[i*2 + 1]);
        }
        
        // Desenha na metade direita (X=8-15)
        if(screen_x >= 8) {
            byte_x = screen_x & 0x07;  // Converte para 0-7
            WriteCmdLcd(0x80 | current_y);
            WriteCmdLcd(0x88 | byte_x);  // 0x88 = 8 (metade direita)
            WriteDataLcd(car_sprite[i*2]);
            WriteDataLcd(car_sprite[i*2 + 1]);
        }
    }
    
    WriteCmdLcd(0x30);
}

// ==========================================================
// Colisão
// ==========================================================

// Vou guardar o y do centro do carro, e o y do centro do carrro_inimigo. Se eu quiser saber se ouve colisão
// Eu comparo y do centro + 8, y do centro - 8 (talvez precise ajustar pra ficar mais preciso
// Mesma coisa em X

// ==========================================================
// LIMPA SPRITE (redesenha pista por baixo)
// ==========================================================

void Clear_Sprite(unsigned char screen_y, unsigned char screen_x, unsigned char scroll_offset) {
    unsigned char gdram_y = (screen_y + scroll_offset) & 0x3F;
    unsigned char i, byte_x;
    
    WriteCmdLcd(0x34);
    WriteCmdLcd(0x36);
    
    for(i = 0; i < 16; i++) {
        unsigned char current_y = (gdram_y + i) & 0x3F;
        unsigned char dash_pattern = ((current_y % 8) < 4) ? 0xFF : 0x00;
        
        // Metade esquerda
        if(screen_x < 8) {
            byte_x = screen_x;
            WriteCmdLcd(0x80 | current_y);
            WriteCmdLcd(0x80 | byte_x);
            
            // Redesenha a pista normal
            if(byte_x == 0) {
                WriteDataLcd(0xFF); WriteDataLcd(0x00);  // Borda
            } else if(byte_x >= 3 && byte_x <= 4) {
                WriteDataLcd(dash_pattern); WriteDataLcd(dash_pattern);  // Tracejado
            } else {
                WriteDataLcd(0x00); WriteDataLcd(0x00);  // Vazio
            }
        }
        
        // Metade direita
        if(screen_x >= 8) {
            byte_x = screen_x & 0x07;
            WriteCmdLcd(0x80 | current_y);
            WriteCmdLcd(0x88 | byte_x);
            
            if(byte_x == 0) {
                WriteDataLcd(0xFF); WriteDataLcd(0x00);  // Borda
            } else if(byte_x >= 3 && byte_x <= 4) {
                WriteDataLcd(dash_pattern); WriteDataLcd(dash_pattern);  // Tracejado
            } else {
                WriteDataLcd(0x00); WriteDataLcd(0x00);  // Vazio
            }
        }
    }
    
    WriteCmdLcd(0x30);
}


// ==========================================================
// INICIALIZAÇÃO
// ==========================================================
void GLCD_Init() {
		// Idealmente, modo de 8 bits paralelos
    PSB = 1;
    RST = 0; DelayMs(50);
    RST = 1; DelayMs(50);
    
    WriteCmdLcd(0x30); DelayMs(5);
    WriteCmdLcd(0x30); DelayMs(5);
    WriteCmdLcd(0x0C); DelayMs(2);
    WriteCmdLcd(0x01); DelayMs(20); // Clear
    WriteCmdLcd(0x06); DelayMs(2);
}

// ==========================================================
// DESENHA PISTA NA GDRAM																		
// ==========================================================

// GDRAM address is set by writing 2
// consecutive bytes for vertical address and horizontal address.

/*
1. Set vertical address(Y)for GDRAM
2. Set horizontal address(X)for GDRAM
3. Write D15?D8 to GDRAM char(first byte)
4. Write D7?D0 to GDRAM char(second byte)

Ou seja, preciso mandar nibbles de 1 byte.
*/ 

void Draw_Track() {
    unsigned char y;
    
    WriteCmdLcd(0x34);
    WriteCmdLcd(0x36);
    
    for(y = 0; y < 64; y++) {
			
        unsigned char dash_pattern = ((y % 8) < 4) ? 0xFF : 0x00;
        // Cada X representa um bloco de 16
			
        // Metade de cima
        WriteCmdLcd(0x80 | y); // set y
        WriteCmdLcd(0x80); // set x
        WriteDataLcd(0xFF); WriteDataLcd(0x00);  // X=0
        WriteDataLcd(0x00); WriteDataLcd(0x00);  // X=1
        WriteDataLcd(0x00); WriteDataLcd(0x00);  // X=2
        WriteDataLcd(dash_pattern); WriteDataLcd(dash_pattern);  // X=3
        WriteDataLcd(dash_pattern); WriteDataLcd(dash_pattern);  // X=4
        WriteDataLcd(0x00); WriteDataLcd(0x00);  // X=5
        WriteDataLcd(0x00); WriteDataLcd(0x00);  // X=6
        WriteDataLcd(0x00); WriteDataLcd(0xFF);  // X=7
        
        // Metade de baixo
        WriteCmdLcd(0x80 | y); // set y
        WriteCmdLcd(0x88); // set x
        WriteDataLcd(0xFF); WriteDataLcd(0x00);  // X=8
        WriteDataLcd(0x00); WriteDataLcd(0x00);  // X=9
        WriteDataLcd(0x00); WriteDataLcd(0x00);  // X=10
        WriteDataLcd(dash_pattern); WriteDataLcd(dash_pattern);  // X=11
        WriteDataLcd(dash_pattern); WriteDataLcd(dash_pattern);  // X=12
        WriteDataLcd(0x00); WriteDataLcd(0x00);  // X=13
        WriteDataLcd(0x00); WriteDataLcd(0x00);  // X=14
        WriteDataLcd(0x00); WriteDataLcd(0xFF);  // X=15
    }
    
    WriteCmdLcd(0x30);
}


// ==========================================================
// LIMPA SPRITE (redesenha pista por baixo)
// ==========================================================
void Clear_Sprite(unsigned char screen_y, unsigned char screen_x, unsigned char scroll_offset) {
    unsigned char gdram_y = (screen_y + scroll_offset) & 0x3F;
    unsigned char i, byte_x;
    
    WriteCmdLcd(0x34);
    WriteCmdLcd(0x36);
    
    for(i = 0; i < 16; i++) {
        unsigned char current_y = (gdram_y + i) & 0x3F;
        unsigned char dash_pattern = ((current_y % 8) < 4) ? 0xFF : 0x00;
        
        // Metade esquerda
        if(screen_x < 8) {
            byte_x = screen_x;
            WriteCmdLcd(0x80 | current_y);
            WriteCmdLcd(0x80 | byte_x);
            
            // Redesenha a pista normal
            if(byte_x == 0) {
                WriteDataLcd(0xFF); WriteDataLcd(0x00);  // Borda
            } else if(byte_x >= 3 && byte_x <= 4) {
                WriteDataLcd(dash_pattern); WriteDataLcd(dash_pattern);  // Tracejado
            } else {
                WriteDataLcd(0x00); WriteDataLcd(0x00);  // Vazio
            }
        }
        
        // Metade direita
        if(screen_x >= 8) {
            byte_x = screen_x & 0x07;
            WriteCmdLcd(0x80 | current_y);
            WriteCmdLcd(0x88 | byte_x);
            
            if(byte_x == 0) {
                WriteDataLcd(0xFF); WriteDataLcd(0x00);  // Borda
            } else if(byte_x >= 3 && byte_x <= 4) {
                WriteDataLcd(dash_pattern); WriteDataLcd(dash_pattern);  // Tracejado
            } else {
                WriteDataLcd(0x00); WriteDataLcd(0x00);  // Vazio
            }
        }
    }
    
    WriteCmdLcd(0x30);
}

void Init_Path(){
	char i;
	for(i = 0; i < 64; i++){
		path[i] = 6;   //reta
	}
}

// ==========================================================
// MAIN
// ==========================================================
void main() {
    GLCD_Init();
    DelayMs(100);
    
    Draw_Track();
    DelayMs(500);
    
		Init_Path();
    // Escolha o modo de jogo:
    Hybrid_Scroll_Two_Sprites();        // Inimigo desce contínuo

} 



