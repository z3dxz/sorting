#pragma once
#define M_PI 3.141592653589793238462643383279502884197169399375105820974944592307816406286
#include <windows.h>

#define BUFFER_COUNT 4
#define BUFFER_SAMPLE_COUNT 1000
#define SAMPLES_PER_SECOND 48000

HWAVEOUT waveOut;
HANDLE waveEvent;
WAVEHDR headers[BUFFER_COUNT];
int16_t buffers[BUFFER_COUNT][BUFFER_SAMPLE_COUNT * 2];
WAVEHDR* currentHeader;
double volume = VOLUME;
double phase;
double phase_increment;
double audio_value;

void sound(double frequency) {
    if (frequency == 0) {
        phase_increment = 0;
        return;
    }
    phase_increment = 2 * M_PI / SAMPLES_PER_SECOND * frequency;
}

void fill_buffer(int16_t* buffer) {
    for (size_t i = 0; i < BUFFER_SAMPLE_COUNT * 2; i += 2) {
        if (phase_increment == 0) {
            phase = 0;
            audio_value *= 0.9;
        }
        else {
            phase += phase_increment;
            if (phase > 0) { phase -= 2 * M_PI; }
            audio_value = ((sin(phase) * volume));
        }
        buffer[i + 0] = audio_value;  // Left channel
        buffer[i + 1] = audio_value;  // Right channel
    }
}


DWORD audio_thread(LPVOID param) {
    while (1) {
        DWORD waitResult = WaitForSingleObject(waveEvent, INFINITE);
        if (waitResult) {
            fprintf(stderr, "Failed to wait for event.\n");
            return 1;
        }

        BOOL success = ResetEvent(waveEvent);
        if (!success) {
            fprintf(stderr, "Failed to reset event.\n");
            return 1;
        }

        while (currentHeader->dwFlags & WHDR_DONE) {
            fill_buffer((int16_t*)currentHeader->lpData);
            MMRESULT result = waveOutWrite(waveOut, currentHeader, sizeof(WAVEHDR));
            if (result) {
                fprintf(stderr, "Failed to write wave data.  Error code %u.\n", result);
                return 1;
            }

            currentHeader++;
            if (currentHeader == headers + BUFFER_COUNT) { currentHeader = headers; }
        }
    }
}

int audio_init() {
    WAVEFORMATEX format = { 0 };
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = 1;
    format.nSamplesPerSec = SAMPLES_PER_SECOND;
    format.wBitsPerSample = 32;
    format.nBlockAlign = format.nChannels * format.wBitsPerSample / 8;
    format.nAvgBytesPerSec = format.nBlockAlign * format.nSamplesPerSec;

    waveEvent = CreateEvent(NULL, true, false, NULL);
    if (waveEvent == NULL) {
        fprintf(stderr, "Failed to create event.");
        return 1;
    }

    MMRESULT result = waveOutOpen(&waveOut, WAVE_MAPPER, &format,
        (DWORD_PTR)waveEvent, 0, CALLBACK_EVENT);
    if (result) {
        fprintf(stderr, "Failed to start audio output.  Error code %u.\n", result);
        return 1;
    }

    for (size_t i = 0; i < BUFFER_COUNT; i++) {
        headers[i] = {
          (char*)buffers[i],
          BUFFER_SAMPLE_COUNT * 4
        };
        result = waveOutPrepareHeader(waveOut, &headers[i], sizeof(WAVEHDR));
        if (result) {
            fprintf(stderr, "Failed to prepare header.  Error code %u.\n", result);
            return 1;
        }
        headers[i].dwFlags |= WHDR_DONE;
    }
    currentHeader = headers;

    HANDLE thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)audio_thread, NULL, 0, NULL);
    if (thread == NULL) {
        fprintf(stderr, "Failed to start thread");
        return 1;
    }
    return 0;
}

