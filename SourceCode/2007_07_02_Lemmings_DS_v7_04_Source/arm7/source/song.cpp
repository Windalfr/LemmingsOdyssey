#include "song.h"

extern "C" {
  #include "ctools.h"
}
#include <stdlib.h>
#include <stdio.h>

#include "../../generic/command.h"

/*
A word on pattern memory management:
patterns is a 3d-array. The dimensions are
pattern - channel - row
The first dimension is completely allocated,
the second and third dimensions are only
allocated as far as needed.
*/

/* ===================== PUBLIC ===================== */

// Everything that changes the song is only possible on arm9.
// This saves memory on arm7 (there's only 64k) and is safer.

#ifdef ARM9

Song::Song(u8 _speed, u8 _bpm, u8 _channels)
	:speed(_speed), bpm(_bpm), n_channels(_channels), restart_position(0), n_patterns(0)
{
	// Init arrays
	patternlengths = (u16*)malloc(sizeof(u16)*MAX_PATTERNS);
	pattern_order_table = (u8*)malloc(sizeof(u8)*MAX_POT_LENGTH);
	instruments = (Instrument**)malloc(sizeof(Instrument*)*MAX_INSTRUMENTS);
	name = (char*)malloc(MAX_SONG_NAME_LENGTH+1);
	my_memset(name, 0, MAX_SONG_NAME_LENGTH+1);
	my_strncpy(name, "unnamed", MAX_SONG_NAME_LENGTH);
	
	for(u16 i=0; i<MAX_PATTERNS; ++i) {
		patternlengths[i] = 0;
	}
	
	for(u16 i=0; i<MAX_POT_LENGTH; ++i) {
		pattern_order_table[i] = 0;
	}
	
	for(u16 i=0; i<MAX_INSTRUMENTS; ++i) {
		instruments[i] = NULL;
	}
	
	n_patterns = 0;
	potsize = 0;

	// Init pattern array
	patterns = (Cell***)malloc(sizeof(Cell**)*MAX_PATTERNS);

	// Create first pattern
	addPattern();

	// Init pattern order table
	potIns(0, 0);
}

Song::~Song()
{
	killInstruments();
	
	killPatterns();

	// Delete arrays
	free(patternlengths);
	free(pattern_order_table);
	free(name);
}

#endif

Cell **Song::getPattern(u8 idx)
{
	if(idx<n_patterns) {
		return patterns[idx];
	} else {
		return 0;
	}
}


u8 Song::getChannels(void) {
	return n_channels;
}

u16 Song::getPatternLength(u8 idx)
{
	if(idx<n_patterns) {
		return patternlengths[idx];
	} else {
		return 0;
	}
}

u32 Song::getMsPerTick(void) {
	return speed*5*1000/2/bpm; // Formula from Fasttracker II
}

Instrument *Song::getInstrument(u8 instidx) {
	return instruments[instidx];
}

u8 Song::getInstruments(void)
{
	// Return highest instrument index+1
	u8 n_inst = 0;
	for(u8 i=0; i<MAX_INSTRUMENTS; ++i) {
		if( instruments[i] != 0 ) {
			n_inst = i+1;
		}
	}
	return n_inst;
}

#ifdef ARM9

void Song::setInstrument(u8 idx, Instrument *instrument) {
	instruments[idx] = instrument;
}

// POT functions
void Song::potAdd(u8 ptn)
{
	pattern_order_table[potsize] = ptn;
	potsize++;
}

void Song::potDel(u8 element)
{
	for(u16 i=element; i<potsize; ++i) {
		pattern_order_table[i] = pattern_order_table[i+1];
	}
	if(potsize > 1) {
		potsize--;
	}	
}

void Song::potIns(u8 idx, u8 pattern)
{
	if(potsize < MAX_POT_LENGTH) {
		for(u8 i=255;i>idx;--i) {
			pattern_order_table[i] = pattern_order_table[i-1];
		}
		pattern_order_table[idx] = pattern;
		potsize++;
	}
}

#endif

u8 Song::getPotLength(void) {
	return potsize;
}

u8 Song::getPotEntry(u8 idx) {
	return pattern_order_table[idx];
}

#ifdef ARM9

void Song::setPotEntry(u8 idx, u8 value) {
	pattern_order_table[idx] = value;
}

