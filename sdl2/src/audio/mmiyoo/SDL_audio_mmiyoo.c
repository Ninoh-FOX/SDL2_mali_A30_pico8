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

#if SDL_AUDIO_DRIVER_MMIYOO

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/soundcard.h>
#include <json-c/json.h>

#include "SDL_timer.h"
#include "SDL_audio.h"
#include "SDL_audio_mmiyoo.h"

#include "../SDL_audio_c.h"
#include "../SDL_audiodev_c.h"

#ifdef MMIYOO
#include "mi_sys.h"
#include "mi_common_datatype.h"
#include "mi_ao.h"

#define MAX_VOLUME      20
#define MIN_RAW_VALUE   -60
#define MAX_RAW_VALUE   30
#define MI_AO_SETVOLUME 0x4008690b
#define MI_AO_GETVOLUME 0xc008690c
#define MI_AO_SETMUTE   0x4008690d
#define JSON_APP_FILE   "/appconfigs/system.json"
#define JSON_VOL_KEY    "vol"

static int cur_volume = 0;
static MI_AUDIO_Attr_t stSetAttr;
static MI_AUDIO_Attr_t stGetAttr;
static MI_AO_CHN AoChn = 0;
static MI_AUDIO_DEV AoDevId = 0;
static struct json_object *jfile = NULL;
#endif

#ifdef TRIMUI
static int dsp_fd = -1;
#endif

#ifdef MMIYOO
static int set_volume_raw(int value, int add)
{
    int fd = -1;
    int buf2[] = {0, 0}, prev_value = 0;
    uint64_t buf1[] = {sizeof(buf2), (uintptr_t)buf2};

    if ((fd = open("/dev/mi_ao", O_RDWR)) < 0) {
        return 0;
    }

    ioctl(fd, MI_AO_GETVOLUME, buf1);
    prev_value = buf2[1];

    if (add) {
        value = prev_value + add;
    }
    else {
        value += MIN_RAW_VALUE;
    }

    if (value > MAX_RAW_VALUE) {
        value = MAX_RAW_VALUE;
    }
    else if (value < MIN_RAW_VALUE) {
        value = MIN_RAW_VALUE;
    }

    if (value == prev_value) {
        close(fd);
        return prev_value;
    }

    buf2[1] = value;
    ioctl(fd, MI_AO_SETVOLUME, buf1);
    if (prev_value <= MIN_RAW_VALUE && value > MIN_RAW_VALUE) {
        buf2[1] = 0;
        ioctl(fd, MI_AO_SETMUTE, buf1);
    }
    else if (prev_value > MIN_RAW_VALUE && value <= MIN_RAW_VALUE) {
        buf2[1] = 1;
        ioctl(fd, MI_AO_SETMUTE, buf1);
    }
    close(fd);
    return value;
}

static int set_volume(int volume)
{
    int volume_raw = 0;

    if (volume > MAX_VOLUME) {
        volume = MAX_VOLUME;
    }
    else if (volume < 0) {
        volume = 0;
    }

    if (volume != 0) {
        volume_raw = round(48 * log10(1 + volume));
    }

    set_volume_raw(volume_raw, 0);
    return volume;
}

int volume_inc(void)
{
    if (cur_volume < MAX_VOLUME) {
        cur_volume+= 1;
        set_volume(cur_volume);
    }
    return cur_volume;
}

int volume_dec(void)
{
    if (cur_volume > 0) {
        cur_volume-= 1;
        set_volume(cur_volume);
    }
    return cur_volume;
}
#endif
static void MMIYOO_CloseDevice(_THIS)
{
    SDL_free(this->hidden->mixbuf);
    SDL_free(this->hidden);

#ifdef MMIYOO
    MI_AO_DisableChn(AoDevId, AoChn);
    MI_AO_Disable(AoDevId);
#endif

#ifdef TRIMUI
    if (dsp_fd > 0) {
        close(dsp_fd);
        dsp_fd = -1;
    }
#endif
}

