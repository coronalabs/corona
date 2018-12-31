#include "ALmixer.h"
#include <stdio.h>

#ifdef ALMIXER_COMPILE_WITHOUT_SDL
	#if defined(_WIN32)
		#define WIN32_LEAN_AND_MEAN
		#include <windows.h>
	#else
   	  	#include <unistd.h>
	#endif

	static void Internal_Delay(ALuint milliseconds_delay)
	{
		#if defined(_WIN32)
		Sleep(milliseconds_delay);
		#else
		usleep(milliseconds_delay*1000);
		#endif
	}
#else
	#include "SDL.h"
	#define Internal_Delay SDL_Delay
#endif
#define MAX_SOURCES 16

ALboolean g_PlayingAudio[MAX_SOURCES];

void Internal_SoundFinished_CallbackIntercept(ALint which_channel, ALuint al_source, ALmixer_Data* almixer_data, ALboolean finished_naturally, void* user_data)
{
	fprintf(stderr, "Channel %d finished\n", which_channel);
	g_PlayingAudio[which_channel] = AL_FALSE;
}

int main(int argc, char* argv[])
{
	ALint i;
	ALboolean still_playing = AL_TRUE;

	ALmixer_Data* audio_data[MAX_SOURCES];
	if(argc < 1)
	{
		printf("Pass a sound file (or files) as a parameter\n");
	}
	else if(argc-1 > MAX_SOURCES)
	{
		printf("Maximum supported files is %d\n", MAX_SOURCES);
	}

	ALmixer_Init(22050, 0, 0);
	
	for(i=1; i<argc; i++)
	{
		if(!(audio_data[i-1]=ALmixer_LoadAll( argv[i], AL_FALSE) ))
		{
			printf("%s. Quiting program.\n", ALmixer_GetError());
			exit(0);
		}
	}

	ALmixer_SetPlaybackFinishedCallback(Internal_SoundFinished_CallbackIntercept, NULL);

	for(i=1; i<argc; i++)
	{
		g_PlayingAudio[i-1] = AL_TRUE;
		ALmixer_PlayChannel(i-1, audio_data[i-1], 0);
	}

	while(still_playing)
	{
		still_playing = AL_FALSE;
		for(i=1; i<argc; i++)
		{
			still_playing |= g_PlayingAudio[i-1];
		}
		ALmixer_Update();
		Internal_Delay(10);
	}
		
	for(i=1; i<argc; i++)
	{
		ALmixer_FreeData(audio_data[i-1]);
	}

	ALmixer_Quit();

	return 0;
}

