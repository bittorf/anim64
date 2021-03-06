/** Copyright (c) 2012, Johan Kotlinski

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

#include "anim_play.h"

#include <conio.h>

#include "irq.h"
#include "music.h"
#include "player.h"
#include "screen.h"

#pragma codeseg("EDITCODE")
#pragma rodataseg("EDITCODE")

static void init_play() {
    *(char*)0xdc0d = 0x7f;  // Disable kernal timer interrupts.
    *(char*)1 = 0x35;  // Switch out kernal - enables $e000-$ffff RAM.

    // Scan all keyboard rows.
    *(char*)0xdc00 = 0;

    *(char*)0xd011 &= 0x7f;  // clear raster line bit 8
    *(char*)0xd012 = 0xfb;  // raster line
}

static void exit_play() {
    *(char*)0xd01a = 0;  // disable raster interrupts
    caught_irqs = 0;
    *(voidFn*)0xfffe = (voidFn)0x314;  // set irq handler pointer
    *(char*)1 = 0x36;  // RAM + I/O + Kernal.
    *(char*)0xdc0d = 0x81;  // Re-enable kernal timer interrupts.
    *(char*)0xd418 = 0;  // Mute sound.
}

void preview_play_anim(unsigned char speed,
        unsigned int skip_music_ticks,
        unsigned char frames) {
    char keyboard_state = 0;

    curr_screen = 0;
    init_music();
    while (skip_music_ticks--) {
        tick_music();
    }
    init_play();
    *(voidFn*)0xfffe = irq_handler_v2;  // set irq handler pointer
    *(char*)0xd01a = 1;  // enable raster interrupts
    while (frames--) {
        unsigned char frame_delay = speed;
        redraw_edit_screen();
        while (frame_delay) {
            if (caught_irqs) {
                --caught_irqs;
                --frame_delay;

                // To exit animation, first all keys should be released, then
                // some key should be pressed.
                if (keyboard_state == 0) {
                    if (0xff == *(char*)0xdc01) {  // All keys released?
                        keyboard_state = 1;
                    }
                } else if (0xff != *(char*)0xdc01) {  // Any key pressed?
                    goto exit;
                }
            }
            blink_vic_from_sid(*curr_bg_color());
        }
        if (end_frame == curr_screen) {
            curr_screen = 0;
        } else {
            ++curr_screen;
        }
    }
exit:
    exit_play();

    if (kbhit()) cgetc();
}

