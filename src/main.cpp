#include "portaudio.hpp"
#include <stdio.h>

#define SAMPLE_RATE (48000)

typedef struct
{
    float left_phase;
    float right_phase;
} paTestData;

static int patestCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
    paTestData *data = (paTestData *)userData;
    float *out = (float *)outputBuffer;


    for (unsigned i = 0; i < framesPerBuffer; i++)
    {
        *out++ = data->left_phase;
        *out++ = data->right_phase;

        data->left_phase += 0.01f;
        if (data->left_phase >= 1.0f)
            data->left_phase -= 2.0f;

        data->right_phase += 0.03f;
        if (data->right_phase >= 1.0f)
            data->right_phase -= 2.0f;
    }

    return 0;
}

int main(int argc, char **argv)
{
    paTestData data;

    PaStream *stream;
    PaError err;
    
    // Initialize Port Audio
    err = Pa_Initialize();
    if (err != paNoError) goto error;


    int numDevices;

    numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) {
        err = numDevices;
        goto error;
    }

    const PaDeviceInfo *deviceInfo;
    for (unsigned i = 0; i < numDevices; i++)
    {
        deviceInfo = Pa_GetDeviceInfo(i);
        printf("Device #%d: %s", i+1, deviceInfo->name);
    }

    // Open Output Stream
    err = Pa_OpenDefaultStream(&stream, 0, 2, paFloat32, SAMPLE_RATE, 256, patestCallback, &data);

    if (err != paNoError) goto error;

    // Start Stream
    err = Pa_StartStream(stream);
    if (err != paNoError) goto error;

    // Sleep
    Pa_Sleep(5000);

    // Stop Stream
    err = Pa_StopStream(stream);
    if (err != paNoError) goto error;

    // Close Stream
    err = Pa_CloseStream(stream);
    if (err != paNoError) goto error;

    // Terminate Port Audio
    err = Pa_Terminate();
    if (err != paNoError) goto error;

    return 0;

error:
    printf("PortAudio Error: %s\n", Pa_GetErrorText(err));
    return -1;
}