#include <reg52.h>

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
    WriteCmdLcd(0x01); DelayMs(20);
    WriteCmdLcd(0x06); DelayMs(2);
}

// ==========================================================
// DESENHA PISTA NA GDRAM
// ==========================================================
void Draw_Track() {
    unsigned char y;
    
    WriteCmdLcd(0x34);
    WriteCmdLcd(0x36);
    
    for(y = 0; y < 64; y++) {
        unsigned char dash_pattern = ((y % 8) < 4) ? 0xFF : 0x00;
        
        // Metade esquerda
        WriteCmdLcd(0x80 | y);
        WriteCmdLcd(0x80);
        WriteDataLcd(0xFF); WriteDataLcd(0x00);  // X=0
        WriteDataLcd(0x00); WriteDataLcd(0x00);  // X=1
        WriteDataLcd(0x00); WriteDataLcd(0x00);  // X=2
        WriteDataLcd(dash_pattern); WriteDataLcd(dash_pattern);  // X=3
        WriteDataLcd(dash_pattern); WriteDataLcd(dash_pattern);  // X=4
        WriteDataLcd(0x00); WriteDataLcd(0x00);  // X=5
        WriteDataLcd(0x00); WriteDataLcd(0x00);  // X=6
        WriteDataLcd(0x00); WriteDataLcd(0xFF);  // X=7
        
        // Metade direita
        WriteCmdLcd(0x80 | y);
        WriteCmdLcd(0x88);
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
// SPRITE DO CARRO (16x16 pixels)
// ==========================================================
void Draw_Sprite(unsigned char screen_y, unsigned char screen_x, unsigned char scroll_offset) {
    unsigned char gdram_y = (screen_y + scroll_offset) & 0x3F;
    
    // Carro esportivo (16x16)
    unsigned char code car_sprite[] = {
        0x00, 0x00,  // Linha 0
        0x07, 0xE0,  // Linha 1:    ******
        0x0F, 0xF0,  // Linha 2:   ********
        0x19, 0x98,  // Linha 3:  **  **  **
        0x10, 0x08,  // Linha 4:  *        *
        0x13, 0xC8,  // Linha 5:  *  ****  *
        0x1F, 0xF8,  // Linha 6:   **********
        0x1F, 0xF8,  // Linha 7:   **********
        0x1F, 0xF8,  // Linha 8:   **********
        0x0F, 0xF0,  // Linha 9:    ********
        0x09, 0x90,  // Linha 10:   *  *  *
        0x09, 0x90,  // Linha 11:   *  *  *
        0x09, 0x90,  // Linha 12:   *  *  *
        0x06, 0x60,  // Linha 13:    **  **
        0x00, 0x00,  // Linha 14
        0x00, 0x00   // Linha 15
    };
    
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
// SCROLL HÍBRIDO COM DOIS SPRITES
// ==========================================================
void Hybrid_Scroll_Two_Sprites(void) {
    unsigned char scroll_addr = 0;
    unsigned char enemy_y = 0;  // Inimigo começa no topo
    
    // Jogador: FIXO na tela (lado direito, meio)
    unsigned char player_screen_y = 12;   // Posição Y fixa na tela
    unsigned char player_screen_x = 10;   // Lado direito (X=10)
    
    // Inimigo: MÓVEL descendo (lado esquerdo)
    unsigned char enemy_screen_x = 1;     // Lado esquerdo (X=1)
    unsigned char enemy_screen_y = 0;     // Posição Y atual na tela
    unsigned char enemy_prev_y = 0;       // Posição Y anterior
    
    WriteCmdLcd(0x34);
    WriteCmdLcd(0x36);
    WriteCmdLcd(0x03);  // Habilita scroll
    
    // Desenha sprites iniciais
    Draw_Sprite(player_screen_y, player_screen_x, scroll_addr);
    Draw_Sprite(enemy_screen_y, enemy_screen_x, scroll_addr);
    
    while(1) {
        // PASSO 1: Scroll (instantâneo)
        scroll_addr++;
        WriteCmdLcd(0x40 | scroll_addr);
        
        // PASSO 2: Pequena pausa para estabilizar
        DelayUs(100);
        
        // PASSO 3: Jogador FIXO - Limpa e redesenha na mesma posição
        Clear_Sprite(player_screen_y, player_screen_x, (scroll_addr - 1) & 0x3F);
        Draw_Sprite(player_screen_y, player_screen_x, scroll_addr);
        
        // PASSO 4: Inimigo MÓVEL - Desce um pixel
        enemy_prev_y = enemy_screen_y;
        enemy_screen_y++;
        
        // Se inimigo saiu da tela, volta ao topo
        if(enemy_screen_y >= 32) {
            enemy_screen_y = 0;
        }
        
        // Limpa inimigo na posição antiga
        Clear_Sprite(enemy_prev_y, enemy_screen_x, scroll_addr);
        
        // Desenha inimigo na nova posição
        Draw_Sprite(enemy_screen_y, enemy_screen_x, scroll_addr);
        
        // PASSO 5: Delay para controle de velocidade
        DelayMs(80);  // Velocidade do jogo (scroll + movimento inimigo)
    }
}

// ==========================================================
// VERSÃO COM INIMIGO MAIS RÁPIDO/DEVAGAR
// ==========================================================
void Hybrid_Scroll_VariableSpeed(void) {
    unsigned char scroll_addr = 0;
    unsigned char frame_count = 0;
    
    // Jogador fixo (direita)
    unsigned char player_y = 12, player_x = 10;
    
    // Inimigo móvel (esquerda)
    unsigned char enemy_y = 0, enemy_x = 1;
    unsigned char enemy_prev_y = 0;
    unsigned char enemy_speed = 2;  // Move a cada 2 frames
    
    WriteCmdLcd(0x34);
    WriteCmdLcd(0x36);
    WriteCmdLcd(0x03);
    
    Draw_Sprite(player_y, player_x, scroll_addr);
    Draw_Sprite(enemy_y, enemy_x, scroll_addr);
    
    while(1) {
        // Scroll sempre acontece
        scroll_addr++;
        WriteCmdLcd(0x40 | scroll_addr);
        DelayUs(100);
        
        // Jogador fixo - sempre redesenha
        Clear_Sprite(player_y, player_x, (scroll_addr - 1) & 0x3F);
        Draw_Sprite(player_y, player_x, scroll_addr);
        
        // Inimigo move a cada 'enemy_speed' frames
        if((frame_count % enemy_speed) == 0) {
            enemy_prev_y = enemy_y;
            enemy_y++;
            
            if(enemy_y >= 32) {
                enemy_y = 0;
            }
            
            Clear_Sprite(enemy_prev_y, enemy_x, scroll_addr);
        }
        
        // Sempre redesenha inimigo na posição atual
        Draw_Sprite(enemy_y, enemy_x, scroll_addr);
        
        frame_count++;
        DelayMs(50);
    }
}

// ==========================================================
// VERSÃO COM INIMIGO OSCILANDO (sobe e desce)
// ==========================================================
void Hybrid_Scroll_OscillatingEnemy(void) {
    unsigned char scroll_addr = 0;
    char enemy_direction = 1;  // 1 = descendo, -1 = subindo
    
    // Jogador fixo (direita)
    unsigned char player_y = 12, player_x = 10;
    
    // Inimigo oscilante (esquerda)
    unsigned char enemy_y = 0, enemy_x = 1;
    unsigned char enemy_prev_y = 0;
    
    WriteCmdLcd(0x34);
    WriteCmdLcd(0x36);
    WriteCmdLcd(0x03);
    
    Draw_Sprite(player_y, player_x, scroll_addr);
    Draw_Sprite(enemy_y, enemy_x, scroll_addr);
    
    while(1) {
        scroll_addr++;
        WriteCmdLcd(0x40 | scroll_addr);
        DelayUs(100);
        
        // Jogador fixo
        Clear_Sprite(player_y, player_x, (scroll_addr - 1) & 0x3F);
        Draw_Sprite(player_y, player_x, scroll_addr);
        
        // Inimigo oscilante
        enemy_prev_y = enemy_y;
        
        if(enemy_direction == 1) {  // Descendo
            enemy_y++;
            if(enemy_y >= 28) {  // Não deixa sair totalmente
                enemy_direction = -1;  // Começa subir
            }
        } else {  // Subindo
            enemy_y--;
            if(enemy_y == 0) {
                enemy_direction = 1;  // Começa descer
            }
        }
        
        Clear_Sprite(enemy_prev_y, enemy_x, scroll_addr);
        Draw_Sprite(enemy_y, enemy_x, scroll_addr);
        
        DelayMs(80);
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
    
    // Escolha o modo de jogo:
    Hybrid_Scroll_Two_Sprites();        // Inimigo desce contínuo
    // Hybrid_Scroll_VariableSpeed();   // Inimigo com velocidade variável
    // Hybrid_Scroll_OscillatingEnemy(); // Inimigo sobe e desce
}




#include <reg52.h>

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
void Clear_Sprite(unsigned char screen_y, unsigned char screen_x, unsigned char scroll_offset);
void Test_Scroll_Direction(void);
void Scroll_With_One_Sprite(void);

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
// COMUNICAÇÃO
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
    WriteCmdLcd(0x01); DelayMs(20);
    WriteCmdLcd(0x06); DelayMs(2);
}

// ==========================================================
// DESENHA PISTA
// ==========================================================
void Draw_Track() {
    unsigned char y;
    
    WriteCmdLcd(0x34);
    WriteCmdLcd(0x36);
    
    for(y = 0; y < 64; y++) {
        unsigned char dash_pattern = ((y % 8) < 4) ? 0xFF : 0x00;
        
        // Metade esquerda
        WriteCmdLcd(0x80 | y);
        WriteCmdLcd(0x80);
        WriteDataLcd(0xFF); WriteDataLcd(0x00);
        WriteDataLcd(0x00); WriteDataLcd(0x00);
        WriteDataLcd(0x00); WriteDataLcd(0x00);
        WriteDataLcd(dash_pattern); WriteDataLcd(dash_pattern);
        WriteDataLcd(dash_pattern); WriteDataLcd(dash_pattern);
        WriteDataLcd(0x00); WriteDataLcd(0x00);
        WriteDataLcd(0x00); WriteDataLcd(0x00);
        WriteDataLcd(0x00); WriteDataLcd(0xFF);
        
        // Metade direita
        WriteCmdLcd(0x80 | y);
        WriteCmdLcd(0x88);
        WriteDataLcd(0xFF); WriteDataLcd(0x00);
        WriteDataLcd(0x00); WriteDataLcd(0x00);
        WriteDataLcd(0x00); WriteDataLcd(0x00);
        WriteDataLcd(dash_pattern); WriteDataLcd(dash_pattern);
        WriteDataLcd(dash_pattern); WriteDataLcd(dash_pattern);
        WriteDataLcd(0x00); WriteDataLcd(0x00);
        WriteDataLcd(0x00); WriteDataLcd(0x00);
        WriteDataLcd(0x00); WriteDataLcd(0xFF);
    }
    
    WriteCmdLcd(0x30);
}

// ==========================================================
// SPRITE SIMPLES (SEM limpeza complexa)
// ==========================================================
void Draw_Sprite(unsigned char screen_y, unsigned char screen_x, unsigned char scroll_offset) {
    // CORREÇÃO: scroll_offset vai de 0-63, mas scroll register usa 0-31
    // A tela mostra GDRAM[scroll_offset] até GDRAM[scroll_offset+31]
    
    unsigned char gdram_y = (screen_y + scroll_offset) & 0x3F;  // 0-63
    
    unsigned char code car_sprite[] = {
        0x00, 0x00, 0x07, 0xE0, 0x0F, 0xF0, 0x19, 0x98,
        0x10, 0x08, 0x13, 0xC8, 0x1F, 0xF8, 0x1F, 0xF8,
        0x1F, 0xF8, 0x0F, 0xF0, 0x09, 0x90, 0x09, 0x90,
        0x09, 0x90, 0x06, 0x60, 0x00, 0x00, 0x00, 0x00
    };
    
    unsigned char i;
    
    WriteCmdLcd(0x34);
    WriteCmdLcd(0x36);
    
    for(i = 0; i < 16; i++) {
        unsigned char current_y = (gdram_y + i) & 0x3F;
        
        // Metade esquerda
        if(screen_x < 8) {
            WriteCmdLcd(0x80 | current_y);
            WriteCmdLcd(0x80 | screen_x);
            WriteDataLcd(car_sprite[i*2]);
            WriteDataLcd(car_sprite[i*2 + 1]);
        }
        
        // Metade direita
        if(screen_x >= 8) {
            WriteCmdLcd(0x80 | current_y);
            WriteCmdLcd(0x88 | (screen_x & 0x07));
            WriteDataLcd(car_sprite[i*2]);
            WriteDataLcd(car_sprite[i*2 + 1]);
        }
    }
    
    WriteCmdLcd(0x30);
}

// ==========================================================
// LIMPA SPRITE (versão simplificada)
// ==========================================================
void Clear_Sprite(unsigned char screen_y, unsigned char screen_x, unsigned char scroll_offset) {
    unsigned char gdram_y = (screen_y + scroll_offset) & 0x3F;
    unsigned char i;
    
    WriteCmdLcd(0x34);
    WriteCmdLcd(0x36);
    
    for(i = 0; i < 16; i++) {
        unsigned char current_y = (gdram_y + i) & 0x3F;
        unsigned char dash_pattern = ((current_y % 8) < 4) ? 0xFF : 0x00;
        
        // Metade esquerda
        if(screen_x < 8) {
            WriteCmdLcd(0x80 | current_y);
            WriteCmdLcd(0x80 | screen_x);
            
            // Redesenha a pista baseado na posição X
            if(screen_x == 0 || screen_x == 8) {
                WriteDataLcd(0xFF); WriteDataLcd(0x00);  // Borda esquerda
            } else if(screen_x == 7 || screen_x == 15) {
                WriteDataLcd(0x00); WriteDataLcd(0xFF);  // Borda direita
            } else if((screen_x >= 3 && screen_x <= 4) || (screen_x >= 11 && screen_x <= 12)) {
                WriteDataLcd(dash_pattern); WriteDataLcd(dash_pattern);  // Tracejado
            } else {
                WriteDataLcd(0x00); WriteDataLcd(0x00);  // Vazio
            }
        }
        
        // Metade direita
        if(screen_x >= 8) {
            WriteCmdLcd(0x80 | current_y);
            WriteCmdLcd(0x88 | (screen_x & 0x07));
            
            if(screen_x == 0 || screen_x == 8) {
                WriteDataLcd(0xFF); WriteDataLcd(0x00);
            } else if(screen_x == 7 || screen_x == 15) {
                WriteDataLcd(0x00); WriteDataLcd(0xFF);
            } else if((screen_x >= 3 && screen_x <= 4) || (screen_x >= 11 && screen_x <= 12)) {
                WriteDataLcd(dash_pattern); WriteDataLcd(dash_pattern);
            } else {
                WriteDataLcd(0x00); WriteDataLcd(0x00);
            }
        }
    }
    
    WriteCmdLcd(0x30);
}

// ==========================================================
// TESTE 1: SCROLL DIRECTION (sobe ou desce?)
// ==========================================================
void Test_Scroll_Direction(void) {
    unsigned char scroll_val = 0;
    
    WriteCmdLcd(0x34);
    WriteCmdLcd(0x36);
    WriteCmdLcd(0x03);  // Habilita scroll
    
    while(1) {
        // Testa valores 0-31
        WriteCmdLcd(0x40 | scroll_val);
        DelayMs(200);
        
        scroll_val++;
        if(scroll_val > 31) scroll_val = 0;
    }
}

// ==========================================================
// TESTE 2: UM SPRITE FIXO COM SCROLL
// ==========================================================
void Scroll_With_One_Sprite(void) {
    unsigned char scroll_offset = 0;   // Offset global (0-63)
    unsigned char scroll_reg;          // Valor para scroll register (0-31)
    
    // Sprite FIXO na tela
    unsigned char sprite_screen_y = 10;  // Posição Y na TELA
    unsigned char sprite_screen_x = 4;   // Posição X (lado esquerdo)
    
    WriteCmdLcd(0x34);
    WriteCmdLcd(0x36);
    WriteCmdLcd(0x03);  // Habilita scroll
    
    // Desenha sprite inicial
    Draw_Sprite(sprite_screen_y, sprite_screen_x, 0);
    
    while(1) {
        // Atualiza offset global (0-63, wrap around)
        scroll_offset++;
        
        // Converte para scroll register (apenas 5 bits: 0-31)
        scroll_reg = scroll_offset & 0x1F;
        
        // PASSO 1: Scroll (instantâneo)
        WriteCmdLcd(0x40 | scroll_reg);
        DelayUs(100);
        
        // PASSO 2: Limpa sprite da posição antiga
        // Usa o offset ANTERIOR para calcular onde estava na GDRAM
        Clear_Sprite(sprite_screen_y, sprite_screen_x, (scroll_offset - 1) & 0x3F);
        
        // PASSO 3: Desenha sprite na nova posição
        Draw_Sprite(sprite_screen_y, sprite_screen_x, scroll_offset & 0x3F);
        
        DelayMs(80);
    }
}

// ==========================================================
// TESTE 3: SOFTWARE SCROLL (GARANTIDO)
// ==========================================================
void Software_Scroll_Guaranteed(void) {
    unsigned char scroll_offset = 0;
    unsigned char y;
    unsigned char sprite_y = 10, sprite_x = 4;
    
    while(1) {
        // PASSO 1: Redesenha APENAS as 32 linhas visíveis
        WriteCmdLcd(0x34);
        WriteCmdLcd(0x36);
        
        for(y = 0; y < 32; y++) {
            unsigned char src_y = (y + scroll_offset) & 0x3F;
            unsigned char dash_pattern = ((src_y % 8) < 4) ? 0xFF : 0x00;
            
            // Metade esquerda
            WriteCmdLcd(0x80 | y);
            WriteCmdLcd(0x80);
            WriteDataLcd(0xFF); WriteDataLcd(0x00);
            WriteDataLcd(0x00); WriteDataLcd(0x00);
            WriteDataLcd(0x00); WriteDataLcd(0x00);
            WriteDataLcd(dash_pattern); WriteDataLcd(dash_pattern);
            WriteDataLcd(dash_pattern); WriteDataLcd(dash_pattern);
            WriteDataLcd(0x00); WriteDataLcd(0x00);
            WriteDataLcd(0x00); WriteDataLcd(0x00);
            WriteDataLcd(0x00); WriteDataLcd(0xFF);
            
            // Metade direita
            WriteCmdLcd(0x80 | y);
            WriteCmdLcd(0x88);
            WriteDataLcd(0xFF); WriteDataLcd(0x00);
            WriteDataLcd(0x00); WriteDataLcd(0x00);
            WriteDataLcd(0x00); WriteDataLcd(0x00);
            WriteDataLcd(dash_pattern); WriteDataLcd(dash_pattern);
            WriteDataLcd(dash_pattern); WriteDataLcd(dash_pattern);
            WriteDataLcd(0x00); WriteDataLcd(0x00);
            WriteDataLcd(0x00); WriteDataLcd(0x00);
            WriteDataLcd(0x00); WriteDataLcd(0xFF);
        }
        
        WriteCmdLcd(0x30);
        
        // PASSO 2: Desenha sprite (agora usa coordenadas da TELA)
        Draw_Sprite(sprite_y, sprite_x, scroll_offset);
        
        // PASSO 3: Avança offset
        scroll_offset++;
        DelayMs(50);
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
    
    // TESTES (execute um de cada vez):
    // Test_Scroll_Direction();       // Teste 1: Vê se scroll sobe ou desce
    Scroll_With_One_Sprite();         // Teste 2: Hardware scroll + 1 sprite
    // Software_Scroll_Guaranteed();  // Teste 3: Software scroll garantido
}

















#include <reg52.h>

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
    WriteCmdLcd(0x01); DelayMs(20);
    WriteCmdLcd(0x06); DelayMs(2);
}

// ==========================================================
// DESENHA PISTA NA GDRAM
// ==========================================================
void Draw_Track() {
    unsigned char y;
    
    WriteCmdLcd(0x34);
    WriteCmdLcd(0x36);
    
    for(y = 0; y < 64; y++) {
        unsigned char dash_pattern = ((y % 8) < 4) ? 0xFF : 0x00;
        
        // Metade esquerda
        WriteCmdLcd(0x80 | y);
        WriteCmdLcd(0x80);
        WriteDataLcd(0xFF); WriteDataLcd(0x00);  // X=0
        WriteDataLcd(0x00); WriteDataLcd(0x00);  // X=1
        WriteDataLcd(0x00); WriteDataLcd(0x00);  // X=2
        WriteDataLcd(dash_pattern); WriteDataLcd(dash_pattern);  // X=3
        WriteDataLcd(dash_pattern); WriteDataLcd(dash_pattern);  // X=4
        WriteDataLcd(0x00); WriteDataLcd(0x00);  // X=5
        WriteDataLcd(0x00); WriteDataLcd(0x00);  // X=6
        WriteDataLcd(0x00); WriteDataLcd(0xFF);  // X=7
        
        // Metade direita
        WriteCmdLcd(0x80 | y);
        WriteCmdLcd(0x88);
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
// SPRITE DO CARRO (16x16 pixels)
// ==========================================================
void Draw_Sprite(unsigned char screen_y, unsigned char screen_x, unsigned char scroll_offset) {
    unsigned char gdram_y = (screen_y + scroll_offset) & 0x3F;
    
    // Carro esportivo (16x16)
    unsigned char code car_sprite[] = {
        0x00, 0x00,  // Linha 0
        0x07, 0xE0,  // Linha 1:    ******
        0x0F, 0xF0,  // Linha 2:   ********
        0x19, 0x98,  // Linha 3:  **  **  **
        0x10, 0x08,  // Linha 4:  *        *
        0x13, 0xC8,  // Linha 5:  *  ****  *
        0x1F, 0xF8,  // Linha 6:   **********
        0x1F, 0xF8,  // Linha 7:   **********
        0x1F, 0xF8,  // Linha 8:   **********
        0x0F, 0xF0,  // Linha 9:    ********
        0x09, 0x90,  // Linha 10:   *  *  *
        0x09, 0x90,  // Linha 11:   *  *  *
        0x09, 0x90,  // Linha 12:   *  *  *
        0x06, 0x60,  // Linha 13:    **  **
        0x00, 0x00,  // Linha 14
        0x00, 0x00   // Linha 15
    };
    
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
// SCROLL HÍBRIDO COM DOIS SPRITES
// ==========================================================
void Hybrid_Scroll_Two_Sprites(void) {
    unsigned char scroll_addr = 0;
    unsigned char enemy_y = 0;  // Inimigo começa no topo
    
    // Jogador: FIXO na tela (lado direito, meio)
    unsigned char player_screen_y = 12;   // Posição Y fixa na tela
    unsigned char player_screen_x = 10;   // Lado direito (X=10)
    
    // Inimigo: MÓVEL descendo (lado esquerdo)
    unsigned char enemy_screen_x = 1;     // Lado esquerdo (X=1)
    unsigned char enemy_screen_y = 0;     // Posição Y atual na tela
    unsigned char enemy_prev_y = 0;       // Posição Y anterior
    
    WriteCmdLcd(0x34);
    WriteCmdLcd(0x36);
    WriteCmdLcd(0x03);  // Habilita scroll
    
    // Desenha sprites iniciais
    Draw_Sprite(player_screen_y, player_screen_x, scroll_addr);
    Draw_Sprite(enemy_screen_y, enemy_screen_x, scroll_addr);
    
    while(1) {
        // PASSO 1: Scroll (instantâneo)
        scroll_addr++;
        WriteCmdLcd(0x40 | scroll_addr);
        
        // PASSO 2: Pequena pausa para estabilizar
        DelayUs(100);
        
        // PASSO 3: Jogador FIXO - Limpa e redesenha na mesma posição
        Clear_Sprite(player_screen_y, player_screen_x, (scroll_addr - 1) & 0x3F);
        Draw_Sprite(player_screen_y, player_screen_x, scroll_addr);
        
        // PASSO 4: Inimigo MÓVEL - Desce um pixel
        enemy_prev_y = enemy_screen_y;
        enemy_screen_y++;
        
        // Se inimigo saiu da tela, volta ao topo
        if(enemy_screen_y >= 32) {
            enemy_screen_y = 0;
        }
        
        // Limpa inimigo na posição antiga
        Clear_Sprite(enemy_prev_y, enemy_screen_x, scroll_addr);
        
        // Desenha inimigo na nova posição
        Draw_Sprite(enemy_screen_y, enemy_screen_x, scroll_addr);
        
        // PASSO 5: Delay para controle de velocidade
        DelayMs(80);  // Velocidade do jogo (scroll + movimento inimigo)
    }
}

// ==========================================================
// VERSÃO COM INIMIGO MAIS RÁPIDO/DEVAGAR
// ==========================================================
void Hybrid_Scroll_VariableSpeed(void) {
    unsigned char scroll_addr = 0;
    unsigned char frame_count = 0;
    
    // Jogador fixo (direita)
    unsigned char player_y = 12, player_x = 10;
    
    // Inimigo móvel (esquerda)
    unsigned char enemy_y = 0, enemy_x = 1;
    unsigned char enemy_prev_y = 0;
    unsigned char enemy_speed = 2;  // Move a cada 2 frames
    
    WriteCmdLcd(0x34);
    WriteCmdLcd(0x36);
    WriteCmdLcd(0x03);
    
    Draw_Sprite(player_y, player_x, scroll_addr);
    Draw_Sprite(enemy_y, enemy_x, scroll_addr);
    
    while(1) {
        // Scroll sempre acontece
        scroll_addr++;
        WriteCmdLcd(0x40 | scroll_addr);
        DelayUs(100);
        
        // Jogador fixo - sempre redesenha
        Clear_Sprite(player_y, player_x, (scroll_addr - 1) & 0x3F);
        Draw_Sprite(player_y, player_x, scroll_addr);
        
        // Inimigo move a cada 'enemy_speed' frames
        if((frame_count % enemy_speed) == 0) {
            enemy_prev_y = enemy_y;
            enemy_y++;
            
            if(enemy_y >= 32) {
                enemy_y = 0;
            }
            
            Clear_Sprite(enemy_prev_y, enemy_x, scroll_addr);
        }
        
        // Sempre redesenha inimigo na posição atual
        Draw_Sprite(enemy_y, enemy_x, scroll_addr);
        
        frame_count++;
        DelayMs(50);
    }
}

// ==========================================================
// VERSÃO COM INIMIGO OSCILANDO (sobe e desce)
// ==========================================================
void Hybrid_Scroll_OscillatingEnemy(void) {
    unsigned char scroll_addr = 0;
    char enemy_direction = 1;  // 1 = descendo, -1 = subindo
    
    // Jogador fixo (direita)
    unsigned char player_y = 12, player_x = 10;
    
    // Inimigo oscilante (esquerda)
    unsigned char enemy_y = 0, enemy_x = 1;
    unsigned char enemy_prev_y = 0;
    
    WriteCmdLcd(0x34);
    WriteCmdLcd(0x36);
    WriteCmdLcd(0x03);
    
    Draw_Sprite(player_y, player_x, scroll_addr);
    Draw_Sprite(enemy_y, enemy_x, scroll_addr);
    
    while(1) {
        scroll_addr++;
        WriteCmdLcd(0x40 | scroll_addr);
        DelayUs(100);
        
        // Jogador fixo
        Clear_Sprite(player_y, player_x, (scroll_addr - 1) & 0x3F);
        Draw_Sprite(player_y, player_x, scroll_addr);
        
        // Inimigo oscilante
        enemy_prev_y = enemy_y;
        
        if(enemy_direction == 1) {  // Descendo
            enemy_y++;
            if(enemy_y >= 28) {  // Não deixa sair totalmente
                enemy_direction = -1;  // Começa subir
            }
        } else {  // Subindo
            enemy_y--;
            if(enemy_y == 0) {
                enemy_direction = 1;  // Começa descer
            }
        }
        
        Clear_Sprite(enemy_prev_y, enemy_x, scroll_addr);
        Draw_Sprite(enemy_y, enemy_x, scroll_addr);
        
        DelayMs(80);
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
    
    // Escolha o modo de jogo:
    Hybrid_Scroll_Two_Sprites();        // Inimigo desce contínuo
    // Hybrid_Scroll_VariableSpeed();   // Inimigo com velocidade variável
    // Hybrid_Scroll_OscillatingEnemy(); // Inimigo sobe e desce
} 



