#include <string.h>
#include <stdio.h> 
#include <unistd.h>
#include <json-c/json.h>

#include <SDL2/SDL.h>
#include "../../cfg/SDL_picocfg_mmiyoo.h"

PICO pico = {0};
int cpuclock = 0;

void picoSetAllDefaults() {
    pico.mouse.scaleFactor = MMIYOO_DEFAULT_SCALE_FACTOR;
    pico.mouse.acceleration = MMIYOO_DEFAULT_ACCELERATION;
    pico.mouse.accelerationRate = MMIYOO_DEFAULT_ACCELERATION_RATE;
    pico.mouse.maxAcceleration = MMIYOO_DEFAULT_MAX_ACCELERATION;
    pico.mouse.incrementModifier = MMIYOO_DEFAULT_INCREMENT_MODIFIER;
    pico.perf.cpuclock = MMIYOO_DEFAULT_CPU_CLOCK;
    pico.perf.cpuclockincrement = MMIYOO_DEFAULT_CPU_CLOCK_INCREMENT;
    pico.customkey.A = MMIYOO_DEFAULT_KEY_A;
    pico.customkey.B = MMIYOO_DEFAULT_KEY_B;
    pico.customkey.X = MMIYOO_DEFAULT_KEY_X;
    pico.customkey.Y = MMIYOO_DEFAULT_KEY_Y;
    pico.customkey.L1 = MMIYOO_DEFAULT_KEY_L1;
    pico.customkey.L2 = MMIYOO_DEFAULT_KEY_L2;
    pico.customkey.R1 = MMIYOO_DEFAULT_KEY_R1;
    pico.customkey.R2 = MMIYOO_DEFAULT_KEY_R2;
    pico.customkey.LeftDpad = MMIYOO_DEFAULT_KEY_LeftDpad;
    pico.customkey.RightDpad = MMIYOO_DEFAULT_KEY_RightDpad;
    pico.customkey.UpDpad = MMIYOO_DEFAULT_KEY_UpDpad;
    pico.customkey.DownDpad = MMIYOO_DEFAULT_KEY_DownDpad;
    pico.customkey.Start = MMIYOO_DEFAULT_KEY_Start;
    pico.customkey.Select = MMIYOO_DEFAULT_KEY_Select;
    pico.customkey.Menu = MMIYOO_DEFAULT_KEY_MENU;
    pico.res.current_bezel_id = DEFAULT_BEZEL_ID;
    
    strncpy(pico.res.digit_path, DEFAULT_DIGIT_PATH, PATH_MAX);
    pico.res.digit_path[PATH_MAX - 1] = '\0';
    strncpy(pico.res.bezel_path, DEFAULT_BEZEL_PATH, PATH_MAX);
    pico.res.bezel_path[PATH_MAX - 1] = '\0';
}

