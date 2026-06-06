
#include <reg52.h>

#define GlcdDataBus P3
sbit RS  = P2^0;
sbit RW  = P2^1;
sbit EN  = P2^2;
sbit PSB = P2^3;
sbit RST = P2^5;

typedef unsigned char u8;
typedef signed char   i8;
typedef unsigned int  u16;

#define DINO_X     10
#define DINO_Y     40
#define JUMP_Y     22
#define GROUND_Y   56
#define MAX_OBS    3
#define CLOUD_Y    4

u8 line_buf[16];

code u8 bit_shr[8] = {128, 64, 32, 16, 8, 4, 2, 1};
code u8 bit_shl[8] = {1, 2, 4, 8, 16, 32, 64, 128};

void set_px(u8 px) {
    if (px < 128) line_buf[px >> 3] |= bit_shr[px & 7];
}

u8 rng_seed = 0x55;
u8 my_rand() {
    rng_seed ^= (rng_seed << 3);
    rng_seed ^= (rng_seed >> 5);
    rng_seed ^= (rng_seed << 2);
    return rng_seed;
}

code u8 bmp_dino[32] = {
    0x03, 0xFF, 0x02, 0xBF, 0x03, 0xFF, 0x03, 0x80,
    0x83, 0x8E, 0xC3, 0xFE, 0xE3, 0xFF, 0xFF, 0xFE,
    0xFF, 0xFE, 0x7F, 0xFE, 0x3F, 0xFC, 0x1F, 0xF0,
    0x07, 0x30, 0x06, 0x30, 0x06, 0x30, 0x07, 0x38
};

code u8 bmp_cactus[32] = {
    0x00, 0x00, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80,
    0x19, 0x8C, 0x19, 0x8C, 0x19, 0x8C, 0x19, 0x8C,
    0x1F, 0x8C, 0x0F, 0x8C, 0x07, 0xFC, 0x03, 0xF8,
    0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x03, 0xC0
};

code u8 bmp_bird[32] = {
    0x00, 0x00, 0x00, 0x08, 0x00, 0x18, 0x00, 0x38,
    0x00, 0x78, 0x01, 0xF8, 0x07, 0xF8, 0x1F, 0xF8,
    0x3F, 0xFF, 0x1F, 0xBE, 0x07, 0xF0, 0x03, 0xE0,
    0x01, 0xC0, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00
};

code u8 bmp_cloud[48] = {
    0x00,0x00,0x00,0x00, 0x00,0x01,0xC0,0x00,
    0x00,0x07,0xF0,0x00, 0x00,0x0F,0xF8,0x00,
    0x00,0x1F,0xFC,0x00, 0x01,0xFF,0xFF,0x80,
    0x03,0xFF,0xFF,0xC0, 0x07,0xFF,0xFF,0xE0,
    0x0F,0xFF,0xFF,0xF0, 0x1F,0xFF,0xFF,0xF8,
    0x0F,0xFF,0xFF,0xF0, 0x00,0x00,0x00,0x00
};


code u8 font5x7[10][5] = {
    {0x3E,0x51,0x49,0x45,0x3E}, {0x00,0x42,0x7F,0x40,0x00},
    {0x42,0x61,0x51,0x49,0x46}, {0x21,0x41,0x45,0x4B,0x31},
    {0x18,0x14,0x12,0x7F,0x10}, {0x27,0x45,0x45,0x45,0x39},
    {0x3C,0x4A,0x49,0x49,0x30}, {0x01,0x71,0x09,0x05,0x03},
    {0x36,0x49,0x49,0x49,0x36}, {0x06,0x49,0x49,0x29,0x1E}
};

typedef struct {
    u8 x; u8 y; u8 type; u8 active;
} Obstacle;

Obstacle obs[MAX_OBS];

/* ============================================================
 * VARIÁVEIS GLOBAIS DO JOGO
 * ============================================================
 */
u8  placar[5];      /* dígitos do placar: [0]=milhar...[4]=unidade */

u8  dino_jump;      /* 1 = dinossauro está no ar                   */
u8  jump_phase;     /* contador de frames do salto (0-7)           */

u8  game_over;      /* 1 = partida encerrada                       */

u8  phase;          /* flag de inversão de cor (efeito flash)      */
u8  phase_ctr;      /* contador para alternância de fase           */

u8  speed_ctr;      /* contador para aumento de velocidade         */

u8  frame_cnt;      /* contador de frames entre obstáculos         */

u8  cloud_x;        /* posição horizontal da nuvem                 */

u16 tick_period;    /* período do tick em ms (controla velocidade) */



void delay_us(u16 us) {
    while (us--) {
    }
}

void delay_ms(u16 ms) {
    u16 i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 120; j++);
}

/* ============================================================
 * LEITURA NÃO BLOQUEANTE DO TECLADO
 * ============================================================
 */
