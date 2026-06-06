#include <reg51.h>
#include <stdio.h>
#include <stdlib.h>

sfr LCD_Port = 0x90;
sbit RW = P1^1;
sbit RS = P1^3;
sbit EN = P1^2;

void delay(unsigned int);
void LCD_Init(void);
void LCD_Command(char);
void LCD_Char(char);
void LCD_String(char *str);
void LCD_String_xy(char row, char pos, char *str);
void store_custom_char(unsigned char slot, unsigned char *map);
void load_custom_chars(void);
void left_shift(unsigned char *array);
void print_score(unsigned char row, unsigned char pos, int val);
void update_display(void);
int collision(void);
void jump(void);
unsigned int rand_gen(void);
void generate_random_obs(char* floor_obs, char* sky_obs);
void game_over_screen(void);
int game(void);

unsigned char code rex2[8]      = {0x07, 0x05, 0x07, 0x1C, 0x1C, 0x1C, 0x16, 0x16};
unsigned char code rex[8]       = {0x07, 0x05, 0x07, 0x1C, 0x1C, 0x1E, 0x13, 0x11};
unsigned char code nuvem[8]     = {0x00, 0x00, 0x0C, 0x13, 0x11, 0x1F, 0x00, 0x00};
unsigned char code cactus[8]    = {0x00, 0x0C, 0x0D, 0x1D, 0x1F, 0x0E, 0x0E, 0x0E};
unsigned char code passaro[8]   = {0x00, 0x00, 0x1B, 0x0A, 0x0C, 0x08, 0x00, 0x00};
unsigned char code chao_char[8] = {0x00, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x1F};

char jump_flag  = 0;
char start_flag = 0;
char rex_y      = 1;
int  frame      = 0;
int  next_obs   = 8;
int  speed      = 50;
int  points     = 0;
char jmp_delay  = 0;
int  i;
char lives      = 3;

unsigned int rand_state = 12345;

unsigned char sky[16]       = {0};
unsigned char floor_arr[16] = {1};

void LCD_Init()
{
    delay(20);
    LCD_Command(0x02);
    LCD_Command(0x28);
    LCD_Command(0x0C);
    LCD_Command(0x06);
    LCD_Command(0x01);
    LCD_Command(0x80);
}

void LCD_Command(char cmnd)
{
    LCD_Port = (LCD_Port & 0x0F) | (cmnd & 0xF0);
    RS = 0; RW = 0; EN = 1;
    delay(1);
    EN = 0;
    delay(2);

    LCD_Port = (LCD_Port & 0x0F) | ((cmnd << 4) & 0xF0);
    EN = 1;
    delay(1);
    EN = 0;
    delay(5);
}

void delay(unsigned int count)
{
    int i, j;
    for(i = 0; i < count; i++)
        for(j = 0; j < count; j++);
}

void LCD_String(char *str)
{
    int i;
    for(i = 0; str[i] != 0; i++)
        LCD_Char(str[i]);
}

void LCD_String_xy(char row, char pos, char *str)
{
    if(row == 0)      LCD_Command((pos & 0x0F) | 0x80);
    else if(row == 1) LCD_Command((pos & 0x0F) | 0xC0);
    LCD_String(str);
}

void LCD_Char(char char_data)
{
    LCD_Port = (LCD_Port & 0x0F) | (char_data & 0xF0);
    RS = 1; RW = 0; EN = 1;
    delay(1);
    EN = 0;
    delay(2);

    LCD_Port = (LCD_Port & 0x0F) | ((char_data << 4) & 0xF0);
    EN = 1;
    delay(1);
    EN = 0;
    delay(5);
}

void store_custom_char(unsigned char slot, unsigned char *map)
{
    unsigned char i;
    LCD_Command(0x40 + (slot * 8));
    for(i = 0; i < 8; i++)
        LCD_Char(map[i]);
}

void load_custom_chars()
{
    store_custom_char(0, rex);
    store_custom_char(1, rex2);
    store_custom_char(2, nuvem);
    store_custom_char(3, cactus);
    store_custom_char(4, passaro);
    store_custom_char(5, chao_char);
    LCD_Command(0x80); // volta para DDRAM após escrever na CGRAM
}

void left_shift(unsigned char *array)
{
    int i;
    for(i = 0; i < 15; i++)
        array[i] = array[i + 1];
    array[15] = 0;
}

void print_score(unsigned char row, unsigned char pos, int val)
{
    unsigned char buf[4];
    unsigned char i    = 0;
    unsigned char leni = 0;
    unsigned char ch[2];

    if(val == 0) {
        LCD_String_xy(row, pos, "0  ");
        return;
    }

    while(val > 0 && leni < 3) {
        buf[leni++] = (val % 10) + '0';
        val /= 10;
    }

    ch[1] = '\0';
    for(i = 0; i < leni; i++) {
        ch[0] = buf[leni - 1 - i];
        LCD_String_xy(row, pos + i, ch);
    }
}

