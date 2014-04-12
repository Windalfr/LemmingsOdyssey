//
// song.h
//
// This class represents a song. The format is kept open. The current feature set
// is a subset of XM, but export and import for mod, it, s3m could come. To edit a
// pattern, get its pointer with getPattern().
//
//                        by 0xtob
//                            http://nitrotracker.tobw.net
//
// distributed under LGPL
//

#ifndef SONG_H
#define SONG_H

#include <nds.h>

#include "instrument.h"

#define MAX_INSTRUMENTS			128
#define MAX_PATTERNS			256
#define MAX_POT_LENGTH			256
#define MAX_CHANNELS			16 // XM has 32, but 16 is DS hardware limited unless software mixing were implemented. Yeah, perhaps.
#define MAX_PATTERN_LENGTH		256
#define DEFAULT_PATTERN_LENGTH	64
#define DEFAULT_BPM				125
#define DEFAULT_SPEED			6
#define DEFAULT_CHANNELS		4

#define EMPTY_NOTE				255
#define STOP_NOTE				254

#define NO_INSTRUMENT			255
#define MAX_SONG_NAME_LENGTH	20

typedef struct {
	u8 note;
	u8 instrument;
	u8 volume;
	u8 effect;
	u8 effect_param;
} Cell;

class Song {
	friend class Player;
	
	public:
		
		Song(u8 _speed=DEFAULT_SPEED, u8 _bpm=DEFAULT_BPM, u8 _channels=DEFAULT_CHANNELS);
		
		~Song();
		
		void setExternalTimerHandler(void (*_externalTimerHandler)(void));
		
		Cell **getPattern(u8 idx);
		u8 getChannels(void);
		u16 getPatternLength(u8 idx);
		u32 getMsPerTick(void);
		Instrument *getInstrument(u8 instidx);
		u8 getInstruments(void);
		
		bool setSampleInstrument(u8 idx, const char *filename, const char *name=0);
		void setInstrument(u8 idx, Instrument *instrument);
		
		//
		// Playback control
		//
		
		// POT functions
		void potAdd(u8 ptn=0);
		void potDel(u8 element);
		void potIns(u8 idx, u8 pattern);
		u8 getPotLength(void);
		u8 getPotEntry(u8 idx);
		void setPotEntry(u8 idx, u8 value);
		
		void addPattern(u16 length=DEFAULT_PATTERN_LENGTH);
		
		// More/less channels
		void channelAdd(void);
		void channelDel(void);
		
		u8 getNumPatterns(void);
		
		void resizePattern(u8 ptn, u16 newlength);
		
		// The most important functions
		void setName(const char *_name);
		const char *getName(void);
		
		void setRestartPosition(u8 _restart_position);
		u8 getRestartPosition(void);
		
		u8 getTempo(void);
		u8 getBPM(void);
		
		void setTempo(u8 _tempo);
		void setBpm(u8 _bpm);
		
		// Zapping
		void zapPatterns(void);
		void zapInstruments(void);
		
	private:
		
		inline void clearCell(Cell *cell);
		void killPatterns(void);
		void killInstruments(void);
		
		u8 speed;
		u8 bpm;
		u8 n_channels;
		u8 restart_position;
		
		u16 *patternlengths;
		u8 *pattern_order_table;
		Instrument **instruments;
		
		char *name;
		
		u16 n_patterns;
		u16 potsize;
		
		Cell ***patterns;
};

#endif
