//
// sample.h
//
// This class represents a tracker sample including sample
// settings and provides funtionality for playing it.
//
//                        by 0xtob
//                            http://nitrotracker.tobw.net
//
// distributed under LGPL
//

#ifndef SAMPLE_H
#define SAMPLE_H

#include <nds.h>
#include "linear_freq_table.h"

#define BASE_NOTE				48	// Index if C-4 (FT2 base note)
#define SILENCE_THRESHOLD_16	2000
#define CROP_IGNORE_START		200

class Sample {
	public:
		Sample(void *_sound_data, u32 _n_samples, u16 _sampling_frequency=44100,
			bool _is_stereo=false, bool _is_16_bit=true, bool _loop=false, u8 _volume=255);
		//Sample(const char *filename, bool _loop, bool *_success);
		~Sample();
		
		void play(u8 note, u8 volume_, u8 channel  /* effects here */);
		u32 calcPlayLength(u8 note);
	
		void setRelNote(s8 _rel_note);
		void setFinetune(s8 _finetune);
		u8 getRelNote(void);
		s8 getFinetune(void);

		u32 getSize(void);
		u32 getNSamples(void);

		void *getData(void);
		
		u8 getLoop(void); // 0: no loop, 1: loop, 2: ping pong loop
		bool is16bit(void);
		bool isStereo(void);
		
		void setLoopLength(u32 _loop_length);
		u32 getLoopLength(void);
		void setLoopStart(u32 _loop_start);
		u32 getLoopStart(void);

		void setVolume(u8 vol);
		u8 getVolume(void);
		
	private:
		void calcSize(void);
		void setFormat(void);
		void calcRelnoteAndFinetune(u32 freq);
		u16 findClosestFreq(u32 freq);
		void make_subsampled_versions(void);
		void delete_subsampled_versions(void);
		
		void **sound_data;
		u32 n_samples;
		bool is_stereo;
		bool is_16_bit;
		bool loop;		// la or lalalala.... ?
		s8 rel_note;		// Offset in the frequency table from base note
		s8 finetune;		// -128: one halftone down, +127: one halftone up
		u32 loop_start;
		u32 loop_length;
		u8 volume;
		
		// These are calculated in the constructor
		//u32 size; // in bytes
		u32 sizes[20];
		u32 loop_starts[20];
		u32 loop_lengths[20];
		u32 sound_format;
		// Other formats may follow
};

#endif
