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
HRESULT rv360_audio_load_xwp(const char *filename, DWORD *effect_offset);
HRESULT rv360_audio_play_effect(DWORD effect_index, BOOL looped);
void rv360_audio_stop(void);

// OG DirectSound volume/pitch ranges (dsound.h not present on 360).
// DSBVOLUME values are standard hundredths-of-dB. Pitch bounds need checking
// against OG dsound.h; the 360 SetPitch mapping itself is still pending (M5).
#ifndef DSBVOLUME_MIN
#define DSBVOLUME_MIN (-10000)
#define DSBVOLUME_MAX 0
#define DSBPITCH_MIN (-10000)
#define DSBPITCH_MAX 10000
#endif

#endif

#endif
