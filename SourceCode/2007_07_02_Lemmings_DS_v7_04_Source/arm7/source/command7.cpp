/*
  Functions for the ARM7 to process the commands from the ARM9.Based
  on code from the MOD player example posted to the GBADEV forums.
  Chris Double (chris.double@double.co.nz)
  http://www.double.co.nz/nintendo_ds
*/
#include <nds.h>
#include <stdarg.h>

#include "../../generic/command.h"
#include "player.h"
#include "linear_freq_table.h"

extern Player *player;

static void RecvCommandSetSong(SetSongCommand *c)
{
	player->setSong((Song*)c->ptr);
}

static void RecvCommandStartPlay(StartPlayCommand *c)
{
	player->play(c->potpos, c->MRD_loop);
}

static void RecvCommandStopPlay(StopPlayCommand *c) {
	player->stop(MRD_NOTIFY_STOP_COMMAND_STOP_TYPE_SONG_STOP_REQUESTED);
}

static void RecvCommandSetFreqTable(SetFreqTableCommand *c) {
	linear_freq_table = c->ptr;
}

static void RecvCommandPlayInst(PlayInstCommand *c) {
	player->playNote(c->note, 255, 0, c->inst);
}

static void RecvCommandMRD_SetPlayerVolume(MRD_SetPlayerVolumeCommand *c) {
	player->MRD_SetVolume(c->volume);
}

static void RecvCommandMRD_PlayOneShotSample(MRD_PlayOneShotSampleCommand *c) {
   int channel = c->channel;

   SCHANNEL_CR(channel) = 0;
   SCHANNEL_TIMER(channel) = SOUND_FREQ(c->frequency);
   SCHANNEL_SOURCE(channel) = (uint32)c->data;
   SCHANNEL_LENGTH(channel) = c->length;
   SCHANNEL_CR(channel) = SCHANNEL_ENABLE
                        | ((c->loop) ? (SOUND_REPEAT) : (SOUND_ONE_SHOT))
                        | ((c->format) ? (SOUND_FORMAT_16BIT) : (SOUND_FORMAT_8BIT))
                        | ((c->pan) << 16)
                        | SOUND_VOL(c->volume);
}

void CommandUpdateRow(u16 row) {

	Command* command = &commandControl->command[commandControl->currentCommand];
	command->destination = DST_ARM9;
	command->commandType = UPDATE_ROW;
	
	UpdateRowCommand *c = &command->updateRow;
	c->row = row;
	
	commandControl->currentCommand++;
	commandControl->currentCommand %= MAX_COMMANDS;
}

void CommandUpdatePotPos(u16 potpos) {
	
	Command* command = &commandControl->command[commandControl->currentCommand];
	command->destination = DST_ARM9;
	command->commandType = UPDATE_POTPOS;
	
	UpdatePotPosCommand *c = &command->updatePotPos;
	c->potpos = potpos;

	commandControl->currentCommand++;
	commandControl->currentCommand %= MAX_COMMANDS;
}

void CommandNotifyStop(u16 MRD_stop_type) {

	Command* command = &commandControl->command[commandControl->currentCommand];
	command->destination = DST_ARM9;
	command->commandType = NOTIFY_STOP;
	
	MRD_NotifyStopCommand *c = &command->MRD_notifyStopCommand;
   c->stop_type = MRD_stop_type;

	commandControl->currentCommand++;
	commandControl->currentCommand %= MAX_COMMANDS;
}

void CommandProcessCommands() {
	static int currentCommand = 0;
	while(currentCommand != commandControl->currentCommand) {
		Command* command = &commandControl->command[currentCommand];
		
		if(command->destination == DST_ARM7) {
		
			switch(command->commandType) {
				case SET_SONG:
					RecvCommandSetSong(&command->setSong);
					break;
				case START_PLAY:
					RecvCommandStartPlay(&command->startPlay);
					break;
				case STOP_PLAY:
					RecvCommandStopPlay(&command->stopPlay);
					break;
				case SET_FREQ_TABLE:
					RecvCommandSetFreqTable(&command->setFreqTable);
					break;
				case PLAY_INST:
					RecvCommandPlayInst(&command->playInst);
					break;
				case MRD_SET_PLAYER_VOLUME:
					RecvCommandMRD_SetPlayerVolume(&command->MRD_setPlayerVolume);
			   break;
				case MRD_PLAY_ONE_SHOT_SAMPLE:
					RecvCommandMRD_PlayOneShotSample(&command->MRD_playOneShotSample);
			   break;
				default:
					break;
			}
		
		}
		currentCommand++;
		currentCommand %= MAX_COMMANDS;
	}
}
