/*
  Customized version for Miyoo-Mini handheld.
  Only tested under Miyoo-Mini stock OS (original firmware) with Parasyte compatible layer.

  Copyright (C) 1997-2022 Sam Lantinga <slouken@libsdl.org>
  Copyright (C) 2022-2022 Steward Fu <steward.fu@gmail.com>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "../../SDL_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <dirent.h>
#include <linux/input.h>

#include "../../events/SDL_events_c.h"
#include "../../core/linux/SDL_evdev.h"
#include "../../thread/SDL_systhread.h"

#include "SDL_video_a30.h"
#include "SDL_event_a30.h"

#define UP      103
#define DOWN    108
#define LEFT    105
#define RIGHT   106
#define A       57
#define B       29
#define X       42
#define Y       56
#define L1      15
#define R1      14
#define L2      18
#define R2      20
#define SELECT  97
#define START   28
#define MENU    1
#define VOLUP   115
#define VOLDOWN 114

A30_EventInfo evt = {0};
extern struct _video vid;

static int running = 0;
static int event_fd = -1;
static int lower_speed = 0;
static SDL_sem *event_sem = NULL;
static SDL_Thread *thread = NULL;
static uint32_t pre_ticks = 0;
static uint32_t pre_keypad_bitmaps = 0;

SDL_Scancode code[] = {
    SDLK_UP,            // UP
    SDLK_DOWN,          // DOWN
    SDLK_LEFT,          // LEFT
    SDLK_RIGHT,         // RIGHT
    SDLK_SPACE,         // A
    SDLK_LCTRL,         // B
    SDLK_LSHIFT,        // X
    SDLK_LALT,          // Y
    SDLK_TAB,           // L1
    SDLK_BACKSPACE,     // R1
    SDLK_e,             // L2
    SDLK_t,             // R2
    SDLK_RCTRL,         // SELECT
    SDLK_RETURN,        // START
    SDLK_ESCAPE,        // MENU
    SDLK_POWER,         // POWER
};

void A30_JoystickUpdate(SDL_Joystick *joystick);

static void check_mouse_pos(void)
{
    if (evt.mouse.y < evt.mouse.miny) {
        evt.mouse.y = evt.mouse.miny;
    }
    if (evt.mouse.y > evt.mouse.maxy) {
        evt.mouse.y = evt.mouse.maxy;
    }
    if (evt.mouse.x < evt.mouse.minx) {
        evt.mouse.x = evt.mouse.minx;
    }
    if (evt.mouse.x >= evt.mouse.maxx) {
        evt.mouse.x = evt.mouse.maxx;
    }
}

static int get_move_interval(int type)
{
    float move = 0.0;
    long yv = 35000;
    long xv = 30000;

    if (lower_speed) {
        yv*= 2;
        xv*= 2;
    }

    move = ((float)clock() - pre_ticks) / ((type == 0) ? xv : yv);
    if (move <= 0.0) {
        move = 1.0;
    }
    return (int)(1.0 * move);
}

static void set_key(uint32_t bit, int val)
{
    if (val) {
        evt.keypad.bitmaps|= (1 << bit);
    }
    else {
        evt.keypad.bitmaps&= ~(1 << bit);
    }
}

int EventUpdate(void *data)
{
    struct input_event ev = {0};
	
	uint32_t up = UP;
    uint32_t down = DOWN;
    uint32_t left = LEFT;
    uint32_t right = RIGHT;

    while (running) {
        SDL_SemWait(event_sem);
		
		up = UP;
        down = DOWN;
        left = LEFT;
        right = RIGHT;

        // Leer eventos del dispositivo de entrada
        if (event_fd > 0 && read(event_fd, &ev, sizeof(struct input_event))) {
            if (ev.type == EV_KEY && ev.value != 2) {
				 if (ev.code == up)      { set_key(MYKEY_UP,    ev.value); }
                 if (ev.code == down)    { set_key(MYKEY_DOWN,  ev.value); }
                 if (ev.code == left)    { set_key(MYKEY_LEFT,  ev.value); }
                 if (ev.code == right)   { set_key(MYKEY_RIGHT, ev.value); }
				
                // Procesar eventos de teclado y cambiar mapeo dinámicamente para A y B
                switch (ev.code) {
                    case L1:
						if (ev.value) {
                            SDL_SendKeyboardKey(SDL_PRESSED, SDL_GetScancodeFromKey(SDLK_PAGEUP));
                        } else {
                            SDL_SendKeyboardKey(SDL_RELEASED, SDL_GetScancodeFromKey(SDLK_PAGEUP));
                        }
                        //set_key(MYKEY_L1, ev.value);
                        break;
                    case R1:
						if (ev.value) {
                            SDL_SendKeyboardKey(SDL_PRESSED, SDL_GetScancodeFromKey(SDLK_PAGEDOWN));
                        } else {
                            SDL_SendKeyboardKey(SDL_RELEASED, SDL_GetScancodeFromKey(SDLK_PAGEDOWN));
                        }
                        //set_key(MYKEY_R1, ev.value);
                        break;
                    case L2:
						if (evt.mode == A30_MOUSE_MODE) {
                            SDL_SendMouseButton(vid.win, 0, ev.value ? SDL_PRESSED : SDL_RELEASED, SDL_BUTTON_RIGHT);
                        } else {
						if (ev.value) {
                            SDL_SendKeyboardKey(SDL_PRESSED, SDL_GetScancodeFromKey(SDLK_PAGEUP));
                        } else {
                            SDL_SendKeyboardKey(SDL_RELEASED, SDL_GetScancodeFromKey(SDLK_PAGEUP));
                        }
						}
                        //set_key(MYKEY_L2, ev.value);
                        break;
                    case R2:
						if (evt.mode == A30_MOUSE_MODE) {
                            SDL_SendMouseButton(vid.win, 0, ev.value ? SDL_PRESSED : SDL_RELEASED, SDL_BUTTON_LEFT);
                        } else {
						if (ev.value) {
                            SDL_SendKeyboardKey(SDL_PRESSED, SDL_GetScancodeFromKey(SDLK_PAGEDOWN));
                        } else {
                            SDL_SendKeyboardKey(SDL_RELEASED, SDL_GetScancodeFromKey(SDLK_PAGEDOWN));
                        }
						}
                        //set_key(MYKEY_R2, ev.value);
                        break;
					case A:
                        if (ev.value) { // Si se presiona A
                            SDL_SendKeyboardKey(SDL_PRESSED, SDL_GetScancodeFromKey(SDLK_x)); // Enviar evento de tecla X presionada
                        } else { // Si se libera A
                            SDL_SendKeyboardKey(SDL_RELEASED, SDL_GetScancodeFromKey(SDLK_x)); // Enviar evento de tecla X liberada
                        }
						//set_key(MYKEY_A, ev.value);
                        break;
                    case B:
                        if (ev.value) { // Si se presiona B
                            SDL_SendKeyboardKey(SDL_PRESSED, SDL_GetScancodeFromKey(SDLK_z)); // Enviar evento de tecla Z presionada
                        } else { // Si se libera B
                            SDL_SendKeyboardKey(SDL_RELEASED, SDL_GetScancodeFromKey(SDLK_z)); // Enviar evento de tecla Z liberada
                        }
						//set_key(MYKEY_B, ev.value);
                        break;
					case X:
						if (ev.value) { // Si se presiona X
                            SDL_SendKeyboardKey(SDL_PRESSED, SDL_GetScancodeFromKey(SDLK_ESCAPE)); // Enviar evento de tecla ESCAPE presionada
                        } else { // Si se libera X
                            SDL_SendKeyboardKey(SDL_RELEASED, SDL_GetScancodeFromKey(SDLK_ESCAPE)); // Enviar evento de tecla ESCAPE liberada
                        }
						//set_key(MYKEY_X, ev.value);
						break;
					case Y:
						//set_key(MYKEY_Y, ev.value);
						break;
                    case SELECT:
						if (ev.value) { // Si se presiona SELECT
        					// Alternar entre modo teclado y modo ratón
        					evt.mode = (evt.mode == A30_KEYPAD_MODE) ? A30_MOUSE_MODE : A30_KEYPAD_MODE;
    					}
                        //set_key(MYKEY_SELECT, ev.value);
                        break;
                    case START:
                        set_key(MYKEY_START, ev.value);
                        break;
                    case MENU:
                        set_key(MYKEY_MENU, ev.value);
                        break;
                    case VOLUP:
                        set_key(MYKEY_VOLUP, ev.value);
                        break;
                    case VOLDOWN:
                        set_key(MYKEY_VOLDOWN, ev.value);
                        break;
                }
            }

            // Reiniciar el temporizador si no se presiona ninguna tecla
            if (!(evt.keypad.bitmaps & 0x0f)) {
                pre_ticks = clock();
            }
#if USE_MYJOY
            A30_JoystickUpdate(NULL);
#endif
        }
        SDL_SemPost(event_sem);
        usleep(1000000 / 60);
    }
    
    return 0;
}

void A30_EventInit(void)
{
    pre_keypad_bitmaps = 0;
    memset(&evt, 0, sizeof(evt));
    evt.mouse.minx = 0;
    evt.mouse.miny = 0;
    evt.mouse.maxx = LCD_H;
    evt.mouse.maxy = LCD_W;
    evt.mouse.x = (evt.mouse.maxx - evt.mouse.minx) / 2;
    evt.mouse.y = (evt.mouse.maxy - evt.mouse.miny) / 2;
    evt.mode = A30_KEYPAD_MODE;

    event_fd = open("/dev/input/event3", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
    if(event_fd < 0){
        printf(PREFIX"Failed to open /dev/input/event3\n");
    }

    event_sem = SDL_CreateSemaphore(1);
    if(event_sem == NULL) {
        printf(PREFIX"Failed to create input semaphore");
    }

    if (event_sem != NULL) {
        running = 1;
        if((thread = SDL_CreateThreadInternal(EventUpdate, "A30InputThread", 4096, NULL)) == NULL) {
            printf(PREFIX"Failed to create input thread");
        }
    }
}

void A30_EventDeinit(void)
{
    running = 0;
    SDL_WaitThread(thread, NULL);
    SDL_DestroySemaphore(event_sem);
    if(event_fd > 0) {
        close(event_fd);
        event_fd = -1;
    }
}

void A30_PumpEvents(_THIS)
{
    SDL_SemWait(event_sem);
    if (evt.mode == A30_KEYPAD_MODE) {
        if (pre_keypad_bitmaps != evt.keypad.bitmaps) {
            int cc = 0;
            uint32_t bit = 0;
            uint32_t changed = pre_keypad_bitmaps ^ evt.keypad.bitmaps;

            for (cc=0; cc<=MYKEY_LAST_BITS; cc++) {
                bit = 1 << cc;
                if (changed & bit) {
                    SDL_SendKeyboardKey((evt.keypad.bitmaps & bit) ? SDL_PRESSED : SDL_RELEASED, SDL_GetScancodeFromKey(code[cc]));
                }
            }
            pre_keypad_bitmaps = evt.keypad.bitmaps;
        }
    }
    else {
        int updated = 0;
        if (pre_keypad_bitmaps != evt.keypad.bitmaps) {
            uint32_t cc = 0;
            uint32_t bit = 0;
            uint32_t changed = pre_keypad_bitmaps ^ evt.keypad.bitmaps;

          /*  if (changed & (1 << MYKEY_A)) {
                SDL_SendMouseButton(vid.win, 0, (evt.keypad.bitmaps & (1 << MYKEY_A)) ? SDL_PRESSED : SDL_RELEASED, SDL_BUTTON_LEFT);
            }
			
			if (changed & (1 << MYKEY_B)) {
                SDL_SendMouseButton(vid.win, 0, (evt.keypad.bitmaps & (1 << MYKEY_B)) ? SDL_PRESSED : SDL_RELEASED, SDL_BUTTON_RIGHT);
            } */
			
			if (changed & (1 << MYKEY_L2)) {
                SDL_SendMouseButton(vid.win, 0, (evt.keypad.bitmaps & (1 << MYKEY_L2)) ? SDL_PRESSED : SDL_RELEASED, SDL_BUTTON_RIGHT);
            }

            if (changed & (1 << MYKEY_R2)) {
                SDL_SendMouseButton(vid.win, 0, (evt.keypad.bitmaps & (1 << MYKEY_R2)) ? SDL_PRESSED : SDL_RELEASED, SDL_BUTTON_LEFT);
            }

            for (cc=0; cc<=MYKEY_LAST_BITS; cc++) {
                bit = 1 << cc;
                if ((cc == MYKEY_FF) || (cc == MYKEY_QSAVE) || (cc == MYKEY_QLOAD) || (cc == MYKEY_EXIT) || (cc == MYKEY_R2)) {
                    if (changed & bit) {
                        SDL_SendKeyboardKey((evt.keypad.bitmaps & bit) ? SDL_PRESSED : SDL_RELEASED, SDL_GetScancodeFromKey(code[cc]));
                    }
                }
                if (cc == MYKEY_R1) {
                    if (changed & bit) {
                        lower_speed = (evt.keypad.bitmaps & bit);
                    }
                }
            }
        }

        if (evt.keypad.bitmaps & (1 << MYKEY_UP)) {
            updated = 1;
            evt.mouse.y-= get_move_interval(1);
        }
        if (evt.keypad.bitmaps & (1 << MYKEY_DOWN)) {
            updated = 1;
            evt.mouse.y+= get_move_interval(1);
        }
        if (evt.keypad.bitmaps & (1 << MYKEY_LEFT)) {
            updated = 1;
            evt.mouse.x-= get_move_interval(0);
        }
        if (evt.keypad.bitmaps & (1 << MYKEY_RIGHT)) {
            updated = 1;
            evt.mouse.x+= get_move_interval(0);
        }
        check_mouse_pos();

        if(updated){
            SDL_SendMouseMotion(vid.win, 0, 0, evt.mouse.x, evt.mouse.y);
        }
        pre_keypad_bitmaps = evt.keypad.bitmaps;
    }
    SDL_SemPost(event_sem);
}