char scanKeyNonBlock() {
    static u8 last_P0 = 0xFF;
    char key = 0;
    u8 curr_P0;

    P0 = 0xF7;
    curr_P0 = P0;

    if      (!(curr_P0 & 0x10) && (last_P0 & 0x10)) key = '3';
    else if (!(curr_P0 & 0x20) && (last_P0 & 0x20)) key = '2';

    last_P0 = curr_P0;
    return key;
}

void glcd_enable() {
    EN = 1;
    delay_us(1);
    EN = 0;
}

void glcd_cmd(u8 cmd) {
    RS = 0; RW = 0; GlcdDataBus = cmd;
    glcd_enable();
    delay_us(5);
}

void glcd_data(u8 dat) {
    RS = 1; RW = 0; GlcdDataBus = dat;
    glcd_enable();
    delay_us(2);
}

/* ============================================================
 * INICIALIZAÇÃO DO GLCD
 * ============================================================
 */
void glcd_init() {
    PSB = 1;
    RST = 0; delay_ms(10);
    RST = 1; delay_ms(20);
    glcd_cmd(0x30);
    glcd_cmd(0x0C);
    glcd_cmd(0x01); delay_ms(10);
    glcd_cmd(0x34);
    glcd_cmd(0x36);
}

void glcd_write_line(u8 y, u8 *buf) {
    u8 x;
    if (y < 32) { glcd_cmd(0x80 | y);        glcd_cmd(0x80); }
    else        { glcd_cmd(0x80 | (y - 32));  glcd_cmd(0x88); }

    for (x = 0; x < 16; x++) {
        if (phase) glcd_data(~buf[x]);
        else       glcd_data( buf[x]);
    }
}

void draw_bmp(u8 x, u8 y_curr, u8 obj_y, u8 code *bmp, u8 w_bytes, u8 h) {
    u8 row, col, b, bmp_data, px;

    if (y_curr >= obj_y && y_curr < obj_y + h) {
        row = y_curr - obj_y;
        for (col = 0; col < w_bytes; col++) {
            bmp_data = bmp[(w_bytes == 2 ? (row << 1) : (row << 2)) + col];
            if (!bmp_data) continue;
            for (b = 0; b < 8; b++) {
                if (bmp_data & bit_shr[b]) {
                    px = x + (col << 3) + b;
                    set_px(px);
                }
            }
        }
    }
}

/* ============================================================
 * DESENHO DO PLACAR
 * ============================================================
 */
void draw_score(u8 y) {
    u8 row, i, col, px, start_x;

    if (y >= 2 && y < 9) {
        row = y - 2;
        for (i = 0; i < 5; i++) {
            start_x = 2 + i * 7;
            for (col = 0; col < 5; col++) {
                if (font5x7[placar[i]][col] & bit_shl[row]) {
                    px = start_x + col;
                    set_px(px);
                }
            }
        }
    }
}

void draw_scene() {
    u8 y, i;

    for (y = 0; y < 64; y++) {
        for (i = 0; i < 16; i++) line_buf[i] = 0x00;

        draw_bmp(cloud_x, y, CLOUD_Y, bmp_cloud, 4, 12);

        if (y == GROUND_Y || y == GROUND_Y + 1) {
            for (i = 0; i < 16; i++) line_buf[i] = 0xFF;
        }

        draw_score(y);

        for (i = 0; i < MAX_OBS; i++) {
            if (!obs[i].active) continue;
            if (obs[i].type == 0) draw_bmp(obs[i].x, y, 40, bmp_cactus, 2, 16);
            else                  draw_bmp(obs[i].x, y, 24, bmp_bird,   2, 16);
        }

        if (dino_jump) draw_bmp(DINO_X, y, JUMP_Y, bmp_dino, 2, 16);
        else           draw_bmp(DINO_X, y, DINO_Y, bmp_dino, 2, 16);

        glcd_write_line(y, line_buf);
    }
}

void draw_start_screen() {
    u8 y, i;
    for (y = 0; y < 64; y++) {
        for (i = 0; i < 16; i++) line_buf[i] = 0x00;
        draw_bmp(56, y, 18, bmp_dino, 2, 16);
        glcd_write_line(y, line_buf);
    }
}

/* ============================================================
 * DETECÇÃO DE COLISÃO
 * ============================================================
 *
 * Caixa do dinossauro (hitbox com margem de 2px para
 * suavizar colisões nos cantos):
 *   x: [DINO_X+2 .. DINO_X+13]
 *   y: [dino_y+2 .. dino_y+13]
 *
 * Caixa dos obstáculos (16x16 com margem de 2px):
 *   x: [obs.x+2 .. obs.x+13]
 *   y: cacto ? [40+2 .. 40+13]
 *      pássaro ? [24+2 .. 24+13]
 */
