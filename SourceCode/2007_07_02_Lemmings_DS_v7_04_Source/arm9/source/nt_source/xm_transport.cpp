#include "xm_transport.h"
#include "tools.h"

#include <string.h>
#include <stdlib.h>
#include <malloc.h>

// DBG
#include <stdio.h>
#include <nds.h>
// DBG

char xmtransporterrors[][100] =
	{"fat init failed",
	"could not open file",
	"not a valid xm file",
	"memory full",
	"pattern read error",
	"file too big for ram",
	"",
	"pattern too long",
	"file is zero byte"};

/* ===================== PUBLIC ===================== */

// Loads a song from memory and puts it in the song argument
// returns 0 on success, an error code else
u16 XMTransport::load_memory(const void *memory_file_begin, Song **_song) {
   MEMFILE_FILE *xmfile = MEMFILE_fopen(memory_file_begin);

	//
	// Read header
	//
	
	// Magic number
	char magicnumber[18] = {0};
	MEMFILE_fread(magicnumber, 1, 17, xmfile);
	
	if( strcmp(magicnumber, "Extended Module: ") != 0 ) {
		MEMFILE_fclose(xmfile);
		return XM_TRANSPORT_ERROR_MAGICNUMBERINVALID;
	}
	
	// Song name
	char songname[21] = {0};
	MEMFILE_fread(songname, 1, 20, xmfile);

	// Skip uninteresting stuff like tracker name and version
	MEMFILE_fseek(xmfile, 23, SEEK_CUR);
	
	// Header size
	u32 header_size;
	MEMFILE_fread(&header_size, 4, 1, xmfile);
	
	// Song length (pot size)
	u16 pot_size;
	MEMFILE_fread(&pot_size, 2, 1, xmfile);
	
	// Restart position
	u16 restart_pos;
	MEMFILE_fread(&restart_pos, 2, 1, xmfile);
	
	// Number of channels
	u16 n_channels;
	MEMFILE_fread(&n_channels, 2, 1, xmfile);
	
	// Number of patterns
	u16 n_patterns;
	MEMFILE_fread(&n_patterns, 2, 1, xmfile);
	
	// Number of instruments
	u16 n_inst;
	MEMFILE_fread(&n_inst, 2, 1, xmfile);

	// Flags, currently only used for the frequency table (0: amiga, 1: linear)
	// TODO: Amiga freq table
	u16 flags;
	MEMFILE_fread(&flags, 2, 1, xmfile);
	
	// Tempo
	u16 tempo;
	MEMFILE_fread(&tempo, 2, 1, xmfile);
	
	// BPM
	u16 bpm;
	MEMFILE_fread(&bpm, 2, 1, xmfile);
	
	// Construct the song with the current info
	Song *song = new Song(tempo, bpm, n_channels);
	
	song->setName(songname);
	
	song->setRestartPosition(restart_pos);
	
	// Pattern order table
	u8 i, potentry;
	
	MEMFILE_fread(&potentry, 1, 1, xmfile);
	song->setPotEntry(0, potentry); // The first entry is made automatically by the song
	
	for(i=1;i<pot_size;++i) {
		MEMFILE_fread(&potentry, 1, 1, xmfile);
		song->potAdd(potentry);
	}
	MEMFILE_fseek(xmfile, 256-pot_size, SEEK_CUR);
	
	//
	// Read patterns
	//
	
	u8 pattern;
	for(pattern=0;pattern<n_patterns;++pattern) {
		
		// Pattern header length
		u32 pattern_header_length;
		MEMFILE_fread(&pattern_header_length, 4, 1, xmfile);
		
		//iprintf("ptn header: %u\n",pattern_header_length);
		
		// Skip packing type (is always 0)
		MEMFILE_fseek(xmfile, 1, SEEK_CUR);
		
		// Number of rows
		u16 n_rows;
		MEMFILE_fread(&n_rows, 2, 1, xmfile);
		
		if(n_rows > MAX_PATTERN_LENGTH) {
			MEMFILE_fclose(xmfile);
			return XM_TRANSPORT_PATTERN_TOO_LONG;
		}
		
		// Packed patterndata size
		u16 patterndata_size;
		MEMFILE_fread(&patterndata_size, 2, 1, xmfile);
		//TODO: Handle empty patterns (which are left out in the xm format)
		
		if(patterndata_size > 0) { // Read the pattern
		
			u8 *ptn_data = (u8*)mymemalign(2, patterndata_size);
			u32 bytes_read;
			bytes_read = MEMFILE_fread(ptn_data, patterndata_size, 1, xmfile);
			
			if(bytes_read != patterndata_size) {
				MEMFILE_fclose(xmfile);
				
				return XM_TRANSPORT_ERROR_PATTERN_READ;
			}
			
			u32 ptn_data_offset = 0;
			
			u8 chn, magicbyte, note, inst, vol, eff_type, eff_param;
			u16 row;

			if(pattern>0) {
				song->addPattern();
			}
			
			song->resizePattern(pattern, n_rows);
			Cell **ptn = song->getPattern(pattern);
			
			for(row=0;row<n_rows;++row) {
				for(chn=0;chn<n_channels;++chn) {
					
					magicbyte = ptn_data[ptn_data_offset];
					ptn_data_offset++;
					//MEMFILE_fread(&magicbyte, 1, 1, xmfile);
					
					bool read_note=true, read_inst=true, read_vol=true,
						read_eff_type=true, read_eff_param=true;
						
					if(magicbyte & 1<<7) { // It's the magic byte!
						
						read_note = magicbyte & 1<<0;
						read_inst = magicbyte & 1<<1;
						read_vol = magicbyte & 1<<2;
						read_eff_type = magicbyte & 1<<3;
						read_eff_param = magicbyte & 1<<4;
						
					} else { // It's the note!
						
						note = magicbyte;
						read_note = false;
						
					}
					
					if(read_note) {
						note = ptn_data[ptn_data_offset];
						ptn_data_offset++;
					} else {
						note = 0;
					}
					
					if(read_inst) {
						inst = ptn_data[ptn_data_offset];
						ptn_data_offset++;
					} else {
						inst = NO_INSTRUMENT;
					}
					
					if(read_vol) {
						vol = ptn_data[ptn_data_offset];
						ptn_data_offset++;
					} else {
						vol = 0; // 'Do nothing'
					}
					
					if(read_eff_type) {
						eff_type = ptn_data[ptn_data_offset];
						ptn_data_offset++;
					} else {
						eff_type = 0;
					}
					
					if(read_eff_param) {
						eff_param = ptn_data[ptn_data_offset];
						ptn_data_offset++;
					} else {
						eff_param = 0;
					}
					
					// Insert note into song
					if(note==0) {
						ptn[chn][row].note = EMPTY_NOTE;
					} else if(note==97) {
						ptn[chn][row].note = STOP_NOTE;
					} else {
						ptn[chn][row].note = note - 1;
					}
					
					if(inst != NO_INSTRUMENT) {
						ptn[chn][row].instrument = inst-1; // XM Inst indices start with 1
					} else {
						ptn[chn][row].instrument = NO_INSTRUMENT;
					}
					
					if((vol >= 16) && (vol <= 80)) {
						u16 volume = (vol-16)*4;
						if(volume==256) volume = 255;
						ptn[chn][row].volume = volume;
					} else if(vol==0) {
						ptn[chn][row].volume = 255;
					} else {
						// TODO: Parse volume effects!
					}
					
					ptn[chn][row].effect = eff_type;
					ptn[chn][row].effect_param = eff_param;
					
				}
				
			}
			
			free(ptn_data);
			
		} else { // Make an empty pattern

			if(pattern > 0) {
				song->addPattern();
			}
			
			song->resizePattern(pattern, n_rows);
		}
	
	}
	
	//
	// Read instruments
	//
	
	for(u8 inst=0; inst<n_inst; ++inst) {
		
		// Size
		u32 inst_size;
		MEMFILE_fread(&inst_size, 4, 1, xmfile);
		
		// Name
		char inst_name[23] = {0};
		MEMFILE_fread(inst_name, 1, 22, xmfile);
		
		Instrument *instrument = new Instrument(inst_name);
		if(instrument == 0) {
			MEMFILE_fclose(xmfile);
			return XM_TRANSPORT_ERROR_MEMFULL;
		}
		song->setInstrument(inst, instrument);
		
		u8 useless_byte;
		//MEMFILE_fseek(xmfile, 1, SEEK_CUR); // Skip inst type as it is always 0 (maybe I could use this?)
		MEMFILE_fread(&useless_byte, 1, 1, xmfile);
		
		// Number of samples
		u16 inst_n_samples;
		MEMFILE_fread(&inst_n_samples, 2, 1, xmfile);
		
		if(inst_n_samples > 0) {
		
			// Sample header size
			u32 sample_header_size;
			MEMFILE_fread(&sample_header_size, 4, 1, xmfile);
			
			// Sample number for all notes
			u8 *note_sample_numbers = (u8*)mymemalign(2, 96);
			MEMFILE_fread(note_sample_numbers, 1, 96, xmfile);
			
			// Points for volume envelope
			u16 *volume_envelope = (u16*)mymemalign(2, 48);
			MEMFILE_fread(volume_envelope, 2, 24, xmfile);
			//MEMFILE_fseek(xmfile, 48, SEEK_CUR); // Skipped for now
			free(volume_envelope);
			
			// Points for panning envelope
			u8 *throwaway_bytes = (u8*)mymemalign(2, 48);
			//MEMFILE_fseek(xmfile, 48, SEEK_CUR); // Skipped for now
			MEMFILE_fread(throwaway_bytes, 1, 48, xmfile);
			free(throwaway_bytes);
			
			/* Various other things:
				Number of volume points
				Number of panning points
				Volume sustain point
				Volume loop start point
				Volume loop end point
				Panning sustain point
				Panning loop start point
				Panning loop end point
				Volume  type: bit 0: On; 1: Sustain; 2: Loop
				Panning type: bit 0: On; 1: Sustain; 2: Loop
				Vibrato type
				Vibrato sweep
				Vibrato depth
				Vibrato rate
				Volume fadeout
			*/
			throwaway_bytes = (u8*)mymemalign(2, 16);
			//MEMFILE_fseek(xmfile, 16, SEEK_CUR); // Skipped for now
			MEMFILE_fread(throwaway_bytes, 1, 16, xmfile);
			free(throwaway_bytes);
			
			// Skip the rest of the header if is longer than the current position
			// This was really strange and took some time (and debugging with Tim)
			// to figure out. Why the fsck is the instrument header that much longer?
			// Well, don't care, skip it.
			if(inst_size>241) {
				throwaway_bytes = (u8*)mymemalign(2, inst_size-241);
				MEMFILE_fread(throwaway_bytes, 1, inst_size-241, xmfile);
				free(throwaway_bytes);
			}
			
			// Construct the instrument
			
			for(u8 i=0; i<96; ++i) {
				instrument->setNoteSample(i, note_sample_numbers[i]);
			}
			free(note_sample_numbers);
			
			// Load the sample(s)
			
			// Headers
			u8 *sample_headers = (u8*)mymemalign(2, inst_n_samples*40);
			MEMFILE_fread(sample_headers, 40, inst_n_samples, xmfile);
			
			for(u8 sample_id=0; sample_id<inst_n_samples; sample_id++) {

				// Sample length
				u32 sample_length;
				sample_length = *(u32*)(sample_headers+40*sample_id + 0);
				
				// Sample loop start
				u32 sample_loop_start;
				sample_loop_start = *(u32*)(sample_headers+40*sample_id + 4);

				// Sample loop length
				u32 sample_loop_length;
				sample_loop_length = *(u32*)(sample_headers+40*sample_id + 8);
				
				// Volume (0-64)
				u8 sample_volume;
				sample_volume = *(u8*)(sample_headers+40*sample_id + 12);
				
				if(sample_volume == 64) { // Convert scale to 0-255
					sample_volume = 255;
				} else {
					sample_volume *= 4;
				}
				
				// Finetune
				s8 sample_finetune;
				sample_finetune = *(s8*)(sample_headers+40*sample_id + 13);
				
				// Type byte (loop type and wether it's 8 or 16 bit)
				u8 sample_type;
				sample_type = *(u8*)(sample_headers+40*sample_id + 14);
				
				u8 loop_type = sample_type & 3;
				
				bool sample_is_16_bit = sample_type & 0x10;
				
				// Panning
				u8 sample_panning;
				sample_panning = *(u8*)(sample_headers+40*sample_id + 15);
				
				// Relative note
				s8 sample_rel_note;
				sample_rel_note = *(s8*)(sample_headers+40*sample_id + 16);
				
				// Sample name
				char sample_name[23] = {0};
				memcpy(sample_name, sample_headers+40*sample_id + 18, 22);
				
				// Sample data
				void *sample_data = mymemalign(2, sample_length); //mymemalign(2, sample_length);
				
				if(sample_data==NULL) {
					MEMFILE_fclose(xmfile);
					return XM_TRANSPORT_ERROR_MEMFULL;
				}
				
				MEMFILE_fread(sample_data, sample_length, 1, xmfile);
				//MEMFILE_fseek(xmfile, sample_length, SEEK_CUR);
				
				// Delta-decode
				if(sample_is_16_bit) {
					s16 last = 0;
					s16 *smp = (s16*)sample_data;
					for(u32 i=0;i<sample_length/2;++i) {
						smp[i] += last;
						last = smp[i];
					}
					
				} else {
					s8 last = 0;
					s8 *smp = (s8*)sample_data;
					for(u32 i=0;i<sample_length;++i) {
						smp[i] += last;
						last = smp[i];
					}
				}
				
				// Insert sample into the instrument
				u32 n_samples;
				if(sample_is_16_bit) {
					n_samples = sample_length/2;
				} else {
					n_samples = sample_length;
				}
				
				Sample *sample = new Sample(sample_data, n_samples, 8363, false, sample_is_16_bit, loop_type == 1, sample_volume);
				sample->setRelNote(sample_rel_note);
				sample->setFinetune(sample_finetune);
				sample->setLoopStart(sample_loop_start);
				sample->setLoopLength(sample_loop_length);
				instrument->addSample(sample);
			}
			
			free(sample_headers);
			
		} else {
		
			// If the instrument has no samples, skip the rest of the instrument header
			// (which should contain rubbish anyway)
			MEMFILE_fseek(xmfile, inst_size-29, SEEK_CUR);
		
		}
	}
	
	//
	// Finish up
	//
	MEMFILE_fclose(xmfile);
	
	// MATT - We have to destroy the old song!
	if (_song != NULL) {
      delete (*_song);
   }

	*_song = song;
	
	return 0;
}

const char *XMTransport::getError(u16 error_id)
{
	return xmtransporterrors[error_id-1];
}
