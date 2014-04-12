#ifndef FORMAT_TRANSPORT_H
#define FORMAT_TRANSPORT_H

#include "song.h"

// This is the abstract base class of transports.
// Transports are classes that handle import and export of songs.
class FormatTransport {
	public:
		
		// Loads a song from a file pots it into _song
		// Returns 0 on success, an error code else
		//virtual u16 load(const char *filename, Song **_song) = 0;

		// Saves a song to a file
		//virtual u16 save(const char *filename, Song *song) = 0;
	
		virtual ~FormatTransport() {};
	
	private:
};

#endif
