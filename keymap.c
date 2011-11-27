/** Copyright (c) 2011, Johan Kotlinski

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE. */

#include "keymap.h"

#include <conio.h>
#include <string.h>

static char keymap['z' - 'a' + 1];
static char keymap_index;

static char cur_x;
static char cur_y;

static char reversed;

void update_mapping() {
    keymap[keymap_index] = cur_x | (cur_y << 4) | (reversed << 7);
    gotoxy(cur_x, cur_y);
}

void draw_keymap() {
    unsigned char x = 0;
    unsigned char y = 0;
    unsigned char ch = reversed ? 0x80 : 0;
    char* ptr = (char*)0x400;
    for (y = 0; y < 8; ++y) {
        for (x = 0; x < 16; ++x) {
            *ptr = ch;
            ++ptr;
            ++ch;
        }
        ptr += 40 - 16;
    }
}

void init_keymap() {
    char i;
    for (i = 0; i < sizeof(keymap); ++i) {
        keymap[i] = 1 + i;
    }

    clrscr();
    draw_keymap();
}

void enter_keymap_mode(char key_index) {
    char cur_xy;
    *(char*)0xd018 = 0x14;  // Point video to 0x400.
    memset((char*)0xd800, COLOR_YELLOW, 40 * 25);
    gotoxy(0, 9);
    textcolor(COLOR_CYAN);
    cprintf("remap %c", key_index + 1);
    gotoxy(0, 11);
    cputs("return = invert");
    gotoxy(0, 12);
    cputs("other = select");
    keymap_index = key_index;
    cur_xy = keymap[key_index];
    cur_x = cur_xy & 0xf;
    cur_y = (cur_xy >> 4) & 7;
    gotoxy(cur_x, cur_y);
}

/* Return 1 if selected key, 0 if done */
char do_keymap(char key) {
    switch (key) {
        case CH_ENTER:
            reversed ^= 1;
            draw_keymap();
            update_mapping();
            break;
        case CH_CURS_UP:
            if (cur_y > 0) {
                --cur_y;
                update_mapping();
            }
            break;
        case CH_CURS_DOWN:
            if (cur_y < 7) {
                ++cur_y;
                update_mapping();
            }
            break;
        case CH_CURS_LEFT:
            if (cur_x > 0) {
                --cur_x;
                update_mapping();
            }
            break;
        case CH_CURS_RIGHT:
            if (cur_x < 15) {
                ++cur_x;
                update_mapping();
            }
            break;
        default:
            return 1;
    }
    return 0;
}

char get_char(char key) {
    return keymap[key - 'a'];
}
