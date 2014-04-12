#include "player.h"

#include "song.h"
extern "C" {
  #include "demokit.h"
  #include "ctools.h"
}
#include "../../generic/command.h"

/* ===================== PUBLIC ===================== */

Player::Player(void (*_externalTimerHandler)(void))
	:MRD_player_volume(255), MRD_loop(1), externalTimerHandler(_externalTimerHandler)
{
	initState();
	
	demoInit();

	// Init callbacks
	onTick = 0;
	onPatternChange = 0;
	
	// Init fading
	my_memset(state.channel_fadeout_active, 0, sizeof(state.channel_fadeout_active));
	my_memset(state.channel_fadeout_ticks, 0, sizeof(state.channel_fadeout_ticks));
	my_memset(state.channel_volume, 0, sizeof(state.channel_volume));
}

void Player::setSong(Song *_song)
{
	song = _song;
	initState();
	
	// Init fading
	my_memset(state.channel_fadeout_active, 0, sizeof(state.channel_fadeout_active));
	my_memset(state.channel_fadeout_ticks, 0, sizeof(state.channel_fadeout_ticks));
	my_memset(state.channel_volume, 0, sizeof(state.channel_volume));
}

// Plays the song till the end starting at pattern order table position potpos
void Player::play(u8 potpos, u32 MRD_loop)
{
	// Mark all channels inactive
	my_memset(state.channel_active, 0, sizeof(state.channel_active));
	my_memset(state.channel_ms_left, 0, sizeof(state.channel_ms_left));
	my_memset(state.channel_loop, 0, sizeof(state.channel_loop));

	state.potpos = potpos;
	state.pattern = song->pattern_order_table[state.potpos];
	
	// Jump to top pos of pattern
	state.row = 0;
	
	// Calculate nr. of ms per tick and reset counter
	state.ms_per_tick = song->getMsPerTick();
	state.tick_ms = 0;
	
	lastticks = getTicks();

	startPlayTimer();
	
	this->MRD_loop = MRD_loop;

	state.playing = true;
}

void Player::stop(u16 MRD_stop_type) // We need a stop type to tell ARM9 the reason for the stop.
{
	// If stop was pressed although we were not playing,
	// playing samples shall be stopped. For this, the play
	// timer needs to be turned on, because it periodiacally
	// calls the fade handler
	if(state.playing == false) {
		startPlayTimer();
	}
	
	state.playing = false;

	// Stop all playing samples
	u8 end;
	if(song->n_channels < MAX_CHANNELS) {
		end = song->n_channels;
	} else {
		end = MAX_CHANNELS;
	}

	for(u8 chn = 0; chn < end; chn++) {
		state.channel_fadeout_active[chn] = 1;
		state.channel_fadeout_ticks[chn] = FADE_OUT_MS;
	}
	                             
   // Notify the ARM9 that we stopped.
	CommandNotifyStop(MRD_stop_type);
}

// Play the note with the given settings
void Player::playNote(u8 note, u8 volume, u8 channel, u8 instidx)
{
   // MRD volume intervention. This master volume allows the master
   // volume of the player to be changed from the ARM9 at runtime.
   volume = (u8)(((u32)volume * (u32)MRD_player_volume) >> 8); // This means MRD_player_volume is 0 - 255

	// Stop possibly active fadeouts
	state.channel_fadeout_active[channel] = 0;
	state.channel_fadeout_ticks[channel] = 0;

	Instrument *inst = song->instruments[instidx];
	
	if(inst != 0) {
		inst->play(note, volume, channel);
	}

	state.channel_volume[channel] = volume*inst->getSampleForNote(note)->getVolume()/2/255;
}

void Player::registerTickCallback(void (*onTick_)(u16)) {
	onTick = onTick_;
}

void Player::registerPatternChangeCallback(void (*onPatternChange_)(u8)) {
	onPatternChange = onPatternChange_;
}

