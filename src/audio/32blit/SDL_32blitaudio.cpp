/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2020 Sam Lantinga <slouken@libsdl.org>

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

#include "SDL_timer.h"
#include "SDL_audio.h"
extern "C" {
#include "../SDL_audio_c.h"
}
#include "SDL_32blitaudio.h"
#include "SDL_assert.h"

#include "../../core/32blit/api.hpp"

static void
RefillBuffer(void *arg)
{
    auto _this = (SDL_AudioDevice *)arg;

    SDL_AudioCallback callback = _this->callbackspec.callback;
    const int stream_len = _this->callbackspec.size;

    /* Only do something if audio is enabled */
    if (!SDL_AtomicGet(&_this->enabled) || SDL_AtomicGet(&_this->paused)) {
        if (_this->stream) {
            SDL_AudioStreamClear(_this->stream);
        }
        return;
    }

    if (_this->stream == NULL) {  /* no conversion necessary. */
        SDL_assert(_this->spec.size == stream_len);
        callback(_this->callbackspec.userdata, _this->work_buffer, stream_len);
    } else {  /* streaming/converting */
        int got;
        while (SDL_AudioStreamAvailable(_this->stream) < ((int) _this->spec.size)) {
            callback(_this->callbackspec.userdata, _this->work_buffer, stream_len);
            if (SDL_AudioStreamPut(_this->stream, _this->work_buffer, stream_len) == -1) {
                SDL_AudioStreamClear(_this->stream);
                SDL_AtomicSet(&_this->enabled, 0);
                break;
            }
        }

        got = SDL_AudioStreamGet(_this->stream, _this->work_buffer, _this->spec.size);
        SDL_assert((got < 0) || (got == _this->spec.size));
        if (got != _this->spec.size) {
            SDL_memset(_this->work_buffer, _this->spec.silence, _this->spec.size);
        }
    }

    for(int i = 0; i < 64; i++)
        blit::api.channels[0].wave_buffer[i] = ((Sint16 *)_this->work_buffer)[i];
}


static void
TTBLITAUDIO_CloseDevice(_THIS)
{
    blit::api.channels[0].adsr_phase = 4; /*OFF*/
}

static int
TTBLITAUDIO_OpenDevice(_THIS, void *handle, const char *devname, int iscapture)
{
    SDL_bool valid_format = SDL_FALSE;
    SDL_AudioFormat test_format;

    test_format = SDL_FirstAudioFormat(_this->spec.format);
    while ((!valid_format) && (test_format)) {
        switch (test_format) {
        case AUDIO_S16:
            _this->spec.format = test_format;

            valid_format = SDL_TRUE;
            break;
        }
        test_format = SDL_NextAudioFormat();
    }

    if (!valid_format) {
        /* Didn't find a compatible format :( */
        return SDL_SetError("No compatible audio format!");
    }

    _this->hidden = (struct SDL_PrivateAudioData *)0x1;

    // limit spec
    _this->spec.freq = 22050;
    _this->spec.channels = 1;
    _this->spec.samples = 64;

    SDL_CalculateAudioSpec(&_this->spec);

    // setup channel
    blit::api.channels[0].waveforms = 1/*WAVE*/;
    blit::api.channels[0].volume = 0xFF;
    blit::api.channels[0].wave_callback_arg = _this;
    blit::api.channels[0].callback_waveBufferRefresh = RefillBuffer;

    blit::api.channels[0].adsr = 0xFFFF00;
    blit::api.channels[0].adsr_frame = 0;
    blit::api.channels[0].adsr_phase = 2 /*SUSTAIN*/;
    blit::api.channels[0].adsr_end_frame = 0;
    blit::api.channels[0].adsr_step = 0;

    return 0;
}

static int
TTBLITAUDIO_Init(SDL_AudioDriverImpl * impl)
{
    /* Set the function pointers */
    impl->OpenDevice = TTBLITAUDIO_OpenDevice;
    impl->CloseDevice = TTBLITAUDIO_CloseDevice;

    impl->OnlyHasDefaultOutputDevice = SDL_TRUE;

    // no threads, just an interrupt
    impl->SkipMixerLock = SDL_TRUE;
    impl->ProvidesOwnCallbackThread = SDL_TRUE;

    // no capture
    impl->OnlyHasDefaultCaptureDevice = SDL_FALSE;
    impl->HasCaptureSupport = SDL_FALSE;

    return 1;   /* this audio target is available. */
}

AudioBootStrap TTBLITAUDIO_bootstrap = {
    "32Blit", "SDL 32Blit audio driver", TTBLITAUDIO_Init, 0
};

/* vi: set ts=4 sw=4 expandtab: */
