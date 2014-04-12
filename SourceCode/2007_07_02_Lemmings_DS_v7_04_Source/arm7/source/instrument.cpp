#include "instrument.h"

#ifdef ARM9
#include <stdio.h>
#endif

#include <stdlib.h>
//#include <string.h>
extern "C" {
  #include "ctools.h"
}

#include "../../generic/command.h"

#ifdef ARM9

Instrument::Instrument(const char *_name, u8 _type, u8 _volume)
	:type(_type), volume(_volume)
{
	name = (char*)malloc(MAX_INST_NAME_LENGTH+1);
	for(u16 i=0; i<MAX_INST_NAME_LENGTH+1; ++i) name[i] = '\0';
	my_strncpy(name, _name, MAX_INST_NAME_LENGTH);
	
	note_samples = (u8*)malloc(sizeof(u8)*MAX_OCTAVE*12);
	for(u16 i=0;i<MAX_OCTAVE*12; ++i) {
		note_samples[i] = 0;
	}
	samples = NULL;
	n_samples = 0;
}

Instrument::Instrument(const char *_name, Sample *_sample, u8 _volume)
	:type(INST_SAMPLE), volume(_volume)
{
	name = (char*)malloc(MAX_INST_NAME_LENGTH+1);
	for(u16 i=0; i<MAX_INST_NAME_LENGTH+1; ++i) name[i] = '\0';
	my_strncpy(name, _name, MAX_INST_NAME_LENGTH);
	
	samples = (Sample**)malloc(sizeof(Sample*)*1);
	samples[0] = _sample;
	n_samples = 1;
	
	note_samples = (u8*)malloc(sizeof(u8)*MAX_OCTAVE*12);
	for(u16 i=0;i<MAX_OCTAVE*12; ++i) {
		note_samples[i] = 0;
	}
}

Instrument::~Instrument()
{
	for(u8 i=0;i<n_samples;++i) {
		delete samples[i];
	}
	if(samples != NULL) {
		free(samples);
	}

	free(note_samples);
	
	free(name);
}

void Instrument::addSample(Sample *sample)
{
	n_samples++;
	samples = (Sample**)realloc(samples, sizeof(Sample*)*n_samples);
	samples[n_samples-1] = sample;
}

#endif

Sample *Instrument::getSample(u8 idx)
{
	if(n_samples>0) {
		return samples[idx];
	} else {
		return NULL;
	}
}

Sample *Instrument::getSampleForNote(u8 _note) {
	return samples[note_samples[_note]];
}

#ifdef ARM7

void Instrument::play(u8 _note, u8 _volume, u8 _channel /* effects here */) {

	switch(type) {
		case INST_SAMPLE:
			if(n_samples > 0) {
				samples[note_samples[_note]]->play(_note, (volume*_volume/255), _channel /* effects here */);
			}
			break;
			
		// case INST_SYNTH:
		//	synth->play(note, volume);
		//	break;
	}
}

#endif

#ifdef ARM9

void Instrument::setNoteSample(u16 note, u8 sample_id) {
	note_samples[note] = sample_id;
}

#endif

u8 Instrument::getNoteSample(u16 note) {
	return note_samples[note];
}

// Calculate how long in ms the instrument will play note given note
u32 Instrument::calcPlayLength(u8 note) {
	return samples[note_samples[note]]->calcPlayLength(note);
}

#ifdef ARM9

const char *Instrument::getName(void) {
	return name;
}

void Instrument::setName(const char *_name) {
	my_strncpy(name, _name, MAX_INST_NAME_LENGTH);
}

#endif

u16 Instrument::getSamples(void) {
	return n_samples;
}