u8 check_collision() {
    u8 i;
    u8 dino_y;
    u8 dino_x1, dino_x2, dino_y1, dino_y2;
    u8 obs_x1,  obs_x2,  obs_y1,  obs_y2;

    dino_y = dino_jump ? JUMP_Y : DINO_Y;

    /* Hitbox do dinossauro com margem de 2px */
    dino_x1 = DINO_X + 2;
    dino_x2 = DINO_X + 13;
    dino_y1 = dino_y + 2;
    dino_y2 = dino_y + 13;

    for (i = 0; i < MAX_OBS; i++) {
        if (!obs[i].active) continue;

        /* Hitbox do obstáculo com margem de 2px */
        obs_x1 = obs[i].x + 2;
        obs_x2 = obs[i].x + 13;

        if (obs[i].type == 0) { /* cacto — no chão */
            obs_y1 = 40 + 2;
            obs_y2 = 40 + 13;
        } else {                /* pássaro — no ar */
            obs_y1 = 24 + 2;
            obs_y2 = 24 + 13;
        }

        /* AABB: colisão se os retângulos se sobrepõem nos dois eixos */
        if (dino_x1 <= obs_x2 && dino_x2 >= obs_x1 &&
            dino_y1 <= obs_y2 && dino_y2 >= obs_y1) {
            return 1;
        }
    }
    return 0;
}

/* ============================================================
 * GERAÇÃO DE OBSTÁCULOS
 * ============================================================
 *
 * Ativa o primeiro slot livre do vetor obs[].
 * O tipo é sorteado: 0 = cacto, 1 = pássaro.
 * O obstáculo sempre nasce na borda direita (x = 120).
 */
void spawn_obstacle() {
    u8 i;
    for (i = 0; i < MAX_OBS; i++) {
        if (!obs[i].active) {
            obs[i].active = 1;
            obs[i].x      = 120;
            obs[i].type   = my_rand() & 1;  /* bit menos significativo: 0 ou 1 */
            break;
        }
    }
}

/* ============================================================
 * ATUALIZAÇÃO DO PLACAR
 * ============================================================
 *
 * O placar é um contador de 5 dígitos decimais armazenado
 * em placar[0..4], onde placar[4] é a unidade.
 * Incrementa 1 ponto a cada chamada (1 tick = 1 ponto).
 */
void add_score() {
    /* Incrementa unidade e propaga carry para dígitos superiores */
    placar[4]++;
    if (placar[4] > 9) { placar[4] = 0; placar[3]++; }
    if (placar[3] > 9) { placar[3] = 0; placar[2]++; }
    if (placar[2] > 9) { placar[2] = 0; placar[1]++; }
    if (placar[1] > 9) { placar[1] = 0; placar[0]++; }
    if (placar[0] > 9) {                placar[0] = 0; } /* overflow ? zera */
}


void game_tick() {
    u8 i;

    /* Move obstáculos para a esquerda */
    for (i = 0; i < MAX_OBS; i++) {
        if (obs[i].active) {
            if (obs[i].x <= 4) obs[i].active = 0;
            else               obs[i].x -= 4;
        }
    }

    /* Move nuvem lentamente */
    if (cloud_x <= 1) cloud_x = 120;
    else              cloud_x -= 1;

    /* Spawna novo obstáculo quando o intervalo aleatório expira */
    frame_cnt++;
    if (frame_cnt >= 18 + (my_rand() & 7)) {
        spawn_obstacle();
        frame_cnt = 0;
    }

    /* Controla duração do salto */
    if (dino_jump) {
        jump_phase++;
        if (jump_phase >= 8) { dino_jump = 0; jump_phase = 0; }
    }

    add_score();

    /* Alterna inversão de cor a cada 100 ticks */
    if (++phase_ctr >= 100) { phase = !phase; phase_ctr = 0; }

    /* Aumenta velocidade a cada 50 ticks (diminui tick_period) */
    if (++speed_ctr >= 50) {
        speed_ctr = 0;
        if (tick_period > 25) tick_period >>= 1;
    }

    if (check_collision()) game_over = 1;
    if (!game_over) draw_scene();
}

void game_init() {
    u8 i;
    for (i = 0; i < 5; i++) placar[i] = 0;

    tick_period = 100;
    speed_ctr   = 0;
    phase_ctr   = 0;
    dino_jump   = 0;
    jump_phase  = 0;
    game_over   = 0;
    frame_cnt   = 30;   /* começa próximo ao primeiro spawn */
    phase       = 0;
    cloud_x     = 100;

    for (i = 0; i < MAX_OBS; i++) obs[i].active = 0;
}

void main() {
    u16 t;
    char key;

    glcd_init();
    draw_start_screen();

    /* Aguarda tecla '2' para iniciar */
    while (1) {
        key = scanKeyNonBlock();
        if (key == '2') break;
    }

    game_init();
    draw_scene();

    while (1) {
        /* Reinicia após game over */
        if (game_over) {
            phase = 0;
            draw_start_screen();
            while (1) {
                key = scanKeyNonBlock();
                if (key == '2') { game_init(); draw_scene(); break; }
            }
        }

        /* Loop de tick: varre teclado a cada 1 ms e executa o tick ao final */
        for (t = 0; t < tick_period; t++) {
            key = scanKeyNonBlock();
            if (key == '3') {           /* botão de salto */
                if (!dino_jump) { dino_jump = 1; jump_phase = 0; }
            }
            delay_ms(1);
        }

        game_tick();
    }
}