static int MMIYOO_OpenDevice(_THIS, void *handle, const char *devname, int iscapture)
{
#ifdef MMIYOO
    MI_S32 miret = 0;
    MI_S32 s32SetVolumeDb = 0;
    MI_S32 s32GetVolumeDb = 0;
    MI_SYS_ChnPort_t stAoChn0OutputPort0;
#endif

#ifdef TRIMUI
    int arg = 0;
#endif

    this->hidden = (struct SDL_PrivateAudioData *)SDL_malloc((sizeof * this->hidden));
    if(this->hidden == NULL) {
        return SDL_OutOfMemory();
    }
    SDL_zerop(this->hidden);

    this->hidden->mixlen = this->spec.samples * 2 * this->spec.channels;
    this->hidden->mixbuf = (Uint8 *) SDL_malloc(this->hidden->mixlen);
    if(this->hidden->mixbuf == NULL) {
        return SDL_OutOfMemory();
    }

#ifdef MMIYOO
    jfile = json_object_from_file(JSON_APP_FILE);
    if (jfile != NULL) {
        struct json_object *volume = NULL;

        json_object_object_get_ex(jfile, JSON_VOL_KEY, &volume);
        cur_volume = json_object_get_int(volume);
        //json_object_object_add(jfile, JSON_VOL_KEY, json_object_new_int(2));
        //json_object_to_file(JSON_APP_FILE, jfile);
        json_object_put(jfile);
    }
#endif

#ifdef MMIYOO
    stSetAttr.eBitwidth = E_MI_AUDIO_BIT_WIDTH_16;
    stSetAttr.eWorkmode = E_MI_AUDIO_MODE_I2S_MASTER;
    stSetAttr.u32FrmNum = 6;
    stSetAttr.u32PtNumPerFrm = this->spec.samples;
    stSetAttr.u32ChnCnt = this->spec.channels;
    stSetAttr.eSoundmode = this->spec.channels == 2 ? E_MI_AUDIO_SOUND_MODE_STEREO : E_MI_AUDIO_SOUND_MODE_MONO;
    stSetAttr.eSamplerate = (MI_AUDIO_SampleRate_e)this->spec.freq;
    printf(PREFIX"Freq:%d, Sample:%d, Channels:%d\n", this->spec.freq, this->spec.samples, this->spec.channels);
    miret = MI_AO_SetPubAttr(AoDevId, &stSetAttr);
    if(miret != MI_SUCCESS) {
        printf(PREFIX"failed to set PubAttr\n");
        return -1;
    }
    miret = MI_AO_GetPubAttr(AoDevId, &stGetAttr);
    if(miret != MI_SUCCESS) {
        printf(PREFIX"failed to get PubAttr\n");
        return -1;
    }
    miret = MI_AO_Enable(AoDevId);
    if(miret != MI_SUCCESS) {
        printf(PREFIX"failed to enable AO\n");
        return -1;
    }
    miret = MI_AO_EnableChn(AoDevId, AoChn);
    if(miret != MI_SUCCESS) {
        printf(PREFIX"failed to enable Channel\n");
        return -1;
    }
    miret = MI_AO_SetVolume(AoDevId, s32SetVolumeDb);
    if(miret != MI_SUCCESS) {
        printf(PREFIX"failed to set Volume\n");
        return -1;
    }
    MI_AO_GetVolume(AoDevId, &s32GetVolumeDb);
    stAoChn0OutputPort0.eModId = E_MI_MODULE_ID_AO;
    stAoChn0OutputPort0.u32DevId = AoDevId;
    stAoChn0OutputPort0.u32ChnId = AoChn;
    stAoChn0OutputPort0.u32PortId = 0;
    MI_SYS_SetChnOutputPortDepth(&stAoChn0OutputPort0, 12, 13);
    set_volume(cur_volume);
#endif

#ifdef TRIMUI
    dsp_fd = open("/dev/dsp", O_RDWR);
    if (dsp_fd < 0) {
        return -1;
    }

    arg = 16;
    ioctl(dsp_fd, SOUND_PCM_WRITE_BITS, &arg);

    arg = CHANNELS;
    ioctl(dsp_fd, SOUND_PCM_WRITE_CHANNELS, &arg);

    arg = FREQ;
    ioctl(dsp_fd, SOUND_PCM_WRITE_RATE, &arg);
#endif

    return 0;
}

static void MMIYOO_PlayDevice(_THIS)
{
#ifdef MMIYOO
    MI_AUDIO_Frame_t aoTestFrame;

    aoTestFrame.eBitwidth = stGetAttr.eBitwidth;
    aoTestFrame.eSoundmode = stGetAttr.eSoundmode;
    aoTestFrame.u32Len = this->hidden->mixlen;
    aoTestFrame.apVirAddr[0] = this->hidden->mixbuf;
    aoTestFrame.apVirAddr[1] = NULL;
    MI_AO_SendFrame(AoDevId, AoChn, &aoTestFrame, 1);
    usleep(((stSetAttr.u32PtNumPerFrm * 1000) / stSetAttr.eSamplerate - 10) * 1000);
#endif

#ifdef TRIMUI
    write(dsp_fd, this->hidden->mixbuf, this->hidden->mixlen);
#endif
}

static uint8_t *MMIYOO_GetDeviceBuf(_THIS)
{
    return (this->hidden->mixbuf);
}

static int MMIYOO_Init(SDL_AudioDriverImpl *impl)
{
    impl->OpenDevice = MMIYOO_OpenDevice;
    impl->PlayDevice = MMIYOO_PlayDevice;
    impl->GetDeviceBuf = MMIYOO_GetDeviceBuf;
    impl->CloseDevice = MMIYOO_CloseDevice;
    impl->OnlyHasDefaultOutputDevice = 1;
    return 1;
}

AudioBootStrap MMIYOOAUDIO_bootstrap = {MMIYOO_DRIVER_NAME, "MMIYOO AUDIO DRIVER", MMIYOO_Init, 0};

#endif

