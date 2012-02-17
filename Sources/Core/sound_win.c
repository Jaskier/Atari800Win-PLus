/****************************************************************************
File    : sound_win.c
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Implementation of sound handling API
@(#) #BY# Richard Lawrence, Tomasz Szymankowski
@(#) #LM# 09.11.2003
*/

#include <stdio.h>
#include <windows.h>
#include <limits.h>
#include "WinConfig.h"
#include "Resource.h"
#include "Helpers.h"
#include "Debug.h"
#include "atari800.h"
#include "globals.h"
#include "macros.h"
#include "registry.h"
#include "timing.h"
#include "avisave.h"
#include "display_win.h"
#include "misc_win.h"
#include "sound_win.h"
#include "SDL.h"

/* Public objects */

struct SoundCtrl_t g_Sound =
{
	DEF_SOUND_STATE,
	DEF_SOUND_RATE,
	DEF_SOUND_VOL,
	DEF_SOUND_QUALITY,
	DEF_SOUND_LATENCY,
	NULL
};

/* Private objects */

static WAVEFORMATEX s_wfxWaveFormat;
BOOL         s_bSoundIsPaused        = TRUE;

static void  SndPlay_NoSound( void );
static void SndPlay_SDLSound( void );
void (*Atari_PlaySound)(void) = SndPlay_NoSound;

static int dsp_buffer_bytes;
static Uint8 *dsp_buffer = NULL;
static int gap_est = 0;
static double avg_gap;
static int dsp_write_pos;
static int dsp_read_pos;
static int callbacktick = 0;

/*========================================================
Function : Sound_GetInterParms
=========================================================*/
/* #FN#
   Returns sound information structure */
void
/* #AS#
   Nothing */
Sound_GetInterParms(
	WAVEFORMATEX *pInfo
)
{
	CopyMemory( pInfo, &s_wfxWaveFormat, sizeof(WAVEFORMATEX) );
} /* #OF# Sound_GetInterParms */