int cfgReadRes(struct json_object *jfile) {
    struct json_object *jbezel = NULL;
    struct json_object *jval = NULL;

    if (!json_object_object_get_ex(jfile, "bezel", &jbezel) || !jbezel) {
        printf("bezel settings not found in json file. Using defaults.\n");
        pico.res.current_bezel_id = DEFAULT_BEZEL_ID;
        pico.res.current_integer_bezel_id = DEFAULT_INTEGER_BEZEL_ID; 
        snprintf(pico.res.bezel_path, PATH_MAX, "%s", DEFAULT_BEZEL_PATH);
        snprintf(pico.res.bezel_int_path, PATH_MAX, "%s", DEFAULT_INTEGER_BEZEL_PATH);
        snprintf(pico.res.digit_path, PATH_MAX, "%s", DEFAULT_DIGIT_PATH);
        return 0;
    }
    
    json_object_object_get_ex(jbezel, "current_bezel", &jval);
    if (jval) {
        pico.res.current_bezel_id = json_object_get_int(jval);
        printf("[json] pico.res.current_bezel_id: %d\n", pico.res.current_bezel_id);
    } else {
        printf("pico.res.current_bezel_id not found in json file. Using default: %d.\n", DEFAULT_BEZEL_ID);
        pico.res.current_bezel_id = DEFAULT_BEZEL_ID;
    }

    jval = NULL;
    if (json_object_object_get_ex(jbezel, "bezel_path", &jval) && jval) {
        const char *bezel_path = json_object_get_string(jval);
        snprintf(pico.res.bezel_path, PATH_MAX, "%s", bezel_path);
        printf("[json] pico.res.bezel_path: %s\n", pico.res.bezel_path);
    } else {
        printf("pico.res.bezel_path not found in json file. Using default: %s.\n", DEFAULT_BEZEL_PATH);
        snprintf(pico.res.bezel_path, PATH_MAX, "%s", DEFAULT_BEZEL_PATH);
    }

    jval = NULL;
    if (json_object_object_get_ex(jbezel, "digit_path", &jval) && jval) {
        const char *digit_path = json_object_get_string(jval);
        snprintf(pico.res.digit_path, PATH_MAX, "%s", digit_path);
        printf("[json] pico.res.digit_path: %s\n", pico.res.digit_path);
    } else {
        printf("pico.res.digit_path not found in json file. Using default: %s.\n", DEFAULT_DIGIT_PATH);
        snprintf(pico.res.digit_path, PATH_MAX, "%s", DEFAULT_DIGIT_PATH);
    }

    jval = NULL;
    if (json_object_object_get_ex(jbezel, "bezel_int_path", &jval) && jval) {
        const char *bezel_int_path = json_object_get_string(jval);
        snprintf(pico.res.bezel_int_path, PATH_MAX, "%s", bezel_int_path);
        printf("[json] pico.res.bezel_int_path: %s\n", pico.res.bezel_int_path);
    } else {
        printf("pico.res.bezel_int_path not found in json file. Using default: %s.\n", DEFAULT_INTEGER_BEZEL_PATH);
        snprintf(pico.res.bezel_int_path, PATH_MAX, "%s", DEFAULT_INTEGER_BEZEL_PATH);
    }
    
    jval = NULL;
    if (json_object_object_get_ex(jbezel, "current_integer_bezel", &jval) && jval) {
        pico.res.current_integer_bezel_id = json_object_get_int(jval);
        printf("[json] pico.res.current_integer_bezel_id: %d\n", pico.res.current_integer_bezel_id);
    } else {
        printf("pico.res.current_integer_bezel_id not found in json file. Using default: %d.\n", DEFAULT_INTEGER_BEZEL_ID);
        pico.res.current_integer_bezel_id = DEFAULT_INTEGER_BEZEL_ID;
    }

    return 0;
}

