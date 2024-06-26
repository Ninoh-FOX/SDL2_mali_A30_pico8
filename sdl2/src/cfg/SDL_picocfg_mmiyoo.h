#ifndef __SDL_PICOCFG_MMIYOO_H__
#define __SDL_PICOCFG_MMIYOO_H__

#include <limits.h>
#include <SDL2/SDL.h>
#include "../../cfg/SDL_picocfg_mmiyoo.h"

#ifndef MAX_PATH
    #define MAX_PATH 255
#endif

#define PICO_CFG_PATH                       "cfg/onioncfg.json"
#define BUFFER_SIZE                         PATH_MAX + 20
#define MMIYOO_DEFAULT_KEY_L2               SDLK_d
#define MMIYOO_DEFAULT_KEY_L1               SDLK_d
#define MMIYOO_DEFAULT_KEY_UpDpad           SDLK_UP
#define MMIYOO_DEFAULT_KEY_DownDpad         SDLK_DOWN
#define MMIYOO_DEFAULT_KEY_LeftDpad         SDLK_LEFT
#define MMIYOO_DEFAULT_KEY_RightDpad        SDLK_RIGHT
#define MMIYOO_DEFAULT_KEY_R2               SDLK_d
#define MMIYOO_DEFAULT_KEY_R1               SDLK_d
#define MMIYOO_DEFAULT_KEY_A                SDLK_z
#define MMIYOO_DEFAULT_KEY_B                SDLK_x
#define MMIYOO_DEFAULT_KEY_X                SDLK_ESCAPE
#define MMIYOO_DEFAULT_KEY_Y                SDLK_d
#define MMIYOO_DEFAULT_KEY_Select           SDLK_m
#define MMIYOO_DEFAULT_KEY_Start            SDLK_RETURN
#define MMIYOO_DEFAULT_KEY_MENU             SDLK_ESCAPE
#define MMIYOO_DEFAULT_CPU_CLOCK            1300
#define MMIYOO_MAX_CPU_CLOCK                1800
#define MMIYOO_MIN_CPU_CLOCK                600
#define MMIYOO_DEFAULT_CPU_CLOCK_INCREMENT  25
#define MMIYOO_MAX_CPU_CLOCK_INCREMENT      100
#define MMIYOO_DEFAULT_SCALE_FACTOR         2
#define MMIYOO_DEFAULT_ACCELERATION         2.0
#define MMIYOO_DEFAULT_ACCELERATION_RATE    1.5
#define MMIYOO_DEFAULT_MAX_ACCELERATION     5
#define MMIYOO_DEFAULT_INCREMENT_MODIFIER   0.1
#define MMIYOO_DEFAULT_MOUSE_HOTKEY         0
#define MMIYOO_DEFAULT_MOUSE_ICON           0
#define MMIYOO_DEFAULT_MOUSE_MINX           40
#define MMIYOO_DEFAULT_MOUSE_MINY           0
#define MMIYOO_DEFAULT_MOUSE_MAXX           280
#define MMIYOO_DEFAULT_MOUSE_MAXY           236

#define DEFAULT_DIGIT_PATH                  "res/digit"
#define DEFAULT_BEZEL_PATH                  "res/bezel/standard/"
#define DEFAULT_INTEGER_BEZEL_PATH          "res/bezel/integer_scaled/"
#define DEFAULT_BEZEL_ID                    0
#define DEFAULT_INTEGER_BEZEL_ID            0
#define MAX_BEZELS                          256
#define DEFAULT_MOUSE_ICON                  "res/icon/mouse.png"

SDL_Keycode stringToKeycode(const char *keyString);
int picoConfigRead(void);
int picoConfigWrite(void);

typedef struct _CUSTKEY {
    SDL_Keycode A, B, X, Y, L1, L2, R1, R2, LeftDpad, RightDpad, UpDpad, DownDpad, Start, Select, Menu;
} CUSTKEY;

typedef struct _MOUSE {
    int scaleFactor;
    float acceleration;
    float accelerationRate;
    float maxAcceleration;
    float incrementModifier;
    int disableMouseHotkey;
    int disableMouseIcon;
    int minx;
    int miny;
    int maxx;
    int maxy;
} MOUSE;

typedef struct _STATE {
    int oc_changed;
    int oc_decay;
    int push_update;
    int refresh_bezel;
    int draw_mouse;
    int screen_scaling;
    int alpha_draw;
    int integer_bezel;
    int wait_frame;
    int lastMouseX;
    int lastMouseY;
} STATE;

typedef struct _PERF {
    int cpuclock;
    int cpuclockincrement;
    int maxcpu;
    int mincpu;
} PERF;

typedef struct _RES {
    SDL_Surface *bezel[MAX_BEZELS];
    SDL_Surface *integer_bezel[MAX_BEZELS];
    SDL_Surface *digit[10];
    SDL_Surface *mouse_indicator;
    int current_bezel_id;
    int current_integer_bezel_id;
    int total_bezels_loaded;
    int total_integer_bezels_loaded;
    char digit_path[PATH_MAX];
    char bezel_path[PATH_MAX];
    char bezel_int_path[PATH_MAX];
} RES;

typedef struct _PICO {
    char cfg_path[MAX_PATH];
    CUSTKEY customkey;
    MOUSE mouse;
    STATE state;
    RES res;
    PERF perf;
} PICO;

#endif