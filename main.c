#include <stdio.h>
#include "include/raylib.h"

int main(void){
	InitAudioDevice();

	Sound sound = LoadSound("Ollie_Seasons.mp3");
	return 0;
}


