#ifndef _LINEAR_FREQ_TABLE_
#define _LINEAR_FREQ_TABLE_

#include <nds.h>

#define LINEAR_FREQ_TABLE_MIN_NOTE	216
#define LINEAR_FREQ_TABLE_MAX_NOTE	228
#define N_FINETUNE_STEPS			128
#define LINEAR_FREQ_TABLE_SIZE		(LINEAR_FREQ_TABLE_MAX_NOTE*N_FINETUNE_STEPS)

#ifdef ARM9
extern const u32 linear_freq_table[LINEAR_FREQ_TABLE_SIZE];
#endif

#ifdef ARM7
extern const u32 *linear_freq_table;
#endif

#endif