void update_display()
{
    int i;
    LCD_Command(0x80);
    for(i = 0; i < 16; i++) {
        if(i == 1 && rex_y == 0) {
            if(frame % 2 == 0) LCD_Char(0);
            else               LCD_Char(1);
        }
        else if(i >= 13)
            continue;
        else if(sky[i] == 1)
            LCD_Char(2);
        else if(sky[i] == 2)
            LCD_Char(4);
        else
            LCD_Char(' ');
    }
    print_score(0, 13, points);

    LCD_Command(0xC0);
    for(i = 0; i < 16; i++) {
        if(i == 1 && rex_y == 1) {
            if(frame % 2 == 0) LCD_Char(0);
            else               LCD_Char(1);
        }
        else if(floor_arr[i] == 1 && i != 1)
            LCD_Char(3);
        else if(floor_arr[i] == 2 || floor_arr[i] == 0)
            LCD_Char(5);
        else
            LCD_Char(5);
    }

    for(i = 0; i < 15; i++) floor_arr[i] = floor_arr[i + 1];
    floor_arr[15] = 0;

    for(i = 0; i < 15; i++) sky[i] = sky[i + 1];
    sky[15] = 0;
}

int collision()
{
    if(rex_y == 0 && sky[1] == 2)       return 1;
    if(rex_y == 1 && floor_arr[1] == 1) return 1;
    return 0;
}

void jump()
{
    if(rex_y == 1) {
        jump_flag = 1;
        rex_y     = 0;
        jmp_delay = 3;
    }
}

unsigned int rand_gen()
{
    rand_state = rand_state * 1664525 + 1013904223;
    return (rand_state >> 8) & 0xFF;
}

void generate_random_obs(char* floor_obs, char* sky_obs)
{
    unsigned int r = rand_gen() % 3;
		unsigned int r2 = rand_gen() % 3;

    if(r == 0 || r == 1 || r2 == 1) {
        floor_obs[15] = 1; // cactus
        sky_obs[15]   = 1;
    } else if(r >= 2) {
        sky_obs[15]   = 2; // passaro
        floor_obs[15] = 0;
    } else if(r2 >= 1 ) {
        sky_obs[15]   = 1; // nuvem
        floor_obs[15] = 0;
    } 
			else if( (r + r2) >= 3){
				sky_obs[15]   = 1;
			}
			else {
        floor_obs[15] = 0; // vazio
        sky_obs[15]   = 0;
    }
}

void game_over_screen()
{
    LCD_Command(0x01);
    LCD_Command(0x80);
    LCD_String("  GAME  OVER  ");
    LCD_Command(0xC0);
    LCD_String("Press to retry");

    while(1) {
        if(!(P0 & 0x20)) {
            delay(20);
            while(!(P0 & 0x20));
            return;
        }
    }
}

int game()
{
    P0 = 0xFD;
    LCD_Init();
    load_custom_chars();
    LCD_Command(0x01);

start:
    if(lives == 0) {
        game_over_screen();
        lives      = 3;
        frame      = 0;
        points     = 0;
        jump_flag  = 0;
        start_flag = 0;
        rex_y      = 1;
        jmp_delay  = 0;
        LCD_Init();
        load_custom_chars();
        LCD_Command(0x01);
        goto start;
    }

    for(i = 0; i < 16; i++) floor_arr[i] = 0;
    for(i = 0; i < 16; i++) sky[i]       = 0;

    rand_state = frame + points + 7919; // varia semente a cada partida

    speed  = 40;
    points = 0;

    while(!start_flag) {
        if(!(P0 & 0x20)) start_flag = 1;
    }

    while(1) {
        if(jump_flag) {
            if(jmp_delay > 0) jmp_delay--;
            else {
                rex_y     = 1;
                jump_flag = 0;
            }
        }

        if(!jump_flag && !(P0 & 0x20)) jump();

        update_display();

        if(frame == next_obs) {
					generate_random_obs(floor_arr, sky);
					next_obs = frame + 4 + (rand_gen() % 6); // intervalo entre 4 e 9 frames
				}

        if(collision()) {
            delay(10);
            lives--;
            jump_flag  = 0;
            rex_y      = 1;
            jmp_delay  = 0;
            start_flag = 1;
            goto start;
        }

        frame++;

        if(points > 0 && points % 50 == 0)
            speed = speed / 2;

        points++;
        delay((5 * speed) / 1);
    }
    return 0;
}

int main()
{
    game();
    return 0;
}
