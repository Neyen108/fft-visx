#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "include/raylib.h"

#define ARRAY_LEN(xs) sizeof(xs)/sizeof(xs[0])

typedef struct {
	float left;
	float right;
} Frame;

Frame global_frames[4800*2] = {0};
size_t global_frames_count = 0;

void callback(void *bufferData, unsigned int frames) {
	size_t capacity = ARRAY_LEN(global_frames);

	if (frames <= capacity - global_frames_count) {
		memcpy(global_frames + global_frames_count, bufferData, sizeof(Frame)*frames);
		global_frames_count += frames;
	}else if (frames <= capacity) {
		memmove(global_frames, global_frames + frames, sizeof(Frame)*(capacity - frames));
		memcpy(global_frames + (capacity - frames), bufferData, sizeof(Frame)*frames);
	}else {
		memcpy(global_frames, bufferData, sizeof(Frame)*capacity);
		global_frames_count = capacity;
	}
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
			float sample = global_frames[i].left;
			
			if (sample > 0){
				DrawRectangle(i*cell_width, render_height/2 - (render_height/2)*sample, 1, (render_height/2)*sample, RED);
			} else {
				DrawRectangle(i*cell_width, render_height/2, 1, (render_height/2)*sample, RED);
			}
		}

		EndDrawing();
	}
	

	return 0;
}


