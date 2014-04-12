/*
  Structures and functions to allow the ARM9 to send commands to the
  ARM7. Based on code from the MOD player example posted to the GBADEV
  forums.
  Chris Double (chris.double@double.co.nz)
  http://www.double.co.nz/nintendo_ds
*/
#include <nds.h>
#include <string.h>
#include <stdio.h>

#include "../../generic/command.h"

extern void handlePotPosChangeFromSong(u16 newpotpos);

// This handles the notify stop command
extern void (*NotifyStopHandler)(MRD_NotifyStopCommand *c);

void CommandInit() {
	memset(commandControl, 0, sizeof(CommandControl));
}

void RecvCommandUpdateRow(UpdateRowCommand *c) {
	
}

void RecvCommandUpdatePotPos(UpdatePotPosCommand *c) {
	//iprintf("potpos: %u\n", c->potpos);
}

void RecvCommandNotifyStop(MRD_NotifyStopCommand *c) {
	(*NotifyStopHandler)(c);
}

void CommandProcessCommands() {
	
	static int currentCommand = 0;
	while(currentCommand != commandControl->currentCommand) {
		Command* command = &commandControl->command[currentCommand];
		
		if(command->destination == DST_ARM9) {
		
			switch(command->commandType) {
				case UPDATE_ROW:
					RecvCommandUpdateRow(&command->updateRow);
					break;
				
				case UPDATE_POTPOS:
					RecvCommandUpdatePotPos(&command->updatePotPos);
					break;
				
				case NOTIFY_STOP:
					RecvCommandNotifyStop(&command->MRD_notifyStopCommand);
					break;
				
				default:
					break;
			}
		
		}
			
		currentCommand++;
		currentCommand %= MAX_COMMANDS;
	}
}

void CommandSetSong(void *song) {
	
	Command* command = &commandControl->command[commandControl->currentCommand];
	command->destination = DST_ARM7;
	SetSongCommand* c = &command->setSong;

	command->commandType = SET_SONG; 
	c->ptr = song;

	commandControl->currentCommand++;
	commandControl->currentCommand %= MAX_COMMANDS;
}

void CommandStartPlay(u8 potpos, u32 MRD_loop) {

	Command* command = &commandControl->command[commandControl->currentCommand];
	command->destination = DST_ARM7;
	StartPlayCommand* c = &command->startPlay;

	command->commandType = START_PLAY;
	c->potpos   = potpos;
	c->MRD_loop = MRD_loop;

	commandControl->currentCommand++;
	commandControl->currentCommand %= MAX_COMMANDS;
}

void CommandStopPlay(void) {
	
	Command* command = &commandControl->command[commandControl->currentCommand];
	command->destination = DST_ARM7;
	command->commandType = STOP_PLAY; 

	commandControl->currentCommand++;
	commandControl->currentCommand %= MAX_COMMANDS;
}

void CommandSetFreqTable(const u32 *ptr) {
	
	Command* command = &commandControl->command[commandControl->currentCommand];
	command->destination = DST_ARM7;
	SetFreqTableCommand* c = &command->setFreqTable;

	command->commandType = SET_FREQ_TABLE; 
	c->ptr = ptr;

	commandControl->currentCommand++;
	commandControl->currentCommand %= MAX_COMMANDS;	
}

void CommandPlayInst(u8 inst, u8 note) {
	
	Command* command = &commandControl->command[commandControl->currentCommand];
	command->destination = DST_ARM7;
	command->commandType = PLAY_INST;
	
	PlayInstCommand* c = &command->playInst;

	c->inst = inst;
	c->note = note;

	commandControl->currentCommand++;
	commandControl->currentCommand %= MAX_COMMANDS;
}

void CommandMRD_SetPlayerVolume(u8 volume) {  

	Command* command = &commandControl->command[commandControl->currentCommand];
	command->destination = DST_ARM7;
	command->commandType = MRD_SET_PLAYER_VOLUME;
	
	MRD_SetPlayerVolumeCommand* c = &command->MRD_setPlayerVolume;

	c->volume = volume;

	commandControl->currentCommand++;      
	commandControl->currentCommand %= MAX_COMMANDS;
}

void CommandMRD_PlayOneShotSample(int channel, int frequency, const void* data, int length, int volume, int format, int pan, bool loop) {

	Command* command = &commandControl->command[commandControl->currentCommand];
	command->destination = DST_ARM7;
	command->commandType = MRD_PLAY_ONE_SHOT_SAMPLE;

	MRD_PlayOneShotSampleCommand* c = &command->MRD_playOneShotSample;

   c->channel   = channel;
   c->frequency = frequency;
   c->data      = data;
   c->length    = length;
   c->volume    = volume;
   c->format    = format;
   c->pan       = pan;
   c->loop      = loop;

	commandControl->currentCommand++;
	commandControl->currentCommand %= MAX_COMMANDS;
}