void Player::playTimerHandler(void)
{
	u32 passed_time = getTicks() - lastticks;
	lastticks = getTicks();
	
	// Fading stuff
	handleFade();
	
	if(state.playing==false) {
		// If the song was stopped, just wait for the fadeouts to finish
		bool fadeouts_finished = true;
		for(u8 channel=0; channel<MAX_CHANNELS; ++channel) {
			if(state.channel_fadeout_active[channel] != 0) {
				fadeouts_finished = false;
			}
		}
		
		if(fadeouts_finished == true) {
			TIMER0_CR = 0;
		}
		
		return; // Don't process the stuff after this
	}
	
	// Update tick ms
	state.tick_ms += passed_time;
	
	// if the number of ms per tick is reached, go to the next tick, play it and call the tick handler
	if(state.tick_ms >= state.ms_per_tick) {
		//state.tick_ms = 0;
		state.tick_ms -= state.ms_per_tick;

		if(state.waittick==true) {
			stop(MRD_NOTIFY_STOP_COMMAND_STOP_TYPE_REACHED_END_OF_SONG);
			state.waittick=false;
			return;
		}
		
		playTick();
		
		if(onTick != NULL) {
			onTick(state.row);
		}
		
		if((state.row==0)&&(onPatternChange != 0)) {
				onPatternChange(state.potpos);
		}
		
		// Go to the next pattern if the end of current pattern is reached
		if(state.row >= song->patternlengths[state.pattern] - 1 ) {
			if(state.potpos < song->getPotLength()-1) {
				state.potpos++;
				state.pattern = song->pattern_order_table[state.potpos];
				state.row = 0;
			} else if(song->getRestartPosition()>0) { // Do we have a restart position?
				state.potpos = song->getRestartPosition();
				state.pattern = song->pattern_order_table[state.potpos];
				state.row = 0;
			} else if (MRD_loop != 1) { // If we've ran out of patterns, and there's no tracker restart commands
			                            // Loop if MRD_loop is zero (infinite loops), or greater than one.
			                            // MRD_loop counts the number of remaining loops including the CURRENT playback

			   if (MRD_loop > 0) --MRD_loop; // Decrement the number of remaining loops this Player has.
			                                 // But only if we're not working with infinite loops here.

            // Send this Player back to the start.
				state.potpos = 0;
				state.pattern = song->pattern_order_table[state.potpos];
				state.row = 0;
         } else {
				// End after the last tick
				state.waittick = true;
			}
		} else {
			state.row++;
		}
	}

	// Update active channels
	for(u8 channel=0; channel<song->n_channels && channel<MAX_CHANNELS; ++channel) {
		if(state.channel_ms_left[channel] > 0) {
			if(state.channel_ms_left[channel] > passed_time) {
				state.channel_ms_left[channel] -= passed_time;
			} else {
				state.channel_ms_left[channel] = 0;
			}
			
			if((state.channel_ms_left[channel]==0)&&(state.channel_loop[channel]==false)) {
				state.channel_active[channel] = 0;
			}
		}
	}
	
	// Check if we are short before the next tick. (As long as a fadeout would take)
	if(state.tick_ms>=state.ms_per_tick-FADE_OUT_MS) {
		
		// If so, check if for any of the active channels a new note starts in the next row.
		u8 nextNote;
		for(u8 channel=0; channel<song->n_channels && channel<MAX_CHANNELS; ++channel) {
			if(state.channel_active[channel] == 1) {
				// TODO: Handle pattern end!
				nextNote = song->patterns[state.pattern][channel][state.row].note;
				if(nextNote!=EMPTY_NOTE) {
					// If so, fade out to avoid a click.
					//iprintf("fade %u\n",channel);
					state.channel_fadeout_active[channel] = 1;
					state.channel_fadeout_ticks[channel] = FADE_OUT_MS;
				}
			}
		}
	}
}

void Player::MRD_SetVolume(u8 volume) {
   MRD_player_volume = volume;

	for(u8 channel=0; channel<MAX_CHANNELS; ++channel) {
		if(state.channel_fadeout_active[channel] == 0) {
         // Time for the MRD volume intervention
         volume = (u8)(((u32)state.channel_volume[channel] * (u32)MRD_player_volume) >> 8); // This means MRD_player_volume is 0 - 255

			SCHANNEL_VOL(channel) = SOUND_VOL(volume);
      }
	}
}

/* ===================== PRIVATE ===================== */

void Player::startPlayTimer(void)
{
	TIMER0_DATA = TIMER_FREQ_64(1000); // Call handler every millisecond
	TIMER0_CR = TIMER_ENABLE | TIMER_IRQ_REQ | TIMER_DIV_64;
}

void Player::playTick(void)
{
	// Play all notes in this row
	for(u8 channel=0; channel < song->n_channels && channel<MAX_CHANNELS; ++channel) {
		u8 note   = song->patterns[state.pattern][channel][state.row].note;
		u8 volume = song->patterns[state.pattern][channel][state.row].volume;
		u8 inst   = song->patterns[state.pattern][channel][state.row].instrument;
	
		if((note!=EMPTY_NOTE)&&(note!=STOP_NOTE)&&(song->instruments[inst]!=0)) {
			playNote(note, volume, channel, inst);
			
			state.channel_active[channel] = 1;
			if(song->instruments[inst]->getSampleForNote(note)->getLoop() != 0) {
				state.channel_loop[channel] = true;
				state.channel_ms_left[channel] = 0;
			} else {
				state.channel_loop[channel] = false;
				state.channel_ms_left[channel] = song->instruments[inst]->calcPlayLength(note);
			}
		}
	}
}

void Player::initState(void)
{
	state.row = 0;
	state.pattern = 0;
	state.potpos = 0;
	state.playing = false;
	state.waittick = false;
	my_memset(state.channel_active, 0, sizeof(state.channel_active));
	my_memset(state.channel_ms_left, 0, sizeof(state.channel_ms_left));
}

void Player::handleFade(void)
{
	// Find channels that need to be faded
	for(u8 channel=0; channel<MAX_CHANNELS; ++channel) {
		if(state.channel_fadeout_active[channel] == 1) {
			state.channel_fadeout_ticks[channel]--; // Decrement ms until fadeout is complete
			
			// Calculate volume from initial volume and remaining fadeout time
			u8 volume = state.channel_volume[channel] *
				    state.channel_fadeout_ticks[channel] / FADE_OUT_MS ;

         // Time for the MRD volume intervention
         volume = (u8)(((u32)volume * (u32)MRD_player_volume) >> 8); // This means MRD_player_volume is 0 - 255

			SCHANNEL_VOL(channel) = SOUND_VOL(volume);

			// If we reached 0 ticks, disable the fader and the channel
			if(state.channel_fadeout_ticks[channel] == 0) {
				state.channel_fadeout_active[channel] = 0;
				SCHANNEL_CR(channel) = 0;
			}
		}
	}
}