/*========================================================
Function : SndPlay_SDLSound
=========================================================*/
static void SndPlay_SDLSound( void ) {
	int bytes_written = 0;
	int samples_written;
	int gap;
	int newpos;
	int bytes_per_sample;
	int i;
	double bytes_per_ms;

	/* produce samples from the sound emulation */
	samples_written = MZPOKEYSND_UpdateProcessBuffer();
	bytes_per_sample = (POKEYSND_stereo_enabled ? 2 : 1) * ((_IsFlagSet( g_Sound.ulState, SS_16BIT_AUDIO )) ? 2 : 1);
	bytes_per_ms = (bytes_per_sample)*(g_Sound.nRate/1000.0);
	bytes_written = ((_IsFlagSet( g_Sound.ulState, SS_16BIT_AUDIO )) ? samples_written * 2 : samples_written);

	/* due to bug in atari800 sound engine, neutral value for
	   8bit sound is 0x20 not 0x80
	   16bit sound is 0xa000 not 0x0000 */
/*	if (_IsFlagSet( g_Sound.ulState, SS_16BIT_AUDIO )) {
		for (i=1; i<bytes_written; i+=2) {
			MZPOKEYSND_process_buffer[i] = MZPOKEYSND_process_buffer[i] + 0x40;
		}
	} else {
		for (i=0; i<bytes_written; i++) {
			MZPOKEYSND_process_buffer[i] = MZPOKEYSND_process_buffer[i] + 0x40;
		}
	}
*/
	if( g_Sound.pfOutput )
		fwrite( MZPOKEYSND_process_buffer, bytes_written, 1, g_Sound.pfOutput );

	Video_SaveFrame( NULL, 0, MZPOKEYSND_process_buffer, bytes_written );

	if (s_bSoundIsPaused || _IsFlagSet( g_Sound.ulState, SS_NO_SOUND ))
		return;

	SDL_LockAudio();
	/* this is the gap as of the most recent callback */
	gap = dsp_write_pos - dsp_read_pos;
	/* an estimation of the current gap, adding time since then */
	if (callbacktick != 0) {
		gap_est = gap - (bytes_per_ms)*(SDL_GetTicks() - callbacktick);
	}
	/* if there isn't enough room... */
	while (gap + bytes_written > dsp_buffer_bytes) {
		/* then we allow the callback to run.. */
		SDL_UnlockAudio();
		/* and delay until it runs and allows space. */
		SleepEx( 1, TRUE );
		SDL_LockAudio();
		/*printf("sound buffer overflow:%d %d\n",gap, dsp_buffer_bytes);*/
		gap = dsp_write_pos - dsp_read_pos;
	}
	/* now we copy the data into the buffer and adjust the positions */
	newpos = dsp_write_pos + bytes_written;
	if (newpos/dsp_buffer_bytes == dsp_write_pos/dsp_buffer_bytes) {
		/* no wrap */
		memcpy(dsp_buffer+(dsp_write_pos%dsp_buffer_bytes), MZPOKEYSND_process_buffer, bytes_written);
	}
	else {
		/* wraps */
		int first_part_size = dsp_buffer_bytes - (dsp_write_pos%dsp_buffer_bytes);
		memcpy(dsp_buffer+(dsp_write_pos%dsp_buffer_bytes), MZPOKEYSND_process_buffer, first_part_size);
		memcpy(dsp_buffer, MZPOKEYSND_process_buffer+first_part_size, bytes_written-first_part_size);
	}
	dsp_write_pos = newpos;
	if (callbacktick == 0) {
		/* Sound callback has not yet been called */
		dsp_read_pos += bytes_written;
	}
//	if (dsp_write_pos < dsp_read_pos) {
		/* should not occur */
//		printf("Error: dsp_write_pos < dsp_read_pos\n");
//	}
	while (dsp_read_pos > dsp_buffer_bytes) {
		dsp_write_pos -= dsp_buffer_bytes;
		dsp_read_pos -= dsp_buffer_bytes;
	}
	SDL_UnlockAudio();
}

/*========================================================
Function : SoundCallback
=========================================================*/
static void SoundCallback(void *userdata, Uint8 *stream, int len)
{
	int gap;
	int newpos;
	int underflow_amount = 0;
#define MAX_SAMPLE_SIZE 4
	static char last_bytes[MAX_SAMPLE_SIZE];
	int bytes_per_sample = (POKEYSND_stereo_enabled ? 2 : 1)*((_IsFlagSet( g_Sound.ulState, SS_16BIT_AUDIO )) ? 2:1);

	gap = dsp_write_pos - dsp_read_pos;
	if (gap < len) {
		underflow_amount = len - gap;
		len = gap;
		/*return;*/
	}
	newpos = dsp_read_pos + len;

	SDL_memset(stream, 0, len);
	if (newpos/dsp_buffer_bytes == dsp_read_pos/dsp_buffer_bytes) {
		/* no wrap */
//		memcpy(stream, dsp_buffer + (dsp_read_pos%dsp_buffer_bytes), len);
		SDL_MixAudio(stream, dsp_buffer + (dsp_read_pos%dsp_buffer_bytes), len, g_Sound.nVolume);
	}
	else {
		/* wraps */
		int first_part_size = dsp_buffer_bytes - (dsp_read_pos%dsp_buffer_bytes);
//		memcpy(stream,  dsp_buffer + (dsp_read_pos%dsp_buffer_bytes), first_part_size);
//		memcpy(stream + first_part_size, dsp_buffer, len - first_part_size);
		SDL_MixAudio(stream, dsp_buffer + (dsp_read_pos%dsp_buffer_bytes), first_part_size, g_Sound.nVolume);
		SDL_MixAudio(stream + first_part_size, dsp_buffer, len - first_part_size, g_Sound.nVolume);
	}
	/* save the last sample as we may need it to fill underflow */
	if (gap >= bytes_per_sample) {
		memcpy(last_bytes, stream + len - bytes_per_sample, bytes_per_sample);
	}
	/* Just repeat the last good sample if underflow */
	if (underflow_amount > 0 ) {
		int i;
		for (i = 0; i < underflow_amount/bytes_per_sample; i++) {
			memcpy(stream + len +i*bytes_per_sample, last_bytes, bytes_per_sample);
		}
	}
	dsp_read_pos = newpos;
	callbacktick = SDL_GetTicks();
}