int cfgReadMouse(struct json_object *jfile) {
    struct json_object *jval = NULL;
    struct json_object *jScaleFactor = NULL, *jAcceleration = NULL, *jAccelerationRate = NULL;
    struct json_object *jMaxAcceleration = NULL, *jIncrementModifier = NULL, *jDisableMouseHotkey = NULL, *jDisableMouseIcon = NULL;
    struct json_object *jMinX = NULL, *jMinY = NULL, *jMaxX = NULL, *jMaxY = NULL;
       
    json_object_object_get_ex(jfile, "mouse", &jval);

    if (jval) {
        json_object_object_get_ex(jval, "scaleFactor", &jScaleFactor);
        if (jScaleFactor) {
            pico.mouse.scaleFactor = json_object_get_int(jScaleFactor);
            printf("[json] pico.mouse.scaleFactor: %d\n", pico.mouse.scaleFactor);
        } else {
            pico.mouse.scaleFactor = MMIYOO_DEFAULT_SCALE_FACTOR;
            printf("scaleFactor not found in json file. Using default: %d.\n", MMIYOO_DEFAULT_SCALE_FACTOR);
        }

        json_object_object_get_ex(jval, "acceleration", &jAcceleration);
        if (jAcceleration) {
            pico.mouse.acceleration = json_object_get_double(jAcceleration);
            printf("[json] pico.mouse.acceleration: %f\n", pico.mouse.acceleration);
        } else {
            pico.mouse.acceleration = MMIYOO_DEFAULT_ACCELERATION;
            printf("acceleration not found in json file. Using default: %f.\n", MMIYOO_DEFAULT_ACCELERATION);
        }

        json_object_object_get_ex(jval, "accelerationRate", &jAccelerationRate);
        if (jAccelerationRate) {
            pico.mouse.accelerationRate = json_object_get_double(jAccelerationRate);
            printf("[json] pico.mouse.accelerationRate: %f\n", pico.mouse.accelerationRate);
        } else {
            pico.mouse.accelerationRate = MMIYOO_DEFAULT_ACCELERATION_RATE;
            printf("accelerationRate not found in json file. Using default: %f.\n", MMIYOO_DEFAULT_ACCELERATION_RATE);
        }

        json_object_object_get_ex(jval, "maxAcceleration", &jMaxAcceleration);
        if (jMaxAcceleration) {
            pico.mouse.maxAcceleration = json_object_get_double(jMaxAcceleration);
            printf("[json] pico.mouse.maxAcceleration: %f\n", pico.mouse.maxAcceleration);
        } else {
            pico.mouse.maxAcceleration = MMIYOO_DEFAULT_MAX_ACCELERATION;
            printf("maxAcceleration not found in json file. Using default: %d.\n", MMIYOO_DEFAULT_MAX_ACCELERATION);
        }

        json_object_object_get_ex(jval, "incrementModifier", &jIncrementModifier);
        if (jIncrementModifier) {
            pico.mouse.incrementModifier = json_object_get_int(jIncrementModifier);
            printf("[json] pico.mouse.incrementModifier: %f\n", pico.mouse.incrementModifier);
        } else {
            pico.mouse.incrementModifier = MMIYOO_DEFAULT_INCREMENT_MODIFIER;
            printf("incrementModifier not found in json file. Using default: %f.\n", MMIYOO_DEFAULT_INCREMENT_MODIFIER);
        }
                
        json_object_object_get_ex(jval, "disableMouseHotkey", &jDisableMouseHotkey);
        if (jDisableMouseHotkey) {
            pico.mouse.disableMouseHotkey = json_object_get_int(jDisableMouseHotkey);
            printf("[json] pico.mouse.disableMouseHotkey: %d\n", pico.mouse.disableMouseHotkey);
        } else {
            pico.mouse.disableMouseHotkey = MMIYOO_DEFAULT_MOUSE_HOTKEY;
            printf("disableMouseHotkey not found in json file. Using default: %d.\n", MMIYOO_DEFAULT_MOUSE_HOTKEY);
        }
        
        json_object_object_get_ex(jval, "disableMouseIcon", &jDisableMouseIcon);
        if (jDisableMouseIcon) {
            pico.mouse.disableMouseIcon = json_object_get_int(jDisableMouseIcon);
            printf("[json] pico.mouse.disableMouseIcon: %d\n", pico.mouse.disableMouseIcon);
        } else {
            pico.mouse.disableMouseIcon = MMIYOO_DEFAULT_MOUSE_ICON;
            printf("disableMouseIcon not found in json file. Using default: %d.\n", MMIYOO_DEFAULT_MOUSE_ICON);
        }

        json_object_object_get_ex(jval, "minx", &jMinX);
        if (jMinX) {
            pico.mouse.minx = json_object_get_int(jMinX);
            printf("[json] pico.mouse.minx: %d\n", pico.mouse.minx);
        } else {
            pico.mouse.minx = MMIYOO_DEFAULT_MOUSE_MINX;
            printf("minx not found in json file. Using default: %d.\n", MMIYOO_DEFAULT_MOUSE_MINX);
        }

        json_object_object_get_ex(jval, "miny", &jMinY);
        if (jMinY) {
            pico.mouse.miny = json_object_get_int(jMinY);
            printf("[json] pico.mouse.miny: %d\n", pico.mouse.miny);
        } else {
            pico.mouse.miny = MMIYOO_DEFAULT_MOUSE_MINY;
            printf("miny not found in json file. Using default: %d.\n", MMIYOO_DEFAULT_MOUSE_MINY);
        }

        json_object_object_get_ex(jval, "maxx", &jMaxX);
        if (jMaxX) {
            pico.mouse.maxx = json_object_get_int(jMaxX);
            printf("[json] pico.mouse.maxx: %d\n", pico.mouse.maxx);
        } else {
            pico.mouse.maxx = MMIYOO_DEFAULT_MOUSE_MAXX;
            printf("maxx not found in json file. Using default: %d.\n", MMIYOO_DEFAULT_MOUSE_MAXX);
        }

        json_object_object_get_ex(jval, "maxy", &jMaxY);
        if (jMaxY) {
            pico.mouse.maxy = json_object_get_int(jMaxY);
            printf("[json] pico.mouse.maxy: %d\n", pico.mouse.maxy);
        } else {
            pico.mouse.maxy = MMIYOO_DEFAULT_MOUSE_MAXY;
            printf("maxy not found in json file. Using default: %d.\n", MMIYOO_DEFAULT_MOUSE_MAXY);
        }

        if (!jval) {
            printf("Mouse settings not found in json file. Using defaults.\n");
            pico.mouse.scaleFactor = MMIYOO_DEFAULT_SCALE_FACTOR;
            pico.mouse.acceleration = MMIYOO_DEFAULT_ACCELERATION;
            pico.mouse.accelerationRate = MMIYOO_DEFAULT_ACCELERATION_RATE;
            pico.mouse.maxAcceleration = MMIYOO_DEFAULT_MAX_ACCELERATION;
            pico.mouse.incrementModifier = MMIYOO_DEFAULT_INCREMENT_MODIFIER;
            pico.mouse.minx = MMIYOO_DEFAULT_MOUSE_MINX;
            pico.mouse.miny = MMIYOO_DEFAULT_MOUSE_MINY;
            pico.mouse.maxx = MMIYOO_DEFAULT_MOUSE_MAXX;
            pico.mouse.maxy = MMIYOO_DEFAULT_MOUSE_MAXY;
            printf("Defaults applied: minx=%d, miny=%d, maxx=%d, maxy=%d.\n", 
                   pico.mouse.minx, pico.mouse.miny, pico.mouse.maxx, pico.mouse.maxy);
        }
    }
    
    return 0;
}

