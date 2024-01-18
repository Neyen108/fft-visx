#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>
#include <math.h>
#include <assert.h>
#include "include/raylib.h"

#define ARRAY_LEN(xs) sizeof(xs)/sizeof(xs[0])
#define N (1<<10)

float in[N];
float complex out[N];

typedef struct {
	float left;
	float right;
} Frame;
	
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
		in[i] = frames_arr[i].left;
	}
	
	fft(in, 1, out, N);	
}



int main(void){
	int width = 800;
	int height = 600;
	InitWindow(width, height, "fft-visx");
	SetTargetFPS(60);
	
	InitAudioDevice();
	Music music = LoadMusicStream("Ollie_Seasons.mp3");
	
	printf("music.frameCount = %u\n", music.frameCount);
	printf("music.stream.sampleRate = %u\n", music.stream.sampleRate);
	printf("music.stream.sampleSize = %u\n", music.stream.sampleSize);
	printf("music.stream.channels = %u\n", music.stream.channels);


	PlayMusicStream(music);
	
	AttachAudioStreamProcessor(music.stream, callback);

	while (!WindowShouldClose()) {
		UpdateMusicStream(music);

		if (IsKeyPressed(KEY_SPACE)) {
			if (IsMusicStreamPlaying(music)) {
				PauseMusicStream(music);
			} else {
				ResumeMusicStream(music);
			}
		}

		int render_width = GetRenderWidth();
		int render_height = GetRenderHeight();

		BeginDrawing();
		ClearBackground(CLITERAL(Color) {0x18, 0x18, 0x18, 0xFF});
		

		float max_amp = 0.0f;
		for(size_t i = 0; i < N; i++) {
			float a = amp(out[i]);
			if (max_amp < a) max_amp = a;
		}

		float step = 1.06;
		size_t m = 0;
		for(float f = 20.0f; (size_t) f < N; f *= step) {
			m += 1;
		}

		float cell_width = (float)render_width/m;
		
		m = 0;
		for (float f = 20.0f; (size_t) f < N; f *= step) {
			float f1 = f*step;
			float a = 0.0f;
			for(size_t q = (size_t) f; q < N && q < (size_t)f1; q++) {
				a += amp(out[q]);
			}
			a /= (size_t) f1 - (size_t) f + 1;
			float t = a / max_amp;
			DrawRectangle(m*cell_width, render_height/2 - (render_height/2)*t, cell_width, (render_height/2)*t, RED);
			m += 1;
		}
		

		EndDrawing();
	}
	

	return 0;
}


