#include "sample.h"

#include "../../generic/command.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef ARM7
extern char *debugstr;
extern u16 debugstrsize;
extern u8 debugbuf;
#endif

#define LOOKUP_FREQ(note,finetune)		(linear_freq_table_lookup(N_FINETUNE_STEPS*(note)+(finetune)))

// This is defined in audio.h for arm7 but not for arm9
#if !defined(SOUND_FORMAT_ADPCM)
#define SOUND_FORMAT_ADPCM	(2<<29)
#define SOUND_16BIT 		(1<<29)
#define SOUND_8BIT 		(0)
#endif

/* ===================== PUBLIC ===================== */

#ifdef ARM9

Sample::Sample(void *_sound_data, u32 _n_samples, u16 _sampling_frequency, bool _is_stereo, bool _is_16_bit,
	bool _loop, u8 _volume)
	:n_samples(_n_samples), is_stereo(_is_stereo),
		is_16_bit(_is_16_bit), loop(_loop), volume(_volume)
{
	sound_data = (void**)malloc(20*sizeof(void*));
	sound_data[0] = _sound_data;
	
	calcSize();
	setFormat();
	calcRelnoteAndFinetune(_sampling_frequency);
	
	make_subsampled_versions();
}

Sample::~Sample()
{
	free(sound_data[0]);
	delete_subsampled_versions();
	free(sound_data);
}

#endif

inline u32 linear_freq_table_lookup(u32 note)
{
	if(note<=LINEAR_FREQ_TABLE_MAX_NOTE*N_FINETUNE_STEPS) {
		if(note>=LINEAR_FREQ_TABLE_MIN_NOTE*N_FINETUNE_STEPS) {
			return linear_freq_table[note-LINEAR_FREQ_TABLE_MIN_NOTE*N_FINETUNE_STEPS];
		} else {
			return linear_freq_table[note % (12*N_FINETUNE_STEPS)] >> (((LINEAR_FREQ_TABLE_MIN_NOTE*N_FINETUNE_STEPS-1)-note) / (12*N_FINETUNE_STEPS)  + 1);
		}
	}
	return 0;
}

#if defined(ARM7)

void Sample::play(u8 note, u8 volume_ , u8 channel /* effects here */)
{
	if(channel>15) return; // DS has only 16 channels!
	
	u32 loop_bit;
	if((loop == true)&&(loop_length>0)) {
		loop_bit = SOUND_REPEAT;
	} else {
		loop_bit = SOUND_ONE_SHOT;
	}
	
	// Choose the subsampled version. The first 8 octaves will be fine,
	// if the note is higher, choose a subsampled version.
	// Octave 8 is more of a good guess, so there could be better, more
	// reasonable values.
	u8 octave = (note+rel_note) / 12;
	u8 srcsmp, realnote;
	
	if(octave <= 8) {
		srcsmp = 0;
		realnote = (note+rel_note);
	} else {
		srcsmp = octave-8;
		realnote = (note+rel_note) % 12 + 8*12;
	}
	
	SCHANNEL_CR(channel) = 0;
	SCHANNEL_TIMER(channel) = SOUND_FREQ((int)LOOKUP_FREQ(realnote,finetune));
	SCHANNEL_SOURCE(channel) = (uint32)sound_data[srcsmp];
	if(loop==true) {
		SCHANNEL_REPEAT_POINT(channel) = loop_starts[srcsmp] >> 2;
		SCHANNEL_LENGTH(channel) = loop_lengths[srcsmp] >> 2;
	} else {
		SCHANNEL_REPEAT_POINT(channel) = 0;
		SCHANNEL_LENGTH(channel) = sizes[srcsmp] >> 2;
	}
	//SCHANNEL_PAN(channel) = 100;
	SCHANNEL_CR(channel) =
		SCHANNEL_ENABLE |
		loop_bit |
		sound_format |
		SOUND_PAN(64) |
		SOUND_VOL(volume_*volume/2/255);
}

#endif

u32 Sample::calcPlayLength(u8 note)
{
	u32 samples_per_second = LOOKUP_FREQ(note+rel_note,finetune);
	return n_samples * 1000 / samples_per_second;
}

#ifdef ARM9

void Sample::setRelNote(s8 _rel_note) {
	rel_note = _rel_note;
}

void Sample::setFinetune(s8 _finetune) {
	finetune = _finetune;
}

#endif

u8 Sample::getRelNote(void) {
	return rel_note;
}

s8 Sample::getFinetune(void) {
	return finetune;
}

u32 Sample::getSize(void) {
	return sizes[0];
}

u32 Sample::getNSamples(void) {
	return n_samples;
}

void *Sample::getData(void) {
	return sound_data[0];
}

u8 Sample::getLoop(void)
{
	// 0: no loop, 1: loop, 2: ping pong loop
	//TODO: ping pong
	if(loop==true) {
		return 1;
	} else {
		return 0;
	}
}

bool Sample::is16bit(void) {
	return is_16_bit;
}

bool Sample::isStereo(void) {
	return is_stereo;
}

void Sample::setLoopStart(u32 _loop_start)
{
	loop_start = _loop_start;
	loop_starts[0] = _loop_start;
	for(u8 i=1;i<20;++i) {
		loop_starts[i] = loop_starts[i-1] / 2;
	}
}

