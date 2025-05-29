/*
Copyright (c) 2004-2019, Lode Vandevenne

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <sys/mman.h>
#include <sys/time.h>
#include <inttypes.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "demo.h"

#include "uC_braille.h"


extern uC_screen_t *active_screen;

char status[33];

// -----------------------------------------------------------------------

static uint16_t mapWidth  = 24;
static uint16_t mapHeight = 24;

static double posX = 22;       // x and y start position
static double posY = 12;
static double dirX = -1;       // initial direction vector
static double dirY = 0;

static double moveSpeed;
static double rotSpeed;

static double planeX = 0;      // the 2d raycaster version of camera plane
static double planeY = 0.66;

static uC_screen_t *scr;
static uC_window_t *win;
static uC_window_t *status_win;

static uint8_t *dark_map;     // adjacent walls are different shades
static uint8_t *color_map;    // what color to draw each column of screen

static uint8_t *fb;           // array of set "pixles" one bit per byte

static uint16_t *braile_data; // interim state from fb to display

static uint16_t fb_width;     // dimensuons of frame buffer
static uint16_t fb_height;

// -----------------------------------------------------------------------

uint8_t worldMap[24][24]=
{
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 0, 0, 0, 0, 3, 0, 3, 0, 3, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 2, 2, 0, 2, 2, 0, 0, 0, 0, 3, 0, 3, 0, 3, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 4, 0, 4, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 4, 0, 0, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 4, 0, 4, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 4, 0, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
};

// -----------------------------------------------------------------------

#define O 1
#define Z 0

uint8_t wall[] =
{
    O,O,O,Z,Z,O,O,O,O,O,O,O,O,O,Z,O,O,O,O,O,O,Z,Z,O,O,Z,Z,Z,Z,O,O,O,O,O,O,Z,Z,O,O,O,O,O,O,O,O,O,Z,O,O,O,O,O,O,Z,Z,O,O,Z,Z,Z,Z,O,O,O,
    O,O,O,O,Z,Z,Z,O,O,O,O,O,O,O,Z,O,O,O,O,O,O,Z,O,O,O,O,O,Z,O,O,O,O,O,O,O,O,Z,Z,Z,O,O,O,O,O,O,O,Z,O,O,O,O,O,O,Z,O,O,O,O,O,Z,O,O,O,O,
    O,O,O,O,Z,O,Z,Z,O,O,O,O,O,O,Z,O,O,O,O,O,O,Z,O,O,O,O,Z,Z,O,O,O,O,O,O,O,O,Z,O,Z,Z,O,O,O,O,O,O,Z,O,O,O,O,O,O,Z,O,O,O,O,Z,Z,O,O,O,O,
    O,O,Z,Z,Z,O,O,Z,Z,Z,Z,O,O,O,Z,O,O,O,O,O,Z,Z,O,O,O,Z,Z,O,O,O,O,O,O,O,Z,Z,Z,O,O,Z,Z,Z,Z,O,O,O,Z,O,O,O,O,O,Z,Z,O,O,O,Z,Z,O,O,O,O,O,
    O,Z,Z,O,Z,O,O,O,O,O,Z,Z,O,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,O,Z,Z,Z,Z,Z,O,O,O,O,Z,Z,O,Z,O,O,O,O,O,Z,Z,O,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,O,Z,Z,Z,Z,Z,O,O,O,
    Z,Z,O,O,Z,O,O,O,O,O,O,Z,Z,Z,Z,Z,O,O,O,O,O,O,Z,Z,Z,O,O,O,Z,Z,O,Z,Z,Z,O,O,Z,O,O,O,O,O,O,Z,Z,Z,Z,Z,O,O,O,O,O,O,Z,Z,Z,O,O,O,Z,Z,O,Z,
    Z,O,O,O,Z,O,O,O,O,O,O,O,O,O,Z,O,O,O,O,O,O,O,Z,Z,O,O,O,O,O,Z,Z,Z,Z,O,O,O,Z,O,O,O,O,O,O,O,O,O,Z,O,O,O,O,O,O,O,Z,Z,O,O,O,O,O,Z,Z,Z,
    Z,O,O,O,Z,O,O,O,O,O,O,O,O,O,Z,O,O,O,O,O,O,Z,Z,Z,Z,O,O,Z,Z,Z,O,O,Z,O,O,O,Z,O,O,O,O,O,O,O,O,O,Z,O,O,O,O,O,O,Z,Z,Z,Z,O,O,Z,Z,Z,O,O,
    Z,Z,O,O,Z,Z,O,O,O,O,O,O,O,O,Z,O,O,O,O,O,O,Z,O,O,Z,O,Z,Z,Z,O,O,O,Z,Z,O,O,Z,Z,O,O,O,O,O,O,O,O,Z,O,O,O,O,O,O,Z,O,O,Z,O,Z,Z,Z,O,O,O,
    O,Z,Z,O,O,Z,Z,O,O,O,O,O,O,Z,Z,Z,O,O,O,O,Z,Z,O,O,Z,Z,Z,O,O,O,O,O,O,Z,Z,O,O,Z,Z,O,O,O,O,O,O,Z,Z,Z,O,O,O,O,Z,Z,O,O,Z,Z,Z,O,O,O,O,O,
    O,O,Z,O,O,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,O,O,O,Z,O,O,O,O,O,O,O,O,O,Z,O,O,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,O,O,O,Z,O,O,O,O,O,O,O,
    O,O,Z,Z,Z,Z,O,O,O,O,Z,O,O,O,O,O,O,O,Z,Z,Z,O,O,O,Z,O,O,O,O,O,O,O,O,O,Z,Z,Z,Z,O,O,O,O,Z,O,O,O,O,O,O,O,Z,Z,Z,O,O,O,Z,O,O,O,O,O,O,O,
    O,O,O,O,Z,O,O,O,O,O,Z,Z,O,O,O,O,O,Z,Z,O,Z,O,O,O,Z,Z,O,O,O,O,O,O,O,O,O,O,Z,O,O,O,O,O,Z,Z,O,O,O,O,O,Z,Z,O,Z,O,O,O,Z,Z,O,O,O,O,O,O,
    Z,Z,Z,Z,Z,O,O,O,O,Z,Z,Z,Z,O,O,O,Z,Z,O,O,Z,O,O,O,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,O,O,O,O,Z,Z,Z,Z,O,O,O,Z,Z,O,O,Z,O,O,O,Z,Z,Z,Z,Z,Z,Z,Z,
    O,O,O,O,Z,Z,O,O,Z,Z,O,O,Z,Z,O,Z,Z,O,O,O,Z,O,O,O,Z,Z,O,O,O,O,Z,Z,O,O,O,O,Z,Z,O,O,Z,Z,O,O,Z,Z,O,Z,Z,O,O,O,Z,O,O,O,Z,Z,O,O,O,O,Z,Z,
    O,O,O,O,O,Z,Z,Z,Z,O,O,O,O,Z,Z,Z,O,O,O,O,Z,O,O,O,Z,O,O,O,O,O,O,Z,O,O,O,O,O,Z,Z,Z,Z,O,O,O,O,Z,Z,Z,O,O,O,O,Z,O,O,O,Z,O,O,O,O,O,O,Z,
    O,O,O,O,O,Z,O,Z,Z,O,O,O,O,O,Z,Z,O,O,O,O,Z,O,O,O,Z,O,O,O,O,O,O,Z,O,O,O,O,O,Z,O,Z,Z,O,O,O,O,O,Z,Z,O,O,O,O,Z,O,O,O,Z,O,O,O,O,O,O,Z,
    O,O,O,O,Z,Z,O,O,Z,Z,Z,O,O,Z,Z,Z,Z,O,O,O,Z,Z,O,O,Z,O,O,O,O,O,O,Z,O,O,O,O,Z,Z,O,O,Z,Z,Z,O,O,Z,Z,Z,Z,O,O,O,Z,Z,O,O,Z,O,O,O,O,O,O,Z,
    Z,Z,Z,Z,Z,Z,O,O,O,O,Z,Z,O,Z,O,O,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,O,O,O,O,Z,Z,Z,Z,Z,Z,Z,Z,O,O,O,O,Z,Z,O,Z,O,O,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,O,O,O,O,Z,Z,
    O,O,O,O,Z,Z,O,O,O,O,O,Z,Z,Z,O,O,O,O,O,Z,Z,O,Z,Z,Z,Z,Z,Z,Z,Z,Z,O,O,O,O,O,Z,Z,O,O,O,O,O,Z,Z,Z,O,O,O,O,O,Z,Z,O,Z,Z,Z,Z,Z,Z,Z,Z,Z,O,
    O,O,O,O,O,Z,O,O,O,O,O,O,O,Z,Z,O,O,O,Z,Z,O,O,Z,Z,O,O,Z,O,O,O,O,O,O,O,O,O,O,Z,O,O,O,O,O,O,O,Z,Z,O,O,O,Z,Z,O,O,Z,Z,O,O,Z,O,O,O,O,O,
    O,O,O,O,O,Z,O,O,O,O,O,O,O,O,Z,Z,O,Z,Z,O,O,O,Z,O,O,O,Z,Z,O,O,O,O,O,O,O,O,O,Z,O,O,O,O,O,O,O,O,Z,Z,O,Z,Z,O,O,O,Z,O,O,O,Z,Z,O,O,O,O,
    O,O,O,O,O,Z,O,O,O,O,O,O,Z,Z,O,Z,Z,Z,O,O,O,O,Z,O,O,O,O,Z,Z,O,O,O,O,O,O,O,O,Z,O,O,O,O,O,O,Z,Z,O,Z,Z,Z,O,O,O,O,Z,O,O,O,O,Z,Z,O,O,O,
    O,O,O,O,Z,Z,Z,O,O,O,O,Z,Z,O,O,O,O,Z,O,O,O,O,Z,O,O,O,O,O,Z,O,O,O,O,O,O,O,Z,Z,Z,O,O,O,O,Z,Z,O,O,O,O,Z,O,O,O,O,Z,O,O,O,O,O,Z,O,O,O,
    Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,O,O,O,O,O,Z,O,O,O,O,Z,O,O,O,O,O,Z,Z,O,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,O,O,O,O,O,Z,O,O,O,O,Z,O,O,O,O,O,Z,Z,O,Z,
    O,O,O,O,O,O,Z,Z,O,O,O,Z,Z,Z,Z,Z,Z,Z,O,O,O,Z,Z,Z,O,O,O,O,O,Z,Z,Z,O,O,O,O,O,O,Z,Z,O,O,O,Z,Z,Z,Z,Z,Z,Z,O,O,O,Z,Z,Z,O,O,O,O,O,Z,Z,Z,
    O,O,O,O,O,O,Z,O,O,O,O,O,O,O,O,O,O,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,O,O,O,O,O,O,Z,O,O,O,O,O,O,O,O,O,O,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,
    O,O,O,O,O,O,Z,O,O,O,O,O,O,O,O,O,O,Z,Z,O,O,O,O,O,O,O,O,O,O,O,Z,Z,O,O,O,O,O,O,Z,O,O,O,O,O,O,O,O,O,O,Z,Z,O,O,O,O,O,O,O,O,O,O,O,Z,Z,
    O,O,O,O,O,O,Z,Z,O,O,O,O,O,O,O,O,Z,Z,O,O,O,O,O,O,O,O,O,O,O,O,O,Z,O,O,O,O,O,O,Z,Z,O,O,O,O,O,O,O,O,Z,Z,O,O,O,O,O,O,O,O,O,O,O,O,O,Z,
    O,O,O,O,O,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,O,O,O,O,O,O,O,O,O,O,O,O,O,Z,O,O,O,O,O,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,O,O,O,O,O,O,O,O,O,O,O,O,O,Z,
    O,O,O,O,Z,Z,O,O,O,O,O,O,Z,Z,O,O,O,Z,Z,Z,Z,O,O,O,O,O,O,O,O,O,O,Z,O,O,O,O,Z,Z,O,O,O,O,O,O,Z,Z,O,O,O,Z,Z,Z,Z,O,O,O,O,O,O,O,O,O,O,Z,
    Z,Z,Z,Z,Z,O,O,O,O,O,O,O,O,Z,O,O,O,O,O,O,Z,Z,Z,O,O,O,O,O,O,O,O,Z,Z,Z,Z,Z,Z,O,O,O,O,O,O,O,O,Z,O,O,O,O,O,O,Z,Z,Z,O,O,O,O,O,O,O,O,Z,
    O,O,O,Z,Z,O,O,O,O,O,O,O,O,O,Z,O,O,O,O,O,O,Z,Z,O,O,Z,Z,Z,Z,O,O,O,O,O,O,Z,Z,O,O,O,O,O,O,O,O,O,Z,O,O,O,O,O,O,Z,Z,O,O,Z,Z,Z,Z,O,O,O,
    O,O,O,O,Z,Z,Z,O,O,O,O,O,O,O,Z,O,O,O,O,O,O,Z,O,O,O,O,O,Z,O,O,O,O,O,O,O,O,Z,Z,Z,O,O,O,O,O,O,O,Z,O,O,O,O,O,O,Z,O,O,O,O,O,Z,O,O,O,O,
    O,O,O,O,Z,O,Z,Z,O,O,O,O,O,O,Z,O,O,O,O,O,O,Z,O,O,O,O,Z,Z,O,O,O,O,O,O,O,O,Z,O,Z,Z,O,O,O,O,O,O,Z,O,O,O,O,O,O,Z,O,O,O,O,Z,Z,O,O,O,O,
    O,O,Z,Z,Z,O,O,Z,Z,Z,Z,O,O,O,Z,O,O,O,O,O,Z,Z,O,O,O,Z,Z,O,O,O,O,O,O,O,Z,Z,Z,O,O,Z,Z,Z,Z,O,O,O,Z,O,O,O,O,O,Z,Z,O,O,O,Z,Z,O,O,O,O,O,
    O,Z,Z,O,Z,O,O,O,O,O,Z,Z,O,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,O,Z,Z,Z,Z,Z,O,O,O,O,Z,Z,O,Z,O,O,O,O,O,Z,Z,O,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,O,Z,Z,Z,Z,Z,O,O,O,
    Z,Z,O,O,Z,O,O,O,O,O,O,Z,Z,Z,Z,Z,O,O,O,O,O,O,Z,Z,Z,O,O,O,Z,Z,O,Z,Z,Z,O,O,Z,O,O,O,O,O,O,Z,Z,Z,Z,Z,O,O,O,O,O,O,Z,Z,Z,O,O,O,Z,Z,O,Z,
    Z,O,O,O,Z,O,O,O,O,O,O,O,O,O,Z,O,O,O,O,O,O,O,Z,Z,O,O,O,O,O,Z,Z,Z,Z,O,O,O,Z,O,O,O,O,O,O,O,O,O,Z,O,O,O,O,O,O,O,Z,Z,O,O,O,O,O,Z,Z,Z,
    Z,O,O,O,Z,O,O,O,O,O,O,O,O,O,Z,O,O,O,O,O,O,Z,Z,Z,Z,O,O,Z,Z,Z,O,O,Z,O,O,O,Z,O,O,O,O,O,O,O,O,O,Z,O,O,O,O,O,O,Z,Z,Z,Z,O,O,Z,Z,Z,O,O,
    Z,Z,O,O,Z,Z,O,O,O,O,O,O,O,O,Z,O,O,O,O,O,O,Z,O,O,Z,O,Z,Z,Z,O,O,O,Z,Z,O,O,Z,Z,O,O,O,O,O,O,O,O,Z,O,O,O,O,O,O,Z,O,O,Z,O,Z,Z,Z,O,O,O,
    O,Z,Z,O,O,Z,Z,O,O,O,O,O,O,Z,Z,Z,O,O,O,O,Z,Z,O,O,Z,Z,Z,O,O,O,O,O,O,Z,Z,O,O,Z,Z,O,O,O,O,O,O,Z,Z,Z,O,O,O,O,Z,Z,O,O,Z,Z,Z,O,O,O,O,O,
    O,O,Z,O,O,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,O,O,O,Z,O,O,O,O,O,O,O,O,O,Z,O,O,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,O,O,O,Z,O,O,O,O,O,O,O,
    O,O,Z,Z,Z,Z,O,O,O,O,Z,O,O,O,O,O,O,O,Z,Z,Z,O,O,O,Z,O,O,O,O,O,O,O,O,O,Z,Z,Z,Z,O,O,O,O,Z,O,O,O,O,O,O,O,Z,Z,Z,O,O,O,Z,O,O,O,O,O,O,O,
    O,O,O,O,Z,O,O,O,O,O,Z,Z,O,O,O,O,O,Z,Z,O,Z,O,O,O,Z,Z,O,O,O,O,O,O,O,O,O,O,Z,O,O,O,O,O,Z,Z,O,O,O,O,O,Z,Z,O,Z,O,O,O,Z,Z,O,O,O,O,O,O,
    Z,Z,Z,Z,Z,O,O,O,O,Z,Z,Z,Z,O,O,O,Z,Z,O,O,Z,O,O,O,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,O,O,O,O,Z,Z,Z,Z,O,O,O,Z,Z,O,O,Z,O,O,O,Z,Z,Z,Z,Z,Z,Z,Z,
    O,O,O,O,Z,Z,O,O,Z,Z,O,O,Z,Z,O,Z,Z,O,O,O,Z,O,O,O,Z,Z,O,O,O,O,Z,Z,O,O,O,O,Z,Z,O,O,Z,Z,O,O,Z,Z,O,Z,Z,O,O,O,Z,O,O,O,Z,Z,O,O,O,O,Z,Z,
    O,O,O,O,O,Z,Z,Z,Z,O,O,O,O,Z,Z,Z,O,O,O,O,Z,O,O,O,Z,O,O,O,O,O,O,Z,O,O,O,O,O,Z,Z,Z,Z,O,O,O,O,Z,Z,Z,O,O,O,O,Z,O,O,O,Z,O,O,O,O,O,O,Z,
    O,O,O,O,O,Z,O,Z,Z,O,O,O,O,O,Z,Z,O,O,O,O,Z,O,O,O,Z,O,O,O,O,O,O,Z,O,O,O,O,O,Z,O,Z,Z,O,O,O,O,O,Z,Z,O,O,O,O,Z,O,O,O,Z,O,O,O,O,O,O,Z,
    O,O,O,O,Z,Z,O,O,Z,Z,Z,O,O,Z,Z,Z,Z,O,O,O,Z,Z,O,O,Z,O,O,O,O,O,O,Z,O,O,O,O,Z,Z,O,O,Z,Z,Z,O,O,Z,Z,Z,Z,O,O,O,Z,Z,O,O,Z,O,O,O,O,O,O,Z,
    Z,Z,Z,Z,Z,Z,O,O,O,O,Z,Z,O,Z,O,O,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,O,O,O,O,Z,Z,Z,Z,Z,Z,Z,Z,O,O,O,O,Z,Z,O,Z,O,O,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,O,O,O,O,Z,Z,
    O,O,O,O,Z,Z,O,O,O,O,O,Z,Z,Z,O,O,O,O,O,Z,Z,O,Z,Z,Z,Z,Z,Z,Z,Z,Z,O,O,O,O,O,Z,Z,O,O,O,O,O,Z,Z,Z,O,O,O,O,O,Z,Z,O,Z,Z,Z,Z,Z,Z,Z,Z,Z,O,
    O,O,O,O,O,Z,O,O,O,O,O,O,O,Z,Z,O,O,O,Z,Z,O,O,Z,Z,O,O,Z,O,O,O,O,O,O,O,O,O,O,Z,O,O,O,O,O,O,O,Z,Z,O,O,O,Z,Z,O,O,Z,Z,O,O,Z,O,O,O,O,O,
    O,O,O,O,O,Z,O,O,O,O,O,O,O,O,Z,Z,O,Z,Z,O,O,O,Z,O,O,O,Z,Z,O,O,O,O,O,O,O,O,O,Z,O,O,O,O,O,O,O,O,Z,Z,O,Z,Z,O,O,O,Z,O,O,O,Z,Z,O,O,O,O,
    O,O,O,O,O,Z,O,O,O,O,O,O,Z,Z,O,Z,Z,Z,O,O,O,O,Z,O,O,O,O,Z,Z,O,O,O,O,O,O,O,O,Z,O,O,O,O,O,O,Z,Z,O,Z,Z,Z,O,O,O,O,Z,O,O,O,O,Z,Z,O,O,O,
    O,O,O,O,Z,Z,Z,O,O,O,O,Z,Z,O,O,O,O,Z,O,O,O,O,Z,O,O,O,O,O,Z,O,O,O,O,O,O,O,Z,Z,Z,O,O,O,O,Z,Z,O,O,O,O,Z,O,O,O,O,Z,O,O,O,O,O,Z,O,O,O,
    Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,O,O,O,O,O,Z,O,O,O,O,Z,O,O,O,O,O,Z,Z,O,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,O,O,O,O,O,Z,O,O,O,O,Z,O,O,O,O,O,Z,Z,O,Z,
    O,O,O,O,O,O,Z,Z,O,O,O,Z,Z,Z,Z,Z,Z,Z,O,O,O,Z,Z,Z,O,O,O,O,O,Z,Z,Z,O,O,O,O,O,O,Z,Z,O,O,O,Z,Z,Z,Z,Z,Z,Z,O,O,O,Z,Z,Z,O,O,O,O,O,Z,Z,Z,
    O,O,O,O,O,O,Z,O,O,O,O,O,O,O,O,O,O,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,O,O,O,O,O,O,Z,O,O,O,O,O,O,O,O,O,O,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,
    O,O,O,O,O,O,Z,O,O,O,O,O,O,O,O,O,O,Z,Z,O,O,O,O,O,O,O,O,O,O,O,Z,Z,O,O,O,O,O,O,Z,O,O,O,O,O,O,O,O,O,O,Z,Z,O,O,O,O,O,O,O,O,O,O,O,Z,Z,
    O,O,O,O,O,O,Z,Z,O,O,O,O,O,O,O,O,Z,Z,O,O,O,O,O,O,O,O,O,O,O,O,O,Z,O,O,O,O,O,O,Z,Z,O,O,O,O,O,O,O,O,Z,Z,O,O,O,O,O,O,O,O,O,O,O,O,O,Z,
    O,O,O,O,O,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,O,O,O,O,O,O,O,O,O,O,O,O,O,Z,O,O,O,O,O,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,O,O,O,O,O,O,O,O,O,O,O,O,O,Z,
    O,O,O,O,Z,Z,O,O,O,O,O,O,Z,Z,O,O,O,Z,Z,Z,Z,O,O,O,O,O,O,O,O,O,O,Z,O,O,O,O,Z,Z,O,O,O,O,O,O,Z,Z,O,O,O,Z,Z,Z,Z,O,O,O,O,O,O,O,O,O,O,Z,
    Z,Z,Z,Z,Z,O,O,O,O,O,O,O,O,Z,O,O,O,O,O,O,Z,Z,Z,O,O,O,O,O,O,O,O,Z,Z,Z,Z,Z,Z,O,O,O,O,O,O,O,O,Z,O,O,O,O,O,O,Z,Z,Z,O,O,O,O,O,O,O,O,Z,
};

// -----------------------------------------------------------------------

static void render_walls(void)
{
    int16_t lineHeight;
    int16_t drawStart;
    int16_t drawEnd;
    int16_t stepX;
    int16_t stepY;
    int16_t mapX;
    int16_t mapY;
    int16_t x;

    int8_t side = 0;

    double sideDistX;
    double sideDistY;
    double wallDist;
    double cameraX;
    double rayDirX;
    double rayDirY;
    double deltaX;
    double deltaY;

    double wallX;
    int texX;
    int texY;
    double step;
    double texPos;
    int y;


    for(x = 1; x < fb_width; x++)
    {
        //  calculate ray position and direction

        cameraX = (2 * x) / ((double)fb_width) - 1;   // x-coordinate in camera space

        rayDirX = dirX + (planeX * cameraX);
        rayDirY = dirY + (planeY * cameraX);

        // which box of the map we're in
        mapX = (int) posX;
        mapY = (int) posY;

        // length of ray from one x or y-side to next x or y-side
        deltaX = fabs(1 / rayDirX);
        deltaY = fabs(1 / rayDirY);

        // calculate step and initial sideDist
        if (rayDirX < 0)
        {
            stepX = -1;
            sideDistX = (posX - mapX) * deltaX;
        }
        else
        {
            stepX = 1;
            sideDistX = (mapX + 1.0 - posX) * deltaX;
        }

        if (rayDirY < 0)
        {
            stepY = -1;
            sideDistY = (posY - mapY) * deltaY;
        }
        else
        {
            stepY = 1;
            sideDistY = (mapY + 1.0 - posY) * deltaY;
        }

        // perform DDA

        do
        {
             if (sideDistX < sideDistY)
             {
                 sideDistX += deltaX;
                 mapX += stepX;
                 side = 0;
             }
             else
             {
                 sideDistY += deltaY;
                 mapY += stepY;
                 side = 1;
             }
        } while (worldMap[mapX][mapY] == 0);

        if (side == 0)
        {
            wallDist = (mapX - posX + (1 - stepX) / 2) / rayDirX;
            dark_map[x] = 1;
        }
        else
        {
             wallDist = (mapY - posY + (1 - stepY) / 2) / rayDirY;
             dark_map[x] = 2;
        }

        // Calculate height of line to draw on screen
        lineHeight = (fb_height / wallDist);
        lineHeight &= ~1;      // ensure symetrical jaggies

        // calculate lowest and highest pixel to fill in current stripe
        drawStart = (fb_height / 2) - (lineHeight / 2);
        while (drawStart < 1)
        {
            drawStart++;
            lineHeight--;
        }

        // drawEnd = (lineHeight / 2) + (fb_height / 2);
        drawEnd = drawStart + lineHeight;
        while (drawEnd > fb_height - 2)
        {
            drawEnd--;
            lineHeight--;
        }

        wallX = (side == 0)
           ? posY + (wallDist * rayDirY)
           : posX + (wallDist * rayDirX);

        wallX -= floor(wallX);
        texX = (int)(wallX * (double)64);

        if (side == 0 && rayDirX > 0) texX = 64 - texX -1;
        if (side == 1 && rayDirY < 0) texX = 64 - texX -1;

        step = 64.0 / lineHeight;

        texPos = (drawStart - (fb_height / 2) + (lineHeight / 2)) * step;

        for (y = drawStart; y < drawEnd; y++)
        {
            texY = (int)texPos & 0x1f;
            texPos += step;
            color_map[x] = (23 - (int)(wallDist) % 23) / 2;
            fb[y * fb_width + x] = wall[(texY * 64) + texX];
        }
    }
}

// -----------------------------------------------------------------------

void process_key(uint8_t keypress)
{
    double oldPlaneX ;
    double oldDirX;
    double x, y;

    // move forward if no wall in front of you

    if (keypress == 'w')
    {
        x = posX + dirX * moveSpeed;
        y = posY + dirY * moveSpeed;

        if (worldMap[(int)x][(int)y] == 0)
        {
            posX = x;
            posY = y;
        }
    }

    // move backwards if no wall behind you

    if (keypress == 's')
    {
       x = posX - dirX * moveSpeed;
       y = posY - dirY * moveSpeed;

        if(worldMap[(int)x][(int)y] == 0)
        {
            posX = x;
            posY = y;
        }
    }

    oldDirX   = dirX;
    oldPlaneX = planeX;

    // both camera direction and camera plane must be rotated
    // rotate to the right

    if (keypress == 'd')
    {
        //both camera direction and camera plane must be rotated
        dirX   = dirX      * cos(-rotSpeed) - dirY   * sin(-rotSpeed);
        dirY   = oldDirX   * sin(-rotSpeed) + dirY   * cos(-rotSpeed);
        planeX = planeX    * cos(-rotSpeed) - planeY * sin(-rotSpeed);
        planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
    }
    //rotate to the left
    if (keypress == 'a')
    {
        //both camera direction and camera plane must be rotated
        dirX      = dirX      * cos(rotSpeed) - dirY   * sin(rotSpeed);
        dirY      = oldDirX   * sin(rotSpeed) + dirY   * cos(rotSpeed);
        planeX    = planeX    * cos(rotSpeed) - planeY * sin(rotSpeed);
        planeY    = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
    }
}

// -----------------------------------------------------------------------

static void draw_walls(void)
{
    uint16_t c;
    uint8_t pixle;
    uint8_t x, y;

    cell_t *cell;

    uint32_t offset;
    uint8_t color;

    uC_braille_1(win, braile_data, fb, fb_width, fb_height);

    for (x = 0; x < win->width; x++)
    {
        color = color_map[x * 2];

        if (dark_map[x * 2] != 2) { color *= 2; }

        uC_win_set_gray_fg(win, color);

        for (y = 0; y < win->height; y++)
        {
            c = braile_data[(y * win->width) + x];

            cell = uC_win_peek_xy(win, x, y);

            if (c != 0x2800)
            {
                uC_win_cup(win, x, y);
                uC_win_emit(win, c);
            }
        }
    }
}

// -----------------------------------------------------------------------

void raycast(void)
{
    uint8_t keypress;
    struct timeval tv;

    int fps;

    int old_count  = 0;
    int framecount = 0;
    int time       = 0;
    int old_time;
    int frame_time = 1;

    gettimeofday(&tv, NULL);
    old_time = tv.tv_sec;

    fb_width     = win->width  * 2;
    fb_height    = win->height * 4;

    braile_data  = calloc((win->height    * win->width), 2);
    fb           = calloc(fb_height       * fb_width,    1);

    // wall colors and shading
    dark_map     = calloc(fb_width, 1);
    color_map    = calloc(fb_width, 1);

    for (;;)
    {
        memset(dark_map, 0, fb_width);
        memset(fb, 0, fb_width * fb_height);

        memset(color_map, 0, fb_width);

        uC_win_clear(win);
        render_walls();

        draw_walls();

        snprintf(status, 32, "FPS: %d", fps);
        uC_set_status(status_win, status);

        uC_scr_draw_screen(scr);

        framecount++;

        // timing for input and FPS counter

        gettimeofday(&tv, NULL);
        time = tv.tv_sec;

        if (old_time != time)
        {
            old_time = time;
            fps = framecount - old_count;
            old_count = framecount;
            frame_time = (1 / fps);
        }

        if (frame_time == 0) { frame_time = 1; }

        moveSpeed = frame_time * 0.2;
        rotSpeed  = frame_time * 0.08;

        if (uC_test_keys() != 0)
        {
            keypress = uC_key();

            if (keypress == 0x1b)
            {
                break;
            }
            process_key(keypress);
        }
    }
}

// -----------------------------------------------------------------------

static void exit_prog(void)
{
    uC_set_key(0x1b);
}

// -----------------------------------------------------------------------

static uC_switch_t menu_vectors[] =
{
    { 0x8d9c616c, exit_prog }
};

#define VCOUNT sizeof(menu_vectors) / sizeof(menu_vectors[0])

// -----------------------------------------------------------------------

opt_t menu_address_cb(int32_t hash)
{
    int16_t i;
    uC_switch_t *s = menu_vectors;

    for(i = 0; i < VCOUNT; i++)
    {
        if(hash == s->option)
        {
            return s->vector;
        }
        s++;
    }

    return NULL;
}

// -----------------------------------------------------------------------

int main(void)
{
    uC_list_node_t *n;

    uCurses_init();
    uC_json_file_create_ui("json/dots.json", menu_address_cb);
    uC_menu_init();

    scr = active_screen;
    n   = scr->windows.head;
    win = n->payload;

    status_win = uC_add_status(scr, 32, 55, 0);
    uC_win_printf(status_win, "%fs%bs%0", 9, 3);

    uC_set_status(status_win, status);
    uC_clr_status(status_win);

    raycast();

    uC_scr_close(active_screen);

    uC_console_reset_attrs();
    uC_clear();
    uC_cup(10, 0);

    uCurses_deInit();

    printf("Au revoir!\n");

    return 0;
}

// =======================================================================