/*========================================================
Function : Sound_AdjustSpeed
=========================================================*/
double Sound_AdjustSpeed(void)
{
	int bytes_per_sample = (POKEYSND_stereo_enabled ? 2 : 1) * (_IsFlagSet( g_Sound.ulState, SS_16BIT_AUDIO ) ? 2 : 1);

	double alpha = 2.0/(1.0+40.0);
	int gap_too_small;
	int gap_too_large;
	static int inited = FALSE;

	if (s_bSoundIsPaused || _IsFlagSet( g_Sound.ulState, SS_NO_SOUND ))
		return 1.0;
	if (!inited) {
		inited = TRUE;
		avg_gap = gap_est;
	}
	else {
		avg_gap = avg_gap + alpha * (gap_est - avg_gap);
	}

	gap_too_small = ((g_Sound.nLatency - DEF_SOUND_SPREAD) * 
					g_Sound.nRate * bytes_per_sample) / 1000;
	gap_too_large = ((g_Sound.nLatency + DEF_SOUND_SPREAD) *
					g_Sound.nRate * bytes_per_sample) / 1000;
	if (avg_gap < gap_too_small) {
		return 0.95;
	}
	if (avg_gap > gap_too_large) {
		return 1.05;
	}
	return 1.0;
}

/*========================================================
Function : Sound_Initialise
=========================================================*/
/* #FN#
   Initializes the Waveout/DirectSound playback */
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
Sound_Initialise(
	BOOL bClearRegs
)
{
#define DSP_BUFFER_FRAGS 8

	SDL_AudioSpec desired;
	int specified_delay_samps = (g_Sound.nRate * g_Sound.nLatency) / 1000;
	int dsp_buffer_samps = 1024 * DSP_BUFFER_FRAGS + specified_delay_samps;
	int bytes_per_sample = (POKEYSND_stereo_enabled ? 2 : 1) * ((_IsFlagSet( g_Sound.ulState, SS_16BIT_AUDIO )) ? 2 : 1);

	desired.freq = g_Sound.nRate;
	desired.format = _IsFlagSet( g_Sound.ulState, SS_16BIT_AUDIO ) ? AUDIO_S16 : AUDIO_U8;
	desired.channels = POKEYSND_stereo_enabled ? 2 : 1;
	desired.samples = (g_Sound.nLatency<=20) ? 512 : 1024;
	desired.callback = SoundCallback;
	desired.userdata = NULL;

	SDL_CloseAudio();
	SDL_OpenAudio(&desired, NULL);

	dsp_buffer_bytes = dsp_buffer_samps * bytes_per_sample;
	dsp_read_pos = 0;
	dsp_write_pos = specified_delay_samps * bytes_per_sample;
	avg_gap = 0.0;

	free(dsp_buffer);
	dsp_buffer = (Uint8 *)Util_malloc(dsp_buffer_bytes);
	if (_IsFlagSet( g_Sound.ulState, SS_16BIT_AUDIO ))
		memset(dsp_buffer, 0, dsp_buffer_bytes);
	else
		memset(dsp_buffer, 0x80, dsp_buffer_bytes);

	/* Initialize the kernel sound machine */
	POKEYSND_Init( POKEYSND_FREQ_17_EXACT, desired.freq, desired.channels,
		(_IsFlagSet( g_Sound.ulState, SS_16BIT_AUDIO ) ? 1 : 0), bClearRegs );

	Atari_PlaySound = SndPlay_SDLSound;
	s_bSoundIsPaused = FALSE;
	SDL_PauseAudio(0);

	/* Set this up for PCM, 1/2 channels, 8/16 bits unsigned samples */
	ZeroMemory( &s_wfxWaveFormat, sizeof(WAVEFORMATEX) );
	s_wfxWaveFormat.wFormatTag      = WAVE_FORMAT_PCM; /* The only tag valid with DirectSound */
	s_wfxWaveFormat.nChannels       = desired.channels;
	s_wfxWaveFormat.nSamplesPerSec  = g_Sound.nRate;
	s_wfxWaveFormat.wBitsPerSample  = _IsFlagSet( g_Sound.ulState, SS_16BIT_AUDIO ) ? 16 : 8;
	s_wfxWaveFormat.nBlockAlign     = s_wfxWaveFormat.nChannels * s_wfxWaveFormat.wBitsPerSample / 8;
	s_wfxWaveFormat.nAvgBytesPerSec = s_wfxWaveFormat.nSamplesPerSec * s_wfxWaveFormat.nBlockAlign;
	s_wfxWaveFormat.cbSize          = 0; /* This member is always zero for PCM formats */

	return TRUE;
} /* #OF# Sound_Initialise */

