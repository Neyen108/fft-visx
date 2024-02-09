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

float interpolate_to_range(float x, float min_range, float max_range, float new_max) {
    // Ensure x is within the original range
    x = (x < min_range) ? min_range : ((x > max_range) ? max_range : x);

    // Interpolate to the new range
    float interpolated_value = (x - min_range) / (max_range - min_range) * new_max;

    return interpolated_value;
}

void DrawHistoricalGraph(int numOfBins, Entry history[][numOfBins]) {
	for(int i = 0; i < MAX_ENTRIES - 1; i++) {
		float maxY = 0.0f;

		for(int k = 0; k < numOfBins; k++) {
			if ((900 - history[i][k].currY) > maxY) maxY = 900 - history[i][k].currY;
		}

		for(int j = 0; j < numOfBins; j++) {
			float opacity = 0.3f - interpolate_to_range(900 - history[i][j].currY, 0.0f, maxY, 0.3f);
			if(i % 100 < 5 && i > 5) {
				opacity = 0.5f;
			}
			Color color = ColorAlpha(RED, opacity);
			DrawLine(history[i][j].currX + i, history[i][j].currY - i, history[i+1][j].currX + i+1, history[i+1][j].currY - (i+1), color);
			if(i % 100 == 0) {
				DrawLine(history[i][j].prevX + i, history[i][j].prevY - i, history[i][j].currX + i+1, history[i][j].currY - (i+1), color);
			}
		}
	}
}


int main(void){
	int width = 1000;
	int height = 1000;
	
	// 1.04 for more than words
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
	Music music = LoadMusicStream("more_than_words.mp3");
	
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
			
		long int currFrequencyBin = 0;
		int prevX = 0;
		int prevY = render_height - 100;

		for (float currFreq = 20.0f; (size_t) currFreq < N; currFreq *= step) {
			size_t nextFrequency = currFreq * step;
			float sumOfAmplitudes = 0.0f;
			
			for(size_t q = (size_t) currFreq; q < N && q < (size_t) nextFrequency; q++) {
				float a = amp(out[q]);
				sumOfAmplitudes += (size_t) a;
			}

			size_t avgAmplitudeOfFreqBin = sumOfAmplitudes / ((size_t) nextFrequency - (size_t) currFreq + 1);
			float interpolatedAmplitude = avgAmplitudeOfFreqBin / max_amp;

			int currX = currFrequencyBin * cell_width;
			int currY = (render_height - 100) - (render_height/2)*interpolatedAmplitude;
			//DrawRectangle(currX, currY, cell_width, (render_height/3)*interpolatedAmplitude, RED);
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