int cfgReadClock(struct json_object *jfile) {
    
    struct json_object *jperf = NULL;
    struct json_object *jval = NULL;
         
    if (!json_object_object_get_ex(jfile, "performance", &jperf) || !jperf) {
        printf("Performance settings not found in json file. Using defaults.\n");
        pico.perf.cpuclock = MMIYOO_DEFAULT_CPU_CLOCK;
        pico.perf.cpuclockincrement = MMIYOO_DEFAULT_CPU_CLOCK_INCREMENT;
        pico.perf.maxcpu = MMIYOO_MAX_CPU_CLOCK;
        pico.perf.mincpu = MMIYOO_MIN_CPU_CLOCK;
        return 0;
    }
    
    json_object_object_get_ex(jperf, "maxcpu", &jval);
    if (jval) {
        pico.perf.maxcpu = json_object_get_int(jval);
        if (pico.perf.maxcpu < pico.perf.mincpu) {
            printf("Invalid maxcpu value. Using default: %d\n", pico.perf.maxcpu);
            pico.perf.maxcpu = MMIYOO_MAX_CPU_CLOCK;
        } else {
            printf("[json] pico.perf.maxcpu: %d\n", pico.perf.maxcpu);
        }
    } else {
        printf("maxcpu not found in json file. Using default: %d.\n", MMIYOO_MAX_CPU_CLOCK);
        pico.perf.maxcpu = MMIYOO_MAX_CPU_CLOCK;
    }

    json_object_object_get_ex(jperf, "mincpu", &jval);
    if (jval) {
        pico.perf.mincpu = json_object_get_int(jval);
        if (pico.perf.mincpu > pico.perf.maxcpu) {
            printf("Invalid mincpu value. Using default: %d\n", pico.perf.mincpu);
            pico.perf.mincpu = MMIYOO_MIN_CPU_CLOCK;
        } else {
            printf("[json] pico.perf.mincpu: %d\n", pico.perf.mincpu);
        }
    } else {
        printf("mincpu not found in json file. Using default: %d.\n", MMIYOO_MIN_CPU_CLOCK);
        pico.perf.mincpu = MMIYOO_MIN_CPU_CLOCK;
    }
    
    json_object_object_get_ex(jperf, "cpuclock", &jval);
    if (jval) {
        const char *cpuclock_str = json_object_get_string(jval);
        if (cpuclock_str) {
            int cpuclock = atoi(cpuclock_str);
            if (cpuclock > pico.perf.maxcpu) {
                printf("[json] pico.perf.cpuclock too high! Using default: %d\n", MMIYOO_DEFAULT_CPU_CLOCK);
                pico.perf.cpuclock = MMIYOO_DEFAULT_CPU_CLOCK;
            } else if (cpuclock < pico.perf.mincpu) {
                printf("[json] pico.perf.cpuclock too low! Using default: %d\n", MMIYOO_DEFAULT_CPU_CLOCK);
                pico.perf.cpuclock = MMIYOO_DEFAULT_CPU_CLOCK;
            } else {
                printf("[json] pico.perf.cpuclock: %d\n", cpuclock);
                pico.perf.cpuclock = cpuclock;
            }
        } else {
            printf("Invalid pico.perf.cpuclock value. Using default: %d\n", MMIYOO_DEFAULT_CPU_CLOCK);
            pico.perf.cpuclock = MMIYOO_DEFAULT_CPU_CLOCK;
        }
    } else {
        printf("pico.perf.cpuclock not found in json file. Using default: %d.\n", MMIYOO_DEFAULT_CPU_CLOCK);
        pico.perf.cpuclock = MMIYOO_DEFAULT_CPU_CLOCK;
    }

    json_object_object_get_ex(jperf, "cpuclockincrement", &jval);
    if (jval) {
        const char *cpuclockincrement_str = json_object_get_string(jval);
        if (cpuclockincrement_str) {
            int cpuclockincrement = atoi(cpuclockincrement_str);
            if (cpuclockincrement > MMIYOO_MAX_CPU_CLOCK_INCREMENT) {
                printf("[json] pico.perf.cpuclockincrement too high! Using maximum allowed: 100\n");
                pico.perf.cpuclockincrement = MMIYOO_MAX_CPU_CLOCK_INCREMENT;
            } else {
                printf("[json] pico.perf.cpuclockincrement: %d\n", cpuclockincrement);
                pico.perf.cpuclockincrement = cpuclockincrement;
            }
        } else {
            printf("Invalid pico.perf.cpuclockincrement value. Using default: %d\n", MMIYOO_DEFAULT_CPU_CLOCK_INCREMENT);
            pico.perf.cpuclockincrement = MMIYOO_DEFAULT_CPU_CLOCK_INCREMENT;
        }
    } else {
        printf("pico.perf.cpuclockincrement not found in json file. Using default: %d.\n", MMIYOO_DEFAULT_CPU_CLOCK_INCREMENT);
        pico.perf.cpuclockincrement = MMIYOO_DEFAULT_CPU_CLOCK_INCREMENT;
    }
    
    return 0;
}


