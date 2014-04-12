//
// xm_transport.h
//
// This class implements loading from the XM file format
// introduced by Fasttracker II. Man, those were the days!
//
//                        by 0xtob
//                            http://nitrotracker.tobw.net
//
// distributed under LGPL
//

#ifndef XM_TRANSPORT
#define XM_TRANSPORT

#include "format_transport.h"

#include "memfile_file.h"

#define XM_TRANSPORT_ERROR_INITFAIL				1
#define XM_TRANSPORT_ERROR_FOPENFAIL			2
#define XM_TRANSPORT_ERROR_MAGICNUMBERINVALID	3
#define XM_TRANSPORT_ERROR_MEMFULL				4
#define XM_TRANSPORT_ERROR_PATTERN_READ			5
#define XM_TRANSPORT_FILE_TOO_BIG_FOR_RAM		6
#define XM_TRANSPORT_NULL_INSTRUMENT			7 // Deprecated
#define XM_TRANSPORT_PATTERN_TOO_LONG			8
#define XM_TRANSPORT_FILE_ZERO_BYTE				9

class XMTransport: public FormatTransport {
	public:
		
		// Loads a song from memory and puts it in the song argument
		// returns 0 on success, an error code else
		u16 load_memory(const void *memory_file_begin, Song **_song);
		
		const char *getError(u16 error_id);
		
	private:
};

#endif
