#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "include/raylib.h"

#define ARRAY_LEN(xs) sizeof(xs)/sizeof(xs[0])

int32_t global_frames[1024] = {0};
size_t global_frames_count = 0;

void callback(void *bufferData, unsigned int frames) {
	
	if (frames > ARRAY_LEN(global_frames)) {
		frames = ARRAY_LEN(global_frames);
	}

	memcpy(global_frames, bufferData, sizeof(int32_t)*frames);

	global_frames_count = frames;

}


int main(void){
	int width = 800;
	int height = 600;
	InitWindow(width, height, "fft-visx");
	SetTargetFPS(60);
	
	InitAudioDevice();
	Music music = LoadMusicStream("Ollie_Seasons.mp3");
	//Sound sound = LoadSound("Ollie_Seasons.mp3");
	
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

		float cell_width = (float)render_width/global_frames_count;

		for (size_t i=0; i<global_frames_count; ++i) {
			int16_t sample = *(int16_t*)&global_frames[i];
			
			if (sample > 0){
				float t = (float)sample/INT16_MAX;
				DrawRectangle(i*cell_width, render_height/2 - render_height/2*t, cell_width, render_height/2*t, RED);
			} else {
				float t = (float)sample/INT16_MIN;
				DrawRectangle(i*cell_width, render_height/2, cell_width, render_height/2*t, RED);

			}
		}

		EndDrawing();
	}
	

	return 0;
}


