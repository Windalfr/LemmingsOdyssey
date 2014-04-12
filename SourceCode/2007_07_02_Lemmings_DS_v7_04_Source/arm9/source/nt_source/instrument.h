//
// instrument.h
//
// This class represents a tracker instrument.
//
//                        by 0xtob
//                            http://nitrotracker.tobw.net
//
// distributed under LGPL
//

#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include "sample.h"

#define MAX_OCTAVE		9
#define MAX_NOTE	(MAX_OCTAVE*12-1)

#define INST_SAMPLE	0
#define INST_SYNTH	1 // Yes, who knows!

#define MAX_INST_NAME_LENGTH	22

class Instrument {
	public:
		Instrument(const char *_name, u8 _type=INST_SAMPLE, u8 _volume=255);
		Instrument(const char *_name, Sample *_sample, u8 _volume=255);
		~Instrument();
	
		void addSample(Sample *sample);
		Sample *getSample(u8 idx);
		Sample *getSampleForNote(u8 _note);
		void play(u8 _note, u8 _volume, u8 _channel /* effects here */);
		void setNoteSample(u16 note, u8 sample_id);
		u8 getNoteSample(u16 note);
		
		// Calculate how long in ms the instrument will play note given note
		u32 calcPlayLength(u8 note);
		
		const char *getName(void);
		void setName(const char *_name);
	
		u16 getSamples(void);
		
	private:
		char *name;
		// More to come ...
		
		u8 type;
		u8 volume;
		
		// Actually this would become dirty if more types emerge.
		// I think making an abstract Instrument base class that
		// subclasses SampleInstrument and SynthInstrument would
		// be derived from would be the best choice. But I'll do
		// this when required ^^
	
		Sample **samples;
		u16 n_samples;
	
		// Synth *synth;
	
		u8 *note_samples;
};

#endif
