#include "cfg.h"
#include "colours.h"
#include "colours_external.h"

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
	Colours_setup->saturation = saturation / 100;
	Colours_setup->contrast = contrast / 100;
	Colours_setup->brightness = brightness / 100;
	Colours_setup->gamma = gamma / 100;
	Colours_Update();
}

void Palette_Adjust(int black, int white, int saturation, int contrast, int brightness, int gamma) {
	Colours_setup->black_level = black;
	Colours_setup->white_level = white;
	Colours_setup->saturation = saturation / 100;
	Colours_setup->contrast = contrast / 100;
	Colours_setup->brightness = brightness / 100;
	Colours_setup->gamma = gamma / 100;
	Colours_Update();
}