// needs refactoring
int cfgReadCustomKeys(struct json_object *jfile) {
    struct json_object *jCustomKeys = NULL;
    SDL_Keycode default_keycode = SDLK_UNKNOWN;
    SDL_Keycode received_keycode = SDLK_UNKNOWN;
    const char *keys[] = {"A", "B", "X", "Y", "L1", "L2", "R1", "R2", "LeftDpad", "RightDpad", "UpDpad", "DownDpad", "Start", "Select", "Menu"};
    int numKeys = sizeof(keys) / sizeof(char*);
    struct json_object *jval = NULL;
    
    json_object_object_get_ex(jfile, "customkeys", &jCustomKeys);
    
    if (!jCustomKeys) {
        printf("customkeys block not found in json file. Using defaults.\n");
        return 0;
    }

    for (int i = 0; i < numKeys; ++i) {
        json_object_object_get_ex(jCustomKeys, keys[i], &jval);
        
        if (strcmp(keys[i], "A") == 0) default_keycode = MMIYOO_DEFAULT_KEY_A;
        else if (strcmp(keys[i], "B") == 0) default_keycode = MMIYOO_DEFAULT_KEY_B;
        else if (strcmp(keys[i], "X") == 0) default_keycode = MMIYOO_DEFAULT_KEY_X;
        else if (strcmp(keys[i], "Y") == 0) default_keycode = MMIYOO_DEFAULT_KEY_Y;
        else if (strcmp(keys[i], "L1") == 0) default_keycode = MMIYOO_DEFAULT_KEY_L1;
        else if (strcmp(keys[i], "L2") == 0) default_keycode = MMIYOO_DEFAULT_KEY_L2;
        else if (strcmp(keys[i], "R1") == 0) default_keycode = MMIYOO_DEFAULT_KEY_R1;
        else if (strcmp(keys[i], "R2") == 0) default_keycode = MMIYOO_DEFAULT_KEY_R2;
        else if (strcmp(keys[i], "LeftDpad") == 0) default_keycode = MMIYOO_DEFAULT_KEY_LeftDpad;
        else if (strcmp(keys[i], "RightDpad") == 0) default_keycode = MMIYOO_DEFAULT_KEY_RightDpad;
        else if (strcmp(keys[i], "UpDpad") == 0) default_keycode = MMIYOO_DEFAULT_KEY_UpDpad;
        else if (strcmp(keys[i], "DownDpad") == 0) default_keycode = MMIYOO_DEFAULT_KEY_DownDpad;
        else if (strcmp(keys[i], "Start") == 0) default_keycode = MMIYOO_DEFAULT_KEY_Start;
        else if (strcmp(keys[i], "Select") == 0) default_keycode = MMIYOO_DEFAULT_KEY_Select;
        else if (strcmp(keys[i], "Menu") == 0) default_keycode = MMIYOO_DEFAULT_KEY_MENU;
        
        if (jval) {
            received_keycode = SDL_GetKeyFromName(json_object_get_string(jval));
        }
        
        if (strcmp(keys[i], "A") == 0) {
            pico.customkey.A = (jval) ? received_keycode : default_keycode;
        } else if (strcmp(keys[i], "B") == 0) {
            pico.customkey.B = (jval) ? received_keycode : default_keycode;
        } else if (strcmp(keys[i], "X") == 0) {
            pico.customkey.X = (jval) ? received_keycode : default_keycode;
        } else if (strcmp(keys[i], "Y") == 0) {
            pico.customkey.Y = (jval) ? received_keycode : default_keycode;
        } else if (strcmp(keys[i], "L1") == 0) {
            pico.customkey.L1 = (jval) ? received_keycode : default_keycode;
        } else if (strcmp(keys[i], "L2") == 0) {
            pico.customkey.L2 = (jval) ? received_keycode : default_keycode;
        } else if (strcmp(keys[i], "R1") == 0) {
            pico.customkey.R1 = (jval) ? received_keycode : default_keycode;
        } else if (strcmp(keys[i], "R2") == 0) {
            pico.customkey.R2 = (jval) ? received_keycode : default_keycode;
        } else if (strcmp(keys[i], "LeftDpad") == 0) {
            pico.customkey.LeftDpad = (jval) ? received_keycode : default_keycode;
        } else if (strcmp(keys[i], "RightDpad") == 0) {
            pico.customkey.RightDpad = (jval) ? received_keycode : default_keycode;
        } else if (strcmp(keys[i], "UpDpad") == 0) {
            pico.customkey.UpDpad = (jval) ? received_keycode : default_keycode;
        } else if (strcmp(keys[i], "DownDpad") == 0) {
            pico.customkey.DownDpad = (jval) ? received_keycode : default_keycode;
        } else if (strcmp(keys[i], "Start") == 0) {
            pico.customkey.Start = (jval) ? received_keycode : default_keycode;
        } else if (strcmp(keys[i], "Select") == 0) {
            pico.customkey.Select = (jval) ? received_keycode : default_keycode;
        } else if (strcmp(keys[i], "Menu") == 0) {
            pico.customkey.Menu = (jval) ? received_keycode : default_keycode;
        }
        
        if (jval) {
            if (received_keycode != SDLK_UNKNOWN) {
                printf("[json] pico.customkey.%s: %d\n", keys[i], received_keycode);
            } else {
                printf("Invalid pico.customkey.%s, reset as %d\n", keys[i], default_keycode);
            }
        }
    }
    return 0;
}

