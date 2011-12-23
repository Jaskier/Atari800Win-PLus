#include "cfg.h"
#include "colours.h"
#include "colours_external.h"
#include "monitor.h"

char CFG_osa_filename[FILENAME_MAX] = Util_FILENAME_NOT_SET;
char CFG_osb_filename[FILENAME_MAX] = Util_FILENAME_NOT_SET;
char CFG_xlxe_filename[FILENAME_MAX] = Util_FILENAME_NOT_SET;
char CFG_5200_filename[FILENAME_MAX] = Util_FILENAME_NOT_SET;
char CFG_basic_filename[FILENAME_MAX] = Util_FILENAME_NOT_SET;

int CFG_MatchTextParameter(char const *param, char const * const cfg_strings[], int cfg_strings_size)
{
	int i;
	for (i = 0; i < cfg_strings_size; i ++) {
		if (Util_stricmp(param, cfg_strings[i]) == 0)
			return i;
	}
	/* Unrecognised value */
	return -1;
}

int Palette_Read(char * file) {
	strcpy(Colours_external->filename, file);
	return COLOURS_EXTERNAL_Read(Colours_external);
}

void Palette_Generate(int black, int white, int saturation, int contrast, int brightness, int gamma) {
	Colours_setup->black_level = black;
	Colours_setup->white_level = white;
	Colours_setup->saturation = ((double)saturation - 50) / 50;
	Colours_setup->contrast = ((double)contrast - 50) / 25;
	Colours_setup->brightness = ((double) brightness - 50) / 25;
	Colours_setup->gamma = ((double) gamma - 50) / 50;
	Colours_Update();
}

int PLATFORM_Initialise(int *argc, char *argv[]) {
	return TRUE;
};

int PLATFORM_Exit(int run_monitor) {
	if (run_monitor && MONITOR_Run())
		return TRUE;
	return FALSE;
};

int PLATFORM_PORT(int num) {
	return 0xff;
};

int PLATFORM_TRIG(int num) { 
	return 1;
};
