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
#include <mmsystem.h>
#include <dsound.h>
#ifdef __cplusplus
extern "C" {
#endif

/* Constants declarations */

#define SS_MM_SOUND						0x0001	/* Sound states and registry stuff */
#define SS_DS_SOUND						0x0002
#define SS_CUSTOM_RATE					0x0004
#define SS_16BIT_AUDIO					0x0008
#define SS_NO_SOUND						0x0010

#define DEF_SOUND_STATE					(SS_DS_SOUND | SS_CUSTOM_RATE | SS_16BIT_AUDIO)
#define DEF_SOUND_RATE					44100
#define DEF_SOUND_VOL					0
#define DEF_SKIP_UPDATE					1
#define DEF_SOUND_LATENCY				2
#define DEF_SOUND_QUALITY				2
#define DEF_SOUND_DIGITIZED				1

/* Exported methods */

BOOL Sound_Initialise   ( BOOL bClearRegs );
void Sound_Restart      ( void );
void Sound_SetQuality   ( int nQuality );
void Sound_Clear        ( BOOL bPermanent, BOOL bFreeBuffer );
BOOL Sound_Disable      ( BOOL bClearSound );
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
	int   nSkipUpdate;	/* Pokey update frequency  */
	int   nLatency;		/* Sound latency in frames */
	int   nQuality;		/* Sound quality           */
	int   nDigitized;	/* Digitized effects       */
	int   nBieniasFix;	/* Adam Bienias' fix       */
	FILE *pfOutput;		/* Audio stream            */
};
extern struct SoundCtrl_t g_Sound;

extern void (*Atari_PlaySound)( void );

#ifdef __cplusplus
}
#endif

#endif /*__SOUND_WIN_H__*/
