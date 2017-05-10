#ifndef __WOLFCC_WTIME_H__
#define __WOLFCC_WTIME_H__
#include <sys/time.h>
#include <stdint.h>

/* Return the UNIX time in seconds */
extern uint32_t get_stime(void);

/* Return the UNIX time in useconds */
extern int64_t get_ustime(void);

/* Return the UNIX time in milliseconds */
extern int64_t get_mstime(void);

/* Return the UNIX time pair in seconds and milliseconds */
extern void get_time_pair(uint32_t& s, uint32_t& m);

#endif
