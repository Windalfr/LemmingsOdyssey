#if !defined(COMMAND_H)
#define COMMAND_H

#define DST_ARM7	0
#define DST_ARM9	1

#define DEBUGSTRSIZE 40

#include <nds.h>

/*
  Structures and functions to allow the ARM9 to send commands to the
  ARM7. Based on code from the MOD player example posted to the GBADEV
  forums.
*/

/* Enumeration of commands that the ARM9 can send to the ARM7 */
enum CommandType {
	SET_SONG,
	START_PLAY,
	STOP_PLAY,
	SET_FREQ_TABLE,
	SET_DEBUG_STR,
	UPDATE_ROW,
	UPDATE_POTPOS,
	PLAY_INST,
	NOTIFY_STOP,
	MRD_SET_PLAYER_VOLUME,
	MRD_PLAY_ONE_SHOT_SAMPLE
};

/* Command parameters for playing a sound sample */
struct PlaySampleSoundCommand
{
	int channel;
	int frequency;
	const void* data;
	int length;
	int volume;
	int format;
	bool loop;
};

struct SetSongCommand {
	void *ptr;
};

struct StartPlayCommand {
	u8  potpos;
	u32 MRD_loop; // Zero is infinite.
};

struct StopPlayCommand {
};

struct SetFreqTableCommand {
	const u32 *ptr;
};

struct UpdateRowCommand {
	u16 row;
};

struct UpdatePotPosCommand {
	u16 potpos;
};

struct PlayInstCommand {
	u8 inst;
	u8 note;
};

struct MRD_SetPlayerVolumeCommand {
	u8 volume;
};

struct MRD_PlayOneShotSampleCommand {
   int channel;
   int frequency;
   const void *data;
   int length;
   int volume;
   int format;
   int pan;
   bool loop;
};

struct MRD_NotifyStopCommand {
   // This struct allows the player to track what kind of song end happened.
   
   // Did the song just run out of patterns, or was it forced?
   
#define MRD_NOTIFY_STOP_COMMAND_STOP_TYPE_SONG_IS_STILL_PLAYING 0 // Default value.
#define MRD_NOTIFY_STOP_COMMAND_STOP_TYPE_SONG_STOP_REQUESTED   1
#define MRD_NOTIFY_STOP_COMMAND_STOP_TYPE_REACHED_END_OF_SONG   2
	u16 stop_type;
};

/* The ARM9 fills out values in this structure to tell the ARM7 what
   to do. */
struct Command {
	u8 destination;
	CommandType commandType;
	union {
		void* data;
		SetSongCommand               setSong;
		StartPlayCommand             startPlay;
		StopPlayCommand              stopPlay;
		SetFreqTableCommand          setFreqTable;
		UpdateRowCommand             updateRow;
		UpdatePotPosCommand          updatePotPos;
		PlayInstCommand              playInst;
		MRD_SetPlayerVolumeCommand   MRD_setPlayerVolume;
		MRD_PlayOneShotSampleCommand MRD_playOneShotSample;
		MRD_NotifyStopCommand        MRD_notifyStopCommand;
	};
};

/* Maximum number of commands */
#define MAX_COMMANDS 40

/* A structure shared between the ARM7 and ARM9. The ARM9
   places commands here and the ARM7 reads and acts upon them.
*/
struct CommandControl {
	Command command[MAX_COMMANDS];
	int currentCommand;
	int return_data;
};

/* Address of the shared CommandControl structure */
#define commandControl ((CommandControl*)((uint32)(IPC) + sizeof(TransferRegion)))

#if defined(ARM9)
void CommandInit();
void CommandStopSample(int channel);
void CommandStartRecording(u16* buffer, int length);
 int CommandStopRecording();
void CommandSetSong(void *song);
void CommandStartPlay(u8 potpos, u32 MRD_loop = 0);
void CommandStopPlay(void);
void CommandSetFreqTable(const u32 *ptr);
void CommandPlayInst(u8 inst, u8 note);
void CommandMRD_SetPlayerVolume(u8 volume);
void CommandMRD_PlayOneShotSample(int channel, int frequency, const void* data, int length, int volume, int format, int pan, bool loop);
#endif

void CommandProcessCommands();

#if defined(ARM7)
void CommandUpdateRow(u16 row);
void CommandUpdatePotPos(u16 potpos);
void CommandNotifyStop(u16 type);
#endif

#endif
