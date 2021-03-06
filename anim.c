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

#include "anim.h"
#include "convert.h"
#include "player.h"
#include "irq.h"
#include "rle.h"
#include "lz77.h"
#include "screen.h"

#include <conio.h>
#include <string.h>
#include "cc65/mycbm.h"

#pragma codeseg("EDITCODE")

#define MY_LFN 1
static void load_v2v3_anim(char lz77) {
    unsigned int read_bytes;
    const unsigned char* rle_start;
    curr_screen = 0;
    mycbm_read(MY_LFN, &end_frame, 1);
    --end_frame;
    // Read all compressed frames to start of screen area...
    read_bytes = mycbm_read(MY_LFN, SCREEN_BASE, SCREEN_AREA_SIZE);
    // ...move them to end of screen area...
    rle_start = CLIPBOARD - read_bytes;
    memmove(rle_start, SCREEN_BASE, read_bytes);
    // ...then unpack them one by one.
    while (curr_screen <= end_frame) {
        if (lz77)
            rle_start = lz77_unpack(curr_screen_chars(), rle_start);
        else
            rle_start = rle_unpack(curr_screen_chars(), rle_start);
        if (curr_screen != 0) {
            if (*rle_start) {
                xor_prev_v2();
            }
            ++rle_start;
        }
        ++curr_screen;
    }
}

char load_and_unpack_anim() {
    unsigned char first_byte;
    if (mycbm_read(MY_LFN, &first_byte, 1) <= 0) {
        mycbm_close(MY_LFN);
        return 0;  // Error.
    }
    switch (first_byte) {
        case 0:
        case 1:
            // Version 1: first_byte is interframe compression on/off.
            convert_v1_v2(first_byte);
            break;
        case 2:
            load_v2v3_anim(0);
            break;
        case 3:
            load_v2v3_anim(1);
            break;
        default:
            for (;;) ++*(char*)0xd020;  // Not supported.
    }
    mycbm_close(MY_LFN);
    curr_screen = 0;
    return 1;
}

char load_linde() {
    char tmp = 0;
    // skip 0x801
    if (0 == mycbm_read(MY_LFN, &tmp, 1)) return 0;
    if (tmp != 1) return 0;
    mycbm_read(MY_LFN, &tmp, 1);
    if (tmp != 8) return 0;
    mycbm_read(MY_LFN, (char*)0x400, 0x53);  // skip code
    mycbm_read(MY_LFN, (char*)0x400, 40 * 25);  // chars
    mycbm_read(MY_LFN, (char*)0xd800, 40 * 25);  // colors
    mycbm_read(MY_LFN, (char*)0xd020, 2);  // border
    mycbm_close(MY_LFN);
}