void Song::addPattern(u16 length)
{
	patternlengths[n_patterns] = length;

	n_patterns++;
	
	patterns[n_patterns-1] = (Cell**)malloc(sizeof(Cell*)*n_channels);

	u8 i,j;
	for(i=0;i<n_channels;++i) {
		patterns[n_patterns-1][i] = (Cell*)malloc(sizeof(Cell)*patternlengths[n_patterns-1]);
		Cell *cell;
		for(j=0;j<patternlengths[n_patterns-1];++j) {
			cell = &patterns[n_patterns-1][i][j];
			clearCell(cell);
		}
	}
	
}

void Song::channelAdd(void) {
	
	if(n_channels==MAX_CHANNELS) return;
	
	// Go through all patterns and add a channel
	for(u8 pattern=0;pattern<n_patterns;++pattern) {
		patterns[pattern] = (Cell**)realloc(patterns[pattern], sizeof(Cell*)*(n_channels+1));
		patterns[pattern][n_channels] = (Cell*)malloc(sizeof(Cell)*patternlengths[pattern]);
		
		// Clear
		Cell *cell;
		for(u8 j=0;j<patternlengths[pattern];++j) {
			cell = &patterns[pattern][n_channels][j];
			clearCell(cell);
		}
	}
	
	n_channels++;
	
}

void Song::channelDel(void) {
	
	if(n_channels==1) return;
	
	// Go through all patterns and delete the last channel
	for(u8 pattern=0;pattern<n_patterns;++pattern) {
		free(patterns[pattern][n_channels-1]);
		patterns[pattern] = (Cell**)realloc(patterns[pattern], sizeof(Cell*)*(n_channels-1));
	}
	
	n_channels--;
	
}

#endif

u8 Song::getNumPatterns(void) {
	return n_patterns;
}

#ifdef ARM9

void Song::resizePattern(u8 ptn, u16 newlength)
{
	// Go through all channels of this pattern and resize them
	for(u8 channel=0; channel<n_channels; ++channel) {
		patterns[ptn][channel] = (Cell*)realloc(patterns[ptn][channel], sizeof(Cell)*newlength);
		
		// The the patterns are enlarged, the new cells must be cleared
		if(newlength > patternlengths[ptn]) {
			Cell *cell;
			for(u16 i=patternlengths[ptn]; i<newlength;++i) {
				cell = &patterns[ptn][channel][i];
				clearCell(cell);
			}
		}
	}
	
	patternlengths[ptn] = newlength;
}

// The most important function
void Song::setName(const char *_name) {
	my_strncpy(name, _name, MAX_SONG_NAME_LENGTH);
}

#endif

const char *Song::getName(void) {
	return name;
}

#ifdef ARM9

void Song::setRestartPosition(u8 _restart_position) {
	restart_position = _restart_position;
}

#endif

u8 Song::getRestartPosition(void) {
	return restart_position;
}

u8 Song::getTempo(void) {
	return speed;
}

u8 Song::getBPM(void) {
	return bpm;
}

#ifdef ARM9

void Song::setTempo(u8 _tempo) {
	speed = _tempo;
}

void Song::setBpm(u8 _bpm) {
	bpm = _bpm;
}

// Zapping
void Song::zapPatterns(void) {
	
	while(potsize > 1) {
		potDel(potsize-1);
	}
	setPotEntry(0, 0);
	
	killPatterns();
	n_channels = DEFAULT_CHANNELS;
	n_patterns = 0;
	
	patterns = (Cell***)malloc(sizeof(Cell**)*MAX_PATTERNS);
	
	addPattern();
	
	restart_position = 0;
}

void Song::zapInstruments(void)
{
	killInstruments();
	
	instruments = (Instrument**)malloc(sizeof(Instrument*)*MAX_INSTRUMENTS);
	for(u16 i=0; i<MAX_INSTRUMENTS; ++i) {
		instruments[i] = NULL;
	}
	
}

#endif

/* ===================== PRIVATE ===================== */

#ifdef ARM9

inline void Song::clearCell(Cell *cell) {
	
	cell->note = EMPTY_NOTE;
	cell->volume = 0xFF;
	cell->instrument = 0;
	cell->effect = 0;
	cell->effect_param = 0;
	
}

void Song::killPatterns(void) {
	
	for(u8 ptn=0; ptn<n_patterns; ++ptn) {
		
		for(u8 chn=0; chn<n_channels; ++chn) {
			
			free(patterns[ptn][chn]);
		}
		
		free(patterns[ptn]);
	}
	free(patterns);
}

void Song::killInstruments(void) {
	
	for(u8 i=0;i<MAX_INSTRUMENTS;++i) {
		if(instruments[i] != NULL) {
			delete instruments[i];
			instruments[i] = NULL;
		}
	}
	free(instruments);
}

#endif
