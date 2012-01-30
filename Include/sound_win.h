/****************************************************************************
File    : sound_win.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# SoundWin public methods and objects prototypes
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 15.10.2003
*/

#ifndef __SOUND_WIN_H__
#define __SOUND_WIN_H__

#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif

/* Constants declarations */

#define SS_16BIT_AUDIO					0x0008
#define SS_NO_SOUND						0x0010

#define DEF_SOUND_STATE					SS_16BIT_AUDIO
#define DEF_SOUND_RATE					44100
#define DEF_SOUND_VOL					SDL_MIX_MAXVOLUME
#define DEF_SOUND_QUALITY				2
#define DEF_SOUND_LATENCY				20

#define DEF_SOUND_SPREAD				5

/* Exported methods */

BOOL Sound_Initialise   ( BOOL );
void Sound_Restart      ( void );
void Sound_SetQuality   ( int nQuality );
void Sound_Clear        ( void );
void Sound_OpenOutput   ( char *pszOutFileName );
void Sound_CloseOutput  ( void );
double Sound_AdjustSpeed ( void );

/* Internal state info */

void Sound_GetInterParms( WAVEFORMATEX *pInfo );

/* Exported globals */

struct SoundCtrl_t
{
	ULONG ulState;		/* Sound flags             */
	int   nRate;		/* Sound rate              */
	int   nVolume;		/* Sound volume            */
	int	  nQuality;		/* Sound quality		   */
	int   nLatency;		/* Sound latency in frames */
	FILE *pfOutput;		/* Audio stream            */
};
extern struct SoundCtrl_t g_Sound;

extern void (*Atari_PlaySound)( void );

#ifdef __cplusplus
}
#endif

#endif /*__SOUND_WIN_H__*/
