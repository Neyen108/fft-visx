#include <stdio.h>
#include "include/raylib.h"

int main(void){
	int width = 800;
	int height = 600;
	InitWindow(width, height, "fft-visx");
	SetTargetFPS(60);
	
	InitAudioDevice();
	Music music = LoadMusicStream("Ollie_Seasons.mp3");
	//Sound sound = LoadSound("Ollie_Seasons.mp3");
	PlayMusicStream(music);
	
	while (!WindowShouldClose()) {
		UpdateMusicStream(music);
		BeginDrawing();
		ClearBackground(RED);
		EndDrawing();
	}
	

	return 0;
}


