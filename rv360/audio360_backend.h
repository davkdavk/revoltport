#ifndef RV360_AUDIO360_BACKEND_H
#define RV360_AUDIO360_BACKEND_H

#ifdef _XBOX360
#include <xtl.h>
#include <xaudio2.h>

HRESULT rv360_audio_init(void);
void rv360_audio_shutdown(void);
HRESULT rv360_audio_play_pcm16(const void *data, DWORD bytes,
                               DWORD sample_rate, WORD channels,
                               BOOL looped);
HRESULT rv360_audio_play_wav_file(const char *filename, BOOL looped);
void rv360_audio_stop(void);

#endif

#endif