SDL_Keycode stringToKeycode(const char *keyString) {
    char upperKeyString[strlen(keyString) + 1];
    for (size_t i = 0; i < strlen(keyString); ++i) {
        upperKeyString[i] = toupper(keyString[i]);
    }
    upperKeyString[strlen(keyString)] = '\0';
    
    if (strlen(upperKeyString) == 1 && isalpha(upperKeyString[0])) {
        return (SDL_Keycode)tolower(upperKeyString[0]);
    }

    if (strlen(upperKeyString) > 1) {
        return SDL_GetKeyFromName(upperKeyString);
    }
    
    return SDLK_UNKNOWN;
}

int picoConfigRead(void)
{
    char *last_slash = strrchr(pico.cfg_path, '/');
    struct json_object *jfile = NULL;
    
    getcwd(pico.cfg_path, sizeof(pico.cfg_path));

    if (last_slash) *last_slash = '\0';

    strcat(pico.cfg_path, "/");
    strcat(pico.cfg_path, PICO_CFG_PATH);

    jfile = json_object_from_file(pico.cfg_path);
    if (jfile == NULL) {
        printf("Failed to read settings from json file (%s)\n", pico.cfg_path);
        picoSetAllDefaults();
        return -1;
    }
    
    cfgReadCustomKeys(jfile);
    cfgReadClock(jfile);
    cfgReadMouse(jfile);
    cfgReadRes(jfile);

    json_object_put(jfile);
    return 0;
}

