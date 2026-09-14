#include "audio360_backend.h"
#include <stdio.h>

#ifdef _XBOX360

static IXAudio2 *g_rv360_audio = NULL;
static IXAudio2MasteringVoice *g_rv360_master = NULL;
static IXAudio2SourceVoice *g_rv360_voice = NULL;
static BYTE *g_rv360_pcm_data = NULL;
struct RV360_AUDIO_EFFECT { BYTE *data; DWORD bytes; DWORD rate; WORD channels; };
static RV360_AUDIO_EFFECT g_rv360_effects[1024];
static DWORD g_rv360_effect_count = 0;

static DWORD rv360_read_le32(const BYTE *p)
{
    return (DWORD)p[0] | ((DWORD)p[1] << 8) | ((DWORD)p[2] << 16) |
           ((DWORD)p[3] << 24);
}

static WORD rv360_read_le16(const BYTE *p)
{
    return (WORD)(p[0] | ((WORD)p[1] << 8));
}

static HRESULT rv360_read_wav_pcm16(const char *filename, BYTE **out_data,
                                    DWORD *out_bytes, DWORD *out_rate,
                                    WORD *out_channels)
{
    FILE *file = fopen(filename, "rb");
    if (!file) return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    BYTE riff[12];
    if (fread(riff, 1, sizeof(riff), file) != sizeof(riff) ||
        memcmp(riff, "RIFF", 4) || memcmp(riff + 8, "WAVE", 4)) {
        fclose(file); return E_FAIL;
    }
    WORD format = 0, channels = 0, bits = 0;
    DWORD rate = 0, size = 0; long data_offset = 0; BYTE chunk[8];
    while (fread(chunk, 1, sizeof(chunk), file) == sizeof(chunk)) {
        DWORD chunk_size = rv360_read_le32(chunk + 4); long payload = ftell(file);
        if (!memcmp(chunk, "fmt ", 4) && chunk_size >= 16) {
            BYTE fmt[16]; if (fread(fmt, 1, 16, file) != 16) break;
            format = rv360_read_le16(fmt); channels = rv360_read_le16(fmt + 2);
            rate = rv360_read_le32(fmt + 4); bits = rv360_read_le16(fmt + 14);
            fseek(file, payload + chunk_size, SEEK_SET);
        } else if (!memcmp(chunk, "data", 4)) {
            data_offset = payload; size = chunk_size; fseek(file, chunk_size, SEEK_CUR);
        } else fseek(file, chunk_size, SEEK_CUR);
        if (chunk_size & 1) fseek(file, 1, SEEK_CUR);
    }
    if (format != 1 || bits != 16 || !channels || !rate || !size) {
        fclose(file); return E_NOTIMPL;
    }
    BYTE *data = (BYTE *)malloc(size);
    if (!data) { fclose(file); return E_OUTOFMEMORY; }
    fseek(file, data_offset, SEEK_SET);
    BOOL ok = fread(data, 1, size, file) == size; fclose(file);
    if (!ok) { free(data); return E_FAIL; }
    *out_data = data; *out_bytes = size; *out_rate = rate; *out_channels = channels;
    return S_OK;
}

HRESULT rv360_audio_init(void)
{
    if (g_rv360_audio) return S_OK;

    HRESULT hr = XAudio2Create(&g_rv360_audio, 0, XAUDIO2_DEFAULT_PROCESSOR);
    if (FAILED(hr)) return hr;
    hr = g_rv360_audio->CreateMasteringVoice(&g_rv360_master);
    if (FAILED(hr)) {
        g_rv360_audio->Release();
        g_rv360_audio = NULL;
    }
    return hr;
}

void rv360_audio_shutdown(void)
{
    rv360_audio_stop();
    for (DWORD i = 0; i < g_rv360_effect_count; i++) free(g_rv360_effects[i].data);
    g_rv360_effect_count = 0;
    if (g_rv360_master) {
        g_rv360_master->DestroyVoice();
        g_rv360_master = NULL;
    }
    if (g_rv360_audio) {
        g_rv360_audio->Release();
        g_rv360_audio = NULL;
    }
}

HRESULT rv360_audio_play_pcm16(const void *data, DWORD bytes,
                               DWORD sample_rate, WORD channels,
                               BOOL looped)
{
    if (!g_rv360_audio || !g_rv360_master || !data || !bytes || !channels)
        return E_INVALIDARG;

    rv360_audio_stop();
    g_rv360_pcm_data = (BYTE *)malloc(bytes);
    if (!g_rv360_pcm_data) return E_OUTOFMEMORY;
    memcpy(g_rv360_pcm_data, data, bytes);
    WAVEFORMATEX format;
    ZeroMemory(&format, sizeof(format));
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = channels;
    format.nSamplesPerSec = sample_rate;
    format.wBitsPerSample = 16;
    format.nBlockAlign = (WORD)(channels * sizeof(short));
    format.nAvgBytesPerSec = sample_rate * format.nBlockAlign;

    HRESULT hr = g_rv360_audio->CreateSourceVoice(&g_rv360_voice, &format);
    if (FAILED(hr)) return hr;

    XAUDIO2_BUFFER buffer;
    ZeroMemory(&buffer, sizeof(buffer));
    buffer.AudioBytes = bytes;
    buffer.pAudioData = g_rv360_pcm_data;
    buffer.LoopCount = looped ? XAUDIO2_LOOP_INFINITE : 0;
    hr = g_rv360_voice->SubmitSourceBuffer(&buffer);
    if (SUCCEEDED(hr)) hr = g_rv360_voice->Start(0);
    if (FAILED(hr)) rv360_audio_stop();
    return hr;
}