/*========================================================
Function : Sound_SetQuality
=========================================================*/
/* #FN#
   Sets quality of a sound playback */
void
/* #AS#
   Nothing */
Sound_SetQuality(
  int nQuality /* #IN# Sound quality */
)
{
	if (nQuality > 2)
		nQuality = 2;
	POKEYSND_SetMzQuality( nQuality );
	POKEYSND_enable_new_pokey = 1;
	POKEYSND_DoInit();
} /* #OF# Sound_SetQuality */

/*========================================================
Function : SndPlay_NoSound
=========================================================*/
/* #FN#
   Generates sound stream but doesn't play the sound */
static
void
/* #AS#
   Nothing */
SndPlay_NoSound( void ) {
	int bytes_written = 0;
	int samples_written;
	int bytes_per_sample;
	double bytes_per_ms;

	/* produce samples from the sound emulation */
	samples_written = MZPOKEYSND_UpdateProcessBuffer();
	bytes_per_sample = (POKEYSND_stereo_enabled ? 2 : 1) * ((_IsFlagSet( g_Sound.ulState, SS_16BIT_AUDIO )) ? 2 : 1);
	bytes_per_ms = (bytes_per_sample)*(g_Sound.nRate/1000.0);
	bytes_written = ((_IsFlagSet( g_Sound.ulState, SS_16BIT_AUDIO )) ? samples_written * 2 : samples_written);

	if( g_Sound.pfOutput )
		fwrite( MZPOKEYSND_process_buffer, bytes_written, 1, g_Sound.pfOutput );

	Video_SaveFrame( NULL, 0, MZPOKEYSND_process_buffer, bytes_written );
} /* #OF# SndPlay_NoSound */

/*========================================================
Function : Sound_Clear
=========================================================*/
/* #FN#
   Closes/mutes a sound output device */
void
/* #AS#
   Nothing */
Sound_Clear()
{
	s_bSoundIsPaused = TRUE;
	SDL_PauseAudio(1);
} /* #OF# Sound_Clear */

/*========================================================
Function : Sound_Restart
=========================================================*/
/* #FN#
   Resumes playback on a paused sound output device */
void
/* #AS#
   Nothing */
Sound_Restart( void )
{
	if( _IsFlagSet( g_Misc.ulState, MS_FULL_SPEED ) )
		return;

	if (_IsFlagSet( g_Sound.ulState, SS_16BIT_AUDIO ))
		memset(dsp_buffer, 0, dsp_buffer_bytes);
	else
		memset(dsp_buffer, 0x80, dsp_buffer_bytes);

	s_bSoundIsPaused = FALSE;
	SDL_PauseAudio(0);
} /* #OF# Sound_Restart */

