//
// player.h
//
// NitroTracker player.
//
//                        by 0xtob
//                            http://nitrotracker.tobw.net
//
// distributed under LGPL
//

#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "song.h"

#define FADE_OUT_MS	10 // Milliseconds that a click-preventing fadeout takes

typedef struct {
	u16 row;		// Current row
	u8 pattern; 	// Current pattern
	u8 potpos;		// Current position in pattern order table
	u32 ms_per_tick;// How many milliseconds per tick (1 tick = time for 1 row)
	bool playing;	// D'uh!
	u32 tick_ms;	// ms spent in the current row
	u8 channel_active[MAX_CHANNELS];	// 0 for inactive, 1 for active
	u16 channel_ms_left[MAX_CHANNELS];	// how many milliseconds still to play?
	bool channel_loop[MAX_CHANNELS]; // Is the sample that is played looped?
	u8 channel_fadeout_active[MAX_CHANNELS]; // Is fadeout for channel i active?
	u8 channel_fadeout_ticks[MAX_CHANNELS]; // How long (ms) till channel i is faded out?
	u8 channel_volume[MAX_CHANNELS];
	bool waittick; // Wait until the end of the last tick before muting instruments
} PlayerState;

class Player {
	public:
		
		// Constructor. The first arument is a function pointer to a function that calls the
		// playTimerHandler() funtion of the player. This is a complicated solution, but
		// the timer callback must be a static function.
		Player(void (*_externalTimerHandler)(void)=0);
	
		//
		// Play Control
		//
	
		void setSong(Song *_song);
	
		// Plays the song till the end starting at the given pattern order table position
		void play(u8 potpos, u32 MRD_loop);
	
		// Plays on the specified pattern
		void playPtn(u8 ptn);
		
		void stop(u16 MRD_stop_type);
	
		// Play the note with the given settings
		void playNote(u8 note, u8 volume, u8 channel, u8 instidx);
	
		//
		// Callbacks
		//
	
		void registerTickCallback(void (*onTick_)(u16));
		void registerPatternChangeCallback(void (*onPatternChange_)(u8));
	
		//
		// Misc
		//
	
		void playTimerHandler(void);
		void stopSampleFadeoutTimerHandler(void);
		
		void MRD_SetVolume(u8);
	
	private:
		
		void startPlayTimer(void);
		void playTick(void);
	
		void initState(void);
	
		void handleFade(void);
	
		Song *song;
		PlayerState state;

		u8   MRD_player_volume;
		u32  MRD_loop;
	
		void (*externalTimerHandler)(void);
		void (*onTick)(u16);
		void (*onPatternChange)(u8);
	
		u32 lastticks; // Ticks for timer
};

#endif