u32 Sample::getLoopStart(void) {
	return loop_start;
}

void Sample::setLoopLength(u32 _loop_length)
{
	loop_length = _loop_length;
	loop_lengths[0] = _loop_length;
	for(u8 i=1;i<20;++i) {
		loop_lengths[i] = loop_lengths[i-1] / 2;
	}
}

u32 Sample::getLoopLength(void) {
	return loop_length;
}

void Sample::setVolume(u8 vol) {
	volume = vol;
}

u8 Sample::getVolume(void) {
	return volume;
}

/* ===================== PRIVATE ===================== */

void Sample::calcSize(void)
{
	if(is_16_bit) {
		if(is_stereo) {
			sizes[0] = n_samples*4;
		} else {
			sizes[0] = n_samples*2;
		}
	} else {
		if(is_stereo) {
			sizes[0] = n_samples*2;
		} else {
			sizes[0] = n_samples;
		}
	}
}

#ifdef ARM9

void Sample::setFormat(void) {
	
	// TODO ADPCM and stuff
	if(is_16_bit) {
		sound_format = SOUND_16BIT;
	} else {
		sound_format = SOUND_8BIT;
	}
}

int fncompare (const void *elem1, const void *elem2 )
{
	if ( *(u16*)elem1 < *(u16*)elem2) return -1;
	else if (*(u16*)elem1 == *(u16*)elem2) return 0;
	else return 1;
}

// Takes the sampling rate in hz and searches for FT2-compatible values for
// finetune and rel_note in the freq_table
void Sample::calcRelnoteAndFinetune(u32 freq)
{
	u16 freqpos = findClosestFreq(freq);
	
	finetune = freqpos%N_FINETUNE_STEPS;
	rel_note = freqpos/N_FINETUNE_STEPS - BASE_NOTE;
}

// finds the freq in the freq table that is closest to freq ^^
u16 Sample::findClosestFreq(u32 freq)
{
	// Binary search!
	bool found = false;
	u16 pos = LINEAR_FREQ_TABLE_SIZE / 2;
	u16 step = LINEAR_FREQ_TABLE_SIZE / 2;
	
	if(linear_freq_table_lookup(pos) == freq) return pos;
	else
	while(!found) {
		step /= 2;
		if(step == 0) return 0; // Error
		
		if(linear_freq_table_lookup(pos) < freq) {
			pos += step;
		} else {
			pos -= step;
		}
		
		if ( (linear_freq_table_lookup(pos) <= freq) && (linear_freq_table_lookup(pos+1) >= freq) ) {
			found = true;
		}
		
	}
	
	return pos;
}

void Sample::make_subsampled_versions(void)
{
	u32 cursize = sizes[0] / 2;
	u16 curjump = 2;
	
	if(is_16_bit) {
		
		if(!is_stereo) {
		
			for(u8 octave = 1;octave<20;++octave) {
				sound_data[octave] = malloc(cursize);
				sizes[octave] = cursize;
				
				s16 *srcsample = (s16*)sound_data[0];
				s16 *destsample = (s16*)sound_data[octave];
				for(u32 smp=0; smp<cursize/2; ++smp) {
					destsample[smp] = *srcsample;
					srcsample += curjump;
				}
				
				cursize /= 2;
				curjump *= 2;
			}
			
		} else {
			
			for(u8 octave = 1;octave<20;++octave) {
				sound_data[octave] = malloc(cursize);
				sizes[octave] = cursize;
				
				s16 *srcsample = (s16*)sound_data[0];
				s16 *destsample = (s16*)sound_data[octave];
				for(u32 smp=0; smp<cursize/2; ++smp) {
					destsample[smp] = *srcsample;
					smp++;
					srcsample+=2;
					destsample[smp] = *srcsample;
					
					srcsample += curjump*2; // *2 because of stereo
				}
				
				cursize /= 2;
				curjump *= 2;
			}
			
		}
		
	} else {
		
		if(!is_stereo) {
		
			for(u8 octave = 1;octave<20;++octave) {
				sound_data[octave] = malloc(cursize);
				sizes[octave] = cursize;
				
				s8 *srcsample = (s8*)sound_data[0];
				s8 *destsample = (s8*)sound_data[octave];
				for(u32 smp=0; smp<cursize; ++smp) {
					destsample[smp] = *srcsample;
					srcsample += curjump;
				}
				
				cursize /= 2;
				curjump *= 2;
			}
			
		} else {
			
			for(u8 octave = 1;octave<20;++octave) {
				sound_data[octave] = malloc(cursize);
				sizes[octave] = cursize;
				
				s8 *srcsample = (s8*)sound_data[0];
				s8 *destsample = (s8*)sound_data[octave];
				for(u32 smp=0; smp<cursize; ++smp) {
					destsample[smp] = *srcsample;
					smp++;
					srcsample++;
					destsample[smp] = *srcsample;
					
					srcsample += curjump*2; // *2 because of stereo
				}
				
				cursize /= 2;
				curjump *= 2;
			}
			
		}
		
	}
}

void Sample::delete_subsampled_versions(void)
{
	for(u8 i=1;i<20;++i) {
		free(sound_data[i]);
	}
}

#endif

