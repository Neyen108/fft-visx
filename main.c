#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>
#include <math.h>
#include <assert.h>
#include "include/raylib.h"

#define ARRAY_LEN(xs) sizeof(xs)/sizeof(xs[0])
#define N (1<<12)
#define MAX_ENTRIES 500

float in[N];
float complex out[N];

typedef struct {
	float left;
	float right;
} Frame;

typedef struct {
    int prevX;
    int prevY;
    int currX;
    int currY;
} Entry;
	
void fft(float in[], size_t stride, float complex out[], size_t n) {
	assert(n > 0);

	if (n == 1) {
		out[0] = in[0];
		return;
	}

	fft(in, stride*2, out, n/2);
	fft(in + stride, stride*2, out + n/2, n/2);

	for (size_t k = 0; k < n/2; ++k) {
		float t = (float)k/n;
		float complex v = cexp(-2*I*PI*t) * out[k + n/2];
		float complex e = out[k];
		out[k] = e + v;
		out[k + n/2] = e - v;
	}
}

float amp(float complex z) {
	float a = fabsf(crealf(z));
	float b = fabsf(cimagf(z));

	if (a < b) return b;
	return a;
}

void callback(void *bufferData, unsigned int frames) {
	Frame *frames_arr = bufferData;

	for (size_t i = 0; i < frames; ++i) {
		memmove(in, in + 1, (N - 1)*sizeof(in[0]));
		in[N-1] = frames_arr[i].left;
	}
	
	fft(in, 1, out, N);	
}


void DrawHistoricalGraph(int numOfBins, Entry history[][numOfBins]) {
	Color color = ColorAlpha(RED, 0.3f);
	for(int i = 0; i < MAX_ENTRIES; i++) {
		for(int j = 0; j < numOfBins; j++) {
			DrawLine(history[i][j].prevX + i, history[i][j].prevY - i, history[i][j].currX + i, history[i][j].currY - i, color);
		}
	}
}


int main(void){
	int width = 1000;
	int height = 1000;

	float step = 1.06;
	size_t numOfFrequencyBins = 0;
	for(float f = 20.0f; (size_t) f < N; f *= step) {
		numOfFrequencyBins += 1;
	}

	Entry history[MAX_ENTRIES][numOfFrequencyBins];

	InitWindow(width, height, "fft-visx");
	SetTargetFPS(60);

	int render_width = GetRenderWidth();
	int render_height = GetRenderHeight();

	float cell_width = (float) render_width / numOfFrequencyBins;

	InitAudioDevice();
	Music music = LoadMusicStream("Ollie_Seasons.mp3");
	
	printf("music.frameCount = %u\n", music.frameCount);
	printf("music.stream.sampleRate = %u\n", music.stream.sampleRate);
	printf("music.stream.sampleSize = %u\n", music.stream.sampleSize);
	printf("music.stream.channels = %u\n", music.stream.channels);


	PlayMusicStream(music);
	
	AttachAudioStreamProcessor(music.stream, callback);
	size_t iter = 0;
	while (!WindowShouldClose()) {
		UpdateMusicStream(music);

		if (IsKeyPressed(KEY_SPACE)) {
			if (IsMusicStreamPlaying(music)) {
				PauseMusicStream(music);
			} else {
				ResumeMusicStream(music);
			}
		}
		
		
		if(iter % MAX_ENTRIES == 0 && iter != 0) {
			iter = 0;
		}
		memmove(history + 1, history, (MAX_ENTRIES - 1)*sizeof(history[0]));	

		BeginDrawing();
		ClearBackground(CLITERAL(Color) {0x18, 0x18, 0x18, 0xFF});

		float max_amp = 0.0f;
		for(size_t i = 0; i < N; i++) {
			float a = amp(out[i]);
			if (max_amp < a) max_amp = a;
		}
			
		int currFrequencyBin = 0;
		int prevX = 0;
		int prevY = render_height - 100;
		
		for (float currFreq = 20.0f; (size_t) currFreq < N; currFreq *= step) {
			float nextFrequency = currFreq * step;
			float sumOfAmplitudes = 0.0f;

			for(size_t q = (size_t) currFreq; q < N && q < (size_t) nextFrequency; q++) {
				sumOfAmplitudes += amp(out[q]);
			}

			float avgAmplitudeOfFreqBin = sumOfAmplitudes / ((size_t) nextFrequency - (size_t) currFreq + 1);
			float interpolatedAmplitude = avgAmplitudeOfFreqBin / max_amp;

			float currX = currFrequencyBin * cell_width;
			float currY = (render_height - 100) - (render_height/2)*interpolatedAmplitude;
			DrawRectangle(currX, currY, cell_width, (render_height/2)*interpolatedAmplitude, RED);
			DrawLine(prevX, prevY, currX, currY, GREEN);
			
			history[0][currFrequencyBin].prevX = prevX;
			history[0][currFrequencyBin].prevY = prevY;
			history[0][currFrequencyBin].currX = currX;
			history[0][currFrequencyBin].currY = currY;
			
			prevX = currX;
			prevY = currY;
			currFrequencyBin += 1;
		}

		DrawHistoricalGraph(numOfFrequencyBins, history);
		iter += 1;
		

		EndDrawing();
	}
	

	return 0;
}