// this needs reworking so it doesn't write the max/min as they wont change at runtime
int picoConfigWrite(void) {
    struct json_object *jfile = NULL;
    struct json_object *jmouse = NULL;
    struct json_object *jperformance = NULL;
    struct json_object *jbezel = NULL;

    jfile = json_object_from_file(pico.cfg_path);

    if (jfile == NULL) { // would the file REALLY disappear while the app is running ?
        jfile = json_object_new_object();
        if (jfile == NULL) {
            printf("Failed to create json object\n");
            return -1;
        }
    } else {
        struct json_object *existingbezel = NULL;
        struct json_object *existingPerformance = NULL;

        if (json_object_object_get_ex(jfile, "bezel", &existingbezel)) {
            jbezel = json_object_get(existingbezel);
        }

        if (json_object_object_get_ex(jfile, "performance", &existingPerformance)) {
            struct json_object *maxCpu = NULL;
            struct json_object *minCpu = NULL;
            if (json_object_object_get_ex(existingPerformance, "maxcpu", &maxCpu) &&
                json_object_object_get_ex(existingPerformance, "mincpu", &minCpu)) {
                jperformance = json_object_new_object();
                json_object_object_add(jperformance, "maxcpu", json_object_get(maxCpu));
                json_object_object_add(jperformance, "mincpu", json_object_get(minCpu));
            }
        }
    }

    // mouse
    jmouse = json_object_new_object();
    json_object_object_add(jmouse, "scaleFactor", json_object_new_int(pico.mouse.scaleFactor));
    json_object_object_add(jmouse, "acceleration", json_object_new_double(pico.mouse.acceleration));
    json_object_object_add(jmouse, "accelerationRate", json_object_new_double(pico.mouse.accelerationRate));
    json_object_object_add(jmouse, "maxAcceleration", json_object_new_double(pico.mouse.maxAcceleration));
    json_object_object_add(jmouse, "incrementModifier", json_object_new_double(pico.mouse.incrementModifier));
    json_object_object_add(jmouse, "disableMouseHotkey", json_object_new_int(pico.mouse.disableMouseHotkey));
    json_object_object_add(jmouse, "disableMouseIcon", json_object_new_int(pico.mouse.disableMouseIcon));
    json_object_object_add(jmouse, "minx", json_object_new_int(pico.mouse.minx));
    json_object_object_add(jmouse, "miny", json_object_new_int(pico.mouse.miny));
    json_object_object_add(jmouse, "maxx", json_object_new_int(pico.mouse.maxx));
    json_object_object_add(jmouse, "maxy", json_object_new_int(pico.mouse.maxy));
    json_object_object_add(jfile, "mouse", jmouse);
    
    // performance
    jperformance = json_object_new_object();
    json_object_object_add(jperformance, "cpuclock", json_object_new_int(pico.perf.cpuclock));
    json_object_object_add(jperformance, "cpuclockincrement", json_object_new_int(pico.perf.cpuclockincrement));
    json_object_object_add(jperformance, "maxcpu", json_object_new_int(pico.perf.maxcpu));
    json_object_object_add(jperformance, "mincpu", json_object_new_int(pico.perf.mincpu));
    json_object_object_add(jfile, "performance", jperformance);

    // bezel
    jbezel = json_object_new_object();
    json_object_object_add(jbezel, "current_bezel", json_object_new_int(pico.res.current_bezel_id));
    json_object_object_add(jbezel, "current_integer_bezel", json_object_new_int(pico.res.current_integer_bezel_id));
    json_object_object_add(jbezel, "bezel_path", json_object_new_string(pico.res.bezel_path));
    json_object_object_add(jbezel, "digit_path", json_object_new_string(pico.res.digit_path));
    json_object_object_add(jbezel, "bezel_int_path", json_object_new_string(pico.res.bezel_int_path));
    json_object_object_add(jfile, "bezel", jbezel);

    // write it then
    if (json_object_to_file_ext(pico.cfg_path, jfile, JSON_C_TO_STRING_PRETTY) != 0) {
        printf("Failed to write settings to json file (%s)\n", pico.cfg_path);
        json_object_put(jfile);
        return -1;
    }

    json_object_put(jfile);
    printf("Updated settings in json file (%s) successfully!\n", pico.cfg_path);
    return 0;
}


     // custom keys but don't write these as they shouldn't change
    // jcustomkeys = json_object_new_object();
    // json_object_object_add(jfile, "A", json_object_new_string(SDL_GetKeyName(pico.customkey.A)));
    // json_object_object_add(jfile, "B", json_object_new_string(SDL_GetKeyName(pico.customkey.B)));
    // json_object_object_add(jfile, "X", json_object_new_string(SDL_GetKeyName(pico.customkey.X)));
    // json_object_object_add(jfile, "Y", json_object_new_string(SDL_GetKeyName(pico.customkey.Y)));
    // json_object_object_add(jfile, "L1", json_object_new_string(SDL_GetKeyName(pico.customkey.L1)));
    // json_object_object_add(jfile, "L2", json_object_new_string(SDL_GetKeyName(pico.customkey.L2)));
    // json_object_object_add(jfile, "R1", json_object_new_string(SDL_GetKeyName(pico.customkey.R1)));
    // json_object_object_add(jfile, "R2", json_object_new_string(SDL_GetKeyName(pico.customkey.R2)));
    // json_object_object_add(jfile, "LeftDpad", json_object_new_string(SDL_GetKeyName(pico.customkey.LeftDpad)));
    // json_object_object_add(jfile, "RightDpad", json_object_new_string(SDL_GetKeyName(pico.customkey.RightDpad)));
    // json_object_object_add(jfile, "UpDpad", json_object_new_string(SDL_GetKeyName(pico.customkey.UpDpad)));
    // json_object_object_add(jfile, "DownDpad", json_object_new_string(SDL_GetKeyName(pico.customkey.DownDpad)));
    // json_object_object_add(jfile, "Start", json_object_new_string(SDL_GetKeyName(pico.customkey.Start)));
    // json_object_object_add(jfile, "Select", json_object_new_string(SDL_GetKeyName(pico.customkey.Select)));
    // json_object_object_add(jfile, "Menu", json_object_new_string(SDL_GetKeyName(pico.customkey.Menu)));
    // json_object_object_add(jfile, "customkeys", jcustomkeys);