/*========================================================
Function : Sound_OpenOutput
=========================================================*/
/* #FN#
   Opens a sound file */
void
/* #AS#
   Nothing */
Sound_OpenOutput( char *pszOutFileName )
{
	if( g_Sound.pfOutput )
		/* Close Sound Output file */
		Sound_CloseOutput();

	if( (g_Sound.pfOutput = fopen( pszOutFileName, "wb" )) )
	{
		WAVEFORMAT wf;
		WORD wBitsPerSample = _IsFlagSet( g_Sound.ulState, SS_16BIT_AUDIO ) ? 16 : 8;

//		wf.wFormatTag      = WAVE_FORMAT_PCM;
		wf.nChannels       = POKEYSND_stereo_enabled + 1;
		wf.nSamplesPerSec  = g_Sound.nRate;
		wf.nBlockAlign     = wf.nChannels * wBitsPerSample / 8;
		wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
		/*
		  Offset  Length   Contents

		The RIFF header:
		  0       4 bytes  'RIFF'
		  4       4 bytes  <file length - 8>
		  8       4 bytes  'WAVE'

		The format (fmt) chunk:
		  12      4 bytes  'fmt '
		  16      4 bytes  0x00000010     // Length of the fmt data (16 bytes)
		  20      2 bytes  0x0001         // Format tag: 1 = PCM
		  22      2 bytes  <channels>     // Channels: 1 = mono, 2 = stereo
		  24      4 bytes  <sample rate>  // Samples per second: e.g., 44100
		  28      4 bytes  <bytes/second> // sample rate * block align
		  32      2 bytes  <block align>  // channels * bits/sample / 8
		  34      2 bytes  <bits/sample>  // 8 or 16

		The data chunk:
		  36      4 bytes  'data'
		  40      4 bytes  <length of the data block>
		  44      ? bytes  <sample data>
		*/
		fwrite( "RIFF\000\000\000\000WAVEfmt\040\020\000\000\000\001\000", 22, 1, g_Sound.pfOutput );
		fwrite( &wf.nChannels,       2, 1, g_Sound.pfOutput );
		fwrite( &wf.nSamplesPerSec,  4, 1, g_Sound.pfOutput );
		fwrite( &wf.nAvgBytesPerSec, 4, 1, g_Sound.pfOutput );
		fwrite( &wf.nBlockAlign,     2, 1, g_Sound.pfOutput );
		fwrite( &wBitsPerSample,     2, 1, g_Sound.pfOutput );
		fwrite( "data\000\000\000\000", 8, 1, g_Sound.pfOutput );
	}
	else
		DisplayMessage( NULL, IDS_ERROR_FILE_OPEN, 0, MB_ICONEXCLAMATION | MB_OK, pszOutFileName );

} /* #OF# Sound_OpenOutput */

/*========================================================
Function : Sound_CloseOutput
=========================================================*/
/* #FN#
   Closes a sound file */
void
/* #AS#
   Nothing */
Sound_CloseOutput( void )
{
	if( g_Sound.pfOutput )
	{
		UINT unPos = 0;

		/* Sound file is finished, so modify header and close it */
		unPos = ftell( g_Sound.pfOutput ) - 8;
		fseek ( g_Sound.pfOutput, 4, SEEK_SET );	// Seek past RIFF
		fwrite( &unPos, 4, 1, g_Sound.pfOutput );	// Write out size of entire data chunk
		fseek ( g_Sound.pfOutput, 40, SEEK_SET );
		unPos -= 36;
		fwrite( &unPos, 4, 1, g_Sound.pfOutput );	// Write out size of just sample data
		fclose( g_Sound.pfOutput );

		g_Sound.pfOutput = NULL;

		DisplayMessage( NULL, IDS_SFX_FILE_CLOSED, 0, MB_ICONINFORMATION | MB_OK );
	}
} /* #OF# Sound_CloseOutput */