HRESULT rv360_audio_play_wav_file(const char *filename, BOOL looped)
{
    FILE *file = fopen(filename, "rb");
    if (!file) return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

    BYTE riff[12];
    if (fread(riff, 1, sizeof(riff), file) != sizeof(riff) ||
        memcmp(riff, "RIFF", 4) != 0 || memcmp(riff + 8, "WAVE", 4) != 0) {
        fclose(file);
        return E_FAIL;
    }

    WORD format = 0, channels = 0, bits = 0;
    DWORD sample_rate = 0, data_size = 0;
    long data_offset = 0;
    BYTE chunk_header[8];
    while (fread(chunk_header, 1, sizeof(chunk_header), file) == sizeof(chunk_header)) {
        DWORD size = rv360_read_le32(chunk_header + 4);
        long payload = ftell(file);
        if (memcmp(chunk_header, "fmt ", 4) == 0 && size >= 16) {
            BYTE fmt[16];
            if (fread(fmt, 1, sizeof(fmt), file) != sizeof(fmt)) break;
            format = rv360_read_le16(fmt);
            channels = rv360_read_le16(fmt + 2);
            sample_rate = rv360_read_le32(fmt + 4);
            bits = rv360_read_le16(fmt + 14);
            fseek(file, payload + size, SEEK_SET);
        } else if (memcmp(chunk_header, "data", 4) == 0) {
            data_offset = payload;
            data_size = size;
            fseek(file, size, SEEK_CUR);
        } else {
            fseek(file, size, SEEK_CUR);
        }
        if (size & 1) fseek(file, 1, SEEK_CUR);
    }

    if (format != 1 || bits != 16 || !channels || !sample_rate || !data_size) {
        fclose(file);
        return E_NOTIMPL;
    }

    BYTE *pcm = (BYTE *)malloc(data_size);
    if (!pcm) { fclose(file); return E_OUTOFMEMORY; }
    fseek(file, data_offset, SEEK_SET);
    BOOL ok = fread(pcm, 1, data_size, file) == data_size;
    fclose(file);
    if (!ok) { free(pcm); return E_FAIL; }

    HRESULT hr = rv360_audio_play_pcm16(pcm, data_size, sample_rate, channels, looped);
    free(pcm);
    return hr;
}

HRESULT rv360_audio_load_xwp(const char *filename, DWORD *effect_offset)
{
    FILE *file = fopen(filename, "rb");
    if (!file) return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    char directory[256], line[512];
    strncpy(directory, filename, sizeof(directory) - 1); directory[sizeof(directory) - 1] = 0;
    char *slash = strrchr(directory, '/'); char *backslash = strrchr(directory, '\\');
    if (backslash > slash) slash = backslash;
    if (slash) *(slash + 1) = 0; else directory[0] = 0;
    DWORD offset = g_rv360_effect_count;
    while (fgets(line, sizeof(line), file) && g_rv360_effect_count < 1024) {
        if (strncmp(line, "ENTRY", 5)) continue;
        char *relative = strchr(line, ','); if (!relative) continue;
        relative++;
        while (*relative == ' ' || *relative == '\t') relative++;
        char *end = relative + strlen(relative);
        while (end > relative && (end[-1] == '\r' || end[-1] == '\n')) *--end = 0;
        for (char *p = relative; *p; p++) if (*p == '\\') *p = '/';
        char path[512]; _snprintf(path, sizeof(path), "%s%s", directory, relative);
        path[sizeof(path) - 1] = 0;
        BYTE *data; DWORD bytes, rate; WORD channels;
        if (SUCCEEDED(rv360_read_wav_pcm16(path, &data, &bytes, &rate, &channels))) {
            g_rv360_effects[g_rv360_effect_count].data = data;
            g_rv360_effects[g_rv360_effect_count].bytes = bytes;
            g_rv360_effects[g_rv360_effect_count].rate = rate;
            g_rv360_effects[g_rv360_effect_count].channels = channels;
            g_rv360_effect_count++;
        }
    }
    fclose(file);
    if (effect_offset) *effect_offset = offset;
    return S_OK;
}

HRESULT rv360_audio_play_effect(DWORD effect_index, BOOL looped)
{
    if (effect_index >= g_rv360_effect_count) return E_INVALIDARG;
    RV360_AUDIO_EFFECT *effect = &g_rv360_effects[effect_index];
    return rv360_audio_play_pcm16(effect->data, effect->bytes, effect->rate,
                                  effect->channels, looped);
}

void rv360_audio_stop(void)
{
    if (!g_rv360_voice) return;
    g_rv360_voice->Stop(0);
    g_rv360_voice->FlushSourceBuffers();
    g_rv360_voice->DestroyVoice();
    g_rv360_voice = NULL;
    free(g_rv360_pcm_data);
    g_rv360_pcm_data = NULL;
}

#endif
