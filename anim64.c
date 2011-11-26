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

#include <conio.h>

#include "keymap.h"

#define SCREEN (char*)0x400u

static unsigned char cur_x;
static unsigned char cur_y;

static char color;
static void set_color(char c) {
    textcolor(c);
    color = c;
}

static void init() {
    clrscr();
    bordercolor(0);
    set_color(1);
    bgcolor(0);
    cursor(1);
    init_keymap();
}

#define PAINT_MODE 0
#define KEYMAP_MODE 1
static char mode;

static char last_char = 'a';
static char paint_char = 'a';
static char painting;

static unsigned int offset() {
    return 40 * cur_y + cur_x;
}

static void punch(char ch) {
    *(char*)(0x400u + offset()) = ch;
    *(char*)(0xd800u + offset()) = color;
}

static char screen_char() {
    unsigned int offset = 40 * cur_y + cur_x;
    return *(char*)(0x400u + offset);
}

static void move_cursor() {
    gotoxy(cur_x, cur_y);
}

static char hidden_screen_char = ' ';

static void pre_cur_move() {
    if (!painting) {
        punch(hidden_screen_char);
    }
}

static void post_cur_move() {
    move_cursor();
    hidden_screen_char = painting ? paint_char : screen_char();
    punch(paint_char);
}

static void do_paint(char ch) {
    if (ch >= '1' && ch <= '8') {  // textcolor 1-8
        set_color(ch - '1');
    } else if (ch >= '1' - 16 && ch <= '8' - 16) {  // textcolor 9-16
        set_color(ch - '1' - 16 + 8);
    } else if (ch >= 'a' && ch <= 'z') {
        if (last_char == ch) {
            ch = ' ';  // Paint!
        } else {
            last_char = ch;
            paint_char = get_char(ch);
            punch(paint_char);
        }
    } else if (ch >= 'A' && ch <= 'Z') {
        mode = KEYMAP_MODE;
        last_char = ch - 'A' + 'a';
        enter_keymap_mode(ch - 'A');
    }

    switch (ch) {
        case CH_CURS_UP:
            if (cur_y > 0) {
                pre_cur_move();
                --cur_y;
                post_cur_move();
            }
            break;
        case CH_CURS_DOWN:
            if (cur_y < 24) {
                pre_cur_move();
                ++cur_y;
                post_cur_move();
            }
            break;
        case CH_CURS_LEFT:
            if (cur_x > 0) {
                pre_cur_move();
                --cur_x;
                post_cur_move();
            }
            break;
        case CH_CURS_RIGHT:
            if (cur_x < 39) {
                pre_cur_move();
                ++cur_x;
                post_cur_move();
            }
            break;
        case ' ':
            if (painting) {
                painting = 0;
            } else {
                painting = 1;
                hidden_screen_char = paint_char;
                punch(paint_char);
            }
            break;
    }
}

void main() {
    init();
    punch(paint_char);
    while (1) {
        const char ch = cgetc();
        switch (mode) {
            case PAINT_MODE:
                do_paint(ch);
                break;
            case KEYMAP_MODE:
                if (do_keymap(ch)) {
                    mode = PAINT_MODE;
                    clrscr();
                    move_cursor();
                    paint_char = get_char(last_char);
                    punch(paint_char);
                }
                break;
        }
    }
}
