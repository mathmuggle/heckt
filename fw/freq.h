#ifndef _HECKT_FREQ_H_
#define _HECKT_FREQ_H_

#include <inttypes.h>

bool freq_init(uint16_t gate_ms);

// Measurement control
void freq_start(void);
void freq_stop(void);

// Frequency reading
bool freq_ready(void);
double freq_read(void);

#endif
